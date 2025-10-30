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
\class DataGridWidget
\brief Implements the operations to handle data on of a single table in a grid
*/

#ifndef DATA_GRID_WIDGET_H
#define DATA_GRID_WIDGET_H

#include <QMenu>
#include "ui_datagridwidget.h"
#include "catalog.h"
#include "utils/syntaxhighlighter.h"
#include "widgets/codecompletionwidget.h"
#include "widgets/csvloadwidget.h"

class __libgui DataGridWidget: public QWidget, public Ui::DataGridWidget {
	Q_OBJECT

	private:	
		//! \brief Constants used to mark the type of operation performed on rows
		enum OperationId: unsigned {
			NoOperation,
			OpInsert,
			OpUpdate,
			OpDelete
		};

		//! \brief A CSV loader widget that loads data from CSV to the data grid
		CsvLoadWidget *csv_load_wgt;

		//! \brief A syntax highlighter installed on filter input field
		SyntaxHighlighter *filter_hl;
		
		//! \brief A code completion widget that displays column names in the filter input field
		CodeCompletionWidget *code_compl_wgt;

		//! \brief The name of the schema of the currently browsed table
		QString sch_name,

		//! \brief The name of the currently browsed table
		tab_name;

		//! \brief The object type of the table (table, foreign table, view)
		ObjectType obj_type;

		//! \brief The connection instance that is used to run SQL commands
		Connection _conn_sql;

		//! \brief The catalog instance that is used to retrive objects information from system catalogs
		Catalog _catalog;

		QAction *action_add, *action_delete, *action_bulk_edit,
		*action_duplicate, *action_clear;

		QMenu items_menu, fks_menu, copy_menu, truncate_menu,
		paste_menu, edit_menu, export_menu, save_menu;

		bool save_enabled, undo_enabled, browse_enabled,
		selection_enabled, edit_enabled,
		export_enabled, truncate_enabled, filter_enabled;

		//! \brief Store the connection params to be used by catalogs and command execution connections
		attribs_map conn_params;
		
		//! \brief Current editing table columns names
		QStringList col_names,
		
		//! \brief Current editing table pk columns names
		pk_col_names;

		/*! \brief Stores the current opened table's oid. This attribute is filled only the table has an primary
		 * and it is used to retrieve all foreign keys that references the current table */
		unsigned table_oid;
		
		//! \brief Stores the ids of changed rows. These ids are handled on saveChanges() method
		std::vector<int> changed_rows;
		
		//! \brief Stores the previous background color of the rows before being marked with some operation
		std::map<int, QBrush> prev_bg_colors,

		//! \brief Stores the previous foreground color of the rows before being marked with some operation
		prev_fg_colors;

		//! \brief Stores the fk informations about referenced tables
		std::map<QString, attribs_map> fk_infos,

		//! \brief Stores the fk informations about referencing tables
		ref_fk_infos;
		
		//! \brief Retrieve the primary key column ids for the specified table
		void retrievePKColumns(Catalog &catalog);

		/*! \brief Retrieve the foreign key columns info for the specified table. These data is used to browse referenced tables in the data
		 that the selected line holds */
		void retrieveFKColumns(Catalog &catalog);
		
		/*! \brief Mark the line as changed, changing its background color and applying the respective operation (see OP_??? constant)
				when user call saveChanged() */
		void markOperationOnRow(OperationId operation, int row);
		
		//! \brief Generates a DML command for the row depending on the it's operation type
		QString getDMLCommand(int row);
		
		//! \brief Remove the rows marked as OP_INSERT which ids are specified on the parameter vector
		void removeNewRows(std::vector<int> ins_rows);
		
		//! \brief Reset the state of changed rows, clearing all attributes used to control the modifications on them
		void clearChangedRows();

		//! \brief Browse a referenced or referencing table by the provided foreign key name
		void browseTable(const QString &fk_name, bool browse_ref_tab);

		//! \brief Set the check state of the columns in the "Columns" tab in the filter widget
		void setColumnsCheckState(Qt::CheckState state);

		/*! \brief This event filter toggles the visibility of the columns in the data grid
		 *  when clicking the columns items in the "Columns"tab in the filter widget */
		bool eventFilter(QObject *object, QEvent *event) override;

		//! \brief Updates the information about operations pending over the rows
		void updateRowOperationsInfo();

	public:
		DataGridWidget(const QString &sch_name, const QString &tab_name,
									 ObjectType obj_type, const attribs_map &conn_params,
									 QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);
		
		//! \brief Returns whether the filter widget is toggled
		bool isFilterToggled();

		//! \brief Returns whether the CSV loader widget is toggled
		bool isCsvLoaderToggled();

		//! \brief Returns whether a save operation is enabled for the data grid
		bool isSaveEnabled();

		//! \brief Returns whether a undo operation is enabled for the data grid
		bool isUndoEnabled();

		//! \brief Returns whether a table browse operation is enabled for the data grid
		bool isBrowseEnabled();

		//! \brief Returns whether a selection is enabled/available for the data grid
		bool isSelectionEnabled();

		//! \brief Returns whether a export operation is enabled/available for the data grid
		bool isExportEnabled();

		//! \brief Returns whether a truncate operation is enabled for the data grid
		bool isTruncateEnabled();

		//! \brief Returns whether the filter widget is enabled
		bool isFilterEnabled();

		//! \brief Returns whether one or more edit operations are enabled for the data grid
		bool isEditEnabled();

		//! \brief Returns whether a paste operation is enabled for the data grid
		bool isPasteEnabled();

		//! \brief Returns whether the data grid has modified rows waiting a commit
		bool hasChangedRows();

	private slots:
		//! \brief Reset the entire UI of the data grid
		void resetDataGrid();

		//! \brief Clears the text of the selected cells
		void clearItemsText();

		//! \brief Sorts the results when the user clicks a section in the vertical header by holding Control key
		void sortResults(int column, Qt::SortOrder order);

		//! \brief Selected a entire section in the results when the user clicks a section in the vertical header without holding Control key
		void selectColumn(int column, Qt::SortOrder order);

		//! \brief List the columns based upon the current table
		void listColumns(const std::vector<attribs_map> &cols);
		
		//! \brief Retrieve the data for the current table filtering the data as configured on the advanced tab
		void retrieveData();
		
		//! \brief Enables the delete/duplicate/copy buttons depending on the selected rows
		void enableRowControlButtons();
		
		//! \brief Enables/disables the buttons of the order by list depending on the state of it
		void enableColumnControlButtons();
		
		//! \brief Add a column to the "order by" list
		void addSortColumnToList();
		
		//! \brief Remove a column from the "order by" list
		void removeSortColumnFromList();
		
		//! \brief Clears the "order by" list
		void clearSortColumnList();
		
		//! \brief Toggles the sort mode between ASC and DESC when right clicking on a element at order by list
		void changeOrderMode(QListWidgetItem *item);
		
		//! \brief Mark the entire row in which the item resides
		void markUpdateOnRow(QTableWidgetItem *item);
		
		//! \brief Mark a seleciton of rows to be delete. New rows are automatically removed
		void markDeleteOnRows();
		
		//! \brief Add a new row on the grid with the first column with edition enabled
		void addRow(bool focus_new_row = true);
		
		//! \brief Duplicate the selected rows creating new ones with the same values as the selection
		void duplicateRows();

		//! \brief Undo the operation made on all rows or in a set of selected rows
		void undoOperations();
		
		//! \brief Insert a new row as the user press tab key on the last column at last row
		void insertRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col);
		
		//! \brief Commit all changes made on the rows rolling back changes when some error is triggered
		void saveChanges();
		
		//! \brief Swap two rows on the order by list
		void swapColumns();

		//! \brief Add new rows to the grid based upon the CSV loaded
		void loadDataFromCsv(bool load_from_clipboard = false, bool force_csv_parsing = false);

		//! \brief Browse the referenced table data using the selected row in the results grid
		void browseReferencedTable();

		//! \brief Browse the referencing table data using the selected row in the results grid
		void browseReferrerTable();

		//! \brief Truncates the browsed table
		void truncateTable();

		//! \brief Display or hides a column when the related item is interacted in the column list at filter section
		void toggleColumnDisplay(QListWidgetItem *item);

		//! \brief Shows the popup menu over the current selection
		void showPopupMenu(const QPoint &pnt);

		//! \brief Save the selected items to external file
		void saveSelectedItems(bool csv_format);

		//! \brief Toggles the visibility of the filter widget
		void toggleFilter(bool toggle);

		//! \brief Toggles the visibility of the CSV loader widget
		void toggleCsvLoader(bool toggle);

	signals:
		//! \brief Signal emitted whenever the data changes in the grid
		void s_dataModified(bool);

		//! \brief Signal emitted whenever the undo operation availiability changes in the grid
		void s_undoEnabled(bool);

		//! \brief Signal emitted whenever the save operation availiability changes in the grid
		void s_saveEnabled(bool);

		//! \brief Signal emitted whenever the selection availiability changes in the grid
		void s_selectionEnabled(bool);

		//! \brief Signal emitted whenever the table browsing availiability changes in the grid
		void s_browseEnabled(bool);

		//! \brief Signal emitted whenever the paste operation availiability changes in the grid
		void s_pasteEnabled(bool);

		//! \brief Signal emitted whenever the edit operation availiability changes in the grid
		void s_editEnabled(bool);

		//! \brief Signal emitted whenever the export operation availiability changes in the grid
		void s_exportEnabled(bool);

		//! \brief Signal emitted whenever the truncate operation availiability changes in the grid
		void s_truncateEnabled(bool);

		//! \brief Signal emitted whenever the filter widget availiability changes in the grid
		void s_filterEnabled(bool);

		//! \brief Signal emitted whenever the user tries to browse a referenced/referred table
		void s_browseTableRequested(const QString &schema, const QString &table, const QString &filter, ObjectType obj_type);

		//! \brief Signal emitted whenever the user tries to duplicate the current data grid
		void s_gridDuplicationRequested(DataGridWidget *grid);

		friend class DataHandlingForm;
};

#endif
