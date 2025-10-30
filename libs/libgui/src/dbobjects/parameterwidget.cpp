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

#include "parameterwidget.h"

ParameterWidget::ParameterWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Parameter)
{
	QGridLayout *parameter_grid=nullptr;
	QSpacerItem *spacer=nullptr;

	Ui_ParameterWidget::setupUi(this);

	data_type=new PgSQLTypeWidget(this);
	parameter_grid=new QGridLayout(this);
	spacer=new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding);
	parameter_grid->setContentsMargins(0, 0, 0, 0);

	parameter_grid->addWidget(default_value_lbl, 0, 0, 1, 1);
	parameter_grid->addWidget(default_value_edt, 0, 1, 1, 3);
	parameter_grid->addWidget(mode_lbl, 1, 0, 1, 1);
	parameter_grid->addWidget(param_in_chk, 1, 1, 1, 1);
	parameter_grid->addWidget(param_out_chk, 1, 2, 1, 1);
	parameter_grid->addWidget(param_variadic_chk, 1, 3, 1, 1);
	parameter_grid->addWidget(data_type,2, 0, 1, 4);
	parameter_grid->addItem(spacer, parameter_grid->count()+1,0);

	configureFormLayout(parameter_grid, ObjectType::Parameter);
	connect(param_variadic_chk, &QCheckBox::toggled, param_in_chk, &QCheckBox::setDisabled);
	connect(param_variadic_chk, &QCheckBox::toggled, param_out_chk, &QCheckBox::setDisabled);
	connect(param_in_chk, &QCheckBox::toggled, this, &ParameterWidget::enableVariadic);
	connect(param_out_chk, &QCheckBox::toggled, this, &ParameterWidget::enableVariadic);

	setMinimumSize(500, 200);
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

