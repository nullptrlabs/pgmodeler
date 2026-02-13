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

class DataDictTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		DataDictTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void generateASimpleDataDict();
		void generateASplittedDataDictFromSampleModel();
};

void DataDictTest::generateASimpleDataDict()
{	
	DatabaseModel dbmodel;
	Table *table = nullptr, *table1 = nullptr;
	Schema *schema = nullptr;
	Column *col = nullptr;
	Constraint *constr = nullptr;

	try
	{
		dbmodel.createSystemObjects(true);
		schema = dbmodel.getSchema("public");

		table = new Table;
		table->setName("table_test");
		table->setSchema(schema);

		col = new Column;
		col->setName("col_1");
		col->setType(PgSqlType("integer"));
		col->setDefaultValue("50");
		col->setComment("Some test comment on col_1");
		table->addColumn(col);

		col = new Column;
		col->setName("col_2");
		col->setType(PgSqlType("varchar", 1, 30));
		col->setNotNull(true);
		col->setComment("Some test comment on col_2, now a bit longer.");
		col->setDefaultValue("foo bar");
		table->addColumn(col);

		col = new Column;
		col->setName("col_3");
		col->setType(PgSqlType("text"));
		col->setNotNull(true);
		table->addColumn(col);

		col = new Column;
		col->setName("col_4");
		col->setType(PgSqlType::parseString("timestamp with time zone"));
		col->setDefaultValue("now()");
		table->addColumn(col);

		col = new Column;
		col->setName("col_5");
		col->setType(PgSqlType::parseString("varchar(200)"));
		table->addColumn(col);

		col = new Column;
		col->setName("col_6");
		col->setType(PgSqlType::parseString("numeric(10,6)"));
		table->addColumn(col);

		col = new Column;
		col->setName("col_7");
		col->setType(PgSqlType::parseString("geography(POINTZ, 4296)"));
		table->addColumn(col);

		constr = new Constraint;
		constr->setName("table_test_pk");
		constr->setConstraintType(ConstraintType::PrimaryKey);
		constr->addColumn(table->getColumn("col_1"), Constraint::SourceCols);
		table->addConstraint(constr);

		constr = new Constraint;
		constr->setName("table_test_uq");
		constr->setConstraintType(ConstraintType::Unique);
		constr->addColumn(table->getColumn("col_2"), Constraint::SourceCols);
		constr->addColumn(table->getColumn("col_3"), Constraint::SourceCols);
		constr->setComment("This is a unique constraint");
		table->addConstraint(constr);
		table->setComment("This is some test comment on the table in order to test the data dictionary generation.");

		table1 = new Table;
		table1->setName("table_test1");
		table1->setSchema(schema);

		col = new Column;
		col->setName("col_table_test");
		col->setType(PgSqlType("integer"));
		table1->addColumn(col);

		constr = new Constraint;
		constr->setName("table_test1_fk");
		constr->setConstraintType(ConstraintType::ForeignKey);
		constr->addColumn(table1->getColumn(0), Constraint::SourceCols);
		constr->addColumn(table->getColumn(0), Constraint::ReferencedCols);
		constr->setComment("This is a foreign key constraint");
		constr->setReferencedTable(table);
		table1->addConstraint(constr);
		table1->setComment("This is some test comment on the table in order to test the data dictionary generation.");

		dbmodel.addTable(table);
		dbmodel.addTable(table1);
		dbmodel.saveDataDictionary("./simpledict.html", true, false, false);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void DataDictTest::generateASplittedDataDictFromSampleModel()
{
	DatabaseModel dbmodel;
	try
	{
		dbmodel.createSystemObjects(false);
		dbmodel.loadModel(QString(SAMPLESDIR)+ "/demo.dbm");
		dbmodel.saveDataDictionary("./dict_test.html", true, false, false);
	}
	catch (Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(DataDictTest)
#include "datadicttest.moc"
