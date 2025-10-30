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
\class PluginsConfigWidget
\brief Implements the operations to manage plugins.
*/

#ifndef PLUGINS_CONFIG_WIDGET_H
#define PLUGINS_CONFIG_WIDGET_H

#include "baseconfigwidget.h"
#include "ui_pluginsconfigwidget.h"
#include "widgets/customtablewidget.h"
#include "pgmodelerguiplugin.h"
#include "widgets/fileselectorwidget.h"

class __libgui PluginsConfigWidget: public BaseConfigWidget, public Ui::PluginsConfigWidget {
	Q_OBJECT

	private:
		//! \brief Loaded plugins
		QList<PgModelerGuiPlugin *> plugins;

		//! \brief Table used to show the loaded plugins
		CustomTableWidget *plugins_tab;

		FileSelectorWidget *root_dir_sel;

		/* Disabled methods */
		void applyConfiguration() override {}
		void saveConfiguration() override {}
		void restoreDefaults() override {}
		void addConfigurationParam(const QString &, const attribs_map &){}

	public:
		PluginsConfigWidget(QWidget *parent = nullptr);

		~PluginsConfigWidget() override;

		//! \brief Since plugins has its own configurations this method load all plugins instead
		void loadConfiguration() override;

		//! \brief Performs the initialization of all loaded plugins (see PgModelerPlugin::initPlugin())
		void initPlugins(MainWindow *main_window);

		//! \brief Execute the post initialization of all loaded plugins
		void postInitPlugins();

	private slots:
		void showPluginInfo(int idx);
};

#endif
