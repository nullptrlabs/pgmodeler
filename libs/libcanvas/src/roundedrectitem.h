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
\class RoundedRectItem
\brief Represents a rounded rectangle item
*/

#ifndef ROUNDED_RECT_ITEM_H
#define ROUNDED_RECT_ITEM_H

#include "canvasglobal.h"
#include <QAbstractGraphicsShapeItem>
#include <QPainter>

class __libcanvas RoundedRectItem: public QAbstractGraphicsShapeItem {
	public:
		enum RectCorners: unsigned {
			//! \brief Implies a normal rectangle (no rounded corner)
			NoCorners=0,

			//! \brief The top-left corner will be rounded
			TopLeftCorner=2,

			//! \brief The top-right corner will be rounded
			TopRightCorner=4,

			//! \brief The Bottom-left corner will be rounded
			BottomLeftCorner=8,

			//! \brief The Bottom-right corner will be rounded
			BottomRightCorner=16,

			//! \brief All corners will be rounded
			AllCorners=31
		};

	protected:
		//! \brief The rect that defines the rounded corner rectangle item
		QRectF rect;

		//! \brief Rect border radius
		double radius;

		//! \brief Polygon used to simulate the rounded rectangle with custom rounded corners
		QPolygonF polygon;

		//! \brief Stores which corners of the rectangle must be rounded
		RectCorners corners;

		//! \brief Creates the polygon that represents the rounded rectangle
		void createPolygon();

		/*! \brief Calculates the points to simulate the rounded border. It must be informed
		the starting point and start/end angles. Based upon those parameters a line
		is configured and rotated from start_angle to end_angle and its final point
		(the rounded border point) is detected. The method returns all the calculated
		points which forms the rounded border. */
		QVector<QPointF> calculatePoints(QPointF start_pnt, double start_angle, double end_angle);

	public:
		static constexpr double DefaultRadius = 9.0;

		explicit RoundedRectItem(QGraphicsItem *parent = nullptr);

		void setBorderRadius(double radius);
		double getBorderRadius();

		void setRoundedCorners(RectCorners corners);
		RectCorners getRoundedCorners();

		//! \brief Test if a corners is configured to be rounded
		bool isCornerRounded(RectCorners corner);

		void setRect(const QRectF &rect);

		QPolygonF getPolygon();

		QRectF boundingRect() const override;

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
};

#endif

