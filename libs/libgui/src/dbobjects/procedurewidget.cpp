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

#include "procedurewidget.h"

ProcedureWidget::ProcedureWidget(QWidget *parent): BaseFunctionWidget(parent, ObjectType::Procedure)
{
	func_config_twg->widget(0)->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Preferred, QSizePolicy::Expanding));
	configureFormLayout(base_function_grid, ObjectType::Procedure);

	connect(parameters_tab, &CustomTableWidget::s_rowAdded, this, __slot(this, ProcedureWidget::showParameterForm));
	connect(parameters_tab, &CustomTableWidget::s_rowEdited, this, __slot(this, ProcedureWidget::showParameterForm));
	connect(parameters_tab, &CustomTableWidget::s_rowDuplicated, this, &ProcedureWidget::duplicateParameter);

	configureTabOrder();
	setMinimumSize(650, 700);
}

void ProcedureWidget::handleParameter(Parameter param, int result)
{
	BaseFunctionWidget::handleParameter(parameters_tab, param, result, true);
}

void ProcedureWidget::duplicateParameter(int curr_row, int new_row)
{
	BaseFunctionWidget::duplicateParameter(parameters_tab, curr_row, new_row, true);
}

void ProcedureWidget::showParameterForm()
{
	BaseFunctionWidget::showParameterForm(parameters_tab, true);
}

void ProcedureWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Procedure *proc)
{
	BaseFunctionWidget::setAttributes(model, op_list, schema, proc);
}

void ProcedureWidget::applyConfiguration()
{
	try
	{
		Procedure *proc = nullptr;

		startConfiguration<Procedure>();
		proc = dynamic_cast<Procedure *>(this->object);
		applyBasicConfiguration(proc);
		BaseFunctionWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
