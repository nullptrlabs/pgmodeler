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

class DatabaseModelTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		DatabaseModelTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void saveObjectsMetadata();
		void loadObjectsMetadata();
		void saveSplitSQLDefinition();
};

void DatabaseModelTest::saveObjectsMetadata()
{
	DatabaseModel dbmodel;
	QTextStream out(stdout);
	QString output=QFileInfo(BINDIR).absolutePath() + GlobalAttributes::DirSeparator + QString("demo.omf"),
			input=SAMPLESDIR + GlobalAttributes::DirSeparator + QString("demo.dbm");

	try
	{
		QDir dir;
		dir.remove(output);
		dbmodel.createSystemObjects(false);
		dbmodel.loadModel(input);
		dbmodel.saveObjectsMetadata(output);
	}
	catch (Exception &e)
	{
		out << e.getExceptionsText() << Qt::endl;
	}

	QCOMPARE(QFileInfo(output).exists(), true);
}

void DatabaseModelTest::loadObjectsMetadata()
{
	DatabaseModel dbmodel;
	QTextStream out(stdout);
	QString input_opf=QFileInfo(BINDIR).absolutePath() + GlobalAttributes::DirSeparator + QString("demo.omf"),
			input_dbm=SAMPLESDIR + GlobalAttributes::DirSeparator + QString("demo.dbm"),
			output=QFileInfo(BINDIR).absolutePath() + GlobalAttributes::DirSeparator + QString("demo_changed.dbm");

	try
	{
		dbmodel.createSystemObjects(false);
		dbmodel.loadModel(input_dbm);
		dbmodel.loadObjectsMetadata(input_opf);
		dbmodel.saveModel(output, SchemaParser::XmlCode);
		QCOMPARE(true, true);
	}
	catch (Exception &e)
	{
		out << e.getExceptionsText() << Qt::endl;
		QCOMPARE(false, true);
	}
}

void DatabaseModelTest::saveSplitSQLDefinition()
{
	DatabaseModel dbmodel;
	QTextStream out(stdout);
	QString output=QFileInfo(BINDIR).absolutePath() + GlobalAttributes::DirSeparator + "demo_split_test",
			input_dbm=SAMPLESDIR + GlobalAttributes::DirSeparator + QString("demo.dbm");

	try
	{
		QDir dir(output);
		dir.removeRecursively();
		dbmodel.createSystemObjects(false);
		dbmodel.loadModel(input_dbm);
		dbmodel.saveSplitSQLDefinition(output);
		QCOMPARE(true, true);
	}
	catch (Exception &e)
	{
		out << e.getExceptionsText() << Qt::endl;
		QCOMPARE(false, true);
	}
}

QTEST_MAIN(DatabaseModelTest)
#include "databasemodeltest.moc"
