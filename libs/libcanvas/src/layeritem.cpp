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

#include "layeritem.h"
#include "baseobjectview.h"

LayerItem::LayerItem(QGraphicsItem *parent) : QGraphicsPathItem(parent)
{

}

void LayerItem::setRects(const QList<QRectF> &rects)
{
	QPainterPath path;
	layer_rects = rects;

	for(auto &rect : rects)
		path.addRoundedRect(rect, 10, 10);

	path.setFillRule(Qt::WindingFill);
	QGraphicsPathItem::setPath(path);
}

void LayerItem::setTextAlignment(Qt::Alignment align)
{
	text_align = align;
}

void LayerItem::setText(const QString &txt)
{
	text = txt;
}

void LayerItem::setTextColor(const QColor &color)
{
	txt_color = color;
}

QColor LayerItem::getTextColor()
{
	return txt_color;
}

void LayerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	painter->save();
	QGraphicsPathItem::paint(painter, option, widget);
	painter->restore();

	if(text.isEmpty())
		return;

	QFont font = getDefaultFont();
	QFontMetrics fm(font);
	QPointF pnt;
	double dy = fm.height() * 0.72;

	painter->save();
	painter->translate(0,0);
	painter->setFont(font);
	painter->setPen(txt_color);
	painter->setBrush(txt_color);

	for(auto &brect : layer_rects)
	{
		if(text_align == Qt::AlignLeft)
			pnt.setX(brect.left() + LayerPadding);
		else
			pnt.setX(brect.right() - (LayerPadding + fm.horizontalAdvance(text)));

		pnt.setY(brect.top() + dy);
		painter->drawText(pnt, text);
	}

	painter->restore();
}

QFont LayerItem::getDefaultFont()
{
	QFont font = BaseObjectView::getFontStyle(Attributes::Global).font();

	font.setItalic(false);
	font.setPointSizeF(font.pointSizeF() * 0.725);

	return font;
}
