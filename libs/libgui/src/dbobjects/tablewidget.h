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
\class TableWidget
\brief  Implements the operations to create/edit tables via form.
*/

#ifndef TABLE_WIDGET_H
#define TABLE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_tablewidget.h"
#include "widgets/customtablewidget.h"
#include "elementstablewidget.h"

class __libgui TableWidget: public BaseObjectWidget, public Ui::TableWidget {
	Q_OBJECT

	private:
		CustomTableWidget *parent_tables, *options_tab;

		ElementsTableWidget *partition_keys_tab;

		ObjectSelectorWidget *tag_sel, *server_sel;

		QFrame *warn_frame;

		//! \brief Stores the objects tables used to handle columns, constraints, indexes, rules and triggers
		std::map<ObjectType, CustomTableWidget *> objects_tab_map;

		//! \brief Lists (on the correct object table) the table objects according to the specified type
		void listObjects(ObjectType obj_type);

		//! \brief Shows the specified object data at the correct object table, at specified row
		void showObjectData(TableObject *object, int row);

		//! \brief Returns the object table according with the child type
		CustomTableWidget *getObjectTable(ObjectType obj_type);

		//! \brief Returns the object type according to the widget (in this case a object table) that called the method
		ObjectType getObjectType(QObject *sender);

		/*! \brief Template method that opens the editing form for the specified object.
				Class and ClassWidget should be compatible, e.g., "Column" can only be edited using ColumnWidget */
		template<class Class, class ClassWidget>
		int openEditingForm(TableObject *object);

		void __setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, PhysicalTable *table, double pos_x, double pos_y);

		void updatePkColumnsCheckState(bool has_pk);

	public:
		TableWidget(QWidget * parent = nullptr, ObjectType tab_type = ObjectType::Table);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Table *table, double pos_x, double pos_y);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, ForeignTable *ftable, double pos_x, double pos_y);

		static void setTableItemColor(unsigned color_idx, const QColor color);
		static QColor getTableItemColor(unsigned color_idx);

	private slots:
		//! \brief Adds or edit a object on the object table that calls the slot
		void handleObject();

		//! \brief Removes the selected object from the table that calls the slot
		void removeObject(int row);

		//! \brief Duplicate the selected object. This method will desambigate names if needed
		void duplicateObject(int sel_row, int new_row);

		//! \brief Removes all objects from the table that calls the slot
		void removeObjects();

		//! \brief Swap the index between two rows of the table that calls the slot
		void swapObjects(int idx1, int idx2);

		void editData();

	public slots:
		void applyConfiguration() override;
		void cancelConfiguration() override;
};

#endif
