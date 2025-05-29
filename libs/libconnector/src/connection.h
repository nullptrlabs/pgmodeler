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
\ingroup libconnector
\class Connection
\brief This class encapsulates some connection manipulation functions implemented by the PostgreSQL libpq
\note <strong>Creation date:</strong> 13/07/2009
*/

#ifndef CONNECTION_H
#define CONNECTION_H

#include "resultset.h"
#include "attribsmap.h"
#include <QRegularExpression>
#include <QDateTime>

class __libconnector Connection {
	private:
		//! \brief Database connection descriptor
		PGconn *connection;

		//! \brief Parameters map used to generate the connection string
		attribs_map connection_params;

		//! \brief Formated connection string
		QString connection_str;

		/*! \brief Date-time value used to check the timeout between commands execution.
		This attribute is used to abort the command execution to avoid program crashes
		if the connection is closed by the server due to timeouts */
		QDateTime last_cmd_execution;

		/*! \brief Stores the maximum timeout (in seconds) between two command executions.
		A zero value means no timeout in this case the validateConnection() will not raise
		errors related to the exceeded timeout */
		unsigned cmd_exec_timeout;

		/*! \brief List of notices generated during the command execution
		The list is filled only if notice_enabled is true */
		static QStringList notices;

		//! \brief Generates the connection string based on the parameter map
		void generateConnectionString();

		/*! \brief This static method disable the notice messages when executing commands.
		By default all connections are created with notice disabled. To enable it the user
		must call Connection::setNoticeEnabled(). Note: already established connections 
		aren't affected when calling this method the user must disconnect then connect again
		to enable output. */
		static void disableNoticeOutput(void *, const PGresult *){}

		/*! \brief This function overrides the default notice handler of the connections and
		captures and stores all message in a string list that can be retrieved by the user
		for later usage */
		static void noticeProcessor(void *, const char *message);

		//! \brief Indicates if notices are enabled
		static bool notice_enabled;

		//! \brief Indicates if executed sql must be printed (stdout) [default is false]
		static bool print_sql;

		//! \brief Indicates if error silence is enabled
		static bool silence_conn_err;

		/*! \brief Indicates if the PostgreSQL version checking must be ignored during connection.
		 * This flag allows connecting (poorly and error-prone!) to older versions ( < 10). */
		static bool ignore_db_version;

		/*! \brief Indicates that the initial database configured in the connection can be automatically
		browsed after connect the server. This attribute is useful only in SQLTool */
		bool auto_browse_db,

		/*! \brief Indicates in which operations (diff, export, import, validation) the connection
		is used if none is explicitly specified by the user in the UI */
		default_for_oper[4];

		/*! \brief Validates the connection status (command exec. timeout and connection status) and
		raise errors in case of exceeded timeout or bad connection. This method is called prior any
		command execution */
		void validateConnectionStatus();

	public:
		//! \brief Constants used to reference the connections parameters
		static const QString ParamAlias;
		static const QString ParamApplicationName;
		static const QString ParamService;
		static const QString ParamServerFqdn;
		static const QString ParamServerIp;
		static const QString ParamPort;
		static const QString ParamDbName;
		static const QString ParamUser;
		static const QString ParamPassword;
		static const QString ParamConnTimeout;
		static const QString ParamOthers;
		static const QString ParamSslMode;
		static const QString ParamSslCert;
		static const QString ParamSslKey;
		static const QString ParamSslRootCert;
		static const QString ParamSslCrl;
		static const QString ParamKerberosServer;
		static const QString ParamLibGssapi;

		/*! \brief Special parameter that store the name of the role
		 *  to be switched in case of a successful connection */
		static const QString ParamSetRole;

		//! \brief Constants used to reference the connection ssl mode
		static const QString SslDisable;
		static const QString SslAllow;
		static const QString SslPrefer;
		static const QString SslRequire;
		static const QString SslCaVerify;
		static const QString SslFullVerify;

		//! \brief Constants used to reference the server info details (see getServerInfo())
		static const QString ServerVersion;
		static const QString ServerProtocol;
		static const QString ServerPid;

		//! \brief Constants used to reference the default usage in model operations (see setDefaultForOperation())
		enum ConnOperation: unsigned {
			OpValidation = 0,
			OpExport = 1,
			OpImport = 2,
			OpDiff = 3,
			OpNone = 4
		};

		Connection();
		Connection(const Connection &);
		Connection(const attribs_map &params);
		~Connection();

		/*! \brief Set the maximum timeout that a connectio can be idle (without running commands)
		Setting a zero value will cause not timemout checking */
		void setSQLExecutionTimout(unsigned timeout);

		//! \brief Toggles the notice output for connections. By default any notice are omitted
		static void setNoticeEnabled(bool value);

		//! \brief Returns the current state for notice output
		static bool isNoticeEnabled();

		//! \brief Toggles the executed sql output for connections. By default any sql are omitted
		static void setPrintSQL(bool value);

		//! \brief Returns the current state for sql output
		static bool isSQLPrinted();

		/*! \brief When calling this method with a true parameter any try to connect when the connection is already
		opened will raise exceptions. If calling the method using false the issue mentioned will be reported on
		stderr */
		static void setSilenceConnError(bool value);

		//! \brief Returns the current state for silence connection errors
		static bool isConnErrorSilenced();

		/*! \brief Ignores the PostgreSQL version checking during connection.
		 *  When false (the default behavior), when connecting to a server which version is < 10, an error
		 *  is raised. When true, the error is not raised, but the overall usage of the tool may be affected
		 *  since pgModeler is strictly tied to newer version. */
		static void setIgnoreDbVersion(bool ignore);

		//! \brief Indicates if the database version is ignored in the connection
		static bool isDbVersionIgnored();

		/*! \brief Sets one connection parameter. This method can only be called before
		 the connection to the database */
		void setConnectionParam(const QString &param, const QString &value);

		//! \brief Sets all the connection parameters at once
		void setConnectionParams(const attribs_map &params);

		//! \brief Set if the database configured on the connection is auto browseable when using the SQLTool manage database
		void setAutoBrowseDB(bool value);

		//! \brief Open the connection to the database.
		void connect();

		//! \brief Resets the database connection
		void reset();

		//! \brief Close the opened connection
		void close();

		//! \brief Request the cancel of the running commands on a opened connection
		void requestCancel();

		//! \brief Returns true when the server version is supported
		bool isServerSupported();

		//! \brief Returns the value of specified parameter name
		QString getConnectionParam(const QString &param);

		//! \brief Returns the full parameter map
		attribs_map getConnectionParams() const;

		//! \brief Returns a map containing some server's info
		attribs_map getServerInfo();

		//! \brief Returns the connection string used to connect to de database
		QString getConnectionString();

		/*! \brief Returns a string containing the following signature 'alias (host:port)' by default.
			If host_port_only is true the signature will not contain the alias being in the form 'host:port'
			If incl_db_name is true the database name will be prepended to the final signature 'dbname@host:port'
			If html_format s true the generated string will have the format <strong>db_name</strong>@<em>host:port (alias)</em>	*/
		QString getConnectionId(bool host_port_only = false, bool incl_db_name = false, bool html_format = false);

		/*! \brief Returns the DBMS version in format XX.YY[.ZZ]
		If major_only is true only XX.YY portion is returned */
		QString getPgSQLVersion(bool major_only=false);

		/*! Returns all notices/warnings produced by the command executions.
		This method will return an empty list if notices/warnings are disabled in the connections */
		static QStringList getNotices();

		/*! \brief Change the current database to the specified db name using the parameters from the current
		established connection causing the connection to be reset and moved to the new database.
		The effect of this is the same by type \c dbname on psql console. In case of errors the method will
		raise an exception and switch back to the previous database. */
		void switchToDatabase(const QString &dbname);

		//! \brief Returns if the connection is established
		bool isEstablished();

		//! \brief Returns if the connection is configured (has some attributes set)
		bool isConfigured();

		//! \brief Returns if the db configured in the connection can be automatically browsed in SQLTool
		bool isAutoBrowseDB();

		/*! \brief Executes a DML command on the server using the opened connection.
		 Its mandatory to specify the object to receive the returned resultset. */
		void executeDMLCommand(const QString &sql, ResultSet &result);

		/*! \brief Executes a DDL command on the server using the opened connection.
		 The user don't need to specify the resultset since the commando executed is intended
		 to be an data definition one  */
		void executeDDLCommand(const QString &sql);

		//! \brief Toggles the default status for the connect in the specified operation (OP_??? constants).
		void setDefaultForOperation(ConnOperation op_id, bool value);

		//! \brief Returns if the connection is the default for the specifed operation
		bool isDefaultForOperation(ConnOperation op_id);

		//! \brief Makes an copy between two connections
		void operator = (const Connection &conn);
};

#endif
