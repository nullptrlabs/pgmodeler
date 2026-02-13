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
#include "databasemodel.h"
#include "pgmodelerunittest.h"

class ServerTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		ServerTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void codeGeneratedIsWellFormed();
		void modelReturnsDepsAndRefsForServer();
		void modelCreatesServerfromXMLandResultingXMLisEqual();
};

void ServerTest::codeGeneratedIsWellFormed()
{
	ForeignDataWrapper fdw;
	ForeignServer server;
	Role owner;
	QString sql_code =QString(
"-- object: server_test | type: SERVER -- \
-- DROP SERVER IF EXISTS server_test CASCADE; \
CREATE SERVER server_test \
TYPE 'dbtype' \
VERSION '1.0' \
FOREIGN DATA WRAPPER fdw \
OPTIONS (opt1 'value1',opt2 'value2'); \
-- ddl-end -- \
ALTER SERVER server_test OWNER TO postgres; \
-- ddl-end -- \
COMMENT ON SERVER server_test IS E'This is a test comment on server'; \
-- ddl-end -- ").simplified();

	QString xml_code =QString(
"<foreignserver name=\"server_test\" type=\"dbtype\" version=\"1.0\" options=\"opt1#value1*opt2#value2\"> \
<role name=\"postgres\"/> \
<comment><![CDATA[This is a test comment on server]]></comment> \
<foreigndatawrapper name=\"fdw\"/> \
</foreignserver>").replace("#", ForeignServer::OptionValueSeparator)
											 .replace("*", ForeignServer::OptionsSeparator).simplified();

	try
	{
		owner.setName("postgres");
		fdw.setName("fdw");

		server.setName("server_test");
		server.setOwner(&owner);
		server.setType("dbtype");
		server.setVersion("1.0");
		server.setOption("opt1", "value1");
		server.setOption("opt2", "value2");
		server.setComment("This is a test comment on server");
		server.setForeignDataWrapper(&fdw);

		QString res_sql_code = server.getSourceCode(SchemaParser::SqlCode).simplified();
		QCOMPARE(sql_code, res_sql_code);

		QString res_xml_code = server.getSourceCode(SchemaParser::XmlCode).simplified();
		QCOMPARE(xml_code, res_xml_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ServerTest::modelReturnsDepsAndRefsForServer()
{
	DatabaseModel model;
	Role owner;
	Schema public_sch;
	ForeignDataWrapper fdw;
	ForeignServer server;

	try
	{
		public_sch.setName("public");
		owner.setName("postgres");

		model.addSchema(&public_sch);
		model.addRole(&owner);

		fdw.setName("fdw");
		model.addForeignDataWrapper(&fdw);

		server.setName("server_test");
		server.setForeignDataWrapper(&fdw);
		model.addForeignServer(&server);

		std::vector<BaseObject *> refs, deps;
		//model.getObjectDependecies(&server, deps);
		deps = server.getDependencies();

		//model.getObjectReferences(&fdw, refs);
		refs = fdw.getReferences();

		model.removeForeignServer(&server);
		model.removeForeignDataWrapper(&fdw);
		model.removeSchema(&public_sch);
		model.removeRole(&owner);

		QVERIFY(deps.size() >= 2);
		QVERIFY(refs.size() == 1);
	}
	catch (Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void ServerTest::modelCreatesServerfromXMLandResultingXMLisEqual()
{
	DatabaseModel model;
	Role owner;
	Schema public_sch;
	ForeignDataWrapper fdw;
	ForeignServer *server = nullptr;
	QString xml_code, res_xml_code;

	try
	{
		public_sch.setName("public");
		owner.setName("postgres");

		model.addSchema(&public_sch);
		model.addRole(&owner);

		fdw.setName("fdw");
		model.addForeignDataWrapper(&fdw);

		xml_code=QString("<foreignserver name=\"server_test\" options=\"opt1#value1*opt2#value2\"> \
<role name=\"postgres\"/> \
<comment><![CDATA[This is a test comment on server]]></comment> \
<foreigndatawrapper name=\"fdw\"/> \
</foreignserver>").replace("#", ForeignDataWrapper::OptionValueSeparator)
											 .replace("*", ForeignDataWrapper::OptionsSeparator);

		model.getXMLParser()->loadXMLBuffer(xml_code);
		server = dynamic_cast<ForeignServer *>(model.createObject(ObjectType::ForeignServer));

		QVERIFY(server != nullptr);

		res_xml_code = server->getSourceCode(SchemaParser::XmlCode).simplified();
		xml_code = xml_code.simplified();

		model.removeForeignServer(server);
		model.removeForeignDataWrapper(&fdw);
		model.removeSchema(&public_sch);
		model.removeRole(&owner);

		if(server)
			delete server;

		QCOMPARE(xml_code, res_xml_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

QTEST_MAIN(ServerTest)
#include "servertest.moc"
