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
\class ConnectionsConfigWidget
\brief Implements the operations to manage database connections.
*/

#ifndef CONNECTIONS_CONFIG_WIDGET_H
#define CONNECTIONS_CONFIG_WIDGET_H

#include "ui_connectionsconfigwidget.h"
#include "baseconfigwidget.h"
#include "connection.h"

class __libgui ConnectionsConfigWidget: public BaseConfigWidget, public Ui::ConnectionsConfigWidget {
	Q_OBJECT

	private:	
		static const QString DefaultFor;

		bool one_time_conn_edit;
		
		//! \brief Stores the connections created by the user
		static std::vector<Connection *> connections;
		
		/*! \brief Stores the connections attributes. This map is used to write the connections.conf file
		as well to create the connections stored by the 'connections' vector */
		static std::map<QString, attribs_map> config_params;
		
		//! \brief Configures the passed connection setting it's attributes using the values from the form
		void configureConnection(Connection *conn, bool is_update);

		void hideEvent(QHideEvent *event) override;
		void showEvent(QShowEvent *event) override;
		void updateConnectionsCombo();
		
	public:
		ConnectionsConfigWidget(QWidget * parent = nullptr);
		~ConnectionsConfigWidget() override = default;

		void saveConfiguration() override;

		void loadConfiguration() override;

		/*! \brief Puts the connection editing form in one-time edit mode.
		 *  In this mode, the connection handling buttons are hidden and the
		 *  host/port fields are locked for edition if a value is specified
		 *  for them. Once the user accepts or rejects the editions the form
		 *  returns to the original operation mode */
		void setOneTimeEditMode(bool one_time_edit,
														const QString &conn_alias, const QString &dbname,
														const QString &host, int port,
														const QString &username, const QString &password);
		
		static std::map<QString, attribs_map> getConfigurationParams();
		
		//! \brief Fills the passed map with all the loaded connections.
		static void getConnections(std::map<QString, Connection *> &conns, bool inc_hosts=true);

		//! \brief Return a connection with the provided ID. If no connection is found the method returns nullptr
		static Connection *getConnection(const QString &conn_id);
		
		//! \brief Fills the passed combobox with all the loaded connections
		static void fillConnectionsComboBox(QComboBox *combo, bool incl_placeholder, Connection::ConnOperation check_def_for=Connection::OpNone);
		
		/*! \brief Opens a local instance of connection config dialog to permit user configures connections on-the-fly
		 *  Returns true when the connection were changed somehow, either by restoring the defaults, adding new connections
		 *  or removing current ones. The provided combobox is filled with new connections added during the process. */
		static bool openConnectionsConfiguration(QComboBox *combo, bool incl_placeholder);

		/*! \brief Opens a local instance of connection config dialog to permit user configures connections on-the-fly
		 *  Returns true when the connection were changed somehow, either by restoring the defaults, adding new connections
		 *  or removing current ones. Default values for connection alias, dbname, host, port and user can be provided */
		/* static bool openConnectionsConfiguration(const QString &conn_alias = "", const QString &dbname = "",
																						 const QString &host = "", int port = 5432,
																						 const QString &username = "", const QString &password = ""); */

		/*! \brief Opens a local instance of connection config dialog to permit user configures connections on-the-fly
		 *  Returns true when the connection were changed somehow, either by restoring the defaults, adding new connections
		 *  or removing current ones. Default values for connection alias, dbname, host, port and user can be provided. */
		static bool openConnectionsConfiguration(bool one_time_edit = false,
																						 const QString &conn_alias = "", const QString &dbname = "",
																						 const QString &host = "", int port = 5432,
																						 const QString &username = "", const QString &password = "");

		//! \brief Returns the first connection found which is defined as the default for the specified operation
		static Connection *getDefaultConnection(Connection::ConnOperation operation);
		
	protected:
		void destroyConnections();
		
	public slots:
		void restoreDefaults() override;

	private slots:
		void newConnection();
		void duplicateConnection();
		void handleConnection();
		void editConnection();
		void testConnection();
		void removeConnection();
		void enableCertificates();
		void enableConnectionTest();
		void applyConfiguration(void) override {}

		friend class ConfigurationWidget;
};

#endif
