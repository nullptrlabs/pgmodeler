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
\class SettingsWidget
\brief Reunites in a single form all available configuration widgets.
*/

#ifndef CONFIGURATION_WIDGET_H
#define CONFIGURATION_WIDGET_H

#include "ui_configurationwidget.h"
#include "appearanceconfigwidget.h"
#include "generalconfigwidget.h"
#include "connectionsconfigwidget.h"
#include "pluginsconfigwidget.h"
#include "relationshipconfigwidget.h"
#include "snippetsconfigwidget.h"

class __libgui ConfigurationWidget: public QWidget, public Ui::ConfigurationWidget {
	Q_OBJECT

	private:
		GeneralConfigWidget *general_conf;
		AppearanceConfigWidget *appearance_conf;
		ConnectionsConfigWidget *connections_conf;
		RelationshipConfigWidget *relationships_conf;
		SnippetsConfigWidget *snippets_conf;
		PluginsConfigWidget *plugins_conf;

		void hideEvent(QHideEvent *) override;
		void showEvent(QShowEvent *) override;

	public:
		enum ConfWidgetsId {
			GeneralConfWgt,
			AppearanceConfWgt,
			RelationshipsConfWgt,
			ConnectionsConfWgt,
			SnippetsConfWgt,
			PluginsConfWgt
		};

		ConfigurationWidget(QWidget * parent = nullptr);

		~ConfigurationWidget() override;

		int checkChangedConfiguration();
		
		template<class Widget, std::enable_if_t<std::is_base_of_v<BaseConfigWidget, Widget>, bool> = true>
		Widget *getConfigurationWidget()
		{
			return confs_stw->findChild<Widget *>();
		}

		template<class Widget, std::enable_if_t<std::is_base_of_v<BaseConfigWidget, Widget>, bool> = true>
		void discardConfiguration()
		{
			Widget * conf_wgt = getConfigurationWidget<Widget>();

			if(!conf_wgt ||
				 (conf_wgt && !conf_wgt->isConfigurationChanged()))
				return;

			try
			{
				conf_wgt->loadConfiguration();
			}
			catch(Exception &e)
			{
				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}

	public slots:
		void applyConfiguration();
		void loadConfiguration();

	private slots:
		void restoreDefaults();

		/*! \brief This method is discard any uncommited configuration changes
		 *  in all configuration sections. Different from restoreDefaults() that
		 *  restore the default configuration files, this method reloads the current
		 *  configuration file of each section */
		void __discardConfiguration();

	signals:
		void s_invalidateModelsRequested();
		void s_configurationChanged();
		void s_configurationReverted();
};

#endif
