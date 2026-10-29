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

#include "parameterwidget.h"

ParameterWidget::ParameterWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Parameter)
{
	Ui_ParameterWidget::setupUi(this);

	data_type = new PgSQLTypeWidget(this);
	parameter_lt->insertWidget(1, data_type);
	configureFormLayout(parameter_lt, ObjectType::Parameter);

	connect(param_variadic_chk, &QCheckBox::toggled, param_in_chk, &QCheckBox::setDisabled);
	connect(param_variadic_chk, &QCheckBox::toggled, param_out_chk, &QCheckBox::setDisabled);
	connect(param_in_chk, &QCheckBox::toggled, this, &ParameterWidget::enableVariadic);
	connect(param_out_chk, &QCheckBox::toggled, this, &ParameterWidget::enableVariadic);

	setMinimumSize(550, 300);
}

void ParameterWidget::enableVariadic()
{
	param_variadic_chk->setEnabled(!param_in_chk->isChecked() &&
																 !param_out_chk->isChecked());

	if(!param_variadic_chk->isEnabled())
		param_variadic_chk->setChecked(false);
}

void ParameterWidget::setAttributes(Parameter param, DatabaseModel *model)
{
	this->parameter=param;

	param_in_chk->setChecked(param.isIn());
	param_out_chk->setChecked(param.isOut());
	param_variadic_chk->setChecked(param.isVariadic());
	default_value_edt->setText(param.getDefaultValue());
	data_type->setAttributes(param.getType(), model, false);

	BaseObjectWidget::setAttributes(model,&this->parameter, nullptr);
}

void ParameterWidget::applyConfiguration()
{
	try
	{
		parameter.setDefaultValue(default_value_edt->text());
		parameter.setType(data_type->getPgSQLType());
		parameter.setIn(param_in_chk->isChecked());
		parameter.setOut(param_out_chk->isChecked());
		parameter.setVariadic(param_variadic_chk->isChecked());

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Parameter ParameterWidget::getParameter()
{
	return parameter;
}

