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

#include "tablespacewidget.h"

TablespaceWidget::TablespaceWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Tablespace)
{
	Ui_TablespaceWidget::setupUi(this);

	directory_sel = new FileSelectorWidget(this);
	directory_sel->setDirectoryMode(true);
	directory_sel->setAcceptMode(QFileDialog::AcceptOpen);
	directory_lt->addWidget(directory_lbl);
	directory_lt->addWidget(directory_sel);

	configureTabbedLayout(false);
	setRequiredField(directory_lbl);
	setRequiredField(directory_sel);
	configureTabOrder();

	setMinimumSize(550, 300);
}

void TablespaceWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Tablespace *tablespc)
{
	BaseObjectWidget::setAttributes(model, op_list, tablespc);

	if(tablespc)
		directory_sel->setSelectedFile(tablespc->getDirectory());
}

void TablespaceWidget::applyConfiguration()
{
	try
	{
		Tablespace *tablespc = nullptr;

		startConfiguration<Tablespace>();
		tablespc = dynamic_cast<Tablespace *>(this->object);
		tablespc->setDirectory(directory_sel->getSelectedFile());

		BaseObjectWidget::applyConfiguration();

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

