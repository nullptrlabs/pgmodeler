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
#include "operationlist.h"
#include "pgmodelerunittest.h"

/*! \brief In schema-segmentation mode, a Schema and its children must always land in the same
ObjectsScene, since SchemaView::configureObject()
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
		void undoRedoOfTablesMovedTogetherKeepsExactlyOneRelationshipView();
		void undoRedoOfTableWithTwoCutoffRelationshipsLeavesNoOrphanedView();
		void thresholdModeOpensNewSceneAfterCrossingCount();
		void thresholdModeSceneAssignmentSurvivesSaveAndReload();
		void thresholdModeCrossSceneRelationshipGetsCutoffMarkers();
		void thresholdModeDoesNotRebalanceEmptiedScenes();
		void segmentedModesNeverCreateSchemaRectangles();
		void pendingSegmentationChangesDoNotApplyUntilReload();
		void schemaSegmentationScenesComboStaysInSyncWithSchemaNames();
		void pgCatalogChildrenShareItsSceneAfterSchemaSegmentedReload();
		void nonActiveSceneRectsMatchTheirItemsAfterReload();
		void savingWritesLegacySceneRectAsUnionOfAllScenes();
		void lastSceneSurvivesSaveAndReload();
		void perSceneRectsSurviveSaveAndReload();
		void reloadingModelActivatesTheSavedLastScene();
		void invalidLastSceneFallsBackToFirstScene();
		void fase3EndToEndSchemaSegmentedReloadPreservesFocusAndSegmentation();
		void legacyFileWithOnlyLegacySceneRectLoadsWithoutError();
};

void ModelWidgetTest::schemaAndItsTablesShareTheSameScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

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

		ObjectsScene *schema_scene = model_wgt.getSchemaScene(schema_a);

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

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));

		QVERIFY(public_sch != nullptr);
		QVERIFY(model_wgt.getSchemaScene(schema_a) != model_wgt.getSchemaScene(public_sch));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for a SIGSEGV found while testing schema removal: relocating an existing
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

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

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
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
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

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

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
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);

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

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

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

/*! \brief Regression test for a leak the user caught manually with ASan: redoing a moveToSchema()
that relocated two related tables together left an orphaned RelationshipView allocation behind
(LeakSanitizer traced it to ModelWidget::routeRelationship(), called from both the rels and the
ext_rels loop in relocateObjectView() -- see the ASan report attached to this session). Reproduces
moveToSchema()'s exact production sequence for each table in turn -- op_list->registerObject(),
setSchema(), relocateObjectView(), chained via startOperationChain()/finishOperationChain() the same
way moveToSchema() does -- then drives OperationList::undoOperation()/redoOperation() directly,
which is what now reaches ModelWidget::handleObjectSchemaChanged() through
OperationList::s_operationExecuted(). Asserts the relationship ends up with exactly one
RelationshipView in the destination scene after undo+redo, not a duplicate/orphaned one. */
void ModelWidgetTest::undoRedoOfTablesMovedTogetherKeepsExactlyOneRelationshipView()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		OperationList *op_list = model_wgt.getOperationList();

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
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);

		QVERIFY(scene_a != nullptr);
		QVERIFY(scene_b != nullptr);
		QVERIFY(scene_a != scene_b);

		//Same sequence moveToSchema() runs for each selected object, in the same order, chained the same way
		op_list->startOperationChain();

		for(BaseObject *obj : std::vector<BaseObject *>{ table_a, table_b })
		{
			op_list->registerObject(obj, Operation::ObjModified, -1);
			obj->clearDependencies();
			obj->setSchema(schema_b);
			obj->updateDependencies();
			model_wgt.relocateObjectView(dynamic_cast<BaseGraphicObject *>(obj), scene_b);
		}

		op_list->finishOperationChain();

		QGraphicsItem *rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY2(rel_item != nullptr, "relationship should be a real RelationshipView once both tables share scene_b");
		QCOMPARE(rel_item->scene(), scene_b);

		//Undo the whole chain, then redo it -- this is exactly what the user's manual test did when ASan caught the leak
		op_list->undoOperation();

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_a);
		QCOMPARE(model_wgt.getSceneForObject(table_b), scene_a);

		op_list->redoOperation();

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_b);
		QCOMPARE(model_wgt.getSceneForObject(table_b), scene_b);

		rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());

		QVERIFY2(rel_item != nullptr, "relationship lost its view after undo+redo");
		QCOMPARE(rel_item->scene(), scene_b);

		//Exactly one RelationshipView for this relationship must be alive in scene_b -- an orphaned
		//extra one, still allocated but unreachable from rel->getOverlyingObject(), is the leak itself
		int rel_view_count = 0;

		for(auto &item : scene_b->items())
		{
			RelationshipView *rv = dynamic_cast<RelationshipView *>(item);

			if(rv && rv->getUnderlyingObject() == rel)
				rel_view_count++;
		}

		QCOMPARE(rel_view_count, 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test reproducing the user's exact manual repro for the leak in the attached ASan
report: pick a table in schema "public" that has a cut-off marker representing two cross-scene
relationships (both partners already living together in one other schema), move it there (both
relationships become real RelationshipViews), undo (table back in public, both relationships demoted
back to cut-off), then redo -- this last step is not something the user explicitly narrated doing,
but the ASan stack is unambiguous (OperationListWidget::redoOperation() is on it), so it's included
here to match the proven repro rather than the literal narration. Unlike
undoRedoOfTablesMovedTogetherKeepsExactlyOneRelationshipView() above, only the ONE table is
registered/moved (matching moveToSchema() selecting a single table, not two chained together), and
both of its relationships are cut-off (external) rather than one real one external. */
void ModelWidgetTest::undoRedoOfTableWithTwoCutoffRelationshipsLeavesNoOrphanedView()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		//This test exercises schema-segmentation specifically -- opt in explicitly, it's no longer the default
		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		OperationList *op_list = model_wgt.getOperationList();

		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_dest = new Schema;
		schema_dest->setName("schema_dest");
		db_model->addSchema(schema_dest);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_pub);

		Column *col_a = new Column;
		col_a->setName("id");
		col_a->setType(PgSqlType("serial"));
		table_a->addColumn(col_a);

		Constraint *pk = new Constraint;
		pk->setName("table_a_pk");
		pk->setConstraintType(ConstraintType::PrimaryKey);
		pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk);

		//table_b/table_c are table_a's two partners -- both already share schema_dest, unlike table_a
		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(schema_dest);

		Table *table_c = new Table;
		table_c->setName("table_c");
		table_c->setSchema(schema_dest);

		db_model->addTable(table_a);
		db_model->addTable(table_b);
		db_model->addTable(table_c);

		Relationship *rel_ab = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
		db_model->addRelationship(rel_ab);

		Relationship *rel_ac = new Relationship(BaseRelationship::Relationship1n, table_a, table_c);
		db_model->addRelationship(rel_ac);

		ObjectsScene *scene_pub = model_wgt.getSceneForObject(table_a);
		ObjectsScene *scene_dest = model_wgt.getSchemaScene(schema_dest);

		QVERIFY(scene_pub != nullptr);
		QVERIFY(scene_dest != nullptr);
		QVERIFY(scene_pub != scene_dest);

		//Both cross-scene at insertion -- table_a's cut-off marker represents the two of them
		QVERIFY(rel_ab->getOverlyingObject() == nullptr);
		QVERIFY(rel_ac->getOverlyingObject() == nullptr);

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());
		QCOMPARE(table_a_view->getExternalRelsCount(), 2);
		QVERIFY(table_a_view->getCutoffItem() != nullptr);

		//Same call moveToSchema() makes for the single selected object
		op_list->registerObject(table_a, Operation::ObjModified, -1);
		table_a->clearDependencies();
		table_a->setSchema(schema_dest);
		table_a->updateDependencies();
		model_wgt.relocateObjectView(table_a, scene_dest);

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_dest);
		QVERIFY2(dynamic_cast<QGraphicsItem *>(rel_ab->getOverlyingObject()) != nullptr, "rel_ab should have become a real RelationshipView");
		QVERIFY2(dynamic_cast<QGraphicsItem *>(rel_ac->getOverlyingObject()) != nullptr, "rel_ac should have become a real RelationshipView");

		//Undo -- table_a back in public, both relationships demoted back to cut-off
		op_list->undoOperation();

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_pub);
		QVERIFY(rel_ab->getOverlyingObject() == nullptr);
		QVERIFY(rel_ac->getOverlyingObject() == nullptr);

		//User confirmed navigating to scene_dest's tab in between undo and redo -- switch the active scene the same way
		model_wgt.setActiveScene(model_wgt.getObjectsScenes().indexOf(scene_dest));

		//Redo -- table_a moves to schema_dest again, exactly as ASan's stack (OperationList::redoOperation()) proves happened
		op_list->redoOperation();

		QCOMPARE(model_wgt.getSceneForObject(table_a), scene_dest);

		QGraphicsItem *rel_ab_item = dynamic_cast<QGraphicsItem *>(rel_ab->getOverlyingObject()),
									*rel_ac_item = dynamic_cast<QGraphicsItem *>(rel_ac->getOverlyingObject());

		QVERIFY2(rel_ab_item != nullptr, "rel_ab lost its view after undo+redo");
		QVERIFY2(rel_ac_item != nullptr, "rel_ac lost its view after undo+redo");
		QCOMPARE(rel_ab_item->scene(), scene_dest);
		QCOMPARE(rel_ac_item->scene(), scene_dest);

		//Exactly one RelationshipView per relationship must be alive in scene_dest -- an orphaned
		//extra one, still allocated but unreachable from getOverlyingObject(), is the leak itself
		int rel_ab_count = 0, rel_ac_count = 0;

		for(auto &item : scene_dest->items())
		{
			RelationshipView *rv = dynamic_cast<RelationshipView *>(item);

			if(!rv)
				continue;

			if(rv->getUnderlyingObject() == rel_ab)
				rel_ab_count++;
			else if(rv->getUnderlyingObject() == rel_ac)
				rel_ac_count++;
		}

		QCOMPARE(rel_ab_count, 1);
		QCOMPARE(rel_ac_count, 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief In threshold-segmentation mode, the Table/View/ForeignTable insertion counter opens a
new scene only once getSegThreshold() insertions
have landed in the current one -- the insertion that crosses the threshold still belongs to the
old scene, and only the next one after it goes to the freshly-opened scene. */
void ModelWidgetTest::thresholdModeOpensNewSceneAfterCrossingCount()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(public_sch != nullptr);

		//No SchemaView exists in threshold mode -- grab the sole scene created so far directly
		ObjectsScene *first_scene = model_wgt.getObjectsScene(0);
		Table *last_table = nullptr;

		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(public_sch);
			db_model->addTable(table);
			last_table = table;
		}

		QCOMPARE(model_wgt.getSceneForObject(last_table), first_scene);
		QCOMPARE(model_wgt.getObjectsScenes().size(), 2);

		Table *overflow_table = new Table;
		overflow_table->setName("table_overflow");
		overflow_table->setSchema(public_sch);
		db_model->addTable(overflow_table);

		QVERIFY(model_wgt.getSceneForObject(overflow_table) != first_scene);
		QCOMPARE(model_wgt.getObjectsScenes().size(), 2);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Each Table's persisted `scene` attribute must survive a save/reload round-trip and route
it back to the same scene bucket, and
threshold_obj_count must be reseeded from the reloaded data instead of restarting at zero -- otherwise
a single insertion right after reload would open a spurious extra scene instead of resuming the one
already left partially filled by the file. */
void ModelWidgetTest::thresholdModeSceneAssignmentSurvivesSaveAndReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_threshold_reload.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegByThreshold);

			//The saved (pending) mode is what gets persisted -- set it too so the reload below picks it up
			db_model->setSavedSegmentationMode(DatabaseModel::SegByThreshold);

			db_model->createSystemObjects(true);

			Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
			QVERIFY(public_sch != nullptr);

			for(unsigned i = 0; i < db_model->getSegThreshold() + 5; i++)
			{
				Table *table = new Table;
				table->setName(QString("table_%1").arg(i));
				table->setSchema(public_sch);
				db_model->addTable(table);
			}

			QCOMPARE(model_wgt.getObjectsScenes().size(), 2);
			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//The active mode comes from the file's persisted seg-mode attribute now (set above), not from here
			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			QCOMPARE(model_wgt.getObjectsScenes().size(), 2);

			ObjectsScene *last_scene = model_wgt.getObjectsScenes().last();

			//The very first table crosses the threshold count much later -- it stays in the first scene
			Table *table_0 = dynamic_cast<Table *>(db_model->getObject("table_0", ObjectType::Table));
			QVERIFY(table_0 != nullptr);
			QCOMPARE(model_wgt.getSceneForObject(table_0), model_wgt.getObjectsScenes().first());

			//The last table (getSegThreshold() + 4) was the overflow that landed in the second scene
			Table *overflow_table = dynamic_cast<Table *>(db_model->getObject(
					QString("table_%1").arg(db_model->getSegThreshold() + 4), ObjectType::Table));
			QVERIFY(overflow_table != nullptr);
			QCOMPARE(model_wgt.getSceneForObject(overflow_table), last_scene);

			Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
			Table *new_table = new Table;
			new_table->setName("table_after_reload");
			new_table->setSchema(public_sch);
			db_model->addTable(new_table);

			//The threshold counter must resume from the reloaded occupancy, not restart at zero
			QCOMPARE(model_wgt.getObjectsScenes().size(), 2);
			QCOMPARE(model_wgt.getSceneForObject(new_table), last_scene);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief A relationship between two tables that end up in different scenes purely because of
threshold routing must behave exactly like a cross-scene relationship in schema-segmentation mode:
no RelationshipView gets created, and each table draws its own cut-off marker instead. */
void ModelWidgetTest::thresholdModeCrossSceneRelationshipGetsCutoffMarkers()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(public_sch != nullptr);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(public_sch);

		Column *col_a = new Column;
		col_a->setName("id");
		col_a->setType(PgSqlType("serial"));
		table_a->addColumn(col_a);

		Constraint *pk = new Constraint;
		pk->setName("table_a_pk");
		pk->setConstraintType(ConstraintType::PrimaryKey);
		pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk);

		db_model->addTable(table_a);

		//Fill up the first scene so the next table lands in a freshly-opened second one
		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *filler = new Table;
			filler->setName(QString("filler_%1").arg(i));
			filler->setSchema(public_sch);
			db_model->addTable(filler);
		}

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(public_sch);
		db_model->addTable(table_b);

		QVERIFY(model_wgt.getSceneForObject(table_a) != model_wgt.getSceneForObject(table_b));

		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
		db_model->addRelationship(rel);

		QVERIFY2(rel->getOverlyingObject() == nullptr, "cross-scene relationship must not get a RelationshipView");

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject()),
									 *table_b_view = dynamic_cast<BaseTableView *>(table_b->getOverlyingObject());

		QCOMPARE(table_a_view->getConnectRelsCount(), 0);
		QCOMPARE(table_b_view->getConnectRelsCount(), 0);
		QCOMPARE(table_a_view->getExternalRelsCount(), 1);
		QCOMPARE(table_b_view->getExternalRelsCount(), 1);

		QVERIFY(table_a_view->getCutoffItem() != nullptr);
		QVERIFY(table_b_view->getCutoffItem() != nullptr);
		QCOMPARE(table_a_view->getCutoffItem()->getRelationshipsCount(), 1);
		QCOMPARE(table_b_view->getCutoffItem()->getRelationshipsCount(), 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Unlike schema-segmentation mode -- where a scene is destroyed because its owning Schema
was removed -- a threshold-segmentation scene has no owning object whose removal could trigger
destruction. Emptying every table out of a scene by removal must leave the scene itself untouched:
still present in the collection, never merged into another one or destroyed. */
void ModelWidgetTest::thresholdModeDoesNotRebalanceEmptiedScenes()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(public_sch != nullptr);

		std::vector<Table *> tables;

		//Fills the first scene exactly to the threshold, which also opens a second, still-empty scene
		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(public_sch);
			db_model->addTable(table);
			tables.push_back(table);
		}

		QCOMPARE(model_wgt.getObjectsScenes().size(), 2);

		ObjectsScene *first_scene = model_wgt.getObjectsScenes().first();
		ObjectsScene *second_scene = model_wgt.getObjectsScenes().last();
		QVERIFY(first_scene != second_scene);

		//Empties the first scene entirely by removing every table that landed in it
		for(auto &table : tables)
			db_model->removeObject(table);

		//No automatic merge/destruction: both scenes must still be there, same instances as before
		QCOMPARE(model_wgt.getObjectsScenes().size(), 2);
		QVERIFY(model_wgt.getObjectsScenes().contains(first_scene));
		QVERIFY(model_wgt.getObjectsScenes().contains(second_scene));

		//The emptied scene keeps accepting new objects normally, it's just not the routing target
		//anymore -- the counter only ever appends to the last scene, never reuses an earlier one
		Table *new_table = new Table;
		new_table->setName("table_after_removal");
		new_table->setSchema(public_sch);
		db_model->addTable(new_table);

		QCOMPARE(model_wgt.getSceneForObject(new_table), second_scene);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Neither segmented mode instantiates a SchemaView -- not schema-segmentation (where the
rectangle would just mirror the scene it's alone in) nor threshold (where it can't even be drawn
correctly, since a schema's children may be split across scenes). The gate lives entirely in
ModelWidget::handleObjectAddition(); Schema::isRectVisible() itself is untouched by segmentation
mode, so the setting survives a later switch back to NoSegmentation. */
void ModelWidgetTest::segmentedModesNeverCreateSchemaRectangles()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		schema_a->setRectVisible(true);
		db_model->addSchema(schema_a);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_a);
		db_model->addTable(table_a);

		//The schema is still routed to its own scene -- only the rectangle item itself is skipped
		QVERIFY(model_wgt.getSchemaScene(schema_a) != nullptr);
		QVERIFY(schema_a->getOverlyingObject() == nullptr);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(public_sch != nullptr);

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		public_sch->setRectVisible(true);

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(public_sch);
		db_model->addTable(table_b);

		QVERIFY(public_sch->getOverlyingObject() == nullptr);

		/* The flag itself was never touched by segmentation mode -- switching back to NoSegmentation
		must not have lost either schema's rect-visible setting */
		db_model->setSegmentationMode(DatabaseModel::NoSegmentation);
		QVERIFY(schema_a->isRectVisible());
		QVERIFY(public_sch->isRectVisible());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief ModelWidget::setPendingSegmentationMode()/setPendingSceneThreshold() (the slots the
SceneInfoWidget's segmentation combo/threshold spinbox forward user edits to) must only ever touch
DatabaseModel's saved (to-be-persisted) fields -- the active ones governing this session's routing
stay put until the model is actually reloaded, at which point the previously-saved values become
the new active ones. */
void ModelWidgetTest::pendingSegmentationChangesDoNotApplyUntilReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_pending_seg.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(true);

			QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::NoSegmentation);

			model_wgt.setModified(false);
			model_wgt.setPendingSegmentationMode(DatabaseModel::SegByThreshold);
			model_wgt.setPendingSceneThreshold(321);

			//Editing the pending fields must not touch the active session's behavior
			QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::NoSegmentation);
			QCOMPARE(db_model->getSavedSegmentationMode(), DatabaseModel::SegByThreshold);
			QCOMPARE(db_model->getSavedSegThreshold(), 321u);
			QVERIFY(model_wgt.isModified());

			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			//Reloading is what finally makes the previously-saved values become active
			QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::SegByThreshold);
			QCOMPARE(db_model->getSegThreshold(), 321u);
			QCOMPARE(db_model->getSavedSegmentationMode(), DatabaseModel::SegByThreshold);
			QCOMPARE(db_model->getSavedSegThreshold(), 321u);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for two bugs found by the user in schema-segmentation mode:
(1) SceneInfoWidget's scenes_cmb never refreshed the scene name for whichever schema was the last
one processed with no later scene-list-changing event afterward -- typically pg_catalog, since
addScene() emits s_scenesListChanged() before ModelWidget::handleObjectAddition() finishes mapping
the new schema into schema_scenes, and nothing re-emitted afterward once that mapping was complete.
(2) Renaming a schema had no path to the combo at all -- schema-segmentation mode no longer creates
a SchemaView, which used to be what reacted to Schema::s_objectModified(); and even after hooking
ModelWidget up to that same signal, DatabaseModel::validateSchemaRenaming() -- the single choke
point ObjectRenameWidget/SchemaWidget/undo-redo all route a schema rename through -- never actually
called setModified() on the schema itself, only on its children, so the signal never fired. */
void ModelWidgetTest::schemaSegmentationScenesComboStaysInSyncWithSchemaNames()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);

		QStringList last_names;
		int emit_count = 0;

		connect(&model_wgt, &ModelWidget::s_scenesListChanged, [&](){
			last_names = model_wgt.getSceneNames();
			emit_count++;
		});

		//pg_catalog ends up the only (and therefore last) schema created here -- no schema is
		//added afterward whose own scene-list-changing event could incidentally fix a stale entry
		db_model->createSystemObjects(false);

		QVERIFY(emit_count > 0);
		QCOMPARE(last_names, QStringList{ "pg_catalog" });

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		QCOMPARE(last_names, (QStringList{ "pg_catalog", "schema_a" }));

		int emit_count_before_rename = emit_count;

		/* Mirrors the real rename path: ObjectRenameWidget, SchemaWidget and OperationList's
		 * undo/redo of a schema rename all call setName() followed by validateSchemaRenaming() --
		 * setName() alone never emits s_objectModified() */
		QString prev_name = schema_a->getName();
		schema_a->setName("schema_b");
		db_model->validateSchemaRenaming(schema_a, prev_name);

		QVERIFY2(emit_count > emit_count_before_rename, "renaming a schema must refresh the scenes combo");
		QCOMPARE(last_names, (QStringList{ "pg_catalog", "schema_b" }));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for a deeper consequence of the pg_catalog-scene bug: createSystemObjects()
always creates pg_catalog before ModelWidget::loadModel() has read the file's persisted seg-mode
attribute (that only happens inside DatabaseModel::loadModel(), called from within
ModelWidget::loadModel()) -- so pg_catalog's own Schema object is routed while segmentation mode still
reads as NoSegmentation. If it has any children of its own (reverse-engineered tables, rare but
possible), those are parsed later, once the mode is genuinely SegBySchema, and must still find
pg_catalog's scene via getSchemaScene() -- otherwise each one falls through its own safety net and
lands in its own separate orphan scene instead of sharing pg_catalog's. */
void ModelWidgetTest::pgCatalogChildrenShareItsSceneAfterSchemaSegmentedReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_pg_catalog_scene.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);

			db_model->createSystemObjects(true);

			Schema *pg_catalog = dynamic_cast<Schema *>(db_model->getObject("pg_catalog", ObjectType::Schema));
			QVERIFY(pg_catalog != nullptr);

			//Stand-in for a reverse-engineered table living directly in pg_catalog
			Table *table = new Table;
			table->setName("pg_class_stub");
			table->setSchema(pg_catalog);
			db_model->addTable(table);

			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//Mirrors MainWindow::addModel()'s real ordering: system objects (pg_catalog included)
			//are created before the file's own seg-mode attribute is read by loadModel() below
			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::SegBySchema);

			Schema *pg_catalog = dynamic_cast<Schema *>(db_model->getObject("pg_catalog", ObjectType::Schema));
			QVERIFY(pg_catalog != nullptr);

			ObjectsScene *pg_catalog_scene = model_wgt.getSchemaScene(pg_catalog);
			QVERIFY(pg_catalog_scene != nullptr);
			QVERIFY(model_wgt.getSceneNames().contains("pg_catalog"));

			Table *table = dynamic_cast<Table *>(db_model->getObject("pg_class_stub", ObjectType::Table));
			QVERIFY(table != nullptr);

			//The table must share pg_catalog's scene, not fall through to its own orphan scene
			QCOMPARE(model_wgt.getSceneForObject(table), pg_catalog_scene);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.0: ModelWidget::adjustSceneRect(true) (called once at the end
of loadModel()) only ever touched the active scene -- every other scene kept the tiny default rect
from ObjectsScene's constructor, confirmed wrong by manual testing once the model had more than one
scene. adjustAllScenesRects() must give every scene, active or not, a rect that actually bounds its
own items. */
void ModelWidgetTest::nonActiveSceneRectsMatchTheirItemsAfterReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_scene_rects.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(true);

			Schema *schema_a = new Schema;
			schema_a->setName("schema_a");
			db_model->addSchema(schema_a);

			//Placed far from the origin -- the pre-fix default rect (anchored near (0,0)) would never contain it
			Table *table_a = new Table;
			table_a->setName("table_a");
			table_a->setSchema(schema_a);
			table_a->setPosition(QPointF(2000, 2000));
			db_model->addTable(table_a);

			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//public's scene is the one explicitly activated during load (see handleObjectAddition()),
			//so schema_a's scene is guaranteed to be a non-active one here
			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			Schema *schema_a = dynamic_cast<Schema *>(db_model->getObject("schema_a", ObjectType::Schema));
			QVERIFY(schema_a != nullptr);

			ObjectsScene *schema_a_scene = model_wgt.getSchemaScene(schema_a);
			QVERIFY(schema_a_scene != nullptr);
			QVERIFY(schema_a_scene != model_wgt.getObjectsScene());

			Table *table_a = dynamic_cast<Table *>(db_model->getObject("table_a", ObjectType::Table));
			QVERIFY(table_a != nullptr);

			BaseTableView *table_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());
			QVERIFY(table_view != nullptr);

			QVERIFY2(schema_a_scene->sceneRect().contains(table_view->sceneBoundingRect()),
					"a non-active scene's rect must actually bound its own items after reload");
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.0: the model's legacy scene-rect attribute must reflect the
union of every scene's rect, written only when the model is actually saved -- not reactively whenever
any single scene resizes. Before the fix, ObjectsScene::s_sceneRectChanged's handler (wired in
ModelWidget::addScene()) called db_model->setSceneRect() with just that one scene's rect on every
resize, silently corrupting the union with whichever scene last changed. */
void ModelWidgetTest::savingWritesLegacySceneRectAsUnionOfAllScenes()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_scene_rect_union.dbm");

	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_a);
		table_a->setPosition(QPointF(3000, 3000));
		db_model->addTable(table_a);

		Table *table_public = new Table;
		table_public->setName("table_public");
		table_public->setSchema(dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema)));
		db_model->addTable(table_public);

		model_wgt.saveModel(tmp_file);

		QList<ObjectsScene *> all_scenes = model_wgt.getObjectsScenes();
		QVERIFY(all_scenes.size() >= 2);

		QRectF expected_union;

		for(auto &scn : all_scenes)
			expected_union = expected_union.united(scn->sceneRect());

		QCOMPARE(db_model->getSceneRect(), expected_union);

		//A single scene resizing must not reactively overwrite the union with just its own rect
		ObjectsScene *scn = all_scenes.first();
		scn->setSceneRect(scn->sceneRect().adjusted(0, 0, 500, 500));

		QCOMPARE(db_model->getSceneRect(), expected_union);
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.1: LastScene (the focused scene's index) must round-trip
through save/reload. Unlike segmentation mode/threshold, there's no "active vs. saved" split for it --
focusing a scene is an ordinary runtime action, so whatever is read on load becomes active right away.
This subtask is persistence-only (no ModelWidget wiring yet, that's 3.3), so it's exercised directly
on DatabaseModel. */
void ModelWidgetTest::lastSceneSurvivesSaveAndReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_last_scene.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(true);
			QCOMPARE(db_model->getLastScene(), 0);

			db_model->setLastScene(2);
			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			QCOMPARE(db_model->getLastScene(), 2);
		}

		//A file saved without the attribute (pre-3.1) must fall back to 0, not misparse into garbage
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(GlobalAttributes::getSamplesPath() +
													 GlobalAttributes::DirSeparator + QString("demo.dbm"));

			QCOMPARE(db_model->getLastScene(), 0);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.2: a non-active scene's persisted rect must be restored as-is
on reload, taking precedence over the itemsBoundingRect() fallback that subtask 3.0 introduced for
scenes with no persisted entry (old file format, or a brand new scene). Sets a deliberately
distinctive rect -- one itemsBoundingRect() would never compute on its own from the table's actual
position -- so a fallback silently kicking in instead of the real persisted value is unambiguous. */
void ModelWidgetTest::perSceneRectsSurviveSaveAndReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_per_scene_rects.dbm");

	/* ObjectsScene::setSceneRect() clamps the size up to a page-derived minimum, but never touches
	 * the top-left position -- offsetting the position far from where the table actually sits is
	 * therefore what makes this distinctive regardless of that clamping */
	QPointF distinctive_pos(9000, 9000);
	QRectF expected_rect;

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(true);

			Schema *schema_a = new Schema;
			schema_a->setName("schema_a");
			db_model->addSchema(schema_a);

			Table *table_a = new Table;
			table_a->setName("table_a");
			table_a->setSchema(schema_a);
			table_a->setPosition(QPointF(50, 50));
			db_model->addTable(table_a);

			ObjectsScene *schema_a_scene = model_wgt.getSchemaScene(schema_a);
			QVERIFY(schema_a_scene != nullptr);
			QVERIFY(schema_a_scene != model_wgt.getObjectsScene());
			QVERIFY(schema_a_scene->sceneRect().topLeft() != distinctive_pos);

			schema_a_scene->setSceneRect(QRectF(distinctive_pos, schema_a_scene->sceneRect().size()));
			expected_rect = schema_a_scene->sceneRect();
			QCOMPARE(expected_rect.topLeft(), distinctive_pos);

			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			Schema *schema_a = dynamic_cast<Schema *>(db_model->getObject("schema_a", ObjectType::Schema));
			QVERIFY(schema_a != nullptr);

			ObjectsScene *schema_a_scene = model_wgt.getSchemaScene(schema_a);
			QVERIFY(schema_a_scene != nullptr);
			QVERIFY(schema_a_scene != model_wgt.getObjectsScene());

			QCOMPARE(schema_a_scene->sceneRect(), expected_rect);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.3: ModelWidget::saveLastCanvasPosition()/restoreLastCanvasPosition()
must round-trip which scene was active, not just LastScene's raw XML value (already covered by
lastSceneSurvivesSaveAndReload() at the DatabaseModel level) -- this exercises the actual runtime
wiring, i.e. that restoreLastCanvasPosition() really calls setActiveScene() with the persisted index. */
void ModelWidgetTest::reloadingModelActivatesTheSavedLastScene()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_last_scene_wiring.dbm");
	int expected_idx = -1;

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(true);

			Schema *schema_a = new Schema;
			schema_a->setName("schema_a");
			db_model->addSchema(schema_a);

			Table *table_a = new Table;
			table_a->setName("table_a");
			table_a->setSchema(schema_a);
			db_model->addTable(table_a);

			ObjectsScene *schema_a_scene = model_wgt.getSchemaScene(schema_a);
			QVERIFY(schema_a_scene != nullptr);

			expected_idx = model_wgt.getObjectsScenes().indexOf(schema_a_scene);
			QVERIFY(expected_idx > 0);
			QVERIFY(expected_idx != model_wgt.getActiveSceneIndex());

			model_wgt.setActiveScene(expected_idx);
			QCOMPARE(model_wgt.getActiveSceneIndex(), expected_idx);

			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			//loadModel() alone doesn't touch the active scene -- only restoreLastCanvasPosition() does
			model_wgt.restoreLastCanvasPosition();

			QCOMPARE(model_wgt.getActiveSceneIndex(), expected_idx);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for subtask 3.4: a LastScene index that no longer maps to an existing
scene -- e.g. the file was hand-edited, or the scene it pointed to was removed after the save that
wrote it -- must fall back to the first available scene, not just silently leave whatever scene
happened to be active after load. Deliberately activates a non-zero scene first, so a no-op
wouldn't be mistaken for the fallback actually firing. */
void ModelWidgetTest::invalidLastSceneFallsBackToFirstScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_a = new Schema;
		schema_a->setName("schema_a");
		db_model->addSchema(schema_a);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_a);
		db_model->addTable(table_a);

		int scene_count = model_wgt.getObjectsScenes().size();
		QVERIFY(scene_count > 1);

		model_wgt.setActiveScene(scene_count - 1);
		QCOMPARE(model_wgt.getActiveSceneIndex(), scene_count - 1);

		//Simulates a stale index -- e.g. the scene it pointed to was removed after the save that wrote it
		db_model->setLastScene(scene_count + 5);

		model_wgt.restoreLastCanvasPosition();

		QCOMPARE(model_wgt.getActiveSceneIndex(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief End-to-end regression test for Fase 3 (subtasks 3.1-3.4 combined): closing, saving and
reopening a schema-segmented model must preserve the focused scene and its zoom, while the schema
segmentation itself reconstructs correctly on load -- exercising the whole chain (LastScene +
per-scene rects + segmentation routing) together rather than each piece in isolation, as the other
Fase 3 tests already do. */
void ModelWidgetTest::fase3EndToEndSchemaSegmentedReloadPreservesFocusAndSegmentation()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_fase3_e2e.dbm");
	double expected_zoom = 0;

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(true);

			Schema *schema_a = new Schema;
			schema_a->setName("schema_a");
			db_model->addSchema(schema_a);

			Table *table_a = new Table;
			table_a->setName("table_a");
			table_a->setSchema(schema_a);
			db_model->addTable(table_a);

			Schema *schema_b = new Schema;
			schema_b->setName("schema_b");
			db_model->addSchema(schema_b);

			Table *table_b = new Table;
			table_b->setName("table_b");
			table_b->setSchema(schema_b);
			db_model->addTable(table_b);

			ObjectsScene *schema_b_scene = model_wgt.getSchemaScene(schema_b);
			QVERIFY(schema_b_scene != nullptr);

			//Focus a scene that isn't the one made active by default during load (see handleObjectAddition())
			model_wgt.setActiveScene(model_wgt.getObjectsScenes().indexOf(schema_b_scene));
			QCOMPARE(model_wgt.getObjectsScene(), schema_b_scene);

			model_wgt.applyZoom(0.65);
			expected_zoom = model_wgt.getCurrentZoom();

			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//Mirrors MainWindow::addModel()'s real ordering
			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);
			model_wgt.restoreLastCanvasPosition();

			//Schema segmentation reconstructs correctly on load
			Schema *schema_a = dynamic_cast<Schema *>(db_model->getObject("schema_a", ObjectType::Schema));
			Schema *schema_b = dynamic_cast<Schema *>(db_model->getObject("schema_b", ObjectType::Schema));
			QVERIFY(schema_a != nullptr);
			QVERIFY(schema_b != nullptr);

			ObjectsScene *schema_a_scene = model_wgt.getSchemaScene(schema_a);
			ObjectsScene *schema_b_scene = model_wgt.getSchemaScene(schema_b);
			QVERIFY(schema_a_scene != nullptr);
			QVERIFY(schema_b_scene != nullptr);
			QVERIFY(schema_a_scene != schema_b_scene);

			Table *table_a = dynamic_cast<Table *>(db_model->getObject("table_a", ObjectType::Table));
			Table *table_b = dynamic_cast<Table *>(db_model->getObject("table_b", ObjectType::Table));
			QVERIFY(table_a != nullptr);
			QVERIFY(table_b != nullptr);
			QCOMPARE(model_wgt.getSceneForObject(table_a), schema_a_scene);
			QCOMPARE(model_wgt.getSceneForObject(table_b), schema_b_scene);

			//Focused scene preserved
			QCOMPARE(model_wgt.getObjectsScene(), schema_b_scene);

			//Zoom of that scene preserved
			QCOMPARE(model_wgt.getCurrentZoom(), expected_zoom);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Backward-compatibility regression test for Fase 3, explicitly requested by the user:
opening a .dbm saved by a version prior to this feature -- only the legacy scene-rect attribute,
no <scenes> and no last-scene -- must keep working exactly as before. rentacar.dbm (a real sample,
predating both subtasks 2.2/3.2) is used as-is rather than a synthetic fixture, so this exercises
the actual old file format instead of a hand-built approximation of it. */
void ModelWidgetTest::legacyFileWithOnlyLegacySceneRectLoadsWithoutError()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->createSystemObjects(false);
		model_wgt.loadModel(GlobalAttributes::getSamplesPath() +
												 GlobalAttributes::DirSeparator + QString("rentacar.dbm"));

		//No seg-mode/scenes/last-scene attributes in the file -- everything falls back to defaults
		QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::NoSegmentation);
		QCOMPARE(db_model->getSceneRects().size(), 0);
		QCOMPARE(db_model->getLastScene(), 0);

		//A single scene in NoSegmentation mode -- its rect must come from the legacy scene-rect
		//attribute (read by DatabaseModel::loadModel() the same way it always has), not an empty
		//default rect, and with no attempt to redistribute that legacy value across scenes
		QCOMPARE(model_wgt.getObjectsScenes().size(), 1);
		QVERIFY(!model_wgt.getObjectsScene()->sceneRect().isEmpty());

		model_wgt.restoreLastCanvasPosition();
		QCOMPARE(model_wgt.getActiveSceneIndex(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(ModelWidgetTest)
#include "modelwidgettest.moc"
