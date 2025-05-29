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
\ingroup libutils
\namespace GlobalAttributes
\brief Definition of GlobalAttributes class which stores a series of static strings constants used
 to reference the global configuration attributes of the software.
\note <strong>Creation date:</strong> 14/05/2010
*/

#ifndef GLOBAL_ATTRIBUTES_H
#define GLOBAL_ATTRIBUTES_H

#include "utilsglobal.h"
#include "attribsmap.h"
#include <QString>
#include <QStandardPaths>

class __libutils GlobalAttributes {
	private:
		/*! \brief Environment variables used to reference the pgModeler directories.
		 * These environment variables can be referenced in code using Env*Path constants.

		 PGMODELER_SCHEMAS_PATH   --> full path to the "schemas" folder  (SQL/XML generation schema files)
		 PGMODELER_CONF_PATH      --> full path to the "conf" folder    (user's own settings for pgModeler)
		 PGMODELER_TMPL_CONF_PATH --> full path to the "conf" folder    (used as template settings and copied to user's settings)
		 PGMODELER_LANG_PATH      --> full path to the "lang" folder    (ui translations)
		 PGMODELER_PLUGINS_PATH   --> full path to the "plugins" folder (where plugins are installed)
		 PGMODELER_TMP_PATH       --> full path to the "tmp" folder     (where temporary work are saved)
		 PGMODELER_SAMPLES_PATH   --> full path to the "samples" folder (contains sample dbm files)

		 Additional vars are used to specify where to find crash handler, command line interface
		 and main application.

		 PGMODELER_CH_PATH  --> Full path to pgmodeler-ch executable
		 PGMODELER_CLI_PATH --> Full path to pgmodeler-cli executable
		 PGMODELER_SE_PATH  --> Full path to pgmodeler-se executable
		 PGMODELER_PATH     --> Full path to pgmodeler executable */

		static QString SchemasRootPath;
		static QString LanguagesPath;
		static QString PluginsPath;
		static QString TemporaryPath;
		static QString SamplesPath;
		static QString TmplConfigurationPath;
		static QString ConfigurationsPath;
		static QString SQLHighlightConfPath;
		static QString XMLHighlightConfPath;
		static QString SchHighlightConfPath;
		static QString PatternHighlightConfPath;
		static QString PgModelerCHandlerPath;
		static QString PgModelerCLIPath;
		static QString PgModelerAppPath;
		static QString PgModelerSchemaEditorPath;
		static QString PgModelerBaseVersion;

		//! \brief Stores the custom paths retrieved either from pgmpaths.conf or from environment variables
		static attribs_map CustomPaths;

		/*! \brief Returns the current value for an environment variable that points to one of the assets/executables.
		 *  These variables can be defined either in the file pgmpaths.conf (key=value file) or from system's environment variables.
		 *
		 * If the current value points to an inexstent path then the function will return 'default_value' if it exists.
		 * Finally, if both current value and default values don't exist then the fallback value is returned even if it not
		 * exists in the filesystem.
		 *
		 * The precedence of value retriaval is the following:
		 *	pgmpaths.conf -> system's env vars -> default_value -> fallback_val */
		static QString getPathFromEnv(const QString &varname, const QString &default_val, const QString &fallback_val="");

		//! \brief Sets the config files paths variables that doesn't depend on a search path.
		static void setConfigFilesPaths();

		/*! \brief Configures the assets/executables paths loading from pgmpaths.conf or from environment variables.
		 * The search_path is from where the pgmpaths.conf must be loaded. */
		static void setCustomPaths(const QString &search_path);

		 //! \brief Sets the path in which the application should search for its internal folders (schemas, lang, conf, etc)
		static void setSearchPath(const QString &search_path);

		/*! \brief Returns the path to a file that is under a root path. Generally the root path is the full path to a folder that
		 * stores assets related to pgModeler (config files, schema files, etc) in its installation root.
		 *
		 * The parameter file_ext is the file extension that is appended to the requested file.
		 * The parameter root_path is the base path where the file name (and eventually subfolders) will be appended
		 * The parameter subfolder is a base subfolder where the file is located
		 * The paramter pack ...pth_elems is the last part of the complete file path. By using this, one can form a path with a
		 * variable number of subfolders. The last element of the parameter pack will be ALWAYS the file that is needed to be retrieved.
		 *
		 * Example:
		 *
		 * > Input: file_ext = .sch, root_path = /usr/local/pgmodeler/conf, subfolder = schemas, pth_elems = [ sql, table ]
		 * > Ouput: /usr/local/pgmodeler/conf/schemas/table.sch
		 *
		 * > Input: file_ext = [empty], root_path = ~/.config/pgmodeler, subfolder = tmp, pth_elems = [ tmpModel.dbm ]
		 * > Ouput: ~/.config/pgmodeler/tmp/tmpModel.dbm
		 *
		 * > Input: file_ext = [empty], root_path = ~/.config/pgmodeler, subfolder = [empty], pth_elems = [ empty ]
		 * > Ouput: ~/.config/pgmodeler
		 * */
		template<typename ...args>
		static QString getFilePath(const QString &file_ext, const QString &root_path, args... pth_elems)
		{
			QStringList path_elems = { pth_elems... };
			QString file, path = root_path;

			path_elems.removeAll("");

			if(path_elems.isEmpty())
				return path;

			// The last element of the parameter pack is the file to be retrieved
			file = path_elems.last();
			path_elems.removeLast();

			for(auto &folder : path_elems)
				path += DirSeparator + folder;

			return path + DirSeparator + file + (file_ext.isEmpty() ? "" : file_ext);
		}

	public:
		static const QString PgModelerVersion;
		static const QString PgModelerAppName;
		static const QString PgModelerOldAppName;

		static const QString PgModelerURI;
		static const QString PgModelerBuildNumber;
		static const QString PgModelerSite;
		static const QString PgModelerSupport;
		static const QString PgModelerSourceURL;
		static const QString PgModelerDownloadURL;
		static const QString PgModelerDonateURL;
		static const QString PgModelerUpdateCheckURL;

		static const QString EnvSchemasPath;
		static const QString EnvConfPath;
		static const QString EnvTmplConfPath;
		static const QString EnvLangPath;
		static const QString EnvPluginsPath;
		static const QString EnvTmpPath;
		static const QString EnvSamplesPath;
		static const QString EnvPgModelerChPath;
		static const QString EnvPgModelerCliPath;
		static const QString EnvPgModelerSePath;
		static const QString EnvPgModelerPath;

		static const QString PgmPathsConfFile;     //! \brief An ini-like (key=value) file that holds custom values for the pgModeler's enviroment variables
		static const QString BugReportEmail;
		static const QString BugReportFile;
		static const QString StacktraceFile;
		static const QString LastModelFile;

		static const QString DbModelExt;
		static const QString DbModelBkpExt;
		static const QString ObjMetadataExt;
		static const QString DirSeparator;
		static const QString ResourcesDir;				 //! \brief Directory name which holds the pgModeler's plug-ins resources directory (res)
		static const QString ConfigurationsDir;		 //! \brief Default name for the configurations directory
		static const QString DefaultConfsDir;			 //! \brief Default name for the default configurations directory
		static const QString ConfsBackupsDir;			 //! \brief Directory name which holds the pgModeler configuration backups
		static const QString SchemasDir;					 //! \brief Default name for the schemas directory
		static const QString SQLSchemaDir;				 //! \brief Default name for the sql schemas directory
		static const QString XMLSchemaDir;				 //! \brief Default name for the xml schemas directory
		static const QString CatalogSchemasDir;	   //! \brief Default name for the catalog schemas directory
		static const QString DataDictSchemaDir;	   //! \brief Default name for the data dictionary schemas root directory
		static const QString DataDictHtmlDir;		   //! \brief Default name for the data dictionary schemas directory (HTML format)
		static const QString DataDictMdDir;			   //! \brief Default name for the dictionary schemas directory (Markdown format)
		static const QString AlterSchemaDir;			 //! \brief Default name for the alter schemas directory
		static const QString LanguagesDir;				 //! \brief Default name for the translation files directory
		static const QString SamplesDir;					 //! \brief Default name for the samples database models directory
		static const QString PluginsDir;					 //! \brief Default name for the plug-ins directory
		static const QString SchemaExt;					   //! \brief Default extension for schema files
		static const QString ObjectDTDDir;				 //! \brief Default directory for dtd files
		static const QString ObjectDTDExt;				 //! \brief Default extension for dtd files
		static const QString RootDTD;						   //! \brief Root DTD of model xml files
		static const QString MetadataDTD;				   //! \brief Root DTD of objects metadata xml files
		static const QString ConfigurationExt;		 //! \brief Default extension for configuration files
		static const QString HighlightFileSuffix;  //! \brief Suffix of language highlight configuration files
		static const QString ThemesDir;					   //! \brief Default name for the ui style directory

		static const QString CodeHighlightConf;		 //! \brief Default name for the language highlight dtd
		static const QString AppearanceConf;			 //! \brief Default name for the appearance configuration file
		static const QString GeneralConf;					 //! \brief Default name for the general pgModeler configuration
		static const QString ConnectionsConf;			 //! \brief Default name for the DBMS connection configuration file
		static const QString RelationshipsConf;		 //! \brief Default name for the relationships configuration file
		static const QString SnippetsConf;				 //! \brief Default name for the code snippets configuration file
		static const QString DiffPresetsConf;			 //! \brief Default name for the diff presets configuration file

		static const QString SQLHighlightConf;		 //! \brief Configuration file for SQL language highlight
		static const QString XMLHighlightConf;		 //! \brief Configuration file for XML language highlight
		static const QString SchHighlightConf;		 //! \brief Configuration file for Schema micro-language highlight
		static const QString PatternHighlightConf; //! \brief Configuration file for name patterns highlight (relationship editing form)
		static const QString SQLHistoryConf;			 //! \brief Default name for the SQL commands history configuration file

		static const QString ExampleModel;		     //! \brief Default name for the sample model loaded on appearence configuration form
		static const QString UiStyleConf;		 	     //! \brief Configuration file ui style
		static const QString IconsMediumConf;      //! \brief Extra configuration file that defines medium icons size
		static const QString IconsSmallConf;	     //! \brief Extra configuration file that defines small icons size
		static const QString IconsBigConf;		     //! \brief Extra configuration file that defines big icons size

		static const QString FileDialogConf;       //! \brief Default name for the file used to save/restore QFileDialog last geometry

		/*! \brief Fusion is the default widget style for pgModeler. User can change this by calling
		the executable using -style option. This same style is applied to crash handler. */
		static const QString DefaultQtStyle;
		static const QString UiStyleOption;

		/*! \brief Performs the initialization of the global attributes by setting the
		 * application's search path (and all assets/executable paths based upon search_path).
		 *
		 * The apply_ui_factor setup the QT_SCALE_FACTOR enviroment variable by getting the custom ui factor
		 * from the file appearance.conf.
		 *
		 * ATTENTION: this method MUST be called before the instantiation of the Application
		 * class otherwise the paths will not be configured correctly. */
		static void init(const QString &search_path, bool apply_ui_factor);

		//! \brief Returns the path to the "schemas" folder
		static QString getSchemasRootPath();

		/*! \brief Returns the path to a schema file under "schemas" folder.
		 * Since this method only operates over schemas folder there's no need to provide a file with
		 * extension because the method already appends the extension .sch automatically. */
		template<typename ...args>
		static QString getSchemaFilePath(args... pth_elems)
		{
			return getFilePath(SchemaExt, SchemasRootPath, pth_elems...);
		}

		/*! \brief Returns the path to a data dictionary schema file under "schemas/datadict/(html|md)" folder(s).
		 * Since this method only operates over the mentioned path there's no need to provide a file with
		 * extension because the method already appends the extension .sch automatically.
		 * The parameter md_format, when true, returns a path to the markdown format data dictionary schemas
		 * otherwise a path to html data dictionary schemas is returned. */
		template<typename ...args>
		static QString getDictSchemaFilePath(bool md_format, args... pth_elems)
		{
			return getSchemaFilePath(GlobalAttributes::DataDictSchemaDir,
															 md_format ? GlobalAttributes::DataDictMdDir :
																					 GlobalAttributes::DataDictHtmlDir,
															 pth_elems...);
		}

		//! \brief Returns the path to the "tmp" folder in user's local storage
		static QString getTemporaryPath();

		//! \brief Returns the path to a temp file under "tmp" folder at user's local storage
		static QString getTemporaryFilePath(const QString &file);

		//! \brief Returns the path to the template "conf" folder in pgModeler's installation
		static QString getTmplConfigurationPath();

		/*! \brief Returns the path to the template file at template "conf" folder in pgModeler's installation
		 * This method will not append any extension to the file since this folder has several kinds of
		 * files inside it, so, the user must inform the file with its extension */
		template<typename ...args>
		static QString getTmplConfigurationFilePath(args... pth_elems)
		{
			return getFilePath("", TmplConfigurationPath, pth_elems...);
		}

		//! \brief Returns the path to the "conf" folder in user's local storage
		static QString getConfigurationsPath();

		/*! \brief Returns the path to a config file under "conf" folder at user's local storage.
		 * There's no need to specify the extension (.conf) since the method will automatically append it. */
		static QString getConfigurationFilePath(const QString &file);

		//! \brief Returns the path to the "samples" folder in pgModeler's installation
		static QString getSamplesPath();

		//! \brief Returns the path to the "lang" folder in pgModeler's installation
		static QString getLanguagesPath();

		//! \brief Returns the path to the "plugins" folder in pgModeler's installation
		static QString getPluginsPath();

		//! \brief Returns the path to the "sql-highlight.conf" file in user's local storage
		static QString getSQLHighlightConfPath();

		//! \brief Returns the path to the "xml-highlight.conf" file in user's local storage
		static QString getXMLHighlightConfPath();

		//! \brief Returns the path to the "sch-highlight.conf" file in user's local storage
		static QString getSchHighlightConfPath();

		//! \brief Returns the path to the "pattern-highlight.conf" file in user's local storage
		static QString getPatternHighlightConfPath();

		//! \brief Returns the path to the "pgmodeler-ch" (crash handler) executable
		static QString getPgModelerCHandlerPath();

		//! \brief Returns the path to the "pgmodeler-cli" (command line interface) executable
		static QString getPgModelerCLIPath();

		//! \brief Returns the path to the "pgmodeler" (main application) executable
		static QString getPgModelerAppPath();

		//! \brief Returns the path to the "pgmodeler-se" executable
		static QString getPgModelerSchemaEditorPath();

		/*! \brief Returns the version of the tool excluding any suffix
		 * appended to the version number (e.g., alpha, beta, snapshot, etc) */
		static QString getPgModelerBaseVersion();

		/*! \brief Returns the param_name value in the specified configuration file.
		 *  Returns empty string when the config parameter or the file does not exist. */
		static QString getConfigParamFromFile(const QString &param_name, const QString &conf_file);

		friend class PgModelerPlugin;
		friend class PgModelerCliPlugin;
};

#endif
