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

#include "connection.h"
#include <QTextStream>
#include "globalattributes.h"
#include "pgsqlversions.h"
#include "exception.h"

QStringList Connection::notices;

bool Connection::notice_enabled {false};
bool Connection::print_sql {false};
bool Connection::silence_conn_err {true};
bool Connection::ignore_db_version {false};

const QString	Connection::ParamAlias {"alias"};
const QString	Connection::ParamApplicationName {"application_name"};
const QString	Connection::ParamServerFqdn {"host"};
const QString	Connection::ParamServerIp {"hostaddr"};
const QString	Connection::ParamPort {"port"};
const QString	Connection::ParamDbName {"dbname"};
const QString	Connection::ParamUser {"user"};
const QString	Connection::ParamPassword {"password"};
const QString	Connection::ParamConnTimeout {"connect_timeout"};
const QString	Connection::ParamOthers {"options"};
const QString	Connection::ParamSslMode {"sslmode"};
const QString	Connection::ParamSslCert {"sslcert"};
const QString	Connection::ParamSslKey {"sslkey"};
const QString	Connection::ParamSslRootCert {"sslrootcert"};
const QString	Connection::ParamSslCrl {"sslcrl"};
const QString	Connection::ParamKerberosServer {"krbsrvname"};
const QString	Connection::ParamLibGssapi {"gsslib"};
const QString	Connection::ParamSetRole {"setrole"};
const QString	Connection::SslDisable {"disable"};
const QString	Connection::SslAllow {"allow"};
const QString	Connection::SslPrefer {"prefer"};
const QString	Connection::SslRequire {"require"};
const QString	Connection::SslCaVerify {"verify-ca"};
const QString	Connection::SslFullVerify {"verify-full"};
const QString	Connection::ServerVersion {"server-version"};
const QString	Connection::ServerProtocol {"server-protocol"};
const QString	Connection::ServerPid {"server-pid"};

Connection::Connection()
{
	connection=nullptr;
	auto_browse_db=false;	
	cmd_exec_timeout=0;

	for(unsigned idx=OpValidation; idx <= OpDiff; idx++)
		default_for_oper[idx]=false;

	setConnectionParam(ParamApplicationName, GlobalAttributes::PgModelerAppName);
}

Connection::Connection(const Connection &conn) : Connection()
{
   setConnectionParams(conn.getConnectionParams());
}

Connection::Connection(const attribs_map &params) : Connection()
{
	setConnectionParams(params);
}

Connection::~Connection()
{
	if(connection)
	{
		PQfinish(connection);
		connection=nullptr;
	}
}

void Connection::setSQLExecutionTimout(unsigned timeout)
{
	cmd_exec_timeout=timeout;
}

void Connection::setConnectionParam(const QString &param, const QString &value)
{
	//Regexp used to validate the host address
	QRegularExpression ip_regexp(QRegularExpression::anchoredPattern("[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+"));

	//Raise an error in case the param name is empty
	if(param.isEmpty())
		throw Exception(ErrorCode::AsgInvalidConnParameter, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	/* Set the value to the specified param on the map.
	One special case is treated here, if user use the parameter SERVER_FQDN and the value
	is a IP address, the method will assign the value to the SERVER_IP parameter */

	if(param==ParamServerFqdn && ip_regexp.match(value).hasMatch())
	{
		connection_params[Connection::ParamServerIp]=value;
		connection_params[Connection::ParamServerFqdn]="";
	}
	else
		connection_params[param]=value;

	//Updates the connection string
	generateConnectionString();
}

void Connection::setConnectionParams(const attribs_map &params)
{
	this->connection_params=params;
	generateConnectionString();
}

void Connection::setAutoBrowseDB(bool value)
{
	auto_browse_db=value;
}

void Connection::generateConnectionString()
{
	QString value, param_str = QString("%1=%2 ");

	//Scans the parameter map concatening the params (itr->first) / values (itr->second)
	connection_str.clear();

	for(auto &itr : connection_params)
	{
		if(itr.first == ParamAlias || itr.first == ParamSetRole)
			continue;

		value=itr.second;
		value.replace("\\","\\\\");
		value.replace("'","\\'");

		if(itr.first==ParamPassword && (value.contains(' ') || value.isEmpty()))
			value = QString("'%1'").arg(value);

		if(!value.isEmpty())
		{
			if(itr.first == ParamDbName)
				connection_str.prepend(param_str.arg(itr.first, value));
			else if(itr.first != ParamOthers)
				connection_str += param_str.arg(itr.first, value);
			else
				connection_str += value + " ";
		}
	}

	if(!connection_str.contains(ParamDbName) ||
		 (!connection_str.contains(ParamServerFqdn) &&
			!connection_str.contains(ParamServerIp)))
		connection_str.clear();
}

void Connection::noticeProcessor(void *, const char *message)
{
	notices.push_back(QString(message));
}

void Connection::validateConnectionStatus()
{
	if(cmd_exec_timeout > 0)
	{
		qint64 dt=(QDateTime::currentDateTime().toMSecsSinceEpoch() -
							 last_cmd_execution.toMSecsSinceEpoch())/1000;

		if(dt >= cmd_exec_timeout)
		{
			close();
			throw Exception(ErrorCode::ConnectionTimeout, __PRETTY_FUNCTION__, __FILE__, __LINE__);
		}
	}

	if(PQstatus(connection)==CONNECTION_BAD)
		throw Exception(Exception::getErrorMessage(ErrorCode::ConnectionBroken)
										.arg(connection_params[ParamServerFqdn].isEmpty() ? connection_params[ParamServerIp] : connection_params[ParamServerFqdn])
										.arg(connection_params[ParamPort]),
										ErrorCode::ConnectionBroken, __PRETTY_FUNCTION__, __FILE__, __LINE__);
}

void Connection::setNoticeEnabled(bool value)
{
	notice_enabled=value;
}

bool Connection::isNoticeEnabled()
{
	return notice_enabled;
}

void Connection::setPrintSQL(bool value)
{
	print_sql=value;
}

bool Connection::isSQLPrinted()
{
	return print_sql;
}

void Connection::setSilenceConnError(bool value)
{
	silence_conn_err=value;
}

bool Connection::isConnErrorSilenced()
{
	return silence_conn_err;
}

void Connection::setIgnoreDbVersion(bool ignore)
{
	ignore_db_version = ignore;
}

bool Connection::isDbVersionIgnored()
{
	return ignore_db_version;
}

void Connection::connect()
{
	/* If the connection string is not established indicates that the user
		is trying to connect without configuring connection parameters,
		thus an error is raised */
	if(connection_str.isEmpty())
		throw Exception(ErrorCode::ConnectionNotConfigured, __PRETTY_FUNCTION__, __FILE__, __LINE__);
	else if(connection)
	{
		if(!silence_conn_err)
			throw Exception(ErrorCode::ConnectionAlreadyStablished, __PRETTY_FUNCTION__, __FILE__, __LINE__);
		else
		{
			QTextStream err(stderr);
			err << QT_TR_NOOP("ERROR: trying to open an already stablished connection.") << Qt::endl
				<< "Conn. info: [ " << connection_str << "]" << Qt::endl;
			this->close();
		}
	}

	//Try to connect to the database
	connection = PQconnectdb(connection_str.toStdString().c_str());
	last_cmd_execution = QDateTime::currentDateTime();

	/* If the connection descriptor has not been allocated or if the connection state
		is CONNECTION_BAD it indicates that the connection was not successful */
	if(connection==nullptr || PQstatus(connection)==CONNECTION_BAD)
	{
		//Raise the error generated by the DBMS
		throw Exception(Exception::getErrorMessage(ErrorCode::ConnectionNotStablished)
						.arg(PQerrorMessage(connection)), ErrorCode::ConnectionNotStablished,
						__PRETTY_FUNCTION__, __FILE__, __LINE__);
	}

	notices.clear();

	if(!notice_enabled)
		//Completely disable notice/warnings in the connection
		PQsetNoticeReceiver(connection, disableNoticeOutput, nullptr);
	else
		//Enable the notice/warnings in the connection by pushing them into the list of generated notices
		PQsetNoticeProcessor(connection, noticeProcessor, nullptr);

	// Aborts the connection if an unsupported version of PostgreSQL is detected
	if(!ignore_db_version && !isServerSupported())
	{
		QString pgver = getPgSQLVersion(true);
		close();

		throw Exception(Exception::getErrorMessage(ErrorCode::UnsupportedPGVersion).arg(pgver),
										ErrorCode::UnsupportedPGVersion,
										__PRETTY_FUNCTION__, __FILE__, __LINE__);
	}

	// Switching to another role if the related parameter is set
	if(!connection_params[ParamSetRole].isEmpty())
		executeDDLCommand(QString("SET ROLE '%1'").arg(connection_params[ParamSetRole]));
}

void Connection::close()
{
	if(connection)
	{
		//Finalizes the connection if the status is OK
		if(PQstatus(connection) == CONNECTION_OK)
			PQfinish(connection);

		connection=nullptr;
		last_cmd_execution=QDateTime();
	}
}

void Connection::reset()
{
	//Raise an erro in case the user try to reset a not opened connection
	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	//Reinicia a conexão
	PQreset(connection);
}

QString Connection::getConnectionParam(const QString &param)
{
	return connection_params[param];
}

attribs_map Connection::getConnectionParams() const
{
	return connection_params;
}

attribs_map Connection::getServerInfo()
{
	attribs_map info;

	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection,__PRETTY_FUNCTION__,__FILE__,__LINE__);

	info[ServerPid]=QString::number(PQbackendPID(connection));
	info[ServerVersion]=getPgSQLVersion();
	info[ServerProtocol]=QString::number(PQprotocolVersion(connection));

	return info;
}

QString Connection::getConnectionString()
{
	return connection_str;
}

QString Connection::getConnectionId(bool host_port_only, bool incl_db_name, bool html_format)
{
	QString addr, db_name, port, conn_id;

	if(!isConfigured())
		return "";

	if(!connection_params[ParamServerFqdn].isEmpty())
		addr=connection_params[ParamServerFqdn];
	else
		addr=connection_params[ParamServerIp];

	if(!connection_params[ParamPort].isEmpty())
		port = QString(":%1").arg(connection_params[ParamPort]);

	if(incl_db_name)
		db_name = QString("%1@").arg(connection_params[ParamDbName]);

	if(host_port_only)
		conn_id = QString("%1%2%3").arg(db_name, addr, port);
	else
		conn_id = QString("%1%2 (%3%4)").arg(db_name, connection_params[ParamAlias], addr, port);

	if(html_format && incl_db_name)
	{
		conn_id.prepend("<strong>");
		conn_id.replace('@', "</strong>@<em>");
		conn_id.append("</em>");
	}

	return conn_id;
}

bool Connection::isStablished()
{
	return (connection != nullptr);
}

bool Connection::isConfigured()
{
	return !connection_str.isEmpty();
}

bool Connection::isAutoBrowseDB()
{
	return auto_browse_db;
}

QString  Connection::getPgSQLVersion(bool major_only)
{
	QString raw_ver, fmt_ver;

	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	raw_ver=QString("%1").arg(PQserverVersion(connection));

	//If the version is 10+
	if(raw_ver.contains(QRegularExpression("^((1)[0-9])(.)+")))
	{
		//New PostgreSQL 10+ versioning: 100001 means 10.1 (Major.Minor)
		fmt_ver=QString("%1.%2")
				.arg(raw_ver.mid(0,2))
				.arg(raw_ver.mid(3,1).toInt());

		if(!major_only)
			return QString("%1.%2").arg(raw_ver.mid(0,2)).arg(raw_ver.mid(4,2).toInt());
	}
	//For versions below or equal to 9.6
	else
	{
		fmt_ver=QString("%1.%2")
				.arg(raw_ver.mid(0,2).toInt()/10)
				.arg(raw_ver.mid(2,2).toInt()/10);

		if(!major_only)
			return QString("%1.%2").arg(fmt_ver).arg(raw_ver.mid(4,1));
	}

	return fmt_ver;
}

QStringList Connection::getNotices()
{
	return notices;
}

void Connection::executeDMLCommand(const QString &sql, ResultSet &result)
{
    PGresult *sql_res = nullptr;

	//Raise an error in case the user try to close a not opened connection
	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	validateConnectionStatus();
	notices.clear();

	//Alocates a new result to receive the resultset returned by the sql command
    sql_res = PQexec(connection, sql.toStdString().c_str());

	//Prints the SQL to stdout when the flag is active
	if(print_sql)
        qDebug().noquote() << "\n---\n" << sql;

	//Raise an error in case the command sql execution is not sucessful
	if(strlen(PQerrorMessage(connection))>0)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::SQLCommandNotExecuted)
						.arg(PQerrorMessage(connection)),
						ErrorCode::SQLCommandNotExecuted, __PRETTY_FUNCTION__, __FILE__, __LINE__, nullptr,
						QString(PQresultErrorField(sql_res, PG_DIAG_SQLSTATE)));
	}

    // Initializes the result set with the PG result instance.
    result.initResultSet(sql_res);
}

void Connection::executeDDLCommand(const QString &sql)
{
	PGresult *sql_res=nullptr;

	//Raise an error in case the user try to close a not opened connection
	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	validateConnectionStatus();
	notices.clear();
	sql_res=PQexec(connection, sql.toStdString().c_str());

	//Prints the SQL to stdout when the flag is active
	if(print_sql)
	{
		qDebug().noquote() << "\n---\n" << sql;
	}

	//Raise an error in case the command sql execution is not sucessful
	if(strlen(PQerrorMessage(connection)) > 0)
	{
		QString field = QString(PQresultErrorField(sql_res, PG_DIAG_SQLSTATE));

		PQclear(sql_res);

		throw Exception(Exception::getErrorMessage(ErrorCode::SQLCommandNotExecuted)
						.arg(PQerrorMessage(connection)),
						ErrorCode::SQLCommandNotExecuted, __PRETTY_FUNCTION__, __FILE__, __LINE__, nullptr,	field);
	}

	PQclear(sql_res);
}

void Connection::setDefaultForOperation(ConnOperation op_id, bool value)
{
	if(op_id > OpNone)
		throw Exception(ErrorCode::RefElementInvalidIndex,  __PRETTY_FUNCTION__, __FILE__, __LINE__);

	if(op_id != OpNone)
		default_for_oper[op_id]=value;
}

bool Connection::isDefaultForOperation(ConnOperation op_id)
{
	if(op_id > OpNone)
		throw Exception(ErrorCode::RefElementInvalidIndex,  __PRETTY_FUNCTION__, __FILE__, __LINE__);

	if(op_id==OpNone)
		return false;

	return default_for_oper[op_id];
}

void Connection::switchToDatabase(const QString &dbname)
{
	QString prev_dbname=connection_params[ParamDbName];

	try
	{
		//Closing the current connection if it's opened
		if(isStablished())
			close();

		//Change the database name and reconfigure the connection string
		connection_params[ParamDbName]=dbname;
		generateConnectionString();

		//Reopen connection
		connect();
	}
	catch(Exception &e)
	{
		connection_params[ParamDbName]=prev_dbname;
		connect();

		throw Exception(e.getErrorMessage(), e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void Connection::operator = (const Connection &conn)
{
	if(this->isStablished())
		this->close();

	this->auto_browse_db=conn.auto_browse_db;
	this->connection_params=conn.connection_params;
	this->connection_str=conn.connection_str;
	this->connection=nullptr;

	for(unsigned idx=OpValidation; idx <= OpDiff; idx++)
		default_for_oper[idx]=conn.default_for_oper[idx];
}

void Connection::requestCancel()
{
	if(!connection)
		throw Exception(ErrorCode::OprNotAllocatedConnection, __PRETTY_FUNCTION__, __FILE__, __LINE__);

	PGcancel *cancel = PQgetCancel(connection);

	if(cancel)
	{
		char errbuf[256] = "";
		PQcancel(cancel, errbuf, 256);
		PQfreeCancel(cancel);
	}
}

bool Connection::isServerSupported()
{
	try
	{
		QString version = getPgSQLVersion(true);
		return version.toDouble() >= PgSqlVersions::MinimumVersion.toDouble();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), __PRETTY_FUNCTION__, __FILE__, __LINE__, &e);
	}
}
