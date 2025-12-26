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
#include "customuistyle.h"

OperatorWidget::OperatorWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Operator)
{
	QGridLayout *grid=nullptr;
	unsigned i, i1;

	Ui_OperatorWidget::setupUi(this);
	CustomUiStyle::setStyleHint(CustomUiStyle::AltDefaultFrmHint, option_frm);

	arg_types[0] = new PgSQLTypeWidget(arguments_tab, tr("Left argument type"));
	arg_types[1] = new PgSQLTypeWidget(arguments_tab, tr("Right argument type"));

	QVBoxLayout *vlayout = GuiUtilsNs::createVBoxLayout(GuiUtilsNs::LtMargin,
																											GuiUtilsNs::LtSpacing,
																											arguments_tab);
	vlayout->addWidget(arg_types[0]);
	vlayout->addWidget(arg_types[1]);
	vlayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

	for(auto &func_sel : functions_sel)
		func_sel = new ObjectSelectorWidget(ObjectType::Function, this);

	op_func_lt->addWidget(functions_sel[Operator::FuncOperator]);
	restriction_lt->addWidget(functions_sel[Operator::FuncRestrict]);
	join_lt->addWidget(functions_sel[Operator::FuncJoin]);

	for(auto &op_sel : operators_sel)
		op_sel = new ObjectSelectorWidget(ObjectType::Operator, this);

	commutator_lt->addWidget(operators_sel[Operator::OperCommutator]);
	negator_lt->addWidget(operators_sel[Operator::OperNegator]);

	layout()->removeWidget(func_opts_wgt);
	extra_wgts_lt->addWidget(func_opts_wgt);
	configureTabbedLayout(attributes_twg);

	setRequiredField(operator_func_lbl);
	setRequiredField(functions_sel[0]);
	//configureTabOrder({ functions_sel[0], merges_chk, hashes_chk, arg_types[0], arg_types[1] });

	setMinimumSize(600, 500);
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

