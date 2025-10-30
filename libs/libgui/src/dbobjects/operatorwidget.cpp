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

#include "operatorwidget.h"
#include "guiutilsns.h"

OperatorWidget::OperatorWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Operator)
{
	QGridLayout *grid=nullptr;
	unsigned i, i1;
	QFrame *frame=nullptr;

	Ui_OperatorWidget::setupUi(this);

	arg_types[0]=nullptr;
	arg_types[0]=new PgSQLTypeWidget(this, tr("Left Argument Type"));
	arg_types[1]=nullptr;
	arg_types[1]=new PgSQLTypeWidget(this, tr("Right Argument Type"));

	grid=new QGridLayout;
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	grid->addWidget(arg_types[0],0,0);
	grid->addWidget(arg_types[1],1,0);

	grid->addItem(new QSpacerItem(10,1,QSizePolicy::Fixed,QSizePolicy::Expanding), 2, 0);

	frame=generateInformationFrame(tr("To create a unary operator it is necessary to specify as <strong><em>'any'</em></strong> one of its arguments. Additionally, the function that defines the operator must have only one parameter and this, in turn, must have the same data type of the the argument of unary operator."));
	grid->addWidget(frame, 3, 0);
	attributes_twg->widget(0)->setLayout(grid);


	grid=dynamic_cast<QGridLayout *>(attributes_twg->widget(1)->layout());
	for(i=Operator::FuncOperator; i <= Operator::FuncRestrict; i++)
	{
		functions_sel[i]=nullptr;
		functions_sel[i]=new ObjectSelectorWidget(ObjectType::Function, this);

		if(i!=Operator::FuncOperator)
			grid->addWidget(functions_sel[i],i,1,1,1);
	}

	for(i=Operator::OperCommutator, i1=3; i <= Operator::OperNegator; i++,i1++)
	{
		operators_sel[i]=nullptr;
		operators_sel[i]=new ObjectSelectorWidget(ObjectType::Operator, this);
		grid->addWidget(operators_sel[i],i1,1,1,1);
	}

	operator_grid->addWidget(functions_sel[0],0,1,1,3);
	configureFormLayout(operator_grid, ObjectType::Operator);

	setRequiredField(operator_func_lbl);
	setRequiredField(functions_sel[0]);
	configureTabOrder({ functions_sel[0], merges_chk, hashes_chk, arg_types[0], arg_types[1] });

	setMinimumSize(600, 620);
}

void OperatorWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Operator *oper)
{
	unsigned i;
	PgSqlType left_type, right_type;

	BaseObjectWidget::setAttributes(model,op_list, oper, schema);

	for(i=Operator::FuncOperator; i <= Operator::FuncRestrict; i++)
		functions_sel[i]->setModel(model);

	for(i=Operator::OperCommutator; i <= Operator::OperNegator; i++)
		operators_sel[i]->setModel(model);

	if(oper)
	{
		hashes_chk->setChecked(oper->isHashes());
		merges_chk->setChecked(oper->isMerges());

		for(i=Operator::FuncOperator; i <= Operator::FuncRestrict; i++)
			functions_sel[i]->setSelectedObject(oper->getFunction(static_cast<Operator::FunctionId>(i)));

		for(i=Operator::OperCommutator; i <= Operator::OperNegator; i++)
			operators_sel[i]->setSelectedObject(oper->getOperator(static_cast<Operator::OperatorId>(i)));

		left_type=oper->getArgumentType(Operator::LeftArg);
		right_type=oper->getArgumentType(Operator::RightArg);
	}

	arg_types[0]->setAttributes(left_type, model, false);
	arg_types[1]->setAttributes(right_type, model, false);
}

void OperatorWidget::applyConfiguration()
{
	try
	{
		unsigned i;
		Operator *oper=nullptr;
		startConfiguration<Operator>();

		oper=dynamic_cast<Operator *>(this->object);

		BaseObjectWidget::applyConfiguration();

		oper->setHashes(hashes_chk->isChecked());
		oper->setMerges(merges_chk->isChecked());

		for(i=Operator::LeftArg; i <= Operator::RightArg; i++)
			oper->setArgumentType(arg_types[i]->getPgSQLType(),
														static_cast<Operator::ArgumentId>(i));

		for(i=Operator::FuncOperator; i <= Operator::FuncRestrict; i++)
			oper->setFunction(dynamic_cast<Function *>(functions_sel[i]->getSelectedObject()),
												static_cast<Operator::FunctionId>(i));

		for(i=Operator::OperCommutator; i <= Operator::OperNegator; i++)
			oper->setOperator(dynamic_cast<Operator *>(operators_sel[i]->getSelectedObject()),
												static_cast<Operator::OperatorId>(i));

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

