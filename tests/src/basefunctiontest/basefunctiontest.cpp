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
#include "globalattributes.h"
#include "pgmodelerunittest.h"
#include "defaultlanguages.h"

class BaseFunctionTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		BaseFunctionTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void doesntAddDuplicatedTransformType();
		void raiseExceptionWhenSettingInvalidConfigParam();
		void raiseExceptionWhenSettingEmptyConfigParamValue();

		void functionHasTransformTypesAndConfigParamsInSQL();
		void functionHasTransformTypesAndConfigParamsInXML();

		void procedureHasTransformTypesInSQL();
		void procedureHasTransformTypesInXML();

		void modelCreatesFunctionWithTransformTypesAndConfigParams();
		void modelCreatesProcedureWithTransformTypesAndConfigParams();
};

void BaseFunctionTest::doesntAddDuplicatedTransformType()
{
	Function func;
	std::vector<PgSqlType> types, types_aux;

	func.addTransformType(PgSqlType("integer"));
	func.addTransformType(PgSqlType("text"));
	types = func.getTransformTypes();

	func.addTransformType(PgSqlType("integer"));
	func.addTransformType(PgSqlType("text"));
	types_aux = func.getTransformTypes();

	QVERIFY(types.size() == types_aux.size());
}

void BaseFunctionTest::raiseExceptionWhenSettingInvalidConfigParam()
{
	Function func;

	try
	{
		func.setConfigurationParam("foo*test--param", "");
		QFAIL("No exception was raised even specifying an invalid configuration parameter!");
	}
	catch(Exception &e)
	{
		QCOMPARE(ErrorCode::InvConfigParameterName, e.getErrorCode());
	}
}

void BaseFunctionTest::raiseExceptionWhenSettingEmptyConfigParamValue()
{
	Function func;

	try
	{
		func.setConfigurationParam("test_param", "");
		QFAIL("No exception was raised even specifying an empty value to the configuration parameter!");
	}
	catch(Exception &e)
	{
		QCOMPARE(ErrorCode::EmptyConfigParameterValue, e.getErrorCode());
	}
}

void BaseFunctionTest::functionHasTransformTypesAndConfigParamsInSQL()
{
	try
	{
		Function func;
		Schema sch;
		Language lang;

		sch.BaseObject::setName("public");
		lang.BaseObject::setName(DefaultLanguages::Sql);

		func.setName("funct_test");
		func.setSchema(&sch);
		func.setLanguage(&lang);
		func.setReturnType(PgSqlType("integer"));
		func.addTransformType(PgSqlType("varchar"));
		func.addTransformType(PgSqlType("text"));
		func.addTransformType(PgSqlType("numeric", 1, 6, 2));
		func.setFunctionSource("return 0;");
		func.setConfigurationParam("search_path", "public,foo,bar");
		func.setConfigurationParam("log_statement_stats", "DEFAULT");

		QString expected_code =
				QString("-- object: public.funct_test | type: FUNCTION --\
 -- DROP FUNCTION IF EXISTS public.funct_test() CASCADE;\
 CREATE FUNCTION public.funct_test ()\
 RETURNS integer\
 LANGUAGE sql\
 TRANSFORM FOR TYPE varchar, FOR TYPE text, FOR TYPE numeric\
 VOLATILE \
 CALLED ON NULL INPUT\
 SECURITY INVOKER\
 PARALLEL UNSAFE\
 COST 100\
 SET log_statement_stats = DEFAULT\
 SET search_path = public,foo,bar\
 AS $$\
	return 0;\
 $$;\
 -- ddl-end --").simplified();

		QString generated_code = func.getSourceCode(SchemaParser::SqlCode).simplified();
		QCOMPARE(expected_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void BaseFunctionTest::functionHasTransformTypesAndConfigParamsInXML()
{
	try
	{
		Function func;
		Schema sch;
		Language lang;

		sch.BaseObject::setName("public");
		lang.BaseObject::setName(DefaultLanguages::Sql);

		func.setName("funct_test");
		func.setSchema(&sch);
		func.setLanguage(&lang);
		func.setReturnType(PgSqlType("integer"));
		func.addTransformType(PgSqlType("varchar"));
		func.addTransformType(PgSqlType("text"));
		func.addTransformType(PgSqlType("numeric", 1, 6, 2));
		func.setFunctionSource("return 0;");
		func.setConfigurationParam("search_path", "public,foo,bar");
		func.setConfigurationParam("log_statement_stats", "DEFAULT");

		QString expected_code =
				QString("<function name=\"funct_test\" window-func=\"false\" \
returns-setof=\"false\" behavior-type=\"CALLED ON NULL INPUT\" \
function-type=\"VOLATILE\" security-type=\"SECURITY INVOKER\" \
parallel-type=\"PARALLEL UNSAFE\" execution-cost=\"100\" \
row-amount=\"1000\"> \
<schema name=\"public\"/> \
<language name=\"sql\"/> \
<return-type> \
<type name=\"integer\" length=\"0\"/> \
</return-type> \
<transform-types names=\"varchar,text,numeric\"/> \
<configuration name=\"log_statement_stats\" value=\"DEFAULT\"/> \
<configuration name=\"search_path\" value=\"public,foo,bar\"/> \
<definition> <![CDATA[return 0;]]> </definition> \
</function>").simplified();

		QString generated_code = func.getSourceCode(SchemaParser::XmlCode).simplified();
		QCOMPARE(expected_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void BaseFunctionTest::procedureHasTransformTypesInSQL()
{
	try
	{
		Procedure proc;
		Schema sch;
		Language lang;

		sch.BaseObject::setName("public");
		lang.BaseObject::setName(DefaultLanguages::Sql);

		proc.setName("proc_test");
		proc.setSchema(&sch);
		proc.setLanguage(&lang);
		proc.addTransformType(PgSqlType("varchar"));
		proc.addTransformType(PgSqlType("text"));
		proc.addTransformType(PgSqlType("numeric", 1, 6, 2));
		proc.setFunctionSource("return 0;");
		proc.setConfigurationParam("search_path", "public,foo,bar");
		proc.setConfigurationParam("log_statement_stats", "DEFAULT");

		QString expected_code =
				QString("-- object: public.proc_test | type: PROCEDURE --\
 -- DROP PROCEDURE IF EXISTS public.proc_test() CASCADE;\
 CREATE PROCEDURE public.proc_test ()\
 LANGUAGE sql\
 TRANSFORM FOR TYPE varchar, FOR TYPE text, FOR TYPE numeric\
 SECURITY INVOKER\
 SET log_statement_stats = DEFAULT\
 SET search_path = public,foo,bar\
 AS $$\
	return 0;\
 $$;\
 -- ddl-end --").simplified();

		QString generated_code = proc.getSourceCode(SchemaParser::SqlCode).simplified();
		QCOMPARE(expected_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void BaseFunctionTest::procedureHasTransformTypesInXML()
{
	try
	{
		Procedure proc;
		Schema sch;
		Language lang;

		sch.BaseObject::setName("public");
		lang.BaseObject::setName(DefaultLanguages::Sql);

		proc.setName("proc_test");
		proc.setSchema(&sch);
		proc.setLanguage(&lang);
		proc.addTransformType(PgSqlType("varchar"));
		proc.addTransformType(PgSqlType("text"));
		proc.addTransformType(PgSqlType("numeric", 1, 6, 2));
		proc.setFunctionSource("return 0;");
		proc.setConfigurationParam("search_path", "public,foo,bar");
		proc.setConfigurationParam("log_statement_stats", "DEFAULT");

		QString expected_code =
				QString("<procedure name=\"proc_test\" \
security-type=\"SECURITY INVOKER\"> \
<schema name=\"public\"/> \
<language name=\"sql\"/> \
<transform-types names=\"varchar,text,numeric\"/> \
<configuration name=\"log_statement_stats\" value=\"DEFAULT\"/> \
<configuration name=\"search_path\" value=\"public,foo,bar\"/> \
<definition> <![CDATA[return 0;]]> </definition> \
</procedure>").simplified();

		QString generated_code = proc.getSourceCode(SchemaParser::XmlCode).simplified();
		QCOMPARE(expected_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void BaseFunctionTest::modelCreatesFunctionWithTransformTypesAndConfigParams()
{
	try
	{
		DatabaseModel dbmodel;
		Function *func = nullptr;
		QString xml_code =
				QString("<function name=\"funct_test\" window-func=\"false\" \
returns-setof=\"false\" behavior-type=\"CALLED ON NULL INPUT\" \
function-type=\"VOLATILE\" security-type=\"SECURITY INVOKER\" \
parallel-type=\"PARALLEL UNSAFE\" execution-cost=\"100\" \
row-amount=\"1000\"> \
<schema name=\"public\"/> \
<language name=\"sql\"/> \
<return-type> \
<type name=\"integer\" length=\"0\"/> \
</return-type> \
<transform-types names=\"varchar,text,numeric\"/> \
<configuration name=\"log_statement_stats\" value=\"DEFAULT\"/> \
<configuration name=\"search_path\" value=\"public,foo,bar\"/> \
<definition> <![CDATA[return 0;]]> </definition> \
</function>").simplified();

		dbmodel.createSystemObjects(true);
		dbmodel.getXMLParser()->setDTDFile(GlobalAttributes::getSchemasRootPath() +
																			 GlobalAttributes::DirSeparator +
																			 GlobalAttributes::XMLSchemaDir +
																			 GlobalAttributes::DirSeparator +
																			 "dtd" + GlobalAttributes::DirSeparator + "dbmodel.dtd",
																			 BaseObject::getSchemaName(ObjectType::Function));
		dbmodel.getXMLParser()->loadXMLBuffer(xml_code);
		func = dbmodel.createFunction();

		QVERIFY(nullptr != func);
		QString generated_code = func->getSourceCode(SchemaParser::XmlCode).simplified();
		QCOMPARE(xml_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void BaseFunctionTest::modelCreatesProcedureWithTransformTypesAndConfigParams()
{
	try
	{
	DatabaseModel dbmodel;
	Procedure *proc = nullptr;

	QString xml_code =
			QString("<procedure name=\"proc_test\" \
security-type=\"SECURITY INVOKER\"> \
<schema name=\"public\"/> \
<language name=\"sql\"/> \
<transform-types names=\"varchar,text,numeric\"/> \
<configuration name=\"log_statement_stats\" value=\"DEFAULT\"/> \
<configuration name=\"search_path\" value=\"public,foo,bar\"/> \
<definition> <![CDATA[return 0;]]> </definition> \
</procedure>").simplified();

		dbmodel.createSystemObjects(true);
		dbmodel.getXMLParser()->setDTDFile(GlobalAttributes::getSchemasRootPath() +
																			 GlobalAttributes::DirSeparator +
																			 GlobalAttributes::XMLSchemaDir +
																			 GlobalAttributes::DirSeparator +
																			 "dtd" + GlobalAttributes::DirSeparator + "dbmodel.dtd",
																			 BaseObject::getSchemaName(ObjectType::Procedure));
		dbmodel.getXMLParser()->loadXMLBuffer(xml_code);
		proc = dbmodel.createProcedure();

		QVERIFY(nullptr != proc);
		QString generated_code = proc->getSourceCode(SchemaParser::XmlCode).simplified();
		QCOMPARE(xml_code, generated_code);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(BaseFunctionTest)
#include "basefunctiontest.moc"
