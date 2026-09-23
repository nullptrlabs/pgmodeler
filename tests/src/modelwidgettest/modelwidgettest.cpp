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
#include "column.h"
#include "constraint.h"
#include "relationship.h"
#include "relationshipview.h"
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
		void discardingAndRecreatingRelationshipViewSurvivesModelClose();
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

/*! \brief Regression test for a SIGSEGV found while testing subtarefa 1.10: relocating an existing
RelationshipView across scenes via ObjectsScene::relocateItem() -- as ModelWidget::
relocateRelationshipView() used to do -- leaves some of the view's internal line items
(lines/fk_lines/pk_lines/src_cf_lines/dst_cf_lines) with corrupted memory that only crashes later,
when the view is finally destroyed (confirmed with gdb: the bad item's vtable pointer no longer
points into Qt's library, not merely a stale scene reference). Multiple attempts to fix
relocateItem() itself (forcing children to re-register with the new scene, via setParentItem()
round-trips and full removeFromGroup()/addToGroup() cycles) failed identically. The actual fix
(ModelWidget::relocateRelationshipView()) sidesteps relocateItem() entirely for relationships:
discard the old view and construct a fresh one directly in the destination scene, exactly like
promoteRelationship() already does. This test reproduces that same discard+recreate sequence
directly (relocateRelationshipView() itself is private) and confirms the model closes cleanly. */
void ModelWidgetTest::discardingAndRecreatingRelationshipViewSurvivesModelClose()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_a);

		Column *col_a = new Column;
		col_a->setName("id");
		col_a->setType(PgSqlType("serial"));
		table_a->addColumn(col_a);

		Constraint *pk = new Constraint;
		pk->setName("table_a_pk");
		pk->setConstraintType(ConstraintType::PrimaryKey);
		pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk);

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(schema_a);

		db_model->addTable(table_a);
		db_model->addTable(table_b);

		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
		db_model->addRelationship(rel);

		ObjectsScene *scene_a = model_wgt.getSceneForObject(table_a);
		ObjectsScene *scene_b = model_wgt.getSceneForObject(schema_b);
		QGraphicsItem *rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY(scene_a != nullptr);
		QVERIFY(scene_b != nullptr);
		QVERIFY(scene_a != scene_b);
		QVERIFY(rel_item != nullptr);
		QCOMPARE(rel_item->scene(), scene_a);

		/* Real usage always relocates the table(s) first (moveToSchema()/relocateEditedObject()) and
		 * only then calls relocateRelationshipView() once the partner already shares the destination
		 * scene -- reproduce that same ordering here */
		QGraphicsItem *table_a_item = dynamic_cast<QGraphicsItem *>(table_a->getOverlyingObject());
		QGraphicsItem *table_b_item = dynamic_cast<QGraphicsItem *>(table_b->getOverlyingObject());

		scene_a->relocateItem(table_a_item, scene_b);
		scene_a->relocateItem(table_b_item, scene_b);

		//Same sequence as ModelWidget::relocateRelationshipView(): discard the old view, build a fresh one
		scene_a->removeItem(rel_item);
		scene_b->addItem(new RelationshipView(rel));

		QGraphicsItem *new_rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY(new_rel_item != nullptr);
		QVERIFY(new_rel_item != rel_item);
		QCOMPARE(new_rel_item->scene(), scene_b);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	//model_wgt goes out of scope here -- this used to SIGSEGV inside the stale RelationshipView's destructor
}

QTEST_MAIN(ModelWidgetTest)
#include "modelwidgettest.moc"
