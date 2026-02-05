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

#include "styledtextboxview.h"
#include <QApplication>

StyledTextboxView::StyledTextboxView(Textbox *txtbox, bool override_style) : TextboxView(txtbox, override_style)
{
	fold = new QGraphicsPolygonItem;
	this->addToGroup(fold);
	StyledTextboxView::configureObject();
}

void StyledTextboxView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if(qApp->keyboardModifiers() == Qt::ShiftModifier &&
		 !this->getUnderlyingObject()->isProtected())
	{
		event->ignore();

		if(event->scenePos().x() > scenePos().x())
		{
			Textbox *txt = dynamic_cast<Textbox *>(getUnderlyingObject());
			double orig_width = txt->getTextWidth(),
						width = 0;

			width = event->scenePos().x() - scenePos().x();

			if(width <= DefaultWidth)
				width = orig_width <= 0 ? DefaultWidth : orig_width;

			txt->setTextWidth(width);
			configureObject();
		}
	}
	else
		TextboxView::mouseMoveEvent(event);
}

StyledTextboxView::~StyledTextboxView()
{
	this->removeFromGroup(fold);
	delete fold;
}

void StyledTextboxView::configureObject()
{
	QRectF rect;
	QPolygonF pol;
	QPointF pnt;
	RoundedRectItem rect_item;
	double old_width = bounding_rect.width(),
			old_height = bounding_rect.height();

	this->__configureObject();

	pol.append(QPointF(0,0));
	pol.append(QPointF(20,0));
	pol.append(QPointF(0,20));

	fold->setBrush(text_item->brush());
	fold->setPen(text_item->pen());
	fold->setPolygon(pol);

	QFontMetricsF fm(text_item->getFont());
	Textbox *txtbox = dynamic_cast<Textbox *>(getUnderlyingObject());

	rect = fm.boundingRect(QRectF(0,0, txtbox->getTextWidth() + (4 * HorizSpacing), 0),
													Qt::TextWordWrap, txtbox->getComment());

	if(rect.width() < DefaultWidth)
		rect.setWidth(DefaultWidth);

	txtbox->setTextWidth(rect.width());

	// Avoiding the fold element to ovelap the textbox body
	rect.setHeight(rect.height() + fold->boundingRect().height() + (2 * VertSpacing));

	rect_item.setRoundedCorners(RoundedRectItem::TopLeftCorner |
															RoundedRectItem::TopRightCorner |
															RoundedRectItem::BottomLeftCorner);

	rect_item.setRect(rect);
	pol = rect_item.getPolygon();
	resizePolygon(pol, rect.width() + fold->boundingRect().width(), rect.height());

	pnt = pol.at(20);
	pol[20] = QPointF(pnt.x(), round(pnt.y() - fold->boundingRect().height()));
	pol[21] = QPointF(round(pnt.x() - fold->boundingRect().width()), pnt.y());

	text_item->setTextPos(HorizSpacing, 0);
	text_item->setWordWrap(true);
	text_item->setPolygon(pol);

	rect = text_item->boundingRect();
	fold->setPos(rect.bottomRight() -
								QPointF(fold->boundingRect().width(), fold->boundingRect().height()));

	bounding_rect = text_item->boundingRect();
	protected_icon->setPos(fold->pos() +
													QPointF(fold->boundingRect().width()/2,
																	fold->boundingRect().height()/2));

	configureObjectShadow();
	configureObjectSelection();

	if((old_width != 0 && bounding_rect.width() != old_width) ||
		 (old_height != 0 && bounding_rect.height()!= old_height))
		emit s_objectDimensionChanged();
}
