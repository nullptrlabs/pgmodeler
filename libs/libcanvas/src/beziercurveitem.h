/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
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

class __libcanvas BezierCurveItem: public QGraphicsPathItem {
	private:
		/*! \brief Stores the stroke of the curve itself.
		 * This object is used to do a better colision detection */
		QPainterPath stroke;

		bool invert_cpoints;

		bool simple_curve;

		bool straight_line;

	protected:
		void setPath(const QPainterPath &path);

	public:
		BezierCurveItem(QGraphicsItem *parent = 0);

		/*! \brief Configures the curve based upon a straight line.
		 *
		 * The simple_curve param causes the curve to be drawn using the quadratic mode and one control point.
		 * Non-simple curves are cubic ones where two control points are used.
		 * See QPainterPath::quadTo and QPainterPath::cubicTo for details.
		 *
		 * The invert_cpoints param causes the control points of the curve to be inverted
		 * making the curve to be drawn inverted. */
		void setLine(const QLineF &line, bool simple_curve, bool invert_cpoints);

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

