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

/**
\ingroup libcanvas
\class BezierCurveItem
\brief Represents a simple bezier curve item used by relationships to draw curved lines
*/

#ifndef BEZIER_CURVE_ITEM_H
#define BEZIER_CURVE_ITEM_H

#include "canvasglobal.h"
#include <QGraphicsPathItem>
#include <QRadialGradient>
#include <vector>

class __libcanvas BezierCurveItem: public QGraphicsPathItem {
	public:
		/*! \brief The arcs the line makes to hop over other lines (see setPoints()). Each one is stroked with a
		 * gradient at its apex that darkens it (convex: the line crossing it is above) or lightens it (concave: the arc is
		 * above the line crossing it), which tells which of the lines is on top */
		struct HopArc {
			QPainterPath path;
			QPointF apex;
			double radius;
			bool convex;
		};

	private:
		/*! \brief Stores the stroke of the curve itself.
		 * This object is used to do a better colision detection.
		 * Computed lazily by updateStroke() -- see stroke_dirty below -- since it's only
		 * ever read from contains()/collidesWithPath(), not from paint(). */
		mutable QPainterPath stroke;

		//! \brief Indicates that stroke no longer matches path() and must be recomputed before its next use
		mutable bool stroke_dirty;

		bool invert_cpoints;

		bool simple_curve;

		bool straight_line;

		//! \brief The arcs the line makes to hop over other lines (see setPoints())
		std::vector<HopArc> hop_arcs;

		/*! \brief A part of the path painted on its own: the arcs of the hops (that are painted with a gradient) and the parts
		 * of the line between them. The line is painted part by part, each one only once, so the gradient is part of the
		 * stroke of the arc (there's no need to paint anything over the line). start_len is the length of the path before the
		 * part, which keeps the dashes of a dashed line continuous from one part to the next. hop is the index of the
		 * arc in hop_arcs or -1 if the part is not an arc. */
		struct PathPiece {
			QPainterPath path;
			double start_len;
			int hop;
		};

		/*! \brief The parts of the path (only when the line has hops, otherwise it's painted as a whole) */
		std::vector<PathPiece> pieces;

		/*! \brief Below this level of detail (see QStyleOptionGraphicsItem::levelOfDetailFromTransform())
		 * paint() skips antialiasing: at that scale the jagged edges aren't perceptible, and stroking
		 * the curve with antialiasing on is one of the more expensive parts of painting it. The curve's
		 * shape/path is never altered, so it stays consistent with the stroke used for click detection. */
		static constexpr double MinLodForAntialiasing = 0.4;

		/*! \brief How much the apex of the convex hop arcs is darkened (see QColor::darker(), in percentage), and how much
		 * the apex of the concave ones is lightened (the part, from 0 to 1, of the way from the color of the line to white) */
		static constexpr int ConvexFactor = 190;

		static constexpr double ConcaveMix = 0.28;

		//! \brief Returns the gradient used to stroke the arc: from the color of the line to a darker or lighter one at its apex
		QRadialGradient getHopGradient(const HopArc &arc) const;

		//! \brief Paints the line part by part, the arcs of the hops with their gradients
		void paintPieces(QPainter *painter);

	protected:
		void setPath(const QPainterPath &path);

		//! \brief Recomputes stroke from the current path() if stroke_dirty is set
		void updateStroke() const;

	public:
		BezierCurveItem(QGraphicsItem *parent = 0);

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

		/*! \brief Configures the curve based upon a straight line.
		 *
		 * The simple_curve param causes the curve to be drawn using the quadratic mode and one control point.
		 * Non-simple curves are cubic ones where two control points are used.
		 * See QPainterPath::quadTo and QPainterPath::cubicTo for details.
		 *
		 * The invert_cpoints param causes the control points of the curve to be inverted
		 * making the curve to be drawn inverted. */
		void setLine(const QLineF &line, bool simple_curve, bool invert_cpoints);

		/*! \brief Configures the item to draw a polyline through the given points, with each
		 * interior corner rounded off by a quadratic curve (using the corner itself as the
		 * control point) instead of a sharp angle. corner_radius is the ideal distance, along
		 * each of the two segments meeting at a corner, at which the straight segment gives way
		 * to the curve; it's automatically clamped down for corners whose adjacent segments are
		 * too short to fit it without overlapping the neighboring corner. */
		void setPoints(const std::vector<QPointF> &points, double corner_radius);

		/*! \brief Same as above but, additionally, the horizontal segments hop over the vertical lines that cross them.
		 * Each point in bumps is the position, over a horizontal segment of the polyline, where an arc with
		 * radius bump_radius is drawn above the segment. Points that are not over a horizontal segment, or
		 * that are too close to the segment's ends, are ignored. */
		void setPoints(const std::vector<QPointF> &points, double corner_radius,
									 const std::vector<QPointF> &bumps, double bump_radius,
									 double start_straight = 0, double end_straight = 0,
									 const std::vector<bool> &convex_bumps = {});

		/* The convex_bumps param tells, for each bump, if it is a convex one (the line it hops over is on top of this
		 * line) or a concave one (this line is on top). The bumps without a value (the vector is shorter) are concave. */

		/* The start_straight and end_straight params define the length of the first and last segments of the
		 * polyline that must remain straight. The first and last corners have their rounding limited, or even
		 * removed, so the curve doesn't start before that length. This is used to preserve the area occupied by
		 * the descriptors (e.g. crow's foot) at the ends of the relationship lines. */

		//! \brief Returns the arcs the line makes to hop over other lines, with the apex and the kind (convex or concave) of each one
		const std::vector<HopArc> &getHopArcs() const;

		//! \brief Returns if the curve has the control points inverted
		bool isControlPointsInverted();

		//! \brief Returns if the curve is a simple one (with only one control point)
		bool isSimpleCurve();

		//! \brief Returns if the curve object was created from a straight line (90 or 180 degrees)
		bool isStraightLine();

		//! \brief Returns if the specified point is contained by the curve (specifically, by the stroke)
		bool contains(const QPointF &pnt) const override;

		//! \brief Returns if the specified path collides with the curve (specifically, by the stroke)
		bool collidesWithPath(const QPainterPath &path, Qt::ItemSelectionMode) const override;
};

#endif

