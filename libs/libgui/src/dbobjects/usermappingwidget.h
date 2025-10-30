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
\class UserMappingWidget
\brief Implements the operations to create/edit user mappings via form.
*/

#ifndef USER_MAPPING_WIDGET_H
#define USER_MAPPING_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_usermappingwidget.h"
#include "widgets/customtablewidget.h"

class __libgui UserMappingWidget: public BaseObjectWidget, public Ui::UserMappingWidget {
	Q_OBJECT

	private:
		ObjectSelectorWidget *server_sel;

		CustomTableWidget *options_tab;

	public:
		UserMappingWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, UserMapping *user_map);

	public slots:
		void applyConfiguration() override;
};

#endif
