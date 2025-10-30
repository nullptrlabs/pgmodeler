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
\class ViewWidget
\brief Implements the operations to create/edit views via form.
*/

#ifndef VIEW_WIDGET_H
#define VIEW_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_viewwidget.h"
#include "widgets/customtablewidget.h"
#include "syntaxhighlighter.h"
#include "widgets/numberedtexteditor.h"
#include "dbobjects/referenceswidget.h"
#include "dbobjects/simplecolumnswidget.h"

class __libgui ViewWidget: public BaseObjectWidget, public Ui::ViewWidget {
	Q_OBJECT

	private:
		SimpleColumnsWidget *custom_cols_wgt;

		ReferencesWidget *obj_refs_wgt;

		ObjectSelectorWidget *tag_sel;

		NumberedTextEditor *sql_preview_txt, *sql_definition_txt;

		std::map<ObjectType, CustomTableWidget *> objects_tab_map;

		SyntaxHighlighter *sql_preview_hl, *sql_definition_hl;

		//! \brief Returns the object table according with the child type
		CustomTableWidget *getObjectTable(ObjectType obj_type);

		ObjectType getObjectType(QObject *sender);

		void showObjectData(TableObject *object, int row);

		void listObjects(ObjectType obj_type);

		/*! \brief Template method that opens the editing form for the specified object.
				Class and ClassWidget should be compatible, e.g., "Column" can only be edited using ColumnWidget */
		template<class Class, class ClassWidget>
		int openEditingForm(TableObject *object);

	public:
		ViewWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, View *view, double px, double py);

	private slots:
		//! \brief Updates the sql code field of the view form
		void updateCodePreview();

		//! \brief Adds or edit a object on the object table that calls the slot
		void handleObject();

		//! \brief Duplicates a object on the object table that calls the slot
		void duplicateObject(int curr_row, int new_row);

		//! \brief Removes the selected object from the table that calls the slot
		void removeObject(int row);

		//! \brief Removes all objects from the table that calls the slot
		void removeObjects();

	public slots:
		void applyConfiguration() override;
		void cancelConfiguration() override;
};

#endif
