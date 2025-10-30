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
\class SQLToolWidget
\brief Implements the operations that permit user to perform basic SQL commands directly on a server
*/

#ifndef SQL_TOOL_WIDGET_H
#define SQL_TOOL_WIDGET_H

#include "ui_sqltoolwidget.h"
#include "utils/syntaxhighlighter.h"
#include "widgets/numberedtexteditor.h"
#include "databaseexplorerwidget.h"
#include "sqlexecutionwidget.h"

class __libgui SQLToolWidget: public QWidget, public Ui::SQLToolWidget {
	Q_OBJECT

	private:
		QVBoxLayout *corner_wgt_lt;

		QList<QToolButton *> plugins_btns;

		QToolButton *sql_exec_corner_tb;

		NumberedTextEditor *sourcecode_txt;

		SyntaxHighlighter *sourcecode_hl;

		QWidget *corner_wgt;

		bool ignore_auto_browse_flag;

		/*! \brief Controls the link between a database explorer instance and SQL execution widgets.
		When a database explorer is closed all the SQL execution panes related to it are closed too.
		(see addSQLExecutionTab and closeSQLExecutionTab for deitails) */
		QMap<QWidget *, QWidgetList> sql_exec_wgts;

		bool eventFilter(QObject *object, QEvent *event) override;

	protected:
		void resizeEvent(QResizeEvent *) override;

		void setCornerButtonPos();

	public:
		SQLToolWidget(QWidget * parent = nullptr);

		~SQLToolWidget() override;

		//! \brief Force the update of the sql command input field and the syntax highligter attached to the opened tabs
		void updateTabs();

		//! \brief Indicates if there is at least one database being browsed through explorer widget
		bool hasDatabasesBrowsed();

		//! \brief Indicates if there is at least one SQL execution panel with typed commands
		bool hasSQLExecutionPanels();

		//! \brief Returns the list of execution tabs associated to the specified database explorer widget
		QWidgetList getExecutionTabs(DatabaseExplorerWidget *db_expl_wgt);

		/*! \brief Moves the execution widgets stored in the internal list of sql_exec_wgts related to the current database
		 * explorer widget when the user moves the tabs. */
		void moveExecutionTab(DatabaseExplorerWidget *db_expl_wgt, int from_idx, int to_idx);

		/*! \brief Indicates if SQL tool must ignore the auto-browse flag of the connections.
		 * This causes a selected connection with auto-browse=true to not open an instance of
		 * the maintanance database. Also, it causes the selected database in database_cmb to not
		 * create an empty SQL explorer tab */
		void ignoreAutoBrowseFlag(bool value);

		//! \brief Updates the connections combo with the latest loaded connection settings
		void updateConnections();

	public slots:
		void configureSnippets();
		void clearDatabases();

		//! \brief Add a tab to permit the SQL execution for the current database being browsed
		SQLExecutionWidget *addSQLExecutionTab(const QString &sql_cmd = "");

		//! \brief Close the database explorer specified by its index. Also, closes any SQL exec. tab related to it
		void closeDatabaseExplorer(int idx, bool confirm_close);

	protected slots:
		//! \brief Add a tab by browsing a database in the specified connection, loads the sql file and put its contents on a SQL execution
		void addSQLExecutionTab(const QString &conn_id, const QString &database, const QString &sql_file);

		//! \brief Force all SQL execution widget to reload the syntax highlight configuration
		void reloadHighlightConfigs();

	private slots:
		//! \brief Opens a connection to the selected server
		void connectToServer();

		//! \brief Disconnect from server and close any opened database explorer or sql execution tab
		void disconnectFromDatabases();

		//! \brief Open the current database in a database explorer instance
		DatabaseExplorerWidget *browseDatabase();

		//! \brief Show the selected snippet on the current opened SQL execution tab
		void showSnippet(const QString &snip);

		//! \brief Show the provided code in the source panel
		void showSourceCode(const QString &source, bool force_display);

		//! \brief Close the SQL execution tab specified by its index
		void closeSQLExecutionTab(int idx, bool confirm_close);

		//! \brief Drops the database selected in the database combo
		void dropDatabase(int database_idx);
		void dropDatabase(const QString &dbname);

	signals:
		/*! \brief This signal is emitted whenever the user changes the connections settings
		within this widget without use the main configurations dialog */
		void s_connectionsUpdateRequested();

		friend class MainWindow;
};

#endif
