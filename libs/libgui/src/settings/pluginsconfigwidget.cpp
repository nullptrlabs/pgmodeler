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

#include "pluginsconfigwidget.h"
#include "guiutilsns.h"
#include <QPluginLoader>

PluginsConfigWidget::PluginsConfigWidget(QWidget *parent) : BaseConfigWidget(parent)
{
	setupUi(this);

	root_dir_sel = new FileSelectorWidget(this);
	root_dir_sel->setToolTip(tr("pgModeler plugins directory"));
	root_dir_sel->setReadOnly(true);
	root_dir_sel->setDirectoryMode(true);
	root_dir_sel->setSelectedFile(GlobalAttributes::getPluginsPath());

	QVBoxLayout *vbox = new QVBoxLayout(plugins_root_gb);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(root_dir_sel);

	plugins_tab = new CustomTableWidget(CustomTableWidget::EditButton |
																			CustomTableWidget::ResizeColsButton, false, this);
	plugins_tab->setColumnCount(3);
	plugins_tab->setHeaderLabel(tr("Plugin"),0);
	plugins_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("plugins")),0);
	plugins_tab->setHeaderLabel(tr("Version"),1);
	plugins_tab->setHeaderLabel(tr("Library"), 2);
	plugins_tab->setSortingEnabled(true);

	connect(plugins_tab, &CustomTableWidget::s_rowEdited, this, &PluginsConfigWidget::showPluginInfo);

	vbox = new QVBoxLayout(loaded_plugins_gb);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(plugins_tab);
	loaded_plugins_gb->setLayout(vbox);
}

PluginsConfigWidget::~PluginsConfigWidget()
{
	while(!plugins.isEmpty())
	{
		delete plugins.last();
		plugins.removeLast();
	}
}

void PluginsConfigWidget::showPluginInfo(int idx)
{
	plugins[idx]->showPluginInfo();
}

void PluginsConfigWidget::loadConfiguration()
{
	std::vector<Exception> errors;
	QString lib,
			dir_plugins=GlobalAttributes::getPluginsPath() +
						GlobalAttributes::DirSeparator;
	QPluginLoader plugin_loader;
	QStringList dir_list;
	PgModelerGuiPlugin *plugin = nullptr;
	QFileInfo fi;
	unsigned row = 0;
	QJsonObject metadata;

	//The plugin loader must resolve all symbols otherwise return an error if some symbol is missing on library
	plugin_loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

	/* Configures an QDir instance to list only directories on the plugins/ subdir.
		If the user does not put the plugin in it's directory the file is ignored  */
	dir_list = QDir(dir_plugins, "*", QDir::Name, QDir::AllDirs | QDir::NoDotAndDotDot).entryList();

	/* Removing the "schemas" dir from the list since it is reserved to shared schema files
	 * for configuration file generation */
	dir_list.removeAll(GlobalAttributes::SchemasDir);

	for(auto &plugin_name : dir_list)
	{
		/* Configures the basic path to the library on the form:
		 [PLUGINS ROOT DIR]/[PLUGIN NAME]/lib[PLUGIN NAME].[EXTENSION] */
#ifdef Q_OS_WIN
		lib = dir_plugins + plugin_name +
					GlobalAttributes::DirSeparator  +
					plugin_name + ".dll";
#else
	#ifdef Q_OS_MACOS
			lib = dir_plugins + plugin_name +
						GlobalAttributes::DirSeparator  +
						"lib" + plugin_name + ".dylib";
	#else
			lib = dir_plugins + plugin_name +
						GlobalAttributes::DirSeparator  +
						"lib" + plugin_name + ".so";
	#endif
#endif

		//Try to load the library
		plugin_loader.setFileName(lib);
		metadata = plugin_loader.metaData();

		/* Ignores the plugin if it doesn't implement the correct interface,
		 * in this case PgModelerGuiPlugin */
		if(metadata["IID"] != "PgModelerGuiPlugin")
			continue;

		plugin = qobject_cast<PgModelerGuiPlugin *>(plugin_loader.instance());

		if(plugin)
		{
			//Inserts the loaded plugin on the vector
			fi.setFile(lib);
			plugin->setLibraryName(fi.fileName());
			plugin->setPluginName(plugin_name);
			plugins.append(plugin);

			plugins_tab->addRow();
			plugins_tab->setCellText(plugin->getPluginTitle(), plugins_tab->getRowCount()-1, 0);
			plugins_tab->setCellIcon(QIcon(plugin->getPluginIcon(plugin_name)), plugins_tab->getRowCount()-1, 0);
			plugins_tab->setCellText(plugin->getPluginVersion(), plugins_tab->getRowCount()-1, 1);
			plugins_tab->setCellText(fi.fileName(), plugins_tab->getRowCount()-1, 2);
			plugins_tab->setRowData(QVariant::fromValue<void *>(plugin), row++);
		}
		else
		{
			errors.push_back(Exception(Exception::getErrorMessage(ErrorCode::PluginNotLoaded)
																 .arg(plugin_name)
																 .arg(lib)
																 .arg(plugin_loader.errorString()),
																 ErrorCode::PluginNotLoaded, PGM_FUNC,PGM_FILE,PGM_LINE));
		}

		plugins_tab->clearSelection();
		plugins_tab->adjustColumnToContents(0);
	}

	if(!errors.empty())
		throw Exception(ErrorCode::PluginsNotLoaded,PGM_FUNC,PGM_FILE,PGM_LINE, errors);
}

void PluginsConfigWidget::initPlugins(MainWindow *main_window)
{
	QList<PgModelerGuiPlugin *> inv_plugins;
	std::vector<Exception> errors;
	int row_idx = -1;

	for(auto &plugin : plugins)
	{
		try
		{
			plugin->initPlugin(main_window);
			PgModelerGuiPlugin::registerPlugin(plugin);
		}
		catch(Exception &e)
		{
			inv_plugins.append(plugin);
			errors.push_back(e);
		}
	}

	// Erasing the plugins/actions related to the ones that failed to initialize
	while(!inv_plugins.empty())
	{
		row_idx = plugins_tab->getRowIndex(QVariant::fromValue<void *>(inv_plugins.last()));
		plugins.removeOne(inv_plugins.last());

		if(row_idx >= 0)
			plugins_tab->removeRow(row_idx);

		delete inv_plugins.last();
		inv_plugins.removeLast();
	}

	if(!errors.empty())
	{
		Messagebox::error(tr("One or more plug-ins failed to initialize and were discarded! Please, check the error stack for more details."),
											ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE, errors);
	}
}

void PluginsConfigWidget::postInitPlugins()
{
	std::vector<Exception> errors;

	for(auto &plugin : plugins)
	{
		try
		{
			plugin->postInitPlugin();
		}
		catch(Exception &e)
		{
			errors.push_back(e);
		}
	}

	if(!errors.empty())
	{
		Messagebox::error(tr("One or more plug-ins failed to perform post initialization operations! Please, check the error stack for more details."),
											ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE, errors);
	}
}
