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
	Ui_LanguageWidget::setupUi(this);

	func_handler_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	func_validator_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	func_inline_sel = new ObjectSelectorWidget(ObjectType::Function, this);

	func_handler_lt->addWidget(func_handler_sel);
	func_validator_lt->addWidget(func_validator_sel);
	func_inline_lt->addWidget(func_inline_sel);

	configureTabbedLayout(false);
	configureTabOrder({ trusted_chk, func_handler_sel, func_validator_sel, func_inline_sel });

	setMinimumSize(600, 500);
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

