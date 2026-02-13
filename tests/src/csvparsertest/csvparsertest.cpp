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
#include "csvparser.h"
#include "utilsns.h"
#include "exception.h"

class CsvParserTest: public QObject {
	Q_OBJECT

	private slots:
		void testColumnsInFirstRowAndOneRowUnquotedWithoutLastBreak();
		void testOneRowWithoutLineBreak();
		void testRaiseExceptionOnMalformedDocument();
		void testRaiseExceptionWhenRefInvValue();
		void testExtractColumnsInFirstRow();
		void testTwoRowsWithQuotesSeparatorLineBreakInValues();
		void testOneRowWithSimpleAndQuotedValues();
		void testTwoRowsWithSimpleAndQuotedValues();
		void testOneRowWithQuotesInValues();
		void testTwoRowsWithQuotesInValues();
		void testSaveParsedDocumentToFile();
		void testRaiseExceptionOnMissingCloseDelim();
};

void CsvParserTest::testColumnsInFirstRowAndOneRowUnquotedWithoutLastBreak()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		csvparser.setColumnInFirstRow(true);
		buffer = "col1;col2;col3;col4;col5\n;val2;val3;val4;";
		csvdoc = csvparser.parseBuffer(buffer);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testRaiseExceptionWhenRefInvValue()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "value 1;value 2;\"value 3\"\n";
		buffer += "\"value 4\";\"value 5\";\"value 6\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		csvdoc.getValue(0, 1);
		csvdoc.getValue(10, 20);

		QFAIL("Expected exception not thrown!");
	}
	catch(Exception &e)
	{
		QVERIFY(e.getErrorCode() == ErrorCode::RefInvCsvDocumentValue);
	}
}

void CsvParserTest::testRaiseExceptionOnMalformedDocument()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "col_1;col_2;\"col_3\"\n";
		buffer += "value 1;value 2;value 3;value 4; value 5\n";

		csvparser.setSpecialChars(';', '"','\n');
		csvparser.setColumnInFirstRow(true);
		csvdoc = csvparser.parseBuffer(buffer);
		QFAIL("Expected exception not thrown!");
	}
	catch(Exception &e)
	{
		QVERIFY(e.getErrorCode() == ErrorCode::MalformedCsvInvalidCols);
	}
}

void CsvParserTest::testRaiseExceptionOnMissingCloseDelim()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "col_1;col_2;\"col_3\n";
		buffer += "value 1;value 2;value 3\n";

		csvparser.setSpecialChars(';', '"','\n');
		csvparser.setColumnInFirstRow(true);
		csvdoc = csvparser.parseBuffer(buffer);
		QFAIL("Expected exception not thrown!");
	}
	catch(Exception &e)
	{
		QVERIFY(e.getErrorCode() == ErrorCode::MalformedCsvMissingDelim);
	}
}

void CsvParserTest::testExtractColumnsInFirstRow()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "col_1;col_2;\"col_3\"\n";
		buffer += "value 1;value 2;\"value 3\"\n";

		csvparser.setSpecialChars(';', '"','\n');
		csvparser.setColumnInFirstRow(true);
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);

		QStringList cols = csvdoc.getColumnNames();
		QCOMPARE(cols[0], "col_1");
		QCOMPARE(cols[1], "col_2");
		QCOMPARE(cols[2], "col_3");

		QCOMPARE(csvdoc.getRowCount(), 1);

		QCOMPARE(csvdoc.getValue(0, 0), "value 1");
		QCOMPARE(csvdoc.getValue(0, 1), "value 2");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testOneRowWithSimpleAndQuotedValues()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "value 1;value 2;\"value 3\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 1);

		QCOMPARE(csvdoc.getValue(0, 0), "value 1");
		QCOMPARE(csvdoc.getValue(0, 1), "value 2");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testTwoRowsWithSimpleAndQuotedValues()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "value 1;value 2;\"value 3\"\n";
		buffer += "\"value 4\";\"value 5\";\"value 6\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 2);

		QCOMPARE(csvdoc.getValue(0, 0), "value 1");
		QCOMPARE(csvdoc.getValue(0, 1), "value 2");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");

		QCOMPARE(csvdoc.getValue(1, 0), "value 4");
		QCOMPARE(csvdoc.getValue(1, 1), "value 5");
		QCOMPARE(csvdoc.getValue(1, 2), "value 6");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testOneRowWithoutLineBreak()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "value 1;value 2;\"value 3\"";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 1);

		QCOMPARE(csvdoc.getValue(0, 0), "value 1");
		QCOMPARE(csvdoc.getValue(0, 1), "value 2");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testOneRowWithQuotesInValues()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "\"value \"\"1\"\"\";\"value\"\"2\";\"value 3\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 1);

		QCOMPARE(csvdoc.getValue(0, 0), "value \"1\"");
		QCOMPARE(csvdoc.getValue(0, 1), "value\"2");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testTwoRowsWithQuotesInValues()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "value 1;\"\"\" \"\" \"\"\";value 3\n";
		buffer += "\"value 4\";\"\"\"\";\"\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 2);

		QCOMPARE(csvdoc.getValue(0, 0), "value 1");
		QCOMPARE(csvdoc.getValue(0, 1), "\" \" \"");
		QCOMPARE(csvdoc.getValue(0, 2), "value 3");

		QCOMPARE(csvdoc.getValue(1, 0), "value 4");
		QCOMPARE(csvdoc.getValue(1, 1), "\"");
		QCOMPARE(csvdoc.getValue(1, 2), "");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testTwoRowsWithQuotesSeparatorLineBreakInValues()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "\"\"\"quoted\"\"\";\"\"\"quoted + ;\"\"\";\"value \n with break\"\n";
		csvdoc = csvparser.parseBuffer(buffer);

		QCOMPARE(csvdoc.getColumnCount(), 3);
		QCOMPARE(csvdoc.getRowCount(), 1);

		QCOMPARE(csvdoc.getValue(0, 0), "\"quoted\"");
		QCOMPARE(csvdoc.getValue(0, 1), "\"quoted + ;\"");
		QCOMPARE(csvdoc.getValue(0, 2), "value \n with break");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void CsvParserTest::testSaveParsedDocumentToFile()
{
	try
	{
		CsvParser csvparser;
		CsvDocument csvdoc;
		QString buffer;

		buffer = "\"column1\";\"colu\"\"mn2\";\"column__3\"\n";
		buffer += "\"\"\"quoted\"\"\";\"\"\"quoted + ;\"\"\";\"value \n with break\"\n";
		csvparser.setSpecialChars(';', '"', '\n');
		csvparser.setColumnInFirstRow(true);
		csvdoc = csvparser.parseBuffer(buffer);
		csvdoc.saveToFile("test.csv");

		QCOMPARE(UtilsNs::loadFile("test.csv"), buffer);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(CsvParserTest)
#include "csvparsertest.moc"
