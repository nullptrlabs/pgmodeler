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

#include <QtTest/QtTest>
#include <QThread>
#include "connection.h"
#include "tools/sqlexecutionhelper.h"
#include "pgmodelerunittest.h"

#if defined(__has_include)
	#if __has_include(<sanitizer/lsan_interface.h>)
		#include <sanitizer/lsan_interface.h>
		#define HAS_LSAN_INTERFACE
	#endif
#endif

/*! \brief Regression tests for the leaks of the SQL execution tool (see SQLExecutionHelper and
SQLExecutionWidget).

The tests need a PostgreSQL server. It is configured through the environment variables below, when
PGMODELER_TEST_PG_HOST is not set (or the server can't be reached) the tests are skipped:

  PGMODELER_TEST_PG_HOST  host name or the directory of the unix socket (e.g. /run/postgresql)
  PGMODELER_TEST_PG_PORT  port (default 5432)
  PGMODELER_TEST_PG_DB    database (default postgres)
  PGMODELER_TEST_PG_USER  user (default is the current user)
  PGMODELER_TEST_PG_PWD   password (optional) */
class SQLExecLeakTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		SQLExecLeakTest() : PgModelerUnitTest(SCHEMASDIR){}

	private:
		Connection conn;
		bool server_available = false;

	private slots:
		void initTestCase();
		void failedCommandsDoNotLeakTheirResult();
		void resultModelIsMovedToTheMainThread();
		void resultModelIsDestroyedEvenAfterTheExecutionThreadFinished();
};

void SQLExecLeakTest::initTestCase()
{
	QString host = qEnvironmentVariable("PGMODELER_TEST_PG_HOST");

	if(host.isEmpty())
		return;

	conn.setConnectionParam(Connection::ParamServerFqdn, host);
	conn.setConnectionParam(Connection::ParamPort, qEnvironmentVariable("PGMODELER_TEST_PG_PORT", "5432"));
	conn.setConnectionParam(Connection::ParamDbName, qEnvironmentVariable("PGMODELER_TEST_PG_DB", "postgres"));
	conn.setConnectionParam(Connection::ParamUser, qEnvironmentVariable("PGMODELER_TEST_PG_USER", qEnvironmentVariable("USER")));
	conn.setConnectionParam(Connection::ParamPassword, qEnvironmentVariable("PGMODELER_TEST_PG_PWD"));

	try
	{
		conn.connect();
		server_available = conn.isStablished();
		conn.close();
	}
	catch(Exception &)
	{
		server_available = false;
	}
}

/*! \brief The PGresult of a command that fails was never freed: executeDMLCommand() threw before
handing it to the ResultSet. */
void SQLExecLeakTest::failedCommandsDoNotLeakTheirResult()
{
#ifndef HAS_LSAN_INTERFACE
	QSKIP("This test needs the leak sanitizer (USE_ADDR_SANITIZER=ON).");
#else
	if(!server_available)
		QSKIP("No PostgreSQL server available (see the environment variables in the description of the test).");

	// Leaks that exist before the test are not its business
	if(__lsan_do_recoverable_leak_check())
		QSKIP("There are leaks that were not caused by the test.");

	{
		Connection aux_conn(conn.getConnectionParams());
		int failures = 0;

		aux_conn.connect();

		for(int i = 0; i < 5; i++)
		{
			ResultSet res;

			try
			{
				aux_conn.executeDMLCommand("select * from table_that_does_not_exist_" + QString::number(i), res);
			}
			catch(Exception &)
			{
				failures++;
			}
		}

		aux_conn.close();
		QCOMPARE(failures, 5);
	}

	QVERIFY2(!__lsan_do_recoverable_leak_check(), "the results of the failed commands were leaked");
#endif
}

/*! \brief Runs a command in a thread the same way SQLExecutionWidget does and returns the model it creates
(nullptr if the execution fails). The thread is stopped and joined before returning, which
is the state it has when the widget is destroyed. */
static ResultSetModel *runCommandInThread(const Connection &conn, const QString &cmd, QThread &thread, SQLExecutionHelper &helper)
{
	QString error;

	helper.setConnection(conn);
	helper.setCommand(cmd);
	helper.moveToThread(&thread);

	QObject::connect(&thread, &QThread::started, &helper, &SQLExecutionHelper::executeCommand);
	QObject::connect(&helper, &SQLExecutionHelper::s_executionAborted, &thread, [&](Exception e){ error = e.getErrorMessage(); });

	QSignalSpy finished_spy(&helper, &SQLExecutionHelper::s_executionFinished),
			aborted_spy(&helper, &SQLExecutionHelper::s_executionAborted);

	thread.start();

	bool ok = finished_spy.wait(15000) || !aborted_spy.isEmpty();

	thread.quit();
	thread.wait();

	return (ok && aborted_spy.isEmpty() ? helper.getResultSetModel() : nullptr);
}

/*! \brief The model is created in the thread of the helper, but it's shown and destroyed by the widget, in the
main thread, so it must belong to the main thread after the execution. */
void SQLExecLeakTest::resultModelIsMovedToTheMainThread()
{
	if(!server_available)
		QSKIP("No PostgreSQL server available (see the environment variables in the description of the test).");

	QThread thread;
	SQLExecutionHelper helper;
	ResultSetModel *model = runCommandInThread(conn, "select * from generate_series(1, 3) as id", thread, helper);

	QVERIFY2(model != nullptr, "the command was not executed");
	QCOMPARE(model->rowCount(QModelIndex()), 3);
	QCOMPARE(model->thread(), qApp->thread());

	delete model;
}

/*! \brief The widget destroys the last model in its destructor with deleteLater(), after the execution
thread was finished. When the model belonged to that thread the deletion was posted to an event queue
that nobody processed anymore and the model (with all its data) was leaked. */
void SQLExecLeakTest::resultModelIsDestroyedEvenAfterTheExecutionThreadFinished()
{
	if(!server_available)
		QSKIP("No PostgreSQL server available (see the environment variables in the description of the test).");

	QThread thread;
	SQLExecutionHelper helper;
	ResultSetModel *model = runCommandInThread(conn, "select * from generate_series(1, 3) as id", thread, helper);

	QVERIFY2(model != nullptr, "the command was not executed");
	QVERIFY(!thread.isRunning());

	QSignalSpy destroyed_spy(model, &QObject::destroyed);

	model->deleteLater();
	QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

	QCOMPARE(destroyed_spy.count(), 1);
}

QTEST_MAIN(SQLExecLeakTest)
#include "sqlexecleaktest.moc"
