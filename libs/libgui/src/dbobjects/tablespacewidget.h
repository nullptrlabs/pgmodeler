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

/**
\ingroup libgui
\class TablespaceWidget
\brief Implements the operations to create/edit tablespaces via form.
*/

#ifndef TABLESPACE_WIDGET_H
#define TABLESPACE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_tablespacewidget.h"

class __libgui TablespaceWidget: public BaseObjectWidget, public Ui::TablespaceWidget {
	Q_OBJECT

	public:
		TablespaceWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Tablespace *tablespc);

	public slots:
		void applyConfiguration() override;
};

#endif
