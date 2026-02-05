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
#include "schemaparser.h"
#include "exception.h"

class SchemaParserTest: public QObject {
	Q_OBJECT

	private slots:
		void testSetAttributeWithLinebreak();
		void testConvertAttribsToXmlEntitiesInCondExpr();
		void testConvertAttribsToXmlEntitiesInQuotes();
		void testConvertAttribsToXmlEntitiesInSet();
		void testConvertAttribsToXmlEntities();
		void testExpressionEvaluationWithCasts();
		void testSetOperationInIf();
		void testSetOperationUnderIfEvaluatedAsFalse();
		void testConvertMetaCharsCorrectly();
		void testConvertEscapedCharsCorrectly();
		void testConvertEscapedPlainTextCharsInPlaintextExpr();
};


void SchemaParserTest::testSetAttributeWithLinebreak()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {attr} [foo\n\t\t\t\tbar]\n";
	buffer += "\n{attr}";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "foo\n\t\t\t\tbar", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testExpressionEvaluationWithCasts()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {ver} 10.0\n";
	buffer += "\n%if ({ver} >f \"9.3\") %then";
	buffer += "\n 10.0";
	buffer += "\n%else";
	buffer += "\n 9.3";
	buffer += "\n%end";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "10.0", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testSetOperationInIf()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {ver} 10.0\n";
	buffer += "\n%if ({ver} <=f \"9.3\") %then";
	buffer += "\n %set {variable1} [don't ]";
	buffer += "\n %set {variable2} [extract]";
	buffer += "\n%else";
	buffer += "\n %set {variable3} [extract ]";
	buffer += "\n %set {variable4} [in else]";
	buffer +=	"\n{variable3}{variable4}";
	buffer += "\n%end";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "extract in else", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testSetOperationUnderIfEvaluatedAsFalse()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	attribs["val"] = "9.0";
	buffer = "%if ({val} >=f \"9.5\") %then\n";
	buffer += "\t%set {ver} 10.0\n";
	buffer += "\t{ver}\n";
	buffer += "\t\n%if ({ver} <=f \"9.3\") %then";
	buffer += "\t\n [in if]";
	buffer += "\t\n%else";
	buffer += "\t\n [in else]";
	buffer += "\t\n%end";
	buffer += "\n%end\n";

	try
	{
		//QTextStream out(stdout);
		//out <<  buffer;
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertMetaCharsCorrectly()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {test} $hs $at $oc foo $cc\n";
	buffer += "{test}\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "#@{foo}", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertEscapedCharsCorrectly()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {test} \\# \\& \\@ \\{ foo \\} \\$ \\% \\[ \\] \\* \n";
	buffer += "{test}\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "#&@{foo}$%[]•", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertEscapedPlainTextCharsInPlaintextExpr()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "[foo bar test \\[ abc \\] foo]";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs) == "foo bar test [ abc ] foo", true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertAttribsToXmlEntities()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {attrib} [<obj name=\"test\"/>]\n";
	buffer+= "&{attrib}\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs), "&lt;obj name=&quot;test&quot;/&gt;");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertAttribsToXmlEntitiesInSet()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	buffer = "%set {attr1} [<p class=\"test\"/>]\n";
	buffer += "%set {attr2} &{attr1}\n";
	buffer += "{attr2}\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs), "&lt;p class=&quot;test&quot;/&gt;");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertAttribsToXmlEntitiesInQuotes()
{

	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	attribs["name"] = "\"<obj_name>\"";
	buffer += "[<column name=] \"&{name}\"\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs), "<column name=\"&quot;&lt;obj_name&gt;&quot;\"");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testConvertAttribsToXmlEntitiesInCondExpr()
{
	SchemaParser schparser;
	QString buffer;
	attribs_map attribs;

	attribs["attr1"] = "<p class=\"test\"/>";
	buffer += "%set {attr2} &{attr1}\n";
	buffer += "%if (&{attr2} == \"&lt;p class=&quot;test&quot;/&gt;\") %then\n";
	buffer += "[value=] \"&{attr1}\"\n";
	buffer += "%end\n";

	try
	{
		schparser.loadBuffer(buffer);
		QCOMPARE(schparser.getSourceCode(attribs), "value=\"&lt;p class=&quot;test&quot;/&gt;\"");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(SchemaParserTest)
#include "schemaparsertest.moc"
