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

#include "foreignserverwidget.h"
#include "guiutilsns.h"

ForeignServerWidget::ForeignServerWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::ForeignServer)
{
	QHBoxLayout *hbox = nullptr;

	Ui_ForeignServerWidget::setupUi(this);

	fdw_sel=nullptr;
	fdw_sel=new ObjectSelectorWidget(ObjectType::ForeignDataWrapper, this);

	hbox = new QHBoxLayout;
	hbox->setContentsMargins(0,0,0,0);
	hbox->addWidget(fdw_sel);
	fdw_wgt->setLayout(hbox);

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

	configureFormLayout(server_grid, ObjectType::ForeignServer);

	setRequiredField(fdw_sel);
	setRequiredField(fdw_lbl);
	configureTabOrder({ type_edt, version_edt, fdw_sel, options_tab });

	setMinimumSize(600, 420);
}

void ForeignServerWidget::setAttributes(DatabaseModel *model, OperationList *op_list, ForeignServer *server)
{
	BaseObjectWidget::setAttributes(model, op_list, server);

	fdw_sel->setModel(model);

	if(server)
	{
		version_edt->setText(server->getVersion());
		type_edt->setText(server->getType());
		fdw_sel->setSelectedObject(server->getForeignDataWrapper());
		options_tab->blockSignals(true);

		for(auto &itr : server->getOptions())
		{
			options_tab->addRow();
			options_tab->setCellText(itr.first, options_tab->getRowCount() - 1, 0);
			options_tab->setCellText(itr.second, options_tab->getRowCount() - 1, 1);
		}

		options_tab->clearSelection();
		options_tab->blockSignals(false);
	}
}

void ForeignServerWidget::applyConfiguration()
{
	try
	{
		ForeignServer *server = nullptr;

		startConfiguration<ForeignServer>();

		server = dynamic_cast<ForeignServer *>(this->object);
		server->setForeignDataWrapper(dynamic_cast<ForeignDataWrapper *>(fdw_sel->getSelectedObject()));

		server->removeOptions();
		for(unsigned row = 0; row < options_tab->getRowCount(); row++)
			server->setOption(options_tab->getCellText(row, 0), options_tab->getCellText(row, 1));

		server->setVersion(version_edt->text());
		server->setType(type_edt->text());

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

