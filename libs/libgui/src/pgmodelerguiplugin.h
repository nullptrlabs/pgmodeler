/*
# Projeto: Modelador de Banco de Dados PostgreSQL (pgsqlDBM)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class PgModelerGuiPlugin
\brief Implements the basic operations to create third party plugins based upon shared libraries.
*/

#ifndef PGMODELER_GUI_PLUGIN_H
#define PGMODELER_GUI_PLUGIN_H

#include "widgets/modelwidget.h"
#include "baseform.h"
#include "pgmodelerplugin.h"

#ifdef PRIV_CODE_SYMBOLS
	#include "privpluginsns.h"
#endif

/*	The plug-ins in pgModeler must be within the "plugins" folder in their own
		directory and must have the following basic structure:

		 [PGMODELER_PLUGINS_DIR]/
					|
					+ - pluginA/
							 |
							 + ---- (lib)*(pluginA.)(so|dylib|dll) (library)
							 |
							 + ---- res/
											 |
											 +--- pluginA.qrc (resource file)
														|
														+--- /pluginA (context)
																 |
																 +--- pluginA.png (icon)

		> Library: it's the shared object that represents the plugin. The prefix (lib) and suffix (so|dylib|dll) are plataform dependent.
		> Resource file: it's the file where all icons used by the plug-in are registered.
		> Context: it's is the default namespace where the plug-ins are organized logically inside the .qrc file.
		> Icon: it is a PNG image that represents an icon of the plug-in

		> Plug-ins can have an optional "lang" subdir to store the UI translations for them. The translation files must be named
			in the form [plugin name].[lang code].qm, for instance, Brazilian Portuguese translation for "sampleplugin"
			would be: "sampleplugin.pt_BR.qm".

	Note: Plug-ins can have additional subdirectories but any reference to them must be made programatically by the plug-in author. */

// Making the MainWindow class of pgModeler be known by the plugin interface
class MainWindow;
class PluginsConfigWidget;

class __libgui PgModelerGuiPlugin: public PgModelerPlugin {
	private:
		//! \brief Stores all successfully loaded plugins
		static QList<PgModelerGuiPlugin *> reg_plugins;

		QLabel	*icon_lbl,
		*title_lbl,
		*author_lbl,
		*version_lbl,
		*description_lbl;

		/*! \brief Register a successfully loaded plugin so it can accessible globally
		 * via getPluginsActions() and getPluginsToolButtons() */
		static bool registerPlugin(PgModelerGuiPlugin *plugin);

	protected:
		BaseForm *plugin_info_frm;

		MainWindow *main_window;

		/*! \brief This method is executed right before the main window is created and can be used to perform
		 * plugin's initializations like UI modications and other miscellaneous initialization that can't be done
		 * in the plug-in constructor. Additionally, a main window instance must be passed to the plugin in order to facilitate
		 * customizations on the UI. The default implementation is to do nothing else than only expose the main window to the plugin. */
		virtual void initPlugin(MainWindow *main_window);

		/*! \brief Performs operations after the main window is completely loaded/initialized.
		 *  This is useful, for example, to perform signal/slot connections when all components of the
		 *  main window are properly initilized. */
		virtual void postInitPlugin();

		//! \brief Sets all the plugin's attributes at once.
		void configurePluginInfo(const QString &title, const QString &version, const QString &author, const QString &description);

	public:
		struct PluginWidgets {
			QToolButton *button;
			QWidget *widget;

			PluginWidgets(QToolButton *btn, QWidget *wgt)
			{
				button = btn;
				widget = wgt;
			}

			PluginWidgets() : PluginWidgets(nullptr, nullptr) {}
		};

		enum ActionId: unsigned {
			//! \brief References the action that will be put in the toolbar at main window
			ToolbarAction,

			//! \brief References the action that will be put in the model's context menu
			ModelAction,

			//! \brief References the action that will be put in the main window menu reserved for plugins settings
			ConfigAction,

			//! \brief References a general purpose action that must be handled by the plugin developer
			OtherAction
		};

		/*! \brief This enum controls where the model widget action of the plugin
		 *  should be placed in the model widget popup menu */
		enum MenuSectionId: unsigned {
			/*! \brief The action is placed at the default section reserved for plugin actions
			 *  in the popup menu. Currently, this section is at the very bottom of the menu */
			DefaultSection,

			//! \brief The action is placed at the top section of the popup menu
			TopSection,

			//! \brief The action is placed at the middle section of the popup menu
			MiddleSection,

			//! \brief The action is placed at the bottom section of the popup menu
			BottomSection
		};

		PgModelerGuiPlugin();

		virtual ~PgModelerGuiPlugin();

		//! \brief Returns the plugin's title, this same text is used as action's text on plugins toolbar.
		virtual QString getPluginTitle() = 0;

		//! \brief Returns the plugin's author
		virtual QString getPluginAuthor() = 0;

		//! \brief Returns the plugin's version
		virtual QString getPluginVersion() = 0;

		//! \brief Returns the plugin's complete description
		virtual QString getPluginDescription() = 0;

		//! \brief Shows the plugin's information dialog
		virtual void showPluginInfo();

		//! \brief Returns the action identified by act_id
		virtual QAction *getAction(ActionId act_id) = 0;

		/*! \brief Returns the menu section id where the ModelAction should be placed in
		 *  the model widget popup menu. */
		virtual MenuSectionId getMenuSection() = 0;

		//! \brief Returns the tool button inserted in database explorer instances
		virtual QToolButton *getToolButton() = 0;

		//! \brief Returns the path to a plugin icon in the plugin's qrc file
		QString getPluginIconPath(const QString &icon_name);

		//! \brief Returns an QIcon instance with the plugin icon in the plugin's qrc file
		QIcon getPluginIcon(const QString &icon_name);

		//! \brief Returns an QPixmap instance with the plugin icon in the plugin's qrc file
		QPixmap getPluginPixmap(const QString &icon_name);

		/*! \brief Returns an struct containing a toolbutton and a widget
		 *  that are installed in the areas in SQLExecutionWidget reserved for plugin
		 *  widgets. The toolbutton is inserted in the top area that holds other buttons.
		 *  The widget is inserted in the right area of the execution widget */
		virtual PluginWidgets createWidgets(QWidget *parent);

		/*! \brief Returns whether the selection in the current database model in main window
		 *  is valid according to the plugin's logic. This method is called in
		 *  ModelWidget::configurePluginsActionsMenu to enable/disable the model action related
		 *  to the plugin based on the current model widget's objects selection */
		virtual bool isSelectionValid();

		/*! \brief Returns the list of actions identified by act_id of all registered plugins
		 *  This method set each action data with a const reference to the parent plugin
		 *  so it can be used on specific operations when needed */
		static QList<QAction *> getPluginsActions(ActionId act_id);

		//! \brief Returns the list of toolbuttons all registered plugins
		static QList<QToolButton *> getPluginsToolButtons();

		//! \brief Returns the list of custom widgets of all registered plugins
		static QList<PluginWidgets> getPluginsWidgets(QWidget *parent);

		friend class PluginsConfigWidget;
};

/* Declares the class PgModelerPlugin as interface, this means that the class is a base
	 for plugin implementation. All plugin must inherit this class and use the Q_INTERFACE
	 directive in its declaration  */
Q_DECLARE_INTERFACE(PgModelerGuiPlugin, "PgModelerGuiPlugin")

#endif
