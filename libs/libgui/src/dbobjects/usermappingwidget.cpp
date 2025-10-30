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

#include "usermappingwidget.h"
#include "guiutilsns.h"

UserMappingWidget::UserMappingWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::UserMapping)
{
	QHBoxLayout *hbox = nullptr;

	Ui_UserMappingWidget::setupUi(this);

	server_sel=nullptr;
	server_sel=new ObjectSelectorWidget(ObjectType::ForeignServer, this);

	hbox = new QHBoxLayout;
	hbox->setContentsMargins(0,0,0,0);
	hbox->addWidget(server_sel);
	server_wgt->setLayout(hbox);

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

	configureFormLayout(server_grid, ObjectType::UserMapping);

	setRequiredField(server_sel);
	setRequiredField(server_lbl);
	configureTabOrder({ server_sel, options_tab });

	setMinimumSize(550, 280);
}

void UserMappingWidget::setAttributes(DatabaseModel *model, OperationList *op_list, UserMapping *user_map)
{
	BaseObjectWidget::setAttributes(model, op_list, user_map);
	server_sel->setModel(model);

	if(user_map)
	{
		server_sel->setSelectedObject(user_map->getForeignServer());
		options_tab->blockSignals(true);

		for(auto &itr : user_map->getOptions())
		{
			options_tab->addRow();
			options_tab->setCellText(itr.first, options_tab->getRowCount() - 1, 0);
			options_tab->setCellText(itr.second, options_tab->getRowCount() - 1, 1);
		}

		options_tab->clearSelection();
		options_tab->blockSignals(false);
	}
}

void UserMappingWidget::applyConfiguration()
{
	try
	{
		UserMapping *user_map = nullptr;

		startConfiguration<UserMapping>();

		user_map = dynamic_cast<UserMapping *>(this->object);
		user_map->setForeignServer(dynamic_cast<ForeignServer *>(server_sel->getSelectedObject()));

		user_map->removeOptions();
		for(unsigned row = 0; row < options_tab->getRowCount(); row++)
			user_map->setOption(options_tab->getCellText(row, 0), options_tab->getCellText(row, 1));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

