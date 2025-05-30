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

#include "sqlexecutionhelper.h"

SQLExecutionHelper::SQLExecutionHelper() : QObject(nullptr)
{
	cancelled = false;
	result_model = nullptr;
}

void SQLExecutionHelper::setConnection(Connection conn)
{
	connection = conn;
}

void SQLExecutionHelper::setCommand(const QString &cmd)
{
	command = cmd;
}

QString SQLExecutionHelper::getCommand()
{
	return command;
}

ResultSetModel *SQLExecutionHelper::getResultSetModel()
{
	return result_model;
}

bool SQLExecutionHelper::isCancelled()
{
	return cancelled;
}

QStringList SQLExecutionHelper::getNotices()
{
	return notices;
}

void SQLExecutionHelper::executeCommand()
{
	try
	{
		ResultSet res;
		Catalog catalog;
		Connection aux_conn = Connection(connection.getConnectionParams());

		catalog.setConnection(aux_conn);
		result_model = nullptr;
		cancelled = false;

		if(!connection.isEstablished())
		{
			connection.setNoticeEnabled(true);
			connection.connect();

			//The connection will break the execution if it keeps idle for one hour or more
			connection.setSQLExecutionTimout(3600);
		}

		connection.executeDMLCommand(command, res);
		notices = connection.getNotices();

		if(!res.isEmpty())
			result_model = new ResultSetModel(res, catalog);

		emit s_executionFinished(res.getTupleCount());
	}
	catch(Exception &e)
	{
		connection.close();
		emit s_executionAborted(e);
	}
}

void SQLExecutionHelper::cancelCommand()
{
	if(connection.isEstablished())
	{
		connection.requestCancel();
		cancelled = true;
	}
}
