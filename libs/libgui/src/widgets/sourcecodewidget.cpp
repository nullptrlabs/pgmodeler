/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
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

#include "sourcecodewidget.h"
#include "taskprogresswidget.h"
#include "guiutilsns.h"
#include "pgsqlversions.h"

SourceCodeWidget::SourceCodeWidget(QWidget *parent): BaseObjectWidget(parent)
{
	Ui_SourceCodeWidget::setupUi(this);
	configureFormLayout(codigofonte_grid, ObjectType::BaseObject);
	comment_lbl->setVisible(false);
	comment_edt->setVisible(false);

	prev_pg_ver = prev_code_opt = -1;
	hl_sqlcode = nullptr;
	hl_xmlcode = nullptr;

	sqlcode_txt = GuiUtilsNs::createNumberedTextEditor(sqlcode_wgt, true);
	sqlcode_txt->setReadOnly(true);

	xmlcode_txt = GuiUtilsNs::createNumberedTextEditor(xmlcode_wgt, true);
	xmlcode_txt->setReadOnly(true);

	name_edt->setReadOnly(true);
	version_cmb->addItems(PgSqlVersions::AllVersions);

	connect(sourcecode_twg, &QTabWidget::currentChanged, this, &SourceCodeWidget::generateSourceCode);
	connect(version_cmb, &QComboBox::currentIndexChanged, this, [this](int){
		generateSourceCode(SchemaParser::SqlCode);
	});

	connect(code_options_cmb, &QComboBox::currentIndexChanged, this, [this](int) {
		generateSourceCode(SchemaParser::SqlCode);
	});

	hl_sqlcode=new SyntaxHighlighter(sqlcode_txt);
	hl_xmlcode=new SyntaxHighlighter(xmlcode_txt);

	setMinimumSize(800, 600);
}

void SourceCodeWidget::saveSQLCode()
{
	try
	{
		GuiUtilsNs::selectAndSaveFile(sqlcode_txt->toPlainText().toUtf8(),
																	 tr("Save SQL code as..."),
																	 QFileDialog::AnyFile,
																	 { tr("SQL code (*.sql)"), tr("All files (*.*)") }, {}, "sql",
																	 QString("%1-%2.sql").arg(object->getSchemaName(), object->getName()));
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SourceCodeWidget::generateSQLCode()
{
	sqlcode_txt->clear();
	prev_pg_ver = version_cmb->currentIndex();
	prev_code_opt = code_options_cmb->currentIndex();

	BaseObject::setPgSQLVersion(version_cmb->currentText());

	if(object && object->getObjectType() == ObjectType::Database)
		sqlcode_txt->setPlainText(object->getSourceCode(SchemaParser::SqlCode));
	else
		sqlcode_txt->setPlainText(model->getSQLDefinition(objects, static_cast<DatabaseModel::CodeGenMode>(code_options_cmb->currentIndex())));

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

void SourceCodeWidget::generateXMLCode()
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

void SourceCodeWidget::generateSourceCode(int def_type)
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
			connect(this->model, &DatabaseModel::s_objectLoaded, &task_prog_wgt, qOverload<int, QString, unsigned>(&TaskProgressWidget::updateProgress));
		}

		if(def_type == SchemaParser::SqlCode)
			generateSQLCode();
		else
			generateXMLCode();

		task_prog_wgt.close();
		disconnect(this->model, nullptr, &task_prog_wgt, nullptr);

		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();

		task_prog_wgt.close();
		disconnect(this->model, nullptr, &task_prog_wgt, nullptr);

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SourceCodeWidget::setAttributes(DatabaseModel *model, const std::vector<BaseObject *> &objs)
{
	try
	{
		objects = objs;

		if(objs.size() <= 1)
		{
			if(objects.empty())
				objects.push_back(model);

			BaseObjectWidget::setAttributes(model, objects[0], nullptr);
			name_edt->setText(QString("%1 (%2)").arg(object->getSignature(), object->getTypeName()));
			obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(object->getObjectType())));
		}
		else
		{
			BaseObjectWidget::setAttributes(model, static_cast<BaseObject *>(nullptr), nullptr);
			name_edt->setVisible(false);
			name_lbl->setVisible(false);
			obj_icon_lbl->setVisible(false);
		}

		protected_obj_frm->setVisible(false);
		obj_id_lbl->setVisible(false);

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

void SourceCodeWidget::applyConfiguration()
{
	emit s_closeRequested();
}

