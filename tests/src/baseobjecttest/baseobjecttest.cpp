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
#include <algorithm>
#include "coreutilsns.h"
#include "pgmodelerunittest.h"
#include "table.h"
#include "databasemodel.h"
#include "schema.h"
#include "column.h"
#include "constraint.h"
#include "relationship.h"

class BaseObjectTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		BaseObjectTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void quoteNameIfKeyword();
		void nameIsInvalidIfStartsWithNumber();
		void dontFormatNameIfAlreadyQuoted();
		void dropGeneratedExpression_data();
		void dropGeneratedExpression();
		void getReferencesWithIndirectLinksIncludesTheFullTransitiveClosure();
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

/*! \brief BaseObject::getReferences(true) must return the full transitive closure of everything
that references the object, directly or through another object -- this is what cascade delete
relies on (ModelWidget::removeObjects(true)) to know everything that has to go along with the
object being removed. getReferences() (default, inc_indirect_refs=false) must keep returning only
the direct references, unchanged -- that's the path almost everything else in the codebase uses
for direct-reference checks (e.g. DatabaseModel::__removeObject()'s non-cascade guard). */
void BaseObjectTest::getReferencesWithIndirectLinksIncludesTheFullTransitiveClosure()
{
	try
	{
		DatabaseModel db_model;
		db_model.createSystemObjects(true);

		Schema *schema_x = new Schema;
		schema_x->setName("schema_x");
		db_model.addSchema(schema_x);

		Schema *schema_y = new Schema;
		schema_y->setName("schema_y");
		db_model.addSchema(schema_y);

		Table *table_1 = new Table;
		table_1->setName("table_1");
		table_1->setSchema(schema_x);

		Column *col_1 = new Column;
		col_1->setName("id");
		col_1->setType(PgSqlType("serial"));
		table_1->addColumn(col_1);

		Constraint *pk_1 = new Constraint;
		pk_1->setName("table_1_pk");
		pk_1->setConstraintType(ConstraintType::PrimaryKey);
		pk_1->addColumn(table_1->getColumn("id"), Constraint::SourceCols);
		table_1->addConstraint(pk_1);

		Table *table_2 = new Table;
		table_2->setName("table_2");
		table_2->setSchema(schema_y);

		db_model.addTable(table_1);
		db_model.addTable(table_2);

		//table_1 is the "1" side (has the primary key); the relationship depends on (references) it,
		//not on schema_x -- schema_x only has table_1 as a direct reference
		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_1, table_2);
		db_model.addRelationship(rel);

		std::vector<BaseObject *> direct_refs = schema_x->getReferences();
		QVERIFY(std::find(direct_refs.begin(), direct_refs.end(), table_1) != direct_refs.end());
		QVERIFY2(std::find(direct_refs.begin(), direct_refs.end(), rel) == direct_refs.end(),
						 "direct references of schema_x must not include rel -- rel references table_1, not schema_x");

		std::vector<BaseObject *> indirect_refs = schema_x->getReferences(true);
		QVERIFY(std::find(indirect_refs.begin(), indirect_refs.end(), table_1) != indirect_refs.end());
		QVERIFY2(std::find(indirect_refs.begin(), indirect_refs.end(), rel) != indirect_refs.end(),
						 "getReferences(true) must reach rel too -- it references table_1, which itself references schema_x");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(BaseObjectTest)
#include "baseobjecttest.moc"
