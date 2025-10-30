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

#include "roundedrectitem.h"

RoundedRectItem::RoundedRectItem(QGraphicsItem *parent) :  QAbstractGraphicsShapeItem(parent)
{
	radius = DefaultRadius;
	corners = AllCorners;
}

void RoundedRectItem::setBorderRadius(double radius)
{
	this->radius = radius;
	createPolygon();
}

double RoundedRectItem::getBorderRadius()
{
	return radius;
}

void RoundedRectItem::setRoundedCorners(RectCorners corners)
{
	if(corners > AllCorners)
		corners = AllCorners;

	this->corners=corners;
	createPolygon();
}

RoundedRectItem::RectCorners RoundedRectItem::getRoundedCorners()
{
	return corners;
}

bool RoundedRectItem::isCornerRounded(RectCorners corner)
{
	return (corners & corner) == corner;
}

void RoundedRectItem::setRect(const QRectF &rect)
{
	this->rect = rect;
	createPolygon();
}

QRectF RoundedRectItem::boundingRect() const
{
	return rect.isValid() ? rect : QRectF();
}

QPolygonF RoundedRectItem::getPolygon()
{
	return polygon;
}

void RoundedRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setPen(pen());
	painter->setBrush(brush());

	if(corners==NoCorners)
		painter->drawRect(rect);
	else if(corners==AllCorners)
		painter->drawRoundedRect(rect, radius, radius);
	else
		painter->drawPolygon(polygon);
}

void RoundedRectItem::createPolygon()
{
	polygon.clear();

	if(corners!=NoCorners && corners!=AllCorners && rect.isValid())
	{
		if(isCornerRounded(TopLeftCorner))
			polygon << calculatePoints(rect.topLeft() + QPointF(radius, radius), 180, 90);
		else
			polygon.append(rect.topLeft());

		if(isCornerRounded(TopRightCorner))
			polygon << calculatePoints(rect.topRight() + QPointF(-radius, radius), 90, 0);
		else
			polygon.append(rect.topRight());

		if(isCornerRounded(BottomRightCorner))
			polygon << calculatePoints(rect.bottomRight() + QPointF(-radius, -radius), 360, 270);
		else
			polygon.append(rect.bottomRight());

		if(isCornerRounded(BottomLeftCorner))
			polygon << calculatePoints(rect.bottomLeft() + QPointF(radius, -radius), 270, 180);
		else
			polygon.append(rect.bottomLeft());
	}
}

QVector<QPointF> RoundedRectItem::calculatePoints(QPointF start_pnt, double start_angle, double end_angle)
{
	QVector<QPointF> points;
	QLineF lin;
	double inc=(start_angle > end_angle ? -10 : 10), ang=start_angle;
	bool end=false;

	while(!end)
	{
		lin.setP1(start_pnt);
		lin.setLength(radius);
		lin.setAngle(ang);
		points.append(lin.p2());
		ang+=inc;

		end=((inc > 0 && ang > end_angle) ||
			 (inc < 0 && ang < end_angle));
	}

	return points;
}
