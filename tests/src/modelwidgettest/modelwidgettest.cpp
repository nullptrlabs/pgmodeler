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
#include "tableview.h"
#include "cutoffitem.h"
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
		void relocatingTablesTogetherPreservesTheirRelationship();
		void relocatingTableIntoPartnersSceneClearsBothCutoffMarkers();
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
RelationshipView across scenes via the now-removed ObjectsScene::relocateItem() left some of the
view's internal line items (lines/fk_lines/pk_lines/src_cf_lines/dst_cf_lines) with corrupted memory
that only crashed later, when the view was finally destroyed (confirmed with gdb: the bad item's
vtable pointer no longer pointed into Qt's library, not merely a stale scene reference). Multiple
attempts to fix relocateItem() itself (forcing children to re-register with the new scene, via
setParentItem() round-trips and full removeFromGroup()/addToGroup() cycles) failed identically. The
fix discards the old view and constructs a fresh one directly in the destination scene instead --
and, per a later decision to avoid this whole class of Qt-internal hazard everywhere, every item
type relocated between scenes (tables via ModelWidget::relocateObjectView(), relationships via
ModelWidget::routeRelationship()) now follows the same discard+recreate pattern, and
ObjectsScene::relocateItem() itself was deleted. This test reproduces the discard+recreate sequence
directly at the lowest level (scene->removeItem() + scene->addItem(new ...)) and confirms the model
closes cleanly -- see relocatingTablesTogetherPreservesTheirRelationship() below for the same fix
exercised through the real ModelWidget::relocateObjectView() path. */
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

		/* Real usage always relocates the table(s) first (moveToSchema()/relocateEditedObject(), via
		 * ModelWidget::relocateObjectView()) and only then calls relocateRelationshipView() once the
		 * partner already shares the destination scene -- reproduce that same discard+recreate
		 * sequence here for both tables and the relationship */
		QGraphicsItem *table_a_item = dynamic_cast<QGraphicsItem *>(table_a->getOverlyingObject());
		QGraphicsItem *table_b_item = dynamic_cast<QGraphicsItem *>(table_b->getOverlyingObject());

		scene_a->removeItem(table_a_item);
		scene_b->addItem(new TableView(table_a));

		scene_a->removeItem(table_b_item);
		scene_b->addItem(new TableView(table_b));

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

/*! \brief Regression test for a bug found right after the fix above: ModelWidget::
relocateObjectView() discarded and rebuilt a table's TableView, but connected_rels/external_rels
(which relationships the table is tracking) live on the *view instance*, not the model -- so a
brand new TableView always starts with both lists empty. relocateObjectView() is public (also used
directly by moveToSchema()/relocateEditedObject()), so it's exercised here the same way. Two tables
that already share a same-scene relationship (a real RelationshipView) are relocated together, one
after another, into a new schema's scene -- the relationship must survive as a real RelationshipView
in the new scene, not be silently dropped. */
void ModelWidgetTest::relocatingTablesTogetherPreservesTheirRelationship()
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

		QVERIFY(scene_a != nullptr);
		QVERIFY(scene_b != nullptr);
		QVERIFY(scene_a != scene_b);
		QVERIFY(model_wgt.getSceneForObject(rel) == scene_a);

		//Same call moveToSchema()/relocateEditedObject() make for each relocated object, in the same order
		model_wgt.relocateObjectView(table_a, scene_b);
		model_wgt.relocateObjectView(table_b, scene_b);

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_b);
		QCOMPARE(model_wgt.getSceneForObject(table_b), scene_b);

		QGraphicsItem *rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY2(rel_item != nullptr, "relationship lost its view instead of being re-routed after both tables moved together");
		QCOMPARE(rel_item->scene(), scene_b);

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject()),
									 *table_b_view = dynamic_cast<BaseTableView *>(table_b->getOverlyingObject());

		QCOMPARE(table_a_view->getConnectRelsCount(), 1);
		QCOMPARE(table_b_view->getConnectRelsCount(), 1);
		QCOMPARE(table_a_view->getExternalRelsCount(), 0);
		QCOMPARE(table_b_view->getExternalRelsCount(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for a bug the user found manually: relocateObjectView() removed a
relationship from the partner table's external list when it stopped being cross-scene, but never
did the same on the *relocated* table's own old view -- its cut-off marker (a standalone
QGraphicsItem added directly to the scene, not a Qt child of the table's item) was never a Qt child
of anything removeObjectView() touched, so it stayed behind, still rendered in the table's old
scene, for as long as that scene lived. Table A (schema A) and table B (schema B) start with a
cross-scene relationship (external on both, a cut-off marker on each). Relocating table A into
table B's scene turns the relationship into a real RelationshipView -- both cut-off markers must be
gone, and table A's old scene must have nothing left in it. */
void ModelWidgetTest::relocatingTableIntoPartnersSceneClearsBothCutoffMarkers()
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
		table_b->setSchema(schema_b);

		db_model->addTable(table_a);
		db_model->addTable(table_b);

		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
		db_model->addRelationship(rel);

		ObjectsScene *scene_a = model_wgt.getSceneForObject(table_a);
		ObjectsScene *scene_b = model_wgt.getSceneForObject(table_b);

		QVERIFY(scene_a != nullptr);
		QVERIFY(scene_b != nullptr);
		QVERIFY(scene_a != scene_b);

		//Cross-scene at insertion time -- no RelationshipView, a cut-off marker on each table instead
		QVERIFY(rel->getOverlyingObject() == nullptr);

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject()),
									 *table_b_view = dynamic_cast<BaseTableView *>(table_b->getOverlyingObject());

		QVERIFY(table_a_view->getCutoffItem() != nullptr);
		QVERIFY(table_b_view->getCutoffItem() != nullptr);
		QCOMPARE(table_a_view->getCutoffItem()->scene(), static_cast<QGraphicsScene *>(scene_a));
		QCOMPARE(table_b_view->getCutoffItem()->scene(), static_cast<QGraphicsScene *>(scene_b));

		model_wgt.relocateObjectView(table_a, scene_b);

		QGraphicsItem *rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY2(rel_item != nullptr, "relationship should have become a real RelationshipView once both tables shared a scene");
		QCOMPARE(rel_item->scene(), scene_b);

		table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());

		QVERIFY2(table_a_view->getCutoffItem() == nullptr, "table_a's new view still has a cut-off marker");
		QVERIFY2(table_b_view->getCutoffItem() == nullptr, "table_b's cut-off marker was left behind");

		//scene_a still legitimately has schema_a's own SchemaView (this test never reassigns table_a's
		//model-level schema) plus the usual scene infrastructure (rel_line, selection_rect...) -- what
		//must NOT be there anymore is the orphaned CutoffItem
		bool cutoff_left_in_old_scene = false;

		for(auto &item : scene_a->items())
			cutoff_left_in_old_scene |= (dynamic_cast<CutoffItem *>(item) != nullptr);

		QVERIFY2(!cutoff_left_in_old_scene, "table_a's old scene still has the orphaned cut-off marker in it");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(ModelWidgetTest)
#include "modelwidgettest.moc"
