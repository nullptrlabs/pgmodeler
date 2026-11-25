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
#include <QtWidgets>
#include <QPageLayout>
#include <cmath>
#include <memory>
#include <vector>
#include "objectsscene.h"
#include "textbox.h"
#include "textboxview.h"
#include "pgmodelerunittest.h"

/*! \brief Tests the computation of the pages used to print/export a scene (ObjectsScene::getPagesForPrinting),
mainly the range of pages covered when the scene has negative coordinates (infinite canvas) and when the pages are
scaled (delimiters size locked). The pages are compared against a brute force enumeration of the grid of pages
aligned to the origin that intersect the items. */
class ObjectsSceneTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		ObjectsSceneTest() : PgModelerUnitTest(SCHEMASDIR){}

	private:
		static constexpr double PageW = 400, PageH = 300;

		//! \brief Page layout without margins so the paint rect is exactly PageW x PageH points
		static QPageLayout pageLayout();

		//! \brief Adds a textbox view to the scene with its top-left at the given position
		static std::vector<std::unique_ptr<Textbox>> &textboxes();
		static QGraphicsItem *addBox(ObjectsScene &scn, const QPointF &pos);

		/*! \brief Enumerates the pages (as a set of grid indexes) that must be printed for the given
		 * items rect, page size and scale. Independent from the implementation under test */
		static QSet<QPair<int, int>> expectedPages(const QRectF &items_rect, double scale);

		static QSet<QPair<int, int>> toIndexes(const QList<QRectF> &pages, double scale);

	private slots:
		void cleanup();

		void emptySceneHasNoPages();
		void singleItemInsideOnePageIsOnePage();
		void sliverInLastColumnIsNotLost_data();
		void sliverInLastColumnIsNotLost();
		void pagesMatchBruteForce_data();
		void pagesMatchBruteForce();
		void everyPointOfTheItemsIsCovered();
		void pageCountsMatchTheGridOfPages();
		void defaultOverloadUsesSceneLayout();
};

std::vector<std::unique_ptr<Textbox>> &ObjectsSceneTest::textboxes()
{
	static std::vector<std::unique_ptr<Textbox>> list;
	return list;
}

QPageLayout ObjectsSceneTest::pageLayout()
{
	return QPageLayout(QPageSize(QSizeF(PageW, PageH), QPageSize::Point),
										 QPageLayout::Portrait, QMarginsF(0, 0, 0, 0));
}

QGraphicsItem *ObjectsSceneTest::addBox(ObjectsScene &scn, const QPointF &pos)
{
	auto txtbox = std::make_unique<Textbox>();
	txtbox->setComment("box");
	txtbox->setPosition(pos);

	/* The scene takes the ownership of the view, the model object is kept
	 * alive here until the end of the test (see cleanup()) */
	auto view = new TextboxView(txtbox.get());
	view->setPos(pos);
	scn.addItem(view);

	// Views are created hidden (the model widget shows them according to the active layers)
	view->setVisible(true);
	textboxes().push_back(std::move(txtbox));
	return view;
}

QSet<QPair<int, int>> ObjectsSceneTest::expectedPages(const QRectF &items_rect, double scale)
{
	QSet<QPair<int, int>> idxs;
	double pw = PageW / scale, ph = PageH / scale;

	for(int v = -40; v <= 40; v++)
	{
		for(int h = -40; h <= 40; h++)
		{
			if(items_rect.intersects(QRectF(h * pw, v * ph, pw + 1, ph + 1)))
				idxs.insert({h, v});
		}
	}

	return idxs;
}

QSet<QPair<int, int>> ObjectsSceneTest::toIndexes(const QList<QRectF> &pages, double scale)
{
	QSet<QPair<int, int>> idxs;
	double pw = PageW / scale, ph = PageH / scale;

	for(auto &pg : pages)
		idxs.insert({qRound(pg.left() / pw), qRound(pg.top() / ph)});

	return idxs;
}

void ObjectsSceneTest::cleanup()
{
	// Views are owned by the scenes that are already destroyed at this point
	textboxes().clear();
}

void ObjectsSceneTest::emptySceneHasNoPages()
{
	ObjectsScene scn;
	unsigned h = 99, v = 99;
	QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, 1);

	QVERIFY(pages.isEmpty());
	QCOMPARE(h, 0u);
	QCOMPARE(v, 0u);
}

void ObjectsSceneTest::singleItemInsideOnePageIsOnePage()
{
	ObjectsScene scn;
	unsigned h = 0, v = 0;

	addBox(scn, QPointF(50, 50));
	QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, 1);

	QCOMPARE(pages.size(), 1);
	QCOMPARE(h, 1u);
	QCOMPARE(v, 1u);
	QCOMPARE(pages.first().topLeft(), QPointF(0, 0));
	QVERIFY(qFuzzyCompare(pages.first().width(), PageW + 1));
}

void ObjectsSceneTest::sliverInLastColumnIsNotLost_data()
{
	QTest::addColumn<double>("scale");
	QTest::addColumn<double>("scene_left");

	/* The reported case: the scene starts a bit before the origin (scene rect with a grid padding
	 * in the infinite canvas) and the items go just a bit beyond the first page horizontally */
	QTest::newRow("scale 0.5, scene left -20") << 0.5 << -20.0;
	QTest::newRow("scale 0.5, scene left 0") << 0.5 << 0.0;
	QTest::newRow("scale 0.5, scene left 30") << 0.5 << 30.0;
	QTest::newRow("scale 1, scene left -20") << 1.0 << -20.0;
	QTest::newRow("scale 0.25, scene left -20") << 0.25 << -20.0;
	QTest::newRow("scale 0.25, scene left -700") << 0.25 << -700.0;
}

void ObjectsSceneTest::sliverInLastColumnIsNotLost()
{
	QFETCH(double, scale);
	QFETCH(double, scene_left);

	ObjectsScene scn;
	double pw = PageW / scale, ph = PageH / scale;
	unsigned h = 0, v = 0;

	/* Tables laid out in a 2x2 pages area: the right column holds only a small portion of
	 * the items (the boxes at the extreme right start a few units before the page boundary) */
	addBox(scn, QPointF(30, 30));
	addBox(scn, QPointF(pw - 20, 40));
	addBox(scn, QPointF(40, ph + 30));
	addBox(scn, QPointF(pw - 20, ph + 40));

	QRectF items_rect = scn.itemsBoundingRect(true);
	scn.setSceneRect(QRectF(QPointF(scene_left, -20), items_rect.bottomRight() + QPointF(20, 20)));

	QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, scale);

	QCOMPARE(toIndexes(pages, scale), expectedPages(items_rect, scale));
	QCOMPARE(pages.size(), static_cast<qsizetype>(expectedPages(items_rect, scale).size()));
	QCOMPARE(h, 2u);
	QCOMPARE(v, 2u);
}

void ObjectsSceneTest::pagesMatchBruteForce_data()
{
	QTest::addColumn<double>("scale");
	QTest::addColumn<QPointF>("origin");
	QTest::addColumn<QSizeF>("extent");

	// Sweeps scale factors, origins (negative, around zero, positive) and extents (fractions of pages)
	for(double scale : { 1.0, 0.8, 0.5, 0.33, 0.1 })
	{
		for(QPointF origin : { QPointF(-3000, -2200), QPointF(-250, -130), QPointF(-20, -20), QPointF(0, 0), QPointF(25, 15), QPointF(900, 700) })
		{
			for(QSizeF extent : { QSizeF(60, 40), QSizeF(PageW / scale * 1.05, PageH / scale * 1.4),
														QSizeF(PageW / scale * 2.3, PageH / scale * 0.9), QSizeF(PageW / scale * 3.6, PageH / scale * 2.7) })
			{
				QTest::newRow(qPrintable(QString("scale %1 origin %2,%3 extent %4x%5")
														.arg(scale).arg(origin.x()).arg(origin.y())
														.arg(qRound(extent.width())).arg(qRound(extent.height()))))
								<< scale << origin << extent;
			}
		}
	}
}

void ObjectsSceneTest::pagesMatchBruteForce()
{
	QFETCH(double, scale);
	QFETCH(QPointF, origin);
	QFETCH(QSizeF, extent);

	ObjectsScene scn;
	unsigned h = 0, v = 0;

	// Two boxes at opposite corners define the items rect
	addBox(scn, origin);
	addBox(scn, origin + QPointF(extent.width(), extent.height()));

	QRectF items_rect = scn.itemsBoundingRect(true);
	QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, scale);
	auto exp = expectedPages(items_rect, scale);

	QVERIFY(!exp.isEmpty());
	QCOMPARE(pages.size(), static_cast<qsizetype>(exp.size()));
	QCOMPARE(toIndexes(pages, scale), exp);
}

void ObjectsSceneTest::everyPointOfTheItemsIsCovered()
{
	// Whatever the scale, the union of the printed pages must contain the whole items rect
	for(double scale : { 1.0, 0.5, 0.3, 0.1 })
	{
		for(double left : { -20.0, 0.0, 35.0 })
		{
			ObjectsScene scn;
			unsigned h = 0, v = 0;
			double pw = PageW / scale, ph = PageH / scale;

			addBox(scn, QPointF(left, -20));
			addBox(scn, QPointF(left + pw * 1.07, ph * 1.9));

			QRectF items_rect = scn.itemsBoundingRect(true);
			scn.setSceneRect(items_rect.adjusted(-20, -20, 20, 20));
			QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, scale);

			QRegion covered;

			for(auto &pg : pages)
				covered += pg.toAlignedRect();

			QVERIFY2(QRegion(items_rect.toAlignedRect()).subtracted(covered).isEmpty(),
							 qPrintable(QString("scale %1 left %2: items rect not covered").arg(scale).arg(left)));
		}
	}
}

void ObjectsSceneTest::pageCountsMatchTheGridOfPages()
{
	/* Items in three of the four corners of a 2x2 pages area: the items rect is a rectangle, so
	 * the four pages are printed and the counts are 2 x 2 */
	ObjectsScene scn;
	unsigned h = 0, v = 0;

	addBox(scn, QPointF(30, 30));
	addBox(scn, QPointF(PageW + 40, 40));
	addBox(scn, QPointF(40, PageH + 40));

	QList<QRectF> pages = scn.getPagesForPrinting(pageLayout(), h, v, 1);

	QCOMPARE(pages.size(), 4);
	QCOMPARE(h, 2u);
	QCOMPARE(v, 2u);
	QCOMPARE(h * v, static_cast<unsigned>(pages.size()));
}

void ObjectsSceneTest::defaultOverloadUsesSceneLayout()
{
	ObjectsScene scn;
	unsigned h = 0, v = 0, h2 = 0, v2 = 0;

	addBox(scn, QPointF(10, 10));
	addBox(scn, QPointF(1500, 1200));

	QList<QRectF> pages = scn.getPagesForPrinting(h, v);
	QList<QRectF> pages2 = scn.getPagesForPrinting(ObjectsScene::getPageLayout(), h2, v2, 1);

	QCOMPARE(pages, pages2);
	QCOMPARE(h, h2);
	QCOMPARE(v, v2);
}

QTEST_MAIN(ObjectsSceneTest)
#include "objectsscenetest.moc"
