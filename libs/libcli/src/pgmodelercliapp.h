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
\ingroup libcli
\class PgModelerCliApp
\brief Implements a set of operations that can be used without the graphical interface
*/

#ifndef PGMODELER_CLI_APP_H
#define PGMODELER_CLI_APP_H

#include "application.h"
#include <QObject>
#include <QTextStream>
#include <QCoreApplication>
#include "tools/modelexporthelper.h"
#include "settings/generalconfigwidget.h"
#include "settings/connectionsconfigwidget.h"
#include "settings/relationshipconfigwidget.h"
#include "tools/databaseimporthelper.h"
#include "tools/modelsdiffhelper.h"
#include "pgmodelercliplugin.h"

class __libcli PgModelerCliApp: public Application {
	Q_OBJECT

	private:
		XmlParser *xmlparser;

		qint64 buffer_size;

		bool has_fix_log,
		fix_model,
		upd_mime,
		import_db,
		diff,
		create_configs,
		list_conns,
		list_plugins,
		plugin_op,
		export_op;

		//! \brief Holds the pgModeler version in which the model was construted (used by the fix operation)
		QString model_version;

		//! \brief Export helper object
		ModelExportHelper *export_hlp;

		//! \brief Import helper object
		DatabaseImportHelper *import_hlp;

		//! \brief Diff helper object
		ModelsDiffHelper *diff_hlp;

		//! \brief Input database model
		DatabaseModel *input_model;

		//! \brief Graphical scene used to export the model to png
		ObjectsScene *scene;

		//! \brief Stores the configured connection
		Connection connection,

		//! \brief Stores the extra configured connection (only for diff)
		extra_connection;

		//! \brief Loaded connections
		std::map<QString, Connection *> connections;

		//! \brief Connection configuration widget used to load available connections from file
		ConnectionsConfigWidget *conn_conf;

		//! \brief Relationship configuration widget used to load custom relationship settings
		RelationshipConfigWidget *rel_conf;

		GeneralConfigWidget *general_conf;

		//! \brief Stores the instances of loaded plugins
		QList<PgModelerCliPlugin *> plugins, plug_exec_order;

		//! \brief Creates an standard out to handles QStrings
		static QTextStream out;

		//! \brief Stores the parsed options names and values.
		attribs_map parsed_opts;

		//! \brief Indicates if the cli must run in silent mode
		bool silent_mode;

		//! \brief Store the error stack related to plugins loading
		QString plugin_load_errors;

		//! \brief Stores the xml code for the objects being fixed
		QStringList objs_xml,

		//! \brief Stores the object filters for reverse engineering
		obj_filters;

		//! \brief Zoom to be applied onto the png export
		double zoom;

		//! \brief Start date used for filter changelog of the input database model (partial diff)
		QDateTime start_date,

		//! \brief End date used for filter changelog of the input database model (partial diff)
		end_date;

		/*! \brief Stores the member of role names that appear in deprecated tags <roles ... role-type="refer">
		 * This map is used to reconfigure the role memberships after all objects are created */
		std::map<QString, QStringList> member_roles;

		//! \brief Stores the changelog of the model that is being fixed to reproduce it in the output model
		QString changelog;

		static const QRegularExpression PasswordRegExp;

		static const QString PasswordPlaceholder;

		//! \brief Parsers the options and executes the action specified by them
		void parseOptions(attribs_map &parsed_opts);

		//! \brief Returns if the specified options exists on short options map
		bool isOptionRecognized(QString &op, bool &accepts_val);

		//! \brief Loads the input model and perform all tasks needed to configure the graphical objects
		void loadModel();

		/*! \brief Extracts the xml defintions from the input model and store them on obj_xml list
		in order to be parsed by the recreateObjects() method */
		void extractObjectXML();

		//! \brief Recreates the objects from the obj_xml list fixing the creation order for them
		void recreateObjects();

		//! \brief Fix some xml attributes and remove unused tags
		void fixObjectAttributes(QString &obj_xml);

		/*! \brief Extracts the foreign key code for the specified table xml. The foreign keys
		are recreated after all the other objects */
		QStringList extractForeignKeys(QString &obj_xml);

		//! \brief Returns if the specified string contains some of relationship attributes
		bool containsRelAttributes(const QString &str);

		/*! \brief Install the .dbm file association in the mime database (default behaviour).
		The paramenter 'uninstall' is used to clean up any file association done previously. */
		void handleMimeDatabase(bool uninstall, bool system_wide, bool force);

		/*! \brief Fixes the references to opertor classes and families by replacing tags like
		<opclass name="name"/> by <opclass signature="name USING index_method"/>. This method operates
		only over operator classes, indexes and constraints */
		void fixOpClassesFamiliesReferences(QString &obj_xml);

		void configureConnection(bool extra_conn);
		void importDatabase(DatabaseModel *model, Connection conn);

		void handleLinuxMimeDatabase(bool uninstall, bool system_wide, bool force);
		void handleWindowsMimeDatabase(bool uninstall, bool system_wide, bool force);

		void loadPlugins();
		bool isPluginOptsValid(PgModelerCliPlugin *plugin);

		/*! \brief Determines the execution order of the plugins by reading the
		 *  list of options provided. This method also returns the number of
		 *  plugins that are set to run custom CLI operations (standalone). */
		int definePluginsExecOrder(const attribs_map &opts);

		//! \brief Execute all activated plugins pre operations
		void runPluginsPreOperations();

		//! \brief Execute all activated plugins custom operations
		void runPluginsOperations();

		//! \brief Execute all activated plugins post operations
		void runPluginsPostOperations();

		int exec();

	public:
		//! \brief Option names constants
		static const QString AllChildren,
		Input,
		Output,
		InputDb,
		ExportToFile,
		ExportToPng,
		ExportToSvg,
		ExportToDbms,
		ExportToDict,
		ImportDb,
		NoIndex,
		Split,
		Markdown,
		DependenciesSql,
		ChildrenSql,
		GroupByType,
		GenDropScript,
		Diff,
		DropDatabase,
		DropObjects,
		NonTransactional,
		PgSqlVer,
		Help,
		ShowGrid,
		ShowDelimiters,
		PageByPage,
		OverrideBgColor,
		IgnoreDuplicates,
		IgnoreErrorCodes,
		ConnAlias,
		Host,
		Port,
		User,
		Passwd,
		InitialDb,
		Silent,
		ListConns,
		Simulate,
		FixModel,
		FixTries,
		ZoomFactor,
		UseTmpNames,
		DbmMimeType,
		Install,
		Uninstall,
		SystemWide,
		IgnoreImportErrors,
		ImportSystemObjs,
		ImportExtensionObjs,
		DebugMode,
		FilterObjects,
		MatchByName,
		ForceChildren,
		OnlyMatching,
		CommentsAsAliases,
		PartialDiff,
		Force,
		StartDate,
		EndDate,
		CompareDb,
		CompareFile,
		SaveDiff,
		ApplyDiff,
		NoDiffPreview,
		DropClusterObjs,
		RevokePermissions,
		DropMissingObjs,
		ForceDropColsConstrs,
		RenameDb,
		NoSequenceReuse,
		NoCascadeDrop,
		RecreateUnmod,
		ReplaceModified,
		ForceReCreateObjs,
		CreateConfigs,
		MissingOnly,
		IgnoreFaultyPlugins,
		ListPlugins,

		ConnOptions,
		TagExpr,
		EndTagExpr,
		AttributeExpr,
		ModelFixLog,

		MsgFileAssociated,
		MsgNoFileAssociation;

		PgModelerCliApp(int argc, char **argv);
		~PgModelerCliApp() override;

		//! \brief Shows the options menu
		void showMenu();

					 //! \brief Shows the version info
		void showVersionInfo();

		/*! \brief Prints to the stdout the provided text appending a \n on the string
		 * even if the silent mode is active. */
		void printText(const QString &txt = "");

		//! \brief Prints to the stdout only if the silent mode is not active
		void printMessage(const QString &txt = "");

		//! \brief Returns the options parsed when calling the application
		attribs_map getParsedOptions();

		//! \brief Returns a single parsed option value
		QString getParsedOptValue(const QString &opt);

		//! \brief Replaces the value of a single parsed option
		void setParsedOptValue(const QString &opt, const QString &value);

		void fixModel();
		void exportModel();
		void importDatabase();
		void diffModels();
		void updateMimeType();
		void createConfigurations();
		void listConnections();
		void listPlugins();

	private slots:
		void handleObjectAddition(BaseObject *);
		void updateProgress(int progress, QString msg, ObjectType = ObjectType::BaseObject);
		void printIgnoredError(QString err_cod, QString err_msg, QString cmd);
		void handleObjectRemoval(BaseObject *object);

	private:
		//! \brief Stores the long option names. The boolean indicates if the option accepts a value
		static std::map<QString, bool> long_opts;

		//! \brief Stores the short option names.
		static attribs_map short_opts;

		//! \brief Stores the accepted options by the different operations
		static std::map<QString, QStringList> accepted_opts;

		/* We make the main() a friend function of PgModelerCliApp just to
		 * allow it to call exec() that is a private function.
		 *
		 * This will avoid the exec() method to be called from within
		 * plugins which is not desirable. */
		friend int main(int, char **);
};

#endif
