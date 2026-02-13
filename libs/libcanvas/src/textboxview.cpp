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

#include "textboxview.h"

TextboxView::TextboxView(Textbox *txtbox, bool override_style) : BaseObjectView(txtbox)
{
	connect(txtbox, &Textbox::s_objectModified, this, &TextboxView::configureObject);

	text_item = new TextPolygonItem;
	this->addToGroup(text_item);

	obj_shadow=new QGraphicsPolygonItem;
	obj_shadow->setZValue(-1);
	this->addToGroup(obj_shadow);

	obj_selection=new QGraphicsPolygonItem;
	obj_selection->setVisible(false);
	obj_selection->setZValue(4);
	this->addToGroup(obj_selection);

	this->override_style=override_style;
	this->configureObject();
}

TextboxView::~TextboxView()
{
	this->removeFromGroup(text_item);
	delete text_item;
}

void TextboxView::setColorStyle(const QBrush &fill_style, const QPen &border_style)
{
	if(override_style)
	{
		text_item->setBrush(fill_style);
		text_item->setPen(border_style);
	}
}

void TextboxView::setFontStyle(const QTextCharFormat &fmt)
{
	if(override_style)
	{
		text_item->setFont(fmt.font());
		text_item->setTextColor(fmt.foreground().color());
	}
}

void TextboxView::setToolTip(const QString &tooltip)
{
	txtbox_tooltip = tooltip;
}

void TextboxView::__configureObject()
{
	Textbox *txtbox=dynamic_cast<Textbox *>(this->getUnderlyingObject());
	QTextCharFormat fmt=font_config[Attributes::Global];
	QPolygonF polygon;

	prepareGeometryChange();

	polygon.append(QPointF(0.0,0.0));
	polygon.append(QPointF(1.0,0.0));
	polygon.append(QPointF(1.0,1.0));
	polygon.append(QPointF(0.0,1.0));

	if(!override_style)
	{
		QFont font;

		text_item->setBrush(this->getFillStyle(BaseObject::getSchemaName(ObjectType::Textbox)));
		text_item->setPen(this->getBorderStyle(BaseObject::getSchemaName(ObjectType::Textbox)));

		font=fmt.font();
		font.setItalic(txtbox->getTextAttribute(Textbox::ItalicText));
		font.setBold(txtbox->getTextAttribute(Textbox::BoldText));
		font.setUnderline(txtbox->getTextAttribute(Textbox::UnderlineText));
		font.setPointSizeF(txtbox->getFontSize());

		text_item->setFont(font);
		text_item->setTextColor(txtbox->getTextColor());
	}

	text_item->setText(txtbox->getComment());
	text_item->setTextPos(HorizSpacing * (text_item->getFont().italic() ? 1.5 : 2),
												VertSpacing * (text_item->getFont().italic() ? 0.90 : 0.50));

	resizePolygon(polygon, round(text_item->getTextBoundingRect().width() + (1.5 * HorizSpacing)),
								round(text_item->getTextBoundingRect().height() + (1.5 * VertSpacing)));

	text_item->setPos(0, 0);
	text_item->setPolygon(polygon);

	this->bounding_rect.setTopLeft(text_item->boundingRect().topLeft());
	this->bounding_rect.setBottomRight(text_item->boundingRect().bottomRight());

	protected_icon->setPos(text_item->boundingRect().width(),
												 text_item->boundingRect().height());

	this->setZValue(dynamic_cast<Textbox *>(getUnderlyingObject())->getZValue());
	BaseObjectView::__configureObject();

	if(!txtbox_tooltip.isEmpty())
		this->BaseObjectView::setToolTip(txtbox_tooltip);
}

void TextboxView::configureObject()
{
	if(!BaseGraphicObject::isUpdatesEnabled())
		return;

	double old_width = bounding_rect.width(),
			old_height = bounding_rect.height();

	this->__configureObject();
	this->configureObjectShadow();
	this->configureObjectSelection();

	if((old_width != 0 && bounding_rect.width() != old_width) ||
		 (old_height != 0 && bounding_rect.height()!= old_height))
		emit s_objectDimensionChanged();
}

void TextboxView::configureObjectShadow()
{
	QGraphicsPolygonItem *pol_item=dynamic_cast<QGraphicsPolygonItem *>(obj_shadow);

	pol_item->setPen(getBorderStyle(Attributes::ObjShadow));
	pol_item->setBrush(getFillStyle(Attributes::ObjShadow));
	pol_item->setPolygon(text_item->polygon());
	pol_item->setPos(ObjectShadowXPos, ObjectShadowYPos);

	BaseObjectView::configureObjectShadow();
}

void TextboxView::configureObjectSelection()
{
	QGraphicsPolygonItem *pol_item=dynamic_cast<QGraphicsPolygonItem *>(obj_selection);

	pol_item->setPolygon(text_item->polygon());
	pol_item->setPos(0,0);
	pol_item->setBrush(this->getFillStyle(Attributes::ObjSelection));
	pol_item->setPen(this->getBorderStyle(Attributes::ObjSelection));
}

QVariant TextboxView::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change == ItemZValueHasChanged)
	{
		Textbox *txtbox = dynamic_cast<Textbox *>(getUnderlyingObject());
		txtbox->setZValue(zValue());
	}

	return BaseObjectView::itemChange(change, value);
}
