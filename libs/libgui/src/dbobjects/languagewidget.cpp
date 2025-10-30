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

#include "languagewidget.h"

LanguageWidget::LanguageWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Language)
{
	QFrame *frame=nullptr;
	Ui_LanguageWidget::setupUi(this);

	func_handler_sel=nullptr;
	func_validator_sel=nullptr;
	func_inline_sel=nullptr;

	func_handler_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	func_validator_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	func_inline_sel=new ObjectSelectorWidget(ObjectType::Function, this);

	language_grid->addWidget(func_handler_sel,1,1,1,2);
	language_grid->addWidget(func_validator_sel,2,1,1,2);
	language_grid->addWidget(func_inline_sel,3,1,1,2);

	configureFormLayout(language_grid, ObjectType::Language);

	frame=generateInformationFrame(tr("The functions to be assigned to the language should have, respectively, the following signatures:<br/><br/>  <strong>Handler Function:</strong> <em>language_handler function()</em><br/>  <strong>Validator Function:</strong> <em>void function(oid)</em><br/>  <strong>Inline Function:</strong> <em>void function(internal)</em>"));

	language_grid->addItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding), language_grid->count()+1, 0, 1, 0);
	language_grid->addWidget(frame, language_grid->count()+1, 0, 1, 0);
	frame->setParent(this);

	configureTabOrder({ trusted_chk, func_handler_sel, func_validator_sel, func_inline_sel });

	setMinimumSize(600, 420);
}

void LanguageWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Language *language)
{
	BaseObjectWidget::setAttributes(model, op_list, language);

	func_handler_sel->setModel(model);
	func_validator_sel->setModel(model);
	func_inline_sel->setModel(model);

	if(language)
	{
		trusted_chk->setChecked(language->isTrusted());
		func_handler_sel->setSelectedObject(language->getFunction(Language::HandlerFunc));
		func_validator_sel->setSelectedObject(language->getFunction(Language::ValidatorFunc));
		func_inline_sel->setSelectedObject(language->getFunction(Language::InlineFunc));
	}
}

void LanguageWidget::applyConfiguration()
{
	try
	{
		Language *language=nullptr;

		startConfiguration<Language>();

		language=dynamic_cast<Language *>(this->object);
		language->setTrusted(trusted_chk->isChecked());

		language->setFunction(dynamic_cast<Function *>(func_handler_sel->getSelectedObject()), Language::HandlerFunc);
		language->setFunction(dynamic_cast<Function *>(func_validator_sel->getSelectedObject()), Language::ValidatorFunc);
		language->setFunction(dynamic_cast<Function *>(func_inline_sel->getSelectedObject()), Language::InlineFunc);

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

