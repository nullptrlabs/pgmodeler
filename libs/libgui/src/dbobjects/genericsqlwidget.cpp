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

#include "genericsqlwidget.h"
#include "guiutilsns.h"

GenericSQLWidget::GenericSQLWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::GenericSql)
{
	std::vector<ObjectType> types;

	Ui_GenericSQLWidget::setupUi(this);
	configureFormLayout(genericsql_grid, ObjectType::GenericSql);

	// Configuring the object types accepted by object references
	types = BaseObject::getObjectTypes(false, { ObjectType::Database, ObjectType::GenericSql,
																							 ObjectType::Permission, ObjectType::Relationship,
																							 ObjectType::Tag, ObjectType::Textbox });
	types.push_back(ObjectType::Column);
	obj_refs_wgt = new ReferencesWidget(types, false, this);

	QVBoxLayout *vbox = new QVBoxLayout(references_tab);
	vbox->addWidget(obj_refs_wgt);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);

	definition_txt = GuiUtilsNs::createNumberedTextEditor(attribs_tbw->widget(0), true);
	definition_hl = new SyntaxHighlighter(definition_txt);
	definition_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	definition_cp=new CodeCompletionWidget(definition_txt, true);

	comment_edt->setVisible(false);
	comment_lbl->setVisible(false);

	preview_txt = GuiUtilsNs::createNumberedTextEditor(attribs_tbw->widget(2), false);
	preview_txt->setReadOnly(true);
	preview_hl = new SyntaxHighlighter(preview_txt);
	preview_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	attribs_tbw->widget(0)->layout()->setContentsMargins(GuiUtilsNs::LtMargins);
	attribs_tbw->widget(0)->layout()->addWidget(definition_txt);

	attribs_tbw->widget(2)->layout()->setContentsMargins(GuiUtilsNs::LtMargins);
	attribs_tbw->widget(2)->layout()->addWidget(preview_txt);

	setMinimumSize(700, 500);

	connect(attribs_tbw, &QTabWidget::currentChanged, this, [this](int idx){
		if(idx == attribs_tbw->count() - 1)
			updateCodePreview();
	});
}

void GenericSQLWidget::setAttributes(DatabaseModel *model, OperationList *op_list, GenericSQL *genericsql)
{
	BaseObjectWidget::setAttributes(model, op_list, genericsql);
	std::vector<Reference> refs;

	if(genericsql)
	{
		dummy_gsql = *genericsql;
		definition_txt->setPlainText(genericsql->getDefinition());
		refs = genericsql->getObjectsReferences();
	}

	obj_refs_wgt->setAttributes(this->model, refs);
	definition_cp->configureCompletion(model, definition_hl);
}

void GenericSQLWidget::updateCodePreview()
{
	try
	{
		if(!name_edt->text().isEmpty() && !definition_txt->toPlainText().isEmpty())
		{
			if(!name_edt->text().isEmpty())
				dummy_gsql.setName(name_edt->text());

			dummy_gsql.removeObjectReferences();
			dummy_gsql.addReferences(obj_refs_wgt->getObjectReferences());
			dummy_gsql.setDefinition(definition_txt->toPlainText());
			dummy_gsql.setCodeInvalidated(true);
			preview_txt->setPlainText(dummy_gsql.getSourceCode(SchemaParser::SqlCode));
		}
		else
			preview_txt->setPlainText(QString("-- %1 --").arg(tr("No object name, SQL code or references defined! Preview unavailable.")));
	}
	catch(Exception &e)
	{
		preview_txt->setPlainText(QString("/* %1 */").arg(e.getExceptionsText()));
	}
}

void GenericSQLWidget::applyConfiguration()
{
	try
	{
		GenericSQL *genericsql=nullptr;

		startConfiguration<GenericSQL>();
		genericsql=dynamic_cast<GenericSQL *>(this->object);
		dummy_gsql.setDefinition(definition_txt->toPlainText());
		dummy_gsql.removeObjectReferences();
		dummy_gsql.addReferences(obj_refs_wgt->getObjectReferences());
		*genericsql = dummy_gsql;

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
