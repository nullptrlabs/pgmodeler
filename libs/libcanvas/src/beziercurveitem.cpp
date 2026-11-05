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
#include <QRadialGradient>
#include <algorithm>
#include <cmath>

namespace {
	//! \brief Longest chord of the polylines that replace the curves of the parts the line is painted by
	constexpr double MaxChord = 1.5;

	/* Appends to the path the polyline that follows the quadratic curve that starts at the current position of the path.
	 * The curves are flattened here, with chords short enough to look smooth, so the length of the path is exactly
	 * the length of its lines. The dashes of a dashed line are measured along the path, and the dash stroker
	 * flattens the curves in a way the length of them (QPainterPath::length()) doesn't predict. */
	void appendQuad(QPainterPath &path, const QPointF &ctrl, const QPointF &end)
	{
		QPointF start = path.currentPosition(), pnt;
		int count = std::max(6, static_cast<int>(std::ceil((QLineF(start, ctrl).length() + QLineF(ctrl, end).length()) / MaxChord)));
		double t = 0;

		for(int i = 1; i <= count; i++)
		{
			t = i / static_cast<double>(count);
			pnt = (start * ((1 - t) * (1 - t))) + (ctrl * (2 * (1 - t) * t)) + (end * (t * t));
			path.lineTo(pnt);
		}
	}

	//! \brief Same as above for a cubic curve
	void appendCubic(QPainterPath &path, const QPointF &ctrl1, const QPointF &ctrl2, const QPointF &end)
	{
		QPointF start = path.currentPosition(), pnt;
		int count = std::max(8, static_cast<int>(std::ceil((QLineF(start, ctrl1).length() + QLineF(ctrl1, ctrl2).length() +
																												QLineF(ctrl2, end).length()) / MaxChord)));
		double t = 0, u = 0;

		for(int i = 1; i <= count; i++)
		{
			t = i / static_cast<double>(count);
			u = 1 - t;
			pnt = (start * (u * u * u)) + (ctrl1 * (3 * u * u * t)) + (ctrl2 * (3 * u * t * t)) + (end * (t * t * t));
			path.lineTo(pnt);
		}
	}
}

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
																double start_straight, double end_straight,
																const std::vector<bool> &convex_bumps)
{
	this->straight_line = this->invert_cpoints = this->simple_curve = false;
	hop_arcs.clear();
	pieces.clear();

	if(points.empty())
	{
		this->setPath(QPainterPath());
		return;
	}

	QPainterPath path { points.front() }, piece { points.front() };
	std::vector<PathPiece> new_pieces;
	double total_len = 0;

	// Ends the part of the path that is being built and starts another one at the current position of the path
	auto end_piece = [&](int hop) {
		double len = piece.length();

		if(len > 0.001)
			new_pieces.push_back({ piece, total_len, hop });

		total_len += len;
		piece = QPainterPath(path.currentPosition());
	};

	/* Draws a straight piece of the polyline from the current position of the path to the target one.
	 * If the piece is horizontal, the arcs of the bumps lying over it are included */
	auto line_to = [&](const QPointF &target) {
		QPointF start = path.currentPosition();
		double dir = (target.x() >= start.x() ? 1 : -1), min_pos = 0, max_pos = 0;
		std::vector<std::pair<double, bool>> xs;

		if(!bumps.empty() && std::fabs(target.y() - start.y()) < 0.001)
		{
			// Bumps closer than the radius to the piece ends would collide with the corners
			min_pos = std::min(start.x(), target.x()) + bump_radius;
			max_pos = std::max(start.x(), target.x()) - bump_radius;

			for(size_t idx = 0; idx < bumps.size(); idx++)
			{
				if(std::fabs(bumps[idx].y() - start.y()) < 0.001 && bumps[idx].x() >= min_pos && bumps[idx].x() <= max_pos)
					xs.push_back({ bumps[idx].x(), (idx < convex_bumps.size() ? convex_bumps[idx] : false) });
			}

			std::sort(xs.begin(), xs.end());

			if(dir < 0)
				std::reverse(xs.begin(), xs.end());
		}

		for(auto &itr : xs)
		{
			double x = itr.first;
			QPainterPath arc(QPointF(x - (dir * bump_radius), start.y()));

			path.lineTo(QPointF(x - (dir * bump_radius), start.y()));
			piece.lineTo(QPointF(x - (dir * bump_radius), start.y()));
			end_piece(-1);

			// Semicircle-like arch above the segment
			path.cubicTo(QPointF(x - (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									 QPointF(x + (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									 QPointF(x + (dir * bump_radius), start.y()));

			arc.cubicTo(QPointF(x - (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									QPointF(x + (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									QPointF(x + (dir * bump_radius), start.y()));

			// The apex of the cubic curve is at 3/4 of the height of its control points
			hop_arcs.push_back({ arc, QPointF(x, start.y() - (bump_radius * 1.33 * 0.75)), bump_radius, itr.second });

			// The arc is a part of the path on its own: it's painted with its gradient
			piece = QPainterPath(arc.elementAt(0));
			appendCubic(piece, QPointF(x - (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									QPointF(x + (dir * bump_radius), start.y() - (bump_radius * 1.33)),
									QPointF(x + (dir * bump_radius), start.y()));
			end_piece(hop_arcs.size() - 1);
		}

		path.lineTo(target);
		piece.lineTo(target);
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
			appendQuad(piece, points[i], post);
		}

		line_to(points.back());
	}

	// Only the lines with hops are painted by parts
	if(!hop_arcs.empty())
	{
		end_piece(-1);
		pieces = new_pieces;
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
		// The lines with hops are painted part by part so the gradient of the hops is part of the stroke
		if(!pieces.empty())
			paintPieces(painter);
		else
			QGraphicsPathItem::paint(painter, option, widget);

		return;
	}

	// The painter is shared across items painted in the same pass, so save/restore around the hint change
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);
	QGraphicsPathItem::paint(painter, option, widget);
	painter->restore();

	// The gradients can't be noticed at this level of detail, so the line is painted as a whole
}

QRadialGradient BezierCurveItem::getHopGradient(const HopArc &arc) const
{
	constexpr int Steps = 8;
	QColor color = pen().color(), effect;
	QRadialGradient gradient(arc.apex, arc.radius * 1.3);
	double weight = 0;

	// Mixes the color of the line with another one keeping the opacity of the line
	auto mix = [&color](const QColor &other, double part) {
		return QColor::fromRgbF(color.redF() + ((other.redF() - color.redF()) * part),
														color.greenF() + ((other.greenF() - color.greenF()) * part),
														color.blueF() + ((other.blueF() - color.blueF()) * part),
														color.alphaF());
	};

	/* The gradient starts at a darker (convex arc: the line it hops over is above it) or lighter color (concave arc: it
	 * is above the other line) at the apex and fades to the color of the line. The fading follows a smooth curve (the change
	 * is slow at the apex and at the end), and the gradient is over before the ends of the arc, so it blends with the
	 * rest of the line without any visible edge */
	effect = (arc.convex ? color.darker(ConvexFactor) : mix(Qt::white, ConcaveMix));

	for(int step = 0; step <= Steps; step++)
	{
		double dist = step / static_cast<double>(Steps);

		// The weight of the effect goes from 1 at the apex to 0 at the end of the gradient (smoothstep)
		weight = 1 - (dist * dist * (3 - (2 * dist)));
		gradient.setColorAt(dist, mix(effect, weight));
	}

	return gradient;
}

void BezierCurveItem::paintPieces(QPainter *painter)
{
	QPen piece_pen;
	double width = std::max(pen().widthF(), 1.0), base_offset = pen().dashOffset();
	bool dashed = (pen().style() != Qt::SolidLine && pen().style() != Qt::NoPen);

	painter->save();
	painter->setBrush(Qt::NoBrush);

	/* Each part of the line is painted once. The dashes of a dashed line are calculated from the start of what is
	 * stroked, so each part starts its pattern where the previous part ended (the offset is in pen widths) */
	for(auto &piece : pieces)
	{
		piece_pen = pen();

		if(dashed)
			piece_pen.setDashOffset(base_offset + (piece.start_len / width));

		if(piece.hop >= 0)
			piece_pen.setBrush(QBrush(getHopGradient(hop_arcs[piece.hop])));

		painter->setPen(piece_pen);
		painter->drawPath(piece.path);
	}

	painter->restore();
}

const std::vector<BezierCurveItem::HopArc> &BezierCurveItem::getHopArcs() const
{
	return hop_arcs;
}

bool BezierCurveItem::collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode) const
{
	updateStroke();
	return stroke.intersects(path);
}
