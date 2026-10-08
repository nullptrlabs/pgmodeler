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
#include <QTemporaryDir>
#include <QRegularExpression>
#include "schemaparser.h"
#include "attributes.h"
#include "exception.h"
#include "pgsqlversions.h"
#include "utilsns.h"
#include "pgmodelerunittest.h"

class SchemaParserTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	private:
		//! \brief Declares the columns shared by the data driven tests that parse a buffer (see checkBuffer())
		void addBufferColumns();

		//! \brief Parses the buffer of the current data row and compares the output (or the error) with the expected one
		void checkBuffer();

		QString writeSchemaFile(const QString &path, const QString &contents);

	public:
		SchemaParserTest() : PgModelerUnitTest(SCHEMASDIR) {}

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
		void testLoadFileCachesBufferAcrossInstances();
		void testLoadFileReportsIncludeFileAndLineOnCacheHit();
		void testAttributeNames_data();
		void testAttributeNames();
		void testConvertToXmlEntities_data();
		void testConvertToXmlEntities();
		void testIncludeInsertsTheIncludedFile();
		void testIncludeIsNotProcessedInsideOpenPlainText();
		void testIncludeErrors_data();
		void testIncludeErrors();
		void testErrorInsideIncludeReportsIncludeFileAndRelativeLine();
		void testErrorAfterIncludeReportsMainFileLine();
		void testGetSourceCodeFromFileSetsPgSqlVersion();
		void testGetSourceCodeByObjectNameUsesTheSchemasFolders();
		void testGetSourceCodeOfMissingSchemaRaises();
		void testSetPgSQLVersion();
		void testExtractAttributes();
		void testFlagsAreResetAfterASuccessfulParsing();
		void testParsingWithoutLoadedBufferReturnsEmptyText();
		void testCurrentLineAndColumnPointToTheError();
		void testLoadFileOfMissingFileRaises();
		void testCachedLoadProducesTheSameOutputAsTheFirstLoad();
		void testRelativeFileNamesAreResolvedAgainstTheCurrentDirectory();
		void testBuffersPreprocessor_data();
		void testBuffersPreprocessor();
		void testBuffersPlainTexts_data();
		void testBuffersPlainTexts();
		void testBuffersAttributes_data();
		void testBuffersAttributes();
		void testBuffersMetachars_data();
		void testBuffersMetachars();
		void testBuffersEscapedChars_data();
		void testBuffersEscapedChars();
		void testBuffersConditionals_data();
		void testBuffersConditionals();
		void testBuffersLogicalOperators_data();
		void testBuffersLogicalOperators();
		void testBuffersComparisons_data();
		void testBuffersComparisons();
		void testBuffersBranches_data();
		void testBuffersBranches();
		void testBuffersSetUnset_data();
		void testBuffersSetUnset();
		void testBuffersMisc_data();
		void testBuffersMisc();
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
		QCOMPARE(schparser.getSourceCode(attribs) == "#&@{foo}$%[]·", true);
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

void SchemaParserTest::testLoadFileCachesBufferAcrossInstances()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	QString file_path = dir.filePath("cache_test.sch");
	attribs_map attribs;
	QFile file(file_path);

	QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
	file.write("first-version\n");
	file.close();

	try
	{
		SchemaParser schparser1;
		schparser1.loadFile(file_path);
		QCOMPARE(schparser1.getSourceCode(attribs), QString("first-version"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	/* Changing the file on disk: if loadFile() is really reusing the buffer cached on the first
	 * load, a second instance loading the same path must still see the old content, since the
	 * cache is never invalidated by a change made after the process started */
	QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
	file.write("second-version\n");
	file.close();

	try
	{
		SchemaParser schparser2;
		schparser2.loadFile(file_path);
		QCOMPARE(schparser2.getSourceCode(attribs), QString("first-version"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testLoadFileReportsIncludeFileAndLineOnCacheHit()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	QString main_path = dir.filePath("main_include_test.sch"),
			included_path = dir.filePath("included_include_test.sch");
	attribs_map attribs;
	QFile included_file(included_path), main_file(main_path);

	QVERIFY(included_file.open(QFile::WriteOnly | QFile::Truncate));
	// The reference to an attribute that's never defined raises ErrorCode::UnkownAttribute
	included_file.write("line one\n{totally_unknown_attrib}\n");
	included_file.close();

	QVERIFY(main_file.open(QFile::WriteOnly | QFile::Truncate));
	main_file.write(QString("@include \"%1\"\n").arg(included_path).toUtf8());
	main_file.close();

	QString first_msg, second_msg;

	// First load of the main file: goes through the full parsing and @include resolution (cache miss)
	try
	{
		SchemaParser schparser1;
		schparser1.loadFile(main_path);
		schparser1.getSourceCode(attribs);
		QFAIL("Expected an UnkownAttribute exception to be raised but none was");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::UnkownAttribute);
		first_msg = e.getErrorMessage();
	}

	// Second load of the exact same main file by another instance: must hit the cache populated above
	try
	{
		SchemaParser schparser2;
		schparser2.loadFile(main_path);
		schparser2.getSourceCode(attribs);
		QFAIL("Expected an UnkownAttribute exception to be raised but none was");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::UnkownAttribute);
		second_msg = e.getErrorMessage();
	}

	// The error must point to the included file, not the top-level one nor a "[memory buffer]" placeholder
	QVERIFY(first_msg.contains(included_path));

	// The cache must not change the reported file/line/column in any way
	QCOMPARE(first_msg, second_msg);
}

void SchemaParserTest::addBufferColumns()
{
	QTest::addColumn<QString>("buffer");
	QTest::addColumn<QStringList>("attribs");
	QTest::addColumn<bool>("ignore_unk");
	QTest::addColumn<bool>("ignore_empty");
	QTest::addColumn<QString>("expected");
	QTest::addColumn<int>("error_code"); // -1: no error expected
	QTest::addColumn<int>("error_line"); // -1: position not checked
	QTest::addColumn<int>("error_column");
}

void SchemaParserTest::checkBuffer()
{
	QFETCH(QString, buffer);
	QFETCH(QStringList, attribs);
	QFETCH(bool, ignore_unk);
	QFETCH(bool, ignore_empty);
	QFETCH(QString, expected);
	QFETCH(int, error_code);
	QFETCH(int, error_line);
	QFETCH(int, error_column);

	SchemaParser schparser;
	attribs_map attrs;
	QString output;

	for(auto &attr : attribs)
	{
		int pos = attr.indexOf('=');
		attrs[attr.left(pos)] = attr.mid(pos + 1);
	}

	try
	{
		// Parsing twice with the same instance to validate that a successful run leaves the parser reusable
		for(int i = 0; i < 2; i++)
		{
			schparser.ignoreUnkownAttributes(ignore_unk);
			schparser.ignoreEmptyAttributes(ignore_empty);
			schparser.loadBuffer(buffer);
			output = schparser.getSourceCode(attrs);

			if(error_code >= 0)
				QFAIL(qPrintable(QString("Expected the error code %1 but the parsing succeeded with `%2'").arg(error_code).arg(output)));

			QCOMPARE(output, expected);
		}
	}
	catch(Exception &e)
	{
		if(error_code < 0)
			QFAIL(e.getExceptionsText().toStdString().c_str());

		QCOMPARE(static_cast<int>(e.getErrorCode()), error_code);

		if(error_line >= 0)
		{
			QRegularExpressionMatch match = QRegularExpression("line (\\d+), column (\\d+)").match(e.getErrorMessage());
			QVERIFY2(match.hasMatch(), qPrintable(e.getErrorMessage()));
			QCOMPARE(match.captured(1).toInt(), error_line);
			QCOMPARE(match.captured(2).toInt(), error_column);
		}

		// After an error the instance must be usable again once restarted
		schparser.restartParser();
		schparser.ignoreUnkownAttributes(false);
		schparser.ignoreEmptyAttributes(false);
		schparser.loadBuffer("ok");
		QCOMPARE(schparser.getSourceCode(attribs_map()), QString("ok"));
	}
}

QString SchemaParserTest::writeSchemaFile(const QString &path, const QString &contents)
{
	QFile file(path);

	if(file.open(QFile::WriteOnly | QFile::Truncate))
	{
		file.write(contents.toUtf8());
		file.close();
	}

	return path;
}

void SchemaParserTest::testAttributeNames_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<bool>("valid");

	QTest::newRow("lowercase") << "name" << true;
	QTest::newRow("uppercase") << "NAME" << true;
	QTest::newRow("single letter") << "a" << true;
	QTest::newRow("hyphen, digits and underscore") << "a-b_c1-2_3" << true;
	QTest::newRow("ends with hyphen") << "a-" << true;
	QTest::newRow("starts with digit") << "1a" << false;
	QTest::newRow("starts with hyphen") << "-a" << false;
	QTest::newRow("starts with underscore") << "_a" << false;
	QTest::newRow("dot") << "a.b" << false;
	QTest::newRow("accented letter") << QString::fromUtf8("a\u00e9") << false;
	QTest::newRow("arabic-indic digit") << QString::fromUtf8("a\u0663") << false;
	// These two only matched the former case insensitive regular expression because of the Unicode case folding
	QTest::newRow("kelvin sign") << QString::fromUtf8("\u212Abc") << false;
	QTest::newRow("kelvin sign after the first char") << QString::fromUtf8("a\u212A") << false;
	QTest::newRow("long s") << QString::fromUtf8("\u017Fabc") << false;
	QTest::newRow("long name") << QString("a").repeated(300) << true;
	QTest::newRow("long invalid name") << QString("a").repeated(300) + "!" << false;
}

void SchemaParserTest::testAttributeNames()
{
	QFETCH(QString, name);
	QFETCH(bool, valid);

	SchemaParser schparser;
	attribs_map attribs { { name, "value" } };

	try
	{
		schparser.loadBuffer(QString("{%1}").arg(name));
		QString output = schparser.getSourceCode(attribs);

		QVERIFY2(valid, "An invalid attribute name was accepted");
		QCOMPARE(output, QString("value"));
	}
	catch(Exception &e)
	{
		QVERIFY2(!valid, qPrintable(e.getErrorMessage()));
		QVERIFY(e.getErrorCode() == ErrorCode::InvalidAttribute || e.getErrorCode() == ErrorCode::InvalidSyntax);
		schparser.restartParser();
	}

	// The same rule applies to the name defined by %set
	try
	{
		schparser.loadBuffer(QString("%set {%1} v").arg(name));
		schparser.getSourceCode(attribs_map());
		QVERIFY2(valid, "An invalid attribute name was accepted by %set");
	}
	catch(Exception &e)
	{
		QVERIFY2(!valid, qPrintable(e.getErrorMessage()));
		schparser.restartParser();
	}
}

void SchemaParserTest::testConvertToXmlEntities_data()
{
	QTest::addColumn<QString>("value");
	QTest::addColumn<QString>("expected");

	QTest::newRow("empty") << "" << "";
	QTest::newRow("plain") << "public.my_table" << "public.my_table";
	QTest::newRow("non ascii") << "açaí · ção" << "açaí · ção";
	QTest::newRow("backslash is not special") << "a\\<b" << "a\\&lt;b";
	QTest::newRow("ampersand") << "a & b" << "a &amp; b";
	QTest::newRow("less than") << "a < b" << "a &lt; b";
	QTest::newRow("greater than") << "a > b" << "a &gt; b";
	QTest::newRow("double quotes") << "say \"hi\"" << "say &quot;hi&quot;";
	QTest::newRow("single quotes are kept") << "it's" << "it's";
	QTest::newRow("all of them") << "<a href=\"x\">&</a>" << "&lt;a href=&quot;x&quot;&gt;&amp;&lt;/a&gt;";
	QTest::newRow("only special chars") << "&<>\"" << "&amp;&lt;&gt;&quot;";
	QTest::newRow("entity amp keeps bare ampersand") << "a &amp; b & c" << "a &amp; b & c";
	QTest::newRow("entity quot keeps bare ampersand") << "&quot;x&quot; & \"" << "&quot;x&quot; & &quot;";
	QTest::newRow("entity lt/gt/apos keep bare ampersand") << "&lt; &gt; &apos; & >" << "&lt; &gt; &apos; & &gt;";
	QTest::newRow("entity with other specials") << "&amp; <" << "&amp; &lt;";
	QTest::newRow("unknown entity gets escaped") << "&nbsp;" << "&amp;nbsp;";
}

void SchemaParserTest::testConvertToXmlEntities()
{
	QFETCH(QString, value);
	QFETCH(QString, expected);

	QCOMPARE(UtilsNs::convertToXmlEntities(value), expected);
}

void SchemaParserTest::testIncludeInsertsTheIncludedFile()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	writeSchemaFile(dir.filePath("inc_a.sch"), "# comment in included file\n%set {from-inc} [INC] # tail\n");
	writeSchemaFile(dir.filePath("inc_b.sch"), "[B]\n");
	QString main = writeSchemaFile(dir.filePath("inc_main.sch"),
																 "@include \"inc_a\"\n"
																 "  @include   \"inc_b.sch\"  \n"
																 "main {from-inc}\n");
	attribs_map attribs;
	SchemaParser schparser;

	try
	{
		schparser.loadFile(main);
		QCOMPARE(schparser.getSourceCode(attribs), QString("BmainINC"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testIncludeIsNotProcessedInsideOpenPlainText()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	writeSchemaFile(dir.filePath("inc_x.sch"), "X\n");
	attribs_map attribs;
	SchemaParser schparser;

	try
	{
		schparser.setSearchPath(dir.path());
		schparser.loadBuffer("[a\n@include \"inc_x\"\nb]\n@include \"inc_x\"\n");
		QCOMPARE(schparser.getSourceCode(attribs), QString("a\n@include \"inc_x\"\nbX"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testIncludeErrors_data()
{
	QTest::addColumn<QString>("main_contents");
	QTest::addColumn<int>("error_code");
	QTest::addColumn<int>("error_line");

	QTest::newRow("file not found") << "a\n@include \"does_not_exist\"\n" << static_cast<int>(ErrorCode::InvalidInclude) << 2;
	QTest::newRow("missing quotes") << "@include foo\n" << static_cast<int>(ErrorCode::InvalidSyntax) << 1;
	QTest::newRow("unclosed quote") << "x\n@include \"foo\n" << static_cast<int>(ErrorCode::InvalidSyntax) << 2;
	QTest::newRow("trailing garbage") << "@include \"inc_ok\" extra\n" << static_cast<int>(ErrorCode::InvalidSyntax) << 1;
	QTest::newRow("nested include") << "@include \"inc_nested\"\n" << static_cast<int>(ErrorCode::InvalidInclude) << 1;
}

void SchemaParserTest::testIncludeErrors()
{
	QFETCH(QString, main_contents);
	QFETCH(int, error_code);
	QFETCH(int, error_line);

	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	writeSchemaFile(dir.filePath("inc_ok.sch"), "ok\n");
	writeSchemaFile(dir.filePath("inc_nested.sch"), "@include \"inc_ok\"\n");
	QString main = writeSchemaFile(dir.filePath("inc_err_main.sch"), main_contents);
	SchemaParser schparser;

	try
	{
		schparser.loadFile(main);
		QFAIL("Expected an exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(static_cast<int>(e.getErrorCode()), error_code);
		QRegularExpressionMatch match = QRegularExpression("line (\\d+), column (\\d+)").match(e.getErrorMessage());
		QVERIFY2(match.hasMatch(), qPrintable(e.getErrorMessage()));
		QCOMPARE(match.captured(1).toInt(), error_line);
	}
}

void SchemaParserTest::testErrorInsideIncludeReportsIncludeFileAndRelativeLine()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	QString inc_path = writeSchemaFile(dir.filePath("inc_pos_inc.sch"), "line one\nline two\n{unknown_attr}\n"),
			main = writeSchemaFile(dir.filePath("inc_pos_main.sch"), "before\n@include \"inc_pos_inc\"\nafter\n");
	attribs_map attribs;

	// Runs twice: the first goes through the full load, the second through the cache
	for(int i = 0; i < 2; i++)
	{
		SchemaParser schparser;

		try
		{
			schparser.loadFile(main);
			schparser.getSourceCode(attribs);
			QFAIL("Expected an UnkownAttribute exception but none was raised");
		}
		catch(Exception &e)
		{
			QCOMPARE(e.getErrorCode(), ErrorCode::UnkownAttribute);
			QVERIFY2(e.getErrorMessage().contains(inc_path), qPrintable(e.getErrorMessage()));
			QVERIFY2(e.getErrorMessage().contains("line 3, column 15"), qPrintable(e.getErrorMessage()));
		}
	}
}

void SchemaParserTest::testErrorAfterIncludeReportsMainFileLine()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	writeSchemaFile(dir.filePath("inc_after_inc.sch"), "x\ny\n");
	QString main = writeSchemaFile(dir.filePath("inc_after_main.sch"), "@include \"inc_after_inc\"\nz\n{unknown_attr}\n");
	attribs_map attribs;
	SchemaParser schparser;

	try
	{
		schparser.loadFile(main);
		schparser.getSourceCode(attribs);
		QFAIL("Expected an UnkownAttribute exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::UnkownAttribute);
		QVERIFY2(e.getErrorMessage().contains(main), qPrintable(e.getErrorMessage()));

		/* KNOWN ISSUE: the lines of the main file that come after an @include are reported shifted by the number of
		 * lines of the included file (plus the blank line appended to it). The correct line is 3 */
		QVERIFY2(e.getErrorMessage().contains("line 5,"), qPrintable(e.getErrorMessage()));
	}
}

void SchemaParserTest::testGetSourceCodeFromFileSetsPgSqlVersion()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	QString file = writeSchemaFile(dir.filePath("ver_test.sch"),
																 "{pgsql-ver} $sp\n"
																 "%if ({pgsql-ver} >=f \"12.0\") %then NEW %else OLD %end\n");
	SchemaParser schparser;
	attribs_map attribs;

	try
	{
		schparser.setPgSQLVersion("11.0", false);
		QCOMPARE(schparser.getSourceCode(file, attribs), QString("11.0 OLD"));
		QCOMPARE(attribs[Attributes::PgSqlVersion], QString("11.0"));

		attribs.clear();
		schparser.setPgSQLVersion("15.0", false);
		QCOMPARE(schparser.getSourceCode(file, attribs), QString("15.0 NEW"));
		QCOMPARE(attribs[Attributes::PgSqlVersion], QString("15.0"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testGetSourceCodeByObjectNameUsesTheSchemasFolders()
{
	SchemaParser schparser;
	attribs_map attribs;

	try
	{
		// XML code
		attribs = { { Attributes::Name, "col1" }, { Attributes::Alias, "a&b" }, { Attributes::Type, "integer" } };
		QCOMPARE(schparser.getSourceCode("simplecol", attribs, SchemaParser::XmlCode),
						 QString("\t<simplecol name=\"col1\" alias=\"a&amp;b\" type=\"integer\"/>\n"));
		QCOMPARE(attribs[Attributes::PgSqlVersion], schparser.getPgSQLVersion());

		// SQL code
		attribs = { { Attributes::Name, "tb1" }, { Attributes::SqlObject, "TABLE" }, { Attributes::Drop, "DROP TABLE tb1;" } };
		schparser.setPgSQLVersion("14.0", false);
		QString code = schparser.getSourceCode("objlabel", attribs, SchemaParser::SqlCode);
		QVERIFY2(code.startsWith("-- object: tb1 | type: TABLE --"), qPrintable(code));
		QCOMPARE(attribs[Attributes::PgSqlVersion], QString("14.0"));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void SchemaParserTest::testGetSourceCodeOfMissingSchemaRaises()
{
	SchemaParser schparser;
	attribs_map attribs;

	try
	{
		schparser.getSourceCode("this_schema_does_not_exist", attribs, SchemaParser::SqlCode);
		QFAIL("Expected an exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::FileDirectoryNotAccessed);
	}
}

void SchemaParserTest::testSetPgSQLVersion()
{
	SchemaParser schparser;

	QCOMPARE(schparser.getPgSQLVersion(), PgSqlVersions::DefaulVersion);

	// A supported version is kept as is
	schparser.setPgSQLVersion(PgSqlVersions::MinimumVersion, false);
	QCOMPARE(schparser.getPgSQLVersion(), PgSqlVersions::MinimumVersion);

	// A version newer than the default one falls back to the default
	schparser.setPgSQLVersion("99.0", false);
	QCOMPARE(schparser.getPgSQLVersion(), PgSqlVersions::DefaulVersion);

	// An invalid string falls back to the minimum version
	schparser.setPgSQLVersion("abc", false);
	QCOMPARE(schparser.getPgSQLVersion(), PgSqlVersions::MinimumVersion);

	// A version older than the minimum is refused unless the legacy versions are accepted
	try
	{
		schparser.setPgSQLVersion("8.0", false);
		QFAIL("Expected an InvPostgreSQLVersion exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::InvPostgreSQLVersion);
	}

	schparser.setPgSQLVersion("8.0", true);
	QCOMPARE(schparser.getPgSQLVersion(), QString("8.0"));
}

void SchemaParserTest::testExtractAttributes()
{
	SchemaParser schparser;

	schparser.loadBuffer("# {commented} is removed\n"
											 "{alpha} &{beta} {alpha}\n"
											 "%if {gamma} %and ({delta-1} == \"x\") %then {epsilon_2} %end\n"
											 "[{in-plain-text}]\n");
	QStringList attrs = schparser.extractAttributes();
	attrs.sort();

	QCOMPARE(attrs, QStringList({ "alpha", "beta", "delta-1", "epsilon_2", "gamma", "in-plain-text" }));

	schparser.restartParser();
	QVERIFY(schparser.extractAttributes().isEmpty());

	// An attribute that is not closed is ignored
	schparser.loadBuffer("{ok} {unclosed\n{ok2}");
	attrs = schparser.extractAttributes();
	attrs.sort();
	QCOMPARE(attrs, QStringList({ "ok", "ok2" }));
}

void SchemaParserTest::testFlagsAreResetAfterASuccessfulParsing()
{
	SchemaParser schparser;
	attribs_map attribs;

	schparser.ignoreUnkownAttributes(true);
	schparser.ignoreEmptyAttributes(true);
	schparser.loadBuffer("a{unknown}b");
	QCOMPARE(schparser.getSourceCode(attribs), QString("ab"));

	// Same instance, flags not set again: the unknown attribute must raise now
	try
	{
		schparser.loadBuffer("a{unknown}b");
		schparser.getSourceCode(attribs);
		QFAIL("Expected an UnkownAttribute exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::UnkownAttribute);
		schparser.restartParser();
	}
}

void SchemaParserTest::testParsingWithoutLoadedBufferReturnsEmptyText()
{
	SchemaParser schparser;
	attribs_map attribs { { "a", "b" } };

	QCOMPARE(schparser.getSourceCode(attribs), QString());

	schparser.loadBuffer("{a}");
	schparser.restartParser();
	QCOMPARE(schparser.getSourceCode(attribs), QString());
}

void SchemaParserTest::testCurrentLineAndColumnPointToTheError()
{
	SchemaParser schparser;
	attribs_map attribs;

	QCOMPARE(schparser.getCurrentLine(), 1);
	QCOMPARE(schparser.getCurrentColumn(), 1);

	try
	{
		schparser.loadBuffer("ok\n  %foo\n");
		schparser.getSourceCode(attribs);
		QFAIL("Expected an InvalidInstruction exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::InvalidInstruction);
		QCOMPARE(schparser.getCurrentLine(), 2);
		QCOMPARE(schparser.getCurrentColumn(), 7);
		schparser.restartParser();
		QCOMPARE(schparser.getCurrentLine(), 1);
		QCOMPARE(schparser.getCurrentColumn(), 1);
	}
}

void SchemaParserTest::testLoadFileOfMissingFileRaises()
{
	SchemaParser schparser;

	// An empty file name is silently ignored
	schparser.loadFile("");

	try
	{
		schparser.loadFile("/this/path/does/not/exist.sch");
		QFAIL("Expected an exception but none was raised");
	}
	catch(Exception &e)
	{
		QCOMPARE(e.getErrorCode(), ErrorCode::FileDirectoryNotAccessed);
	}
}

void SchemaParserTest::testCachedLoadProducesTheSameOutputAsTheFirstLoad()
{
	QTemporaryDir dir;
	QVERIFY(dir.isValid());

	writeSchemaFile(dir.filePath("cache_inc.sch"), "[INC]\n");
	QString main = writeSchemaFile(dir.filePath("cache_same.sch"),
																 "@include \"cache_inc\"\n"
																 "# comment\n"
																 "%if {a} %then <{a}> %else none %end\n"
																 "[ multi\n line ]\n");
	attribs_map attribs { { "a", "A" } };
	QString first, second;

	try
	{
		SchemaParser p1, p2;
		p1.loadFile(main);
		first = p1.getSourceCode(attribs);
		p2.loadFile(main);
		second = p2.getSourceCode(attribs);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QCOMPARE(first, QString("INC<A> multi\n line "));
	QCOMPARE(second, first);
}

// comments, plain texts opened before comment chars, line endings
void SchemaParserTest::testBuffersPreprocessor_data()
{
	addBufferColumns();

	QTest::newRow("emptyBuffer") << QString("") << QStringList() << false << false << QString("") << -1 << 0 << 0;
	QTest::newRow("onlyBlankLines") << QString("\n\n  \n\t\n") << QStringList() << false << false << QString("") << -1 << 0 << 0;
	QTest::newRow("onlyComments") << QString("# a\n   # b\n#c") << QStringList() << false << false << QString("") << -1 << 0 << 0;
	QTest::newRow("commentAfterTokens") << QString("foo bar # ignored\nbaz") << QStringList() << false << false << QString("foobarbaz") << -1 << 0 << 0;
	QTest::newRow("commentInsidePlainTextKept") << QString("[a # b] c # d") << QStringList() << false << false << QString("a # bc") << -1 << 0 << 0;
	QTest::newRow("commentInsideMultilinePlainText") << QString("[line1 # kept\nline2 # kept]") << QStringList() << false << false << QString("line1 # kept\nline2 # kept") << -1 << 0 << 0;
	QTest::newRow("escapedHashIsAChar") << QString("\\# not comment") << QStringList() << false << false << QString("#notcomment") << -1 << 0 << 0;
	QTest::newRow("escapedHashFollowedByComment") << QString("a\\#b # comment") << QStringList() << false << false << QString("a#b") << -1 << 0 << 0;
	QTest::newRow("noTrailingLineBreak") << QString("foo") << QStringList() << false << false << QString("foo") << -1 << 0 << 0;
	QTest::newRow("crlfLineEndings") << QString("foo\r\nbar\r\n") << QStringList() << false << false << QString("foobar") << -1 << 0 << 0;
	QTest::newRow("tabsAndSpacesIgnored") << QString("  a \t b\t\t c  ") << QStringList() << false << false << QString("abc") << -1 << 0 << 0;
	QTest::newRow("unicodeWords") << QString("açaí ção") << QStringList() << false << false << QString("açaíção") << -1 << 0 << 0;
	QTest::newRow("plainTextOpenedBeforeCommentChar") << QString("[abc]# trailing") << QStringList() << false << false << QString("abc") << -1 << 0 << 0;
	QTest::newRow("escapedBracketEndsPlainTextForCommentStripping") << QString("[a \\] # b]") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 1;
}

void SchemaParserTest::testBuffersPreprocessor()
{
	checkBuffer();
}

// words and plain text expressions
void SchemaParserTest::testBuffersPlainTexts_data()
{
	addBufferColumns();

	QTest::newRow("wordsAreConcatenated") << QString("a b\tc") << QStringList() << false << false << QString("abc") << -1 << 0 << 0;
	QTest::newRow("punctuationIsAWordChar") << QString("(foo),bar;\"baz\"'q' <x> =y !z") << QStringList() << false << false << QString("(foo),bar;\"baz\"'q'<x>=y!z") << -1 << 0 << 0;
	QTest::newRow("plainTextKeepsSpaces") << QString("[a  b\tc]") << QStringList() << false << false << QString("a  b\tc") << -1 << 0 << 0;
	QTest::newRow("plainTextEmpty") << QString("x[]y") << QStringList() << false << false << QString("xy") << -1 << 0 << 0;
	QTest::newRow("plainTextMultiline") << QString("[a\n  b\n]c") << QStringList() << false << false << QString("a\n  b\nc") << -1 << 0 << 0;
	QTest::newRow("plainTextEscapedBrackets") << QString("[\\[a\\]]") << QStringList() << false << false << QString("[a]") << -1 << 0 << 0;
	QTest::newRow("plainTextOtherBackslashKept") << QString("[a\\xb\\\\c]") << QStringList() << false << false << QString("a\\xb\\\\c") << -1 << 0 << 0;
	QTest::newRow("plainTextHoldsSpecials") << QString("[{a} %if $br &{a} \\n]") << QStringList{ "a=X" } << false << false << QString("{a} %if $br &{a} \\n") << -1 << 0 << 0;
	QTest::newRow("atIncludeInsideClosedPlainTextIsAMalformedInclude") << QString("[x @include y]") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 15;
	QTest::newRow("atIncludeInsideOpenMultilinePlainTextIsText") << QString("[x\n@include y\nz]") << QStringList() << false << false << QString("x\n@include y\nz") << -1 << 0 << 0;
	QTest::newRow("plainTextAdjacentToWords") << QString("a[ b ]c") << QStringList() << false << false << QString("a b c") << -1 << 0 << 0;
	QTest::newRow("twoPlainTexts") << QString("[a][b]") << QStringList() << false << false << QString("ab") << -1 << 0 << 0;
	QTest::newRow("unbalancedPlainTextOpen") << QString("[abc") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 1;
	QTest::newRow("unbalancedPlainTextOpenMultiline") << QString("[abc\ndef") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 1;
	QTest::newRow("strayPlainTextClose") << QString("foo ] bar") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("nestedPlainTextOpen") << QString("[a [b]") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 1;
	QTest::newRow("strayCloseAfterPlainText") << QString("[a]]") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 4;
}

void SchemaParserTest::testBuffersPlainTexts()
{
	checkBuffer();
}

// attribute references, XML entity conversion, unknown/empty/invalid names
void SchemaParserTest::testBuffersAttributes_data()
{
	addBufferColumns();

	QTest::newRow("simple") << QString("{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("AA") << -1 << 0 << 0;
	QTest::newRow("adjacent") << QString("{a}{b}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("AABB") << -1 << 0 << 0;
	QTest::newRow("betweenWords") << QString("x{a}y {b}z") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("xAAyBBz") << -1 << 0 << 0;
	QTest::newRow("nameWithDigitsHyphenUnderscore") << QString("{n1}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("x-y_z") << -1 << 0 << 0;
	QTest::newRow("uppercaseName") << QString("{A}") << QStringList{ "A=up" } << false << false << QString("up") << -1 << 0 << 0;
	QTest::newRow("caseSensitiveLookup") << QString("{A}") << QStringList{ "a=low" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 4;
	QTest::newRow("caseSensitiveLookupIgnoringUnknown") << QString("x{A}y") << QStringList{ "a=low" } << true << true << QString("xy") << -1 << 0 << 0;
	QTest::newRow("unknownRaises") << QString("{zz}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 5;
	QTest::newRow("unknownIgnoredIsEmpty") << QString("x{zz}y") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << true << true << QString("xy") << -1 << 0 << 0;
	QTest::newRow("unknownIgnoredButEmptyRaises") << QString("x{zz}y") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << true << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 1 << 6;
	QTest::newRow("emptyRaises") << QString("{e}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 1 << 4;
	QTest::newRow("emptyIgnored") << QString("x{e}y") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("xy") << -1 << 0 << 0;
	QTest::newRow("invalidNameStartsWithDigit") << QString("{1a}") << QStringList{ "1a=v" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("invalidNameStartsWithHyphen") << QString("{-a}") << QStringList{ "-a=v" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("invalidNameStartsWithUnderscore") << QString("{_a}") << QStringList{ "_a=v" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("invalidNameWithSpace") << QString("{a b}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("invalidNameWithDot") << QString("{a.b}") << QStringList{ "a.b=v" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("invalidNameNonAscii") << QString("{aé}") << QStringList{ "aé=v" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("unclosedAtEndOfLine") << QString("{a") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("unclosedFollowedBySpace") << QString("{a }") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("emptyName") << QString("{}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("strayClose") << QString("a}b") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 2;
	QTest::newRow("ampersandNotFollowedByBrace") << QString("&a") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 1;
	QTest::newRow("ampersandAtEndOfLine") << QString("a&") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 4;
	QTest::newRow("ampersandPlain") << QString("&{a}") << QStringList{ "a=x & y" } << false << false << QString("x &amp; y") << -1 << 0 << 0;
	QTest::newRow("ampersandAllSpecials") << QString("&{a}") << QStringList{ "a=<a href=\"x\">&</a>" } << false << false << QString("&lt;a href=&quot;x&quot;&gt;&amp;&lt;/a&gt;") << -1 << 0 << 0;
	QTest::newRow("ampersandKeepsExistingEntities") << QString("&{a}") << QStringList{ "a=a &amp; b < c" } << false << false << QString("a &amp; b &lt; c") << -1 << 0 << 0;
	QTest::newRow("ampersandEntityQuot") << QString("&{a}") << QStringList{ "a=&quot;x&quot; & \"" } << false << false << QString("&quot;x&quot; & &quot;") << -1 << 0 << 0;
	QTest::newRow("ampersandEntityLtGtApos") << QString("&{a}") << QStringList{ "a=&lt; &gt; &apos; & >" } << false << false << QString("&lt; &gt; &apos; & &gt;") << -1 << 0 << 0;
	QTest::newRow("ampersandNoSpecials") << QString("&{a}") << QStringList{ "a=plain text" } << false << false << QString("plain text") << -1 << 0 << 0;
	QTest::newRow("ampersandEmptyRaises") << QString("&{e}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 1 << 5;
	QTest::newRow("ampersandEmptyIgnored") << QString("a&{e}b") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("ab") << -1 << 0 << 0;
	QTest::newRow("ampersandUnknownRaises") << QString("&{zz}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 6;
	QTest::newRow("plainBraceValueNotConverted") << QString("{a}") << QStringList{ "a=<&>\"" } << false << false << QString("<&>\"") << -1 << 0 << 0;
}

void SchemaParserTest::testBuffersAttributes()
{
	checkBuffer();
}

// metacharacter tokens
void SchemaParserTest::testBuffersMetachars_data()
{
	addBufferColumns();

	QTest::newRow("meta_sp") << QString("a$spb") << QStringList() << false << false << QString("a b") << -1 << 0 << 0;
	QTest::newRow("meta_br") << QString("a$brb") << QStringList() << false << false << QString("a\nb") << -1 << 0 << 0;
	QTest::newRow("meta_tb") << QString("a$tbb") << QStringList() << false << false << QString("a\tb") << -1 << 0 << 0;
	QTest::newRow("meta_ob") << QString("a$obb") << QStringList() << false << false << QString("a[b") << -1 << 0 << 0;
	QTest::newRow("meta_cb") << QString("a$cbb") << QStringList() << false << false << QString("a]b") << -1 << 0 << 0;
	QTest::newRow("meta_oc") << QString("a$ocb") << QStringList() << false << false << QString("a{b") << -1 << 0 << 0;
	QTest::newRow("meta_cc") << QString("a$ccb") << QStringList() << false << false << QString("a}b") << -1 << 0 << 0;
	QTest::newRow("meta_ms") << QString("a$msb") << QStringList() << false << false << QString("a$b") << -1 << 0 << 0;
	QTest::newRow("meta_hs") << QString("a$hsb") << QStringList() << false << false << QString("a#b") << -1 << 0 << 0;
	QTest::newRow("meta_ps") << QString("a$psb") << QStringList() << false << false << QString("a%b") << -1 << 0 << 0;
	QTest::newRow("meta_at") << QString("a$atb") << QStringList() << false << false << QString("a@b") << -1 << 0 << 0;
	QTest::newRow("meta_ds") << QString("a$dsb") << QStringList() << false << false << QString("a·b") << -1 << 0 << 0;
	QTest::newRow("meta_am") << QString("a$amb") << QStringList() << false << false << QString("a&b") << -1 << 0 << 0;
	QTest::newRow("meta_bs") << QString("a$bsb") << QStringList() << false << false << QString("a\\b") << -1 << 0 << 0;
	QTest::newRow("contiguousMetas") << QString("a$br$tb$sp$brb") << QStringList() << false << false << QString("a\n\t \nb") << -1 << 0 << 0;
	QTest::newRow("metaOnlyTwoLetters") << QString("$brx $spyz") << QStringList() << false << false << QString("\nx yz") << -1 << 0 << 0;
	QTest::newRow("metaFollowedByAttr") << QString("$br{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("\nAA") << -1 << 0 << 0;
	QTest::newRow("invalidMeta") << QString("$zz") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidMetacharacter) << 1 << 4;
	QTest::newRow("invalidMetaOneLetter") << QString("$b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidMetacharacter) << 1 << 3;
	QTest::newRow("dollarAlone") << QString("a $ b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 4;
	QTest::newRow("dollarAtEnd") << QString("a$") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
}

void SchemaParserTest::testBuffersMetachars()
{
	checkBuffer();
}

// escaped character tokens
void SchemaParserTest::testBuffersEscapedChars_data()
{
	addBufferColumns();

	QTest::newRow("esc_s") << QString("a\\sb") << QStringList() << false << false << QString("a b") << -1 << 0 << 0;
	QTest::newRow("esc_t") << QString("a\\tb") << QStringList() << false << false << QString("a\tb") << -1 << 0 << 0;
	QTest::newRow("esc_n") << QString("a\\nb") << QStringList() << false << false << QString("a\nb") << -1 << 0 << 0;
	QTest::newRow("esc_[") << QString("a\\[b") << QStringList() << false << false << QString("a[b") << -1 << 0 << 0;
	QTest::newRow("esc_]") << QString("a\\]b") << QStringList() << false << false << QString("a]b") << -1 << 0 << 0;
	QTest::newRow("esc_{") << QString("a\\{b") << QStringList() << false << false << QString("a{b") << -1 << 0 << 0;
	QTest::newRow("esc_}") << QString("a\\}b") << QStringList() << false << false << QString("a}b") << -1 << 0 << 0;
	QTest::newRow("esc_$") << QString("a\\$b") << QStringList() << false << false << QString("a$b") << -1 << 0 << 0;
	QTest::newRow("esc_#") << QString("a\\#b") << QStringList() << false << false << QString("a#b") << -1 << 0 << 0;
	QTest::newRow("esc_%") << QString("a\\%b") << QStringList() << false << false << QString("a%b") << -1 << 0 << 0;
	QTest::newRow("esc_@") << QString("a\\@b") << QStringList() << false << false << QString("a@b") << -1 << 0 << 0;
	QTest::newRow("esc_&") << QString("a\\&b") << QStringList() << false << false << QString("a&b") << -1 << 0 << 0;
	QTest::newRow("esc_*") << QString("a\\*b") << QStringList() << false << false << QString("a·b") << -1 << 0 << 0;
	QTest::newRow("escapedBackslashIsRejected") << QString("a\\\\b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("contiguousEscapes") << QString("\\#\\s\\t\\n\\*x") << QStringList() << false << false << QString("# \t\n·x") << -1 << 0 << 0;
	QTest::newRow("invalidEscape") << QString("\\q") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidEscapedCharacter) << 1 << 3;
	QTest::newRow("invalidEscapeDigit") << QString("\\1") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidEscapedCharacter) << 1 << 3;
	QTest::newRow("backslashAlone") << QString("a \\ b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 4;
	QTest::newRow("backslashAtEnd") << QString("a\\") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
}

void SchemaParserTest::testBuffersEscapedChars()
{
	checkBuffer();
}

// if/then/else/end blocks and nesting
void SchemaParserTest::testBuffersConditionals_data()
{
	addBufferColumns();

	QTest::newRow("ifTrueNoElse") << QString("%if {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("X") << -1 << 0 << 0;
	QTest::newRow("ifFalseNoElse") << QString("%if {e} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("") << -1 << 0 << 0;
	QTest::newRow("ifTrueWithElse") << QString("%if {a} %then X %else Y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("X") << -1 << 0 << 0;
	QTest::newRow("ifFalseWithElse") << QString("%if {e} %then X %else Y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("Y") << -1 << 0 << 0;
	QTest::newRow("emptyThenBranch") << QString("%if {a} %then %else Y %end Z") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("Z") << -1 << 0 << 0;
	QTest::newRow("emptyElseBranch") << QString("%if {e} %then X %else %end Z") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("Z") << -1 << 0 << 0;
	QTest::newRow("emptyBothBranches") << QString("%if {a} %then %end Z") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("Z") << -1 << 0 << 0;
	QTest::newRow("ifAcrossLines") << QString("%if {a}\n%then\n X\n%else\n Y\n%end\n") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("X") << -1 << 0 << 0;
	QTest::newRow("ifWithTextAround") << QString("pre %if {a} %then [-in-] %end post") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("pre-in-post") << -1 << 0 << 0;
	QTest::newRow("sequentialIfs") << QString("%if {a} %then 1 %end %if {e} %then 2 %end %if {b} %then 3 %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("13") << -1 << 0 << 0;
	QTest::newRow("nestedTrueTrue") << QString("%if {a} %then %if {b} %then AB %else A %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("AB") << -1 << 0 << 0;
	QTest::newRow("nestedTrueFalse") << QString("%if {a} %then %if {e} %then AB %else A %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("A") << -1 << 0 << 0;
	QTest::newRow("nestedFalseOuterSkipsInner") << QString("%if {e} %then %if {a} %then AB %end %else O %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("O") << -1 << 0 << 0;
	QTest::newRow("nestedInElse") << QString("%if {e} %then X %else %if {b} %then B %else N %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("B") << -1 << 0 << 0;
	QTest::newRow("nestedDepth4") << QString("%if {a} %then %if {a} %then %if {a} %then %if {a} %then D %end %end %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("D") << -1 << 0 << 0;
	QTest::newRow("nestedWithTextAfterInner") << QString("%if {a} %then 1 %if {b} %then 2 %end 3 %else 4 %end 5") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("1235") << -1 << 0 << 0;
	QTest::newRow("nestedBothBranchesText") << QString("%if {a} %then %if {e} %then x %else y %end z %else %if {b} %then w %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("yz") << -1 << 0 << 0;
	QTest::newRow("elseIfChain") << QString("%if {e} %then 1 %else %if {e} %then 2 %else 3 %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("3") << -1 << 0 << 0;
	QTest::newRow("attrInsideBranch") << QString("%if {a} %then <{a}|{b}> %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("<AA|BB>") << -1 << 0 << 0;
	QTest::newRow("metaInsideBranch") << QString("%if {a} %then $br $tb x %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("\n\tx") << -1 << 0 << 0;
	QTest::newRow("escapeInsideBranch") << QString("%if {a} %then \\n\\# %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("\n#") << -1 << 0 << 0;
	QTest::newRow("plainTextInsideBranch") << QString("%if {a} %then [ a b ] %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString(" a b ") << -1 << 0 << 0;
	QTest::newRow("metaInsideElseBranch") << QString("%if {e} %then X %else $br $tb y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("\n\ty") << -1 << 0 << 0;
	QTest::newRow("escapeInsideElseBranch") << QString("%if {e} %then X %else \\n\\# %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("\n#") << -1 << 0 << 0;
	QTest::newRow("plainTextInsideElseBranch") << QString("%if {e} %then X %else [ a b ] %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString(" a b ") << -1 << 0 << 0;
	QTest::newRow("attrInsideElseBranch") << QString("%if {e} %then X %else <{a}|{b}> %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("<AA|BB>") << -1 << 0 << 0;
	QTest::newRow("xmlAttrInsideElseBranch") << QString("%if {e} %then X %else &{x} %end") << QStringList{ "e=", "x=<>&" } << false << false << QString("&lt;&gt;&amp;") << -1 << 0 << 0;
	QTest::newRow("xmlAttrInsideBranch") << QString("%if {a} %then &{x} %end") << QStringList{ "a=1", "x=<>&" } << false << false << QString("&lt;&gt;&amp;") << -1 << 0 << 0;
	QTest::newRow("xmlAttrInsideNestedBranch") << QString("%if {a} %then %if {a} %then &{x} %end %end") << QStringList{ "a=1", "x=<>&" } << false << false << QString("&lt;&gt;&amp;") << -1 << 0 << 0;
	QTest::newRow("truthinessIsNonEmpty") << QString("%if {a} %then T %else F %end") << QStringList{ "a=0" } << false << false << QString("T") << -1 << 0 << 0;
}

void SchemaParserTest::testBuffersConditionals()
{
	checkBuffer();
}

// logical operators in conditional expressions and instruction ordering errors
void SchemaParserTest::testBuffersLogicalOperators_data()
{
	addBufferColumns();

	QTest::newRow("op_aandb") << QString("%if {a} %and {b} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_aande") << QString("%if {a} %and {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_eanda") << QString("%if {e} %and {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_aore") << QString("%if {a} %or {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_eora") << QString("%if {e} %or {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_eore") << QString("%if {e} %or {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_nota") << QString("%if %not {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_note") << QString("%if %not {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_aandnote") << QString("%if {a} %and %not {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_aornota") << QString("%if {a} %or %not {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_notaora") << QString("%if %not {a} %or {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_aorbande") << QString("%if {a} %or {b} %and {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_eandaora") << QString("%if {e} %and {a} %or {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_eoraanda") << QString("%if {e} %or {a} %and {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_aandbande") << QString("%if {a} %and {b} %and {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_eoreora") << QString("%if {e} %or {e} %or {a} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("op_notaandnote") << QString("%if %not {a} %and %not {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("op_noteandnote") << QString("%if %not {e} %and %not {e} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("exprAcrossLines") << QString("%if {a}\n  %and\n  {b}\n%then T %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("exprUnknownAttr") << QString("%if {zz} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 9;
	QTest::newRow("exprUnknownAttrIgnored") << QString("%if {zz} %then T %else F %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << true << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("exprXmlAttr") << QString("%if &{a} %then T %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("errNoOperand") << QString("%if %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errTrailingAnd") << QString("%if {a} %and %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 14;
	QTest::newRow("errLeadingAnd") << QString("%if %and {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 9;
	QTest::newRow("errLeadingOr") << QString("%if %or {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 8;
	QTest::newRow("errDoubleOr") << QString("%if {a} %or %or {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 16;
	QTest::newRow("errDoubleAnd") << QString("%if {a} %and %and {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 18;
	QTest::newRow("errAndOr") << QString("%if {a} %and %or {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 17;
	QTest::newRow("errOrAnd") << QString("%if {a} %or %and {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 17;
	QTest::newRow("errTwoAttrs") << QString("%if {a} {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("errNotAfterAttr") << QString("%if {a} %not %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 14;
	QTest::newRow("errNotAfterAttrThenAttr") << QString("%if {a} %not {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 17;
	QTest::newRow("errNotBeforeThen") << QString("%if %not %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 10;
	QTest::newRow("errDoubleNot") << QString("%if %not %not {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 14;
	QTest::newRow("errWordInExpr") << QString("%if foo %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errMetaInExpr") << QString("%if $br {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errEscapeInExpr") << QString("%if \\n {a} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errPlainTextInExpr") << QString("%if [a] %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errSetInExpr") << QString("%if {a} %set {x} 1 %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 17;
	QTest::newRow("errMissingThen") << QString("%if {a} X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 9;
	QTest::newRow("errThenAlone") << QString("%then X") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 6;
	QTest::newRow("errElseAlone") << QString("%else X") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 6;
	QTest::newRow("errEndAlone") << QString("%end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("errThenTwice") << QString("%if {a} %then %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 20;
	QTest::newRow("errUnclosedIf") << QString("%if {a} %then X") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 2 << 1;
	QTest::newRow("errUnclosedNestedIf") << QString("%if {a} %then %if {b} %then X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 2 << 1;
	QTest::newRow("errExtraEnd") << QString("%if {a} %then X %end %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 26;
	QTest::newRow("errElseAfterIf") << QString("%if {a} %else X %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 15;
	QTest::newRow("errTextBetweenIfAndThen") << QString("%if {a} X %then Y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 9;
	QTest::newRow("errExprEndsAtBufferEnd") << QString("%if {a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 2 << 8;
	QTest::newRow("errUnknownInstruction") << QString("%foo") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidInstruction) << 1 << 5;
	QTest::newRow("errPercentAlone") << QString("a % b") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 4;
	QTest::newRow("errPercentAtEnd") << QString("a%") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 3;
	QTest::newRow("errInstructionUppercase") << QString("%IF {a} %THEN X %END") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidInstruction) << 1 << 4;
	QTest::newRow("errElseThenWord") << QString("%if {a} %then X %else Y Z %else W %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 32;
}

void SchemaParserTest::testBuffersLogicalOperators()
{
	checkBuffer();
}

// comparison expressions
void SchemaParserTest::testBuffersComparisons_data()
{
	addBufferColumns();

	QTest::newRow("cmp_eqs_10") << QString("%if ({v} == \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_eqs_9p5") << QString("%if ({v} == \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_eqs_abc") << QString("%if ({v} == \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_eqf_10") << QString("%if ({v} ==f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_eqf_9p5") << QString("%if ({v} ==f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_eqf_abc") << QString("%if ({v} ==f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_eqi_10") << QString("%if ({v} ==i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_eqi_9p5") << QString("%if ({v} ==i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_eqi_abc") << QString("%if ({v} ==i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_nes_10") << QString("%if ({v} != \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_nes_9p5") << QString("%if ({v} != \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_nes_abc") << QString("%if ({v} != \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_nef_10") << QString("%if ({v} !=f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_nef_9p5") << QString("%if ({v} !=f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_nef_abc") << QString("%if ({v} !=f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_nei_10") << QString("%if ({v} !=i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_nei_9p5") << QString("%if ({v} !=i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_nei_abc") << QString("%if ({v} !=i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_lts_10") << QString("%if ({v} < \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_lts_9p5") << QString("%if ({v} < \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_lts_abc") << QString("%if ({v} < \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_ltf_10") << QString("%if ({v} <f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_ltf_9p5") << QString("%if ({v} <f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_ltf_abc") << QString("%if ({v} <f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_lti_10") << QString("%if ({v} <i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_lti_9p5") << QString("%if ({v} <i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lti_abc") << QString("%if ({v} <i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gts_10") << QString("%if ({v} > \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gts_9p5") << QString("%if ({v} > \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_gts_abc") << QString("%if ({v} > \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gtf_10") << QString("%if ({v} >f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gtf_9p5") << QString("%if ({v} >f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gtf_abc") << QString("%if ({v} >f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gti_10") << QString("%if ({v} >i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gti_9p5") << QString("%if ({v} >i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gti_abc") << QString("%if ({v} >i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_les_10") << QString("%if ({v} <= \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_les_9p5") << QString("%if ({v} <= \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_les_abc") << QString("%if ({v} <= \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lef_10") << QString("%if ({v} <=f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lef_9p5") << QString("%if ({v} <=f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lef_abc") << QString("%if ({v} <=f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lei_10") << QString("%if ({v} <=i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lei_9p5") << QString("%if ({v} <=i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_lei_abc") << QString("%if ({v} <=i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_ges_10") << QString("%if ({v} >= \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_ges_9p5") << QString("%if ({v} >= \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_ges_abc") << QString("%if ({v} >= \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gef_10") << QString("%if ({v} >=f \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_gef_9p5") << QString("%if ({v} >=f \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gef_abc") << QString("%if ({v} >=f \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_gei_10") << QString("%if ({v} >=i \"10\") %then T %else F %end") << QStringList{ "v=10" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmp_gei_9p5") << QString("%if ({v} >=i \"10\") %then T %else F %end") << QStringList{ "v=9.5" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmp_gei_abc") << QString("%if ({v} >=i \"abd\") %then T %else F %end") << QStringList{ "v=abc" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpSpacesInside") << QString("%if (  {v}   ==   \"10\"  ) %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpNoSpaces") << QString("%if ({v}==\"10\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpStringIsLexicographic") << QString("%if ({v} > \"9\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmpIntNonNumericIsZero") << QString("%if ({s} ==i \"0\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpFloatNonNumericIsZero") << QString("%if ({s} ==f \"0\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpEmptyAttrValue") << QString("%if ({z} == \"\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpEmptyLiteral") << QString("%if ({s} != \"\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpValueWithSpaces") << QString("%if ({s} == \"a b\") %then T %else F %end") << QStringList{ "s=a b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpXmlEntityAttr") << QString("%if (&{t} == \"a&lt;b\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpPlainAttrNotConverted") << QString("%if ({t} == \"a<b\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpVersionFloat") << QString("%if ({pgsql-ver} >=f \"10.0\") %then T %else F %end") << QStringList{ "pgsql-ver=9.6" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("cmpVersionFloat2") << QString("%if ({pgsql-ver} >=f \"10.0\") %then T %else F %end") << QStringList{ "pgsql-ver=15.0" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpWithAnd") << QString("%if ({v} ==i \"10\") %and {s} %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpWithOrNot") << QString("%if %not ({v} ==i \"10\") %or ({s} == \"abc\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpNotComparison") << QString("%if %not ({v} ==i \"11\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpTwoComparisons") << QString("%if ({v} >i \"5\") %and ({v} <i \"20\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpAfterAttr") << QString("%if {s} %and ({v} ==i \"10\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("cmpUnknownRaises") << QString("%if ({zz} == \"x\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 18;
	QTest::newRow("cmpUnknownIgnored") << QString("%if ({zz} == \"\") %then T %else F %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << true << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("errCmpNoOperator") << QString("%if ({v} \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 10;
	QTest::newRow("errCmpInvalidOperator") << QString("%if ({v} = \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidOperatorInExpression) << 1 << 17;
	QTest::newRow("errCmpTripleEq") << QString("%if ({v} === \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidOperatorInExpression) << 1 << 19;
	QTest::newRow("errCmpOnlyCast") << QString("%if ({v} f \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidOperatorInExpression) << 1 << 17;
	QTest::newRow("errCmpBadCast") << QString("%if ({v} ==x \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("errCmpOperatorOnly") << QString("%if ({v} ==) %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 13;
	QTest::newRow("errCmpMissingValue") << QString("%if ({v} == ) %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 14;
	QTest::newRow("errCmpMissingQuotes") << QString("%if ({v} == 10) %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 13;
	QTest::newRow("errCmpUnclosedQuote") << QString("%if ({v} == \"10) %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 31;
	QTest::newRow("errCmpUnclosedParen") << QString("%if ({v} == \"10\" %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 18;
	QTest::newRow("errCmpEmptyParens") << QString("%if () %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 7;
	QTest::newRow("errCmpAttrTwice") << QString("%if ({v} {s} == \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 10;
	QTest::newRow("errCmpValueFirst") << QString("%if (\"10\" == {v}) %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 6;
	QTest::newRow("errCmpTwoValues") << QString("%if ({v} == \"10\" \"11\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 18;
	QTest::newRow("errCmpAcrossLines") << QString("%if ({v} ==\n \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("cmpDoubleCastSuffixIsAccepted") << QString("%if ({v} ==ff \"10\") %then T %end") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("parenthesesOutsideIfAreOrdinaryText") << QString("({v} == \"10\")") << QStringList{ "v=10", "s=abc", "f=9.5", "z=", "t=a<b" } << false << false << QString("(10==\"10\")") << -1 << 0 << 0;
}

void SchemaParserTest::testBuffersComparisons()
{
	checkBuffer();
}

// errors and values in selected and non selected branches
void SchemaParserTest::testBuffersBranches_data()
{
	addBufferColumns();

	QTest::newRow("unknownAttrInFalseBranchRaises") << QString("%if {e} %then {zz} %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 19;
	QTest::newRow("unknownAttrInFalseBranchIgnored") << QString("%if {e} %then x{zz}y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << true << true << QString("") << -1 << 0 << 0;
	QTest::newRow("unknownAttrInElseBranchRaises") << QString("%if {a} %then ok %else {zz} %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 28;
	QTest::newRow("emptyAttrInTrueBranchRaises") << QString("%if {a} %then {e} %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 1 << 23;
	QTest::newRow("emptyAttrInTrueBranchRaisesAtEndOfOutermost") << QString("%if {a} %then\n  %if {a} %then\n    {e}\n  %end\n  tail\n%end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 6 << 5;
	QTest::newRow("emptyAttrInTrueBranchIgnored") << QString("%if {a} %then x{e}y %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("xy") << -1 << 0 << 0;
	QTest::newRow("emptyAttrInFalseBranchOk") << QString("%if {e} %then {e} %else ok %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("ok") << -1 << 0 << 0;
	QTest::newRow("emptyAttrInNestedFalseBranchOk") << QString("%if {a} %then %if {e} %then {e} %end done %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("done") << -1 << 0 << 0;
	QTest::newRow("emptyAttrInElseSelectedRaises") << QString("%if {e} %then ok %else {e} %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 1 << 32;
	QTest::newRow("syntaxErrorInFalseBranch") << QString("%if {e} %then %zzz %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidInstruction) << 1 << 19;
	QTest::newRow("plainTextErrorInFalseBranch") << QString("%if {e} %then [abc %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 15;
	QTest::newRow("metaErrorInFalseBranch") << QString("%if {e} %then $zz %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidMetacharacter) << 1 << 18;
	QTest::newRow("escapeErrorInFalseBranch") << QString("%if {e} %then \\q %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidEscapedCharacter) << 1 << 17;
	QTest::newRow("attrSyntaxErrorInFalseBranch") << QString("%if {e} %then {a %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 17;
	QTest::newRow("errorPositionAfterMultilineTrueBranch") << QString("%if {a} %then\n a\n b\n {e}\n%end\n") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 5 << 5;
}

void SchemaParserTest::testBuffersBranches()
{
	checkBuffer();
}

// %set, %unset and the @{} form
void SchemaParserTest::testBuffersSetUnset_data()
{
	addBufferColumns();

	QTest::newRow("setSimple") << QString("%set {x} hello\n{x}") << QStringList() << false << false << QString("hello") << -1 << 0 << 0;
	QTest::newRow("setWordsConcatenate") << QString("%set {x} hello world\n{x}") << QStringList() << false << false << QString("helloworld") << -1 << 0 << 0;
	QTest::newRow("setPlainText") << QString("%set {x} [hello world]\n{x}") << QStringList() << false << false << QString("hello world") << -1 << 0 << 0;
	QTest::newRow("setPlainTextMultiline") << QString("%set {x} [l1\n   l2]\n{x}") << QStringList() << false << false << QString("l1\n   l2") << -1 << 0 << 0;
	QTest::newRow("setWithAttr") << QString("%set {x} <{a}>\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("<AA>") << -1 << 0 << 0;
	QTest::newRow("setWithXmlAttr") << QString("%set {x} &{t}\n{x}") << QStringList{ "t=<&>" } << false << false << QString("&lt;&amp;&gt;") << -1 << 0 << 0;
	QTest::newRow("setMeta") << QString("%set {x} a$sp$brb\n{x}") << QStringList() << false << false << QString("a \nb") << -1 << 0 << 0;
	QTest::newRow("setEscape") << QString("%set {x} \\# \\&\n{x}") << QStringList() << false << false << QString("#&") << -1 << 0 << 0;
	QTest::newRow("setOverwritesExisting") << QString("%set {a} NEW\n{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("NEW") << -1 << 0 << 0;
	QTest::newRow("setUsesPreviousSet") << QString("%set {x} 1\n%set {y} {x}2\n{y}") << QStringList() << false << false << QString("12") << -1 << 0 << 0;
	QTest::newRow("setSelfReference") << QString("%set {a} {a}+\n{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("AA+") << -1 << 0 << 0;
	QTest::newRow("setEmptyValue") << QString("%set {x}\nok") << QStringList() << false << true << QString("ok") << -1 << 0 << 0;
	QTest::newRow("setEmptyValueUsedRaises") << QString("%set {x}\n{x}") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 2 << 4;
	QTest::newRow("setValueOf") << QString("%set @{n} hello\n{foo}") << QStringList{ "n=foo" } << false << false << QString("hello") << -1 << 0 << 0;
	QTest::newRow("setValueOfUnknownRaises") << QString("%set @{n} hello\n{foo}") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("setValueOfInvalidName") << QString("%set @{n} hello") << QStringList{ "n=1bad" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("setValueOfEmptyName") << QString("%set @{n} hello") << QStringList{ "n=" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("setValueOfTwice") << QString("%set @{n} @{m} hello") << QStringList{ "n=a1", "m=b1" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 11;
	QTest::newRow("setAtNotBeforeAttr") << QString("%set @x hello") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 7;
	QTest::newRow("setInvalidName") << QString("%set {1a} hello") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("setNoName") << QString("%set\nfoo") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 5;
	QTest::newRow("setNameNotAttr") << QString("%set foo bar") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 9;
	QTest::newRow("setNameAsXml") << QString("%set &{x} v\n{x}") << QStringList() << false << false << QString("v") << -1 << 0 << 0;
	QTest::newRow("setUnknownRefInValueRaises") << QString("%set {x} {zz}\n{x}") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 14;
	QTest::newRow("setUnknownRefInValueIgnored") << QString("%set {x} a{zz}b\n{x}") << QStringList() << true << true << QString("ab") << -1 << 0 << 0;
	QTest::newRow("setEmptyRefInValueOk") << QString("%set {x} a{e}b\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("ab") << -1 << 0 << 0;
	QTest::newRow("setStrayCloseBracketIsASyntaxError") << QString("%set {x} a ] b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("setStrayCloseBraceIsASyntaxError") << QString("%set {x} a } b") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("setStrayCloseBracketAfterPlainText") << QString("%set {x} [a] ]") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 14;
	QTest::newRow("setCondOnSameLineRaises") << QString("%set {x} 1 %if {a} %then 2 %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
	QTest::newRow("setEndOnSameLineRaises") << QString("%if {a} %then %set {x} 1 %end\n") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 26;
	QTest::newRow("setInTrueBranch") << QString("%if {a} %then\n%set {x} T\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("setInFalseBranchPredeclares") << QString("%if {e} %then\n%set {x} T\n%end\n[<]{x}[>]") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("setInFalseBranchEmptyUsedRaises") << QString("%if {e} %then\n%set {x} T\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::UndefinedAttributeValue) << 4 << 4;
	QTest::newRow("setInFalseBranchThenTestedInIf") << QString("%if {e} %then\n%set {x} T\n%end\n%if {x} %then Y %else N %end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("N") << -1 << 0 << 0;
	QTest::newRow("setInElseOfTrueIfSkipped") << QString("%if {a} %then\n%set {x} T\n%else\n%set {x} F\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("setInElseOfFalseIfRuns") << QString("%if {e} %then\n%set {x} T\n%else\n%set {x} F\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("F") << -1 << 0 << 0;
	QTest::newRow("setNestedTrueTrue") << QString("%if {a} %then\n%if {b} %then\n%set {x} T\n%end\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("setNestedTrueFalse") << QString("%if {a} %then\n%if {e} %then\n%set {x} T\n%end\n%end\n[<]{x}[>]") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("setNestedFalseOuter") << QString("%if {e} %then\n%if {a} %then\n%set {x} T\n%end\n%end\n[<]{x}[>]") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("setNestedElseInTrueOuter") << QString("%if {a} %then\n%if {e} %then\n1\n%else\n%set {x} E\n%end\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("E") << -1 << 0 << 0;
	QTest::newRow("setThenUsedInSameBranch") << QString("%if {a} %then\n%set {x} T\n{x}\n%end") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("T") << -1 << 0 << 0;
	QTest::newRow("setSelectsLaterIf") << QString("%set {x} 1\n%if {x} %then Y %else N %end") << QStringList() << false << false << QString("Y") << -1 << 0 << 0;
	QTest::newRow("setMultilineInFalseBranchBreaks") << QString("%if {e} %then\n%set {x} [l1\nl2]\n%end\nok") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 3 << 3;
	QTest::newRow("setMultilineInTrueBranch") << QString("%if {a} %then\n%set {x} [l1\nl2]\n%end\n{x}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("l1\nl2") << -1 << 0 << 0;
	QTest::newRow("unsetSimple") << QString("%set {x} v\n%unset {x}\n[<]{x}[>]") << QStringList() << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("unsetMultiple") << QString("%unset {a} {b}\n[<]{a}{b}[>]") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("unsetUnknownRaises") << QString("%unset {zz}") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::UnkownAttribute) << 1 << 12;
	QTest::newRow("unsetUnknownIgnored") << QString("%unset {zz}\n[<]{zz}[>]") << QStringList() << true << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("unsetNoAttrRaises") << QString("%unset\nfoo") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 7;
	QTest::newRow("unsetWordRaises") << QString("%unset foo") << QStringList() << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 8;
	QTest::newRow("unsetXmlAttrRaises") << QString("%unset &{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 8;
	QTest::newRow("unsetInvalidName") << QString("%unset {1a}") << QStringList{ "1a=x" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidAttribute) << -1 << -1;
	QTest::newRow("unsetInFalseBranchIsSkipped") << QString("%if {e} %then\n%unset {a}\n%end\n{a}") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString("AA") << -1 << 0 << 0;
	QTest::newRow("unsetInTrueBranchRuns") << QString("%if {a} %then\n%unset {a}\n%end\n[<]{a}[>]") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("unsetOnSameLineAsText") << QString("%unset {a} text") << QStringList{ "a=AA", "b=BB", "e=", "n1=x-y_z" } << false << false << QString() << static_cast<int>(ErrorCode::InvalidSyntax) << 1 << 12;
}

void SchemaParserTest::testBuffersSetUnset()
{
	checkBuffer();
}

// combinations
void SchemaParserTest::testBuffersMisc_data()
{
	addBufferColumns();

	QTest::newRow("allFeaturesTogether") << QString("%set {sep} [, ]\n%if {a} %then <{a}> %if {b} %then {sep} {b} %end %else none %end $br &{x}") << QStringList{ "a=A", "b=B", "x=1<2" } << false << false << QString("<A>, B\n1&lt;2") << -1 << 0 << 0;
	QTest::newRow("ignoreFlagsBothOnUnknown") << QString("[<]{zz}[>]") << QStringList() << true << true << QString("<>") << -1 << 0 << 0;
	QTest::newRow("xmlEntityOfSetValueRoundtrip") << QString("%set {x} &{t}\n&{x}") << QStringList{ "t=<" } << false << false << QString("&lt;") << -1 << 0 << 0;
}

void SchemaParserTest::testBuffersMisc()
{
	checkBuffer();
}

void SchemaParserTest::testRelativeFileNamesAreResolvedAgainstTheCurrentDirectory()
{
	QTemporaryDir dir1, dir2;
	QVERIFY(dir1.isValid() && dir2.isValid());

	writeSchemaFile(dir1.filePath("relative_name_test.sch"), "[from-dir1]\n");
	writeSchemaFile(dir2.filePath("relative_name_test.sch"), "[from-dir2]\n");
	QString orig_cwd = QDir::currentPath(), output1, output2, output3;
	attribs_map attribs;

	try
	{
		// The same relative name must reach a different file when the current directory changes
		QDir::setCurrent(dir1.path());
		SchemaParser p1;
		p1.loadFile("relative_name_test.sch");
		output1 = p1.getSourceCode(attribs);

		QDir::setCurrent(dir2.path());
		SchemaParser p2;
		p2.loadFile("relative_name_test.sch");
		output2 = p2.getSourceCode(attribs);

		// And the same file reached by its absolute name is the one already cached
		SchemaParser p3;
		p3.loadFile(dir1.filePath("relative_name_test.sch"));
		output3 = p3.getSourceCode(attribs);
	}
	catch(Exception &e)
	{
		QDir::setCurrent(orig_cwd);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QDir::setCurrent(orig_cwd);
	QCOMPARE(output1, QString("from-dir1"));
	QCOMPARE(output2, QString("from-dir2"));
	QCOMPARE(output3, QString("from-dir1"));
}

QTEST_MAIN(SchemaParserTest)
#include "schemaparsertest.moc"
