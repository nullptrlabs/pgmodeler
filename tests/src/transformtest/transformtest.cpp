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

class TransformTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		TransformTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void throwsErrorOnInvalidType();
		void throwsErrorOnInvalidLanguage();
		void throwsErrorOnInvalidFunctionParamCount();
		void throwsErrorOnInvalidFunctionParamType();
		void throwsErrorOnInvalidFunctionReturnType();
		void generatesNameAndSignatureCorrectly();
		void generatesSQLCorrectly();
		void generatesXMLCorrectly();
		void modelReturnsTransformDepsRefsToFuncs();
		void modelCreatesTransformFromXML();
};

void TransformTest::throwsErrorOnInvalidType()
{
	try
	{
		Transform transf;
		transf.setType(PgSqlType(""));
		QFAIL("No error related to invalid type raised!");
	}
	catch(Exception &e)
	{
			QCOMPARE(e.getErrorCode(), ErrorCode::AsgInvalidTypeObject);
	}
}

void TransformTest::throwsErrorOnInvalidLanguage()
{
	try
	{
		Transform transf;
		transf.setLanguage(nullptr);
		QFAIL("No error related to invalid language raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgNotAllocatedLanguage);
	}
}

void TransformTest::throwsErrorOnInvalidFunctionParamCount()
{
	Transform transf;
	Function func;

	try
	{
		func.addParameter(Parameter("p1", PgSqlType("internal")));
		func.addParameter(Parameter("p2", PgSqlType("internal")));
		transf.setFunction(&func, Transform::ToSqlFunc);
		QFAIL("No error related to invalid function (param count) raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgFunctionInvalidParamCount);
	}
}

void TransformTest::throwsErrorOnInvalidFunctionParamType()
{
	Transform transf;
	Function func;

	try
	{
		func.addParameter(Parameter("p1", PgSqlType("varchar")));
		transf.setType(PgSqlType("varchar"));
		transf.setFunction(&func, Transform::ToSqlFunc);
		QFAIL("No error related to invalid function (param type) raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgFunctionInvalidParameters);
	}

	try
	{
		func.removeParameters();
		func.addParameter(Parameter("p1", PgSqlType("bigint")));
		transf.setFunction(&func, Transform::FromSqlFunc);
		QFAIL("No error related to invalid function (param type) raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgFunctionInvalidParameters);
	}
}

void TransformTest::throwsErrorOnInvalidFunctionReturnType()
{
	Transform transf;
	Function func;

	try
	{
		transf.setType(PgSqlType("varchar"));
		func.addParameter(Parameter("p1", PgSqlType("internal")));
		func.setReturnType(PgSqlType("bigint"));
		transf.setFunction(&func, Transform::ToSqlFunc);

		QFAIL("No error related to invalid function (return type) raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgFunctionInvalidReturnType);
	}

	try
	{
		func.removeParameters();
		func.addParameter(Parameter("p1", PgSqlType("internal")));
		func.setReturnType(PgSqlType("bigint"));
		transf.setType(PgSqlType("varchar"));
		transf.setFunction(&func, Transform::FromSqlFunc);

		QFAIL("No error related to invalid function (return type) raised!");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::AsgFunctionInvalidReturnType);
	}
}

void TransformTest::generatesNameAndSignatureCorrectly()
{
	Transform transf;
	Function to_sql_func, from_sql_func;
	Language lang;

	try
	{
		QCOMPARE(transf.getName(), QString("smallint_undefined"));
		QCOMPARE(transf.getSignature(), QString("FOR smallint LANGUAGE undefined"));

		lang.setName(DefaultLanguages::PlPgsql);
		transf.setLanguage(&lang);

		transf.setType(PgSqlType("varchar"));
		to_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		to_sql_func.setReturnType(PgSqlType("varchar"));
		transf.setFunction(&to_sql_func, Transform::ToSqlFunc);

		from_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		from_sql_func.setReturnType(PgSqlType("internal"));
		transf.setFunction(&from_sql_func, Transform::FromSqlFunc);

		QCOMPARE(transf.getName(), QString("varchar_plpgsql"));
		QCOMPARE(transf.getSignature(), QString("FOR varchar LANGUAGE plpgsql"));

		transf.setFunction(nullptr, Transform::ToSqlFunc);
		to_sql_func.setReturnType(PgSqlType::parseString("timestamp with time zone"));
		transf.setType(PgSqlType::parseString("timestamp with time zone"));
		transf.setFunction(&to_sql_func, Transform::ToSqlFunc);

		QCOMPARE(transf.getName(), QString("timestamp_with_time_zone_plpgsql"));
		QCOMPARE(transf.getSignature(), QString("FOR timestamp with time zone LANGUAGE plpgsql"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void TransformTest::generatesSQLCorrectly()
{
	Transform transf;
	Function to_sql_func, from_sql_func;
	Language lang;
	QString sql_code, expect_code;
	Schema schema;

	expect_code = QString("-- object: varchar_plpgsql | type: TRANSFORM --\n\
-- DROP TRANSFORM IF EXISTS FOR varchar LANGUAGE plpgsql CASCADE;\n\
CREATE TRANSFORM FOR varchar LANGUAGE plpgsql (\n\
	FROM SQL WITH FUNCTION public.from_sql_func(internal),\n\
	TO SQL WITH FUNCTION public.to_sql_func(internal)\n\
);\n\
-- ddl-end --\n\
COMMENT ON TRANSFORM FOR varchar LANGUAGE plpgsql IS E'This is a comment!';\n\
-- ddl-end --\n");

	try
	{
		schema.setName("public");
		lang.setName(DefaultLanguages::PlPgsql);

		transf.setLanguage(&lang);
		transf.setType(PgSqlType("varchar"));

		to_sql_func.setName("to_sql_func");
		to_sql_func.setSchema(&schema);
		to_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		to_sql_func.setReturnType(PgSqlType("varchar"));
		transf.setFunction(&to_sql_func, Transform::ToSqlFunc);

		from_sql_func.setName("from_sql_func");
		from_sql_func.setSchema(&schema);
		from_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		from_sql_func.setReturnType(PgSqlType("internal"));
		transf.setFunction(&from_sql_func, Transform::FromSqlFunc);

		transf.setComment("This is a comment!");

		sql_code = transf.getSourceCode(SchemaParser::SqlCode);
		//QTextStream out(stdout);
		//out <<  sql_code << Qt::endl;
		//out <<  expect_code <<  Qt::endl;
		//out << "---" <<  Qt::endl;
		QCOMPARE(sql_code.simplified(), expect_code.simplified());
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void TransformTest::generatesXMLCorrectly()
{
	Transform transf;
	Function to_sql_func, from_sql_func;
	Language lang;
	QString xml_code, expect_code;
	Schema schema;

	expect_code = QString("<transform>\n\
	<comment><![CDATA[This is a comment!]]></comment>\n\
	<appended-sql><![CDATA[-- APPENDED SQL --;]]></appended-sql>\n\
	<prepended-sql><![CDATA[-- PREPENDED SQL --;]]></prepended-sql>\n\
	<type name=\"varchar\" length=\"0\"/>\n\
	<language name=\"plpgsql\"/>\n\
	<function ref-type=\"fromsql\" signature=\"public.from_sql_func(internal)\"/>\n\
	<function ref-type=\"tosql\" signature=\"public.to_sql_func(internal)\"/>\n\
</transform>");

	try
	{
		schema.setName("public");
		lang.setName(DefaultLanguages::PlPgsql);

		transf.setLanguage(&lang);
		transf.setType(PgSqlType("varchar"));

		to_sql_func.setName("to_sql_func");
		to_sql_func.setSchema(&schema);
		to_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		to_sql_func.setReturnType(PgSqlType("varchar"));
		transf.setFunction(&to_sql_func, Transform::ToSqlFunc);

		from_sql_func.setName("from_sql_func");
		from_sql_func.setSchema(&schema);
		from_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		from_sql_func.setReturnType(PgSqlType("internal"));
		transf.setFunction(&from_sql_func, Transform::FromSqlFunc);

		transf.setComment("This is a comment!");
		transf.setAppendedSQL("-- APPENDED SQL --;");
		transf.setPrependedSQL("-- PREPENDED SQL --;");

		xml_code = transf.getSourceCode(SchemaParser::XmlCode);
		/*QTextStream out(stdout);
		out <<  xml_code << Qt::endl;
		out <<  expect_code <<  Qt::endl;
		out << "---" <<  Qt::endl;*/
		QCOMPARE(xml_code.simplified(), expect_code.simplified());
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void TransformTest::modelReturnsTransformDepsRefsToFuncs()
{
	Transform transf;
	Function to_sql_func, from_sql_func;
	Language lang;
	QString xml_code, expect_code;
	Schema schema;
	DatabaseModel dbmodel;
	std::vector<BaseObject *> refs, deps;
	unsigned to_sql_refs, from_sql_refs, transf_deps;
	Type custom_type;

	try
	{
		schema.setName("public");
		dbmodel.addObject(&schema);

		custom_type.setName("custom_type");
		custom_type.setSchema(&schema);
		custom_type.setConfiguration(Type::EnumerationType);
		custom_type.addEnumeration("enum1");
		custom_type.addEnumeration("enum2");
		dbmodel.addObject(&custom_type);

		lang.setName(DefaultLanguages::PlPgsql);
		dbmodel.addObject(&lang);

		transf.setLanguage(&lang);
		transf.setType(PgSqlType("public.custom_type"));

		to_sql_func.setName("to_sql_func");
		to_sql_func.setSchema(&schema);
		to_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		to_sql_func.setReturnType(PgSqlType("public.custom_type"));
		to_sql_func.setFunctionSource("begin; end");
		to_sql_func.setLanguage(&lang);
		transf.setFunction(&to_sql_func, Transform::ToSqlFunc);

		from_sql_func.setName("from_sql_func");
		from_sql_func.setSchema(&schema);
		from_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		from_sql_func.setReturnType(PgSqlType("internal"));
		from_sql_func.setFunctionSource("begin; end");
		from_sql_func.setLanguage(&lang);
		transf.setFunction(&from_sql_func, Transform::FromSqlFunc);

		dbmodel.addObject(&from_sql_func);
		dbmodel.addObject(&to_sql_func);
		dbmodel.addObject(&transf);

		//dbmodel.getObjectReferences(&from_sql_func, refs);
		refs = from_sql_func.getReferences();
		from_sql_refs = refs.size();

		//dbmodel.getObjectReferences(&to_sql_func, refs);
		refs = to_sql_func.getReferences();
		to_sql_refs = refs.size();

		//dbmodel.getObjectDependecies(&transf, deps);
		deps = transf.getDependencies();
		transf_deps = deps.size();

		dbmodel.removeObject(&transf);
		dbmodel.removeObject(&to_sql_func);
		dbmodel.removeObject(&from_sql_func);
		dbmodel.removeObject(&lang);
		dbmodel.removeObject(&custom_type);
		dbmodel.removeObject(&schema);

		QCOMPARE(from_sql_refs, static_cast<unsigned>(1));
		QCOMPARE(to_sql_refs, static_cast<unsigned>(1));
		QCOMPARE(transf_deps, static_cast<unsigned>(5));
	}
	catch(Exception &e)
	{
		dbmodel.removeObject(&transf);
		dbmodel.removeObject(&to_sql_func);
		dbmodel.removeObject(&from_sql_func);
		dbmodel.removeObject(&lang);
		dbmodel.removeObject(&custom_type);
		dbmodel.removeObject(&schema);

		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void TransformTest::modelCreatesTransformFromXML()
{
	QString xml_code = "<transform>\n\
	<comment><![CDATA[This is a comment test in transform!]]></comment>\n\
	<appended-sql><![CDATA[ /* Some appended SQL code */]]></appended-sql>\n\
	<prepended-sql><![CDATA[ /* Some appended SQL code */]]></prepended-sql>\n\
	<type name=\"text\" length=\"1\"/>\n\
	<language name=\"plpgsql\"/>\n\
	<function ref-type=\"fromsql\" signature=\"public.from_sql_func(internal)\"/>\n\
	<function ref-type=\"tosql\" signature=\"public.to_sql_func(internal)\"/>\n\
</transform>";
	Function to_sql_func, from_sql_func;
	Language lang;
	Schema schema;
	DatabaseModel dbmodel;
	Transform *transf = nullptr;

	try
	{
		schema.setName("public");
		dbmodel.addObject(&schema);

		lang.setName(DefaultLanguages::PlPgsql);
		dbmodel.addObject(&lang);

		to_sql_func.setName("to_sql_func");
		to_sql_func.setSchema(&schema);
		to_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		to_sql_func.setReturnType(PgSqlType("text"));
		to_sql_func.setFunctionSource("begin; end");
		to_sql_func.setLanguage(&lang);

		from_sql_func.setName("from_sql_func");
		from_sql_func.setSchema(&schema);
		from_sql_func.addParameter(Parameter("p1", PgSqlType("internal")));
		from_sql_func.setReturnType(PgSqlType("internal"));
		from_sql_func.setFunctionSource("begin; end");
		from_sql_func.setLanguage(&lang);

		dbmodel.addObject(&from_sql_func);
		dbmodel.addObject(&to_sql_func);

		dbmodel.getXMLParser()->setDTDFile(GlobalAttributes::getSchemasRootPath() +
																			 GlobalAttributes::DirSeparator +
																			 GlobalAttributes::XMLSchemaDir +
																			 GlobalAttributes::DirSeparator +
																			 "dtd" + GlobalAttributes::DirSeparator + "dbmodel.dtd",
																			 BaseObject::getSchemaName(ObjectType::Transform));
		dbmodel.getXMLParser()->loadXMLBuffer(xml_code);
		transf = dbmodel.createTransform();

		dbmodel.removeObject(&to_sql_func);
		dbmodel.removeObject(&from_sql_func);
		dbmodel.removeObject(&lang);
		dbmodel.removeObject(&schema);

		QVERIFY(nullptr != transf);

		/*QTextStream out(stdout);
		out << "Expected:" << Qt::endl;
		out << xml_code << Qt::endl;
		out << "---" << Qt::endl;
		out << "Result:" << Qt::endl;
		out << transf->getSourceCode(SchemaParser::XmlCode) << Qt::endl;*/

		QVERIFY(transf->getSourceCode(SchemaParser::XmlCode).simplified() == xml_code.simplified());
	}
	catch(Exception &e)
	{
		dbmodel.removeObject(&to_sql_func);
		dbmodel.removeObject(&from_sql_func);
		dbmodel.removeObject(&lang);
		dbmodel.removeObject(&schema);

		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(TransformTest)
#include "transformtest.moc"
