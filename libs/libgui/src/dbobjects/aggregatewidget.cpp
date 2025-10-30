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

#include "aggregatewidget.h"
#include "guiutilsns.h"

AggregateWidget::AggregateWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Aggregate)
{
	QGridLayout *grid=nullptr;
	Ui_AggregateWidget::setupUi(this);
	QSpacerItem *spacer=nullptr;
	QFrame *frame=nullptr;

	initial_cond_hl=new SyntaxHighlighter(initial_cond_txt, true, false, font().pointSizeF());
	initial_cond_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	final_func_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	transition_func_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	sort_op_sel=new ObjectSelectorWidget(ObjectType::Operator, this);

	input_type=new PgSQLTypeWidget(this, tr("Input Data Type"));
	state_type=new PgSQLTypeWidget(this, tr("State Data Type"));

	input_types_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
											CustomTableWidget::EditButton, true, this);
	input_types_tab->setColumnCount(1);

	funcaoagregacao_grid->addWidget(final_func_sel,0,1,1,1);
	funcaoagregacao_grid->addWidget(transition_func_sel,1,1,1,1);
	funcaoagregacao_grid->addWidget(sort_op_sel,2,1,1,1);

	grid=new QGridLayout;
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	grid->addWidget(input_type,0,0);
	grid->addWidget(input_types_tab,1,0);
	state_input_types_twg->widget(0)->setLayout(grid);

	grid=new QGridLayout;
	spacer=new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);

	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	grid->addWidget(state_type,0,0);
	grid->addItem(spacer,1,0);
	state_input_types_twg->widget(1)->setLayout(grid);

	connect(input_types_tab, &CustomTableWidget::s_rowAdded, this, __slot_n(this, AggregateWidget::handleDataType));
	connect(input_types_tab, &CustomTableWidget::s_rowUpdated, this, __slot_n(this, AggregateWidget::handleDataType));

	frame=generateInformationFrame(tr("An aggregate function that accepts the types <em><strong>typeA</strong></em> and <em><strong>typeB</strong></em> as input types and which type of state is <em><strong>state_type</strong></em>, must obey the following rules: <br/><br/> <strong> &nbsp;&nbsp;&nbsp;• Final Function:</strong> <em>void final_function(<strong>state_type</strong>)</em><br/>  <strong> &nbsp;&nbsp;&nbsp;• Transition Function:</strong> <em><strong>state_type</strong> transition_function(<strong>state_type</strong>, <strong>typeA</strong>, <strong>typeB</strong>)</em>"));
	funcaoagregacao_grid->addWidget(frame, funcaoagregacao_grid->count()+1, 0, 1, 2);
	frame->setParent(this);

	configureFormLayout(funcaoagregacao_grid, ObjectType::Aggregate);

	setRequiredField(state_type);
	setRequiredField(input_type);
	setRequiredField(transition_func_sel);
	setRequiredField(transition_func_lbl);

	configureTabOrder({ final_func_sel, transition_func_sel, sort_op_sel });
	setMinimumSize(620,700);
}

void AggregateWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Aggregate *aggregate)
{
	unsigned count, i;
	PgSqlType type;

	BaseObjectWidget::setAttributes(model,op_list, aggregate, schema);

	input_type->setAttributes(type, model, false);
	state_type->setAttributes(type, model, false);
	final_func_sel->setModel(model);
	transition_func_sel->setModel(model);
	sort_op_sel->setModel(model);

	if(aggregate)
	{
		final_func_sel->setSelectedObject(aggregate->getFunction(Aggregate::FinalFunc));
		transition_func_sel->setSelectedObject(aggregate->getFunction(Aggregate::TransitionFunc));
		sort_op_sel->setSelectedObject(aggregate->getSortOperator());
		initial_cond_txt->setPlainText(aggregate->getInitialCondition());

		input_types_tab->blockSignals(true);
		count=aggregate->getDataTypeCount();

		for(i=0; i < count; i++)
		{
			input_types_tab->addRow();
			type=aggregate->getDataType(i);
			input_types_tab->setRowData(QVariant::fromValue<PgSqlType>(type), i);
			input_types_tab->setCellText(*type,i,0);
		}
		input_types_tab->blockSignals(false);
		input_types_tab->clearSelection();

		state_type->setAttributes(aggregate->getStateType(), model, false);
	}
}

void AggregateWidget::handleDataType(int row)
{
	try
	{
		PgSqlType type;

		type=input_type->getPgSQLType();
		input_types_tab->setRowData(QVariant::fromValue<PgSqlType>(type), row);
		input_types_tab->setCellText(*type,row,0);
	}
	catch(Exception &e)
	{
		if(input_types_tab->getCellText(row, 0).isEmpty())
			input_types_tab->removeRow(row);

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void AggregateWidget::applyConfiguration()
{
	try
	{
		Aggregate *aggregate=nullptr;
		unsigned count, i;

		startConfiguration<Aggregate>();

		aggregate=dynamic_cast<Aggregate *>(this->object);
		aggregate->setInitialCondition(initial_cond_txt->toPlainText());
		aggregate->setStateType(state_type->getPgSQLType());

		aggregate->removeDataTypes();
		count=input_types_tab->getRowCount();

		for(i=0; i < count; i++)
			aggregate->addDataType(input_types_tab->getRowData(i).value<PgSqlType>());

		aggregate->setFunction(Aggregate::TransitionFunc, dynamic_cast<Function *>(transition_func_sel->getSelectedObject()));
		aggregate->setFunction(Aggregate::FinalFunc, dynamic_cast<Function *>(final_func_sel->getSelectedObject()));
		aggregate->setSortOperator(dynamic_cast<Operator *>(sort_op_sel->getSelectedObject()));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

