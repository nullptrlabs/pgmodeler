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
\class ColumnPickerWidget
\brief This widget allows picking up columns of a table from a combobox and placing the selected ones in a grid.
*/

#ifndef COLUMN_PICKER_WIDGET_H
#define COLUMN_PICKER_WIDGET_H

#include "ui_columnpickerwidget.h"
#include "customtablewidget.h"
#include "simplecolumn.h"
#include "column.h"

class __libgui ColumnPickerWidget: public QWidget, public Ui::ColumnPickerWidget {
	Q_OBJECT

	private:
		//! \brief Stores the object in which the columns to be listed will be retrieved (must be a table or relationship)
		BaseObject *parent_obj;

		CustomTableWidget *columns_tab;

		//! \brief Adds the column to the specified row in the columns grid
		void addColumn(Column *column, int row);

		//! \brief Adds the view column to the specified row in the columns grid
		void addColumn(const SimpleColumn &column, int row);

	public:
		explicit ColumnPickerWidget(QWidget *parent = nullptr);

		/*! \brief Specifies the parent table or relationship in which the columns are retrived
		 * If a null object is specified the widget will be disabled, if an non-null object is assigned
		 * but is not a table or relationship the form will also be disabled. */
		void setParentObject(BaseObject *p_obj);

		/*! \brief Fills the columns grid with the elements in the vector.
		 * Columns that aren't child of the parent object specified by setParentObject()
		 * will be ignored */
		void setColumns(const std::vector<Column *> &cols);

		//! \brief Fills the columns grid with the elements view's columns
		void setColumns(const std::vector<SimpleColumn> &cols);

		//! \brief Returns a list of the columns that are in the grid.
		std::vector<Column *> getColumns();

		//! \brief Returns a list of the view columns that are in the grid.
		std::vector<SimpleColumn> getSimpleColumns();

		//! \brief Removes all columns from the grid
		void clear();

	private slots:
		void updateColumnsCombo();
		void addColumn(int row);
};

#endif
