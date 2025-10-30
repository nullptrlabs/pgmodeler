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
\class CustomTableWidget
\brief Implements a customized table widget (grid) which can be used commonly to store
objects references and show information about them on it's rows. This
class also implements operations to handle insertion/deletion/update/movements
of rows on the table.
*/

#ifndef CUSTOM_TABLE_WIDGET_H
#define CUSTOM_TABLE_WIDGET_H

#include "ui_customtablewidget.h"
#include "guiglobal.h"

class __libgui CustomTableWidget: public QWidget, public Ui::CustomTableWidget {
	Q_OBJECT

	private:
		static QColor item_colors[12];

		/*! \brief Indicates that a confirmation message must be shown when the user try
		to remove an element from table. By default, the exclusions are made without confirmation */
		bool conf_exclusion,

		tab_adds_row;

		QTableWidgetItem *getItem(unsigned row_idx, unsigned col_idx);

		//! \brief Updates the row numbers in the vertical header after remove/moving rows in the table
		void updateVerticalHeader();

	public:
		//! \brief Constants used to configure the table buttons
		enum ButtonConf: unsigned {
			NoButtons = 0,
			AddButton = 1,
			RemoveButton = 2,
			UpdateButton = 4,
			MoveButtons = 8,
			EditButton = 16,
			DuplicateButton = 32,
			RemoveAllButton = 64,
			ResizeColsButton = 128,
			AllButtons = 255
		};

		enum TableItemColor: unsigned {
			//! \brief References the default background color  for protected items
			ProtItemBgColor,

			//! \brief References the default foreground color  for protected items
			ProtItemFgColor,

			//! \brief References the default background color  for relationship added items
			RelAddedItemBgColor,

			//! \brief References the default foreground color  for relationship added items
			RelAddedItemFgColor,

			//! \brief References the default background color  for added items
			AddedItemBgColor,

			//! \brief References the default foreground color  for added items
			AddedItemFgColor,

			//! \brief References the default background color  for updated items
			UpdatedItemBgColor,

			//! \brief References the default foreground color  for updated items
			UpdatedItemFgColor,

			//! \brief References the default background color  for updated items
			RemovedItemBgColor,

			//! \brief References the default foreground color  for removed items
			RemovedItemFgColor,

			//! \brief References the alternative foreground color  for protected items
			ProtItemAltFgColor,

			//! \brief References the alternative foreground color  for relationship added items
			RelAddedItemAltFgColor
		};

		CustomTableWidget(QWidget * parent = nullptr);

		CustomTableWidget(ButtonConf button_conf, bool conf_exclusion, QWidget * parent = nullptr);

		//! \brief Sets the table's column count
		void setColumnCount(unsigned col_count);

		//! \brief Sets the specified column header label
		void setHeaderLabel(const QString &label, unsigned col_idx);

		//! \brief Sets the specified column header to be visible or not
		void setHeaderVisible(unsigned col_idx, bool visible);

		//! \brief Sets the specified column header icon
		void setHeaderIcon(const QIcon &icon, unsigned col_idx);

		//! \brief Sets the icon of the specified cell
		void setCellIcon(const QIcon &icon, unsigned row_idx, unsigned col_idx);

		//! \brief Sets the text of the specified cell
		void setCellText(const QString &text, unsigned row_idx, unsigned col_idx);

		//! \brief Sets the flags of the specified cell
		void setCellFlags(Qt::ItemFlags flags, unsigned row_idx, unsigned col_idx);

		//! \brief Sets the data which the specified row stores
		void setRowData(const QVariant &data, unsigned row_idx);

		//! \brief Sets a individual font configuration for the specified row
		void setRowFont(int row_idx, const QFont &font);

		//! \brief Sets a individual foreground/background configuration for the specified row
		void setRowColors(int row_idx, const QColor &fg_color, const QColor &bg_color);

		//! \brief Returns the table column count
		unsigned getColumnCount();

		//! \brief Returns the table row count
		unsigned getRowCount();

		//! \brief Returns the specified column header label
		QString getHeaderLabel(unsigned col_idx);

		//! \brief Returns the specified cell text
		QString getCellText(unsigned row_idx, unsigned col_idx);

		/*! \brief Returns the specified cell texts.
		 * The section_idx parameter indicates the section (row/column) in which the texts must be retrieved.
		 *
		 * If the orientation is Qt::Horizontal then section_idx will refer to the row and the returned
		 * list will contain the texts of all columns (cells) in that row.
		 *
		 * If the orientation is Qt::Vertical then section_idx will refer to the column and the returned
		 * list will contain the texts of all rows in that column. */
		QStringList getCellTexts(unsigned section_idx, Qt::Orientation orientation = Qt::Horizontal);

		Qt::CheckState getCellCheckState(unsigned row_idx, unsigned col_idx);

		void setCellCheckState(Qt::CheckState check_state, unsigned row_idx, unsigned col_idx);

		void setCellDisabled(bool disabled, unsigned row_idx, unsigned col_idx);

		bool isCellDisabled(unsigned row_idx, unsigned col_idx);

		//! \brief Returns the data relative to the specified row
		QVariant getRowData(unsigned row_idx);

		//! \brief Remove the specified column from table
		void removeColumn(unsigned col_idx);

		//! \brief Adds a new column at the specified index
		void addColumn(unsigned col_idx);

		//! \brief Adds a new row at the specified index
		void addRow(unsigned lin_idx);

		//! \brief Returns the row index currently selected
		int getSelectedRow();

		/*! \brief Returns the row index search it through the specified row data. If
		no row is found returns -1 */
		int getRowIndex(const QVariant &data);

		//! \brief Clears the specified cell text
		void clearCellText(unsigned row_idx, unsigned col_idx);

		//! \brief Sets the table button configuration. Use the constants ???_BUTTON combined via bitwise operation.
		void setButtonConfiguration(ButtonConf button_conf);

		void setSelectionMode(QTableWidget::SelectionMode sel_mode);

		void adjustColumnToContents(int col);

		void setVerticalHeaderVisible(bool value);

		void setSortingEnabled(bool value);

		/*! \brief Toggles the addition of rows by using tab press when the cursor
		 *  is at the last cell of the last row */
		void setAddRowOnTabPress(bool value);

		/*! \brief Adds a custom tool button to the end of buttons' list.
		 *  The parent of the button is changed to the custom table itself */
		void addCustomButton(QToolButton *btn);

		void setItemContextMenu(QMenu *menu);

		static void setTableItemColor(TableItemColor color_idx, const QColor color);

		static QColor getTableItemColor(TableItemColor color_idx);

	private slots:
		//! \brief Moves a row up or down according to the button that triggers the slot
		void moveRows();

		//! \brief Removes the currently selected line
		void removeRow();

		//! \brief Duplicate the selected row creating a new item in the end of the grid
		void duplicateRow();

		/*! \brief This method does not execute any action, only emit a signal indicating that the row
		is ready to the edition. The edit operation must be implemented by the user and be connected to
		the emitted signal. */
		void editRow();

		/*! \brief This method does not execute any action, only emit a signal indicating that the row
		is ready to the update. The update operation must be implemented by the user and be connected to
		the emitted signal. */
		void updateRow();

		void emitRowSelected();

		void addRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col);

	public slots:
		//! \brief Adds a new row at the end of the table and returns its id
		int addRow();

		//! \brief Removes all the rows from table
		void removeRows();

		//! \brief Remove the specified row
		void removeRow(unsigned row_idx);

		//! \brief Clears the row selection disabling the buttons if necessary
		void clearSelection();

		//! \brief Selects the specified row
		void selectRow(int lin_idx);

		//! \brief Enables the handle buttons according to the selected row
		void setButtonsEnabled();

		//! \brief Controls the enable state of each button
		void setButtonsEnabled(CustomTableWidget::ButtonConf button_conf, bool value);

		//! \brief Toggle the edition of individual cells
		void setCellsEditable(bool value);

		//! \brief Resize equally the rows and columns to their contents
		void resizeContents();

	signals:
		//! \brief Signal emitted when a new row is added. The new row index is send with the signal
		void s_rowAdded(int);

		//! \brief Signal emitted when two rows are swapped. The rows indexes are send together with the signal
		void s_rowsMoved(int,int);

		//! \brief Signal emitted when all rows are removed from table
		void s_rowsRemoved();

		//! \brief Signal emitted when a single row is removed. The row index is sent together with the signal
		void s_rowRemoved(int);

		//! \brief Signal emitted when a single row is about to be removed. The row index is sent together with the signal
		void s_rowAboutToRemove(int);

		//! \brief Signal emitted when a row is selected. The row index is sent together with the signal
		void s_rowSelected(int);

		//! \brief Signal emitted when a row is edited. The row index is sent together with the signal
		void s_rowEdited(int);

		//! \brief Signal emitted when a row is duplicated. The indexes of the selected and generated rows are sent together with the signal
		void s_rowDuplicated(int, int);

		//! \brief Signal emitted when a row is updated. The row index is sent together with the signal
		void s_rowUpdated(int);

		//! \brief Signal emitted when a column is removed. The column index is sent together with the signal
		void s_columnRemoved(int);

		//! \brief Signal emitted when a column is added. The column index is sent together with the signal
		void s_columnAdded(int);

		//! \brief Signal emitted when a specific cell is clicked. The column and rows indexes are sent together with the signal
		void s_cellClicked(int, int);

		//! \brief Signal emitted when a specific cell has its text or other attribute changed. The column and rows indexes are sent together with the signal
		void s_cellChanged(int, int);

		//! \brief Signal emitted when the table selection is cleared
		void s_selectionCleared();

		//! \brief Signal emitted when the table row count changes by adding, deleting, duplicating rows
		void s_rowCountChanged(int);

		/*! \brief Signal emitted when the user request the context menu (right-click on table)
		 *  The signal carries the point where the context menu was requested in global coordinate,
		 *  as well as if an item is selected on the cursor's position */
		void s_contextMenuRequested(const QPoint &, bool);

	protected:
		void resizeEvent(QResizeEvent *) override;
};

#endif
