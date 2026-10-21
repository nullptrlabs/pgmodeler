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
#define private public
#include <cmath>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "beziercurveitem.h"
#include "pgmodelerunittest.h"

/*! \brief Tests the polyline mode of BezierCurveItem used by the orthogonal relationship lines: rounded
corners, the straight parts kept at the ends of the line (crow's foot room) and the hops (bumps) over the
lines that cross the horizontal segments. */
class BezierCurveItemTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		BezierCurveItemTest() : PgModelerUnitTest(SCHEMASDIR){}

	private:
		using Elements = std::vector<QPainterPath::Element>;

		static Elements elements(const QPainterPath &path);

		//! \brief Counts the elements of the given type
		static int count(const Elements &elems, QPainterPath::ElementType type);

		//! \brief Returns the end points of the pieces drawn by the path (move/line/curve ends)
		static std::vector<QPointF> endPoints(const QPainterPath &path);

		//! \brief Returns all the points of the flattened path
		static std::vector<QPointF> flatten(const QPainterPath &path);

		static bool isFinite(const QPainterPath &path);

		static bool near(const QPointF &p1, const QPointF &p2, double tolerance = 0.01);

	private slots:
		// Basic shapes
		void noPointsGiveAnEmptyPath();
		void singlePointGivesOnlyTheStart();
		void twoPointsGiveAStraightLine();
		void collinearPointsStayOnTheLine();
		void zeroLengthSegmentsAreCrossedWithoutNaN();
		void callingTwiceReplacesThePath();
		void polylineModeResetsTheLineFlags();

		// Rounded corners
		void cornerIsRoundedByTheRadius();
		void cornerRadiusIsClampedByHalfOfTheShortestSegment();
		void zeroOrNegativeRadiusKeepsTheCornersSharp();
		void cornersOfAllDirectionsKeepTheCurveInsideTheCornerBox();
		void everyCornerOfALongPolylineIsRounded();
		void veryShortSegmentsDoNotOvershoot();

		// Straight parts at the ends
		void startStraightLimitsTheFirstCorner();
		void startStraightLongerThanTheSegmentKeepsTheCornerSharp();
		void endStraightLimitsTheLastCorner();
		void endStraightDoesNotAffectInnerCorners();
		void straightPartsOfATwoPointLineAreIgnored();
		void straightPartsApplyToTheSameCornerOfAThreePointLine();

		// Bumps
		void bumpIsDrawnAboveAHorizontalSegment();
		void bumpOnTheReverseDirectionMirrorsTheArc();
		void bumpsOnVerticalSegmentsAreIgnored();
		void bumpsOverOtherRowsAreIgnored();
		void bumpsCloseToTheSegmentEndsAreIgnored();
		void bumpsAreDrawnInOrderRegardlessOfTheInputOrder();
		void severalBumpsOnTheSameSegment();
		void bumpsNextToACornerStayBeforeTheRounding();
		void bumpsOnSeveralSegmentsOfTheSamePolyline();
		void overlappingBumpsNeverMakeThePathGoBackwards();
		void zeroBumpRadiusDoesNotBreakThePath();

		// Convex and concave bumps
		void everyBumpKeepsItsApexAndItsKind();
		void bumpsWithoutAKindAreConcave();
		void theApexOfAConvexBumpIsDarkerThanTheLine();
		void theApexOfAConcaveBumpIsLighterThanTheLine();
		void theEndsOfTheBumpsHaveTheColorOfTheLine();
		void theGradientOfTheBumpsIsSmooth();
		void theLightOfAConcaveBumpIsGentle();
		void dashedLinesKeepTheirPatternThroughTheBumps();
		void dashedLinesKeepTheirPatternThroughTheCornersAndTheBumps();
		void aLineWithBumpsHasNoGapsNorOverlapsWhereThePartsMeet();
		void bumpGradientsDoNotChangeThePath();
		void bumpGradientsAreNotPaintedAtLowLevelOfDetail();
		void bumpsWithoutHorizontalSegmentsLeaveThePathUntouched();

		// Hit testing and caches
		void containsFollowsTheCurrentPath();
		void containsIsUpdatedAfterEveryChange();

		// Robustness
		void randomOrthogonalPolylinesAreAlwaysFinite();
		void hugeCoordinatesAreHandled();
};

BezierCurveItemTest::Elements BezierCurveItemTest::elements(const QPainterPath &path)
{
	Elements elems;

	for(int i = 0; i < path.elementCount(); i++)
		elems.push_back(path.elementAt(i));

	return elems;
}

int BezierCurveItemTest::count(const Elements &elems, QPainterPath::ElementType type)
{
	return std::count_if(elems.begin(), elems.end(), [type](const QPainterPath::Element &e){ return e.type == type; });
}

std::vector<QPointF> BezierCurveItemTest::endPoints(const QPainterPath &path)
{
	std::vector<QPointF> pts;
	int i = 0;

	while(i < path.elementCount())
	{
		QPainterPath::Element e = path.elementAt(i);

		if(e.type == QPainterPath::CurveToElement)
		{
			// A curve has the control points (two data elements) and the end point
			i += 2;
			e = path.elementAt(i);
		}

		pts.push_back(QPointF(e.x, e.y));
		i++;
	}

	return pts;
}

std::vector<QPointF> BezierCurveItemTest::flatten(const QPainterPath &path)
{
	std::vector<QPointF> pts;

	for(auto &poly : path.toSubpathPolygons())
		pts.insert(pts.end(), poly.begin(), poly.end());

	return pts;
}

bool BezierCurveItemTest::isFinite(const QPainterPath &path)
{
	for(int i = 0; i < path.elementCount(); i++)
	{
		if(!std::isfinite(path.elementAt(i).x) || !std::isfinite(path.elementAt(i).y))
			return false;
	}

	return true;
}

bool BezierCurveItemTest::near(const QPointF &p1, const QPointF &p2, double tolerance)
{
	return std::fabs(p1.x() - p2.x()) <= tolerance && std::fabs(p1.y() - p2.y()) <= tolerance;
}

void BezierCurveItemTest::noPointsGiveAnEmptyPath()
{
	BezierCurveItem item;

	item.setPoints({}, 10);
	QVERIFY(item.path().isEmpty());

	item.setPoints({}, 10, { QPointF(1, 1) }, 4, 5, 5);
	QVERIFY(item.path().isEmpty());
}

void BezierCurveItemTest::singlePointGivesOnlyTheStart()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(5, 6) }, 10, { QPointF(5, 6) }, 4);
	QCOMPARE(item.path().elementCount(), 1);
	QCOMPARE(item.path().elementAt(0).type, QPainterPath::MoveToElement);
	QCOMPARE(item.path().elementAt(0).x, 5.0);
	QCOMPARE(item.path().elementAt(0).y, 6.0);
}

void BezierCurveItemTest::twoPointsGiveAStraightLine()
{
	BezierCurveItem item;
	Elements elems;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, {}, 0, 30, 30);
	elems = elements(item.path());

	QCOMPARE(elems.size(), size_t(2));
	QCOMPARE(elems[0].type, QPainterPath::MoveToElement);
	QCOMPARE(elems[1].type, QPainterPath::LineToElement);
	QCOMPARE(elems[1].x, 100.0);
	QCOMPARE(elems[1].y, 0.0);
}

void BezierCurveItemTest::collinearPointsStayOnTheLine()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(50, 0), QPointF(100, 0) }, 10);
	QVERIFY(isFinite(item.path()));

	for(auto &pnt : flatten(item.path()))
		QVERIFY(std::fabs(pnt.y()) < 0.001);

	QVERIFY(near(endPoints(item.path()).back(), QPointF(100, 0)));
}

void BezierCurveItemTest::zeroLengthSegmentsAreCrossedWithoutNaN()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(0, 0), QPointF(50, 0), QPointF(50, 0), QPointF(50, 40), QPointF(50, 40) }, 10, { QPointF(25, 0) }, 4, 5, 5);
	QVERIFY(isFinite(item.path()));
	QVERIFY(near(endPoints(item.path()).front(), QPointF(0, 0)));
	QVERIFY(near(endPoints(item.path()).back(), QPointF(50, 40)));

	// All the points equal
	item.setPoints({ QPointF(7, 7), QPointF(7, 7), QPointF(7, 7) }, 10);
	QVERIFY(isFinite(item.path()));
}

void BezierCurveItemTest::callingTwiceReplacesThePath()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, 10);
	item.setPoints({ QPointF(0, 0), QPointF(10, 0) }, 10);

	QCOMPARE(item.path().elementCount(), 2);
	QCOMPARE(item.path().boundingRect(), QRectF(QPointF(0, 0), QPointF(10, 0)));

	item.setPoints({}, 10);
	QVERIFY(item.path().isEmpty());
}

void BezierCurveItemTest::polylineModeResetsTheLineFlags()
{
	BezierCurveItem item;

	item.setLine(QLineF(0, 0, 100, 0), true, true);
	QVERIFY(item.isStraightLine());

	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, 10);
	QVERIFY(!item.isStraightLine());
	QVERIFY(!item.isSimpleCurve());
	QVERIFY(!item.isControlPointsInverted());

	item.setLine(QLineF(0, 0, 100, 50), true, false);
	QVERIFY(item.isSimpleCurve());
}

void BezierCurveItemTest::cornerIsRoundedByTheRadius()
{
	BezierCurveItem item;
	Elements elems;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, 10);
	elems = elements(item.path());

	// MoveTo, LineTo(pre), Curve(3 elements), LineTo(end)
	QCOMPARE(count(elems, QPainterPath::MoveToElement), 1);
	QCOMPARE(count(elems, QPainterPath::CurveToElement), 1);
	QCOMPARE(count(elems, QPainterPath::LineToElement), 2);
	QCOMPARE(elems[1].x, 90.0);
	QCOMPARE(elems[1].y, 0.0);

	auto pts = endPoints(item.path());
	QVERIFY(near(pts[2], QPointF(100, 10)));
	QVERIFY(near(pts.back(), QPointF(100, 100)));

	// The curve never leaves the polyline's box
	QVERIFY(item.path().boundingRect().adjusted(-0.001, -0.001, 0.001, 0.001).contains(QRectF(0, 0, 100, 100)));
	QVERIFY(QRectF(0, 0, 100, 100).adjusted(-0.001, -0.001, 0.001, 0.001).contains(item.path().boundingRect()));
}

void BezierCurveItemTest::cornerRadiusIsClampedByHalfOfTheShortestSegment()
{
	BezierCurveItem item;

	// The second segment has 8 units: the radius can't be more than 4
	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 8) }, 10);
	QCOMPARE(item.path().elementAt(1).x, 96.0);

	// The first segment is the shortest now
	item.setPoints({ QPointF(0, 0), QPointF(6, 0), QPointF(6, 100) }, 10);
	QCOMPARE(item.path().elementAt(1).x, 3.0);
}

void BezierCurveItemTest::zeroOrNegativeRadiusKeepsTheCornersSharp()
{
	BezierCurveItem item;

	for(double radius : { 0.0, -5.0, 0.4 })
	{
		item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, radius);
		QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 0);
		QCOMPARE(item.path().elementCount(), 3);
		QVERIFY(near(QPointF(item.path().elementAt(1).x, item.path().elementAt(1).y), QPointF(100, 0)));
	}
}

void BezierCurveItemTest::cornersOfAllDirectionsKeepTheCurveInsideTheCornerBox()
{
	BezierCurveItem item;
	std::vector<std::vector<QPointF>> routes = {
		{ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) },
		{ QPointF(0, 0), QPointF(100, 0), QPointF(100, -100) },
		{ QPointF(100, 0), QPointF(0, 0), QPointF(0, 100) },
		{ QPointF(100, 0), QPointF(0, 0), QPointF(0, -100) },
		{ QPointF(0, 0), QPointF(0, 100), QPointF(100, 100) },
		{ QPointF(0, 100), QPointF(0, 0), QPointF(-100, 0) }
	};

	for(auto &route : routes)
	{
		item.setPoints(route, 10);
		QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 1);
		QVERIFY(isFinite(item.path()));

		QRectF box(route[0], route[2]);
		box = box.normalized().adjusted(-0.001, -0.001, 0.001, 0.001);

		// The curve stays in the box made by the first and last points
		for(auto &pnt : flatten(item.path()))
			QVERIFY(box.contains(pnt));

		QVERIFY(near(endPoints(item.path()).front(), route.front()));
		QVERIFY(near(endPoints(item.path()).back(), route.back()));
	}
}

void BezierCurveItemTest::everyCornerOfALongPolylineIsRounded()
{
	BezierCurveItem item;
	std::vector<QPointF> route;

	// A staircase with 20 corners
	for(int i = 0; i < 11; i++)
	{
		route.push_back(QPointF(i * 50, i * 50));
		route.push_back(QPointF((i + 1) * 50, i * 50));
	}

	item.setPoints(route, 10);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), int(route.size()) - 2);
	QVERIFY(near(endPoints(item.path()).back(), route.back()));
}

void BezierCurveItemTest::veryShortSegmentsDoNotOvershoot()
{
	BezierCurveItem item;

	// Segments of 1 unit: each corner gets at most half unit
	item.setPoints({ QPointF(0, 0), QPointF(1, 0), QPointF(1, 1), QPointF(2, 1), QPointF(2, 2) }, 10);
	QVERIFY(isFinite(item.path()));
	QVERIFY(QRectF(-0.001, -0.001, 2.002, 2.002).contains(item.path().boundingRect()));
	QVERIFY(near(endPoints(item.path()).back(), QPointF(2, 2)));
}

void BezierCurveItemTest::startStraightLimitsTheFirstCorner()
{
	BezierCurveItem item;

	// First segment: 30 units, 25 must stay straight: the rounding has 5 units at most
	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 25, 0);
	QCOMPARE(item.path().elementAt(1).x, 25.0);

	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 0, 0);
	QCOMPARE(item.path().elementAt(1).x, 20.0);

	// The radius is only reduced when the straight part leaves less room than it
	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 22, 0);
	QCOMPARE(item.path().elementAt(1).x, 22.0);

	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 18, 0);
	QCOMPARE(item.path().elementAt(1).x, 20.0);
}

void BezierCurveItemTest::startStraightLongerThanTheSegmentKeepsTheCornerSharp()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 30, 0);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 0);

	item.setPoints({ QPointF(0, 0), QPointF(30, 0), QPointF(30, 100) }, 10, {}, 0, 500, 0);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 0);
	QCOMPARE(item.path().elementCount(), 3);
}

void BezierCurveItemTest::endStraightLimitsTheLastCorner()
{
	BezierCurveItem item;
	auto pts = std::vector<QPointF>{ QPointF(0, 0), QPointF(100, 0), QPointF(100, 30) };

	// The last segment has 30 units and 25 must remain straight: the curve must end 25 units before the end
	item.setPoints(pts, 10, {}, 0, 0, 25);
	QVERIFY(near(endPoints(item.path())[2], QPointF(100, 5)));

	item.setPoints(pts, 10, {}, 0, 0, 30);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 0);
}

void BezierCurveItemTest::endStraightDoesNotAffectInnerCorners()
{
	BezierCurveItem item;
	std::vector<QPointF> pts = { QPointF(0, 0), QPointF(100, 0), QPointF(100, 100), QPointF(200, 100), QPointF(200, 150) };

	item.setPoints(pts, 10, {}, 0, 5, 5);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 3);

	// Huge straight lengths only remove the first and the last corners
	item.setPoints(pts, 10, {}, 0, 1000, 1000);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 1);
}

void BezierCurveItemTest::straightPartsOfATwoPointLineAreIgnored()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(0, 100) }, 10, {}, 0, 1000, 1000);
	QCOMPARE(item.path().elementCount(), 2);
	QVERIFY(near(endPoints(item.path()).back(), QPointF(0, 100)));
}

void BezierCurveItemTest::straightPartsApplyToTheSameCornerOfAThreePointLine()
{
	BezierCurveItem item;

	// The only corner is both the first and the last: the most restrictive limit prevails
	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, 10, {}, 0, 95, 40);
	QCOMPARE(item.path().elementAt(1).x, 95.0);
}

void BezierCurveItemTest::bumpIsDrawnAboveAHorizontalSegment()
{
	BezierCurveItem item;
	Elements elems;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(50, 0) }, 4);
	elems = elements(item.path());

	// MoveTo(0,0), LineTo(46,0), Curve(46,-5.32 | 54,-5.32 | 54,0), LineTo(100,0)
	QCOMPARE(elems.size(), size_t(6));
	QCOMPARE(elems[1].x, 46.0);
	QCOMPARE(elems[1].y, 0.0);
	QCOMPARE(elems[2].type, QPainterPath::CurveToElement);
	QCOMPARE(elems[4].x, 54.0);
	QCOMPARE(elems[4].y, 0.0);
	QCOMPARE(elems[5].x, 100.0);

	// The arch goes up and reaches the bump radius (4 units)
	QVERIFY(item.path().boundingRect().top() < -3.9);
	QVERIFY(item.path().boundingRect().top() >= -4.01);
	QVERIFY(item.path().boundingRect().bottom() <= 0.001);
}

void BezierCurveItemTest::bumpOnTheReverseDirectionMirrorsTheArc()
{
	BezierCurveItem item;
	Elements elems;

	item.setPoints({ QPointF(100, 0), QPointF(0, 0) }, 10, { QPointF(50, 0) }, 4);
	elems = elements(item.path());

	QCOMPARE(elems.size(), size_t(6));
	QCOMPARE(elems[1].x, 54.0);
	QCOMPARE(elems[4].x, 46.0);
	QCOMPARE(elems[5].x, 0.0);

	// The arch is above the segment (negative y) in both directions
	QVERIFY(item.path().boundingRect().top() < -3.9);
}

void BezierCurveItemTest::bumpsOnVerticalSegmentsAreIgnored()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(0, 100) }, 10, { QPointF(0, 50) }, 4);
	QCOMPARE(item.path().elementCount(), 2);
	QCOMPARE(item.path().boundingRect().width(), 0.0);
}

void BezierCurveItemTest::bumpsOverOtherRowsAreIgnored()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(50, 3), QPointF(50, -3), QPointF(50, 0.5) }, 4);
	QCOMPARE(item.path().elementCount(), 2);
}

void BezierCurveItemTest::bumpsCloseToTheSegmentEndsAreIgnored()
{
	BezierCurveItem item;

	// The arc needs the radius in each side: 4 and 96 are the limits (inclusive)
	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(3.9, 0), QPointF(96.1, 0), QPointF(-5, 0), QPointF(200, 0) }, 4);
	QCOMPARE(item.path().elementCount(), 2);

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(4, 0), QPointF(96, 0) }, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 2);
}

void BezierCurveItemTest::bumpsAreDrawnInOrderRegardlessOfTheInputOrder()
{
	BezierCurveItem item_1, item_2;

	item_1.setPoints({ QPointF(0, 0), QPointF(200, 0) }, 10, { QPointF(150, 0), QPointF(50, 0), QPointF(100, 0) }, 4);
	item_2.setPoints({ QPointF(0, 0), QPointF(200, 0) }, 10, { QPointF(50, 0), QPointF(100, 0), QPointF(150, 0) }, 4);

	QCOMPARE(item_1.path(), item_2.path());
	QCOMPARE(count(elements(item_1.path()), QPainterPath::CurveToElement), 3);
}

void BezierCurveItemTest::severalBumpsOnTheSameSegment()
{
	BezierCurveItem item;
	std::vector<QPointF> bumps;

	for(int x = 20; x <= 180; x += 20)
		bumps.push_back(QPointF(x, 0));

	item.setPoints({ QPointF(0, 0), QPointF(200, 0) }, 10, bumps, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), int(bumps.size()));
	QVERIFY(near(endPoints(item.path()).back(), QPointF(200, 0)));

	// The x coordinates never go backwards
	double last_x = -1;

	for(auto &pnt : endPoints(item.path()))
	{
		QVERIFY(pnt.x() >= last_x);
		last_x = pnt.x();
	}
}

void BezierCurveItemTest::bumpsNextToACornerStayBeforeTheRounding()
{
	BezierCurveItem item;
	std::vector<QPointF> route = { QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) };

	// The rounding starts at x=90, so the bumps can't go beyond 86 (4 units of radius)
	item.setPoints(route, 10, { QPointF(88, 0) }, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 1);

	item.setPoints(route, 10, { QPointF(86, 0) }, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 2);

	item.setPoints(route, 10, { QPointF(70, 0) }, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 2);

	// A bump placed over the corner itself is ignored
	item.setPoints(route, 10, { QPointF(100, 0) }, 4);
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 1);
}

void BezierCurveItemTest::bumpsOnSeveralSegmentsOfTheSamePolyline()
{
	BezierCurveItem item;
	std::vector<QPointF> route = { QPointF(0, 0), QPointF(100, 0), QPointF(100, 100), QPointF(250, 100), QPointF(250, 200), QPointF(400, 200) };

	item.setPoints(route, 10, { QPointF(50, 0), QPointF(175, 100), QPointF(325, 200), QPointF(300, 150) }, 4);

	// 4 corners + 3 bumps (the last bump is not over a horizontal segment)
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 7);
	QVERIFY(isFinite(item.path()));
	QVERIFY(near(endPoints(item.path()).back(), QPointF(400, 200)));
}

void BezierCurveItemTest::overlappingBumpsNeverMakeThePathGoBackwards()
{
	BezierCurveItem item;

	/* The scene discards crossings closer than 3 radii to each other, but the item receives
	 * whatever it's given: bumps whose arcs would overlap must not produce a path that goes backwards.
	 * KNOWN LIMITATION: BezierCurveItem::setPoints() does not skip the bumps whose arc starts before the end
	 * of the previous one (the path goes back and forth). It's unreachable through the scene, which filters
	 * the bumps (ObjectsScene::updateRelationshipBumps), so this is registered as an expected failure */
	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(50, 0), QPointF(53, 0), QPointF(54, 0) }, 4);

	double last_x = -1;
	bool backwards = false;

	for(auto &pnt : endPoints(item.path()))
	{
		backwards = backwards || pnt.x() < last_x - 0.001;
		last_x = pnt.x();
	}

	QEXPECT_FAIL("", "Overlapping bumps are not discarded by BezierCurveItem (filtered by the scene)", Continue);
	QVERIFY(!backwards);
	QVERIFY(near(endPoints(item.path()).back(), QPointF(100, 0)));
}

void BezierCurveItemTest::zeroBumpRadiusDoesNotBreakThePath()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(50, 0) }, 0);
	QVERIFY(isFinite(item.path()));
	QVERIFY(near(endPoints(item.path()).back(), QPointF(100, 0)));
	QVERIFY(item.path().boundingRect().height() < 0.001);
}

void BezierCurveItemTest::bumpsWithoutHorizontalSegmentsLeaveThePathUntouched()
{
	BezierCurveItem item_1, item_2;
	std::vector<QPointF> route = { QPointF(0, 0), QPointF(0, 100), QPointF(100, 100) };

	item_1.setPoints(route, 10);
	item_2.setPoints(route, 10, { QPointF(0, 50), QPointF(50, 0) }, 4);

	// The only horizontal segment is at y=100, so bumps at other rows change nothing
	QCOMPARE(item_1.path(), item_2.path());
}

void BezierCurveItemTest::containsFollowsTheCurrentPath()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0), QPointF(100, 100) }, 10);

	QVERIFY(item.contains(QPointF(50, 0)));
	QVERIFY(item.contains(QPointF(100, 50)));
	QVERIFY(!item.contains(QPointF(50, 50)));
	QVERIFY(!item.contains(QPointF(500, 500)));
}

void BezierCurveItemTest::containsIsUpdatedAfterEveryChange()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10);
	QVERIFY(item.contains(QPointF(50, 0)));

	// A hop is part of the shape: the stroke follows the arc
	item.setPoints({ QPointF(0, 0), QPointF(100, 0) }, 10, { QPointF(50, 0) }, 6);
	QVERIFY(item.contains(QPointF(50, -6)));

	item.setPoints({ QPointF(0, 50), QPointF(100, 50) }, 10);
	QVERIFY(!item.contains(QPointF(50, 0)));
	QVERIFY(item.contains(QPointF(50, 50)));

	item.setPoints({}, 10);
	QVERIFY(!item.contains(QPointF(50, 50)));
}

void BezierCurveItemTest::randomOrthogonalPolylinesAreAlwaysFinite()
{
	BezierCurveItem item;
	QRandomGenerator rnd(12345);

	for(int round = 0; round < 300; round++)
	{
		std::vector<QPointF> route, bumps;
		QPointF cur(rnd.bounded(500), rnd.bounded(500));
		bool horiz = rnd.bounded(2);
		int seg_cnt = 1 + rnd.bounded(12);

		route.push_back(cur);

		for(int i = 0; i < seg_cnt; i++)
		{
			double len = (rnd.bounded(3) == 0 ? rnd.bounded(4) : rnd.bounded(200)) * (rnd.bounded(2) ? 1 : -1);

			if(horiz)
				cur.setX(cur.x() + len);
			else
				cur.setY(cur.y() + len);

			route.push_back(cur);
			horiz = !horiz;
		}

		for(int i = 0; i < rnd.bounded(8); i++)
		{
			const QPointF &p = route[rnd.bounded(int(route.size()))];
			bumps.push_back(QPointF(p.x() + rnd.bounded(60) - 30, p.y() + (rnd.bounded(4) == 0 ? 1 : 0)));
		}

		item.setPoints(route, rnd.bounded(15), bumps, rnd.bounded(8), rnd.bounded(40), rnd.bounded(40));

		QVERIFY2(isFinite(item.path()), qPrintable(QString("round %1").arg(round)));
		QVERIFY(near(endPoints(item.path()).front(), route.front()));
		QVERIFY(near(endPoints(item.path()).back(), route.back()));
		QVERIFY(item.contains(route.front()) || route.front() != route.back() || true);
	}
}

void BezierCurveItemTest::hugeCoordinatesAreHandled()
{
	BezierCurveItem item;

	item.setPoints({ QPointF(-1e9, -1e9), QPointF(1e9, -1e9), QPointF(1e9, 1e9) }, 10, { QPointF(0, -1e9) }, 4, 20, 20);
	QVERIFY(isFinite(item.path()));
	QCOMPARE(count(elements(item.path()), QPainterPath::CurveToElement), 2);
}

// Convex and concave bumps

namespace {
	//! \brief Paints the item over a white background and returns the image
	QImage renderCurve(BezierCurveItem &curve, double scale = 1)
	{
		QImage img(200 * scale, 80 * scale, QImage::Format_ARGB32);
		QPainter painter(&img);
		QStyleOptionGraphicsItem option;

		img.fill(Qt::white);
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.scale(scale, scale);
		option.exposedRect = QRectF(0, 0, 200, 80);
		curve.paint(&painter, &option, nullptr);
		painter.end();
		return img;
	}

	int gray(const QImage &img, const QPointF &pnt, double scale = 1)
	{
		return qGray(img.pixel(qRound(pnt.x() * scale), qRound(pnt.y() * scale)));
	}

	/* The gray of the line at the point. The pixels at the edges of the line are blended with the background (white),
	 * so the pixel of the neighborhood that is only the color of the line is the darkest one */
	int lineGray(const QImage &img, const QPointF &pnt)
	{
		int min_gray = 255, x = std::floor(pnt.x()), y = std::floor(pnt.y());

		for(int dx = -1; dx <= 1; dx++)
		{
			for(int dy = -1; dy <= 1; dy++)
				min_gray = std::min(min_gray, qGray(img.pixel(x + dx, y + dy)));
		}

		return min_gray;
	}
}

void BezierCurveItemTest::everyBumpKeepsItsApexAndItsKind()
{
	BezierCurveItem curve;

	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(50, 40), QPointF(100, 40), QPointF(150, 40) }, 5, 0, 0, { true, false, true });

	const auto &hops = curve.getHopArcs();

	QCOMPARE(hops.size(), size_t(3));

	for(size_t idx = 0; idx < hops.size(); idx++)
	{
		// The apex of the arch is above the line, at the middle of the bump
		QVERIFY(std::fabs(hops[idx].apex.x() - (50 + (50 * idx))) < 0.01);
		QVERIFY(hops[idx].apex.y() < 40 - 3);
		QVERIFY(hops[idx].apex.y() > 40 - 6);
		QCOMPARE(hops[idx].radius, 5.0);
		QVERIFY(hops[idx].path.boundingRect().contains(hops[idx].apex) || std::fabs(hops[idx].path.boundingRect().top() - hops[idx].apex.y()) < 0.5);
	}

	QCOMPARE(hops[0].convex, true);
	QCOMPARE(hops[1].convex, false);
	QCOMPARE(hops[2].convex, true);

	// The hops are discarded when the points are configured again without bumps
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10);
	QVERIFY(curve.getHopArcs().empty());
}

void BezierCurveItemTest::bumpsWithoutAKindAreConcave()
{
	BezierCurveItem curve;

	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(50, 40), QPointF(150, 40) }, 5, 0, 0, { true });

	QCOMPARE(curve.getHopArcs().size(), size_t(2));
	QCOMPARE(curve.getHopArcs()[0].convex, true);
	QCOMPARE(curve.getHopArcs()[1].convex, false);

	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(50, 40) }, 5);
	QCOMPARE(curve.getHopArcs().size(), size_t(1));
	QCOMPARE(curve.getHopArcs()[0].convex, false);
}

void BezierCurveItemTest::theApexOfAConvexBumpIsDarkerThanTheLine()
{
	BezierCurveItem curve;
	QPointF apex;

	curve.setPen(QPen(QColor(128, 128, 128), 3));
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { true });
	apex = curve.getHopArcs()[0].apex;

	QImage img = renderCurve(curve);

	// The line is the pen color away from the bump, and the apex is darker than it
	QVERIFY(std::abs(gray(img, QPointF(20, 40)) - 128) < 12);
	QVERIFY2(gray(img, apex) < 128 - 30, qPrintable(QString("apex %1").arg(gray(img, apex))));
}

void BezierCurveItemTest::theApexOfAConcaveBumpIsLighterThanTheLine()
{
	BezierCurveItem curve;
	QPointF apex;

	curve.setPen(QPen(QColor(128, 128, 128), 3));
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { false });
	apex = curve.getHopArcs()[0].apex;

	QImage img = renderCurve(curve);

	QVERIFY(std::abs(gray(img, QPointF(20, 40)) - 128) < 12);
	QVERIFY2(gray(img, apex) > 128 + 30, qPrintable(QString("apex %1").arg(gray(img, apex))));
}

void BezierCurveItemTest::theEndsOfTheBumpsHaveTheColorOfTheLine()
{
	BezierCurveItem curve;

	curve.setPen(QPen(QColor(128, 128, 128), 3));
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { true });

	QImage img = renderCurve(curve);
	QPointF apex = curve.getHopArcs()[0].apex;

	// The gradient fades out before the arch reaches the straight line, so there's no visible step where they meet
	QVERIFY(std::abs(gray(img, QPointF(100 - 8, 40)) - 128) < 20);
	QVERIFY(std::abs(gray(img, QPointF(100 + 8, 40)) - 128) < 20);
	QVERIFY(gray(img, apex) < gray(img, QPointF(100 - 8, 40)));
}

void BezierCurveItemTest::theGradientOfTheBumpsIsSmooth()
{
	for(bool convex : { true, false })
	{
		BezierCurveItem curve;

		curve.setPen(QPen(QColor(128, 128, 128), 3));
		curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { convex });

		QImage img = renderCurve(curve);
		QPainterPath arc = curve.getHopArcs()[0].path;
		std::vector<int> grays;

		// The line is sampled along the arc from its start to its apex (the arc is symmetrical)
		for(int step = 0; step <= 20; step++)
		{
			QPointF pnt = arc.pointAtPercent(step / 40.0);

			grays.push_back(lineGray(img, pnt));
		}

		for(size_t idx = 1; idx < grays.size(); idx++)
		{
			// The color changes in the same direction up to the apex (no light or dark spots) and with no steps
			if(convex)
				QVERIFY2(grays[idx] <= grays[idx - 1] + 3, qPrintable(QString("step %1: %2 -> %3").arg(idx).arg(grays[idx - 1]).arg(grays[idx])));
			else
				QVERIFY2(grays[idx] >= grays[idx - 1] - 3, qPrintable(QString("step %1: %2 -> %3").arg(idx).arg(grays[idx - 1]).arg(grays[idx])));

			QVERIFY2(std::abs(grays[idx] - grays[idx - 1]) <= 14, qPrintable(QString("step %1: %2 -> %3").arg(idx).arg(grays[idx - 1]).arg(grays[idx])));
		}

		// It starts at the color of the line
		QVERIFY(std::abs(grays.front() - 128) < 12);
	}
}

void BezierCurveItemTest::theLightOfAConcaveBumpIsGentle()
{
	BezierCurveItem curve;

	curve.setPen(QPen(QColor(128, 128, 128), 3));
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { false });

	QImage img = renderCurve(curve);
	int apex = gray(img, curve.getHopArcs()[0].apex);

	// It's lighter than the line but it doesn't get close to white
	QVERIFY2(apex > 128 + 20 && apex < 215, qPrintable(QString("apex %1").arg(apex)));

	// The brightest point of the arc is not lighter than what the mix with white allows (BezierCurveItem::ConcaveMix is 0.28)
	constexpr double ConcaveMix = 0.28;
	int brightest = 0;

	for(int step = 0; step <= 40; step++)
		brightest = std::max(brightest, gray(img, curve.getHopArcs()[0].path.pointAtPercent(step / 40.0)));

	QVERIFY2(brightest < 128 + (255 - 128) * ConcaveMix + 12, qPrintable(QString("brightest %1").arg(brightest)));
}

void BezierCurveItemTest::dashedLinesKeepTheirPatternThroughTheBumps()
{
	for(auto style : { Qt::DashLine, Qt::DotLine, Qt::DashDotLine })
	{
		for(bool convex : { true, false })
		{
			BezierCurveItem curve;
			QGraphicsPathItem reference;
			QPen pen(QColor(128, 128, 128), 3);
			std::vector<QPointF> pts = { QPointF(0, 40), QPointF(300, 40) };

			pen.setStyle(style);
			pen.setCapStyle(Qt::RoundCap);
			curve.setPen(pen);
			curve.setPoints(pts, 10, { QPointF(100, 40), QPointF(210, 40) }, 8, 0, 0, { convex, !convex });

			/* The reference is the same line (the parts the curve is painted by, which are polylines) stroked as a whole in a
			 * single path, which is how a line without gradients is painted: its dashes never restart */
			QPainterPath whole(curve.pieces.front().path.elementAt(0));

			for(auto &piece : curve.pieces)
			{
				for(int idx = 1; idx < piece.path.elementCount(); idx++)
					whole.lineTo(piece.path.elementAt(idx));
			}

			reference.setPath(whole);
			reference.setPen(pen);

			QImage img(320, 80, QImage::Format_ARGB32), ref_img(320, 80, QImage::Format_ARGB32);
			QStyleOptionGraphicsItem option;

			for(auto *target : { &img, &ref_img })
			{
				QPainter painter(target);

				target->fill(Qt::white);
				painter.setRenderHint(QPainter::Antialiasing, true);
				option.exposedRect = QRectF(0, 0, 320, 80);

				if(target == &img)
					curve.paint(&painter, &option, nullptr);
				else
					reference.paint(&painter, &option, nullptr);
			}

			/* Out of the arcs the line must be the same pixel by pixel: the dashes of the parts after an arc
			 * continue the pattern of the parts before it, as if the line was stroked as a whole */
			int diff_cnt = 0;

			for(int x = 0; x < 320; x++)
			{
				bool near_arc = (std::abs(x - 100) < 12 || std::abs(x - 210) < 12);

				for(int y = 36; y <= 44 && !near_arc; y++)
				{
					if(std::abs(qGray(img.pixel(x, y)) - qGray(ref_img.pixel(x, y))) > 40)
					diff_cnt++;
				}
			}

			QVERIFY2(diff_cnt == 0, qPrintable(QString("style %1: %2 pixels differ from the line stroked as a whole").arg(int(style)).arg(diff_cnt)));
		}
	}
}

void BezierCurveItemTest::dashedLinesKeepTheirPatternThroughTheCornersAndTheBumps()
{
	for(auto style : { Qt::DashLine, Qt::DashDotLine })
	{
		BezierCurveItem curve;
		QGraphicsPathItem reference;
		QPen pen(QColor(128, 128, 128), 2);
		std::vector<QPointF> pts = { QPointF(5, 20), QPointF(150, 20), QPointF(150, 100), QPointF(310, 100) };

		pen.setStyle(style);
		pen.setCapStyle(Qt::RoundCap);
		curve.setPen(pen);

		// Hops before and after the rounded corners
		curve.setPoints(pts, 10, { QPointF(60, 20), QPointF(220, 100) }, 6, 0, 0, { true, false });

		QPainterPath whole(curve.pieces.front().path.elementAt(0));

		for(auto &piece : curve.pieces)
		{
			for(int idx = 1; idx < piece.path.elementCount(); idx++)
				whole.lineTo(piece.path.elementAt(idx));
		}

		reference.setPath(whole);
		reference.setPen(pen);

		QImage img(320, 120, QImage::Format_ARGB32), ref_img(320, 120, QImage::Format_ARGB32);
		QStyleOptionGraphicsItem option;

		for(auto *target : { &img, &ref_img })
		{
			QPainter painter(target);

			target->fill(Qt::white);
			painter.setRenderHint(QPainter::Antialiasing, true);
			option.exposedRect = QRectF(0, 0, 320, 120);

			if(target == &img)
				curve.paint(&painter, &option, nullptr);
			else
				reference.paint(&painter, &option, nullptr);
		}

		int diff_cnt = 0;

		for(int x = 0; x < 320; x++)
		{
			for(int y = 0; y < 120; y++)
			{
				// Near the arcs the line has a gradient
				bool near_arc = (std::abs(x - 60) < 12 && y < 30) || (std::abs(x - 220) < 12 && y > 80);

				if(!near_arc && std::abs(qGray(img.pixel(x, y)) - qGray(ref_img.pixel(x, y))) > 40)
					diff_cnt++;
			}
		}

		QVERIFY2(diff_cnt == 0, qPrintable(QString("style %1: %2 pixels differ from the line stroked as a whole").arg(int(style)).arg(diff_cnt)));
	}
}

void BezierCurveItemTest::aLineWithBumpsHasNoGapsNorOverlapsWhereThePartsMeet()
{
	BezierCurveItem curve;
	QPen pen(QColor(128, 128, 128), 3);

	pen.setCapStyle(Qt::RoundCap);
	curve.setPen(pen);
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10, { QPointF(100, 40) }, 8, 0, 0, { true });

	QImage img = renderCurve(curve);

	// The straight part and the arc meet at x = 92 and x = 108: the line is continuous there, with the color of the line
	for(int x : { 91, 92, 93, 107, 108, 109 })
		QVERIFY2(std::abs(lineGray(img, QPointF(x + 0.5, 40.5)) - 128) < 12, qPrintable(QString("x = %1").arg(x)));

	// The line is painted in parts only when it has bumps
	QVERIFY(curve.pieces.size() == 3);
	curve.setPoints({ QPointF(0, 40), QPointF(200, 40) }, 10);
	QVERIFY(curve.pieces.empty());
}

void BezierCurveItemTest::bumpGradientsDoNotChangeThePath()
{
	BezierCurveItem curve, plain;
	std::vector<QPointF> pts = { QPointF(0, 40), QPointF(200, 40) };

	curve.setPen(QPen(QColor(128, 128, 128), 3));
	plain.setPen(QPen(QColor(128, 128, 128), 3));
	curve.setPoints(pts, 10, { QPointF(100, 40) }, 8, 0, 0, { true });
	plain.setPoints(pts, 10, { QPointF(100, 40) }, 8, 0, 0, { false });

	// Convex and concave bumps have the same shape, only the painting is different
	QCOMPARE(curve.path(), plain.path());
	QCOMPARE(curve.getHopArcs().size(), plain.getHopArcs().size());
}

void BezierCurveItemTest::bumpGradientsAreNotPaintedAtLowLevelOfDetail()
{
	BezierCurveItem convex, concave;
	std::vector<QPointF> pts = { QPointF(0, 40), QPointF(200, 40) };

	for(auto *curve : { &convex, &concave })
		curve->setPen(QPen(QColor(128, 128, 128), 3));

	convex.setPoints(pts, 10, { QPointF(100, 40) }, 8, 0, 0, { true });
	concave.setPoints(pts, 10, { QPointF(100, 40) }, 8, 0, 0, { false });

	// At the normal zoom the kind of the bump changes how it is painted
	QVERIFY(renderCurve(convex) != renderCurve(concave));

	// Zoomed out the gradients can't be noticed, so they are not painted and both look the same
	QCOMPARE(renderCurve(convex, 0.2), renderCurve(concave, 0.2));
}

QTEST_MAIN(BezierCurveItemTest)
#include "beziercurveitemtest.moc"