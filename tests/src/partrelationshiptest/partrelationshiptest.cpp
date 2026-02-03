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
#include "relationship.h"
#include "pgmodelerunittest.h"
#include "schema.h"

class PartRelationhipTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		PartRelationhipTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void connRelTableNoColumnsAndTableTwoColumns();
		void connRelTablesTwoColumnsSameNameShouldNotRaiseError();
		void connRelTablePartionHasMoreColsThanPartitionedTableShouldRaiseError();
		void connRelTablePartionHasDifferentColsThanPartitionedTableShouldRaiseError();
		void connRelShouldInvalidateIfPartitionedColumnChanges();
};

void PartRelationhipTest::connRelTableNoColumnsAndTableTwoColumns()
{
	Schema *schema = new Schema;
	Table *partitioned = new Table;
	Table *partition = new Table;
	Column *col = new Column, *col1 = new Column;
	Relationship *part_rel = nullptr;

	try
	{
		schema->setName("public");
		partitioned->setSchema(schema);
		partition->setSchema(schema);

		partitioned->setPartitioningType(PartitioningType::List);
		partitioned->setName("table_a");
		partition->setName("partion_a");

		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partitioned->addColumn(col);
		partitioned->addColumn(col1);

		part_rel = new Relationship(BaseRelationship::RelationshipPart, partition, partitioned);
		part_rel->connectRelationship();

		QVERIFY(partitioned->getColumnCount() == partition->getColumnCount());
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void PartRelationhipTest::connRelTablesTwoColumnsSameNameShouldNotRaiseError()
{
	Schema *schema = new Schema;
	Table *partitioned = new Table;
	Table *partition = new Table;
	Column *col = new Column, *col1 = new Column;
	Relationship *part_rel = nullptr;

	try
	{
		schema->setName("public");
		partitioned->setSchema(schema);
		partition->setSchema(schema);

		partitioned->setPartitioningType(PartitioningType::List);
		partitioned->setName("table_a");
		partition->setName("partion_a");

		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partitioned->addColumn(col);
		partitioned->addColumn(col1);

		col = new Column;
		col1 = new Column;
		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partition->addColumn(col);
		partition->addColumn(col1);

		part_rel = new Relationship(BaseRelationship::RelationshipPart, partition, partitioned);
		part_rel->connectRelationship();

		QVERIFY(partitioned->getColumnCount() == partition->getColumnCount());
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

void PartRelationhipTest::connRelTablePartionHasMoreColsThanPartitionedTableShouldRaiseError()
{
	Schema *schema = new Schema;
	Table *partitioned = new Table;
	Table *partition = new Table;
	Column *col = new Column, *col1 = new Column, *col2 = nullptr;
	Relationship *part_rel = nullptr;

	try
	{
		schema->setName("public");
		partitioned->setSchema(schema);
		partition->setSchema(schema);

		partitioned->setPartitioningType(PartitioningType::List);
		partitioned->setName("table_a");
		partition->setName("partion_a");

		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partitioned->addColumn(col);
		partitioned->addColumn(col1);

		col = new Column;
		col1 = new Column;
		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));
		col2 = new Column;
		col2->setName("date");
		col2->setType(PgSqlType("date"));

		partition->addColumn(col);
		partition->addColumn(col1);
		partition->addColumn(col2);

		part_rel = new Relationship(BaseRelationship::RelationshipPart, partition, partitioned);
		part_rel->connectRelationship();

		QFAIL("Should return exception because tables have different column count!");
	}
	catch(Exception &e)
	{
		QVERIFY(e.getErrorCode() == ErrorCode::InvColumnCountPartRel);
	}
}

void PartRelationhipTest::connRelTablePartionHasDifferentColsThanPartitionedTableShouldRaiseError()
{
	Schema *schema = new Schema;
	Table *partitioned = new Table;
	Table *partition = new Table;
	Column *col = new Column, *col1 = new Column;
	Relationship *part_rel = nullptr;

	try
	{
		schema->setName("public");
		partitioned->setSchema(schema);
		partition->setSchema(schema);

		partitioned->setPartitioningType(PartitioningType::List);
		partitioned->setName("table_a");
		partition->setName("partion_a");

		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partitioned->addColumn(col);
		partitioned->addColumn(col1);

		col = new Column;
		col1 = new Column;
		col->setName("tag_id");
		col->setType(PgSqlType("serial"));
		col1->setName("serial_number");
		col1->setType(PgSqlType("smallint"));

		partition->addColumn(col);
		partition->addColumn(col1);

		part_rel = new Relationship(BaseRelationship::RelationshipPart, partition, partitioned);
		part_rel->connectRelationship();

		QFAIL("Should return exception because tables have different columns names!");
	}
	catch(Exception &e)
	{
		QVERIFY(e.getErrorCode() == ErrorCode::InvColumnCountPartRel);
	}
}

void PartRelationhipTest::connRelShouldInvalidateIfPartitionedColumnChanges()
{
	Schema *schema = new Schema;
	Table *partitioned = new Table;
	Table *partition = new Table;
	Column *col = new Column, *col1 = new Column;
	Relationship *part_rel = nullptr;

	try
	{
		schema->setName("public");
		partitioned->setSchema(schema);
		partition->setSchema(schema);

		partitioned->setPartitioningType(PartitioningType::List);
		partitioned->setName("table_a");
		partition->setName("partion_a");

		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partitioned->addColumn(col);
		partitioned->addColumn(col1);

		col = new Column;
		col1 = new Column;
		col->setName("id");
		col->setType(PgSqlType("serial"));
		col1->setName("sku");
		col1->setType(PgSqlType("smallint"));

		partition->addColumn(col);
		partition->addColumn(col1);

		part_rel = new Relationship(BaseRelationship::RelationshipPart, partition, partitioned);
		part_rel->connectRelationship();

		partitioned->getColumn(0)->setName("new_id");
		QVERIFY(part_rel->isInvalidated() == true);

		partitioned->getColumn(0)->setName("id");
		partitioned->getColumn(0)->setType(PgSqlType("date"));
		QVERIFY(part_rel->isInvalidated() == true);
	}
	catch(Exception &e)
	{
		QFAIL(e.getErrorMessage().toStdString().c_str());
	}
}

QTEST_MAIN(PartRelationhipTest)
#include "partrelationshiptest.moc"
