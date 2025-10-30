/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgsqlDBM)
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
\ingroup pgmodeler-cli
\class PgModelerCliPlugin
\brief Implements the basic operations to create third party plugins based upon shared libraries for the CLI application.
*/

#ifndef PGMODELER_CLI_PLUGIN_H
#define PGMODELER_CLI_PLUGIN_H

#include "cliglobal.h"
#include <QObject>
#include <QString>
#include "attribsmap.h"
#include "pgmodelerplugin.h"

#if defined(PRIVATE_PLUGINS_SYMBOLS)
	#include "privpluginsns.h"
#endif

class PgModelerCliApp;

class __libcli PgModelerCliPlugin: public PgModelerPlugin {
	protected:
		//! \brief Instance of cli application that the plugin has access
		PgModelerCliApp *cli_app;

		/*! \brief Initializes the plugin with an instance of the cli application.
		 *  This method can be overriden to perform extra initializations */
		virtual void initPlugin(PgModelerCliApp *app);

	public:
		//! \brief This enum is used to identify the CLI operations in which a plugin can be attached to.
		enum OperationId {
			Export, // Any export operation
			ExportToFile, // Export to SQL script
			ExportToPng, // Export to PNG image
			ExportToSvg, // Export to SVG file
			ExportToDbms, // Export directly to database server
			ExportToDict, // Export to data dictionary (.html)
			ImportDb,
			Diff,
			FixModel,
			CreateConfigs,
			CustomCliOp // The plugin runs a standalone operation, detached from any CLI operation
		};

		PgModelerCliPlugin();
		virtual ~PgModelerCliPlugin() = default;

		//! \brief Returns the plugin's title
		virtual QString getPluginTitle() = 0;

		//! \brief Returns the plugin's author
		virtual QString getPluginAuthor() = 0;

		//! \brief Returns the plugin's version
		virtual QString getPluginVersion() = 0;

		//! \brief Returns the plugin's complete description
		virtual QString getPluginDescription() = 0;

		/*! \brief Returns a list of short options used by the plugin
		 *  Short options must be formed of one dash and at most three
		 *  alphanumeric characters (e.g. -foo) */
		virtual attribs_map getShortOptions() = 0;

		/*! \brief Returns a list of long options used by the plugin
		 *  Long options must be formed of two dashes and at least one
		 *  alphanumeric character (e.g. --foo-bar-abc) */
		virtual std::map<QString, bool> getLongOptions() = 0;

		/*! \brief Returns a list of known/valid plugin's options that is used to trigger
		 * the runOperation() method. */
		virtual QStringList getOpModeOptions() = 0;

		/*! \brief Returns a list of plugin's options descriptions.
		 *  The key of the map must be the long option id and the
		 *  value the full option description */
		virtual attribs_map getOptsDescription() = 0;

		//! \brief Returns the id of the operation that causes the triggering of the plugin
		virtual OperationId getOperationId() = 0;

		//! \brief Returns if the provided option is one of those that triggers the plugin operation
		virtual bool isOpModeOption(const QString &opt);

		/*! \brief Returns the valid options of the plugin. By default this method returns
		 *  all the options returned by getLongOptions() */
		virtual QStringList getValidOptions();

		//! \brief This method is used to execute operations before the main plugin operation.
		virtual void runPreOperation() = 0;

		/*! \brief This method is used to execute main operations of the plugin.
		 * This method is called only when the plugin implements a custom CLI operation */
		virtual void runOperation() = 0;

		/*! \brief This method is used to execute operations after the main plugin operation
		 *  was executed successfuly */
		virtual void runPostOperation() = 0;

		friend class PgModelerCliApp;
};

/* Declares the class PgModelerCliPlugin as interface, this means that the class is a base
	 for plugin implementation. All plugin must inherit this class and use the Q_INTERFACE
	 directive in its declaration  */
Q_DECLARE_INTERFACE(PgModelerCliPlugin, "PgModelerCliPlugin")

#endif
