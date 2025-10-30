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
\class TypeWidget
\brief Implements the operations to create/edit user defined data types via form.
*/

#ifndef TYPE_WIDGET_H
#define TYPE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_typewidget.h"
#include "pgsqltypewidget.h"
#include "widgets/customtablewidget.h"

class __libgui TypeWidget: public BaseObjectWidget, public Ui::TypeWidget {
	Q_OBJECT

	private:
		//! \brief Type functions selectors widgets
		ObjectSelectorWidget *functions_sel[9],

		//! \brief Range type operator class selector widget
		*opclass_sel;

		//! \brief Type configuration widgets, used to handle "like type", "element type" and "range subtype"
		PgSQLTypeWidget *like_type,
		*element_type,
		*range_subtype;

		//! \brief Tables that store enumaration elementas and composite attributes
		CustomTableWidget *enumerations_tab,
		*attributes_tab;

		//! \brief Composite type attribute collation selector
		ObjectSelectorWidget *attrib_collation_sel;

		//! \brief Composite type attribute datatype configurator
		PgSQLTypeWidget *attrib_type_wgt;

	public:
		TypeWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Type *type);

	private slots:
		//! \brief Handles the form showing the fields according to the selected type configuration (base, composite, enumaration)
		void selectTypeConfiguration();

		//! \brief Handles the type enumarations
		void handleEnumeration(int row);

		//! \brief Create/Updated an attribute based upon values of the form
		void handleAttribute(int row);

		//! \brief Exposes on form the values of the currently selected attribute
		void editAttribute(int row);

		//! \brief Shows the attribute configured onto the attributes table
		void showAttributeData(TypeAttribute attrib, int row);

	public slots:
		void applyConfiguration() override;
};

#endif
