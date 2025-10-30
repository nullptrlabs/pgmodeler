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

#include "foreigndatawrapperwidget.h"
#include "guiutilsns.h"

ForeignDataWrapperWidget::ForeignDataWrapperWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::ForeignDataWrapper)
{
	QHBoxLayout *hbox = nullptr;

	Ui_ForeignDataWrapperWidget::setupUi(this);

	func_handler_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	func_validator_sel=new ObjectSelectorWidget(ObjectType::Function, this);

	func_handler_sel->setToolTip(tr("The handler function must have the following signature:  <strong>fdw_handler</strong> <em>function_name</em>()"));
	func_validator_sel->setToolTip(tr("The validator function must have the following signature: <em>function_name</em>(<strong>text[]</strong>,<strong>oid</strong>). The return type of ths function is ignored."));

	hbox = new QHBoxLayout;
	hbox->setContentsMargins(0,0,0,0);
	hbox->addWidget(func_handler_sel);
	func_handler_wgt->setLayout(hbox);

	hbox = new QHBoxLayout;
	hbox->setContentsMargins(0,0,0,0);
	hbox->addWidget(func_validator_sel);
	func_validator_wgt->setLayout(hbox);

	options_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																			 (CustomTableWidget::EditButton | CustomTableWidget::UpdateButton), true, this);
	options_tab->setCellsEditable(true);
	options_tab->setColumnCount(2);
	options_tab->setHeaderLabel(tr("Option"), 0);
	options_tab->setHeaderLabel(tr("Value"), 1);

	hbox = new QHBoxLayout;
	hbox->setContentsMargins(GuiUtilsNs::LtMargins);
	hbox->addWidget(options_tab);
	options_gb->setLayout(hbox);

	configureFormLayout(fdw_grid, ObjectType::ForeignDataWrapper);
	configureTabOrder({ func_handler_sel, func_validator_sel, options_tab });

	setMinimumSize(600, 420);
}

void ForeignDataWrapperWidget::setAttributes(DatabaseModel *model, OperationList *op_list, ForeignDataWrapper *fdw)
{
	BaseObjectWidget::setAttributes(model, op_list, fdw);

	func_handler_sel->setModel(model);
	func_validator_sel->setModel(model);

	if(fdw)
	{
		func_handler_sel->setSelectedObject(fdw->getHandlerFunction());
		func_validator_sel->setSelectedObject(fdw->getValidatorFunction());

		options_tab->blockSignals(true);

		for(auto &itr : fdw->getOptions())
		{
			options_tab->addRow();
			options_tab->setCellText(itr.first, options_tab->getRowCount() - 1, 0);
			options_tab->setCellText(itr.second, options_tab->getRowCount() - 1, 1);
		}

		options_tab->clearSelection();
		options_tab->blockSignals(false);
	}
}

void ForeignDataWrapperWidget::applyConfiguration()
{
	try
	{
		ForeignDataWrapper *fdw=nullptr;

		startConfiguration<ForeignDataWrapper>();

		fdw=dynamic_cast<ForeignDataWrapper *>(this->object);
		fdw->setHandlerFunction(dynamic_cast<Function *>(func_handler_sel->getSelectedObject()));
		fdw->setValidatorFunction(dynamic_cast<Function *>(func_validator_sel->getSelectedObject()));

		fdw->removeOptions();
		for(unsigned row = 0; row < options_tab->getRowCount(); row++)
			fdw->setOption(options_tab->getCellText(row, 0), options_tab->getCellText(row, 1));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
