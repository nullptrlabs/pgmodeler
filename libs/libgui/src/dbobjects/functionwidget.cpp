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

#include "functionwidget.h"
#include "guiutilsns.h"
#include "customuistyle.h"

FunctionWidget::FunctionWidget(QWidget *parent): BaseFunctionWidget(parent, ObjectType::Function)
{
	Ui_FunctionWidget::setupUi(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::AltDefaultFrmHint, func_opts_frm);

	ret_type = GuiUtilsNs::createWidgetInParent<PgSQLTypeWidget>(ret_type_parent, tr("Return type"));
	ret_type_parent->layout()->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));

	return_tab = GuiUtilsNs::createWidgetInParent<CustomTableWidget>(GuiUtilsNs::LtMargin,
																																	 CustomTableWidget::AllButtons ^
																																	 CustomTableWidget::UpdateButton, true,
																																	 ret_table_grp);
	return_tab->setColumnCount(2);
	return_tab->setHeaderLabel(tr("Column"), 0);
	return_tab->setHeaderIcon(GuiUtilsNs::getIcon("column"), 0);
	return_tab->setHeaderLabel(tr("Type"), 1);
	return_tab->setHeaderIcon(GuiUtilsNs::getIcon("usertype"), 1);

	ret_table_grp->setVisible(false);

	func_type_cmb->addItems(FunctionType::getTypes());
	behavior_cmb->addItems(BehaviorType::getTypes());
	parallel_cmb->addItems(ParallelType::getTypes());

	connect(ret_method_cmb, &QComboBox::activated, this, &FunctionWidget::alternateReturnTypes);
	connect(parameters_tab, &CustomTableWidget::s_rowAdded, this, &FunctionWidget::showParameterForm);
	connect(parameters_tab, &CustomTableWidget::s_rowEdited, this, &FunctionWidget::showParameterForm);
	connect(parameters_tab,  &CustomTableWidget::s_rowDuplicated, this, &FunctionWidget::duplicateParameter);

	connect(return_tab, &CustomTableWidget::s_rowAdded, this, &FunctionWidget::showParameterForm);
	connect(return_tab, &CustomTableWidget::s_rowEdited, this, &FunctionWidget::showParameterForm);
	connect(return_tab, &CustomTableWidget::s_rowDuplicated, this, &FunctionWidget::duplicateParameter);

	lang_security_lt->insertWidget(lang_security_lt->count() - 1 , opts_ret_met_wgt);

	QLayout *attribs_lt = attributes_tab->layout();
	attribs_lt->addWidget(settings_grp);
	attribs_lt->addWidget(ret_type_parent);
	attribs_lt->addWidget(ret_table_grp);

	configureTabbedLayout(func_config_twg);
	setRequiredField(ret_method_lbl);

	ret_method_cmb->setMinimumWidth(ret_method_lbl->width());

	configureTabOrder();
	setMinimumSize(650, 500);
}

void FunctionWidget::handleParameter(Parameter param, int result)
{
	bool is_param = parameters_tab->isVisible();
	BaseFunctionWidget::handleParameter(is_param ? parameters_tab : return_tab,
																			param, result, is_param);
}

void FunctionWidget::duplicateParameter(int curr_row, int new_row)
{
	bool is_param = parameters_tab->isVisible();
	BaseFunctionWidget::duplicateParameter(is_param ? parameters_tab : return_tab,
																				 curr_row, new_row, is_param);
}

void FunctionWidget::showParameterForm()
{
	QObject *obj_sender=sender();
	CustomTableWidget *table=nullptr;

	if(obj_sender == parameters_tab || obj_sender == return_tab)
	{
		table = dynamic_cast<CustomTableWidget *>(obj_sender);
		BaseFunctionWidget::showParameterForm(table, obj_sender == parameters_tab);
	}
}

void FunctionWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Function *func)
{
	PgSqlType aux_type;
	unsigned count = 0, i = 0;
	Parameter param;

	BaseFunctionWidget::setAttributes(model, op_list, schema, func);

	if(func)
	{
		aux_type = func->getReturnType();

		func_type_cmb->setCurrentIndex(func_type_cmb->findText(~func->getFunctionType()));
		window_func_chk->setChecked(func->isWindowFunction());
		leakproof_chk->setChecked(func->isLeakProof());
		exec_cost_spb->setValue(func->getExecutionCost());
		rows_ret_spb->setValue(func->getRowAmount());
		behavior_cmb->setCurrentIndex(behavior_cmb->findText(~func->getBehaviorType()));
		parallel_cmb->setCurrentIndex(parallel_cmb->findText(~func->getParallelType()));

		if(func->isReturnSetOf())
			ret_method_cmb->setCurrentIndex(SetOfReturn);
		else if(func->isReturnTable())
			ret_method_cmb->setCurrentIndex(TableReturn);
		else
			ret_method_cmb->setCurrentIndex(SimpleReturn);

		count = func->getReturnedTableColumnCount();
		return_tab->blockSignals(true);

		if(count > 0)
		{
			ret_table_grp->setVisible(true);
			ret_type_parent->setVisible(false);

			for(i = 0; i < count; i++)
			{
				return_tab->addRow();
				param = func->getReturnedTableColumn(i);
				BaseFunctionWidget::showParameterData(return_tab, param, i, false);
			}
		}

		return_tab->clearSelection();
		return_tab->blockSignals(false);

		if(!func->getLibrary().isEmpty())
		{
			symbol_edt->setText(func->getSymbol());
			library_edt->setText(func->getLibrary());
		}
		else
		{
			source_code_txt->setPlainText(func->getFunctionSource());
		}
	}

	ret_type->setAttributes(aux_type, model, false);
}

void FunctionWidget::alternateReturnTypes()
{
	bool is_ret_table = (ret_method_cmb->currentIndex() == TableReturn);
	ret_table_grp->setVisible(is_ret_table);
	ret_type_parent->setVisible(!is_ret_table);
}

void FunctionWidget::validateConfiguredFunction()
{
	std::vector<BaseObject *> obj_list;
	Conversion *conv = nullptr;
	Cast *cast = nullptr;
	Aggregate *aggr = nullptr;
	Language *lang = nullptr;
	Operator *oper = nullptr;
	Type *type = nullptr;
	Function *func = nullptr;
	Transform *transf = nullptr;
	ForeignDataWrapper *fdw = nullptr;
	ObjectType obj_type;
	unsigned idx = 0;

	func = dynamic_cast<Function *>(this->object);

	try
	{
		for(auto &object : func->getReferences())
		{
			obj_type = object->getObjectType();

			/* The validation of the function happens as follows:

				For each type of object in vector 'types' is obtained the list of objects.
				If there are elements in this list, the function is assigned for each element
				and these elements internally validates the function according to required by the
				each class rules.

				If the function is invalid the instances raises	exceptions accusing the error
				that is enough to check	the validity of the function in relation to objects that reference it. */
			if(obj_type == ObjectType::Conversion)
			{
				conv = dynamic_cast<Conversion *>(object);
				if(conv->getConversionFunction() == func)
					conv->setConversionFunction(func);
			}
			else if(obj_type == ObjectType::Cast)
			{
				cast = dynamic_cast<Cast *>(object);
				if(cast->getCastFunction() == func)
					cast->setCastFunction(func);
			}
			else if(obj_type == ObjectType::Aggregate)
			{
				aggr = dynamic_cast<Aggregate *>(object);
				if(aggr->getFunction(Aggregate::FinalFunc) == func)
					aggr->setFunction(Aggregate::FinalFunc, func);
				else if(aggr->getFunction(Aggregate::TransitionFunc) == func)
					aggr->setFunction(Aggregate::TransitionFunc, func);
			}
			else if(obj_type == ObjectType::Trigger)
			{
				dynamic_cast<Trigger *>(object)->setFunction(func);
			}
			else if(obj_type == ObjectType::Language)
			{
				lang = dynamic_cast<Language *>(object);

				for(idx = Language::ValidatorFunc; idx <= Language::InlineFunc; idx++)
				{
					if(lang->getFunction(static_cast<Language::FunctionId>(idx))==func)
						lang->setFunction(func, static_cast<Language::FunctionId>(idx));
				}
			}
			else if(obj_type == ObjectType::Operator)
			{
				oper = dynamic_cast<Operator *>(object);
				for(idx = Operator::FuncOperator; idx <= Operator::FuncRestrict; idx++)
				{
					if(oper->getFunction(static_cast<Operator::FunctionId>(idx)) == func)
						oper->setFunction(func, static_cast<Operator::FunctionId>(idx));
				}
			}
			else if(obj_type == ObjectType::Type)
			{
				type = dynamic_cast<Type *>(object);
				if(type->getConfiguration() == Type::BaseType)
				{
					for(idx = Type::InputFunc; idx <= Type::AnalyzeFunc; idx++)
					{
						if(type->getFunction(static_cast<Type::FunctionId>(idx))==func)
							type->setFunction(static_cast<Type::FunctionId>(idx), func);
					}
				}
			}
			else if(obj_type == ObjectType::EventTrigger)
			{
				dynamic_cast<EventTrigger *>(object)->setFunction(func);
			}
			else if(obj_type == ObjectType::Transform)
			{
				transf = dynamic_cast<Transform *>(object);

				for(auto func_id : { Transform::FromSqlFunc, Transform::ToSqlFunc })
				{
					if(func == transf->getFunction(func_id))
						transf->setFunction(func, func_id);
				}
			}
			else if(obj_type == 	ObjectType::ForeignDataWrapper)
			{
				fdw = dynamic_cast<ForeignDataWrapper *>(object);

				if(fdw->getHandlerFunction() == func)
					fdw->setHandlerFunction(func);
				else if(fdw->getValidatorFunction() == func)
					fdw->setValidatorFunction(func);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvFuncConfigInvalidatesObject)
										.arg(object->getName(true))
										.arg(object->getTypeName()),
										ErrorCode::InvFuncConfigInvalidatesObject,
										PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void FunctionWidget::applyConfiguration()
{
	try
	{
		Function *func = nullptr;

		startConfiguration<Function>();
		func = dynamic_cast<Function *>(this->object);
		applyBasicConfiguration(func);

		func->setFunctionType(func_type_cmb->currentText());
		func->setWindowFunction(window_func_chk->isChecked());
		func->setLeakProof(leakproof_chk->isChecked());
		func->setExecutionCost(exec_cost_spb->value());
		func->setRowAmount(rows_ret_spb->value());
		func->setBehaviorType(behavior_cmb->currentText());
		func->setParalleType(parallel_cmb->currentText());

		int curr_ret_met = ret_method_cmb->currentIndex();

		if(curr_ret_met == SimpleReturn || curr_ret_met == SetOfReturn)
		{
			func->setReturnType(ret_type->getPgSQLType());
			func->setReturnSetOf(curr_ret_met == SetOfReturn);
		}
		else
		{
			unsigned count = 0, i = 0;

			func->removeReturnedTableColumns();
			count = return_tab->getRowCount();

			for(i = 0; i < count; i++)
			{
				func->addReturnedTableColumn(return_tab->getCellText(i, 0),
																		 return_tab->getRowData(i).value<PgSqlType>());
			}
		}

		BaseFunctionWidget::applyConfiguration();
		validateConfiguredFunction();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

