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
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include "modelwidget.h"
#include "crossscenerelswidget.h"
#include "baseform.h"
#include "databasemodel.h"
#include "schema.h"
#include "table.h"
#include "column.h"
#include "constraint.h"
#include "relationship.h"
#include "relationshipview.h"
#include "textbox.h"
#include "tableview.h"
#include "cutoffitem.h"
#include "objectsscene.h"
#include "operationlist.h"
#include "basegraphicobject.h"
#include "taskprogresswidget.h"
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
		void changingThresholdRebucketsScenesOnReload();
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
		void pgCatalogNeverNamesTheSceneOutsideOfSchemaSegmentation();
		void cascadeRemovalOfFocusedSchemaWithCrossSceneChildrenDestroysSceneAndUndoRestoresIt();
		void cascadeDeleteThroughRealActionReachesTransitivelyReferencedObjects();
		void moveToSceneMovesTextboxFreelyEvenInSchemaSegmentationMode();
		void moveToSceneSkipsTableInSchemaSegmentationModeAndAlerts();
		void moveToSceneMovesTableFreelyInThresholdMode();
		void moveToSceneChangeSurvivesUndo();
		void movedTextboxScenePersistsAcrossSaveAndReload();
		void moveToSceneAdjustsDestinationSceneRectForTextbox();
		void moveToSceneAdjustsDestinationSceneRectForTable();
		void moveToSceneUndoRestoresTextboxToItsOriginScene();
		void selectSchemaChildrenSpansMultipleScenesInThresholdMode();
		void switchingActiveSceneNoLongerClearsSelection();
		void handleObjectsMovementOnlyRegistersObjectsFromTheEmittingScene();
		void selectionResetInOneSceneClearsEveryOtherScene();
		void clickOnEmptySceneWithNothingLocallySelectedStillClearsOtherScenes();
		void selectAllObjectsOfTypeSpansMultipleScenes();
		void cancelingRelationshipCreationClearsLogicalSelectionToo();
		void switchingSceneMidRelationshipLineToolCancelsIt();
		void selectingACutoffClearsObjectSelectionAndOtherCutoffs();
		void selectingARegularObjectClearsAnySelectedCutoff();
		void cutoffMarkerCustomPositionSurvivesSaveAndReload();
		void loadModelCanBeCancelledMidLoad();
		void taskProgressWidgetCancelButtonSurvivesShow();
		void rearrangeAlgorithmsRespectSceneScopeInThresholdMode();
		void doubleClickingACutoffEmitsItsTable();
		void closingModelWidgetDestroysItsDatabaseModel();
		void cutoffRelationshipsWidgetListsAndRefreshesRelationships();
		void crossSceneRelsWidgetIgnoresStaleRelationship();
		void crossSceneRelsWidgetSurvivesItsTableBeingRemoved();
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
		model_wgt.setActiveScene(model_wgt.getScenes().indexOf(scene_dest));

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
		ObjectsScene *first_scene = model_wgt.getScene(0);
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
		QCOMPARE(model_wgt.getScenes().size(), 2);

		Table *overflow_table = new Table;
		overflow_table->setName("table_overflow");
		overflow_table->setSchema(public_sch);
		db_model->addTable(overflow_table);

		QVERIFY(model_wgt.getSceneForObject(overflow_table) != first_scene);
		QCOMPARE(model_wgt.getScenes().size(), 2);
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

			QCOMPARE(model_wgt.getScenes().size(), 2);
			db_model->saveModel(tmp_file, SchemaParser::XmlCode);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//The active mode comes from the file's persisted seg-mode attribute now (set above), not from here
			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			QCOMPARE(model_wgt.getScenes().size(), 2);

			ObjectsScene *last_scene = model_wgt.getScenes().last();

			//The very first table crosses the threshold count much later -- it stays in the first scene
			Table *table_0 = dynamic_cast<Table *>(db_model->getObject("table_0", ObjectType::Table));
			QVERIFY(table_0 != nullptr);
			QCOMPARE(model_wgt.getSceneForObject(table_0), model_wgt.getScenes().first());

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
			QCOMPARE(model_wgt.getScenes().size(), 2);
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

/*! \brief Changing seg-threshold and saving without touching any Table/View/ForeignTable leaves
the per-object scene indices AND the per-scene rects on disk still reflecting the OLD threshold --
a plain reload must detect that mismatch, redo the bucketing under the new threshold, and recompute
every scene's rect from its real (now different) contents instead of trusting either stale value.
This guards two bug reports: changing the threshold from the UI silently had no effect on an
already-segmented model no matter how many times it was reloaded, and once that was fixed, tables
merged into a scene whose rect was still the old, narrower one ended up positioned outside it --
unreachable by scrolling, effectively "disappeared". */
void ModelWidgetTest::changingThresholdRebucketsScenesOnReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_threshold_rebucket.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
			db_model->setSavedSegmentationMode(DatabaseModel::SegByThreshold);
			db_model->setSegThreshold(5);
			db_model->setSavedSegThreshold(5);

			db_model->createSystemObjects(true);

			Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
			QVERIFY(public_sch != nullptr);

			/* Each old-scene batch of 5 gets positions far apart from the others -- mirrors how
			 * unrelated scenes end up with unrelated coordinate ranges in a real model, so a stale
			 * rect fitted to only one batch can be shown to provably not contain another's */
			for(unsigned i = 0; i < 12; i++)
			{
				Table *table = new Table;
				table->setName(QString("table_%1").arg(i));
				table->setSchema(public_sch);
				table->setPosition(QPointF((i / 5) * 9000, (i / 5) * 9000));
				db_model->addTable(table);
			}

			//12 tables at threshold 5 -> 3 occupied scenes (5, 5, 2)
			QCOMPARE(model_wgt.getScenes().size(), 3);

			model_wgt.saveModel(tmp_file);

			/* Widen the threshold well past the object count without touching any table -- the scene
			 * indices and per-scene rects already on disk still reflect the threshold of 5 that
			 * produced them */
			model_wgt.setPendingSceneThreshold(1000);
			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			QCOMPARE(db_model->getSegThreshold(), 1000u);

			//12 tables under threshold 1000 all belong in a single scene -- the stale indices must have been discarded
			QCOMPARE(model_wgt.getScenes().size(), 1);

			ObjectsScene *merged_scene = model_wgt.getScenes().first();

			//Every table's real position must fall within the scene's rect, not just the batch a stale persisted rect happened to fit
			for(unsigned i = 0; i < 12; i++)
			{
				Table *table = dynamic_cast<Table *>(db_model->getObject(QString("table_%1").arg(i), ObjectType::Table));
				QVERIFY(table != nullptr);

				QGraphicsItem *item = dynamic_cast<QGraphicsItem *>(table->getOverlyingObject());
				QVERIFY(item != nullptr);

				QVERIFY2(merged_scene->sceneRect().contains(item->sceneBoundingRect()),
								 qPrintable(QString("table_%1 fell outside the merged scene's rect").arg(i)));
			}
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

		QCOMPARE(model_wgt.getScenes().size(), 2);

		ObjectsScene *first_scene = model_wgt.getScenes().first();
		ObjectsScene *second_scene = model_wgt.getScenes().last();
		QVERIFY(first_scene != second_scene);

		//Empties the first scene entirely by removing every table that landed in it
		for(auto &table : tables)
			db_model->removeObject(table);

		//No automatic merge/destruction: both scenes must still be there, same instances as before
		QCOMPARE(model_wgt.getScenes().size(), 2);
		QVERIFY(model_wgt.getScenes().contains(first_scene));
		QVERIFY(model_wgt.getScenes().contains(second_scene));

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
			QVERIFY(schema_a_scene != model_wgt.getScene());

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

		QList<ObjectsScene *> all_scenes = model_wgt.getScenes();
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
			QVERIFY(schema_a_scene != model_wgt.getScene());
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
			QVERIFY(schema_a_scene != model_wgt.getScene());

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

			expected_idx = model_wgt.getScenes().indexOf(schema_a_scene);
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

		int scene_count = model_wgt.getScenes().size();
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
			model_wgt.setActiveScene(model_wgt.getScenes().indexOf(schema_b_scene));
			QCOMPARE(model_wgt.getScene(), schema_b_scene);

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
			QCOMPARE(model_wgt.getScene(), schema_b_scene);

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
		QCOMPARE(model_wgt.getScenes().size(), 1);
		QVERIFY(!model_wgt.getScene()->sceneRect().isEmpty());

		model_wgt.restoreLastCanvasPosition();
		QCOMPARE(model_wgt.getActiveSceneIndex(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for a bug found by the user: pg_catalog is always the first schema
created (createSystemObjects(), before any scene exists and before the file's own seg-mode is
known), so it always falls through handleObjectAddition()'s safety net and gets registered in
schema_scenes -- regardless of what segmentation mode the model actually ends up in. getSceneNames()
used to trust that map unconditionally, so the single scene in NoSegmentation mode (and the first
scene in SegByThreshold mode) ended up displayed as "pg_catalog" instead of the generic name those
modes are supposed to use. Only SegBySchema mode is supposed to name scenes after schemas. */
void ModelWidgetTest::pgCatalogNeverNamesTheSceneOutsideOfSchemaSegmentation()
{
	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			//Mirrors MainWindow::addModel()'s ordering for an existing file (create_public=false) --
			//pg_catalog ends up the only schema created before the mode is known, same as the user's report
			db_model->createSystemObjects(false);

			QCOMPARE(db_model->getSegmentationMode(), DatabaseModel::NoSegmentation);
			QCOMPARE(model_wgt.getSceneNames(), QStringList{ "Default scene" });
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
			db_model->createSystemObjects(false);

			QCOMPARE(model_wgt.getSceneNames(), QStringList{ "Scene 1" });
		}

		//Sanity check: SegBySchema must keep naming scenes after their schema, unaffected by the fix above
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(false);

			QVERIFY(model_wgt.getSceneNames().contains("pg_catalog"));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Fase 4, subtarefa 4.1 -- dedicated regression test for cascade schema removal, covering
the three points flagged in the plan's risk section together: a schema with cross-scene children
(a relationship whose partner lives in another scene, so it's a cut-off marker rather than a real
RelationshipView), removing the schema whose scene is the currently focused one (so
ModelWidget::removeScene() has to switch the viewport away before the scene is destroyed), and a
full undo of the whole cascade chain. Cascade removal itself is driven directly against
DatabaseModel/OperationList here, the same way every other undo/redo test in this file does,
because ModelWidget::removeObjects(true) is the interactive action bound to a menu entry and pops a
confirmation Messagebox -- unusable from an automated test. */
void ModelWidgetTest::cascadeRemovalOfFocusedSchemaWithCrossSceneChildrenDestroysSceneAndUndoRestoresIt()
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

		Schema *schema_doomed = new Schema;
		schema_doomed->setName("schema_doomed");
		db_model->addSchema(schema_doomed);

		Table *table_pub = new Table;
		table_pub->setName("table_pub");
		table_pub->setSchema(schema_pub);

		Column *col_pub = new Column;
		col_pub->setName("id");
		col_pub->setType(PgSqlType("serial"));
		table_pub->addColumn(col_pub);

		Constraint *pk_pub = new Constraint;
		pk_pub->setName("table_pub_pk");
		pk_pub->setConstraintType(ConstraintType::PrimaryKey);
		pk_pub->addColumn(table_pub->getColumn("id"), Constraint::SourceCols);
		table_pub->addConstraint(pk_pub);

		Table *table_doomed = new Table;
		table_doomed->setName("table_doomed");
		table_doomed->setSchema(schema_doomed);

		db_model->addTable(table_pub);
		db_model->addTable(table_doomed);

		//Cross-scene child of schema_doomed: table_pub stays in the public scene, so this relationship
		//never gets a RelationshipView, only cut-off markers on both partners. table_pub is the "1"
		//side (it's the one with a primary key), table_doomed the "n" side.
		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_pub, table_doomed);
		db_model->addRelationship(rel);

		ObjectsScene *scene_pub = model_wgt.getSceneForObject(table_pub);
		ObjectsScene *scene_doomed = model_wgt.getSchemaScene(schema_doomed);

		QVERIFY(scene_pub != nullptr);
		QVERIFY(scene_doomed != nullptr);
		QVERIFY(scene_pub != scene_doomed);
		QVERIFY2(rel->getOverlyingObject() == nullptr, "cross-scene relationship must not get a real RelationshipView");

		//Three scenes exist at this point: public, pg_catalog (createSystemObjects(true) always creates
		//it too) and schema_doomed
		QCOMPARE(model_wgt.getScenes().size(), 3);

		BaseTableView *table_pub_view = dynamic_cast<BaseTableView *>(table_pub->getOverlyingObject());
		QCOMPARE(table_pub_view->getExternalRelsCount(), 1);
		QVERIFY(table_pub_view->getCutoffItem() != nullptr);

		//Focus the scene about to be destroyed -- removeScene() must reroute the viewport away from it
		int scene_doomed_idx = model_wgt.getScenes().indexOf(scene_doomed);
		model_wgt.setActiveScene(scene_doomed_idx);
		QCOMPARE(model_wgt.getScenes().at(model_wgt.getActiveSceneIndex()), scene_doomed);

		/* Cascade removal, leaf to root, mirroring the order ModelWidget::removeObjects(true) itself
		 * removes objects in -- the interactive action isn't used directly here since it pops a
		 * confirmation Messagebox that would block the test */
		op_list->startOperationChain();

		int rel_idx = db_model->getObjectIndex(rel);
		db_model->removeObject(rel, rel_idx);
		op_list->registerObject(rel, Operation::ObjRemoved, rel_idx);

		int table_doomed_idx = db_model->getObjectIndex(table_doomed);
		db_model->removeObject(table_doomed, table_doomed_idx);
		op_list->registerObject(table_doomed, Operation::ObjRemoved, table_doomed_idx);

		int schema_doomed_idx = db_model->getObjectIndex(schema_doomed);
		db_model->removeObject(schema_doomed, schema_doomed_idx);
		op_list->registerObject(schema_doomed, Operation::ObjRemoved, schema_doomed_idx);

		op_list->finishOperationChain();

		//scene_doomed must be gone (public and pg_catalog's scenes remain), and the viewport must have
		//moved off it onto whatever scene remains
		QCOMPARE(model_wgt.getScenes().size(), 2);
		QVERIFY(!model_wgt.getScenes().contains(scene_doomed));
		QCOMPARE(model_wgt.getScenes().at(model_wgt.getActiveSceneIndex()), scene_pub);

		//The cut-off marker on table_pub must be gone too, since its only external relationship was removed
		QCOMPARE(table_pub_view->getExternalRelsCount(), 0);
		QVERIFY(table_pub_view->getCutoffItem() == nullptr);

		//Undo the whole cascade chain in one call -- schema, then table, then relationship, in that order
		op_list->undoOperation();

		Schema *restored_schema = dynamic_cast<Schema *>(db_model->getObject("schema_doomed", ObjectType::Schema));
		QVERIFY2(restored_schema != nullptr, "schema_doomed must exist again after undo");

		Table *restored_table = dynamic_cast<Table *>(db_model->getObject("table_doomed", ObjectType::Table));
		QVERIFY2(restored_table != nullptr, "table_doomed must exist again after undo");

		QCOMPARE(model_wgt.getScenes().size(), 3);

		ObjectsScene *restored_scene = model_wgt.getSchemaScene(restored_schema);
		QVERIFY2(restored_scene != nullptr, "schema_doomed's scene must be recreated by undo, routed through the same handleObjectAddition() choke point as any other insertion");
		QCOMPARE(model_wgt.getSceneForObject(restored_table), restored_scene);

		//The relationship is back too, and still cross-scene -- back to a cut-off marker, not a real RelationshipView
		QVERIFY2(rel->getOverlyingObject() == nullptr, "relationship must be back as a cut-off marker after undo, not a real RelationshipView");
		QCOMPARE(table_pub_view->getExternalRelsCount(), 1);
		QVERIFY(table_pub_view->getCutoffItem() != nullptr);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for the user's real bug report: cascade removal of a schema with
transitively-referenced children (not just direct ones) failed with a chain of RemDirectReference/
RemInderectReference errors, leaving the schema and several of its children behind instead of
removing everything. Root cause was two bugs, fixed in two separate commits: (1)
BaseObject::getLinkedObjects() silently discarded the indirect-links result it had just computed
(baseobjecttest.cpp), and (2) even with that fixed, ModelWidget::removeObjects(true)'s cascade
collection loop called sel_obj->getReferences() with no argument, defaulting to direct-only. This
test exercises the real, interactive ModelWidget::removeObjects(true) slot -- unlike
cascadeRemovalOfFocusedSchemaWithCrossSceneChildrenDestroysSceneAndUndoRestoresIt() above, which
drives DatabaseModel/OperationList directly to get precise control over the undo chain, this one
is specifically about proving the actual user-facing action works, confirmation dialog included.
configurePopupMenu(BaseObject *) is the same call a non-canvas object selection (e.g. a tree-based
object browser) uses to populate selected_objects for a context-menu action without the object
having to be graphically selectable -- schema has no scene item at all in SegBySchema mode. Since
removeObjects() is a private slot, it's invoked here via QMetaObject::invokeMethod(), same as Qt
itself would for any other slot; the confirmation Messagebox it pops (a real, modal QDialog::exec())
is accepted by a QTimer polling for the active modal widget while the call is in flight. */
void ModelWidgetTest::cascadeDeleteThroughRealActionReachesTransitivelyReferencedObjects()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_pub);

		Column *col_a = new Column;
		col_a->setName("id");
		col_a->setType(PgSqlType("serial"));
		table_a->addColumn(col_a);

		Constraint *pk_a = new Constraint;
		pk_a->setName("table_a_pk");
		pk_a->setConstraintType(ConstraintType::PrimaryKey);
		pk_a->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk_a);

		//table_g is schema_b's only direct child; it only references schema_b directly
		Table *table_g = new Table;
		table_g->setName("table_g");
		table_g->setSchema(schema_b);

		db_model->addTable(table_a);
		db_model->addTable(table_g);

		//table_g_copies_table_a references table_g, not schema_b -- two hops away from schema_b,
		//exactly the shape that failed in the user's report
		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_g);
		rel->setName("table_g_copies_table_a");
		db_model->addRelationship(rel);

		QVERIFY(db_model->getObject("schema_b", ObjectType::Schema) != nullptr);
		QVERIFY(db_model->getObject("table_g", ObjectType::Table) != nullptr);

		//Same call a non-canvas object selection uses to populate selected_objects for a context-menu
		//action -- schema_b has no scene item in SegBySchema mode, so it can't be scene-selected.
		//It's a private slot too, so it's invoked the same way removeObjects() is below.
		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(schema_b)));

		//Auto-accepts the confirmation Messagebox(es) removeObjects(true) pops while it's in flight
		QTimer poll_timer;
		poll_timer.setInterval(20);

		QObject::connect(&poll_timer, &QTimer::timeout, [](){
			QDialog *dlg = qobject_cast<QDialog *>(qApp->activeModalWidget());

			if(dlg)
				dlg->accept();
		});

		poll_timer.start();
		QMetaObject::invokeMethod(&model_wgt, "removeObjects", Qt::DirectConnection, Q_ARG(bool, true));
		poll_timer.stop();

		QVERIFY2(db_model->getObject("schema_b", ObjectType::Schema) == nullptr, "schema_b must be gone");
		QVERIFY2(db_model->getObject("table_g", ObjectType::Table) == nullptr, "table_g must be gone too -- cascade must reach it even though it only references table_g, not schema_b, directly");
		QVERIFY2(db_model->getObjectIndex(rel) < 0, "the relationship must be gone as well");
		QVERIFY2(db_model->getObject("table_a", ObjectType::Table) != nullptr, "table_a (schema_b's sibling, unrelated to the cascade) must survive");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Drives ModelWidget::moveToScene() (subtask 4.2) the same way the real "Move to scene"
context-menu entry would, without reaching into ModelWidget's private QMenu machinery:
configurePopupMenu(BaseObject *) (a private slot, invoked via QMetaObject::invokeMethod --
established technique, see cascadeDeleteThroughRealActionReachesTransitivelyReferencedObjects())
selects the object, then a throwaway QAction carrying the destination scene index in its data() is
connected to the private moveToScene() slot via the old string-based SIGNAL/SLOT macros -- this
bypasses C++ access control exactly like invokeMethod does, but through a real
QAction::trigger()/triggered() emission this time, so sender() inside moveToScene() resolves to a
genuine QAction with valid data(), same as the real menu entry would supply. */
static void triggerMoveToScene(ModelWidget &model_wgt, int dst_idx)
{
	QAction move_action;
	move_action.setData(dst_idx);
	QObject::connect(&move_action, SIGNAL(triggered(bool)), &model_wgt, SLOT(moveToScene()));
	move_action.trigger();
}

/*! \brief Textbox has no schema, so schema-segmentation mode's restriction (a Table/View/
ForeignTable's scene must follow its schema) doesn't apply to it -- user-requested scope
restriction, 2026-09-28: it's the one type moveToScene() lets move freely regardless of
segmentation mode. */
void ModelWidgetTest::moveToSceneMovesTextboxFreelyEvenInSchemaSegmentationMode()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		//Textbox has no schema to route it -- it lands wherever the active scene is (public's, here)
		Textbox *txtbox = new Textbox;
		txtbox->setName("note");
		txtbox->setComment("note text");
		db_model->addTextbox(txtbox);

		ObjectsScene *scene_pub = model_wgt.getSchemaScene(schema_pub);
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);

		QVERIFY(scene_pub != nullptr);
		QVERIFY(scene_b != nullptr);
		QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_pub);

		int dst_idx = model_wgt.getScenes().indexOf(scene_b);
		QVERIFY(dst_idx >= 0);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(txtbox)));

		triggerMoveToScene(model_wgt, dst_idx);

		QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_b);
		QCOMPARE(txtbox->getSceneIndex(), dst_idx);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief The other half of the same scope restriction: a Table's scene is pinned to its schema in
schema-segmentation mode (handleObjectAddition()), so moving just the table without its schema
would break that invariant -- moveToScene() must silently skip it and surface an alert instead of
moving it. */
void ModelWidgetTest::moveToSceneSkipsTableInSchemaSegmentationModeAndAlerts()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_pub = new Table;
		table_pub->setName("table_pub");
		table_pub->setSchema(schema_pub);
		db_model->addTable(table_pub);

		ObjectsScene *scene_pub = model_wgt.getSchemaScene(schema_pub);
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);

		QVERIFY(scene_pub != nullptr);
		QVERIFY(scene_b != nullptr);
		QCOMPARE(model_wgt.getSceneForObject(table_pub), scene_pub);

		int dst_idx = model_wgt.getScenes().indexOf(scene_b);
		QVERIFY(dst_idx >= 0);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(table_pub)));

		//Auto-accepts (and records) the alert Messagebox moveToScene() pops when it skips something
		bool alert_seen = false;
		QTimer poll_timer;
		poll_timer.setInterval(20);

		QObject::connect(&poll_timer, &QTimer::timeout, [&alert_seen](){
			QDialog *dlg = qobject_cast<QDialog *>(qApp->activeModalWidget());

			if(dlg)
			{
				alert_seen = true;
				dlg->accept();
			}
		});

		poll_timer.start();
		triggerMoveToScene(model_wgt, dst_idx);
		poll_timer.stop();

		QVERIFY2(alert_seen, "moveToScene() must alert the user that the table was not moved");
		QCOMPARE(model_wgt.getSceneForObject(table_pub), scene_pub);
		QCOMPARE(table_pub->getSceneIndex(), -1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Threshold-segmentation has no schema/scene coupling at all -- every eligible type
(Table/View/ForeignTable, and Textbox) can move freely, matching the user's own description
("moveToScene se aplica a qualquer objeto gráfico" in this mode). */
void ModelWidgetTest::moveToSceneMovesTableFreelyInThresholdMode()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		std::vector<Table *> tables;

		//Fills the first scene exactly to the threshold, which also opens a second, still-empty scene
		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);
			tables.push_back(table);
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		Table *table_first = tables.front();
		ObjectsScene *scene0 = model_wgt.getScenes().at(0);
		ObjectsScene *scene1 = model_wgt.getScenes().at(1);

		QCOMPARE(model_wgt.getSceneForObject(table_first), scene0);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(table_first)));

		triggerMoveToScene(model_wgt, 1);

		QCOMPARE(model_wgt.getSceneForObject(table_first), scene1);
		QCOMPARE(table_first->getSceneIndex(), 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief handleObjectSchemaChanged() (extended by subtask 4.2 to also resync scene-index-driven
placement, not just schema-driven) is what makes undo/redo of moveToScene() actually move the view
back -- OperationList::executeOperation() only restores the object's scene_idx attribute, it
never touches the graphical item on its own. */
/*! \brief A Textbox that lands on its scene through the default routing path (never explicitly
moved before) must still carry that scene's index on the model, not just on the view -- otherwise
an operation that snapshots/restores it later (moveToScene()'s undo) captures a stale/absent index
and can't resolve the object back to a real scene, leaving the view stranded wherever it was. */
void ModelWidgetTest::moveToSceneUndoRestoresTextboxToItsOriginScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Textbox *txtbox = new Textbox;
		txtbox->setName("note");
		txtbox->setComment("note text");
		db_model->addTextbox(txtbox);

		ObjectsScene *scene_pub = model_wgt.getSchemaScene(schema_pub);
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
		int origin_idx = model_wgt.getScenes().indexOf(scene_pub);
		int dst_idx = model_wgt.getScenes().indexOf(scene_b);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(txtbox)));

		triggerMoveToScene(model_wgt, dst_idx);

		QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_b);
		QCOMPARE(txtbox->getSceneIndex(), dst_idx);

		model_wgt.getOperationList()->undoOperation();

		QCOMPARE(txtbox->getSceneIndex(), origin_idx);
		QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_pub);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ModelWidgetTest::moveToSceneChangeSurvivesUndo()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		std::vector<Table *> tables;

		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);
			tables.push_back(table);
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		Table *table_first = tables.front();
		ObjectsScene *scene0 = model_wgt.getScenes().at(0);
		ObjectsScene *scene1 = model_wgt.getScenes().at(1);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(table_first)));

		triggerMoveToScene(model_wgt, 1);

		QCOMPARE(model_wgt.getSceneForObject(table_first), scene1);
		QCOMPARE(table_first->getSceneIndex(), 1);

		model_wgt.getOperationList()->undoOperation();

		QCOMPARE(table_first->getSceneIndex(), 0);
		QCOMPARE(model_wgt.getSceneForObject(table_first), scene0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief The whole point of extending the persisted Scene attribute to Textbox (user's explicit
choice when this subtask was discussed, over leaving it session-only): a manual scene reassignment
must survive save+reload, the same way it already did for Table/View/ForeignTable since subtask
2.2 -- without it, moveToScene() on a Textbox would silently revert on the next load. */
void ModelWidgetTest::movedTextboxScenePersistsAcrossSaveAndReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_textbox_scene.dbm");

	try
	{
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegBySchema);
			db_model->setSavedSegmentationMode(DatabaseModel::SegBySchema);
			db_model->createSystemObjects(true);

			Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
			QVERIFY(schema_pub != nullptr);

			Schema *schema_b = new Schema;
			schema_b->setName("schema_b");
			db_model->addSchema(schema_b);

			Textbox *txtbox = new Textbox;
			txtbox->setName("note");
			txtbox->setComment("note text");
			db_model->addTextbox(txtbox);

			ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
			QVERIFY(scene_b != nullptr);

			int dst_idx = model_wgt.getScenes().indexOf(scene_b);
			QVERIFY(dst_idx >= 0);

			QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
																 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(txtbox)));

			triggerMoveToScene(model_wgt, dst_idx);

			QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_b);
			QCOMPARE(txtbox->getSceneIndex(), dst_idx);

			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			Schema *schema_b = dynamic_cast<Schema *>(db_model->getObject("schema_b", ObjectType::Schema));
			QVERIFY(schema_b != nullptr);

			Textbox *txtbox = dynamic_cast<Textbox *>(db_model->getObject("note", ObjectType::Textbox));
			QVERIFY(txtbox != nullptr);

			ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
			QVERIFY(scene_b != nullptr);

			QVERIFY2(txtbox->getSceneIndex() >= 0, "textbox's manually-assigned scene index must survive reload");
			QCOMPARE(model_wgt.getSceneForObject(txtbox), scene_b);
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief A moved item keeps its own (x, y) -- the destination scene is an independent canvas with
its own, unrelated bounds, so the item can easily fall outside them and render invisible until
something else happens to touch the scene's rect. relocateTextboxView()/relocateObjectView() must
grow the destination scene's rect to include the item right after placing it there. */
void ModelWidgetTest::moveToSceneAdjustsDestinationSceneRectForTextbox()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Textbox *txtbox = new Textbox;
		txtbox->setName("note");
		txtbox->setComment("note text");
		txtbox->setPosition(QPointF(9000, 9000));
		db_model->addTextbox(txtbox);

		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
		QVERIFY(scene_b != nullptr);
		QVERIFY2(!scene_b->sceneRect().contains(txtbox->getPosition()),
						 "schema_b's scene must start out too small to already contain the far-away position");

		int dst_idx = model_wgt.getScenes().indexOf(scene_b);
		QVERIFY(dst_idx >= 0);

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(txtbox)));

		triggerMoveToScene(model_wgt, dst_idx);

		QGraphicsItem *item = dynamic_cast<QGraphicsItem *>(txtbox->getOverlyingObject());
		QVERIFY(item != nullptr);
		QCOMPARE(item->scene(), scene_b);
		QVERIFY2(scene_b->sceneRect().contains(item->sceneBoundingRect()),
						 "schema_b's scene rect must grow to include the relocated textbox");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

//Table/View/ForeignTable counterpart of the above, going through relocateObjectView() instead of
//relocateTextboxView() -- the same gap affected both, since neither called adjustSceneRect()
void ModelWidgetTest::moveToSceneAdjustsDestinationSceneRectForTable()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		//The view reads its initial on-screen position from the model object at creation time --
		//setting it only takes effect for the far-away check below if done before addTable()
		Table *table_first = new Table;
		table_first->setName("table_0");
		table_first->setSchema(schema_pub);
		table_first->setPosition(QPointF(9000, 9000));
		db_model->addTable(table_first);

		for(unsigned i = 1; i < db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		ObjectsScene *scene1 = model_wgt.getScenes().at(1);
		QVERIFY2(!scene1->sceneRect().contains(table_first->getPosition()),
						 "the still-empty second scene must start out too small to already contain the far-away position");

		QMetaObject::invokeMethod(&model_wgt, "configurePopupMenu", Qt::DirectConnection,
															 Q_ARG(BaseObject *, dynamic_cast<BaseObject *>(table_first)));

		triggerMoveToScene(model_wgt, 1);

		QGraphicsItem *item = dynamic_cast<QGraphicsItem *>(table_first->getOverlyingObject());
		QVERIFY(item != nullptr);
		QCOMPARE(item->scene(), scene1);
		QVERIFY2(scene1->sceneRect().contains(item->sceneBoundingRect()),
						 "the second scene's rect must grow to include the relocated table");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief A schema's children can be scattered across several scenes in threshold segmentation --
selectSchemaChildren() must reach all of them, not just whichever happen to sit in the active
scene, since it no longer goes through SchemaView (which has no notion of scenes at all). */
void ModelWidgetTest::selectSchemaChildrenSpansMultipleScenesInThresholdMode()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		std::vector<Table *> tables;

		for(unsigned i = 0; i <= db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);
			tables.push_back(table);
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		//The last table, inserted after the threshold was crossed, must have landed in the second scene
		BaseObjectView *last_view = dynamic_cast<BaseObjectView *>(tables.back()->getOverlyingObject());
		QVERIFY(last_view != nullptr);
		QCOMPARE(last_view->scene(), model_wgt.getScenes().at(1));

		QAction sel_action;
		sel_action.setData(QVariant::fromValue(static_cast<void *>(dynamic_cast<BaseObject *>(schema_pub))));
		QObject::connect(&sel_action, SIGNAL(triggered(bool)), &model_wgt, SLOT(selectSchemaChildren()));
		sel_action.trigger();

		QCOMPARE(model_wgt.getSelectedObjects().size(), tables.size());

		for(auto &table : tables)
		{
			BaseObjectView *view = dynamic_cast<BaseObjectView *>(table->getOverlyingObject());
			QVERIFY(view != nullptr);
			QVERIFY2(view->isSelected(), table->getName().toStdString().c_str());
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Selection now spans every scene at once -- switching the active scene must leave a
previously-selected object (in a scene that just became inactive) still selected. */
void ModelWidgetTest::switchingActiveSceneNoLongerClearsSelection()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_pub = new Table;
		table_pub->setName("table_pub");
		table_pub->setSchema(schema_pub);
		db_model->addTable(table_pub);

		int idx_pub = model_wgt.getScenes().indexOf(model_wgt.getSchemaScene(schema_pub));
		int idx_b = model_wgt.getScenes().indexOf(model_wgt.getSchemaScene(schema_b));

		BaseObjectView *view_pub = dynamic_cast<BaseObjectView *>(table_pub->getOverlyingObject());
		QVERIFY(view_pub != nullptr);

		view_pub->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));

		model_wgt.setActiveScene(idx_b);

		QVERIFY2(view_pub->isSelected(), "switching scenes must not clear a selection made in the scene left behind");
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Dragging is a mouse gesture that only ever happens in the scene currently attached to
the viewport -- an object selected in another scene never receives a mouseMoveEvent and never
actually moves, so it must not be registered on the operation list when the drag ends. */
void ModelWidgetTest::handleObjectsMovementOnlyRegistersObjectsFromTheEmittingScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Table *table_scene0 = nullptr;

		for(unsigned i = 0; i <= db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);

			if(i == 0)
				table_scene0 = table;
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		Table *table_scene1 = dynamic_cast<Table *>(
					db_model->getObject(QString("table_%1").arg(db_model->getSegThreshold()), ObjectType::Table));
		QVERIFY(table_scene1 != nullptr);

		ObjectsScene *scene0 = model_wgt.getScenes().at(0);
		ObjectsScene *scene1 = model_wgt.getScenes().at(1);

		BaseObjectView *view0 = dynamic_cast<BaseObjectView *>(table_scene0->getOverlyingObject());
		BaseObjectView *view1 = dynamic_cast<BaseObjectView *>(table_scene1->getOverlyingObject());

		QVERIFY(view0 != nullptr && view1 != nullptr);
		QCOMPARE(view0->scene(), scene0);
		QCOMPARE(view1->scene(), scene1);

		view0->setSelected(true);
		view1->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(2));

		int idx_before = model_wgt.getOperationList()->getCurrentIndex();

		QMetaObject::invokeMethod(&model_wgt, "handleObjectsMovement", Qt::DirectConnection,
															 Q_ARG(ObjectsScene *, scene0), Q_ARG(bool, false));
		QMetaObject::invokeMethod(&model_wgt, "handleObjectsMovement", Qt::DirectConnection,
															 Q_ARG(ObjectsScene *, scene0), Q_ARG(bool, true));

		int idx_after = model_wgt.getOperationList()->getCurrentIndex();

		//Only table_scene0 (the scene that "emitted" the drag) must have been registered -- not table_scene1
		QCOMPARE(idx_after - idx_before, 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief A click that resets one scene's own selection (empty area, or a plain click replacing a
previous selection) must clear every other scene's selection too, not just the one that was clicked. */
void ModelWidgetTest::selectionResetInOneSceneClearsEveryOtherScene()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_pub = new Table;
		table_pub->setName("table_pub");
		table_pub->setSchema(schema_pub);
		db_model->addTable(table_pub);

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(schema_b);
		db_model->addTable(table_b);

		BaseObjectView *view_pub = dynamic_cast<BaseObjectView *>(table_pub->getOverlyingObject());
		BaseObjectView *view_b = dynamic_cast<BaseObjectView *>(table_b->getOverlyingObject());
		QVERIFY(view_pub != nullptr && view_b != nullptr);

		view_pub->setSelected(true);
		view_b->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(2));

		//Mirrors what ObjectsScene::mousePressEvent() emits when a click resets its own selection
		emit model_wgt.getSceneForObject(table_pub)->s_selectionResetTriggered();

		QVERIFY2(!view_b->isSelected(), "a selection reset in one scene must clear every other scene too");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief The reset-detection inside ObjectsScene::mousePressEvent() must not be gated on this
scene's own selectedItems() being non-empty -- a scene with nothing selected locally (because the
selection lives entirely in some OTHER scene) still needs a plain click on its empty area to clear
that other scene's selection. A local-only check would never see anything to reset and silently
leave it behind, which is exactly the bug the user found manually. */
void ModelWidgetTest::clickOnEmptySceneWithNothingLocallySelectedStillClearsOtherScenes()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_pub = new Table;
		table_pub->setName("table_pub");
		table_pub->setSchema(schema_pub);
		db_model->addTable(table_pub);

		BaseObjectView *view_pub = dynamic_cast<BaseObjectView *>(table_pub->getOverlyingObject());
		QVERIFY(view_pub != nullptr);

		view_pub->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));

		//Switches to schema_b's scene, which never had anything selected in it at all
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
		int idx_b = model_wgt.getScenes().indexOf(scene_b);
		model_wgt.setActiveScene(idx_b);

		QVERIFY(scene_b->selectedItems().isEmpty());

		//A plain left click on an empty area of scene_b, delivered the same way Qt would from a real click
		QGraphicsSceneMouseEvent press_event(QEvent::GraphicsSceneMousePress);
		press_event.setScenePos(QPointF(9000, 9000));
		press_event.setButton(Qt::LeftButton);
		press_event.setButtons(Qt::LeftButton);
		QCoreApplication::sendEvent(scene_b, &press_event);

		QVERIFY2(!view_pub->isSelected(),
						 "a reset click on a scene with nothing selected locally must still clear other scenes");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief "Select all objects of type X" is, by nature, a whole-model operation -- restricting it
to the active scene would silently leave out every table routed to another scene in threshold
segmentation, the same kind of gap selectSchemaChildren() had before it was reformulated. */
void ModelWidgetTest::selectAllObjectsOfTypeSpansMultipleScenes()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		std::vector<Table *> tables;

		for(unsigned i = 0; i <= db_model->getSegThreshold(); i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);
			db_model->addTable(table);
			tables.push_back(table);
		}

		QCOMPARE(model_wgt.getScenes().size(), 2);

		BaseObjectView *last_view = dynamic_cast<BaseObjectView *>(tables.back()->getOverlyingObject());
		QVERIFY(last_view != nullptr);
		QCOMPARE(last_view->scene(), model_wgt.getScenes().at(1));

		QAction sel_action;
		sel_action.setData(static_cast<unsigned>(ObjectType::Table));
		QObject::connect(&sel_action, SIGNAL(triggered(bool)), &model_wgt, SLOT(selectAllObjects()));
		sel_action.trigger();

		QCOMPARE(model_wgt.getSelectedObjects().size(), tables.size());

		for(auto &table : tables)
		{
			BaseObjectView *view = dynamic_cast<BaseObjectView *>(table->getOverlyingObject());
			QVERIFY(view != nullptr);
			QVERIFY2(view->isSelected(), table->getName().toStdString().c_str());
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Regression test for a bug the user found manually: with a single table selected,
triggering "New > Relationship > 1:N" and then clicking a second table on canvas opens the
relationship form through ModelWidget::configureObjectSelection()'s blockSignals(true)/
showObjectForm()/blockSignals(false) window. Canceling that form left both tables visually
deselected but ModelWidget::selected_objects (feeding SceneInfoWidget) stale with both tables --
the resync used to rely on a selection-changed signal that window silences. */
void ModelWidgetTest::cancelingRelationshipCreationClearsLogicalSelectionToo()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::NoSegmentation);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

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

		Table *table_b = new Table;
		table_b->setName("table_b");
		table_b->setSchema(schema_pub);

		db_model->addTable(table_a);
		db_model->addTable(table_b);

		BaseObjectView *view_a = dynamic_cast<BaseObjectView *>(table_a->getOverlyingObject());
		BaseObjectView *view_b = dynamic_cast<BaseObjectView *>(table_b->getOverlyingObject());
		QVERIFY(view_a != nullptr && view_b != nullptr);

		//Only table_a is selected when the user triggers "New > Relationship > 1:N"
		view_a->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));

		//Mirrors triggering the "1:N" action from the "New > Relationship" submenu (addNewObject())
		QAction rel_action;
		rel_action.setData(static_cast<unsigned>(ObjectType::Relationship) +
												static_cast<unsigned>(BaseRelationship::Relationship1n));
		QObject::connect(&rel_action, SIGNAL(triggered(bool)), &model_wgt, SLOT(addNewObject()));
		rel_action.trigger();

		//Auto-cancels the relationship form (Esc/Cancel) as soon as it appears
		QTimer poll_timer;
		poll_timer.setInterval(20);

		QObject::connect(&poll_timer, &QTimer::timeout, [](){
			QDialog *dlg = qobject_cast<QDialog *>(qApp->activeModalWidget());

			if(dlg)
				dlg->reject();
		});

		poll_timer.start();

		//Picking the second table on canvas -- same signal chain a real click on it would go through
		view_b->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);

		poll_timer.stop();

		QVERIFY2(!view_a->isSelected(), "table_a must end up visually deselected after the cancel");
		QVERIFY2(!view_b->isSelected(), "table_b must end up visually deselected after the cancel");
		QVERIFY2(model_wgt.getSelectedObjects().empty(),
						 "canceling relationship creation must clear the logical selection too, not just the visual one");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Subtask 4.3 (deferred, then judged unnecessary once 4.4 shipped): pre-selecting both
tables across scenes -- Ctrl+click, switching scenes freely in between -- and triggering "New >
Relationship" already works with no scene awareness at all, since addNewObject()/showObjectForm()
only ever look at selected_objects[0]/[1]. The one gesture that still can't survive a scene switch
is the interactive line tool (click table A, drag/click table B): rel_line lives in the old
ObjectsScene and only that scene gets mouse events while attached to the viewport, so switching
scenes mid-gesture must cancel it instead of leaving it stuck pointing at an unreachable table. */
void ModelWidgetTest::switchingSceneMidRelationshipLineToolCancelsIt()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegBySchema);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		Schema *schema_b = new Schema;
		schema_b->setName("schema_b");
		db_model->addSchema(schema_b);

		Table *table_a = new Table;
		table_a->setName("table_a");
		table_a->setSchema(schema_pub);
		db_model->addTable(table_a);

		BaseObjectView *view_a = dynamic_cast<BaseObjectView *>(table_a->getOverlyingObject());
		QVERIFY(view_a != nullptr);

		//Only table_a is selected when the user triggers "New > Relationship > 1:N"
		view_a->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));

		//Mirrors triggering the "1:N" action from the "New > Relationship" submenu (addNewObject())
		QAction rel_action;
		rel_action.setData(static_cast<unsigned>(ObjectType::Relationship) +
												static_cast<unsigned>(BaseRelationship::Relationship1n));
		QObject::connect(&rel_action, SIGNAL(triggered(bool)), &model_wgt, SLOT(addNewObject()));
		rel_action.trigger();

		ObjectsScene *scene_pub = model_wgt.getSceneForObject(table_a);
		QVERIFY(scene_pub != nullptr);
		QVERIFY2(scene_pub->isRelationshipLineVisible(),
						 "the preview line must be showing after picking table_a as the source");

		//User switches scenes mid-gesture, before clicking a second table
		ObjectsScene *scene_b = model_wgt.getSchemaScene(schema_b);
		int idx_b = model_wgt.getScenes().indexOf(scene_b);
		model_wgt.setActiveScene(idx_b);

		QVERIFY2(!scene_pub->isRelationshipLineVisible(),
						 "switching scenes mid-gesture must cancel the pending relationship-line tool");
		QCOMPARE(model_wgt.getViewport()->cursor().shape(), Qt::ArrowCursor);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief User-requested invariant: a cutoff marker's selection must be exclusive -- selecting one
must clear any regular object selection, and only one cutoff may be selected at a time (no mixed
selection of tables/relationships/cutoffs). Covers CutoffItem::itemChange()'s side of the rule. */
void ModelWidgetTest::selectingACutoffClearsObjectSelectionAndOtherCutoffs()
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

		//Fill up the first scene so table_b lands in a freshly-opened second one
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

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject()),
									 *table_b_view = dynamic_cast<BaseTableView *>(table_b->getOverlyingObject());

		QVERIFY(table_a_view->getCutoffItem() != nullptr);
		QVERIFY(table_b_view->getCutoffItem() != nullptr);

		Table *filler_0 = dynamic_cast<Table *>(db_model->getObject("filler_0", ObjectType::Table));
		BaseObjectView *filler_view = dynamic_cast<BaseObjectView *>(filler_0->getOverlyingObject());
		QVERIFY(filler_view != nullptr);

		//Selects a regular object first, in the same scene table_a's cutoff lives in
		filler_view->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));

		//Selects table_a's cutoff marker
		table_a_view->getCutoffItem()->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);

		QVERIFY2(!filler_view->isSelected(), "selecting a cutoff must clear the regular object selection");
		QVERIFY2(model_wgt.getSelectedObjects().empty(),
						 "selecting a cutoff must clear ModelWidget::selected_objects too");
		QVERIFY2(table_a_view->getCutoffItem()->isSelected(), "the clicked cutoff must remain selected");

		//Selects table_b's cutoff marker (a different scene) -- only one cutoff may stay selected
		table_b_view->getCutoffItem()->setSelected(true);

		QVERIFY2(!table_a_view->getCutoffItem()->isSelected(), "selecting a second cutoff must deselect the first one");
		QVERIFY2(table_b_view->getCutoffItem()->isSelected(), "the newly selected cutoff must remain selected");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief The opposite direction of the same invariant: selecting a regular object must clear any
cutoff marker currently selected, even one sitting in a different scene (selection has spanned every
scene since 4.4, so the cutoff can outlive the scene that held it losing focus). Covers
ModelWidget::configureObjectSelection()'s side of the rule. */
void ModelWidgetTest::selectingARegularObjectClearsAnySelectedCutoff()
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

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject()),
									 *table_b_view = dynamic_cast<BaseTableView *>(table_b->getOverlyingObject());

		QVERIFY(table_a_view->getCutoffItem() != nullptr);

		table_a_view->getCutoffItem()->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);
		QVERIFY(table_a_view->getCutoffItem()->isSelected());

		//table_b lives in a different scene than table_a's cutoff
		table_b_view->setSelected(true);
		QMetaObject::invokeMethod(&model_wgt, "configureObjectSelection", Qt::DirectConnection);

		QVERIFY2(!table_a_view->getCutoffItem()->isSelected(),
						 "selecting a regular object in another scene must still clear a selected cutoff marker");
		QCOMPARE(model_wgt.getSelectedObjects().size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief User-reported bug: dragging a cutoff marker to a custom position and saving the model
doesn't restore that position on reload. */
void ModelWidgetTest::cutoffMarkerCustomPositionSurvivesSaveAndReload()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_cutoff_pos.dbm");

	try
	{
		QPointF saved_pos;

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
			db_model->setSavedSegmentationMode(DatabaseModel::SegByThreshold);
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

			//Fill up the first scene so table_b lands in a freshly-opened second one
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

			Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
			db_model->addRelationship(rel);

			BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());
			QVERIFY(table_a_view->getCutoffItem() != nullptr);

			//Drags the cutoff marker to a custom position, same as a real drag would via itemChange(ItemPositionHasChanged)
			QPointF custom_pos = table_a_view->getCutoffItem()->pos() + QPointF(40, -25);
			table_a_view->getCutoffItem()->setPos(custom_pos);
			saved_pos = table_a_view->getCutoffItem()->pos();

			model_wgt.saveModel(tmp_file);
		}

		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();

			db_model->createSystemObjects(false);
			model_wgt.loadModel(tmp_file);

			Table *table_a = dynamic_cast<Table *>(db_model->getObject("table_a", ObjectType::Table));
			QVERIFY(table_a != nullptr);

			BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());
			QVERIFY2(table_a_view->getCutoffItem() != nullptr, "table_a's cutoff marker must survive reload");

			QPointF reloaded_pos = table_a_view->getCutoffItem()->pos();
			bool close_enough = std::abs(reloaded_pos.x() - saved_pos.x()) < 1.0 &&
													 std::abs(reloaded_pos.y() - saved_pos.y()) < 1.0;

			QVERIFY2(close_enough,
							 qPrintable(QString("saved (%1, %2) vs reloaded (%3, %4)")
													.arg(saved_pos.x()).arg(saved_pos.y())
													.arg(reloaded_pos.x()).arg(reloaded_pos.y())));
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Covers Fase 4.12: the three "Rearrange objects" algorithms restricted to a single scene
 * (SegByThreshold, where a schema doesn't map to a scene at all -- the trickiest of the three
 * segmentation modes for them, since rearrangeSchemasInGrid()/rearrangeTablesInSchemas() have no
 * SchemaView to work with there and fall back to a flat per-scene arrangement). Confirms none of
 * the three crash in that fallback, and that current_scene_only=true never touches the other
 * scene's tables -- including via a cross-scene relationship, which rearrangeTablesHierarchically()
 * must treat as not connected (its partner has no same-scene RelationshipView to traverse). */
void ModelWidgetTest::rearrangeAlgorithmsRespectSceneScopeInThresholdMode()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->setSegThreshold(3);
		db_model->createSystemObjects(true);

		Schema *schema_pub = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		QVERIFY(schema_pub != nullptr);

		std::vector<Table *> tables;

		for(unsigned i = 0; i < 5; i++)
		{
			Table *table = new Table;
			table->setName(QString("table_%1").arg(i));
			table->setSchema(schema_pub);

			Column *col = new Column;
			col->setName("id");
			col->setType(PgSqlType("serial"));
			table->addColumn(col);

			Constraint *pk = new Constraint;
			pk->setName(QString("table_%1_pk").arg(i));
			pk->setConstraintType(ConstraintType::PrimaryKey);
			pk->addColumn(table->getColumn("id"), Constraint::SourceCols);
			table->addConstraint(pk);

			db_model->addTable(table);
			tables.push_back(table);
		}

		//Threshold 3, 5 tables: table_0/1/2 land in the first scene, table_3/4 in the second
		QCOMPARE(model_wgt.getScenes().size(), 2);

		ObjectsScene *scene0 = model_wgt.getScenes().at(0);
		ObjectsScene *scene1 = model_wgt.getScenes().at(1);

		QCOMPARE(model_wgt.getSceneForObject(tables[0]), scene0);
		QCOMPARE(model_wgt.getSceneForObject(tables[3]), scene1);

		//Same-scene relationship, exercises the normal traversal
		Relationship *rel_same_scene = new Relationship(BaseRelationship::Relationship1n, tables[0], tables[1]);
		db_model->addRelationship(rel_same_scene);

		//Cross-scene (cut-off) relationship -- table_3 must never be reachable while scoped to scene0
		Relationship *rel_cross_scene = new Relationship(BaseRelationship::Relationship1n, tables[2], tables[3]);
		db_model->addRelationship(rel_cross_scene);

		QPointF table3_pos_before = dynamic_cast<BaseObjectView *>(tables[3]->getOverlyingObject())->pos();
		QPointF table4_pos_before = dynamic_cast<BaseObjectView *>(tables[4]->getOverlyingObject())->pos();

		model_wgt.setActiveScene(0);

		model_wgt.rearrangeTablesHierarchically(true);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[3]->getOverlyingObject())->pos(), table3_pos_before);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[4]->getOverlyingObject())->pos(), table4_pos_before);

		model_wgt.rearrangeSchemasInGrid(true);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[3]->getOverlyingObject())->pos(), table3_pos_before);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[4]->getOverlyingObject())->pos(), table4_pos_before);

		model_wgt.rearrangeTablesInSchemas(true);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[3]->getOverlyingObject())->pos(), table3_pos_before);
		QCOMPARE(dynamic_cast<BaseObjectView *>(tables[4]->getOverlyingObject())->pos(), table4_pos_before);

		//"All scenes" mode: every scene is scoped internally, one at a time -- just needs to not crash
		model_wgt.rearrangeTablesInSchemas(false);
		model_wgt.rearrangeSchemasInGrid(false);
		model_wgt.rearrangeTablesHierarchically(false);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Covers the new cooperative cancellation support in DatabaseModel::loadModel(): the
 * is_canceled callback, checked once per object right after each s_objectLoaded tick, aborts the
 * load by throwing ErrorCode::OperationCancelled. Exercises the mechanism directly (a plain
 * callback, no TaskProgressWidget/GUI click involved -- that part isn't reachable from a test,
 * since ModelWidget::loadModel() keeps its TaskProgressWidget as a private local). Also covers the
 * pre-existing bug this change piggybacked a fix for: the outer catch in loadModel() used to leave
 * BaseGraphicObject's updates-enabled flag stuck at false on ANY load failure, not just a cancelled
 * one -- which would have broken rendering for every model loaded after this one, in this process */
void ModelWidgetTest::loadModelCanBeCancelledMidLoad()
{
	QString tmp_file = QDir::temp().absoluteFilePath("modelwidgettest_cancel_load.dbm");

	try
	{
		//Build and save a small model with a few tables to load back
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();
			db_model->createSystemObjects(true);

			Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
			QVERIFY(public_sch != nullptr);

			for(unsigned i = 0; i < 5; i++)
			{
				Table *table = new Table;
				table->setName(QString("table_%1").arg(i));
				table->setSchema(public_sch);

				Column *col = new Column;
				col->setName("id");
				col->setType(PgSqlType("serial"));
				table->addColumn(col);

				db_model->addTable(table);
			}

			model_wgt.saveModel(tmp_file);
		}

		//Reload it with a callback that requests cancellation on the very first object seen
		{
			ModelWidget model_wgt;
			DatabaseModel *db_model = model_wgt.getDatabaseModel();
			db_model->createSystemObjects(false);

			QVERIFY2(BaseGraphicObject::isUpdatesEnabled(), "sanity: must start enabled, or the assertion below proves nothing");

			bool threw = false;
			ErrorCode caught_code = ErrorCode::Custom;

			try
			{
				db_model->loadModel(tmp_file, false, DatabaseModel::NoSegmentation, 0, false,
														 [](){ return true; });
			}
			catch(Exception &e)
			{
				threw = true;
				caught_code = e.getErrorCode();
			}

			QVERIFY2(threw, "cancelling mid-load must throw, not just stop silently");
			QCOMPARE(enum_t(caught_code), enum_t(ErrorCode::OperationCancelled));

			//Aborted on the first object -- the model must be left incomplete, not all 5 tables in it
			QVERIFY(db_model->getObjectList(ObjectType::Table)->size() < 5);

			QVERIFY2(BaseGraphicObject::isUpdatesEnabled(),
								"updates must be re-enabled even when the load was cancelled, not just on success");
		}
	}
	catch(Exception &e)
	{
		QFile::remove(tmp_file);
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}

	QFile::remove(tmp_file);
}

/*! \brief Regression test for a real bug: show() used to re-derive cancel_btn's enabled state from
 * cancel_btn->isVisible(), but that call lands before QDialog::show() actually makes the dialog (and
 * so its children) visible -- isVisible() misreports false right at that point even though
 * setCancelable(true) had already been called, leaving the button visible but permanently
 * unclickable. Fixed by tracking the cancelable request in its own member instead of re-deriving it
 * from widget visibility. Covers both the fresh-show case and the reuse-after-a-click case. */
void ModelWidgetTest::taskProgressWidgetCancelButtonSurvivesShow()
{
	TaskProgressWidget task_prog_wgt;

	//Cancelable: button must end up both visible and enabled right after show(), not just visible
	task_prog_wgt.setCancelable(true);
	task_prog_wgt.show();

	QVERIFY2(task_prog_wgt.cancel_btn->isVisible(), "cancel_btn must be visible when cancelable");
	QVERIFY2(task_prog_wgt.cancel_btn->isEnabled(), "this is the bug: show() was disabling it right after setCancelable(true)");

	//Simulates a click (cancelTask() is a private slot -- QAbstractButton::click() triggers the same
	//press+release+clicked() sequence a real mouse click would, without needing to synthesize one)
	task_prog_wgt.cancel_btn->click();

	QVERIFY(task_prog_wgt.wasCanceled());
	QVERIFY2(!task_prog_wgt.cancel_btn->isEnabled(), "must disable itself on click, to not pile up further cancel requests");

	//Reused for a second task (same instance, same as how a caller might show() it again): show() must
	//reset both wasCanceled() and the button back to enabled, not leave it stuck disabled from the click above
	task_prog_wgt.show();

	QVERIFY2(!task_prog_wgt.wasCanceled(), "show() must reset the cancellation flag for the new task");
	QVERIFY2(task_prog_wgt.cancel_btn->isEnabled(), "show() must also re-enable the button, not just hide the staleness");

	task_prog_wgt.close();

	//Not cancelable: button must stay hidden through show(), regardless of the above
	TaskProgressWidget task_prog_wgt2;
	task_prog_wgt2.show();

	QVERIFY2(!task_prog_wgt2.cancel_btn->isVisible(), "cancel_btn must stay hidden when setCancelable() was never called");

	task_prog_wgt2.close();
}

void ModelWidgetTest::doubleClickingACutoffEmitsItsTable()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		Table *table_a = new Table, *table_b = new Table;

		table_a->setName("table_a");
		table_a->setSchema(public_sch);

		Column *col = new Column;
		col->setName("id");
		col->setType(PgSqlType("serial"));
		table_a->addColumn(col);

		Constraint *pk = new Constraint;
		pk->setName("table_a_pk");
		pk->setConstraintType(ConstraintType::PrimaryKey);
		pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk);
		db_model->addTable(table_a);

		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *filler = new Table;
			filler->setName(QString("filler_%1").arg(i));
			filler->setSchema(public_sch);
			db_model->addTable(filler);
		}

		table_b->setName("table_b");
		table_b->setSchema(public_sch);
		db_model->addTable(table_b);
		db_model->addRelationship(new Relationship(BaseRelationship::Relationship1n, table_a, table_b));

		BaseTableView *table_a_view = dynamic_cast<BaseTableView *>(table_a->getOverlyingObject());
		QVERIFY(table_a_view->getCutoffItem() != nullptr);

		ObjectsScene *scn = model_wgt.getSceneForObject(table_a);
		QSignalSpy table_spy(scn, &ObjectsScene::s_cutoffDoubleClicked),
							 obj_spy(scn, &ObjectsScene::s_objectDoubleClicked);

		table_a_view->getCutoffItem()->setSelected(true);

		QGraphicsSceneMouseEvent event(QEvent::GraphicsSceneMouseDoubleClick);
		event.setButton(Qt::LeftButton);
		event.setButtons(Qt::LeftButton);

		/* The marker's overall rect center can fall in a gap between its line and its label, so
		 * the click lands on the center of the first child that actually resolves back to the marker */
		CutoffItem *cutoff = table_a_view->getCutoffItem();
		QPointF click_pos;

		for(auto &child : cutoff->childItems())
		{
			QPointF pos = child->sceneBoundingRect().center();

			QGraphicsItem *hit = scn->itemAt(pos, QTransform());

			if(hit && (hit == cutoff || hit->group() == cutoff))
			{
				click_pos = pos;
				break;
			}
		}

		QVERIFY(!click_pos.isNull());
		event.setScenePos(click_pos);

		//The signal opens the (modal) relationships form, so it's closed from outside to let the event return
		bool form_shown = false;
		QTimer::singleShot(200, [&form_shown](){
			BaseForm *form = qobject_cast<BaseForm *>(QApplication::activeModalWidget());
			form_shown = (form != nullptr);

			if(form)
				form->accept();
		});

		QCoreApplication::sendEvent(scn, &event);

		QVERIFY2(form_shown, "double-clicking a cutoff must open the cross-scene relationships form");
		QCOMPARE(table_spy.count(), 1);
		QCOMPARE(table_spy.at(0).at(0).value<BaseTable *>(), table_a);
		QCOMPARE(obj_spy.count(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ModelWidgetTest::cutoffRelationshipsWidgetListsAndRefreshesRelationships()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();

		db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		Table *table_a = new Table, *table_b = new Table;

		table_a->setName("table_a");
		table_a->setSchema(public_sch);

		Column *col = new Column;
		col->setName("id");
		col->setType(PgSqlType("serial"));
		table_a->addColumn(col);

		Constraint *pk = new Constraint;
		pk->setName("table_a_pk");
		pk->setConstraintType(ConstraintType::PrimaryKey);
		pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
		table_a->addConstraint(pk);
		db_model->addTable(table_a);

		for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
		{
			Table *filler = new Table;
			filler->setName(QString("filler_%1").arg(i));
			filler->setSchema(public_sch);
			db_model->addTable(filler);
		}

		table_b->setName("table_b");
		table_b->setSchema(public_sch);
		db_model->addTable(table_b);

		Relationship *rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
		db_model->addRelationship(rel);

		CrossSceneRelsWidget cutoff_wgt;
		cutoff_wgt.setAttributes(db_model, table_a);

		CustomTableWidget *rels_tab = cutoff_wgt.findChild<CustomTableWidget *>();
		ObjectSelectorWidget *table_sel = cutoff_wgt.findChild<ObjectSelectorWidget *>();

		QVERIFY(table_sel != nullptr);
		QCOMPARE(table_sel->getSelectedObject(), table_a);
		QVERIFY(!cutoff_wgt.windowTitle().isEmpty());
		QVERIFY(rels_tab != nullptr);
		QCOMPARE(rels_tab->getRowCount(), 1u);
		QCOMPARE(rels_tab->getCellText(0, 0), rel->getName());
		QCOMPARE(rels_tab->getCellText(0, 2), table_b->getSignature(true));

		//Double-clicking a row asks for that relationship's edition
		QSignalSpy rel_spy(&cutoff_wgt, &CrossSceneRelsWidget::s_relationshipActivated);
		emit rels_tab->s_cellDoubleClicked(0, 0);
		QCOMPARE(rel_spy.count(), 1);
		QCOMPARE(rel_spy.at(0).at(0).value<BaseRelationship *>(), rel);

		//Once the relationship is gone, refreshing drops it from the grid
		db_model->removeRelationship(rel);
		cutoff_wgt.updateRelationships();
		QCOMPARE(rels_tab->getRowCount(), 0u);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

/*! \brief Builds a model where table_a and table_b live in different scenes (threshold mode)
and are linked by rel, so table_a's cutoff marker lists rel */
static void buildCrossSceneModel(ModelWidget &model_wgt, Table *&table_a, Table *&table_b, Relationship *&rel)
{
	DatabaseModel *db_model = model_wgt.getDatabaseModel();

	db_model->setSegmentationMode(DatabaseModel::SegByThreshold);
	db_model->createSystemObjects(true);

	Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
	table_a = new Table;
	table_b = new Table;

	table_a->setName("table_a");
	table_a->setSchema(public_sch);

	Column *col = new Column;
	col->setName("id");
	col->setType(PgSqlType("serial"));
	table_a->addColumn(col);

	Constraint *pk = new Constraint;
	pk->setName("table_a_pk");
	pk->setConstraintType(ConstraintType::PrimaryKey);
	pk->addColumn(table_a->getColumn("id"), Constraint::SourceCols);
	table_a->addConstraint(pk);
	db_model->addTable(table_a);

	for(unsigned i = 0; i < db_model->getSegThreshold(); i++)
	{
		Table *filler = new Table;
		filler->setName(QString("filler_%1").arg(i));
		filler->setSchema(public_sch);
		db_model->addTable(filler);
	}

	table_b->setName("table_b");
	table_b->setSchema(public_sch);
	db_model->addTable(table_b);

	rel = new Relationship(BaseRelationship::Relationship1n, table_a, table_b);
	db_model->addRelationship(rel);
}

void ModelWidgetTest::crossSceneRelsWidgetIgnoresStaleRelationship()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();
		Table *table_a = nullptr, *table_b = nullptr;
		Relationship *rel = nullptr;

		buildCrossSceneModel(model_wgt, table_a, table_b, rel);

		CrossSceneRelsWidget cross_wgt;
		cross_wgt.setAttributes(db_model, table_a);

		CustomTableWidget *rels_tab = cross_wgt.findChild<CustomTableWidget *>();
		QCOMPARE(rels_tab->getRowCount(), 1u);

		//The relationship is removed behind the grid's back, so its row is now stale
		db_model->removeRelationship(rel);
		delete rel;

		QSignalSpy rel_spy(&cross_wgt, &CrossSceneRelsWidget::s_relationshipActivated);
		emit rels_tab->s_cellDoubleClicked(0, 0);

		QCOMPARE(rel_spy.count(), 0);
		QCOMPARE(rels_tab->getRowCount(), 0u);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ModelWidgetTest::crossSceneRelsWidgetSurvivesItsTableBeingRemoved()
{
	try
	{
		ModelWidget model_wgt;
		DatabaseModel *db_model = model_wgt.getDatabaseModel();
		Table *table_a = nullptr, *table_b = nullptr;
		Relationship *rel = nullptr;

		buildCrossSceneModel(model_wgt, table_a, table_b, rel);

		CrossSceneRelsWidget cross_wgt;
		cross_wgt.setAttributes(db_model, table_a);

		CustomTableWidget *rels_tab = cross_wgt.findChild<CustomTableWidget *>();
		ObjectSelectorWidget *table_sel = cross_wgt.findChild<ObjectSelectorWidget *>();
		QCOMPARE(rels_tab->getRowCount(), 1u);

		//Both the relationship and the widget's own table are deleted, as a cascade removal would do
		db_model->removeRelationship(rel);
		delete rel;
		db_model->removeTable(table_a);
		delete table_a;

		cross_wgt.updateRelationships();

		QCOMPARE(rels_tab->getRowCount(), 0u);
		QVERIFY(table_sel->getSelectedObject() == nullptr);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void ModelWidgetTest::closingModelWidgetDestroysItsDatabaseModel()
{
	try
	{
		ModelWidget *model_wgt = new ModelWidget;
		DatabaseModel *db_model = model_wgt->getDatabaseModel();
		bool destroyed = false;

		connect(db_model, &QObject::destroyed, this, [&destroyed](){ destroyed = true; });
		db_model->createSystemObjects(true);

		Schema *public_sch = dynamic_cast<Schema *>(db_model->getObject("public", ObjectType::Schema));
		Table *table = new Table, *removed_table = new Table;

		table->setName("tab");
		table->setSchema(public_sch);
		db_model->addTable(table);

		/* A table taken out of the model but still alive (as the operation list's pool keeps it)
		 * must not stop the model from being destroyed */
		removed_table->setName("removed_tab");
		removed_table->setSchema(public_sch);
		db_model->addTable(removed_table);
		db_model->removeTable(removed_table);

		delete model_wgt;
		delete removed_table;

		QVERIFY2(destroyed, "deleting the model widget must destroy the database model it owns");
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(ModelWidgetTest)
#include "modelwidgettest.moc"
