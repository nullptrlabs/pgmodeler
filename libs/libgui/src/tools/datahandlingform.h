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
\class DataHandlingForm
\brief Implements the operations to handle several tables' data in separate tabs
*/

#ifndef DATA_HANDLING_FORM_H
#define DATA_HANDLING_FORM_H

#include "ui_datahandlingform.h"
#include "guiglobal.h"
#include "catalog.h"
#include "baseobject.h"

class DataGridWidget;

class __libgui DataHandlingForm: public QDialog, public Ui::DataHandlingForm {
	Q_OBJECT

	private:	
		//! \brief Stores ther reference to the currently focused data grid widget (see setCurrentDataGrid)
		DataGridWidget *curr_grid_wgt;

		//! \brief Store the template connection params to be used by catalogs and command execution connections
		attribs_map tmpl_conn_params;

		//! \brief Stores the id of the connection used by the data handling form
		QString connection_id;

		void resizeEvent(QResizeEvent *event) override;

		void closeEvent(QCloseEvent *event) override;

		//! \brief This event filter override the tool buttons menus
		bool eventFilter(QObject *object, QEvent *event) override;

		//! \brief Shows a confirmation message before closing the form when there are pending operations in the grid
		int confirmFormClose();

		/*! \brief Shows a confirmation message before closing the tab identified by its index.
		 *  This method returns a flag indication the confirmation message display (in case the data grid is modified)
		 *  and the confirmation result */
		std::pair<bool, int> confirmDataGridClose(int idx);

		//! \brief Fills a combobox with the names of objects retrieved from catalog
		void listObjects(QComboBox *combo, std::vector<ObjectType> obj_types, const QString &schema = "");

		/*! \brief Closes the data grid specified by idx. If confirm_close is true then the closing operation
		 *  is conditional to the confirmation message result */
		void closeDataGrid(int idx, bool confirm_close);

	public:
		DataHandlingForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);

		~DataHandlingForm() override;

		/*! \brief Defines the connection and current schema and table to be handled,
		 * this method should be called before show the dialog */
		void setAttributes(const attribs_map &conn_params, const QString curr_schema = "public",
											 const QString curr_table_name = "", ObjectType obj_type = ObjectType::Table);

		//! \brief Opens a new data handling window
		static void openNewWindow(const attribs_map &conn_params, const QString &schema = "public",
															const QString &table = "", ObjectType obj_type = ObjectType::Table);

	private slots:
		void setCurrentDataGrid(int tab_idx);

		void reject() override;

		/*! \brief Adds a new data grid by automatically loading the data of the specified table with the specified filter
		 *  The optional retrieve_data controls wheter the data must be retrieve right after adding the data grid. */
		DataGridWidget *addDataGrid(const QString &schema, const QString &table, const QString &filter,
																ObjectType obj_type, bool retrieve_data);

		/*! \brief Adds a new data grid by automatically loading the data of the specified table with the specified filter
		 *  This version automatically retrive the data of the added data grid */
		DataGridWidget *addDataGrid(const QString &schema, const QString &table, const QString &filter, ObjectType obj_type);

		//! \brief List the tables based upon the current schema
		void listTables();
		
		//! \brief Disable the buttons used to handle data
		void enableRefreshButton();

		/*! \brief Appeds or removes the '*' to the tab name that called this slot indicating
		 *  depending on the value of 'modified', indicating that the tab has unsaved modifications */
		void setDataGridModified(bool modified);

		//! \brief Creates a new data grid by copying the filtering options of the provided grid
		void duplicateDataGrid(DataGridWidget *grid);
};

#endif
