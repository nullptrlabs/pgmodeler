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
#include "objectsscene.h"
#include <cmath>

CutoffItem::CutoffItem(BaseTableView *table)
{
	this->table = table;

	line = new QGraphicsLineItem;
	line->setPen(BaseObjectView::getBorderStyle(Attributes::CutoffMarker));
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

	QTextCharFormat char_fmt = BaseObjectView::getFontStyle(Attributes::CutoffMarker);
	char_fmt.setFontPointSize(char_fmt.font().pointSizeF() * 0.90);

	label->setFont(char_fmt.font());
	label->setHtml(tr("Cross rels.: <b>%1</b>").arg(rels.size()));
	label->setTextColor(char_fmt.foreground().color());

	QSizeF box_size = label->getTextBoundingRect().size() +
										 QSizeF(1.5 * BaseObjectView::HorizSpacing, 1.5 * BaseObjectView::VertSpacing);
	QRectF box_rect(QPointF(0, 0), box_size);

	// The polygon only defines the item's bounding/hit-test area -- label_bg draws the visible box
	label->setPolygon(box_rect);

	label_bg->setRect(box_rect);
	label_bg->setBrush(BaseObjectView::getFillStyle(Attributes::CutoffMarker));

	QPen pen = BaseObjectView::getBorderStyle(Attributes::CutoffMarker);
	pen.setWidthF(BaseObjectView::ObjectBorderWidth * BaseObjectView::getScreenDpiFactor());
	label_bg->setPen(pen);
	label->setTextPos(BaseObjectView::HorizSpacing * 0.75, BaseObjectView::VertSpacing * 0.75);

	updateLabelPosition();
	updateLine();
}

void CutoffItem::updateLabelPosition()
{
	prepareGeometryChange();

	QSizeF box_size = label_bg->boundingRect().size();
	QPointF dir = getOutwardDirection(),
					box_pos((dir.x() * ((box_size.width() / 2) + MaxOffsetDistance)) - (box_size.width() / 2),
									(dir.y() * ((box_size.height() / 2) + MaxOffsetDistance)) - (box_size.height() / 2));

	label->setPos(box_pos);
	label_bg->setPos(box_pos);
	configureObjectSelection();
}

QPointF CutoffItem::getClosestTablePoint(const QPointF &pos) const
{
	QRectF tab_rect = table->sceneBoundingRect();
	return { qBound(tab_rect.left(), pos.x(), tab_rect.right()),
					 qBound(tab_rect.top(), pos.y(), tab_rect.bottom()) };
}

QPointF CutoffItem::getNearestEdgeNormal(const QPointF &pos) const
{
	QRectF tab_rect = table->sceneBoundingRect();
	double dist_left = pos.x() - tab_rect.left(),
				 dist_right = tab_rect.right() - pos.x(),
				 dist_top = pos.y() - tab_rect.top(),
				 dist_bottom = tab_rect.bottom() - pos.y(),
				 min_dist = std::min({ dist_left, dist_right, dist_top, dist_bottom });

	if(min_dist == dist_left)
		return { -1, 0 };

	if(min_dist == dist_right)
		return { 1, 0 };

	if(min_dist == dist_top)
		return { 0, -1 };

	return { 0, 1 };
}

QPointF CutoffItem::getOutwardDirection() const
{
	QRectF tab_rect = table->sceneBoundingRect();
	QPointF pos = this->pos();

	// pos() inside or touching the rect: derive direction from whichever edge is nearest
	if(tab_rect.left() <= pos.x() && pos.x() <= tab_rect.right() &&
		 tab_rect.top() <= pos.y() && pos.y() <= tab_rect.bottom())
		return getNearestEdgeNormal(pos);

	QPointF closest = getClosestTablePoint(pos),
					dir = pos - closest;
	double len = std::hypot(dir.x(), dir.y());

	return dir / len;
}

QPointF CutoffItem::clampToTableOrbit(const QPointF &new_pos) const
{
	QRectF tab_rect = table->sceneBoundingRect();

	if(tab_rect.contains(new_pos))
	{
		QPointF normal = getNearestEdgeNormal(new_pos);

		if(normal.x() < 0)
			return { tab_rect.left(), new_pos.y() };

		if(normal.x() > 0)
			return { tab_rect.right(), new_pos.y() };

		if(normal.y() < 0)
			return { new_pos.x(), tab_rect.top() };

		return { new_pos.x(), tab_rect.bottom() };
	}

	QPointF closest = getClosestTablePoint(new_pos),
					delta = new_pos - closest;
	double dist = std::hypot(delta.x(), delta.y());

	if(dist > MaxOffsetDistance)
		return closest + (delta * (MaxOffsetDistance / dist));

	return new_pos;
}

void CutoffItem::updateLine()
{
	QPointF table_center = table->sceneBoundingRect().center(),
					box_center = label_bg->pos() + QPointF(label_bg->boundingRect().width() / 2,
																									label_bg->boundingRect().height() / 2);

	line->setLine(QLineF(table_center - this->pos(), box_center));

	QPen pen = line->pen();
	pen.setWidthF(getLinePenWidth());
	line->setPen(pen);
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
				line_color = BaseObjectView::getBorderStyle(Attributes::CutoffMarker).color(),
				color;

		color.setRedF((sel_color.redF() + line_color.redF()) / 2.0);
		color.setGreenF((sel_color.greenF() + line_color.greenF()) / 2.0);
		color.setBlueF((sel_color.blueF() + line_color.blueF()) / 2.0);
		color.setAlpha(255);

		pen.setColor(color);
	}
	else
		pen.setColor(BaseObjectView::getBorderStyle(Attributes::CutoffMarker).color());

	pen.setWidthF(getLinePenWidth());
	line->setPen(pen);
}

QVariant CutoffItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change == ItemSelectedHasChanged)
	{
		obj_selection->setVisible(value.toBool());
		configureLineSelection(value.toBool());

		if(value.toBool())
		{
			/* A cutoff marker's selection is exclusive: selecting one must clear every other
			 * selected item -- another cutoff or a regular object -- in this scene, and (reusing the
			 * same cross-scene reset mechanism a plain click already triggers) every other scene too.
			 * A plain click already gets this for free from Qt's own click handling, but Ctrl+click
			 * and rubber-band selection don't reset anything by design, so without this they could
			 * still produce a mixed selection, which is exactly what must never happen */
			ObjectsScene *cutoff_scene = dynamic_cast<ObjectsScene *>(this->scene());

			if(cutoff_scene)
			{
				for(auto &item : cutoff_scene->selectedItems())
				{
					if(item != this)
						item->setSelected(false);
				}

				emit cutoff_scene->s_selectionResetTriggered();
			}
		}
	}
	else if(change == ItemPositionChange)
	{
		/* Keeps the marker orbiting the table's rect, at most MaxOffsetDistance away from it --
		 * lets it be dragged freely around any side/corner, always measuring the gap along the axis
		 * that's actually open (horizontal gap when beside the table, vertical gap when above/below
		 * it, straight-line gap to the nearest corner in between), while never letting it land on
		 * top of the table itself */
		return clampToTableOrbit(value.toPointF());
	}
	else if(change == ItemPositionHasChanged)
	{
		updateLabelPosition();
		updateLine();

		/* Only an actual user drag should persist a new cutoff_pos -- updatePosition()'s own
		 * setPos() calls (construction, or the table moving/resizing) are restoring/following, not
		 * choosing a new position, and must not feed back into the model (see restoring_position's
		 * comment in the header for why that matters) */
		if(!restoring_position)
		{
			BaseTable *base_tab = dynamic_cast<BaseTable *>(table->getUnderlyingObject());
			base_tab->setCutoffPosition(this->pos() - table->getCutoffAnchorPoint());
		}
	}

	return QGraphicsItemGroup::itemChange(change, value);
}

void CutoffItem::updatePosition()
{
	BaseTable *base_tab = dynamic_cast<BaseTable *>(table->getUnderlyingObject());
	QPointF offset = base_tab->getCutoffPosition();

	if(std::isnan(offset.x()))
		offset = QPointF(0, 0);

	restoring_position = true;
	this->setPos(table->getCutoffAnchorPoint() + offset);
	restoring_position = false;

	/* Covers a pure resize, where the table's rect changes but this item's pos() doesn't --
	 * itemChange() above only runs when setPos() actually moves the item, so a resize needs both
	 * calls repeated here explicitly */
	updateLabelPosition();
	updateLine();
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

double CutoffItem::getLinePenWidth()
{
	// Adjusting the line thickness according to the screen dpi
	if(BaseObjectView::getScreenDpiFactor() > 1)
		return  BaseObjectView::ObjectBorderWidth * BaseObjectView::getScreenDpiFactor() * 1.25;

	return BaseObjectView::ObjectBorderWidth * 1.50;
}