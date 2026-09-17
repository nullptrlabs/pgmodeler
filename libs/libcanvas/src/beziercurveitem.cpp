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

#include "beziercurveitem.h"
#include "baserelationship.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

BezierCurveItem::BezierCurveItem(QGraphicsItem *parent) :  QGraphicsPathItem(parent)
{
	invert_cpoints = simple_curve = straight_line = false;
}

void BezierCurveItem::setPath(const QPainterPath &path)
{
	QPainterPathStroker ps;
	QGraphicsPathItem::setPath(path);
	stroke = ps.createStroke(path);
}

void BezierCurveItem::setLine(const QLineF &line, bool simple_curve, bool invert_cpoints)
{
	QPainterPath path;

	this->straight_line = false;
	this->invert_cpoints = invert_cpoints;
	this->simple_curve = simple_curve;

	if(line.dx() == 0 || line.dy() == 0)
	{
		path = QPainterPath(line.p1());
		path.lineTo(line.p2());
		this->straight_line = true;
	}
	else
	{
		if(simple_curve)
		{
			QPointF cp1, start, end;

			start = line.p1();
			end = line.p2();

			if(!invert_cpoints)
				cp1 = QPointF(start.x(), end.y());
			else
				cp1 = QPointF(end.x(), start.y());

			path = QPainterPath(start);
			path.quadTo(cp1, end);
		}
		else
		{
			QRectF brect;

			if(!invert_cpoints)
			{
				brect.setTopLeft(line.p1());
				brect.setBottomRight(line.p2());
				path = QPainterPath(brect.topLeft());

				path.cubicTo(QPointF(brect.center().x(), brect.top()),
										 QPointF(brect.center().x(), brect.bottom()),
										 brect.bottomRight());
			}
			else
			{
				brect.setBottomLeft(line.p1());
				brect.setTopRight(line.p2());
				path = QPainterPath(brect.topRight());

				path.cubicTo(QPointF(brect.right(), brect.center().y()),
										 QPointF(brect.left(), brect.center().y()),
										 brect.bottomLeft());

			}
		}
	}

	this->setPath(path);
}

bool BezierCurveItem::isControlPointsInverted()
{
	return invert_cpoints;
}

bool BezierCurveItem::isSimpleCurve()
{
	return simple_curve;
}

bool BezierCurveItem::isStraightLine()
{
	return straight_line;
}

bool BezierCurveItem::contains(const QPointF &pnt) const
{
	return stroke.contains(pnt);
}

void BezierCurveItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	/* The curve's path is never altered here -- only whether it's stroked with antialiasing.
	 * At low zoom levels the jagged edges of a non-antialiased curve aren't perceptible, and
	 * antialiasing is one of the more expensive parts of painting it. Keeping the path itself
	 * untouched means the visible line and the stroke used for click detection never diverge. */
	bool low_lod = option->levelOfDetailFromTransform(painter->worldTransform()) < MinLodForAntialiasing;

	if(!low_lod)
	{
		QGraphicsPathItem::paint(painter, option, widget);
		return;
	}

	// The painter is shared across items painted in the same pass, so save/restore around the hint change
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);
	QGraphicsPathItem::paint(painter, option, widget);
	painter->restore();
}

bool BezierCurveItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode) const
{
	return stroke.intersects(path);
}
