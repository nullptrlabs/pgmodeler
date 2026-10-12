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

#include "coreutilsns.h"
#include "pgmodelerunittest.h"
#include "table.h"
#include <QtTest/QtTest>

class BaseObjectTest : public QObject, public PgModelerUnitTest
{
	Q_OBJECT

public:
	BaseObjectTest() : PgModelerUnitTest(SCHEMASDIR) {}

private slots:
	void quoteNameIfKeyword();
	void nameIsInvalidIfStartsWithNumber();
	void dontFormatNameIfAlreadyQuoted();
	void dropGeneratedExpression_data();
	void dropGeneratedExpression();
};

void BaseObjectTest::quoteNameIfKeyword()
{
  QString name = "objectname", kw_name="table";
  QCOMPARE(BaseObject::formatName(name), name);
  QCOMPARE(BaseObject::formatName(kw_name), QString("\"%1\"").arg(kw_name));
}

void BaseObjectTest::nameIsInvalidIfStartsWithNumber()
{
  QCOMPARE(BaseObject::isValidName("name"), true);
  QCOMPARE(BaseObject::isValidName("nameA"), true);
}

void BaseObjectTest::dontFormatNameIfAlreadyQuoted()
{
	QString name = "\"SomeObjName\"";
	QCOMPARE(BaseObject::formatName(name), name);
}

void BaseObjectTest::dropGeneratedExpression_data()
{
	QTest::addColumn<QString>("default_value");
	QTest::newRow("no default") << QString();
	QTest::newRow("same text as expression") << QString("42");
	QTest::newRow("different default") << QString("7");
}

void BaseObjectTest::dropGeneratedExpression()
{
	QFETCH(QString, default_value);
	Table table;
	table.setName("test_table");
	Column generated_col, regular_col;
	generated_col.setName("test_column");
	generated_col.setType(PgSqlType("integer"));
	generated_col.setParentTable(&table);
	generated_col.setGenerated(true);
	generated_col.setDefaultValue("42");
	regular_col.setName("test_column");
	regular_col.setType(PgSqlType("integer"));
	regular_col.setParentTable(&table);
	regular_col.setDefaultValue(default_value);

	QString sql = generated_col.getAlterCode(&regular_col);
	sql.replace(QString("\n%1\n").arg(Attributes::DdlEndToken), " ");
	QString expected = "ALTER TABLE test_table ALTER COLUMN test_column DROP EXPRESSION;";
	if(!default_value.isEmpty())
		expected += " ALTER TABLE test_table ALTER COLUMN test_column SET DEFAULT " + default_value + ";";
	QCOMPARE(sql.simplified(), expected);
	regular_col.setGenerated(true);
	regular_col.setDefaultValue("42");
	QVERIFY(generated_col.getAlterCode(&regular_col).isEmpty());
	regular_col.setGenerated(false);
	regular_col.setDefaultValue(default_value);
	QString version = BaseObject::getPgSQLVersion();
	BaseObject::setPgSQLVersion("12.0");
	sql = generated_col.getAlterCode(&regular_col);
	BaseObject::setPgSQLVersion(version);
	QVERIFY(sql.isEmpty());
}

QTEST_MAIN(BaseObjectTest)
#include "baseobjecttest.moc"
