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
#include "modelwidget.h"
#include "databasemodel.h"
#include "schema.h"
#include "table.h"
#include "objectsscene.h"
#include "pgmodelerunittest.h"

/*! \brief Covers MULTI_SCENE_PLAN.md subtarefa 1.4: in schema-segmentation mode, a Schema and
its children must always land in the same ObjectsScene, since SchemaView::configureObject()
computes its bounding rect directly from its children's positions without ever mapping between
scenes. Table is used as the representative child type here -- Table/View/ForeignTable all
resolve their target scene through the same ModelWidget::getSchemaScene() call. */
class ModelWidgetTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		ModelWidgetTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void schemaAndItsTablesShareTheSameScene();
		void differentSchemasGetDifferentScenes();
};

void ModelWidgetTest::schemaAndItsTablesShareTheSameScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Table *table = new Table;
		table->setName("table_a");
		table->setSchema(schema_a);
		db_model->addTable(table);

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(schema_a);
		db_model->addTable(table_b);

		ObjectsScene *schema_scene = model_wgt.getSceneForObject(schema_a);

		QVERIFY(schema_scene != nullptr);
		QCOMPARE(model_wgt.getSceneForObject(table), schema_scene);
		QCOMPARE(model_wgt.getSceneForObject(table_b), schema_scene);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ModelWidgetTest::differentSchemasGetDifferentScenes()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));

		QVERIFY(public_sch != nullptr);
		QVERIFY(model_wgt.getSceneForObject(schema_a) != model_wgt.getSceneForObject(public_sch));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(ModelWidgetTest)
#include "modelwidgettest.moc"
