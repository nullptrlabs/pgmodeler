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

#include "cutoffitem.h"
#include "basetableview.h"
#include <cmath>

CutoffItem::CutoffItem(BaseTableView *table)
{
	this->table = table;
	repositioning = false;

	line = new QGraphicsLineItem;
	line->setLine(QLineF(0, 0, LineLength, 0));
	line->setPen(BaseObjectView::getBorderStyle(Attributes::PositionInfo));
	this->addToGroup(line);

	label_bg = new RoundedRectItem;
	label_bg->setBorderRadius(RoundedRectItem::DefaultRadius / 2);
	this->addToGroup(label_bg);

	label = new TextPolygonItem;
	label->setBrush(Qt::NoBrush);
	label->setPen(Qt::NoPen);
	this->addToGroup(label);

	obj_selection = new RoundedRectItem;
	obj_selection->setVisible(false);
	obj_selection->setZValue(1);
	this->addToGroup(obj_selection);

	this->setFlag(QGraphicsItem::ItemIsSelectable);
	this->setFlag(QGraphicsItem::ItemIsMovable);

	/* Without this flag, QGraphicsItem::setPos() takes a fast path that applies the position
	 * directly and never calls itemChange() -- itemChange(ItemPositionHasChanged) below (which
	 * persists a user drag into the table's cutoff position) would silently never run */
	this->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

	this->setZValue(-50);

	/* Unlike RelationshipView, repositioning here is a cheap setPos() call, so there's no
	 * cost-driven reason to pick only one signal depending on placeholder mode -- connecting
	 * to all three keeps the marker correct regardless of how the table's position was last
	 * committed (live move, drag-release with grid alignment, or a resize) */
	connect(table, &BaseTableView::s_objectMoved, this, &CutoffItem::updatePosition);
	connect(table, &BaseTableView::s_relUpdateRequest, this, &CutoffItem::updatePosition);
	connect(table, &BaseTableView::s_objectDimensionChanged, this, &CutoffItem::updatePosition);

	updateLabel();
	updatePosition();
}

void CutoffItem::addRelationship(BaseRelationship *rel)
{
	if(!rel || rels.contains(rel))
		return;

	rels.append(rel);
	updateLabel();
}

void CutoffItem::removeRelationship(BaseRelationship *rel)
{
	rels.removeOne(rel);
	updateLabel();
}

int CutoffItem::getRelationshipsCount()
{
	return rels.size();
}

QList<BaseRelationship *> CutoffItem::getRelationships()
{
	return rels;
}

void CutoffItem::updateLabel()
{
	// The relationship count can change label_bg's size, which changes boundingRect()'s result
	prepareGeometryChange();

	QTextCharFormat char_fmt = BaseObjectView::getFontStyle(Attributes::PositionInfo);
	char_fmt.setFontPointSize(char_fmt.font().pointSizeF() * 0.80);

	label->setFont(char_fmt.font());
	label->setText(tr("Cross rels: %1").arg(rels.size()));
	label->setTextColor(char_fmt.foreground().color());

	QSizeF box_size = label->getTextBoundingRect().size() +
										 QSizeF(1.5 * BaseObjectView::HorizSpacing, 1.5 * BaseObjectView::VertSpacing);
	QRectF box_rect(QPointF(0, 0), box_size);

	// The polygon only defines the item's bounding/hit-test area -- label_bg draws the visible box
	label->setPolygon(box_rect);

	label_bg->setRect(box_rect);
	label_bg->setBrush(BaseObjectView::getFillStyle(Attributes::PositionInfo));
	label_bg->setPen(BaseObjectView::getBorderStyle(Attributes::PositionInfo));

	QPointF box_pos(LineLength, -(box_size.height() / 2));
	label->setPos(box_pos);
	label->setTextPos(BaseObjectView::HorizSpacing * 0.75, BaseObjectView::VertSpacing * 0.75);
	label_bg->setPos(box_pos);

	configureObjectSelection();
}

void CutoffItem::configureObjectSelection()
{
	obj_selection->setRect(label_bg->boundingRect());
	obj_selection->setPos(label_bg->pos());
	obj_selection->setBorderRadius(RoundedRectItem::DefaultRadius / 2);
	obj_selection->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));
	obj_selection->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
}

void CutoffItem::configureLineSelection(bool selected)
{
	QPen pen = line->pen();

	if(selected)
	{
		QColor sel_color = BaseObjectView::getBorderStyle(Attributes::ObjSelection).color(),
				line_color = BaseObjectView::getBorderStyle(Attributes::PositionInfo).color(),
				color;

		color.setRedF((sel_color.redF() + line_color.redF()) / 2.0);
		color.setGreenF((sel_color.greenF() + line_color.greenF()) / 2.0);
		color.setBlueF((sel_color.blueF() + line_color.blueF()) / 2.0);
		color.setAlpha(255);

		pen.setColor(color);
	}
	else
		pen.setColor(BaseObjectView::getBorderStyle(Attributes::PositionInfo).color());

	line->setPen(pen);
}

QVariant CutoffItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change == ItemSelectedHasChanged)
	{
		obj_selection->setVisible(value.toBool());
		configureLineSelection(value.toBool());
	}
	else if(change == ItemPositionHasChanged && !repositioning)
	{
		BaseTable *base_tab = dynamic_cast<BaseTable *>(table->getUnderlyingObject());
		base_tab->setCutoffPosition(this->pos() - table->getCutoffAnchorPoint());
	}

	return QGraphicsItemGroup::itemChange(change, value);
}

void CutoffItem::updatePosition()
{
	BaseTable *base_tab = dynamic_cast<BaseTable *>(table->getUnderlyingObject());
	QPointF offset = base_tab->getCutoffPosition();

	if(std::isnan(offset.x()))
		offset = QPointF(0, 0);

	repositioning = true;
	this->setPos(table->getCutoffAnchorPoint() + offset);
	repositioning = false;
}

void CutoffItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	/* Same workaround as RelationshipView::paint(): QGraphicsItemGroup::paint() draws an
	 * artifact rectangle around its (often stale) itemsBoundingRect whenever the group is
	 * selected. Forcing no pen suppresses it, leaving only our own child items' visuals */
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setClipRect(option->exposedRect);
	QGraphicsItemGroup::paint(painter, option, widget);
	painter->restore();
}

QRectF CutoffItem::boundingRect() const
{
	return line->boundingRect().translated(line->pos()) |
				 label_bg->boundingRect().translated(label_bg->pos());
}
