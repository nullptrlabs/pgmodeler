/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/

#include "codepreviewwidget.h"
#include "taskprogresswidget.h"
#include "guiutilsns.h"
#include "pgsqlversions.h"
#include "messagebox.h"
#include "customuistyle.h"

CodePreviewWidget::CodePreviewWidget(QWidget *parent): QWidget(parent)
{
	Ui_CodePreviewWidget::setupUi(this);

	prev_pg_ver = prev_code_opt = -1;
	dbmodel = nullptr;
	object = nullptr;

	sqlcode_txt = GuiUtilsNs::createNumberedTextEditor(sqlcode_wgt, true);
	sqlcode_txt->setReadOnly(true);

	xmlcode_txt = GuiUtilsNs::createNumberedTextEditor(xmlcode_wgt, true);
	xmlcode_txt->setReadOnly(true);

	hl_sqlcode = new SyntaxHighlighter(sqlcode_txt);
	hl_xmlcode = new SyntaxHighlighter(xmlcode_txt);

	version_cmb->addItems(PgSqlVersions::AllVersions);

	connect(sourcecode_twg, &QTabWidget::currentChanged, this, &CodePreviewWidget::generateSourceCode);
	connect(version_cmb, &QComboBox::currentIndexChanged, this, [this](int){
		generateSourceCode(SchemaParser::SqlCode);
	});

	connect(code_options_cmb, &QComboBox::currentIndexChanged, this, [this](int) {
		generateSourceCode(SchemaParser::SqlCode);
	});

	GuiUtilsNs::configureBuddyWidgets(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint,
															{ obj_name_frm, separator_ln,
																separator2_ln, code_opts_frm });

	setMinimumSize(800, 600);
}

void CodePreviewWidget::generateSQLCode()
{
	sqlcode_txt->clear();
	prev_pg_ver = version_cmb->currentIndex();
	prev_code_opt = code_options_cmb->currentIndex();

	BaseObject::setPgSQLVersion(version_cmb->currentText());

	if(object && object->getObjectType() == ObjectType::Database)
		sqlcode_txt->setPlainText(object->getSourceCode(SchemaParser::SqlCode));
	else
	{
		sqlcode_txt->setPlainText(dbmodel->getSQLDefinition(objects,
																												static_cast<DatabaseModel::CodeGenMode>(code_options_cmb->currentIndex()),
																												true));
	}

#ifdef DEMO_VERSION
	if(!sqlcode_txt->toPlainText().isEmpty())
	{
		QString code = tr("/*******************************************************/\n\
/* ATTENTION: The SQL code of the objects is purposely */\n\
/* truncated in the demo version!                      */\n\
/*******************************************************/\n\n") +
		sqlcode_txt->toPlainText();
		sqlcode_txt->setPlainText(code);
	}
#endif

	if(sqlcode_txt->toPlainText().isEmpty())
		sqlcode_txt->setPlainText(tr("-- SQL code unavailable for this type of object --"));
}

void CodePreviewWidget::generateXMLCode()
{
	xmlcode_txt->clear();

#ifdef DEMO_VERSION
#warning "DEMO VERSION: XML code preview disabled."
	xmlcode_txt->setPlainText(tr("<!-- XML code preview disabled in demonstration version -->"));
#else
	QString xml_code;

	for(auto &obj : objects)
		xml_code.append(obj->getSourceCode(SchemaParser::XmlCode));

	xmlcode_txt->setPlainText(xml_code);
#endif
}

void CodePreviewWidget::generateSourceCode(int def_type)
{
	if((def_type == SchemaParser::XmlCode && !xmlcode_txt->document()->isEmpty()) ||
		 (def_type == SchemaParser::SqlCode &&
			(prev_pg_ver == version_cmb->currentIndex() &&
			 prev_code_opt == code_options_cmb->currentIndex())))
		return;

	TaskProgressWidget task_prog_wgt;

	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		if(object && object->getObjectType() == ObjectType::Database)
		{
			task_prog_wgt.setWindowTitle(tr("Generating source code..."));
			task_prog_wgt.show();
			connect(dbmodel, &DatabaseModel::s_objectLoaded, &task_prog_wgt,
							qOverload<int, QString, unsigned>(&TaskProgressWidget::updateProgress));
		}

		if(def_type == SchemaParser::SqlCode)
			generateSQLCode();
		else
			generateXMLCode();

		task_prog_wgt.close();
		disconnect(dbmodel, nullptr, &task_prog_wgt, nullptr);
		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		task_prog_wgt.close();
		disconnect(dbmodel, nullptr, &task_prog_wgt, nullptr);
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void CodePreviewWidget::setAttributes(DatabaseModel *model, const std::vector<BaseObject *> &objs)
{
	try
	{
		objects = objs;
		dbmodel = model;

		if(objs.size() <= 1)
		{
			if(objects.empty())
				objects.push_back(model);

			object = objects[0];
		}

		type_name_lbl->setVisible(object != nullptr);
		obj_id_lbl->setVisible(object != nullptr);
		type_icon_lbl->setVisible(object != nullptr);
		oid_icon_lbl->setVisible(object != nullptr);
		separator_ln->setVisible(object != nullptr);
		separator2_ln->setVisible(object != nullptr);

		if(object)
		{
			obj_name_lbl->setText(object->getSignature().remove('"'));
			type_name_lbl->setText(object->getTypeName());
			obj_id_lbl->setText(QString::number(object->getObjectId()));
			type_icon_lbl->setPixmap(GuiUtilsNs::getPixmap(object->getObjectType()));
		}
		else
		{
			obj_name_lbl->setText(tr("Previewing the code of <strong>%1</strong> objects").arg(objects.size()));
		}

		code_options_cmb->setEnabled(std::any_of(objs.begin(), objs.end(), [](auto &obj){
			return obj->getObjectType() != ObjectType::Database &&
						 obj->getObjectType() != ObjectType::Textbox &&
						 obj->getObjectType() != ObjectType::BaseRelationship &&
						 obj->getObjectType() != ObjectType::Relationship;
		}));

#ifdef DEMO_VERSION
#warning "DEMO VERSION: SQL code display options disabled."
		code_options_cmb->setEnabled(false);
#endif

		if(!hl_sqlcode->isConfigurationLoaded())
			hl_sqlcode->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

		if(!hl_xmlcode->isConfigurationLoaded())
			hl_xmlcode->loadConfiguration(GlobalAttributes::getXMLHighlightConfPath());

		generateSourceCode(SchemaParser::SqlCode);
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}
