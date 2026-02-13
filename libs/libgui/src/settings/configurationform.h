/*
# PostgreSQL Database Modeler (pgModeler)
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
\class ConfigurationForm
\brief Reunites in a single form all available configuration widgets.
*/

#ifndef CONFIGURATION_FORM_H
#define CONFIGURATION_FORM_H

#include "ui_configurationform.h"
#include "appearanceconfigwidget.h"
#include "generalconfigwidget.h"
#include "connectionsconfigwidget.h"
#include "pluginsconfigwidget.h"
#include "relationshipconfigwidget.h"
#include "snippetsconfigwidget.h"

class __libgui ConfigurationForm: public QDialog, public Ui::ConfigurationForm {
	Q_OBJECT

	private:	
		GeneralConfigWidget *general_conf;
		AppearanceConfigWidget *appearance_conf;
		ConnectionsConfigWidget *connections_conf;
		RelationshipConfigWidget *relationships_conf;
		SnippetsConfigWidget *snippets_conf;
		PluginsConfigWidget *plugins_conf;
		
		void hideEvent(QHideEvent *event);
		void showEvent(QShowEvent *);
		
	public:
		enum ConfWidgetsId {
			GeneralConfWgt,
			AppearanceConfWgt,
			RelationshipsConfWgt,
			ConnectionsConfWgt,
			SnippetsConfWgt,
			PluginsConfWgt
		};
		
		ConfigurationForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);
		virtual ~ConfigurationForm();
		
		BaseConfigWidget *getConfigurationWidget(unsigned idx);
		
	public slots:
		void applyConfiguration();
		void loadConfiguration();
		void reject();
		
	private slots:
		void restoreDefaults();
		void changeCurrentView();

	signals:
		void s_invalidateModelsRequested();
};

#endif
