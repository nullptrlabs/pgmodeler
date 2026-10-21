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
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <algorithm>
#include <cmath>

BezierCurveItem::BezierCurveItem(QGraphicsItem *parent) :  QGraphicsPathItem(parent)
{
	invert_cpoints = simple_curve = straight_line = false;
	stroke_dirty = true;
}

void BezierCurveItem::setPath(const QPainterPath &path)
{
	QGraphicsPathItem::setPath(path);

	/* stroke is only used for click/collision detection (contains()/collidesWithPath()), never
	 * for paint(), so recomputing it here on every reconfiguration of the line -- which can
	 * happen very often across many relationships during loading or while dragging tables --
	 * is wasted work whenever nothing actually needs to hit-test this curve in between. Defer
	 * it to the first call that does. */
	stroke_dirty = true;
}

void BezierCurveItem::updateStroke() const
{
	if(!stroke_dirty)
		return;

	QPainterPathStroker ps;
	stroke = ps.createStroke(path());
	stroke_dirty = false;
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

void BezierCurveItem::setPoints(const std::vector<QPointF> &points, double corner_radius)
{
	setPoints(points, corner_radius, {}, 0);
}

void BezierCurveItem::setPoints(const std::vector<QPointF> &points, double corner_radius,
																const std::vector<QPointF> &bumps, double bump_radius,
																double start_straight, double end_straight)
{
	this->straight_line = this->invert_cpoints = this->simple_curve = false;

	if(points.empty())
	{
		this->setPath(QPainterPath());
		return;
	}

	QPainterPath path { points.front() };

	/* Draws a straight piece of the polyline from the current position of the path to the target one.
	 * If the piece is horizontal, the arcs of the bumps lying over it are included */
	auto line_to = [&](const QPointF &target) {
		QPointF start = path.currentPosition();
		double dir = (target.x() >= start.x() ? 1 : -1), min_pos = 0, max_pos = 0;
		std::vector<double> xs;

		if(!bumps.empty() && std::fabs(target.y() - start.y()) < 0.001)
		{
			// Bumps closer than the radius to the piece ends would collide with the corners
			min_pos = std::min(start.x(), target.x()) + bump_radius;
			max_pos = std::max(start.x(), target.x()) - bump_radius;

			for(auto &bump : bumps)
			{
				if(std::fabs(bump.y() - start.y()) < 0.001 && bump.x() >= min_pos && bump.x() <= max_pos)
					xs.push_back(bump.x());
			}

			std::sort(xs.begin(), xs.end());

			if(dir < 0)
				std::reverse(xs.begin(), xs.end());
		}

		for(auto &x : xs)
		{
			path.lineTo(QPointF(x - (dir * bump_radius), start.y()));

			// Semicircle-like arch above the segment
			path.cubicTo(QPointF(x - (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									 QPointF(x + (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									 QPointF(x + (dir * bump_radius), start.y()));
		}

		path.lineTo(target);
	};

	if(points.size() > 1)
	{
		std::size_t count = points.size();

		for(std::size_t i = 1; i < count - 1; i++)
		{
			QLineF seg_in(points[i - 1], points[i]), seg_out(points[i], points[i + 1]);
			double len_in = seg_in.length(), len_out = seg_out.length();

			// Degenerate segment (coincident points): no room to round this corner, just go straight through it
			if(len_in < 0.001 || len_out < 0.001)
			{
				line_to(points[i]);
				continue;
			}

			/* Clamp the radius so the rounding on this corner never eats into more than half of
			 * either adjacent segment, which would otherwise overlap the rounding of the next/
			 * previous corner or overshoot the segment entirely on short loops. */
			double radius = std::min({ corner_radius, len_in / 2.0, len_out / 2.0 });

			// The rounding of the first/last corners can't invade the straight part reserved at the line ends
			if(i == 1)
				radius = std::min(radius, len_in - start_straight);

			if(i == count - 2)
				radius = std::min(radius, len_out - end_straight);

			// Without room for the curve the corner is kept sharp
			if(radius < 0.5)
			{
				line_to(points[i]);
				continue;
			}

			QPointF pre = points[i] - ((seg_in.p2() - seg_in.p1()) / len_in) * radius,
					post = points[i] + ((seg_out.p2() - seg_out.p1()) / len_out) * radius;

			line_to(pre);
			path.quadTo(points[i], post);
		}

		line_to(points.back());
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
	updateStroke();
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
	updateStroke();
	return stroke.intersects(path);
}
