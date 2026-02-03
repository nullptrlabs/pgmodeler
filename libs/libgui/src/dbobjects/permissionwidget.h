/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class PermissionWidget
\brief Implements the operations to create/edit permissions via form.
*/

#ifndef PERMISSION_WIDGET_H
#define PERMISSION_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_permissionwidget.h"
#include "widgets/customtablewidget.h"
#include "widgets/modelobjectswidget.h"

class __libgui PermissionWidget: public BaseObjectWidget, public Ui::PermissionWidget {
	Q_OBJECT

	private:
		//! \brief Stores the permission to be edited
		Permission *permission;

		//! \brief Table widget that stores the roles that has permission over the object
		CustomTableWidget *roles_tab,

		//! \brief Table widget that stores the permissions related to the object
		*permissions_tab;

		//! \brief Widget used to select roles on the database model
		ModelObjectsWidget *object_selection_wgt;

		/*! \brief Indicates if some permission was changed. This flag is used to know when
		emit the signal s_objectManipulated() */
		bool perms_changed;

	public:
		PermissionWidget(QWidget * parent = nullptr);

		~PermissionWidget() override;

		void setAttributes(DatabaseModel *model, BaseObject *parent_obj, BaseObject *object);

	public slots:
		void applyConfiguration() override;

	protected:
		QString getSQLCodePreview() override;

	private slots:
		//! \brief Shows the object selection window
		void selectRole();

		//! \brief Select a permission which the index is specified on parameter
		void selectPermission(int perm_id);

		//! \brief Adds the configured permission onto the model
		void addPermission();

		//! \brief Configures a permission based upon the values assinged on the form
		void configurePermission(Permission *perm);

		//! \brief Fills the form with the selected permission attributes
		void editPermission();

		//! \brief Removes the selected permission
		void removePermission(int);

		//! \brief Removes all permissions from the permissions table
		void removePermissions();

		//! \brief Updates the attributes of the currently edited permission
		void updatePermission();

		//! \brief Lists all permissions related to the object
		void listPermissions();

		//! \brief Cancel de permission's creation/edition cleaning the form and blocking the edition buttons
		void cancelOperation();

		//! \brief Shows the selected role data on the role tables
		void showSelectedRoleData();

		void enableEditButtons();

		void checkPrivilege();

		void disableGrantOptions();
};

#endif
