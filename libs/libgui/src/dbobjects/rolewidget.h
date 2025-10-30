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
\class RoleWidget
\brief Implements the operations to create/edit roles via form.
*/

#ifndef ROLE_WIDGET_H
#define ROLE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_rolewidget.h"
#include "widgets/customtablewidget.h"

class __libgui RoleWidget: public BaseObjectWidget, public Ui::RoleWidget {
	Q_OBJECT

	private:
		//! \brief Widget used to select roles on the database model
		ModelObjectsWidget *object_selection_wgt;

		//! \brief Store the table widgets used to reference the member roles
		CustomTableWidget *members_tab[3];

		//! \brief Fills the tables with to member roles of the editing role
		void fillMembersTable();

		//! \brief Show the specified role data on the specified table index at the specified row
		void showRoleData(Role *role, unsigned table_id, unsigned row);

	public:
		RoleWidget(QWidget * parent = nullptr);
		~RoleWidget() override;

		void setAttributes(DatabaseModel *model, OperationList *op_list, Role *role);

	private slots:
		void showSelectedRoleData();
		void selectMemberRole();

		//! \brief Configures the signals/slots to each role table related to object selection
		void configureRoleSelection();

	public slots:
		void applyConfiguration() override;
};

#endif
