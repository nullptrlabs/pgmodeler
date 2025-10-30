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
\class MainWindow
\brief Implements the pgModeler main window. Reuniting all the resources implemented by the libraries.
*/

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPrintDialog>
#include "ui_mainwindow.h"
#include "widgets/modelwidget.h"
#include "widgets/aboutwidget.h"
#include "tools/modelrestorationform.h"
#include "widgets/operationlistwidget.h"
#include "widgets/modelobjectswidget.h"
#include "widgets/modeloverviewwidget.h"
#include "tools/modelvalidationwidget.h"
#include "widgets/objectsearchwidget.h"
#include "tools/sqltoolwidget.h"
#include "widgets/updatenotifierwidget.h"
#include "widgets/modelnavigationwidget.h"
#include "widgets/welcomewidget.h"
#include "widgets/donatewidget.h"
#include "widgets/sceneinfowidget.h"
#include "widgets/layersconfigwidget.h"
#include "widgets/changelogwidget.h"
#include "settings/configurationwidget.h"
#include "tools/databaseimportwidget.h"
#include "tools/modelexportwidget.h"
#include "tools/difftoolwidget.h"
#include "tools/fixtoolswidget.h"

class __libgui MainWindow: public QMainWindow, public Ui::MainWindow {
	Q_OBJECT

	public:
		enum MWViewsId {
			WelcomeView,
			DesignView,
			ManageView,
			ImportView,
			ExportView,
			DiffView,
			FixView,
			ConfigureView,
		};

	private:
		static int ToolsActionsCount;

		static bool confirm_validation;

		static constexpr int InfinityInterval = INT_MAX;

		//! \brief Constants used to mark a pending operation to be executed after validate model
		enum PendingOpId: unsigned {
			NoPendingOp,
			PendingSaveOp,
			PendingSaveAsOp,
			PendingExportOp,
			PendingDiffOp
		};

		//! \brief Store the actions related to views in the main window (Manage, Design, Welcome, etc)
		static QList<QAction *> view_actions;

		PendingOpId pending_op;

		//! \brief Timer used for auto saving the model and temporary model.
		QTimer model_save_timer,	tmpmodel_save_timer;

		AboutWidget *about_wgt;

		DonateWidget *donate_wgt;

		SceneInfoWidget *scene_info_wgt;

		//! \brief Layers management widget
		LayersConfigWidget *layers_cfg_wgt;

		//! \brief Layers management widget
		ChangelogWidget *changelog_wgt;

		/*! \brief Widget positioned on the center of main window that contains some basic operations like
		create new model, open a file, restore session */
		WelcomeWidget *welcome_wgt;

		//! \brief Model overview widget
		ModelOverviewWidget *overview_wgt;

		//! \brief Widget used to navigate through the opened models.
		ModelNavigationWidget *model_nav_wgt;

		//! \brief Model validation widget
		ModelValidationWidget *model_valid_wgt;

		//! \brief SQL tool widget widget
		SQLToolWidget *sql_tool_wgt;

		//! \brief pgModeler configuration widget
		ConfigurationWidget *configuration_wgt;

		//! \brief Reverse engineering widget
		DatabaseImportWidget *db_import_wgt;

		//! \brief Model export widget
		ModelExportWidget *model_export_wgt;

		//! \brief Diff tool widget
		DiffToolWidget *diff_tool_wgt;

		//! \brief Fix tools widget
		FixToolsWidget *fix_tools_wgt;

		//! \brief Operation list dock widget
		OperationListWidget *oper_list_wgt;

		//! \brief Model objects dock widget
		ModelObjectsWidget *model_objs_wgt;

		//! \brief Temporary model restoration form
		ModelRestorationForm *restoration_form;

		//! \brief Object finder used as dock widget
		ObjectSearchWidget *obj_finder_wgt;

		//! \brief Update notifier popup widget
		UpdateNotifierWidget *update_notifier_wgt;

		//! \brief Stores the currently focused model
		ModelWidget *current_model;

		//! \brief Stores the model objects tree state for each opened model
		QMap<ModelWidget *, QStringList> model_tree_states;

		//! \brief Stores the model objects tree vertical scrollbar position for each opened model
		QMap<ModelWidget *, int> model_tree_v_pos;

		//! \brief Stores the defaul window title
		QString window_title;

		//! \brief Stores the recent models file names
		QStringList recent_models,

		//! \brief Stores the last session file names
		prev_session_files;

		//! \brief Stores the actions related to recent models
		QMenu *recent_models_menu,

		clear_recent_menu,

		main_menu,

		sample_mdls_menu,

		arrange_menu,

		more_actions_menu,

		plugins_config_menu,

		expand_canvas_menu;

		QAction *action_expand_canvas;

		//! \brief Stores the loaded plugins toolbar actions
		QList<QAction *> plugins_tb_acts;

		//! \brief Stores the loaded plugins tool buttons
		QList<QToolButton *> plugins_tool_btns;

		QMap<QString, QIcon> recent_models_icons;

		//! \brief QMainWindow::closeEvent() overload: Saves the configurations before close the application
		void closeEvent(QCloseEvent *event) override;

		void resizeEvent(QResizeEvent *) override;

		//! \brief Set the postion of a floating widget based upon an action at a tool bar
		void setFloatingWidgetPos(QWidget *widget, QAction *act, QToolBar *toolbar, bool map_to_window);

		void setBottomFloatingWidgetPos(QWidget *widget, QAbstractButton *btn);

		void configureSamplesMenu();

		/*! \brief Stores the current checkboxes states of the main dock widgets on the set of configuration params
				in order to save them on the main configuration file */
		void storeDockWidgetsSettings();

		//! \brief Restore the dock widget configurations from the parameters loaded from main configuration file
		void restoreDockWidgetsSettings();

		/*! \brief This method determines if the provided layout has togglable buttons and one of them are checked.
		 * This is an auxiliary method used to determine if widget bars (bottom or right) can be displayed based upon
		 * the current button toggle state. */
		bool isToolButtonsChecked(QHBoxLayout *layout, const QWidgetList &ignored_wgts = QWidgetList());

		/*! \brief Resizes the buttons of the general toolbar (the one positioned at the left portion of main window)
		 * The resizing is performed through stylesheet and is related to the minimum width of the tools_acts_tb plus
		 * a size factor that is applied if there are models opened (which expands/enlarges the toolbar a little
		 * more due to the labels of the actions added during model loading ) */
		void resizeGeneralToolbarButtons();

		void connectSignalsToSlots();

		void loadConfigurations();

		void createMainWidgets();

		void configureMenusActionsWidgets();

		void dragEnterEvent(QDragEnterEvent *event) override;

		void dropEvent(QDropEvent *event) override;

		/*! \brief Tries to restore the default configuration files and restart pgModeler
		 *  in case of any configuration file is broken or missing */
		void handleInitializationFailure(Exception &e);

		//! \brief Creates a view widget in insert its in the stacked widget at view_id index
		template <class WgtClass>
		WgtClass *createViewWidget(MWViewsId view_id, const QString &view_name);

	public:
		MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Widget);

		~MainWindow() override;

		//! \brief Loads a set of models from string list
		void loadModels(const QStringList &files);

		//! \brief Loads a model from a specified filename
		void loadModel(const QString &filename);

		//! \brief Indicates if model must be validated before save, diff or export
		static void setConfirmValidation(bool value);

		//! \brief Returns the current working database model widget
		ModelWidget *getCurrentModel();

	public slots:
		/*! \brief Creates a new empty model inside the main window. If the parameter 'filename' is specified,
		creates the model loading it from a file */
		void addModel(const QString &filename="");

		/*! \brief Creates a new model inside the main window using the specified model widget. The method will raise
		an error is the widget isn't allocated or already has a parent */
		void addModel(ModelWidget *model_wgt);

		//! \brief Closes the currently focused model. If the 'model_id' is specified the model at given index is closed
		void closeModel(int model_id=-1);

		//! \brief Returns the currently loaded model count.
		int getModelCount();

		//! \brief Returns the model at given index
		ModelWidget *getModel(int idx);

		/*! \brief This is a convenience method to make able the addition of execution tabs in SQL tool without
		 *  expose the SQL Tool widget itself (useful for plugin developers) */
		void addExecTabInSQLTool(const QString &sql_cmd);

		/*! \brief This is a convenience method that returns a true value when there're databases listed in the SQL tool widget without
		 *  expose the SQL Tool widget itself (useful for plugin developers) */
		bool hasDbsListedInSQLTool();

		//! \brief Adds an entry to the recent models menu
		void registerRecentModel(const QString &filename);

		//! \brief Adds several entries to the recent models menu
		void registerRecentModels(const QStringList &filenames);

		//! \brief Register an icon for a specific file extension (suffix) in the recent models menu
		void registerRecentModelIcon(const QString &suffix, const QIcon &file_type_icon);

		//! \brief Updates the window title taking into account the current model filename
		void updateWindowTitle();

		//! \brief Updates the tab name of the currently opened model if the database name is changed
		void updateModelTabName();

		//! \brief Updates the connections list of the validator widget
		void updateConnections(bool force = false);

		//! \brief Shows a error dialog informing that the model demands a fix after the error ocurred when loading the filename.
		void showFixMessage(Exception &e, const QString &filename);

	private slots:
		void showMainMenu();

		//! \brief Atualiza as definições da grade com base nas ações: Exibir Grade, Alin. Grade e Exibir limites
		void setGridOptions();

		//! \brief Updates the state (enable/disable) of the buttons of toolbars
		void updateToolsState(bool model_closed=false);

		void __updateToolsState();

		//! \brief Updates the operation list and model objects dockwidgets
		void updateDockWidgets();

		//! \brief Updates the reference to the current model when changing the tab focus
		void setCurrentModel();

		//! \brief Loads a model from a file via file dialog
		void loadModel();

		//! \brief Saves the currently focused model. If the parameter 'model' is set, saves the passed model
		void saveModel(ModelWidget *model=nullptr);

		//! \brief Save all loaded models
		void saveAllModels();

		//! \brief Prints the currently focused model
		void printModel();

		//! \brief Executes the validation before the export process
		void validateBeforeOperation();

		//! \brief Executes the model <> database comparison
		//void diffModelDatabase();

		//! \brief Updates the opened models with new configurations
		void applyConfigurations();

		//! \brief Applies the zoom to the currently focused model
		void applyZoom();

		//! \brief Toggles the overview widget for the currently opened model
		void showOverview(bool show);

		//! \brief Loads a recent model. The filename is get from the action that triggered the slot
		void loadModelFromAction();

		//! \brief Clears the recent models menu/list
		void clearRecentModelsMenu(bool missing_only);

		//! \brief Update the recent models menu entries
		void updateRecentModelsMenu();

		/*! \brief Validates if the current entries in the recent models menu
		 *  points to valid files. In case an action contains an invalid file (not accessible)
		 *  the item is striked out indicating the invalid status */
		void validateRecentModelsActions();

		//! \brief Save the temp files for all opened models
		void saveTemporaryModels();

		//! \brief Opens the pgModeler Wiki in a web browser window
		void openSupport();

		/*! \brief Stop the saving timers. This is used when validating the model
		in order to avoid the saving while the validation is working */
		void stopTimers(bool value);

		//! \brief Executes one of the pending operations (save, export, diff) after validate the model
		void executePendingOperation(bool valid_error);

		//! \brief Configures the "More" actions in the general toolbar by usinge the current_model's popup menu
		void configureMoreActionsMenu();

		void showRightWidgetsBar();
		void showBottomWidgetsBar();
		void restoreLastSession();
		void toggleUpdateNotifier(bool show);
		void toggleAboutWidget(bool show);
		void toggleDonateWidget(bool show);
		void removeModelActions();
		void changeCurrentView(bool checked);
		void reportBug();
		void removeOperations();
		void restoreTemporaryModels();
		void arrangeObjects();
		void toggleCompactView();
		void toggleLayersWidget(bool show);
		void toggleChangelogWidget(bool show);
		void expandSceneRect();

		#ifdef	DEMO_VERSION
		void showDemoVersionWarning(bool exit_msg = false);
		#endif

		bool mimeDataHasModelFiles(const QMimeData *mime_data);
		void loadModelsFromMimeData(const QMimeData *mime_data);
		void addNewLayer(const QString &layer_name);
		void handleImportFinished(bool aborted_by_error);
		void loadDiffInSQLTool(const QString &conn_id, const QString &database, const QString &filename);

	signals:
		void s_currentModelChanged(ModelWidget *model_wgt);
		void s_modelSaved(ModelWidget *model_wgt);

		/*! \brief This signal is emitted when an extraneous file (not .dbm) is among
		 * the list of models to be loaded in loadModels(). The intention of this signal
		 * is to notify any plugin that may handle the file type to be loaded */
		void s_modelLoadRequested(const QString &filename);
};

#endif
