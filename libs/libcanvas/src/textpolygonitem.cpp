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

#include "textpolygonitem.h"

TextPolygonItem::TextPolygonItem(QGraphicsItem *parent) : QGraphicsPolygonItem(parent)
{
	text_item = new QGraphicsTextItem;
	word_wrap = false;
}

TextPolygonItem::~TextPolygonItem()
{
	delete text_item;
}

void TextPolygonItem::setText(const QString &text)
{
	text_item->setPlainText(text);
}

void TextPolygonItem::setTextColor(const QColor &color)
{
	text_item->setDefaultTextColor(color);
}

void TextPolygonItem::setWordWrap(bool value)
{
	word_wrap = value;
}

QRectF TextPolygonItem::getTextBoundingRect()
{
	if(word_wrap)
		return boundingRect();

	return text_item->boundingRect();
}

void TextPolygonItem::setTextPos(const QPointF &pos)
{
	text_item->setPos(pos);
}

void TextPolygonItem::setTextPos(double x, double y)
{
	text_item->setPos(x, y);
}

void TextPolygonItem::setFont(const QFont &fnt)
{
	text_item->setFont(fnt);
}

QFont TextPolygonItem::getFont()
{
	return text_item->font();
}

void TextPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QGraphicsPolygonItem::paint(painter, option, widget);
	painter->translate(text_item->pos());
	text_item->setTextWidth(word_wrap ? boundingRect().width() : -1);
	text_item->paint(painter, option, widget);
}
