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
\class DiffToolWidget
\brief Implements the operations to compare and generate a diff script by
			 performing comparisons: model <-> model, model <-> database, database <-> database
*/

#ifndef DIFF_TOOL_WIDGET
#define DIFF_TOOL_WIDGET

#include "ui_difftoolwidget.h"
#include "modelsdiffhelper.h"
#include "databaseimporthelper.h"
#include "modelexporthelper.h"
#include "utils/syntaxhighlighter.h"
#include "utils/htmlitemdelegate.h"
#include "widgets/numberedtexteditor.h"
#include "settings/baseconfigwidget.h"
#include "widgets/fileselectorwidget.h"
#include "widgets/objectsfilterwidget.h"
#include "widgets/searchreplacewidget.h"
#include "widgets/objecttypeslistwidget.h"
#include "widgets/debugoutputwidget.h"
#include "modeldbselectorwidget.h"
#include <QThread>

class __libgui DiffToolWidget: public BaseConfigWidget, public Ui::DiffToolWidget {
	Q_OBJECT

	private:
		//! \brief Constants used to reference the thread/helper to be handled in createThread() and destroyThread()
		enum ThreadId {
			InputImpThread,
			ComparedImpThread,
			DiffThread,
			ExportThread
		};

		/*! \brief Indicates if the full output generated during the process should be displayed
		 * When this attribute is true, only errors and some key info messages are displayed. */
		static bool low_verbosity;

		static std::map<QString, attribs_map> config_params;

		static const QString ForceObjsBtnLabel;

		ModelDbSelectorWidget *input_sel_wgt, *compared_sel_wgt;

		DebugOutputWidget *dbg_output_wgt;

		bool is_adding_new_preset;

		NumberedTextEditor *sqlcode_txt;

		ObjectTypesListWidget *forced_obj_types_wgt;

		FileSelectorWidget *file_sel;

		ObjectsFilterWidget *pd_filter_wgt;

		SearchReplaceWidget *search_sql_wgt;

		//! \brief Custom delegate used to paint html texts in output tree
		HtmlItemDelegate *htmlitem_del;

		//! \brief Syntax highlighter used on the diff preview tab
		SyntaxHighlighter *sqlcode_hl;

		//! \brief Helper that will execute the diff between models
		ModelsDiffHelper *diff_helper;

		//! \brief Helper that will execute the database import
		DatabaseImportHelper *compared_imp_helper, *input_imp_helper;

		//! \brief Helper that will execute the diff export to database
		ModelExportHelper *export_helper;

		//! \brief Threads that will execute each step: import, diff, export
		QThread *compared_imp_thread, *diff_thread, *export_thread, *input_imp_thread;

		//! \brief Tree items generated in each diff step
		QTreeWidgetItem *compared_imp_item, *diff_item, *export_item, *input_imp_item;

		//! \brief Stores the objects filtered from the database model
		std::vector<BaseObject *> filtered_objs;

		/*! \brief This is the model used in the diff process representing the source.
		* It can be the model loaded from file or a representation of the source database
		* (when comparing two dbs) */
		DatabaseModel *input_model,

		/*! \brief This is the model loaded from file when comparing two models or
		 *  the model imported from database when comparing two databases or
		 *  a model and a databas */
		*compared_model;

		ModelWidget *input_model_wgt, *compared_model_wgt;

		//! \brief Connection used to export the diff to database
		Connection *export_conn;

		//! \brief PostgreSQL version used by the diff process
		QString pgsql_ver;

		int diff_progress, curr_step, total_steps;

		bool process_paused;

		void showEvent(QShowEvent *event) override;

		//! \brief Creates the helpers and threads
		void createThread(ThreadId thread_id);

		//! \brief Destroy the helpers and threads
		void destroyThread(ThreadId thread_id);

		//! \brief Destroy the imported model
		void destroyModel();

		void clearOutput();
		void resetButtons();
		void saveDiffToFile();
		void finishDiff();

		//! \brief Applies the loaded configurations to the form. In this widget only list the loaded presets
		void applyConfiguration() override;

		//! \brief Loads a set of configurations from a file
		void loadConfiguration() override;

		//! \brief Saves the current settings to a file
		void saveConfiguration() override;

		void togglePresetConfiguration(bool toggle, bool is_edit = false);

		void enablePresetButtons();

		/*! \brief When performing a partial diff between a model and database this method fills a vector with the
		 * filtered objects in the source database model */
		void getFilteredObjects(std::vector<BaseObject *> &objects);

	public:
		DiffToolWidget(QWidget * parent = nullptr);

		~DiffToolWidget() override;

		//! \brief Defines if all the output generated during the import process should be displayed
		static void setLowVerbosity(bool value);

		//! \brief Returns true when one or more threads of the whole diff process are running.
		bool isThreadsRunning();

		//! \brief Updates the connections combo in the model/db pickers with the latest loaded connection settings
		void updateConnections();

		//! \brief Updates the models combo with in the model/db pickers with the ones open in Design view
		void updateModels(const QList<ModelWidget *> &models);

	private slots:		
		void selectDiffMode();
		void enableDiffControls();
		void startDiff();

		/*! \brief This method cancel any running thread emitting an signal
		 *  to indicate the cancelled status. It also reset the diff widget
		 *  to its initial state */
		void cancelOperation(bool cancel_by_user);

		void updateProgress(int progress, QString msg, ObjectType obj_type, QString cmd="");
		void updateDiffInfo(ObjectsDiffInfo diff_info);
		void captureThreadError(Exception e);
		void handleImportFinished(Exception e);
		void handleDiffFinished();
		void handleExportFinished();
		void handleErrorIgnored(QString err_code, QString err_msg, QString cmd);
		void importDatabase(ThreadId thread_id);
		void diffModels();
		void exportDiff(bool confirm=true);
		void filterDiffInfos();
		void loadDiffInSQLTool();
		void selectPreset();
		void removePreset();
		void savePreset();
		void selectModels();
		void enablePartialDiff();
		void enableFilterByDate();
		void applyPartialDiffFilters();
		void generateFiltersFromChangelog();

		//! \brief Destroy the current configuration file and makes a copy of the default one located at conf/defaults
		void restoreDefaults() override;

	signals:
		/*! \brief This signal is emitted whenever the user changes the connections settings
		within this widget without use the main configurations dialog */
		void s_connectionsUpdateRequested();

		/*! \brief This signal is emitted whenever the user wants to load the generated diff in the sql tool
		 * The signal contains the connection id, the database name and the temp filename that is generated containing
		 * the commands to be loaded */
		void s_loadDiffInSQLTool(QString conn_id, QString database, QString sql_file);

		//! \brief This signal is emitted whenever the diff has started
		void s_diffStarted();

		//! \brief This signal is emitted by cancelOperation.
		void s_diffCanceled();

	friend class MainWindow;
};

#endif
