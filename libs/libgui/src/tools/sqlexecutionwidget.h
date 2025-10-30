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
\class SQLExecutionWidget
\brief Implements the operations that permit user to perform basic SQL commands directly on a server
*/

#ifndef SQL_EXECUTION_WIDGET_H
#define SQL_EXECUTION_WIDGET_H

#include <QThread>
#include <QMenu>
#include "ui_sqlexecutionwidget.h"
#include "utils/syntaxhighlighter.h"
#include "connection.h"
#include "widgets/searchreplacewidget.h"
#include "widgets/codecompletionwidget.h"
#include "widgets/numberedtexteditor.h"
#include "sqlexecutionhelper.h"

class __libgui SQLExecutionWidget: public QWidget, public Ui::SQLExecutionWidget {
	Q_OBJECT

	private:
		static std::map<QString, QString> cmd_history;

		static int cmd_history_max_len;

		qint64 start_exec, end_exec, total_exec;

		SchemaParser schparser;

		QThread sql_exec_thread;

		SQLExecutionHelper sql_exec_hlp;

		//! \brief Syntax highlighter for sql input field
		SyntaxHighlighter *sql_cmd_hl,

		//! \brief Syntax highlighter for command history field
		*cmd_history_hl;

		//! \brief Connection used to run commands specified on sql input field
		Connection sql_cmd_conn;

		QMenu snippets_menu,

		file_menu,

		export_menu,

		code_menu,

		result_menu;

		QAction *action_save, *action_save_as, *action_load,
		*action_wrap, *action_clear_all, *action_search,
		*action_filter, *action_export;

		SearchReplaceWidget *find_replace_wgt;

		CodeCompletionWidget *code_compl_wgt;

		SearchReplaceWidget *search_history_wgt;

		/*! \brief Enables/Disables the fields for sql input and execution.
				When enabling a new connection to server will be opened. */
		void enableSQLExecution(bool enable);

		//! \brief Stores the command on the sql command history
		void addToSQLHistory(const QString &cmd, unsigned rows=0, const QString &error="");

		static void validateSQLHistoryLength(const QString &conn_id, const QString &fmt_cmd = "", NumberedTextEditor *cmd_history_txt = nullptr);

		void switchToExecutionMode(bool value);

		void destroyResultModel();

	protected:
		//! \brief Widget that serves as SQL commands input
		NumberedTextEditor *sql_cmd_txt,

		*cmd_history_txt;

		void showEvent(QShowEvent *) override;
		void resizeEvent(QResizeEvent *) override;
		bool eventFilter(QObject *object, QEvent *event) override;
		void reloadHighlightConfigs();

		void installPluginWidgets(QToolButton *btn, QWidget *wgt);

	public:
		static const QString ColumnNullValue;

		SQLExecutionWidget(QWidget * parent = nullptr);

		~SQLExecutionWidget() override;

		//! \brief Configures the connection to query the server
		void setConnection(Connection conn);

		//! \brief Insert the provided sql commands in the input field. This method clears the current commands before adding new content
		void setSQLCommand(const QString &sql);

		//! \brief Returns true when there is a SQL command typed, otherwise, false is returned
		bool hasSQLCommand();

		//! \brief Returns the currently typed command
		QString getSQLCommand(bool selected);

		/*! \brief Fills up the results grid based upon the specified result set.
		 * The parameter store_data will make each item store the text as its data. */
		static void fillResultsTable(Catalog &catalog, ResultSet &res, QTableWidget *results_tbw, bool store_data = false);

		/*! \brief Copy to clipboard (in csv format) the current selected items on results grid
		 * Optionally, the column names can be included/excluded in the resulting buffer */
		static void copySelection(QTableView *results_tbw, bool use_popup=true, bool csv_is_default = false, bool incl_col_names = true);

		/*! \brief Generates a CSV buffer based upon the selection on the results grid
		 *  Optionally, the column names can be included/excluded in the resulting buffer */
		static QByteArray generateCSVBuffer(QTableView *results_tbw, bool inc_col_names = true);

		/*! \brief Generates a Plain text buffer based upon the selection on the results grid
		 * Optionally, the column names can be included/excluded in the resulting buffer.
		 * In this method the column names are by default excluded */
		static QByteArray generateTextBuffer(QTableView *results_tbw, bool inc_col_names = false);

		/*! \brief Generates a custom text buffer. User can specify a separator for columns, include column names and if the output
		 *  buffer is whether in CSV format or not */
		static QByteArray generateBuffer(QTableView *results_tbw, QChar separator, bool incl_col_names, bool csv_format);

		//! \brief Exports the results to csv file
		static void exportResults(QTableView *results_tbw, bool csv_format);

		//! \brief Save the history of all connections open in the SQL Execution to the sql-history.conf
		static void saveSQLHistory();

		//! \brief Load the history from the file sql-history.conf
		static void loadSQLHistory();

		static void destroySQLHistory();

		static void setSQLHistoryMaxLength(int len);

		static int getSQLHistoryMaxLength();

	public slots:
		void configureSnippets();

		//! \brief Show the exception message in the output widget
		void	handleExecutionAborted(Exception e);

		//! \brief Runs the current typed sql command
		void runSQLCommand();

		//! \brief Runs the provided sql command
		void runSQLCommand(const QString &cmd);

	private slots:
		//! \brief Save the current typed sql command on a file
		void saveCommands();

		//! \brief Load a sql command from a file
		void loadCommands();

		//! \brief Clears the input field as well the results grid
		int clearAll();

		void clearOutput();

		void selectSnippet(QAction *act);

		void toggleOutputPane(bool visible);

		void showHistoryContextMenu();

		void finishExecution(int rows_affected = 0);

		void filterResults();

		void togglePluginButton(bool checked);

		friend class SQLToolWidget;
};

#endif
