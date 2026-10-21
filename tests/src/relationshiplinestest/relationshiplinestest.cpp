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

#if defined(__SANITIZE_ADDRESS__)
	#define HAS_LSAN 1
#elif defined(__has_feature)
	#if __has_feature(address_sanitizer)
		#define HAS_LSAN 1
	#endif
#endif

#ifdef HAS_LSAN
	#include <sanitizer/lsan_interface.h>
#endif
#include <QtWidgets>
#include <QPrinter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QMouseEvent>
#include <cmath>
#include <memory>
#include <queue>
#include <limits>
#include <set>
#include <map>
#include <vector>
#include <algorithm>
#include <functional>

/* The tests inspect the internals of the views (route, bumps, displacements, timers of the scene). The access
 * specifiers are opened only while the project headers are included, in this file and nowhere else. All the Qt and
 * standard headers are included before, so they're not affected, and the headers of the project must not be
 * included before this point by any other header of the file */
#define private public
#define protected public

#include "modelwidget.h"
#include "databasemodel.h"
#include "table.h"
#include "column.h"
#include "constraint.h"
#include "relationship.h"
#include "tableview.h"
#include "relationshipview.h"
#include "objectsscene.h"
#include "beziercurveitem.h"

#undef private
#undef protected

#include "pgmodelerunittest.h"

/*! \brief Pauses the leak detection (LeakSanitizer) while it lives. Deleting a Relationship that was removed from the
model leaks the columns/constraints it generated (libcore, DatabaseModel::removeRelationship() and its siblings,
independent of the lines), so the tests that remove relationships are not checked for leaks. Everything else
(including the use of freed memory and the overflows) keeps being checked by AddressSanitizer */
struct LeakCheckPause {
	#ifdef HAS_LSAN
	LeakCheckPause() { __lsan_disable(); }
	~LeakCheckPause() { __lsan_enable(); }
	#endif
};

/*! \brief Tests the orthogonal relationship lines: the route of the lines in every connection mode, the spacing
of the connection points, the obstacle avoidance, the hops over the crossing lines (bumps), the segments
moved by the user (displacements stored in the model) and the interaction with the mouse, plus the
lifecycle of the views in the scene. The tests inspect the internals of the views (the access specifiers of the headers
are opened in this file, see the includes) since the geometry of the lines is not exposed otherwise. */
class RelationshipLinesTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		RelationshipLinesTest() : PgModelerUnitTest(SCHEMASDIR){}

		//! \brief Everything needed to exercise the views: a model widget with its model and scene
		struct Rig {
			ModelWidget wgt;
			DatabaseModel *db {nullptr};
			ObjectsScene *scene {nullptr};
			int tab_count {0};

			Rig()
			{
				db = wgt.getDatabaseModel();
				db->createSystemObjects(true);
			}
		};

	private:
		/*! \brief The static settings of the views are changed by the tests, so they're restored after each one */
		RelationshipView::LineStyle saved_style;
		unsigned saved_mode;
		bool saved_crows_foot, saved_placeholders;

		static constexpr double Eps = 0.01;

		//! \brief Creates a table (id + extra columns) at the position and returns it
		Table *addTable(Rig &rig, const QString &name, const QPointF &pos, int extra_cols = 2);

		//! \brief Creates a relationship between the tables
		Relationship *addRel(Rig &rig, Table *src, Table *dst, BaseRelationship::RelType type = BaseRelationship::Relationship1n, const QString &name = "");

		TableView *tableView(Table *table);
		RelationshipView *relView(BaseRelationship *rel);
		static QRectF tableRect(BaseTableView *view);

		//! \brief Moves a table the way the model does when the user finishes the movement
		void moveTable(Rig &rig, Table *table, const QPointF &pos, int wait_ms = 0);

		//! \brief Gives time to the timers and the posted events (bumps, deferred updates)
		void settle(int ms = 0);

		/*! \brief Checks the invariants of the route of an orthogonal line. Returns an empty string if they hold,
		 * otherwise the description of what is wrong */
		QString checkRoute(RelationshipView *rv, bool check_borders = false);

		static bool onBorder(const QPointF &pnt, const QRectF &rect, double tolerance = 0.6);

		static bool segmentCrossesRect(const QPointF &p1, const QPointF &p2, const QRectF &rect);

		//! \brief Returns whether any segment of the route crosses the interior of the rect
		static bool routeCrossesRect(const std::vector<QPointF> &route, const QRectF &rect);

		//! \brief Returns whether the route has the given number of segments
		static bool isHorizontal(const QPointF &p1, const QPointF &p2);

		void applyMode(unsigned mode, bool crows_foot);

		//! \brief Points of the path drawn by the curve of the view (flattened)
		std::vector<QPointF> drawnPath(RelationshipView *rv);

		//! \brief Returns the side of the rect where the point lies: L(eft), R(ight), T(op), B(ottom) or ? if it's in none
		static char sideOf(const QPointF &pnt, const QRectF &rect);

		//! \brief Returns whether a horizontal segment of a route crosses a vertical one of the other (excluding the touches)
		static bool routesCross(const std::vector<QPointF> &r1, const std::vector<QPointF> &r2);

		//! \brief Returns the number of crossings between the horizontal segments of r1 and the vertical ones of r2
		static int countCrossings(const std::vector<QPointF> &r1, const std::vector<QPointF> &r2);

		//! \brief Returns the end point of the route (first or last) that lies on the rect's border
		static QPointF endOn(const std::vector<QPointF> &route, const QRectF &rect);

		/*! \brief Returns the description of two connection points that lie on the same spot of the same side of a
		 * table (among all the relationships of the scene) or an empty string if there are none */
		QString findOverlappingAnchors(ObjectsScene *scene);

		//! \brief Two tables and the relationship between them, laid out so the route is Z shaped
		struct ZLayout {
			Table *a {nullptr}, *b {nullptr};
			Relationship *rel {nullptr};
			RelationshipView *rv {nullptr};
		};

		ZLayout makeZ(Rig &rig, bool crows_foot = false);

		void sendMouse(RelationshipView *rv, QEvent::Type type, const QPointF &pos, Qt::MouseButton button,
									 Qt::MouseButtons buttons, Qt::KeyboardModifiers mods);

		void sendHover(RelationshipView *rv, const QPointF &pos, Qt::KeyboardModifiers mods);

		//! \brief Drags (press, moves, release) from a point to another over the view, then lets the queued updates run
		void dragSegment(RelationshipView *rv, const QPointF &from, const QPointF &to, Qt::KeyboardModifiers mods, int steps = 5);

		QPointF segmentMiddle(RelationshipView *rv, size_t idx);

		//! \brief Two tables laid out so the route is Z shaped with a horizontal middle segment
		ZLayout makeVerticalZ(Rig &rig);

		//! \brief Random non-overlapping rectangles in the area, away from the rects that must stay free
		std::vector<QRectF> randomObstacles(QRandomGenerator &rnd, int count, double area, const std::vector<QRectF> &avoid);

		//! \brief Cost of a path: its length plus the penalty of each bend (and of arriving at the goal in the wrong direction)
		double pathCost(const std::vector<QPointF> &path, int goal_dir, int start_dir);

		//! \brief Cost of the best path, found by an independent (unoptimized) search; infinity if there's none
		double optimalPathCost(const QPointF &start, const QPointF &goal, int start_dir, int goal_dir, const std::vector<QRectF> &obstacles);

		//! \brief Moves all the tables together by the delta, as the scene does when the user releases the selected objects
		void moveTables(Rig &rig, const std::vector<Table *> &tables, const QPointF &delta, int wait_ms = 0);

		//! \brief Checks the invariants of all the routes of the scene, returns the first problem found
		QString checkAllRoutes(Rig &rig);

		//! \brief Checks that every segment of the route has at least the length the corners and the ends need
		void minimumLengthsAreKept(RelationshipView *rv, QString &err);

		//! \brief Creates rel_cnt relationships (tables far from each other) whose routes the test sets by hand
		void prepareBumpsRig(Rig &rig, int rel_cnt, std::vector<RelationshipView *> &views);

		//! \brief Replaces the route of the line (discarding its bumps) without reconfiguring it
		void bumpsRigSetRoute(RelationshipView *rv, const std::vector<QPointF> &route);

		//! \brief Returns the placement of the second table in relation to the first one for the arrangement index
		static QPointF arrangement(int idx, const QRectF &a, const QSizeF &b_size);

	private slots:
		void init();
		void cleanup();
	// SLOTS-BEGIN
		void orthogonalIsTheInitialStyleOfTheViews();
		void everyModeAndArrangementGivesAValidRoute_data();
		void everyModeAndArrangementGivesAValidRoute();
		void centerModeStartsAtTheTableCenters();
		void fkToPkModeLeavesTheTablesHorizontally();
		void sidesFollowTheDominantAxisOfTheDistance_data();
		void sidesFollowTheDominantAxisOfTheDistance();
		void sideChangesWhenTheTableCrossesTheDiagonal();
		void tablesOverlappingInThePerpendicularAxisGiveAStraightLine();
		void anchorsOnTheSameSideAreSpreadApart_data();
		void anchorsOnTheSameSideAreSpreadApart();
		void crowsFootNeedsMoreRoomBetweenTheAnchors();
		void manyRelationshipsOnASmallSideKeepTheAnchorsInsideIt();
		void parallelRelationshipsBetweenTheSameTablesAreDisplaced();
		void parallelRelationshipsInOppositeDirectionsDoNotCross();
		void reconfiguringWithoutChangesKeepsTheRoute();
		void crowsFootAreasAreRegisteredOnlyInTheTableEdgesMode();
		void crowsFootAreasTouchTheTablesBorders();
		void lineGoesAroundATableInTheWay();
		void lineIsDirectWhenNothingIsInTheWay();
		void movingTheObstacleAwayRestoresTheDirectRoute();
		void movingAnObstacleIntoThePathReroutesTheLine();
		void obstacleMovedAlongTheLineKeepsAllRoutesValid();
		void hiddenTablesAreNotObstacles();
		void tablesOfTheRelationshipAreNotCrossedByTheDetour();
		void moreObstaclesThanTheSearchSupportsKeepTheSimpleRoute();
		void aTableBoxedInByOthersStillGetsAValidRoute();
		void selfRelationshipsKeepTheirLoop();
		void loadedModelHasTheAnchorsOfEachSideSpreadApart();
		void reconfiguringTheLinesOfALoadedModelSpreadsTheAnchors();
		void relationshipsAddedLaterRespaceTheExistingOnes();
		void aCrossingGivesABumpOnTheHorizontalLine();
		void bumpsDisappearWhenTheCrossingLineMovesAway();
		void linesThatDoNotCrossGiveNoBumps();
		void manualCrossingGivesOnlyTheHorizontalLineABump();
		void crossingsCloseToTheEndsOfTheHorizontalSegmentAreIgnored();
		void crossingsCloseToTheEndsOfTheVerticalSegmentAreIgnored();
		void aLineNeverHopsOverItself();
		void collinearAndParallelSegmentsGiveNoBumps();
		void crossingsTooCloseToEachOtherAreMerged();
		void severalCrossingsGiveSeveralBumpsInOrder();
		void crossingsOverACrowsFootAreaAreIgnored();
		void aLineRightBelowTheAreaOfADescriptorStillHopsOverTheOtherLines();
		void crossingsBesideACrowsFootAreaStillGetABump();
		void invisibleRelationshipsDoNotTakePartInTheBumps();
		void bumpsAreNotCalculatedWhileObjectsAreMoving();
		void requestsAreCoalescedInASingleCalculation();
		void linesInsideTheZonesOfOthersAreRecalculatedAtMostTwice();
		void bumpsMatchABruteForceCalculationOnRandomRoutes();
		void aShiftMovesTheMiddleSegmentOfARoute();
		void shiftsDoNotAccumulateAndTheBaseRouteIsKept();
		void aShiftOfZeroOrOfAnotherOrientationIsIgnored();
		void theSegmentsAtTheEndsAreNeverShifted();
		void indexesOutOfTheRouteAreIgnored();
		void routesWithoutInnerSegmentsIgnoreTheShifts();
		void aShiftIsLimitedByTheLengthTheNeighborSegmentsNeed();
		void aShiftIsLimitedByTheAreaOfTheRelationship();
		void shiftsSurviveTheUpdatesOfTheNeighborhood();
		void shiftsAreDiscardedWhenATableMoves();
		void shiftsAreKeptWhenATableIsMovedToTheSameSpot();
		void shiftsAreDiscardedWhenATableChangesItsSize();
		void shiftsSurviveSavingAndLoadingTheModel();
		void otherLineStylesIgnoreTheShiftsAndTheyComeBack();
		void shiftsApplyInEveryConnectionMode();
		void shiftsOfARouteAroundObstaclesKeepItsShape();
		void adjacentShiftsNeverCrushTheSegmentBetweenThem();
		void aShiftOfOneShapeDoesNotApplyToTheNextShapeOfTheRoute();
		void shiftsDoNotChangeTheCrowsFootAreas();
		void dragWithShiftMovesAVerticalSegmentAndStoresTheShift();
		void dragWithShiftMovesAHorizontalSegment();
		void thePreviewOfTheDragDoesNotTouchTheModelUntilTheRelease();
		void draggingFarBeyondTheLimitsStopsAtTheLimit();
		void releasingWithoutMovingStoresNothing();
		void draggingBackToTheOriginalPlaceRemovesTheShift();
		void aSecondDragStartsFromTheCurrentPosition();
		void dragWithoutShiftNeverMovesASegment();
		void onlyTheLeftButtonStartsTheDrag();
		void theEndSegmentsAndTheEmptySpaceCannotBeDragged();
		void aProtectedRelationshipCannotBeDragged();
		void selfRelationshipsCannotBeDragged();
		void shiftClickInTheOtherStylesStillEditsThePoints();
		void altShiftClickResetsTheShiftsAndTheLabels();
		void altShiftClickWithoutShiftsStillResetsTheLabels();
		void theCursorShowsTheDirectionTheSegmentMoves();
		void theCursorIsRestoredWhenTheMouseLeavesAndAfterTheDrag();
		void draggingThroughTheViewportMovesTheSegment();
		void deletingTheRelationshipRightAfterTheDragIsSafe();
		void draggingAfterATableMovedUsesTheNewRoute();
		void theSceneKeepsTheListsOfTablesAndRelationshipsInSync();
		void addingAndRemovingManyViewsLeavesNoStaleEntries();
		void removingEverythingWithAPendingBumpsUpdateLeavesTheScenesListsEmpty();
		void closingTheModelWithPendingUpdatesIsSafe();
		void aLineClickedOnAnEmptyAreaIsFoundBySceneHitTesting();
		void theShapeFollowsThePreviewOfTheDrag();
		void nearbyRoutesDoNotIncludeLinesThatOnlyHaveTheTableInsideTheirBox();
		void nearbyRoutesExcludeTheConnectedOnesUnlessAsked();
		void nearbyRoutesLookAtTheAreaTheTableLeft();
		void theOriginOfAMovementIsOnlyRegisteredWhileObjectsAreMoving();
		void nearbyRoutesAreEmptyInTheOtherLineStyles();
		void aRouteIsNearAnAreaWhenItsSegmentsOrItsDirectRouteTouchIt();
		void movingTheObstacleAwayWorksWithAndWithoutPlaceholders();
		void movingManyTablesTogetherLeavesEveryRouteValid_data();
		void movingManyTablesTogetherLeavesEveryRouteValid();
		void theStyleCanBeChangedBackAndForthWithoutLeavingTraces();
		void togglingTheCrowsFootChangesTheAreasAndTheAnchors();
		void changingTheConnectionModeKeepsTheLinesValid();
		void hiddenRelationshipsDoNotDisturbTheOthers();
		void theOrthogonalSearchFindsTheCheapestPath_data();
		void theOrthogonalSearchFindsTheCheapestPath();
		void theSearchGivesUpWhenTheGridIsTooBigOrTheEndsAreClosed();
		void segmentsOnlyTouchingAnObstacleAreNotBlocked();
		void cleanRouteRemovesDuplicatedAndCollinearPoints();
		void theRectOfARouteIsMadeOfItsExtremes();
		void theSimpleRouteIsAlwaysOrthogonalAndRespectsTheEnds();
		void theSimpleRouteKeepsTheRoomForTheStubsWhenThereIsSpace();
		void appliedShiftsAlwaysGiveAValidRoute();
		void randomScenesKeepEveryInvariantWhateverTheUserDoes_data();
		void randomScenesKeepEveryInvariantWhateverTheUserDoes();
	// SLOTS-END
};

// ---------------------------------------------------------------------------------------------------
// Helpers

void RelationshipLinesTest::init()
{
	saved_style = RelationshipView::getLineStyle();
	saved_mode = RelationshipView::getLineConnectinMode();
	saved_crows_foot = RelationshipView::isCrowsFoot();
	saved_placeholders = BaseObjectView::isPlaceholderEnabled();

	RelationshipView::setLineStyle(RelationshipView::OrthogonalLines);
	RelationshipView::setCrowsFoot(false);
	RelationshipView::setLineConnectionMode(RelationshipView::ConnectTableEdges);
	BaseObjectView::setPlaceholderEnabled(false);
}

void RelationshipLinesTest::cleanup()
{
	RelationshipView::setLineStyle(saved_style);
	RelationshipView::setCrowsFoot(saved_crows_foot);
	RelationshipView::setLineConnectionMode(static_cast<RelationshipView::LineConnectionMode>(saved_mode));
	BaseObjectView::setPlaceholderEnabled(saved_placeholders);
}

void RelationshipLinesTest::applyMode(unsigned mode, bool crows_foot)
{
	RelationshipView::setCrowsFoot(crows_foot);
	RelationshipView::setLineConnectionMode(static_cast<RelationshipView::LineConnectionMode>(mode));
}

Table *RelationshipLinesTest::addTable(Rig &rig, const QString &name, const QPointF &pos, int extra_cols)
{
	Table *table = new Table;
	Column *col = new Column;
	Constraint *pk = new Constraint;

	table->setName(name);
	table->setSchema(rig.db->getObject("public", ObjectType::Schema));
	table->setPosition(pos);

	col->setName(QString("id_%1").arg(name));
	col->setType(PgSqlType("integer"));
	table->addColumn(col);

	pk->setName(name + "_pk");
	pk->setConstraintType(ConstraintType::PrimaryKey);
	pk->addColumn(col, Constraint::SourceCols);
	table->addConstraint(pk);

	for(int i = 0; i < extra_cols; i++)
	{
		col = new Column;
		col->setName(QString("col_%1_%2").arg(name).arg(i));
		col->setType(PgSqlType("text"));
		table->addColumn(col);
	}

	rig.db->addTable(table);
	rig.tab_count++;

	if(!rig.scene)
		rig.scene = rig.wgt.getSceneForObject(table);

	return table;
}

Relationship *RelationshipLinesTest::addRel(Rig &rig, Table *src, Table *dst, BaseRelationship::RelType type, const QString &name)
{
	Relationship *rel = new Relationship(type, src, dst);

	rel->setName(name.isEmpty() ? QString("rel_%1_%2").arg(src->getName(), dst->getName()) : name);
	rig.db->addRelationship(rel);
	return rel;
}

TableView *RelationshipLinesTest::tableView(Table *table)
{
	return dynamic_cast<TableView *>(table->getOverlyingObject());
}

RelationshipView *RelationshipLinesTest::relView(BaseRelationship *rel)
{
	return dynamic_cast<RelationshipView *>(rel->getOverlyingObject());
}

QRectF RelationshipLinesTest::tableRect(BaseTableView *view)
{
	return QRectF(view->pos(), view->boundingRect().size());
}

void RelationshipLinesTest::moveTable(Rig &rig, Table *table, const QPointF &pos, int wait_ms)
{
	TableView *view = tableView(table);

	rig.scene->clearSelection();
	view->setSelected(true);

	/* The same sequence of the scene when the user moves the selected objects with the mouse:
	 * the movement starts, the table is moved and the scene finishes the movement */
	rig.scene->moving_objs = true;
	view->setPos(pos);
	rig.scene->finishObjectsMove(pos);
	view->setSelected(false);
	settle(wait_ms);
}

void RelationshipLinesTest::settle(int ms)
{
	QCoreApplication::processEvents();

	if(ms > 0)
		QTest::qWait(ms);

	QCoreApplication::processEvents();
}

bool RelationshipLinesTest::isHorizontal(const QPointF &p1, const QPointF &p2)
{
	return std::fabs(p1.y() - p2.y()) < Eps;
}

bool RelationshipLinesTest::onBorder(const QPointF &pnt, const QRectF &rect, double tolerance)
{
	bool in_x = pnt.x() >= rect.left() - tolerance && pnt.x() <= rect.right() + tolerance,
			in_y = pnt.y() >= rect.top() - tolerance && pnt.y() <= rect.bottom() + tolerance;

	return (in_y && (std::fabs(pnt.x() - rect.left()) <= tolerance || std::fabs(pnt.x() - rect.right()) <= tolerance)) ||
				 (in_x && (std::fabs(pnt.y() - rect.top()) <= tolerance || std::fabs(pnt.y() - rect.bottom()) <= tolerance));
}

bool RelationshipLinesTest::segmentCrossesRect(const QPointF &p1, const QPointF &p2, const QRectF &rect)
{
	double left = std::min(p1.x(), p2.x()), right = std::max(p1.x(), p2.x()),
			top = std::min(p1.y(), p2.y()), bottom = std::max(p1.y(), p2.y());

	return left < rect.right() - Eps && right > rect.left() + Eps &&
				 top < rect.bottom() - Eps && bottom > rect.top() + Eps;
}

bool RelationshipLinesTest::routeCrossesRect(const std::vector<QPointF> &route, const QRectF &rect)
{
	for(size_t i = 1; i < route.size(); i++)
	{
		if(segmentCrossesRect(route[i - 1], route[i], rect))
			return true;
	}

	return false;
}

QString RelationshipLinesTest::checkRoute(RelationshipView *rv, bool check_borders)
{
	const std::vector<QPointF> &route = rv->route_points;
	BaseRelationship *rel = rv->getUnderlyingObject();

	if(route.size() < 2)
		return QString("route of %1 has %2 points").arg(rel->getName()).arg(route.size());

	for(size_t i = 0; i < route.size(); i++)
	{
		if(!std::isfinite(route[i].x()) || !std::isfinite(route[i].y()))
			return QString("point %1 of %2 is not finite").arg(i).arg(rel->getName());
	}

	for(size_t i = 1; i < route.size(); i++)
	{
		bool horiz = std::fabs(route[i].y() - route[i - 1].y()) < Eps,
				vert = std::fabs(route[i].x() - route[i - 1].x()) < Eps;

		if(!horiz && !vert)
			return QString("segment %1 of %2 is not horizontal nor vertical: (%3,%4) -> (%5,%6)").arg(i - 1).arg(rel->getName())
					.arg(route[i - 1].x()).arg(route[i - 1].y()).arg(route[i].x()).arg(route[i].y());

		if(horiz && vert)
			return QString("segment %1 of %2 has no length").arg(i - 1).arg(rel->getName());

		if(i >= 2)
		{
			bool prev_horiz = std::fabs(route[i - 1].y() - route[i - 2].y()) < Eps;

			if(prev_horiz == horiz)
				return QString("segments %1 and %2 of %3 are collinear").arg(i - 2).arg(i - 1).arg(rel->getName());
		}
	}

	if(check_borders)
	{
		QRectF rect_src = tableRect(tableView(dynamic_cast<Table *>(rel->getTable(BaseRelationship::SrcTable)))),
				rect_dst = tableRect(tableView(dynamic_cast<Table *>(rel->getTable(BaseRelationship::DstTable))));

		bool front_ok = onBorder(route.front(), rect_src) || onBorder(route.front(), rect_dst),
				back_ok = onBorder(route.back(), rect_src) || onBorder(route.back(), rect_dst);

		if(!front_ok || !back_ok)
			return QString("the ends of %1 are not on the tables' borders: (%2,%3) (%4,%5)").arg(rel->getName())
					.arg(route.front().x()).arg(route.front().y()).arg(route.back().x()).arg(route.back().y());
	}

	return "";
}

std::vector<QPointF> RelationshipLinesTest::drawnPath(RelationshipView *rv)
{
	std::vector<QPointF> pts;

	for(auto &item : rv->childItems())
	{
		BezierCurveItem *curve = dynamic_cast<BezierCurveItem *>(item);

		if(!curve)
			continue;

		for(auto &poly : curve->path().toSubpathPolygons())
			pts.insert(pts.end(), poly.begin(), poly.end());
	}

	return pts;
}

char RelationshipLinesTest::sideOf(const QPointF &pnt, const QRectF &rect)
{
	constexpr double Tol = 0.6;

	if(pnt.y() >= rect.top() - Tol && pnt.y() <= rect.bottom() + Tol)
	{
		if(std::fabs(pnt.x() - rect.left()) <= Tol)
			return 'L';

		if(std::fabs(pnt.x() - rect.right()) <= Tol)
			return 'R';
	}

	if(pnt.x() >= rect.left() - Tol && pnt.x() <= rect.right() + Tol)
	{
		if(std::fabs(pnt.y() - rect.top()) <= Tol)
			return 'T';

		if(std::fabs(pnt.y() - rect.bottom()) <= Tol)
			return 'B';
	}

	return '?';
}

int RelationshipLinesTest::countCrossings(const std::vector<QPointF> &r1, const std::vector<QPointF> &r2)
{
	int cnt = 0;

	for(size_t i = 1; i < r1.size(); i++)
	{
		if(!isHorizontal(r1[i - 1], r1[i]))
			continue;

		for(size_t j = 1; j < r2.size(); j++)
		{
			if(isHorizontal(r2[j - 1], r2[j]))
				continue;

			double y = r1[i].y(), x = r2[j].x();

			if(x > std::min(r1[i - 1].x(), r1[i].x()) + Eps && x < std::max(r1[i - 1].x(), r1[i].x()) - Eps &&
				 y > std::min(r2[j - 1].y(), r2[j].y()) + Eps && y < std::max(r2[j - 1].y(), r2[j].y()) - Eps)
				cnt++;
		}
	}

	return cnt;
}

bool RelationshipLinesTest::routesCross(const std::vector<QPointF> &r1, const std::vector<QPointF> &r2)
{
	return countCrossings(r1, r2) > 0 || countCrossings(r2, r1) > 0;
}

QPointF RelationshipLinesTest::endOn(const std::vector<QPointF> &route, const QRectF &rect)
{
	if(onBorder(route.front(), rect))
		return route.front();

	return route.back();
}

QString RelationshipLinesTest::findOverlappingAnchors(ObjectsScene *scene)
{
	std::map<QString, std::vector<QPointF>> by_side;

	for(auto &item : scene->items())
	{
		RelationshipView *rv = dynamic_cast<RelationshipView *>(item);

		if(!rv || rv->getUnderlyingObject()->isSelfRelationship() || rv->route_points.size() < 2)
			continue;

		for(unsigned idx = 0; idx < 2; idx++)
		{
			QRectF rect = tableRect(rv->tables[idx]);
			QPointF pnt = (idx == 0 ? rv->route_points.front() : rv->route_points.back());

			// The first point is on the border of tables[0] and the last one on the border of tables[1], or the opposite
			if(!onBorder(pnt, rect))
				pnt = (idx == 0 ? rv->route_points.back() : rv->route_points.front());

			char side = sideOf(pnt, rect);

			if(side != '?')
				by_side[QString("%1:%2").arg(rv->tables[idx]->getUnderlyingObject()->getName(true)).arg(side)].push_back(pnt);
		}
	}

	for(auto &itr : by_side)
	{
		std::vector<QPointF> pts = itr.second;

		std::sort(pts.begin(), pts.end(), [](const QPointF &p1, const QPointF &p2){
			return p1.y() != p2.y() ? p1.y() < p2.y() : p1.x() < p2.x(); });

		for(size_t i = 1; i < pts.size(); i++)
		{
			if(QLineF(pts[i - 1], pts[i]).length() <= 1)
				return QString("%1 has overlapping connection points at (%2,%3)").arg(itr.first).arg(pts[i].x()).arg(pts[i].y());
		}
	}

	return "";
}

QPointF RelationshipLinesTest::arrangement(int idx, const QRectF &a, const QSizeF &bs)
{
	switch(idx)
	{
		case 0: return QPointF(a.right() + 200, a.top());                        // right, aligned
		case 1: return QPointF(a.left() - 200 - bs.width(), a.top());            // left, aligned
		case 2: return QPointF(a.left(), a.top() - 200 - bs.height());           // above, aligned
		case 3: return QPointF(a.left(), a.bottom() + 200);                      // below, aligned
		case 4: return QPointF(a.right() + 250, a.top() - 150 - bs.height());    // right-up
		case 5: return QPointF(a.right() + 250, a.bottom() + 150);               // right-down
		case 6: return QPointF(a.left() - 250 - bs.width(), a.top() - 150 - bs.height());  // left-up
		case 7: return QPointF(a.left() - 250 - bs.width(), a.bottom() + 150);   // left-down
		case 8: return QPointF(a.left() + 30, a.top() + 20);                     // overlapping
		case 9: return a.topLeft();                                              // same position
		case 10: return QPointF(a.right(), a.top());                             // touching on the right
		case 11: return QPointF(a.right() + 5000, a.top() + 3000);               // far away
		case 12: return QPointF(a.left() + 30, a.top() - 200 - bs.height());     // above, almost aligned
		case 13: return QPointF(a.right() + 200, a.bottom() - 60);               // right, slightly overlapping vertically
		case 14: return QPointF(a.right() + 200, a.top() + a.height() * 0.5);    // right, half overlapping
		default: return QPointF(a.right() + 200, a.top() - bs.height() - 1);     // right, just above
	}
}

// ---------------------------------------------------------------------------------------------------
// Tests
// TESTS-BEGIN

void RelationshipLinesTest::orthogonalIsTheInitialStyleOfTheViews()
{
	try
	{
		QCOMPARE(saved_style, RelationshipView::OrthogonalLines);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::everyModeAndArrangementGivesAValidRoute_data()
{
	QTest::addColumn<int>("mode");
	QTest::addColumn<bool>("crows_foot");
	QTest::addColumn<int>("arrange_idx");
	QTest::addColumn<int>("rel_type");

	QStringList modes = { "center", "fk_to_pk", "edges" };

	for(int mode = 0; mode < 3; mode++)
	{
		for(int crows = 0; crows < 2; crows++)
		{
			for(int arr = 0; arr < 16; arr++)
			{
				for(auto type : { BaseRelationship::Relationship1n, BaseRelationship::Relationship11, BaseRelationship::RelationshipDep })
				{
					QTest::newRow(qPrintable(QString("%1_%2_arr%3_type%4").arg(modes[mode]).arg(crows ? "crow" : "plain").arg(arr).arg(int(type))))
							<< mode << bool(crows) << arr << int(type);
				}
			}
		}
	}
}

void RelationshipLinesTest::everyModeAndArrangementGivesAValidRoute()
{
	try
	{
		QFETCH(int, mode);
		QFETCH(bool, crows_foot);
		QFETCH(int, arrange_idx);
		QFETCH(int, rel_type);

		applyMode(mode, crows_foot);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(3000, 3000));
		Relationship *rel = addRel(rig, tab_a, tab_b, static_cast<BaseRelationship::RelType>(rel_type));
		RelationshipView *rv = relView(rel);
		QRectF rect_a, rect_b;
		bool edges = (RelationshipView::getLineConnectinMode() == RelationshipView::ConnectTableEdges);

		QVERIFY(rv != nullptr);
		moveTable(rig, tab_b, arrangement(arrange_idx, tableRect(tableView(tab_a)), tableRect(tableView(tab_b)).size()));
		rect_a = tableRect(tableView(tab_a));
		rect_b = tableRect(tableView(tab_b));

		// When the tables overlap there are no sides, the line goes through their centers
		bool overlap = rect_a.intersects(rect_b) || arrange_idx == 10;
		QString err = checkRoute(rv, edges && !overlap);

		QVERIFY2(err.isEmpty(), qPrintable(err));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::centerModeStartsAtTheTableCenters()
{
	try
	{
		applyMode(RelationshipView::ConnectCenterPoints, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(600, 300));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY(rv != nullptr);
		QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(checkRoute(rv)));

		QRectF rect_a = tableRect(tableView(tab_a)), rect_b = tableRect(tableView(tab_b));
		QPointF first = rv->route_points.front(), last = rv->route_points.back();

		// The ends are the centers of the tables (in any order)
		bool a_first = (std::fabs(first.x() - rect_a.center().x()) < 1 && std::fabs(first.y() - rect_a.center().y()) < 1);
		QPointF center_first = (a_first ? rect_a.center() : rect_b.center()), center_last = (a_first ? rect_b.center() : rect_a.center());

		QVERIFY(std::fabs(first.x() - center_first.x()) < 1 && std::fabs(first.y() - center_first.y()) < 1);
		QVERIFY(std::fabs(last.x() - center_last.x()) < 1 && std::fabs(last.y() - center_last.y()) < 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::fkToPkModeLeavesTheTablesHorizontally()
{
	try
	{
		applyMode(RelationshipView::ConnectFkToPk, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(700, 400));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY(rv != nullptr);
		QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(checkRoute(rv)));

		// The first and last segments are horizontal: the line goes from the columns' height
		QVERIFY(isHorizontal(rv->route_points[0], rv->route_points[1]));
		QVERIFY(isHorizontal(rv->route_points[rv->route_points.size() - 2], rv->route_points.back()));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::sidesFollowTheDominantAxisOfTheDistance_data()
{
	QTest::addColumn<int>("arrange_idx");
	QTest::addColumn<char>("side_a");
	QTest::addColumn<char>("side_b");

	QTest::newRow("right") << 0 << 'R' << 'L';
	QTest::newRow("left") << 1 << 'L' << 'R';
	QTest::newRow("above") << 2 << 'T' << 'B';
	QTest::newRow("below") << 3 << 'B' << 'T';
	QTest::newRow("right_up_far") << 4 << 'R' << 'L';
	QTest::newRow("far_away_right") << 11 << 'R' << 'L';
}

void RelationshipLinesTest::sidesFollowTheDominantAxisOfTheDistance()
{
	try
	{
		QFETCH(int, arrange_idx);
		QFETCH(char, side_a);
		QFETCH(char, side_b);

		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		moveTable(rig, tab_b, arrangement(arrange_idx, tableRect(tableView(tab_a)), tableRect(tableView(tab_b)).size()));
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));

		QCOMPARE(sideOf(endOn(rv->route_points, tableRect(tableView(tab_a))), tableRect(tableView(tab_a))), side_a);
		QCOMPARE(sideOf(endOn(rv->route_points, tableRect(tableView(tab_b))), tableRect(tableView(tab_b))), side_b);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::sideChangesWhenTheTableCrossesTheDiagonal()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		QRectF rect_a = tableRect(tableView(tab_a));

		moveTable(rig, tab_b, QPointF(rect_a.right() + 400, rect_a.top() + 100));
		QCOMPARE(sideOf(endOn(rv->route_points, rect_a), rect_a), 'R');

		moveTable(rig, tab_b, QPointF(rect_a.left() + 100, rect_a.bottom() + 400));
		QCOMPARE(sideOf(endOn(rv->route_points, rect_a), rect_a), 'B');

		moveTable(rig, tab_b, QPointF(rect_a.left() - 600, rect_a.top() - 100));
		QCOMPARE(sideOf(endOn(rv->route_points, rect_a), rect_a), 'L');

		moveTable(rig, tab_b, QPointF(rect_a.left() - 100, rect_a.top() - 500));
		QCOMPARE(sideOf(endOn(rv->route_points, rect_a), rect_a), 'T');
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::tablesOverlappingInThePerpendicularAxisGiveAStraightLine()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		// Aligned, half overlapped and barely overlapped (vertically)
		for(int arrange_idx : { 0, 14, 13 })
		{
			moveTable(rig, tab_b, arrangement(arrange_idx, tableRect(tableView(tab_a)), tableRect(tableView(tab_b)).size()));

			QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
			QVERIFY2(rv->route_points.size() == 2, qPrintable(QString("arrangement %1: %2 points").arg(arrange_idx).arg(rv->route_points.size())));
			QVERIFY(isHorizontal(rv->route_points[0], rv->route_points[1]));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::anchorsOnTheSameSideAreSpreadApart_data()
{
	QTest::addColumn<bool>("crows_foot");

	QTest::newRow("plain") << false;
	QTest::newRow("crows_foot") << true;
}

void RelationshipLinesTest::anchorsOnTheSameSideAreSpreadApart()
{
	try
	{
		QFETCH(bool, crows_foot);

		applyMode(RelationshipView::ConnectTableEdges, crows_foot);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 10);
		std::vector<Table *> others;
		std::vector<RelationshipView *> views;

		for(int i = 0; i < 4; i++)
		{
			others.push_back(addTable(rig, QString("tab_b%1").arg(i), QPointF(900, i * 250.0)));
			views.push_back(relView(addRel(rig, tab_a, others.back())));
		}

		// The spacing of the anchors is recalculated when each line is reconfigured
		for(auto &rv : views)
			rv->invalidateRoute();

		QRectF rect_a = tableRect(tableView(tab_a));
		double min_gap = RelationshipView::ConnLineLength * (crows_foot ? 1.3 : 0.8) *
										 BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor(),
				border = views[0]->getEdgeBorderFactor(0);
		std::vector<QPointF> anchors;

		for(auto &rv : views)
		{
			QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
			anchors.push_back(endOn(rv->route_points, rect_a));
			QCOMPARE(sideOf(anchors.back(), rect_a), 'R');
		}

		// Allowed distance: the minimum one, or less if the side doesn't have room for all of them
		double allowed = std::min(min_gap, (rect_a.height() - 2 * border) / (anchors.size() - 1));

		for(size_t i = 1; i < anchors.size(); i++)
		{
			// The tables are placed from top to bottom: the anchors follow them (the lines don't cross)
			QVERIFY2(anchors[i].y() - anchors[i - 1].y() >= allowed - 0.6,
							 qPrintable(QString("anchors %1 and %2 are %3 apart (min %4)").arg(i - 1).arg(i).arg(anchors[i].y() - anchors[i - 1].y()).arg(allowed)));
		}

		for(auto &pnt : anchors)
			QVERIFY(pnt.y() >= rect_a.top() + border - 0.6 && pnt.y() <= rect_a.bottom() - border + 0.6);

		for(size_t i = 0; i < views.size(); i++)
		{
			for(size_t j = i + 1; j < views.size(); j++)
				QVERIFY2(!routesCross(views[i]->route_points, views[j]->route_points), qPrintable(QString("%1 x %2").arg(i).arg(j)));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crowsFootNeedsMoreRoomBetweenTheAnchors()
{
	try
	{
		double min_gaps[2] = { 0, 0 };

		for(int crows = 0; crows < 2; crows++)
		{
			applyMode(RelationshipView::ConnectTableEdges, crows);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 14);
			std::vector<QPointF> anchors;
			QRectF rect_a;

			for(int i = 0; i < 3; i++)
			{
				// The tables are almost at the same place, so the lines want the same connection point
			Table *other = addTable(rig, QString("tab_b%1").arg(i), QPointF(900 + i * 5.0, 0));
				RelationshipView *rv = relView(addRel(rig, tab_a, other));

				QVERIFY(rv != nullptr);
			}

			rect_a = tableRect(tableView(tab_a));

			for(int i = 0; i < 3; i++)
			{
				RelationshipView *rv = nullptr;

				for(auto &item : rig.scene->items())
				{
					RelationshipView *cand = dynamic_cast<RelationshipView *>(item);

					if(cand && cand->getUnderlyingObject()->getName() == QString("rel_tab_a_tab_b%1").arg(i))
						rv = cand;
				}

				QVERIFY(rv != nullptr);
				anchors.push_back(endOn(rv->route_points, rect_a));
			}

			std::sort(anchors.begin(), anchors.end(), [](const QPointF &p1, const QPointF &p2){ return p1.y() < p2.y(); });
			min_gaps[crows] = std::min(anchors[1].y() - anchors[0].y(), anchors[2].y() - anchors[1].y());
		}

		QVERIFY2(min_gaps[1] > min_gaps[0], qPrintable(QString("plain %1 / crow's foot %2").arg(min_gaps[0]).arg(min_gaps[1])));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::manyRelationshipsOnASmallSideKeepTheAnchorsInsideIt()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 0);
		std::vector<RelationshipView *> views;

		for(int i = 0; i < 14; i++)
			views.push_back(relView(addRel(rig, tab_a, addTable(rig, QString("tab_b%1").arg(i), QPointF(900, i * 20.0), 0))));

		for(auto &rv : views)
			rv->invalidateRoute();

		QRectF rect_a = tableRect(tableView(tab_a));
		double border = views[0]->getEdgeBorderFactor(0);

		for(auto &rv : views)
		{
			QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));

			QPointF anchor = endOn(rv->route_points, rect_a);
			QVERIFY(anchor.y() >= std::min(rect_a.top() + border, rect_a.center().y()) - 0.6);
			QVERIFY(anchor.y() <= std::max(rect_a.bottom() - border, rect_a.center().y()) + 0.6);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::parallelRelationshipsBetweenTheSameTablesAreDisplaced()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv_1 = relView(addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, "rel_1")),
				*rv_2 = relView(addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, "rel_2")),
				*rv_3 = relView(addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, "rel_3"));
		QRectF rect_a = tableRect(tableView(tab_a));

		// The second table is on the right, far below: the routes are Z shaped with vertical middle segments
		moveTable(rig, tab_b, QPointF(rect_a.right() + 400, rect_a.bottom() + 500));

		std::vector<double> mids;

		for(auto &rv : { rv_1, rv_2, rv_3 })
		{
			QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
			QCOMPARE(rv->route_points.size(), size_t(4));
			mids.push_back(rv->route_points[1].x());
		}

		QVERIFY2(std::fabs(mids[0] - mids[1]) > 5 && std::fabs(mids[1] - mids[2]) > 5 && std::fabs(mids[0] - mids[2]) > 5,
						 qPrintable(QString("%1 %2 %3").arg(mids[0]).arg(mids[1]).arg(mids[2])));

		QVERIFY(!routesCross(rv_1->route_points, rv_2->route_points));
		QVERIFY(!routesCross(rv_1->route_points, rv_3->route_points));
		QVERIFY(!routesCross(rv_2->route_points, rv_3->route_points));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::parallelRelationshipsInOppositeDirectionsDoNotCross()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv_1 = relView(addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, "rel_ab")),
				*rv_2 = relView(addRel(rig, tab_b, tab_a, BaseRelationship::Relationship1n, "rel_ba"));
		QRectF rect_a = tableRect(tableView(tab_a));

		for(auto pos : { QPointF(rect_a.right() + 400, rect_a.bottom() + 500), QPointF(rect_a.right() + 400, rect_a.top() - 500),
										 QPointF(rect_a.left() - 700, rect_a.bottom() + 500) })
		{
			moveTable(rig, tab_b, pos);
			QVERIFY2(checkRoute(rv_1, true).isEmpty(), qPrintable(checkRoute(rv_1, true)));
			QVERIFY2(checkRoute(rv_2, true).isEmpty(), qPrintable(checkRoute(rv_2, true)));
			QVERIFY2(!routesCross(rv_1->route_points, rv_2->route_points), qPrintable(QString("%1,%2").arg(pos.x()).arg(pos.y())));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::reconfiguringWithoutChangesKeepsTheRoute()
{
	try
	{
		for(int mode = 0; mode < 3; mode++)
		{
			applyMode(mode, false);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(700, 350)),
					*tab_c = addTable(rig, "tab_c", QPointF(350, 150));
			RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

			addRel(rig, tab_c, tab_b);
			settle();

			std::vector<QPointF> before = rv->route_points;

			rv->invalidateRoute();
			QCOMPARE(rv->route_points, before);

			rv->invalidateRoute(false);
			QCOMPARE(rv->route_points, before);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crowsFootAreasAreRegisteredOnlyInTheTableEdgesMode()
{
	try
	{
		for(int mode = 0; mode < 3; mode++)
		{
			applyMode(mode, false);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(700, 350));
			RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

			QVERIFY(rv->end_zones.empty());
		}

		applyMode(RelationshipView::ConnectTableEdges, true);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(700, 350)), *tab_c = addTable(rig, "tab_c", QPointF(1500, 0));
		RelationshipView *rv_1n = relView(addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, "rel_1n")),
				*rv_dep = relView(addRel(rig, tab_c, tab_a, BaseRelationship::RelationshipDep, "rel_dep"));

		// Dependency relationships have no crow's foot descriptors
		QCOMPARE(rv_1n->end_zones.size(), size_t(2));
		QVERIFY(rv_dep->end_zones.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crowsFootAreasTouchTheTablesBorders()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		QRectF rect_a = tableRect(tableView(tab_a));

		for(auto pos : { QPointF(rect_a.right() + 400, rect_a.top()), QPointF(rect_a.left(), rect_a.bottom() + 400),
										 QPointF(rect_a.left() - 700, rect_a.top() + 40), QPointF(rect_a.left() + 20, rect_a.top() - 500) })
		{
			moveTable(rig, tab_b, pos);

			QRectF rect_b = tableRect(tableView(tab_b));

			QCOMPARE(rv->end_zones.size(), size_t(2));

			for(auto &zone : rv->end_zones)
			{
				// Each zone starts at the border of one of the tables and extends outwards
				bool touches = (std::fabs(zone.left() - rect_a.right()) < 0.7 || std::fabs(zone.right() - rect_a.left()) < 0.7 ||
												std::fabs(zone.bottom() - rect_a.top()) < 0.7 || std::fabs(zone.top() - rect_a.bottom()) < 0.7 ||
												std::fabs(zone.left() - rect_b.right()) < 0.7 || std::fabs(zone.right() - rect_b.left()) < 0.7 ||
												std::fabs(zone.bottom() - rect_b.top()) < 0.7 || std::fabs(zone.top() - rect_b.bottom()) < 0.7);

				QVERIFY2(touches, qPrintable(QString("zone (%1,%2 %3x%4)").arg(zone.x()).arg(zone.y()).arg(zone.width()).arg(zone.height())));
				QVERIFY(!zone.intersects(rect_a.adjusted(1, 1, -1, -1)) && !zone.intersects(rect_b.adjusted(1, 1, -1, -1)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::lineGoesAroundATableInTheWay()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 0));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		QRectF rect_c = tableRect(tableView(tab_c));

		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
		QVERIFY(rv->route_points.size() > 2);
		QVERIFY(!routeCrossesRect(rv->route_points, rect_c));

		// Keeps the clearance used by the search around the table
		double clearance = RelationshipView::ConnLineLength * 0.75;
		QVERIFY(!routeCrossesRect(rv->route_points, rect_c.adjusted(-clearance + 0.5, -clearance + 0.5, clearance - 0.5, clearance - 0.5)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::lineIsDirectWhenNothingIsInTheWay()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 1500));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY(tab_c != nullptr);
		QCOMPARE(rv->route_points.size(), size_t(2));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::movingTheObstacleAwayRestoresTheDirectRoute()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 0));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY(rv->route_points.size() > 2);

		moveTable(rig, tab_c, QPointF(700, 1500));
		QCOMPARE(rv->route_points.size(), size_t(2));
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::movingAnObstacleIntoThePathReroutesTheLine()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 1500));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QCOMPARE(rv->route_points.size(), size_t(2));

		moveTable(rig, tab_c, QPointF(700, 0));
		QVERIFY(rv->route_points.size() > 2);
		QVERIFY(!routeCrossesRect(rv->route_points, tableRect(tableView(tab_c))));
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::obstacleMovedAlongTheLineKeepsAllRoutesValid()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 0)), *tab_d = addTable(rig, "tab_d", QPointF(700, 600));
		RelationshipView *rv_ab = relView(addRel(rig, tab_a, tab_b)), *rv_cd = relView(addRel(rig, tab_c, tab_d));

		QRectF rect_a = tableRect(tableView(tab_a)), rect_b = tableRect(tableView(tab_b));
		// The obstacle keeps away from the tables, otherwise there's no room for the line to leave them
		double min_x = rect_a.right() + 140, max_x = rect_b.left() - 140 - tableRect(tableView(tab_c)).width();

		for(double x = min_x; x <= max_x; x += (max_x - min_x) / 6)
		{
			moveTable(rig, tab_c, QPointF(x, 0));
			QVERIFY2(checkRoute(rv_ab, true).isEmpty(), qPrintable(checkRoute(rv_ab, true)));
			QVERIFY2(checkRoute(rv_cd, true).isEmpty(), qPrintable(checkRoute(rv_cd, true)));
			QVERIFY(!routeCrossesRect(rv_ab->route_points, tableRect(tableView(tab_c))));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::hiddenTablesAreNotObstacles()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 0));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY(rv->route_points.size() > 2);

		tableView(tab_c)->setVisible(false);
		rv->invalidateRoute();
		QCOMPARE(rv->route_points.size(), size_t(2));

		tableView(tab_c)->setVisible(true);
		rv->invalidateRoute();
		QVERIFY(rv->route_points.size() > 2);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::tablesOfTheRelationshipAreNotCrossedByTheDetour()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0));

		// Obstacles around both tables force the line to wind its way out
		addTable(rig, "tab_c", QPointF(700, 0));
		addTable(rig, "tab_d", QPointF(700, -400));
		addTable(rig, "tab_e", QPointF(700, 400));

		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		QRectF rect_a = tableRect(tableView(tab_a)), rect_b = tableRect(tableView(tab_b));

		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));

		// Only the first segment starts on a table's border, the rest never goes through the tables
		for(size_t i = 1; i < rv->route_points.size(); i++)
		{
			QVERIFY(!segmentCrossesRect(rv->route_points[i - 1], rv->route_points[i], rect_a));
			QVERIFY(!segmentCrossesRect(rv->route_points[i - 1], rv->route_points[i], rect_b));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::moreObstaclesThanTheSearchSupportsKeepTheSimpleRoute()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 0), *tab_b = addTable(rig, "tab_b", QPointF(3000, 0), 0);

		// 90 tables in the region of the line: more than the search accepts (80)
		for(int i = 0; i < 90; i++)
			addTable(rig, QString("obst_%1").arg(i), QPointF(300 + (i % 10) * 250, -600 + (i / 10) * 140), 0);

		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aTableBoxedInByOthersStillGetsAValidRoute()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 0), *tab_b = addTable(rig, "tab_b", QPointF(2000, 0), 0);
		QRectF rect_b = tableRect(tableView(tab_b));

		// A ring of tables around b: the search finds no way in, so the line keeps the simple route
		for(int i = -1; i <= 1; i++)
		{
			for(int j = -1; j <= 1; j++)
			{
				if(i == 0 && j == 0)
					continue;

				addTable(rig, QString("wall_%1_%2").arg(i + 1).arg(j + 1),
								 QPointF(rect_b.x() + i * (rect_b.width() + 6), rect_b.y() + j * (rect_b.height() + 6)), 0);
			}
		}

		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

		QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(checkRoute(rv)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::selfRelationshipsKeepTheirLoop()
{
	try
	{
		for(int crows = 0; crows < 2; crows++)
		{
			applyMode(RelationshipView::ConnectTableEdges, crows);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(100, 100));
			Relationship *rel = addRel(rig, tab_a, tab_a, BaseRelationship::Relationship1n, "rel_self");
			RelationshipView *rv = relView(rel);

			QVERIFY(rv != nullptr);
			QVERIFY(rv->route_points.size() >= 3);
			QVERIFY(!drawnPath(rv).empty());

			for(auto &pnt : rv->route_points)
				QVERIFY(std::isfinite(pnt.x()) && std::isfinite(pnt.y()));

			// Self-relationships have no route of their own to be displaced
			QVERIFY(rv->base_route.empty());

			QRectF rect_a = tableRect(tableView(tab_a));
			moveTable(rig, tab_a, QPointF(600, 400));
			QVERIFY(rv->route_points.size() >= 3);
			QVERIFY(std::fabs(rv->route_points[0].x() - 500) < rect_a.width() + 400);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::loadedModelHasTheAnchorsOfEachSideSpreadApart()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		ModelWidget wgt;
		QString file = QString("%1/pagila.dbm").arg(SAMPLESDIR);

		wgt.getDatabaseModel()->createSystemObjects(false);
		wgt.loadModel(file);
		settle(200);

		QString overlap = findOverlappingAnchors(wgt.getScenes().at(0));

		/* The relationships are configured one by one while the model is loaded, so the first ones keep the connection
		 * points they got when fewer neighbors existed on the same side of the table, until they are reconfigured
		 * (e.g. when one of the tables moves) */
		QEXPECT_FAIL("", "Known limitation: the connection points of a loaded model are only spread apart after the lines are reconfigured", Continue);
		QVERIFY2(overlap.isEmpty(), qPrintable(overlap));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::reconfiguringTheLinesOfALoadedModelSpreadsTheAnchors()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		ModelWidget wgt;
		QString file = QString("%1/pagila.dbm").arg(SAMPLESDIR);

		wgt.getDatabaseModel()->createSystemObjects(false);
		wgt.loadModel(file);
		settle(200);

		for(auto &item : wgt.getScenes().at(0)->items())
		{
			if(RelationshipView *rv = dynamic_cast<RelationshipView *>(item))
				rv->invalidateRoute();
		}

		QString overlap = findOverlappingAnchors(wgt.getScenes().at(0));
		QVERIFY2(overlap.isEmpty(), qPrintable(overlap));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::relationshipsAddedLaterRespaceTheExistingOnes()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, true);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0), 10);
		std::vector<RelationshipView *> views;

		for(int i = 0; i < 4; i++)
			views.push_back(relView(addRel(rig, tab_a, addTable(rig, QString("tab_b%1").arg(i), QPointF(900, i * 30.0)))));

		settle(200);

		QRectF rect_a = tableRect(tableView(tab_a));
		double min_gap = RelationshipView::ConnLineLength * 1.3 * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();
		std::vector<double> ys;

		for(auto &rv : views)
			ys.push_back(endOn(rv->route_points, rect_a).y());

		std::sort(ys.begin(), ys.end());

		// The relationships created first end up as spread as the last one, not only the one added last
		for(size_t i = 1; i < ys.size(); i++)
			QVERIFY2(ys[i] - ys[i - 1] >= min_gap - 0.6, qPrintable(QString("%1 and %2: %3 (min %4)").arg(i - 1).arg(i).arg(ys[i] - ys[i - 1]).arg(min_gap)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aCrossingGivesABumpOnTheHorizontalLine()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;

		// a -> b is a horizontal line; c -> d is a vertical one, crossing it in the middle
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1600, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, -700)), *tab_d = addTable(rig, "tab_d", QPointF(700, 700));
		RelationshipView *rv_h = relView(addRel(rig, tab_a, tab_b)), *rv_v = relView(addRel(rig, tab_c, tab_d));

		settle(200);

		QCOMPARE(rv_h->route_points.size(), size_t(2));
		QCOMPARE(rv_v->route_points.size(), size_t(2));
		QVERIFY(rv_h->route_points[0].x() != rv_h->route_points[1].x());

		QCOMPARE(rv_h->bumps.size(), size_t(1));
		QVERIFY(rv_v->bumps.empty());
		QVERIFY(std::fabs(rv_h->bumps[0].x() - rv_v->route_points[0].x()) < Eps);
		QVERIFY(std::fabs(rv_h->bumps[0].y() - rv_h->route_points[0].y()) < Eps);

		// The hop is part of the drawn path: the curve goes above the line
		double top = rv_h->route_points[0].y();

		for(auto &pnt : drawnPath(rv_h))
			top = std::min(top, pnt.y());

		QVERIFY(top < rv_h->route_points[0].y() - 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::bumpsDisappearWhenTheCrossingLineMovesAway()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1600, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, -700)), *tab_d = addTable(rig, "tab_d", QPointF(700, 700));
		RelationshipView *rv_h = relView(addRel(rig, tab_a, tab_b));

		addRel(rig, tab_c, tab_d);
		settle(200);
		QCOMPARE(rv_h->bumps.size(), size_t(1));

		// Both tables of the vertical line go to the right of the horizontal one
		moveTable(rig, tab_c, QPointF(2200, -700));
		moveTable(rig, tab_d, QPointF(2200, 700), 200);
		QVERIFY2(rv_h->bumps.empty(), qPrintable(QString("%1 bumps").arg(rv_h->bumps.size())));

		// ...and comes back
		moveTable(rig, tab_c, QPointF(700, -700));
		moveTable(rig, tab_d, QPointF(700, 700), 200);
		QCOMPARE(rv_h->bumps.size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::linesThatDoNotCrossGiveNoBumps()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1600, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 400)), *tab_d = addTable(rig, "tab_d", QPointF(700, 1500));
		RelationshipView *rv_h = relView(addRel(rig, tab_a, tab_b)), *rv_v = relView(addRel(rig, tab_c, tab_d));

		settle(200);
		QVERIFY(rv_h->bumps.empty());
		QVERIFY(rv_v->bumps.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

// Direct control of the routes: the scene calculates the bumps from the stored routes only.

void RelationshipLinesTest::prepareBumpsRig(Rig &rig, int rel_cnt, std::vector<RelationshipView *> &views)
{
	applyMode(RelationshipView::ConnectTableEdges, false);

	for(int i = 0; i < rel_cnt; i++)
	{
		Table *src = addTable(rig, QString("src_%1").arg(i), QPointF(i * 400.0, 0), 0),
				*dst = addTable(rig, QString("dst_%1").arg(i), QPointF(i * 400.0, 5000), 0);

		views.push_back(relView(addRel(rig, src, dst)));
	}

	rig.scene->bumps_timer.stop();
	rig.scene->bumps_update_pending = false;

	for(auto &rv : views)
	{
		rv->bumps.clear();
		rv->end_zones.clear();
		rv->zone_retries = 0;
	}
}

void RelationshipLinesTest::bumpsRigSetRoute(RelationshipView *rv, const std::vector<QPointF> &route)
{
	rv->route_points = route;
	rv->bumps.clear();
}

void RelationshipLinesTest::manualCrossingGivesOnlyTheHorizontalLineABump()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 200) });
		rig.scene->updateRelationshipBumps();

		QCOMPARE(views[0]->bumps.size(), size_t(1));
		QCOMPARE(views[0]->bumps[0], QPointF(100, 100));
		QVERIFY(views[1]->bumps.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crossingsCloseToTheEndsOfTheHorizontalSegmentAreIgnored()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;
		double margin = RelationshipView::RoundedCornerRadius + RelationshipView::BumpRadius;

		prepareBumpsRig(rig, 2, views);

		// The segment goes from 0 to 200: the hop needs the corner radius and the arc radius on both sides
		for(double x : { 1.0, margin - 1, 200 - margin + 1, 199.0, 0.0, 200.0 })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
			bumpsRigSetRoute(views[1], { QPointF(x, 0), QPointF(x, 200) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.empty(), qPrintable(QString("x=%1").arg(x)));
		}

		for(double x : { margin, margin + 1, 100.0, 200 - margin - 1, 200 - margin })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
			bumpsRigSetRoute(views[1], { QPointF(x, 0), QPointF(x, 200) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.size() == 1, qPrintable(QString("x=%1").arg(x)));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crossingsCloseToTheEndsOfTheVerticalSegmentAreIgnored()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;
		double radius = RelationshipView::RoundedCornerRadius;

		prepareBumpsRig(rig, 2, views);

		// The horizontal line touching the vertical one near its ends is a T junction, not a crossing
		for(double y : { 0.0, 5.0, radius, 200 - radius, 195.0, 200.0, 250.0, -50.0 })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, y), QPointF(200, y) });
			bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 200) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.empty(), qPrintable(QString("y=%1").arg(y)));
		}

		for(double y : { radius + 1, 100.0, 200 - radius - 1 })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, y), QPointF(200, y) });
			bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 200) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.size() == 1, qPrintable(QString("y=%1").arg(y)));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aLineNeverHopsOverItself()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 1, views);

		// A spiral-like route crossing itself
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(300, 100), QPointF(300, 300), QPointF(100, 300), QPointF(100, 0) });
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::collinearAndParallelSegmentsGiveNoBumps()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);

		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
		bumpsRigSetRoute(views[1], { QPointF(50, 100), QPointF(300, 100) });
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty() && views[1]->bumps.empty());

		bumpsRigSetRoute(views[0], { QPointF(100, 0), QPointF(100, 200) });
		bumpsRigSetRoute(views[1], { QPointF(100, 50), QPointF(100, 300) });
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty() && views[1]->bumps.empty());

		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
		bumpsRigSetRoute(views[1], { QPointF(0, 120), QPointF(200, 120) });
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty() && views[1]->bumps.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crossingsTooCloseToEachOtherAreMerged()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;
		double radius = RelationshipView::BumpRadius;

		prepareBumpsRig(rig, 3, views);
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(400, 100) });

		// Two vertical lines closer than 3 radii: the arcs would overlap, only the first hop is kept
		bumpsRigSetRoute(views[1], { QPointF(150, 0), QPointF(150, 200) });
		bumpsRigSetRoute(views[2], { QPointF(150 + radius * 2, 0), QPointF(150 + radius * 2, 200) });
		rig.scene->updateRelationshipBumps();
		QCOMPARE(views[0]->bumps.size(), size_t(1));
		QCOMPARE(views[0]->bumps[0].x(), 150.0);

		// Far enough: both hops are kept
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(400, 100) });
		bumpsRigSetRoute(views[2], { QPointF(150 + radius * 3, 0), QPointF(150 + radius * 3, 200) });
		rig.scene->updateRelationshipBumps();
		QCOMPARE(views[0]->bumps.size(), size_t(2));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::severalCrossingsGiveSeveralBumpsInOrder()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 6, views);
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(1000, 100) });

		// The vertical lines are created from right to left, the bumps come sorted
		for(int i = 1; i < 6; i++)
			bumpsRigSetRoute(views[i], { QPointF(1000 - i * 150, 0), QPointF(1000 - i * 150, 200) });

		rig.scene->updateRelationshipBumps();
		QCOMPARE(views[0]->bumps.size(), size_t(5));

		for(size_t i = 1; i < views[0]->bumps.size(); i++)
			QVERIFY(views[0]->bumps[i].x() > views[0]->bumps[i - 1].x());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crossingsOverACrowsFootAreaAreIgnored()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;
		double radius = RelationshipView::BumpRadius, clearance = RelationshipView::ZoneClearance;

		prepareBumpsRig(rig, 2, views);

		// The area of a descriptor of the vertical line: from y=0 to y=40 along it
		views[1]->end_zones = { QRectF(QPointF(90, 0), QPointF(110, 40)) };
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 300) });

		// The arc goes up for 'radius' units; the area is shrunk by the clearance the routes keep from it
		double limit = 40 - clearance + radius;

		// Inside the area, or so close that the arc would go into it: no bump
		for(double y : { 10.0, 39.0, 40.0, limit - 1 })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, y), QPointF(300, y) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.empty(), qPrintable(QString("y=%1").arg(y)));
		}

		// From there on the hop is drawn
		for(double y : { limit, limit + 1, 120.0 })
		{
			bumpsRigSetRoute(views[0], { QPointF(0, y), QPointF(300, y) });
			rig.scene->updateRelationshipBumps();
			QVERIFY2(views[0]->bumps.size() == 1, qPrintable(QString("y=%1").arg(y)));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aLineRightBelowTheAreaOfADescriptorStillHopsOverTheOtherLines()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);

		/* The horizontal line runs at the clearance kept from the area of the vertical line's descriptor (regression:
		 * the area used to be padded by twice the arc radius and the hop disappeared) */
		double y = 40 + RelationshipView::ZoneClearance + 0.5;

		views[1]->end_zones = { QRectF(QPointF(90, 0), QPointF(110, 40)) };
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 300) });
		bumpsRigSetRoute(views[0], { QPointF(0, y), QPointF(300, y) });
		rig.scene->updateRelationshipBumps();

		QCOMPARE(views[0]->bumps.size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::crossingsBesideACrowsFootAreaStillGetABump()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);

		// The zone belongs to a descriptor at the side of the crossing (not over the crossing point): the hop is drawn
		views[1]->end_zones = { QRectF(QPointF(200, 90), QPointF(240, 110)) };
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 300) });
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(300, 100) });
		rig.scene->updateRelationshipBumps();

		QVERIFY(views[0]->bumps.size() == 0 || views[0]->bumps.size() == 1);

		// A zone far from the segments changes nothing
		views[1]->end_zones = { QRectF(QPointF(1000, 1000), QPointF(1040, 1020)) };
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 300) });
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(300, 100) });
		rig.scene->updateRelationshipBumps();
		QCOMPARE(views[0]->bumps.size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::invisibleRelationshipsDoNotTakePartInTheBumps()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 200) });

		// The vertical line is hidden: nothing to hop over
		views[1]->setVisible(false);
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty());

		views[1]->setVisible(true);
		rig.scene->updateRelationshipBumps();
		QCOMPARE(views[0]->bumps.size(), size_t(1));

		// The horizontal line is hidden: it keeps the bumps it had, no new calculation is made for it
		views[0]->setVisible(false);
		views[0]->bumps.clear();
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::bumpsAreNotCalculatedWhileObjectsAreMoving()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);
		bumpsRigSetRoute(views[0], { QPointF(0, 100), QPointF(200, 100) });
		bumpsRigSetRoute(views[1], { QPointF(100, 0), QPointF(100, 200) });

		rig.scene->moving_objs = true;
		rig.scene->updateRelationshipBumps();
		QVERIFY(views[0]->bumps.empty());
		QVERIFY(rig.scene->bumps_update_pending);

		rig.scene->requestBumpsUpdate();
		QVERIFY(!rig.scene->bumps_timer.isActive());

		// The movement ends: the pending update is scheduled
		rig.scene->moving_objs = false;
		rig.scene->bumps_update_pending = true;
		rig.scene->finishObjectsMove(QPointF(0, 0));
		QVERIFY(rig.scene->bumps_timer.isActive());

		settle(150);
		QCOMPARE(views[0]->bumps.size(), size_t(1));
		QVERIFY(!rig.scene->bumps_update_pending);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::requestsAreCoalescedInASingleCalculation()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 2, views);

		for(int i = 0; i < 20; i++)
			rig.scene->requestBumpsUpdate();

		QVERIFY(rig.scene->bumps_timer.isActive());
		QVERIFY(rig.scene->bumps_timer.isSingleShot());
		settle(150);
		QVERIFY(!rig.scene->bumps_timer.isActive());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::linesInsideTheZonesOfOthersAreRecalculatedAtMostTwice()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1600, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 700)), *tab_d = addTable(rig, "tab_d", QPointF(700, 1400));
		RelationshipView *rv_1 = relView(addRel(rig, tab_a, tab_b)), *rv_2 = relView(addRel(rig, tab_c, tab_d));

		settle(200);

		// An area, registered by the second line, over the path of the first one that no route can avoid
		rv_2->end_zones = { QRectF(QPointF(-100000, -100000), QPointF(100000, 100000)) };
		rv_1->zone_retries = 0;
		rig.scene->bumps_timer.stop();

		for(int i = 0; i < 6; i++)
		{
			rig.scene->updateRelationshipBumps();
			settle(10);
		}

		QVERIFY(rv_1->zone_retries <= RelationshipView::MaxZoneRetries);
		QVERIFY2(checkRoute(rv_1).isEmpty(), qPrintable(checkRoute(rv_1)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::bumpsMatchABruteForceCalculationOnRandomRoutes()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;
		QRandomGenerator rnd(2026);
		double radius = RelationshipView::BumpRadius, corner = RelationshipView::RoundedCornerRadius, margin = corner + radius;
		const int RelCount = 14;

		prepareBumpsRig(rig, RelCount, views);

		for(int round = 0; round < 40; round++)
		{
			std::vector<std::vector<QPointF>> routes(RelCount);

			for(int i = 0; i < RelCount; i++)
			{
				QPointF cur(rnd.bounded(600), rnd.bounded(600));
				bool horiz = rnd.bounded(2);

				routes[i].push_back(cur);

				for(int k = 0, segs = 1 + rnd.bounded(5); k < segs; k++)
				{
					double len = (30 + rnd.bounded(300)) * (rnd.bounded(2) ? 1 : -1);

					if(horiz)
						cur.setX(cur.x() + len);
					else
						cur.setY(cur.y() + len);

					routes[i].push_back(cur);
					horiz = !horiz;
				}

				bumpsRigSetRoute(views[i], routes[i]);
			}

			rig.scene->updateRelationshipBumps();

			// Brute force: every horizontal segment against every vertical one of the other lines
			for(int i = 0; i < RelCount; i++)
			{
				std::vector<QPointF> expected;

				for(size_t s = 1; s < routes[i].size(); s++)
				{
					QPointF p1 = routes[i][s - 1], p2 = routes[i][s];

					if(!(std::fabs(p1.y() - p2.y()) < 0.01 && std::fabs(p1.x() - p2.x()) > 0.01) || std::fabs(p1.x() - p2.x()) < margin * 2)
						continue;

					for(int j = 0; j < RelCount; j++)
					{
						if(j == i)
							continue;

						for(size_t t = 1; t < routes[j].size(); t++)
						{
							QPointF q1 = routes[j][t - 1], q2 = routes[j][t];

							if(!(std::fabs(q1.x() - q2.x()) < 0.01 && std::fabs(q1.y() - q2.y()) > 0.01))
								continue;

							if(q1.x() >= std::min(p1.x(), p2.x()) + margin && q1.x() <= std::max(p1.x(), p2.x()) - margin &&
								 p1.y() > std::min(q1.y(), q2.y()) + corner && p1.y() < std::max(q1.y(), q2.y()) - corner)
								expected.push_back(QPointF(q1.x(), p1.y()));
						}
					}
				}

				std::sort(expected.begin(), expected.end(), [](const QPointF &a, const QPointF &b){
					return a.y() != b.y() ? a.y() < b.y() : a.x() < b.x(); });

				std::vector<QPointF> filtered;

				for(auto &bump : expected)
				{
					if(filtered.empty() || filtered.back().y() != bump.y() || bump.x() - filtered.back().x() >= radius * 3)
						filtered.push_back(bump);
				}

				QVERIFY2(views[i]->bumps == filtered, qPrintable(QString("round %1, line %2: %3 bumps, expected %4")
																												 .arg(round).arg(i).arg(views[i]->bumps.size()).arg(filtered.size())));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

RelationshipLinesTest::ZLayout RelationshipLinesTest::makeZ(Rig &rig, bool crows_foot)
{
	ZLayout z;

	applyMode(RelationshipView::ConnectTableEdges, crows_foot);

	z.a = addTable(rig, "tab_a", QPointF(0, 0));
	z.b = addTable(rig, "tab_b", QPointF(5000, 5000));
	z.rel = addRel(rig, z.a, z.b);
	z.rv = relView(z.rel);

	QRectF rect_a = tableRect(tableView(z.a));

	// The second table is on the right and lower: Z shaped route, the middle segment is vertical
	moveTable(rig, z.b, QPointF(rect_a.right() + 500, rect_a.bottom() + 400));
	return z;
}

void RelationshipLinesTest::minimumLengthsAreKept(RelationshipView *rv, QString &err)
{
	double min_len = RelationshipView::RoundedCornerRadius * BaseObjectView::getScreenDpiFactor();
	const std::vector<QPointF> &route = rv->route_points;

	err.clear();

	for(size_t i = 1; i < route.size(); i++)
	{
		double len = QLineF(route[i - 1], route[i]).length(), min = min_len;

		if(i == 1)
			min = std::max(min_len, rv->route_stubs[0]);

		if(i == route.size() - 1)
			min = std::max(min, std::max(min_len, rv->route_stubs[1]));

		// The segments at the ends of a route with only the two of them aren't displaced
		if(route.size() > 2 && len < min - 0.5)
		{
			err = QString("segment %1 has %2 units, it needs %3").arg(i - 1).arg(len).arg(min);
			return;
		}
	}
}

void RelationshipLinesTest::aShiftMovesTheMiddleSegmentOfARoute()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QCOMPARE(z.rv->route_points.size(), size_t(4));
		std::vector<QPointF> base = z.rv->route_points;

		for(double value : { 40.0, -40.0, 5.5, -3.25 })
		{
			z.rel->setSegmentShifts({{ 1, { Qt::Vertical, value } }});
			z.rv->invalidateRoute();

			QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
			QCOMPARE(z.rv->route_points.size(), size_t(4));
			QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + value)) < Eps);
			QVERIFY(std::fabs(z.rv->route_points[2].x() - (base[2].x() + value)) < Eps);

			// The rest of the line doesn't move
			QCOMPARE(z.rv->route_points[0], base[0]);
			QCOMPARE(z.rv->route_points[3], base[3]);
			QCOMPARE(z.rv->route_points[1].y(), base[1].y());
			QCOMPARE(z.rv->route_points[2].y(), base[2].y());
		}

		z.rel->resetSegmentShifts();
		z.rv->invalidateRoute();
		QCOMPARE(z.rv->route_points, base);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsDoNotAccumulateAndTheBaseRouteIsKept()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 30 } }});

		for(int i = 0; i < 5; i++)
		{
			z.rv->invalidateRoute();
			QCOMPARE(z.rv->base_route, base);
			QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 30)) < Eps);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aShiftOfZeroOrOfAnotherOrientationIsIgnored()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		// Zero
		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 0 } }});
		z.rv->invalidateRoute();
		QCOMPARE(z.rv->route_points, base);

		// The middle segment is vertical, a horizontal displacement doesn't apply to it
		z.rel->setSegmentShifts({{ 1, { Qt::Horizontal, 50 } }});
		z.rv->invalidateRoute();
		QCOMPARE(z.rv->route_points, base);

		// The shift is kept in the model even when it doesn't apply
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theSegmentsAtTheEndsAreNeverShifted()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		for(auto orient : { Qt::Horizontal, Qt::Vertical })
		{
			for(unsigned idx : { 0u, 2u })
			{
				z.rel->setSegmentShifts({{ idx, { orient, 60 } }});
				z.rv->invalidateRoute();
				QVERIFY2(z.rv->route_points == base, qPrintable(QString("index %1").arg(idx)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::indexesOutOfTheRouteAreIgnored()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		for(unsigned idx : { 3u, 4u, 100u, 4000000000u, 0xFFFFFFFFu })
		{
			for(auto orient : { Qt::Horizontal, Qt::Vertical })
			{
				z.rel->setSegmentShifts({{ idx, { orient, 60 } }});
				z.rv->invalidateRoute();
				QVERIFY2(z.rv->route_points == base, qPrintable(QString("index %1").arg(idx)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::routesWithoutInnerSegmentsIgnoreTheShifts()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(600, 0));
		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		std::vector<QPointF> base = rv->route_points;

		QCOMPARE(base.size(), size_t(2));

		rel->setSegmentShifts({{ 0, { Qt::Horizontal, 20 } }, { 1, { Qt::Horizontal, 20 } }, { 1, { Qt::Vertical, 20 } }});
		rv->invalidateRoute();
		QCOMPARE(rv->route_points, base);
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aShiftIsLimitedByTheLengthTheNeighborSegmentsNeed()
{
	try
	{
		for(int crows = 0; crows < 2; crows++)
		{
			Rig rig;
			ZLayout z = makeZ(rig, crows);
			std::vector<QPointF> base = z.rv->route_points;
			QString err;

			QCOMPARE(base.size(), size_t(4));

			for(double value : { 100000.0, -100000.0, 1e12, -1e12 })
			{
				z.rel->setSegmentShifts({{ 1, { Qt::Vertical, value } }});
				z.rv->invalidateRoute();

				QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
				minimumLengthsAreKept(z.rv, err);
				QVERIFY2(err.isEmpty(), qPrintable(err));

				// The segment moved to the direction of the shift and stayed between the two ends
				double min_x = std::min(base[0].x(), base[3].x()), max_x = std::max(base[0].x(), base[3].x());
				QVERIFY(z.rv->route_points[1].x() > min_x && z.rv->route_points[1].x() < max_x);
				QVERIFY((value > 0) == (z.rv->route_points[1].x() > base[1].x() - Eps));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aShiftIsLimitedByTheAreaOfTheRelationship()
{
	try
	{
		// A C shaped route (both ends leave the tables to the same side) is the one the area limits
		applyMode(RelationshipView::ConnectFkToPk, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		QRectF rect_a = tableRect(tableView(tab_a));
		bool found = false;

		// The tables are one above the other: in FK to PK mode both lines leave through the same side
		moveTable(rig, tab_b, QPointF(rect_a.left(), rect_a.bottom() + 300));

		if(rv->route_points.size() == 4 && isHorizontal(rv->route_points[0], rv->route_points[1]) &&
			 isHorizontal(rv->route_points[2], rv->route_points[3]) &&
			 (rv->route_points[1].x() - rv->route_points[0].x()) * (rv->route_points[2].x() - rv->route_points[3].x()) > 0)
			found = true;

		if(!found)
			QSKIP("The layout didn't produce a route that goes around the tables");

		std::vector<QPointF> base = rv->route_points;
		QRectF expected = rv->getRouteRect(base).united(rect_a).united(tableRect(tableView(tab_b)));
		double margin = RelationshipView::ConnLineLength * BaseObjectView::getScreenDpiFactor();

		expected.adjust(-margin, -margin, margin, margin);

		for(double value : { 100000.0, -100000.0 })
		{
			rel->setSegmentShifts({{ 1, { Qt::Vertical, value } }});
			rv->invalidateRoute();

			QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(checkRoute(rv)));
			QVERIFY(rv->route_points[1].x() <= expected.right() + Eps);
			QVERIFY(rv->route_points[1].x() >= expected.left() - Eps);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsSurviveTheUpdatesOfTheNeighborhood()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 35 } }});
		z.rv->invalidateRoute();

		std::vector<QPointF> shifted = z.rv->route_points;
		QVERIFY(shifted != base);

		z.rv->invalidateRoute(true);
		QCOMPARE(z.rv->route_points, shifted);
		z.rv->invalidateRoute(false);
		QCOMPARE(z.rv->route_points, shifted);

		// Other tables moving around, nowhere near the line
		Table *other = addTable(rig, "tab_far", QPointF(-3000, -3000));

		for(auto pos : { QPointF(-3100, -3000), QPointF(-3000, -3300), QPointF(-2000, -2000) })
			moveTable(rig, other, pos, 20);

		QCOMPARE(z.rv->route_points, shifted);
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));

		// A table moved close to the line but not over it: nearby routes are updated, the shift stays
		QRectF rect_a = tableRect(tableView(z.a));
		moveTable(rig, other, QPointF(rect_a.right() + 40, rect_a.bottom() + 800), 20);
		QVERIFY(z.rv->route_points == shifted || z.rv->route_points[1].x() != 0);
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsAreDiscardedWhenATableMoves()
{
	try
	{
		for(int which = 0; which < 2; which++)
		{
			Rig rig;
			ZLayout z = makeZ(rig);
			Table *table = (which == 0 ? z.a : z.b);

			z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 35 } }});
			z.rv->invalidateRoute();
			QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));

			QPointF pos = tableView(table)->pos();
			moveTable(rig, table, pos + QPointF(0, 60));

			QVERIFY2(z.rel->getSegmentShifts().empty(), qPrintable(QString("table %1").arg(which)));
			QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));

			// The route is the plain one again
			QVERIFY(std::fabs(z.rv->route_points[1].x() - z.rv->base_route[1].x()) < Eps);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsAreKeptWhenATableIsMovedToTheSameSpot()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 35 } }});
		z.rv->invalidateRoute();

		moveTable(rig, z.a, tableView(z.a)->pos());
		moveTable(rig, z.b, tableView(z.b)->pos());
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsAreDiscardedWhenATableChangesItsSize()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 35 } }});
		z.rv->invalidateRoute();

		QSizeF before = tableView(z.a)->boundingRect().size();

		Column *col = new Column;
		col->setName("one_more_column_with_a_long_name_to_widen_the_table");
		col->setType(PgSqlType("text"));
		z.a->addColumn(col);
		z.a->setModified(true);
		QMetaObject::invokeMethod(tableView(z.a), "configureObject", Qt::DirectConnection);
		settle(50);

		QVERIFY(tableView(z.a)->boundingRect().size() != before);
		QVERIFY(z.rel->getSegmentShifts().empty());
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsSurviveSavingAndLoadingTheModel()
{
	try
	{
		QString file = QDir(QDir::tempPath()).filePath(QString("relshifts_%1.dbm").arg(QRandomGenerator::global()->generate()));
		std::vector<QPointF> shifted;

		{
			Rig rig;
			ZLayout z = makeZ(rig);

			z.rel->setSegmentShifts({{ 1, { Qt::Vertical, -45 } }});
			z.rv->invalidateRoute();
			shifted = z.rv->route_points;
			rig.db->saveModel(file, SchemaParser::XmlCode);
		}

		ModelWidget wgt;

		wgt.getDatabaseModel()->createSystemObjects(false);
		wgt.loadModel(file);
		settle(200);
		QFile::remove(file);

		BaseRelationship *rel = wgt.getDatabaseModel()->getRelationship(0, ObjectType::Relationship);
		RelationshipView *rv = relView(rel);

		QVERIFY(rv != nullptr);

		// The loading (the tables are placed, configured, moved by the scene...) must not discard the displacement
		QCOMPARE(rel->getSegmentShifts().size(), size_t(1));
		QCOMPARE(rel->getSegmentShifts().at(1).value, -45.0);
		QVERIFY2(rv->route_points.size() == shifted.size(), "the route has another shape after loading");

		for(size_t i = 0; i < shifted.size(); i++)
			QVERIFY2(QLineF(rv->route_points[i], shifted[i]).length() < 1, qPrintable(QString("point %1").arg(i)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::otherLineStylesIgnoreTheShiftsAndTheyComeBack()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 35 } }});
		z.rv->invalidateRoute();

		std::vector<QPointF> shifted = z.rv->route_points;
		QVERIFY(shifted != base);

		for(auto style : { RelationshipView::StraightLines, RelationshipView::CurvedLines })
		{
			RelationshipView::setLineStyle(style);
			z.rv->invalidateRoute();
			QVERIFY(z.rv->route_points.empty());
			QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));

			RelationshipView::setLineStyle(RelationshipView::OrthogonalLines);
			z.rv->invalidateRoute();
			QCOMPARE(z.rv->route_points, shifted);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsApplyInEveryConnectionMode()
{
	try
	{
		for(int mode = 0; mode < 3; mode++)
		{
			applyMode(mode, false);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
			Relationship *rel = addRel(rig, tab_a, tab_b);
			RelationshipView *rv = relView(rel);
			QRectF rect_a = tableRect(tableView(tab_a));

			moveTable(rig, tab_b, QPointF(rect_a.right() + 600, rect_a.bottom() + 500));

			std::vector<QPointF> base = rv->route_points;
			QVERIFY(base.size() >= 4);

			for(size_t idx = 1; idx + 2 < base.size(); idx++)
			{
				Qt::Orientation orient = isHorizontal(base[idx], base[idx + 1]) ? Qt::Horizontal : Qt::Vertical;

				rel->setSegmentShifts({{ unsigned(idx), { orient, 12 } }});
				rv->invalidateRoute();
				QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(QString("mode %1: %2").arg(mode).arg(checkRoute(rv))));
				QVERIFY2(rv->route_points != base, qPrintable(QString("mode %1 index %2").arg(mode).arg(idx)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsOfARouteAroundObstaclesKeepItsShape()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0));

		addTable(rig, "tab_c", QPointF(700, 0));

		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		std::vector<QPointF> base = rv->route_points;
		QString err;

		QVERIFY(base.size() >= 4);

		// Each inner segment, one at a time
		for(size_t idx = 1; idx + 2 < base.size(); idx++)
		{
			Qt::Orientation orient = isHorizontal(base[idx], base[idx + 1]) ? Qt::Horizontal : Qt::Vertical;

			for(double value : { 17.0, -17.0, 500.0, -500.0 })
			{
				rel->setSegmentShifts({{ unsigned(idx), { orient, value } }});
				rv->invalidateRoute();

				QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
				QCOMPARE(rv->route_points.size(), base.size());
				minimumLengthsAreKept(rv, err);
				QVERIFY2(err.isEmpty(), qPrintable(err));
			}
		}

		// All of them at once
		std::map<unsigned, BaseRelationship::SegmentShift> all;

		for(size_t idx = 1; idx + 2 < base.size(); idx++)
			all[idx] = { isHorizontal(base[idx], base[idx + 1]) ? Qt::Horizontal : Qt::Vertical, (idx % 2 ? 23.0 : -31.0) };

		rel->setSegmentShifts(all);
		rv->invalidateRoute();
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));
		minimumLengthsAreKept(rv, err);
		QVERIFY2(err.isEmpty(), qPrintable(err));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::adjacentShiftsNeverCrushTheSegmentBetweenThem()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0));

		addTable(rig, "tab_c", QPointF(700, 0));

		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		std::vector<QPointF> base = rv->route_points;
		QString err;

		QVERIFY(base.size() >= 5);

		// Two neighbor inner segments moved against each other, as far as the user can drag them
		for(double a : { 1e6, -1e6 })
		{
			for(double b : { 1e6, -1e6 })
			{
				rel->setSegmentShifts({{ 1, { isHorizontal(base[1], base[2]) ? Qt::Horizontal : Qt::Vertical, a } },
															 { 2, { isHorizontal(base[2], base[3]) ? Qt::Horizontal : Qt::Vertical, b } }});
				rv->invalidateRoute();
				QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(checkRoute(rv)));
				minimumLengthsAreKept(rv, err);
				QVERIFY2(err.isEmpty(), qPrintable(QString("%1,%2: %3").arg(a).arg(b).arg(err)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aShiftOfOneShapeDoesNotApplyToTheNextShapeOfTheRoute()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		QRectF rect_a = tableRect(tableView(tab_a));

		moveTable(rig, tab_b, QPointF(rect_a.right() + 1400, rect_a.bottom() + 400));
		QCOMPARE(rv->route_points.size(), size_t(4));

		rel->setSegmentShifts({{ 1, { Qt::Vertical, 40 } }});
		rv->invalidateRoute();
		QVERIFY(rv->base_route[1].x() != rv->route_points[1].x());

		// An obstacle in the middle of the line changes the shape of the route: the segment 1 is not the same anymore
		Table *obstacle = addTable(rig, "tab_c", QPointF(rect_a.right() + 600, rect_a.bottom() - 50));
		rv->invalidateRoute();
		QVERIFY2(checkRoute(rv, true).isEmpty(), qPrintable(checkRoute(rv, true)));

		// ...and the obstacle goes away: the displacement applies again
		moveTable(rig, obstacle, QPointF(-5000, -5000));
		rv->invalidateRoute();
		QCOMPARE(rv->route_points.size(), size_t(4));
		QVERIFY(std::fabs(rv->route_points[1].x() - (rv->base_route[1].x() + 40)) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftsDoNotChangeTheCrowsFootAreas()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig, true);
		std::vector<QRectF> zones = z.rv->end_zones;

		QCOMPARE(zones.size(), size_t(2));

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 30 } }});
		z.rv->invalidateRoute();
		QCOMPARE(z.rv->end_zones, zones);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::sendMouse(RelationshipView *rv, QEvent::Type type, const QPointF &pos, Qt::MouseButton button,
																			Qt::MouseButtons buttons, Qt::KeyboardModifiers mods)
{
	QGraphicsSceneMouseEvent ev(type);

	ev.setPos(pos);
	ev.setScenePos(pos);
	ev.setButton(button);
	ev.setButtons(buttons);
	ev.setModifiers(mods);

	if(type == QEvent::GraphicsSceneMousePress)
		rv->mousePressEvent(&ev);
	else if(type == QEvent::GraphicsSceneMouseMove)
		rv->mouseMoveEvent(&ev);
	else
		rv->mouseReleaseEvent(&ev);
}

void RelationshipLinesTest::sendHover(RelationshipView *rv, const QPointF &pos, Qt::KeyboardModifiers mods)
{
	QGraphicsSceneHoverEvent ev(QEvent::GraphicsSceneHoverMove);

	ev.setPos(pos);
	ev.setScenePos(pos);
	ev.setModifiers(mods);
	rv->hoverMoveEvent(&ev);
}

void RelationshipLinesTest::dragSegment(RelationshipView *rv, const QPointF &from, const QPointF &to, Qt::KeyboardModifiers mods, int steps)
{
	rv->setSelected(true);
	sendMouse(rv, QEvent::GraphicsSceneMousePress, from, Qt::LeftButton, Qt::LeftButton, mods);

	for(int i = 1; i <= steps; i++)
		sendMouse(rv, QEvent::GraphicsSceneMouseMove, from + (to - from) * i / steps, Qt::NoButton, Qt::LeftButton, mods);

	sendMouse(rv, QEvent::GraphicsSceneMouseRelease, to, Qt::LeftButton, Qt::NoButton, mods);

	// The line is reconfigured after the release, from the event loop
	settle(0);
}

QPointF RelationshipLinesTest::segmentMiddle(RelationshipView *rv, size_t idx)
{
	return (rv->route_points[idx] + rv->route_points[idx + 1]) / 2;
}

RelationshipLinesTest::ZLayout RelationshipLinesTest::makeVerticalZ(Rig &rig)
{
	ZLayout z;

	applyMode(RelationshipView::ConnectTableEdges, false);

	z.a = addTable(rig, "tab_a", QPointF(0, 0));
	z.b = addTable(rig, "tab_b", QPointF(5000, 5000));
	z.rel = addRel(rig, z.a, z.b);
	z.rv = relView(z.rel);

	QRectF rect_a = tableRect(tableView(z.a));

	// The second table is below and slightly to the right: the middle segment is horizontal
	moveTable(rig, z.b, QPointF(rect_a.left() + 300, rect_a.bottom() + 600));
	return z;
}

void RelationshipLinesTest::dragWithShiftMovesAVerticalSegmentAndStoresTheShift()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		QSignalSpy spy(z.rv, &RelationshipView::s_relationshipModified);
		QPointF mid = segmentMiddle(z.rv, 1);

		QCOMPARE(base.size(), size_t(4));
		QVERIFY(!isHorizontal(base[1], base[2]));

		dragSegment(z.rv, mid, mid + QPointF(30, 0), Qt::ShiftModifier);

		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QCOMPARE(z.rel->getSegmentShifts().at(1).orientation, Qt::Vertical);
		QVERIFY(std::fabs(z.rel->getSegmentShifts().at(1).value - 30) < Eps);
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 30)) < Eps);
		QVERIFY(std::fabs(z.rv->route_points[2].x() - (base[2].x() + 30)) < Eps);
		QCOMPARE(z.rv->route_points[1].y(), base[1].y());
		QCOMPARE(spy.count(), 1);
		QCOMPARE(z.rv->drag_seg_idx, -1);
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));

		// The vertical segment only moves sideways: a vertical mouse movement changes nothing
		QPointF new_mid = segmentMiddle(z.rv, 1);
		dragSegment(z.rv, new_mid, new_mid + QPointF(0, 80), Qt::ShiftModifier);
		QVERIFY(std::fabs(z.rel->getSegmentShifts().at(1).value - 30) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::dragWithShiftMovesAHorizontalSegment()
{
	try
	{
		Rig rig;
		ZLayout z = makeVerticalZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		QPointF mid = segmentMiddle(z.rv, 1);

		QCOMPARE(base.size(), size_t(4));
		QVERIFY(isHorizontal(base[1], base[2]));

		dragSegment(z.rv, mid, mid + QPointF(0, -25), Qt::ShiftModifier);

		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QCOMPARE(z.rel->getSegmentShifts().at(1).orientation, Qt::Horizontal);
		QVERIFY(std::fabs(z.rel->getSegmentShifts().at(1).value + 25) < Eps);
		QVERIFY(std::fabs(z.rv->route_points[1].y() - (base[1].y() - 25)) < Eps);
		QVERIFY(std::fabs(z.rv->route_points[2].y() - (base[2].y() - 25)) < Eps);
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::thePreviewOfTheDragDoesNotTouchTheModelUntilTheRelease()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points, before = drawnPath(z.rv);
		QPointF mid = segmentMiddle(z.rv, 1);

		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		QVERIFY(z.rv->drag_seg_idx == 1);

		sendMouse(z.rv, QEvent::GraphicsSceneMouseMove, mid + QPointF(20, 0), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseMove, mid + QPointF(40, 0), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);

		// The curve shows the line moved, the route and the model keep the old values
		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(z.rv->route_points, base);
		QVERIFY(drawnPath(z.rv) != before);

		double max_x = 0;

		for(auto &pnt : drawnPath(z.rv))
			max_x = std::max(max_x, pnt.x());

		QVERIFY(max_x > 0);

		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid + QPointF(40, 0), Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		settle(0);
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 40)) < Eps);

		// What was drawn during the drag is what the line shows after the reconfiguration
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 40)) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::draggingFarBeyondTheLimitsStopsAtTheLimit()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);
		QString err;

		for(double dx : { 100000.0, -100000.0 })
		{
			dragSegment(z.rv, segmentMiddle(z.rv, 1), segmentMiddle(z.rv, 1) + QPointF(dx, 0), Qt::ShiftModifier, 3);

			QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
			minimumLengthsAreKept(z.rv, err);
			QVERIFY2(err.isEmpty(), qPrintable(err));

			// The stored value is the effective displacement, not what the mouse asked for
			double stored = z.rel->getSegmentShifts().at(1).value;
			QVERIFY(std::fabs(stored) < 5000);
			QVERIFY(std::fabs(z.rv->route_points[1].x() - (z.rv->base_route[1].x() + stored)) < Eps);
		}

		QVERIFY(mid.x() > 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::releasingWithoutMovingStoresNothing()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QSignalSpy spy(z.rv, &RelationshipView::s_relationshipModified);
		QPointF mid = segmentMiddle(z.rv, 1);
		std::vector<QPointF> base = z.rv->route_points;

		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		settle(0);

		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(spy.count(), 0);
		QCOMPARE(z.rv->drag_seg_idx, -1);
		QCOMPARE(z.rv->route_points, base);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::draggingBackToTheOriginalPlaceRemovesTheShift()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);

		dragSegment(z.rv, mid, mid + QPointF(40, 0), Qt::ShiftModifier);
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));

		QPointF new_mid = segmentMiddle(z.rv, 1);
		dragSegment(z.rv, new_mid, new_mid - QPointF(40, 0), Qt::ShiftModifier);
		QVERIFY(z.rel->getSegmentShifts().empty());
		QVERIFY(std::fabs(z.rv->route_points[1].x() - z.rv->base_route[1].x()) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aSecondDragStartsFromTheCurrentPosition()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		double base_x = z.rv->route_points[1].x();

		dragSegment(z.rv, segmentMiddle(z.rv, 1), segmentMiddle(z.rv, 1) + QPointF(30, 0), Qt::ShiftModifier);
		dragSegment(z.rv, segmentMiddle(z.rv, 1), segmentMiddle(z.rv, 1) + QPointF(-70, 0), Qt::ShiftModifier);

		QVERIFY(std::fabs(z.rel->getSegmentShifts().at(1).value + 40) < Eps);
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base_x - 40)) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::dragWithoutShiftNeverMovesASegment()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		QPointF mid = segmentMiddle(z.rv, 1);

		for(auto mods : { Qt::KeyboardModifiers(Qt::NoModifier), Qt::KeyboardModifiers(Qt::ControlModifier),
											Qt::KeyboardModifiers(Qt::AltModifier), Qt::KeyboardModifiers(Qt::ControlModifier | Qt::ShiftModifier) })
		{
			dragSegment(z.rv, mid, mid + QPointF(50, 0), mods);
			QVERIFY2(z.rel->getSegmentShifts().empty(), qPrintable(QString("modifiers %1").arg(int(mods))));
			QCOMPARE(z.rv->route_points, base);
			QCOMPARE(z.rv->drag_seg_idx, -1);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::onlyTheLeftButtonStartsTheDrag()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);

		z.rv->setSelected(true);

		for(auto btn : { Qt::RightButton, Qt::MiddleButton })
		{
			sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, btn, btn, Qt::ShiftModifier);
			QCOMPARE(z.rv->drag_seg_idx, -1);
			sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, btn, Qt::NoButton, Qt::ShiftModifier);
		}

		QVERIFY(z.rel->getSegmentShifts().empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theEndSegmentsAndTheEmptySpaceCannotBeDragged()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		double tolerance = RelationshipView::LineSelStrokeWidth / 2;

		z.rv->setSelected(true);

		// First and last segments
		for(size_t idx : { size_t(0), size_t(2) })
		{
			QPointF mid = segmentMiddle(z.rv, idx);

			sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
			QCOMPARE(z.rv->drag_seg_idx, -1);
			sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		}

		// Next to the middle segment, but outside the tolerance, or beyond its ends
		QPointF mid = segmentMiddle(z.rv, 1);

		for(auto pos : { mid + QPointF(tolerance + 5, 0), mid - QPointF(tolerance + 5, 0), mid + QPointF(0, 5000), QPointF(-9000, -9000) })
		{
			sendMouse(z.rv, QEvent::GraphicsSceneMousePress, pos, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
			QCOMPARE(z.rv->drag_seg_idx, -1);
			sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, pos, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		}

		// Inside the tolerance it picks the segment
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid + QPointF(tolerance - 1, 0), Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		QCOMPARE(z.rv->drag_seg_idx, 1);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);

		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(z.rv->route_points, base);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aProtectedRelationshipCannotBeDragged()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		QPointF mid = segmentMiddle(z.rv, 1);

		z.rel->setProtected(true);
		dragSegment(z.rv, mid, mid + QPointF(40, 0), Qt::ShiftModifier);

		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(z.rv->route_points, base);

		// ...not even the cursor suggests it
		sendHover(z.rv, mid, Qt::ShiftModifier);
		QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::selfRelationshipsCannotBeDragged()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(100, 100));
		Relationship *rel = addRel(rig, tab_a, tab_a, BaseRelationship::Relationship1n, "rel_self");
		RelationshipView *rv = relView(rel);

		QVERIFY(rv->route_points.size() >= 3);

		for(size_t i = 0; i + 1 < rv->route_points.size(); i++)
		{
			dragSegment(rv, segmentMiddle(rv, i), segmentMiddle(rv, i) + QPointF(20, 20), Qt::ShiftModifier);
			QVERIFY(rel->getSegmentShifts().empty());
			QCOMPARE(rv->drag_seg_idx, -1);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::shiftClickInTheOtherStylesStillEditsThePoints()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);

		for(auto style : { RelationshipView::StraightLines, RelationshipView::CurvedLines })
		{
			RelationshipView::setLineStyle(style);
			z.rel->setPoints({});
			z.rv->invalidateRoute();

			QRectF rect_a = tableRect(tableView(z.a));
			QPointF pnt = z.rv->lines.empty() ? QPointF() : (z.rv->lines[0]->line().p1() + z.rv->lines[0]->line().p2()) / 2;

			if(style == RelationshipView::CurvedLines && !z.rv->curves.empty())
				pnt = z.rv->curves[0]->path().pointAtPercent(0.5);

			z.rv->setSelected(true);
			sendMouse(z.rv, QEvent::GraphicsSceneMousePress, pnt, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
			sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, pnt, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);

			QVERIFY2(z.rel->getPoints().size() == 1, qPrintable(QString("style %1").arg(int(style))));
			QCOMPARE(z.rv->drag_seg_idx, -1);
			QVERIFY(rect_a.isValid());
		}

		// In the orthogonal style a point is never added
		RelationshipView::setLineStyle(RelationshipView::OrthogonalLines);
		z.rel->setPoints({});
		z.rv->invalidateRoute();

		QPointF mid = segmentMiddle(z.rv, 1);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		QVERIFY(z.rel->getPoints().empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::altShiftClickResetsTheShiftsAndTheLabels()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		std::vector<QPointF> base = z.rv->route_points;
		QSignalSpy spy(z.rv, &RelationshipView::s_relationshipModified);

		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 45 } }});
		z.rel->setLabelDistance(BaseRelationship::SrcCardLabel, QPointF(30, 40));
		z.rv->invalidateRoute();
		QVERIFY(z.rv->route_points != base);

		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, segmentMiddle(z.rv, 0), Qt::LeftButton, Qt::LeftButton, Qt::AltModifier | Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, segmentMiddle(z.rv, 0), Qt::LeftButton, Qt::NoButton, Qt::AltModifier | Qt::ShiftModifier);
		settle(0);

		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(z.rv->route_points, base);
		QVERIFY(std::isnan(z.rel->getLabelDistance(BaseRelationship::SrcCardLabel).x()));
		QVERIFY(spy.count() >= 1);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::altShiftClickWithoutShiftsStillResetsTheLabels()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QSignalSpy spy(z.rv, &RelationshipView::s_relationshipModified);

		z.rel->setLabelDistance(BaseRelationship::DstCardLabel, QPointF(10, 20));
		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, segmentMiddle(z.rv, 0), Qt::LeftButton, Qt::LeftButton, Qt::AltModifier | Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, segmentMiddle(z.rv, 0), Qt::LeftButton, Qt::NoButton, Qt::AltModifier | Qt::ShiftModifier);

		QVERIFY(std::isnan(z.rel->getLabelDistance(BaseRelationship::DstCardLabel).x()));
		QVERIFY(z.rel->getSegmentShifts().empty());
		QCOMPARE(spy.count(), 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theCursorShowsTheDirectionTheSegmentMoves()
{
	try
	{
		{
			Rig rig;
			ZLayout z = makeZ(rig);

			// The middle segment is vertical: it moves sideways
			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ShiftModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::SizeHorCursor);

			// Without SHIFT, over the end segments, or away from the line the cursor is the common one
			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::NoModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);

			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ShiftModifier);
			sendHover(z.rv, segmentMiddle(z.rv, 0), Qt::ShiftModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);

			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ShiftModifier);
			sendHover(z.rv, segmentMiddle(z.rv, 1) + QPointF(200, 200), Qt::ShiftModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);

			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ShiftModifier);
			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ControlModifier | Qt::ShiftModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);
		}

		{
			Rig rig;
			ZLayout z = makeVerticalZ(rig);

			// A horizontal segment moves up and down
			sendHover(z.rv, segmentMiddle(z.rv, 1), Qt::ShiftModifier);
			QCOMPARE(z.rv->cursor().shape(), Qt::SizeVerCursor);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theCursorIsRestoredWhenTheMouseLeavesAndAfterTheDrag()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);

		sendHover(z.rv, mid, Qt::ShiftModifier);
		QCOMPARE(z.rv->cursor().shape(), Qt::SizeHorCursor);

		QGraphicsSceneHoverEvent leave(QEvent::GraphicsSceneHoverLeave);
		z.rv->hoverLeaveEvent(&leave);
		QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);

		// During the drag the cursor stays, even if the mouse (or SHIFT) goes away from the segment
		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		QCOMPARE(z.rv->cursor().shape(), Qt::SizeHorCursor);
		sendHover(z.rv, mid + QPointF(300, 300), Qt::NoModifier);
		QCOMPARE(z.rv->cursor().shape(), Qt::SizeHorCursor);
		z.rv->hoverLeaveEvent(&leave);
		QCOMPARE(z.rv->cursor().shape(), Qt::SizeHorCursor);

		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid, Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		QCOMPARE(z.rv->cursor().shape(), Qt::ArrowCursor);
		settle(0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::draggingThroughTheViewportMovesTheSegment()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QGraphicsView *view = rig.wgt.getViewport();
		QSignalSpy spy(rig.scene, &ObjectsScene::s_objectModified);
		std::vector<QPointF> base = z.rv->route_points;

		rig.wgt.resize(1400, 900);
		rig.wgt.show();
		settle(50);

		QPointF mid = segmentMiddle(z.rv, 1);
		view->centerOn(mid);
		settle(50);

		auto send = [&](QEvent::Type type, const QPointF &scene_pos, Qt::MouseButton btn, Qt::MouseButtons btns) {
			QPoint pos = view->mapFromScene(scene_pos);
			QMouseEvent ev(type, QPointF(pos), QPointF(view->viewport()->mapToGlobal(pos)), btn, btns, Qt::ShiftModifier);
			QApplication::sendEvent(view->viewport(), &ev);
		};

		rig.scene->clearSelection();
		send(QEvent::MouseButtonPress, mid, Qt::LeftButton, Qt::LeftButton);
		QVERIFY(z.rv->isSelected());

		for(int i = 1; i <= 6; i++)
			send(QEvent::MouseMove, mid + QPointF(8 * i, 0), Qt::NoButton, Qt::LeftButton);

		send(QEvent::MouseButtonRelease, mid + QPointF(48, 0), Qt::LeftButton, Qt::NoButton);
		settle(100);

		QVERIFY(!rig.scene->isMovingObjects());
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QVERIFY(std::fabs(z.rel->getSegmentShifts().at(1).value - 48) < 1);
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 48)) < 1);
		QVERIFY(spy.count() >= 1);

		// The line was reconfigured with obstacles avoidance and the bumps are consistent
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::deletingTheRelationshipRightAfterTheDragIsSafe()
{
	LeakCheckPause leak_pause;

	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);

		// The reconfiguration after the release is queued: the line may be gone when it would run
		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseMove, mid + QPointF(30, 0), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid + QPointF(30, 0), Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);

		// The relationship leaves the model and the scene, but is only deleted later (the operation list keeps it for undoing)
		rig.db->removeRelationship(z.rel);
		settle(150);
		delete z.rel;
		settle(50);

		QVERIFY(rig.scene->getRelationshipViews().empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::draggingAfterATableMovedUsesTheNewRoute()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);

		dragSegment(z.rv, segmentMiddle(z.rv, 1), segmentMiddle(z.rv, 1) + QPointF(30, 0), Qt::ShiftModifier);
		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));

		QPointF pos = tableView(z.b)->pos();
		moveTable(rig, z.b, pos + QPointF(100, 80));
		QVERIFY(z.rel->getSegmentShifts().empty());

		std::vector<QPointF> base = z.rv->route_points;
		dragSegment(z.rv, segmentMiddle(z.rv, 1), segmentMiddle(z.rv, 1) + QPointF(-20, 0), Qt::ShiftModifier);

		QCOMPARE(z.rel->getSegmentShifts().size(), size_t(1));
		QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() - 20)) < Eps);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::moveTables(Rig &rig, const std::vector<Table *> &tables, const QPointF &delta, int wait_ms)
{
	rig.scene->clearSelection();

	for(auto &table : tables)
		tableView(table)->setSelected(true);

	/* The same sequence of the scene when the user moves the selected objects with the mouse: the placeholders
	 * (if used) are shown, the tables are moved and the scene finishes the movement */
	rig.scene->moving_objs = true;

	for(auto &table : tables)
		tableView(table)->setPos(tableView(table)->pos() + delta);

	rig.scene->finishObjectsMove(QPointF(0, 0));

	for(auto &table : tables)
		tableView(table)->setSelected(false);

	settle(wait_ms);
}

QString RelationshipLinesTest::checkAllRoutes(Rig &rig)
{
	for(auto &rv : rig.scene->getRelationshipViews())
	{
		if(rv->getUnderlyingObject()->isSelfRelationship() || !rv->isVisible())
			continue;

		QString err = checkRoute(rv);

		if(!err.isEmpty())
			return err;
	}

	return "";
}

void RelationshipLinesTest::theSceneKeepsTheListsOfTablesAndRelationshipsInSync()
{
	LeakCheckPause leak_pause;

	try
	{
		Rig rig;
		std::vector<Table *> tables;
		std::vector<Relationship *> rels;

		for(int i = 0; i < 6; i++)
			tables.push_back(addTable(rig, QString("tab_%1").arg(i), QPointF(i * 300.0, (i % 2) * 200.0)));

		for(int i = 1; i < 6; i++)
			rels.push_back(addRel(rig, tables[i - 1], tables[i]));

		auto check = [&](size_t tab_cnt, size_t rel_cnt) {
			std::set<QGraphicsItem *> tab_items, rel_items;

			for(auto &item : rig.scene->items())
			{
				if(dynamic_cast<TableView *>(item))
					tab_items.insert(item);
				else if(dynamic_cast<RelationshipView *>(item))
					rel_items.insert(item);
			}

			std::set<QGraphicsItem *> tab_list(rig.scene->getTableViews().begin(), rig.scene->getTableViews().end()),
					rel_list(rig.scene->getRelationshipViews().begin(), rig.scene->getRelationshipViews().end());

			QCOMPARE(tab_list, tab_items);
			QCOMPARE(rel_list, rel_items);
			QCOMPARE(tab_list.size(), tab_cnt);
			QCOMPARE(rel_list.size(), rel_cnt);

			// No duplicates
			QCOMPARE(rig.scene->getTableViews().size(), tab_cnt);
			QCOMPARE(rig.scene->getRelationshipViews().size(), rel_cnt);
		};

		check(6, 5);

		// Removing a relationship
		Relationship *removed_rel_1 = rels.back();
		rig.db->removeRelationship(removed_rel_1);
		rels.pop_back();
		settle(10);
		check(6, 4);

		// Removing a table with its relationship
		Relationship *removed_rel_2 = rels.back();
		Table *removed_table = tables.back();
		rig.db->removeRelationship(removed_rel_2);
		rels.pop_back();
		rig.db->removeObject(removed_table);
		tables.pop_back();
		settle(10);
		check(5, 3);

		QVERIFY(!rels.empty());

		delete removed_rel_1;
		delete removed_rel_2;
		delete removed_table;
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::addingAndRemovingManyViewsLeavesNoStaleEntries()
{
	LeakCheckPause leak_pause;

	try
	{
		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(600, 0));

		for(int round = 0; round < 25; round++)
		{
			Relationship *rel = addRel(rig, tab_a, tab_b, BaseRelationship::Relationship1n, QString("rel_%1").arg(round));

			QCOMPARE(rig.scene->getRelationshipViews().size(), size_t(1));
			QVERIFY(rig.scene->getRelationshipViews()[0] == relView(rel));

			rig.db->removeRelationship(rel);
			settle(0);
			QVERIFY(rig.scene->getRelationshipViews().empty());
			delete rel;
		}

		QCOMPARE(rig.scene->getTableViews().size(), size_t(2));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::removingEverythingWithAPendingBumpsUpdateLeavesTheScenesListsEmpty()
{
	LeakCheckPause leak_pause;

	try
	{
		Rig rig;
		std::vector<Table *> tables;
		std::vector<Relationship *> rels;

		for(int i = 0; i < 5; i++)
			tables.push_back(addTable(rig, QString("tab_%1").arg(i), QPointF(i * 300.0, 0)));

		for(int i = 1; i < 5; i++)
			rels.push_back(addRel(rig, tables[i - 1], tables[i]));

		settle(100);
		rig.scene->requestBumpsUpdate();

		// Everything is removed from the model with a bumps update pending
		for(auto &rel : rels)
			rig.db->removeRelationship(rel);

		for(auto &table : tables)
			rig.db->removeObject(table);

		settle(150);

		QVERIFY(rig.scene->getRelationshipViews().empty());
		QVERIFY(rig.scene->getTableViews().empty());

		for(auto &rel : rels)
			delete rel;

		for(auto &table : tables)
			delete table;
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::closingTheModelWithPendingUpdatesIsSafe()
{
	try
	{
		for(int variant = 0; variant < 3; variant++)
		{
			auto rig = std::make_unique<Rig>();

			if(variant < 2)
			{
				Table *tab_a = addTable(*rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(*rig, "tab_b", QPointF(900, 400)),
						*tab_c = addTable(*rig, "tab_c", QPointF(450, 200));

				QVERIFY(addRel(*rig, tab_a, tab_b) != nullptr);

				if(variant == 0)
					rig->scene->requestBumpsUpdate();
				else
					moveTable(*rig, tab_c, QPointF(500, 500));
			}
			else
			{
				ZLayout z = makeZ(*rig);
				QPointF mid = segmentMiddle(z.rv, 1);

				z.rv->setSelected(true);
				sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
				sendMouse(z.rv, QEvent::GraphicsSceneMouseMove, mid + QPointF(20, 0), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);
				sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid + QPointF(20, 0), Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
			}

			// The widget goes away with timers and queued events pending
			rig.reset();
			settle(150);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aLineClickedOnAnEmptyAreaIsFoundBySceneHitTesting()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);

		QPointF mid = segmentMiddle(z.rv, 1);
		QGraphicsItem *item = rig.scene->itemAt(mid, QTransform());

		QVERIFY(item != nullptr);

		// The line, or one of its parts (curve), is what's under the mouse
		QGraphicsItem *top = item;

		while(top->parentItem())
			top = top->parentItem();

		QVERIFY(top == z.rv);
		QVERIFY(z.rv->shape().contains(mid));
		QVERIFY(!z.rv->shape().contains(mid + QPointF(0, 2000)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theShapeFollowsThePreviewOfTheDrag()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QPointF mid = segmentMiddle(z.rv, 1);

		z.rv->setSelected(true);
		sendMouse(z.rv, QEvent::GraphicsSceneMousePress, mid, Qt::LeftButton, Qt::LeftButton, Qt::ShiftModifier);
		sendMouse(z.rv, QEvent::GraphicsSceneMouseMove, mid + QPointF(40, 0), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);

		// The line is clipped to its shape: the moved segment must be inside it, or it wouldn't be painted. A point
		// near the segment's end is used: the middle is where the label of the relationship usually is
		QPointF moved = z.rv->route_points[1] + (z.rv->route_points[2] - z.rv->route_points[1]) * 0.2 + QPointF(40, 0);

		QVERIFY(z.rv->shape().contains(moved));
		QVERIFY(z.rv->boundingRect().contains(moved));
		QVERIFY(!z.rv->shape().contains(moved - QPointF(40, 0)));

		sendMouse(z.rv, QEvent::GraphicsSceneMouseRelease, mid + QPointF(40, 0), Qt::LeftButton, Qt::NoButton, Qt::ShiftModifier);
		settle(0);
		QVERIFY(z.rv->shape().contains(moved));
		QVERIFY(!z.rv->shape().contains(moved - QPointF(40, 0)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::nearbyRoutesDoNotIncludeLinesThatOnlyHaveTheTableInsideTheirBox()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(5000, 5000));
		Relationship *rel = addRel(rig, tab_a, tab_b);
		RelationshipView *rv = relView(rel);
		QRectF rect_a = tableRect(tableView(tab_a));

		moveTable(rig, tab_b, QPointF(rect_a.right() + 3000, rect_a.bottom() + 3000));
		QCOMPARE(rv->route_points.size(), size_t(4));

		// A table in the upper right corner of the line's box: the box has it, the line doesn't pass near it
		Table *tab_c = addTable(rig, "tab_c", QPointF(rect_a.right() + 2800, 500));
		TableView *view_c = tableView(tab_c);

		QVERIFY(rv->sceneBoundingRect().intersects(tableRect(view_c)));

		auto contains = [&](const std::vector<RelationshipView *> &list) { return std::find(list.begin(), list.end(), rv) != list.end(); };

		QVERIFY(!contains(view_c->getNearbyRoutes(true)));
		QVERIFY(!contains(view_c->getNearbyRoutes(false)));

		// Over the vertical segment of the line
		view_c->setPos(rv->route_points[1].x() - 20, (rv->route_points[1].y() + rv->route_points[2].y()) / 2);
		QVERIFY(contains(view_c->getNearbyRoutes(true)));

		// Over the horizontal segment
		view_c->setPos((rv->route_points[0].x() + rv->route_points[1].x()) / 2, rv->route_points[0].y() - 10);
		QVERIFY(contains(view_c->getNearbyRoutes(true)));

		// Within the margin of 20 units around the table
		view_c->setPos(rv->route_points[1].x() + 15, (rv->route_points[1].y() + rv->route_points[2].y()) / 2);
		QVERIFY(contains(view_c->getNearbyRoutes(true)));

		view_c->setPos(rv->route_points[1].x() + 25, (rv->route_points[1].y() + rv->route_points[2].y()) / 2);
		QVERIFY(!contains(view_c->getNearbyRoutes(true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::nearbyRoutesExcludeTheConnectedOnesUnlessAsked()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(800, 0));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		auto contains = [&](const std::vector<RelationshipView *> &list) { return std::find(list.begin(), list.end(), rv) != list.end(); };

		QVERIFY(contains(tableView(tab_a)->getNearbyRoutes(true)));
		QVERIFY(!contains(tableView(tab_a)->getNearbyRoutes(false)));
		QVERIFY(contains(tableView(tab_b)->getNearbyRoutes(true)));
		QVERIFY(!contains(tableView(tab_b)->getNearbyRoutes(false)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::nearbyRoutesLookAtTheAreaTheTableLeft()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, 0));
		RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));
		TableView *view_c = tableView(tab_c);
		auto contains = [&](const std::vector<RelationshipView *> &list) { return std::find(list.begin(), list.end(), rv) != list.end(); };

		// The line goes around c: moving c far away, the line only touches the area c occupied before
		QVERIFY(rv->route_points.size() > 2);

		rig.scene->moving_objs = true;
		view_c->setPos(QPointF(700, 4000));

		QVERIFY(!view_c->move_origin_rect.isNull());
		QVERIFY(contains(view_c->getNearbyRoutes(false)));

		// The origin is consumed by the first query
		QVERIFY(view_c->move_origin_rect.isNull());
		QVERIFY(!contains(view_c->getNearbyRoutes(false)));
		rig.scene->moving_objs = false;
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theOriginOfAMovementIsOnlyRegisteredWhileObjectsAreMoving()
{
	try
	{
		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0));
		TableView *view_a = tableView(tab_a);

		view_a->setPos(QPointF(100, 100));
		QVERIFY(view_a->move_origin_rect.isNull());

		rig.scene->moving_objs = true;
		view_a->setPos(QPointF(200, 200));
		QVERIFY(!view_a->move_origin_rect.isNull());
		QCOMPARE(view_a->move_origin_rect.topLeft(), QPointF(100, 100));

		// Moving again keeps the first origin
		view_a->setPos(QPointF(300, 300));
		QCOMPARE(view_a->move_origin_rect.topLeft(), QPointF(100, 100));
		rig.scene->moving_objs = false;
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::nearbyRoutesAreEmptyInTheOtherLineStyles()
{
	try
	{
		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(800, 0));

		addRel(rig, tab_a, tab_b);

		for(auto style : { RelationshipView::StraightLines, RelationshipView::CurvedLines })
		{
			RelationshipView::setLineStyle(style);
			QVERIFY(tableView(tab_a)->getNearbyRoutes(true).empty());
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::aRouteIsNearAnAreaWhenItsSegmentsOrItsDirectRouteTouchIt()
{
	try
	{
		Rig rig;
		std::vector<RelationshipView *> views;

		prepareBumpsRig(rig, 1, views);

		RelationshipView *rv = views[0];

		rv->route_points = { QPointF(0, 0), QPointF(200, 0), QPointF(200, 100) };
		rv->direct_route = { QPointF(0, 0), QPointF(0, 100), QPointF(200, 100) };

		QVERIFY(rv->isRouteNear(QRectF(90, -10, 20, 20)));    // over the first segment of the route
		QVERIFY(rv->isRouteNear(QRectF(190, 40, 20, 20)));    // over its second segment
		QVERIFY(rv->isRouteNear(QRectF(-10, 40, 20, 20)));    // over the direct route only
		QVERIFY(!rv->isRouteNear(QRectF(80, 40, 40, 40)));    // inside the box but over neither of them
		QVERIFY(!rv->isRouteNear(QRectF(1000, 1000, 20, 20)));

		// Without any route the line is assumed to be near any area
		rv->route_points.clear();
		rv->direct_route.clear();
		QVERIFY(rv->isRouteNear(QRectF(1000, 1000, 20, 20)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::movingTheObstacleAwayWorksWithAndWithoutPlaceholders()
{
	try
	{
		for(int placeholders = 0; placeholders < 2; placeholders++)
		{
			BaseObjectView::setPlaceholderEnabled(placeholders);
			applyMode(RelationshipView::ConnectTableEdges, false);

			Rig rig;
			Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1400, 0)),
					*tab_c = addTable(rig, "tab_c", QPointF(700, 0));
			RelationshipView *rv = relView(addRel(rig, tab_a, tab_b));

			QVERIFY2(rv->route_points.size() > 2, qPrintable(QString("placeholders %1").arg(placeholders)));

			moveTables(rig, { tab_c }, QPointF(0, 1500));
			QVERIFY2(rv->route_points.size() == 2, qPrintable(QString("placeholders %1: %2 points").arg(placeholders).arg(rv->route_points.size())));

			moveTables(rig, { tab_c }, QPointF(0, -1500));
			QVERIFY2(rv->route_points.size() > 2, qPrintable(QString("placeholders %1").arg(placeholders)));
			QVERIFY(!routeCrossesRect(rv->route_points, tableRect(tableView(tab_c))));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::movingManyTablesTogetherLeavesEveryRouteValid_data()
{
	QTest::addColumn<bool>("placeholders");
	QTest::addColumn<bool>("crows_foot");
	QTest::addColumn<int>("moved");

	for(int placeholders = 0; placeholders < 2; placeholders++)
	{
		for(int crows = 0; crows < 2; crows++)
		{
			for(int moved : { 1, 5, 12 })
				QTest::newRow(qPrintable(QString("%1_%2_%3tables").arg(placeholders ? "placeholder" : "direct").arg(crows ? "crow" : "plain").arg(moved)))
						<< bool(placeholders) << bool(crows) << moved;
		}
	}
}

void RelationshipLinesTest::movingManyTablesTogetherLeavesEveryRouteValid()
{
	try
	{
		QFETCH(bool, placeholders);
		QFETCH(bool, crows_foot);
		QFETCH(int, moved);

		BaseObjectView::setPlaceholderEnabled(placeholders);
		applyMode(RelationshipView::ConnectTableEdges, crows_foot);

		Rig rig;
		std::vector<Table *> tables, selected;
		QRandomGenerator rnd(77);

		// A 4x4 grid of tables with a relationship between neighbors and a few long ones
		for(int i = 0; i < 16; i++)
			tables.push_back(addTable(rig, QString("tab_%1").arg(i), QPointF((i % 4) * 520.0, (i / 4) * 380.0)));

		for(int i = 0; i < 16; i++)
		{
			if(i % 4 != 3)
				addRel(rig, tables[i], tables[i + 1]);

			if(i / 4 != 3)
				addRel(rig, tables[i], tables[i + 4]);
		}

		for(int i = 0; i < 4; i++)
			addRel(rig, tables[rnd.bounded(16)], tables[rnd.bounded(16)], BaseRelationship::Relationship1n, QString("rel_long_%1").arg(i));

		settle(100);
		QVERIFY2(checkAllRoutes(rig).isEmpty(), qPrintable(checkAllRoutes(rig)));

		for(int i = 0; i < moved; i++)
			selected.push_back(tables[(i * 5) % 16]);

		moveTables(rig, selected, QPointF(260, 140), 150);
		QVERIFY2(checkAllRoutes(rig).isEmpty(), qPrintable(checkAllRoutes(rig)));

		// None of the lines goes through a table that isn't one of its own (all of them have room around)
		for(auto &rv : rig.scene->getRelationshipViews())
		{
			if(rv->getUnderlyingObject()->isSelfRelationship())
				continue;

			QVERIFY(rv->route_points.size() >= 2);
		}

		moveTables(rig, selected, QPointF(-260, -140), 150);
		QVERIFY2(checkAllRoutes(rig).isEmpty(), qPrintable(checkAllRoutes(rig)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theStyleCanBeChangedBackAndForthWithoutLeavingTraces()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig, true);

		rig.scene->bumps_timer.stop();
		std::vector<QPointF> base = z.rv->route_points;
		z.rel->setSegmentShifts({{ 1, { Qt::Vertical, 20 } }});

		for(int round = 0; round < 3; round++)
		{
			RelationshipView::setLineStyle(RelationshipView::StraightLines);
			z.rv->invalidateRoute();
			QVERIFY(z.rv->route_points.empty());
			QVERIFY(z.rv->bumps.empty());
			QVERIFY(z.rv->curves.empty());
			QVERIFY(!z.rv->lines.empty() && z.rv->lines[0]->isVisible());

			RelationshipView::setLineStyle(RelationshipView::CurvedLines);
			z.rv->invalidateRoute();
			QVERIFY(z.rv->route_points.empty());
			QVERIFY(z.rv->bumps.empty());
			QVERIFY(!z.rv->curves.empty());

			RelationshipView::setLineStyle(RelationshipView::OrthogonalLines);
			z.rv->invalidateRoute();
			QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
			QCOMPARE(z.rv->curves.size(), size_t(1));
			QVERIFY(!z.rv->lines[0]->isVisible());
			QVERIFY(std::fabs(z.rv->route_points[1].x() - (base[1].x() + 20)) < Eps);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::togglingTheCrowsFootChangesTheAreasAndTheAnchors()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig, false);

		QVERIFY(z.rv->end_zones.empty());

		RelationshipView::setCrowsFoot(true);
		z.rv->invalidateRoute();
		QCOMPARE(z.rv->end_zones.size(), size_t(2));
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));

		RelationshipView::setCrowsFoot(false);
		z.rv->invalidateRoute();
		QVERIFY(z.rv->end_zones.empty());
		QVERIFY2(checkRoute(z.rv, true).isEmpty(), qPrintable(checkRoute(z.rv, true)));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::changingTheConnectionModeKeepsTheLinesValid()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig, false);
		Table *tab_c = addTable(rig, "tab_c", QPointF(300, 700));

		addRel(rig, tab_c, z.b);

		for(int round = 0; round < 3; round++)
		{
			for(int mode : { 0, 1, 2, 1, 0 })
			{
				applyMode(mode, false);

				for(auto &rv : rig.scene->getRelationshipViews())
				{
					rv->invalidateRoute();
					QVERIFY2(checkRoute(rv).isEmpty(), qPrintable(QString("mode %1: %2").arg(mode).arg(checkRoute(rv))));
				}
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::hiddenRelationshipsDoNotDisturbTheOthers()
{
	try
	{
		applyMode(RelationshipView::ConnectTableEdges, false);

		Rig rig;
		Table *tab_a = addTable(rig, "tab_a", QPointF(0, 0)), *tab_b = addTable(rig, "tab_b", QPointF(1600, 0)),
				*tab_c = addTable(rig, "tab_c", QPointF(700, -700)), *tab_d = addTable(rig, "tab_d", QPointF(700, 700));
		RelationshipView *rv_h = relView(addRel(rig, tab_a, tab_b)), *rv_v = relView(addRel(rig, tab_c, tab_d));

		settle(200);
		QCOMPARE(rv_h->bumps.size(), size_t(1));

		rv_v->setVisible(false);
		rig.scene->requestBumpsUpdate();
		settle(150);
		QVERIFY(rv_h->bumps.empty());

		rv_v->setVisible(true);
		rig.scene->requestBumpsUpdate();
		settle(150);
		QCOMPARE(rv_h->bumps.size(), size_t(1));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

// Route computations: the static helpers are exercised directly with random inputs (seeded)

std::vector<QRectF> RelationshipLinesTest::randomObstacles(QRandomGenerator &rnd, int count, double area, const std::vector<QRectF> &avoid)
{
	std::vector<QRectF> rects;
	int attempts = 0;

	while(static_cast<int>(rects.size()) < count && attempts++ < count * 50)
	{
		QRectF rect(rnd.bounded(int(area)), rnd.bounded(int(area)), 40 + rnd.bounded(220), 30 + rnd.bounded(160));
		bool ok = true;

		// Non-overlapping, with some room between them, and away from the points that must stay free
		for(auto &other : rects)
			ok = ok && !rect.adjusted(-6, -6, 6, 6).intersects(other);

		for(auto &free_rect : avoid)
			ok = ok && !rect.adjusted(-3, -3, 3, 3).intersects(free_rect);

		if(ok)
			rects.push_back(rect);
	}

	return rects;
}

double RelationshipLinesTest::pathCost(const std::vector<QPointF> &path, int goal_dir, int start_dir)
{
	constexpr double BendPenalty = 50;
	double cost = 0;
	int last_dir = start_dir;

	for(size_t i = 1; i < path.size(); i++)
	{
		int dir = -1;
		double dx = path[i].x() - path[i - 1].x(), dy = path[i].y() - path[i - 1].y();

		if(std::fabs(dy) < Eps && dx > 0) dir = 0;
		else if(std::fabs(dx) < Eps && dy > 0) dir = 1;
		else if(std::fabs(dy) < Eps && dx < 0) dir = 2;
		else if(std::fabs(dx) < Eps && dy < 0) dir = 3;
		else return -1;

		cost += std::fabs(dx) + std::fabs(dy);

		if(last_dir >= 0 && dir != last_dir)
			cost += BendPenalty;

		last_dir = dir;
	}

	if(goal_dir >= 0 && last_dir != goal_dir)
		cost += BendPenalty;

	return cost;
}

double RelationshipLinesTest::optimalPathCost(const QPointF &start, const QPointF &goal, int start_dir, int goal_dir,
																							const std::vector<QRectF> &obstacles)
{
	// An independent Dijkstra over the same grid (Hanan) the search uses, written without the optimizations
	constexpr double BendPenalty = 50;
	std::vector<double> xs { start.x(), goal.x() }, ys { start.y(), goal.y() };

	for(auto &rect : obstacles)
	{
		xs.push_back(rect.left());
		xs.push_back(rect.right());
		ys.push_back(rect.top());
		ys.push_back(rect.bottom());
	}

	auto uniq = [](std::vector<double> &v) {
		std::sort(v.begin(), v.end());
		v.erase(std::unique(v.begin(), v.end(), [](double a, double b){ return std::fabs(a - b) < 0.01; }), v.end());
	};

	uniq(xs);
	uniq(ys);

	int nx = xs.size(), ny = ys.size();
	auto idx_of = [](const std::vector<double> &v, double value) {
		return int(std::lower_bound(v.begin(), v.end(), value - 0.01) - v.begin());
	};

	int sx = idx_of(xs, start.x()), sy = idx_of(ys, start.y()), gx = idx_of(xs, goal.x()), gy = idx_of(ys, goal.y());
	const int dx[4] = { 1, 0, -1, 0 }, dy[4] = { 0, 1, 0, -1 };
	std::vector<double> dist(nx * ny * 4, std::numeric_limits<double>::infinity());
	using Item = std::pair<double, int>;
	std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;

	auto push = [&](int ix, int iy, int dir, double cost) {
		int state = ((iy * nx) + ix) * 4 + dir;

		if(cost < dist[state])
		{
			dist[state] = cost;
			queue.push({ cost, state });
		}
	};

	if(start_dir >= 0)
		push(sx, sy, start_dir, 0);
	else
	{
		for(int d = 0; d < 4; d++)
			push(sx, sy, d, 0);
	}

	double best = std::numeric_limits<double>::infinity();

	while(!queue.empty())
	{
		Item item = queue.top();
		queue.pop();

		if(item.first > dist[item.second])
			continue;

		int dir = item.second % 4, node = item.second / 4, ix = node % nx, iy = node / nx;

		if(ix == gx && iy == gy)
			best = std::min(best, item.first + ((goal_dir >= 0 && dir != goal_dir) ? BendPenalty : 0));

		for(int nd = 0; nd < 4; nd++)
		{
			int new_x = ix + dx[nd], new_y = iy + dy[nd];

			if(nd == (dir + 2) % 4 || new_x < 0 || new_x >= nx || new_y < 0 || new_y >= ny)
				continue;

			QPointF p1(xs[ix], ys[iy]), p2(xs[new_x], ys[new_y]);

			if(RelationshipView::isSegmentBlocked(p1, p2, obstacles))
				continue;

			double cost = item.first + std::fabs(p2.x() - p1.x()) + std::fabs(p2.y() - p1.y()) + (nd != dir ? BendPenalty : 0);

			push(new_x, new_y, nd, cost);
		}
	}

	return best;
}

void RelationshipLinesTest::theOrthogonalSearchFindsTheCheapestPath_data()
{
	QTest::addColumn<int>("seed");

	for(int seed = 1; seed <= 12; seed++)
		QTest::newRow(qPrintable(QString("seed_%1").arg(seed))) << seed;
}

void RelationshipLinesTest::theOrthogonalSearchFindsTheCheapestPath()
{
	try
	{
		QFETCH(int, seed);

		QRandomGenerator rnd(seed);
		int checked = 0;

		for(int round = 0; round < 60; round++)
		{
			QPointF start(rnd.bounded(1400), rnd.bounded(1400)), goal(rnd.bounded(1400), rnd.bounded(1400));
			std::vector<QRectF> free_areas = { QRectF(start - QPointF(8, 8), QSizeF(16, 16)), QRectF(goal - QPointF(8, 8), QSizeF(16, 16)) };
			std::vector<QRectF> obstacles = randomObstacles(rnd, 3 + rnd.bounded(22), 1500, free_areas);
			int start_dir = int(rnd.bounded(5)) - 1, goal_dir = int(rnd.bounded(5)) - 1;

			std::vector<QPointF> path = RelationshipView::findOrthogonalPath(start, goal, start_dir, goal_dir, obstacles);
			double best = optimalPathCost(start, goal, start_dir, goal_dir, obstacles);

			if(path.empty())
			{
				// No way for the search only if there's really none (or the grid is above the limit, which these aren't)
				QVERIFY2(std::isinf(best), qPrintable(QString("round %1: no path found but a path with cost %2 exists").arg(round).arg(best)));
				continue;
			}

			checked++;
			QVERIFY2(!std::isinf(best), qPrintable(QString("round %1: a path was found where there's none").arg(round)));
			QCOMPARE(path.front(), start);
			QVERIFY(QLineF(path.back(), goal).length() < 0.02);

			for(size_t i = 1; i < path.size(); i++)
			{
				QVERIFY2(isHorizontal(path[i - 1], path[i]) || std::fabs(path[i - 1].x() - path[i].x()) < Eps, qPrintable(QString("round %1: diagonal segment").arg(round)));
				QVERIFY2(!RelationshipView::isSegmentBlocked(path[i - 1], path[i], obstacles), qPrintable(QString("round %1: blocked segment").arg(round)));
			}

			double cost = pathCost(path, goal_dir, start_dir);

			QVERIFY2(cost >= 0 && std::fabs(cost - best) < 1e-6, qPrintable(QString("round %1: cost %2, the best is %3").arg(round).arg(cost).arg(best)));
		}

		QVERIFY(checked > 0);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theSearchGivesUpWhenTheGridIsTooBigOrTheEndsAreClosed()
{
	try
	{
		// A start inside a closed ring (the pieces overlap, there are no seams to slip through): no way out
		std::vector<QRectF> ring = { QRectF(0, 0, 100, 25), QRectF(0, 75, 100, 25), QRectF(0, 0, 25, 100), QRectF(75, 0, 25, 100) };

		QVERIFY(RelationshipView::findOrthogonalPath(QPointF(50, 50), QPointF(500, 500), -1, -1, ring).empty());
		QVERIFY(!RelationshipView::findOrthogonalPath(QPointF(50, 150), QPointF(500, 500), -1, -1, ring).empty());

		// Same point: the path has just that point
		std::vector<QPointF> same = RelationshipView::findOrthogonalPath(QPointF(10, 10), QPointF(10, 10), -1, -1, {});
		QVERIFY(same.size() <= 1 || QLineF(same.front(), same.back()).length() < 0.02);

		// Without obstacles: a straight line, or a single bend
		std::vector<QPointF> straight = RelationshipView::findOrthogonalPath(QPointF(0, 0), QPointF(300, 0), -1, -1, {});
		QVERIFY(straight.size() == 2 && isHorizontal(straight[0], straight[1]));

		std::vector<QPointF> bend = RelationshipView::findOrthogonalPath(QPointF(0, 0), QPointF(300, 200), -1, -1, {});
		QCOMPARE(bend.size(), size_t(3));

		// Too many obstacles for the grid (nx * ny over the limit of 40000)
		std::vector<QRectF> many;

		for(int i = 0; i < 120; i++)
			many.push_back(QRectF(i * 30.0, i * 30.0, 10, 10));

		QVERIFY(RelationshipView::findOrthogonalPath(QPointF(-100, -100), QPointF(5000, 5000), -1, -1, many).empty());
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::segmentsOnlyTouchingAnObstacleAreNotBlocked()
{
	try
	{
		std::vector<QRectF> obstacles = { QRectF(100, 100, 100, 100) };

		// Along the borders
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(0, 100), QPointF(300, 100), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(0, 200), QPointF(300, 200), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(100, 0), QPointF(100, 300), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(200, 0), QPointF(200, 300), obstacles));

		// Ending on the border
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(0, 150), QPointF(100, 150), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(200, 150), QPointF(400, 150), obstacles));

		// Through the inside, or ending inside
		QVERIFY(RelationshipView::isSegmentBlocked(QPointF(0, 150), QPointF(300, 150), obstacles));
		QVERIFY(RelationshipView::isSegmentBlocked(QPointF(150, 0), QPointF(150, 300), obstacles));
		QVERIFY(RelationshipView::isSegmentBlocked(QPointF(0, 150), QPointF(101, 150), obstacles));
		QVERIFY(RelationshipView::isSegmentBlocked(QPointF(150, 150), QPointF(150, 150.5), obstacles));

		// Not touching
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(0, 50), QPointF(300, 50), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(300, 0), QPointF(300, 300), obstacles));

		// Several obstacles
		obstacles.push_back(QRectF(400, 100, 50, 50));
		QVERIFY(RelationshipView::isSegmentBlocked(QPointF(350, 120), QPointF(500, 120), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(350, 90), QPointF(500, 90), obstacles));
		QVERIFY(!RelationshipView::isSegmentBlocked(QPointF(0, 0), QPointF(10, 0), {}));

		// A route is blocked when any of its segments is
		QVERIFY(RelationshipView::isRouteBlocked({ QPointF(0, 50), QPointF(300, 50), QPointF(300, 120), QPointF(500, 120) }, obstacles));
		QVERIFY(!RelationshipView::isRouteBlocked({ QPointF(0, 50), QPointF(300, 50), QPointF(300, 90), QPointF(500, 90) }, obstacles));
		QVERIFY(!RelationshipView::isRouteBlocked({}, obstacles));
		QVERIFY(!RelationshipView::isRouteBlocked({ QPointF(150, 150) }, obstacles));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::cleanRouteRemovesDuplicatedAndCollinearPoints()
{
	try
	{
		QRandomGenerator rnd(404);

		for(int round = 0; round < 200; round++)
		{
			std::vector<QPointF> route;
			QPointF cur(rnd.bounded(500), rnd.bounded(500));
			bool horiz = rnd.bounded(2);

			route.push_back(cur);

			for(int i = 0, segs = 1 + rnd.bounded(8); i < segs; i++)
			{
				// Sometimes no movement (duplicated point) or the same direction twice (collinear)
				int kind = rnd.bounded(4);
				double len = (kind == 0 ? 0 : 20 + rnd.bounded(200)) * (rnd.bounded(2) ? 1 : -1);

				if(horiz)
					cur.setX(cur.x() + len);
				else
					cur.setY(cur.y() + len);

				route.push_back(cur);

				if(kind != 1)
					horiz = !horiz;
			}

			std::vector<QPointF> clean = RelationshipView::cleanRoute(route);

			QVERIFY(clean.size() >= 2 || route.size() < 2 || QLineF(route.front(), route.back()).length() < 0.01);
			QCOMPARE(clean.front(), route.front());

			if(clean.size() >= 2)
				QCOMPARE(clean.back(), route.back());

			// (a route that comes back to where it started collapses to a single place: the result keeps two equal points)
			bool collapsed = (clean.size() == 2 && QLineF(clean[0], clean[1]).length() < 0.01);

			for(size_t i = 1; i < clean.size() && !collapsed; i++)
			{
				QVERIFY2(QLineF(clean[i - 1], clean[i]).length() >= 0.01, "duplicated point");

				if(i >= 2)
				{
					double cross = (clean[i - 1].x() - clean[i - 2].x()) * (clean[i].y() - clean[i - 1].y()) -
												 (clean[i - 1].y() - clean[i - 2].y()) * (clean[i].x() - clean[i - 1].x());
					QVERIFY2(std::fabs(cross) >= 0.01, "collinear points");
				}
			}

			// The same polyline: cleaning twice changes nothing, and the covered length is the same
			QCOMPARE(RelationshipView::cleanRoute(clean), clean);

			auto length = [](const std::vector<QPointF> &r) {
				double total = 0;

				for(size_t i = 1; i < r.size(); i++)
					total += QLineF(r[i - 1], r[i]).length();

				return total;
			};

			// (a collinear point where the line goes back over itself is also removed, so this is only a lower bound)
			QVERIFY(length(clean) <= length(route) + Eps);
		}

		QVERIFY(RelationshipView::cleanRoute({}).empty());
		// A route always has at least two points (a single point is repeated)
		QCOMPARE(RelationshipView::cleanRoute({ QPointF(1, 1) }).size(), size_t(2));
		QCOMPARE(RelationshipView::cleanRoute({ QPointF(1, 1), QPointF(1, 1) }).size(), size_t(2));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theRectOfARouteIsMadeOfItsExtremes()
{
	try
	{
		QRandomGenerator rnd(808);

		QVERIFY(RelationshipView::getRouteRect({}).isNull());

		// A single point (and a line on an axis) have no size but are still places
		QCOMPARE(RelationshipView::getRouteRect({ QPointF(5, 7) }).topLeft(), QPointF(5, 7));
		QCOMPARE(RelationshipView::getRouteRect({ QPointF(5, 7) }).bottomRight(), QPointF(5, 7));
		QCOMPARE(RelationshipView::getRouteRect({ QPointF(0, 7), QPointF(100, 7) }), QRectF(QPointF(0, 7), QPointF(100, 7)));

		for(int round = 0; round < 100; round++)
		{
			std::vector<QPointF> route;
			double left = 1e9, top = 1e9, right = -1e9, bottom = -1e9;

			for(int i = 0, n = 1 + rnd.bounded(10); i < n; i++)
			{
				QPointF pnt(rnd.bounded(2000) - 1000, rnd.bounded(2000) - 1000);

				route.push_back(pnt);
				left = std::min(left, pnt.x());
				right = std::max(right, pnt.x());
				top = std::min(top, pnt.y());
				bottom = std::max(bottom, pnt.y());
			}

			QRectF rect = RelationshipView::getRouteRect(route);

			QCOMPARE(rect.left(), left);
			QCOMPARE(rect.right(), right);
			QCOMPARE(rect.top(), top);
			QCOMPARE(rect.bottom(), bottom);
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theSimpleRouteIsAlwaysOrthogonalAndRespectsTheEnds()
{
	try
	{
		QRandomGenerator rnd(909);
		const QPointF dirs[5] = { QPointF(1, 0), QPointF(-1, 0), QPointF(0, 1), QPointF(0, -1), QPointF(0, 0) };

		for(int round = 0; round < 600; round++)
		{
			QPointF p0(rnd.bounded(1000), rnd.bounded(1000)), p1(rnd.bounded(1000), rnd.bounded(1000));
			QPointF dir0 = dirs[rnd.bounded(5)], dir1 = dirs[rnd.bounded(5)];
			double stub0 = (rnd.bounded(3) == 0 ? 0 : rnd.bounded(60)), stub1 = (rnd.bounded(3) == 0 ? 0 : rnd.bounded(60)),
					mid_shift = (rnd.bounded(3) == 0 ? 0 : rnd.bounded(60) - 30);

			std::vector<QPointF> route = RelationshipView::getOrthogonalRoute(p0, dir0, stub0, p1, dir1, stub1, mid_shift);
			QString what = QString("round %1: (%2,%3) -> (%4,%5) dirs (%6,%7)/(%8,%9) stubs %10/%11 shift %12").arg(round).arg(p0.x()).arg(p0.y())
										 .arg(p1.x()).arg(p1.y()).arg(dir0.x()).arg(dir0.y()).arg(dir1.x()).arg(dir1.y()).arg(stub0).arg(stub1).arg(mid_shift);

			QVERIFY2(route.size() >= 1, qPrintable(what));
			QCOMPARE(route.front(), p0);

			if(route.size() >= 2)
				QCOMPARE(route.back(), p1);
			else
				QVERIFY2(QLineF(p0, p1).length() < 0.01, qPrintable(what));

			for(size_t i = 1; i < route.size(); i++)
			{
				QVERIFY2(isHorizontal(route[i - 1], route[i]) || std::fabs(route[i - 1].x() - route[i].x()) < Eps, qPrintable(what));
				QVERIFY2(QLineF(route[i - 1], route[i]).length() >= 0.01, qPrintable(what));

				if(i >= 2)
					QVERIFY2(isHorizontal(route[i - 1], route[i]) != isHorizontal(route[i - 2], route[i - 1]), qPrintable(what));
			}

			QVERIFY2(route.size() <= 4, qPrintable(what));
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::theSimpleRouteKeepsTheRoomForTheStubsWhenThereIsSpace()
{
	try
	{
		// Opposite horizontal directions with room: the middle segment keeps the stubs at both ends
		auto route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(1, 0), 30, QPointF(300, 100), QPointF(-1, 0), 30, 0);

		QCOMPARE(route.size(), size_t(4));
		QVERIFY(route[1].x() >= 30 - Eps && route[1].x() <= 270 + Eps);

		// Even with a displacement larger than the room
		for(double shift : { -1000.0, -100.0, 100.0, 1000.0 })
		{
			route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(1, 0), 30, QPointF(300, 100), QPointF(-1, 0), 30, shift);
			QVERIFY2(route[1].x() >= 30 - Eps && route[1].x() <= 270 + Eps, qPrintable(QString("shift %1: %2").arg(shift).arg(route[1].x())));
		}

		// Same side: the line goes around the points
		route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(1, 0), 30, QPointF(300, 100), QPointF(1, 0), 30, 0);
		QCOMPARE(route.size(), size_t(4));
		QVERIFY(route[1].x() >= 300 + 30 - Eps);

		// The vertical directions are the transposed case
		route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(0, 1), 30, QPointF(100, 300), QPointF(0, -1), 30, 0);
		QCOMPARE(route.size(), size_t(4));
		QVERIFY(route[1].y() >= 30 - Eps && route[1].y() <= 270 + Eps);
		QVERIFY(isHorizontal(route[1], route[2]));

		// Aligned points: a straight line
		route = RelationshipView::getOrthogonalRoute(QPointF(0, 50), QPointF(1, 0), 0, QPointF(300, 50), QPointF(-1, 0), 0, 0);
		QCOMPARE(route.size(), size_t(2));

		// Unknown directions: the axis of the greatest distance
		route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(0, 0), 0, QPointF(300, 100), QPointF(0, 0), 0, 0);
		QCOMPARE(route.size(), size_t(4));
		QVERIFY(isHorizontal(route[0], route[1]));

		route = RelationshipView::getOrthogonalRoute(QPointF(0, 0), QPointF(0, 0), 0, QPointF(100, 300), QPointF(0, 0), 0, 0);
		QVERIFY(!isHorizontal(route[0], route[1]));
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

void RelationshipLinesTest::appliedShiftsAlwaysGiveAValidRoute()
{
	try
	{
		Rig rig;
		ZLayout z = makeZ(rig);
		QRandomGenerator rnd(1212);
		const QPointF dirs[4] = { QPointF(1, 0), QPointF(-1, 0), QPointF(0, 1), QPointF(0, -1) };

		for(int round = 0; round < 400; round++)
		{
			QPointF p0(rnd.bounded(1000), rnd.bounded(1000)), p1(rnd.bounded(1000), rnd.bounded(1000));
			QPointF dir0 = dirs[rnd.bounded(4)], dir1 = dirs[rnd.bounded(4)];
			double stubs[2] = { double(rnd.bounded(50)), double(rnd.bounded(50)) };
			std::vector<QPointF> route = RelationshipView::getOrthogonalRoute(p0, dir0, stubs[0], p1, dir1, stubs[1], 0);
			std::map<unsigned, BaseRelationship::SegmentShift> shifts;

			if(route.size() < 4)
				continue;

			// Random shifts over every segment (including the invalid ones), with any value
			for(unsigned idx = 0; idx < route.size() + 2; idx++)
			{
				if(rnd.bounded(2))
					continue;

				bool horiz = (idx + 1 < route.size()) && isHorizontal(route[idx], route[idx + 1]);
				double value = (rnd.bounded(4) == 0 ? 1e7 : rnd.bounded(600)) * (rnd.bounded(2) ? 1 : -1);

				shifts[idx] = { (rnd.bounded(5) == 0 ? (horiz ? Qt::Vertical : Qt::Horizontal) : (horiz ? Qt::Horizontal : Qt::Vertical)), value };
			}

			z.rv->base_route = route;

			std::vector<QPointF> result = z.rv->applySegmentShifts(route, stubs, &shifts);
			QString what = QString("round %1").arg(round);

			QCOMPARE(result.size(), route.size());
			QCOMPARE(result.front(), route.front());
			QCOMPARE(result.back(), route.back());

			for(size_t i = 1; i < result.size(); i++)
			{
				bool horiz = isHorizontal(result[i - 1], result[i]), vert = std::fabs(result[i - 1].x() - result[i].x()) < Eps;

				QVERIFY2(horiz != vert, qPrintable(QString("%1: segment %2 is not orthogonal or has no length").arg(what).arg(i - 1)));

				// The orientation of every segment is kept (shifts never turn a segment)
				QCOMPARE(horiz, isHorizontal(route[i - 1], route[i]));
			}

			// The segments don't shrink below the length they need: the first and last ones also keep the room of the stubs
			double min_len = RelationshipView::RoundedCornerRadius;

			for(size_t i = 1; i < result.size(); i++)
			{
				double min = min_len;

				if(i == 1)
					min = std::max(min, stubs[0]);

				if(i + 1 == result.size())
					min = std::max(min, stubs[1]);

				double before = QLineF(route[i - 1], route[i]).length(), len = QLineF(result[i - 1], result[i]).length();

				// A segment that was already shorter than the minimum can't get shorter than it was
				QVERIFY2(len >= std::min(min, before) - 0.5, qPrintable(QString("%1: segment %2 went from %3 to %4 (min %5)").arg(what).arg(i - 1).arg(before).arg(len).arg(min)));
			}
		}
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

// Whole scenes with random content

void RelationshipLinesTest::randomScenesKeepEveryInvariantWhateverTheUserDoes_data()
{
	QTest::addColumn<int>("seed");

	for(int seed = 1; seed <= 10; seed++)
		QTest::newRow(qPrintable(QString("seed_%1").arg(seed))) << seed;
}

void RelationshipLinesTest::randomScenesKeepEveryInvariantWhateverTheUserDoes()
{
	LeakCheckPause leak_pause;

	try
	{
		QFETCH(int, seed);

		QRandomGenerator rnd(seed * 7919);
		int mode = rnd.bounded(3), table_cnt = 6 + rnd.bounded(9);
		bool crows = rnd.bounded(2), placeholders = rnd.bounded(2);

		BaseObjectView::setPlaceholderEnabled(placeholders);
		applyMode(mode, crows);

		Rig rig;
		std::vector<Table *> tables;
		std::vector<Relationship *> rels, removed;

		for(int i = 0; i < table_cnt; i++)
			tables.push_back(addTable(rig, QString("tab_%1").arg(i), QPointF(rnd.bounded(2600), rnd.bounded(1800)), rnd.bounded(7)));

		for(int i = 0, rel_cnt = table_cnt + rnd.bounded(table_cnt); i < rel_cnt; i++)
		{
			Table *src = tables[rnd.bounded(table_cnt)], *dst = (rnd.bounded(20) == 0 ? src : tables[rnd.bounded(table_cnt)]);
			BaseRelationship::RelType type = (rnd.bounded(4) == 0 ? BaseRelationship::Relationship11 : BaseRelationship::Relationship1n);

			try
			{
				rels.push_back(addRel(rig, src, dst, type, QString("rel_%1").arg(i)));
			}
			catch(Exception &)
			{
				// Some random combinations are not valid relationships, they're just not created
			}
		}

		settle(120);
		QVERIFY2(checkAllRoutes(rig).isEmpty(), qPrintable(checkAllRoutes(rig)));

		for(int step = 0; step < 14; step++)
		{
			int action = rnd.bounded(7);
			QString what = QString("seed %1 step %2 action %3").arg(seed).arg(step).arg(action);

			if(action == 0)
			{
				std::vector<Table *> selected;

				for(int i = 0, n = 1 + rnd.bounded(5); i < n; i++)
					selected.push_back(tables[rnd.bounded(table_cnt)]);

				std::sort(selected.begin(), selected.end());
				selected.erase(std::unique(selected.begin(), selected.end()), selected.end());
				moveTables(rig, selected, QPointF(rnd.bounded(900) - 450, rnd.bounded(900) - 450), 60);
			}
			else if(action == 1 && !rels.empty())
			{
				for(int i = 0, n = 1 + rnd.bounded(4); i < n; i++)
				{
					RelationshipView *rv = relView(rels[rnd.bounded(int(rels.size()))]);

					if(rv)
						rv->invalidateRoute(rnd.bounded(2));
				}
			}
			else if(action == 2 && !rels.empty())
			{
				Relationship *rel = rels[rnd.bounded(int(rels.size()))];
				RelationshipView *rv = relView(rel);

				rel->setSegmentShifts({{ unsigned(rnd.bounded(7)), { rnd.bounded(2) ? Qt::Vertical : Qt::Horizontal,
																														 (rnd.bounded(5) == 0 ? 1e6 : rnd.bounded(500)) * (rnd.bounded(2) ? 1.0 : -1.0) } }});

				if(rv)
					rv->invalidateRoute();
			}
			else if(action == 3 && !rels.empty())
			{
				// Dragging a segment of some relationship
				RelationshipView *rv = relView(rels[rnd.bounded(int(rels.size()))]);

				if(rv && !rv->getUnderlyingObject()->isSelfRelationship() && rv->route_points.size() >= 4)
				{
					size_t idx = 1 + rnd.bounded(int(rv->route_points.size()) - 3);
					QPointF mid = segmentMiddle(rv, idx), delta = isHorizontal(rv->route_points[idx], rv->route_points[idx + 1]) ?
																								QPointF(0, rnd.bounded(300) - 150) : QPointF(rnd.bounded(300) - 150, 0);

					dragSegment(rv, mid, mid + delta, Qt::ShiftModifier);
				}
			}
			else if(action == 4)
			{
				RelationshipView::LineStyle style = static_cast<RelationshipView::LineStyle>(rnd.bounded(3));

				RelationshipView::setLineStyle(style);

				for(auto &rv : rig.scene->getRelationshipViews())
					rv->invalidateRoute();

				// Back to the orthogonal style so the routes can be checked
				RelationshipView::setLineStyle(RelationshipView::OrthogonalLines);

				for(auto &rv : rig.scene->getRelationshipViews())
					rv->invalidateRoute();
			}
			else if(action == 5 && rels.size() > 2)
			{
				int idx = rnd.bounded(int(rels.size()));

				rig.db->removeRelationship(rels[idx]);
				removed.push_back(rels[idx]);
				rels.erase(rels.begin() + idx);
			}
			else if(action == 6)
			{
				applyMode(rnd.bounded(3), rnd.bounded(2));

				for(auto &rv : rig.scene->getRelationshipViews())
					rv->invalidateRoute();
			}

			settle(40);

			QVERIFY2(checkAllRoutes(rig).isEmpty(), qPrintable(what + ": " + checkAllRoutes(rig)));

			// The lists of the scene always match its items
			std::set<QGraphicsItem *> rel_items;

			for(auto &item : rig.scene->items())
			{
				if(dynamic_cast<RelationshipView *>(item))
					rel_items.insert(item);
			}

			std::set<QGraphicsItem *> rel_list(rig.scene->getRelationshipViews().begin(), rig.scene->getRelationshipViews().end());
			QVERIFY2(rel_items == rel_list, qPrintable(what));

			// The displacements are applied inside their limits
			for(auto &rv : rig.scene->getRelationshipViews())
			{
				if(rv->getUnderlyingObject()->isSelfRelationship() || rv->getUnderlyingObject()->getSegmentShifts().empty())
					continue;

				QString err;

				minimumLengthsAreKept(rv, err);
				QVERIFY2(err.isEmpty(), qPrintable(what + ": " + err));
			}
		}

		for(auto &rel : removed)
			delete rel;
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

// TESTS-END

QTEST_MAIN(RelationshipLinesTest)
#include "relationshiplinestest.moc"
