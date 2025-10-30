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
\class TableDataWidget
\brief Form used to add initial data for tables.
*/

#ifndef TABLE_DATA_WIDGET_H
#define TABLE_DATA_WIDGET_H

#include "dbobjects/baseobjectwidget.h"
#include "ui_tabledatawidget.h"
#include "csvloadwidget.h"

class __libgui TableDataWidget: public BaseObjectWidget, public Ui::TableDataWidget {
	Q_OBJECT

	private:
		CsvLoadWidget *csv_load_wgt;

		/*! \brief Stores the remaining column names not used in the grid.
		This menu is used either to add new columns and fix invalid columns in the grid */
		QMenu col_names_menu;

		//! brief Loads the grid with the initial data of the curret table object
		void populateDataGrid(const CsvDocument &csv_doc = CsvDocument());

		//! brief Configures the col_name_menu with the not used columns names
		void configureColumnNamesMenu();

		//! brief Toggles the warning frame if some invalid or duplicated columns is detected
		void toggleWarningFrame();

		//! brief Marks a certain item as invalid cause it to be deactivated in the grid
		void setItemInvalid(QTableWidgetItem *item);

		//! brief Generated the CSV-like buffer to be used as initial data in the table object
		QString generateDataBuffer();

		void showEvent(QShowEvent *) override;

		void enterEvent(QEnterEvent *) override;

	public:
		static const QString PlaceholderColumn;

		TableDataWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, PhysicalTable *table);

	public slots:
		void applyConfiguration() override;

	private slots:
		void insertRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col);
		void addRow();
		void addColumn(QAction *action);
		void duplicateRows();
		void deleteRows();
		void deleteColumns();
		void clearRows(bool confirm=true);
		void clearColumns();
		void changeColumnName(int col_idx);
		void enableButtons();
		void showItemContextMenu(const QPoint &pos);
};

#endif
