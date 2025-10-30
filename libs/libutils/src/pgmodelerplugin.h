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
\ingroup libutils
\class PgModelerPlugin
\brief Implements the basic operations to create third party plugins based upon shared libraries.
*/

#ifndef PGMODELER_PLUGIN_H
#define PGMODELER_PLUGIN_H

#include "utilsglobal.h"
#include <QString>
#include "globalattributes.h"

class __libutils PgModelerPlugin {
	private:
		QString libname,

		plugin_name;

		//! \brief Defines the name of the library from where the plugin is being loaded
		void setLibraryName(const QString &lib);

		//! \brief Defines the name of plugin itself. In practical terms, it's the plugin's root folder name
		void setPluginName(const QString &name);

	public:
		PgModelerPlugin() = default;
		~PgModelerPlugin() = default;

		//! \brief Returns the name of the library of the plugin
		QString getLibraryName() const;

		//! \brief Returns the name of the plugin
		QString getPluginName() const;

		/*! \brief This method returns a full path to a file using a plugin's name as the base folder.
		 *
		 * The root_path is the full path to the root of plugin's resources.
		 *
		 * The parameters pack pth_elems is a list of 0 or more arguments that will be appended to the path formed by
		 * root_path + plugin_name in the same sequence as they appear.
		 *
		 * If root_path or plugin_name is empty, this method returns an empty (invalid) path.
		 *
		 * NOTE: always validate the path returned by this method since it's not restricted to the pgModeler installation root!
		 */
		template<typename ...args>
		static QString getPluginFilePath(const QString &plugin_name, const QString &root_path, args... pth_elems)
		{
			if(plugin_name.isEmpty() || root_path.isEmpty())
				return "";

			return GlobalAttributes::getFilePath("", root_path, plugin_name, pth_elems...);
		}

		/*! \brief This method returns a path of a template file (asset) that resided in a plugin directory which in turn
		 * is located in the plugin's root directory in a pgModeler installation.
		 *
		 * The parameters pack pth_elems is a list of 0 or more arguments that will be appended to the path formed by
		 * in the same sequence as they appear.
		 *
		 * For example, say we have a pgModeler installation in /opt/pgmodeler, and the plugins directory is located at /opt/pgmodeler/plugins.
		 * Now considering that plugin_name is "foo", we'll get the following results depending on what pth_elems specifis:
		 *
		 *  > pth_elems = "" --> /opt/pgmodeler/plugins/foo
		 *  > pth_elems = "bar" --> /opt/pgmodeler/plugins/foo/bar
		 *  > pth_elems = "bar", "test.conf" --> /opt/pgmodeler/plugins/foo/bar/test.conf
		 *  > pth_elems = "test.conf" --> /opt/pgmodeler/plugins/foo/test.conf
		 */
		template<typename ...args>
		static QString getPluginTmplFilePath(const QString &plugin_name, args ...pth_elems)
		{
			return getPluginFilePath(plugin_name, GlobalAttributes::getPluginsPath(), pth_elems...);
		}

		/*! \brief This method returns the full path to a file inside a subdirectory in the plugin's
		 * configuration directory inside the pgModeler's user settings directory.
		 *
		 * The parameters pack pth_elems is a list of 0 or more arguments that will be appended to the path formed by
		 * in the same sequence as they appear.
		 */
		template<typename ...args>
		QString getConfigFilePath(args... pth_elems) const
		{
			return getPluginFilePath(getPluginName(),
																GlobalAttributes::getConfigurationsPath() +
																GlobalAttributes::DirSeparator +
																GlobalAttributes::PluginsDir, pth_elems...);
		}

		/*! \brief This method calls the static version getPluginTmplFilePath(plugin_name, pth_elems)
		 * but automatically using this->plugin_name as the plugin name parameter.
		 */
		template<typename ...args>
		QString getTmplConfigFilePath(args... pth_elems) const
		{
			return getPluginTmplFilePath(plugin_name, pth_elems...);
		}

		friend class PluginsConfigWidget;
		friend class PgModelerCliApp;
};

#endif
