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
\class CutoffItem
\brief Marks a table as having relationships whose other end lives in a different scene
*/

#ifndef CUTOFF_ITEM_H
#define CUTOFF_ITEM_H

#include "canvasglobal.h"
#include "baserelationship.h"
#include "textpolygonitem.h"
#include "roundedrectitem.h"
#include <QObject>
#include <QGraphicsItemGroup>
#include <QGraphicsLineItem>

class BaseTableView;

class __libcanvas CutoffItem: public QObject, public QGraphicsItemGroup {
	Q_OBJECT

	private:
		/*! \brief Default distance, in scene units, between the anchor point and the marker's box,
		 * and also the maximum distance the user can drag the marker away from the table's rect */
		static constexpr double MaxOffsetDistance = 20;

		//! \brief Table this marker is attached to
		BaseTableView *table;

		//! \brief Relationships whose other table lives in a different scene than table
		QList<BaseRelationship *> rels;

		QGraphicsLineItem *line;

		//! \brief Rounded box drawn behind label, providing its visible background
		RoundedRectItem *label_bg;

		//! \brief Text of the marker. Its own polygon stays transparent -- label_bg draws the background
		TextPolygonItem *label;

		//! \brief Shown/hidden as the item is (de)selected, mirroring BaseObjectView::obj_selection
		RoundedRectItem *obj_selection;

		/*! \brief True while updatePosition() is repositioning this item on its own (construction,
		 * or the table moving/resizing) -- itemChange(ItemPositionHasChanged) checks this to tell
		 * that kind of move apart from an actual user drag, so it only ever persists a new
		 * cutoff_pos in response to the latter. Without this, restoring a saved position (itself
		 * done via setPos(), which re-enters itemChange() like any other move) could get clamped
		 * against transient table geometry -- not yet settled this early in a model load -- and
		 * that transient, over-clamped position would silently overwrite the correct one that was
		 * actually saved */
		bool restoring_position = false;

		//! \brief Refreshes the label text/size to reflect the current relationship count
		void updateLabel();

		/*! \brief Repositions the label box along the direction from the table's rect to this
		 * item's current pos(), so the box always extends away from the table regardless of
		 * which side/corner the marker was dragged to */
		void updateLabelPosition();

		//! \brief Redraws the line connecting the table's center point to the marker box's center point
		void updateLine();

		//! \brief Closest point on the table's scene rect to an arbitrary scene position
		QPointF getClosestTablePoint(const QPointF &pos) const;

		/*! \brief Outward unit normal of the table rect's edge closest to a point known to be inside
		 * it or touching its boundary -- e.g. left edge -> (-1, 0). Shared by getOutwardDirection()
		 * and clampToTableOrbit() so both agree on which edge "owns" a given point */
		QPointF getNearestEdgeNormal(const QPointF &pos) const;

		/*! \brief Unit vector from the table's rect to this item's current pos(), used to keep the
		 * label box on the same side of the table as the point the user dragged the marker to */
		QPointF getOutwardDirection() const;

		/*! \brief Keeps a proposed position within MaxOffsetDistance of the table's rect, on the
		 * outside -- pushes it out to the nearest edge if it falls inside the rect, so the marker can
		 * only orbit the table, never sit on top of it */
		QPointF clampToTableOrbit(const QPointF &new_pos) const;

		//! \brief Resizes obj_selection to the item's current bounding rect
		void configureObjectSelection();

		//! \brief Applies (or reverts, when selected is false) the selection color blend to line's pen
		void configureLineSelection(bool selected);

		double getLinePenWidth();

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	public:
		CutoffItem(BaseTableView *table);

		void addRelationship(BaseRelationship *rel);

		void removeRelationship(BaseRelationship *rel);

		int getRelationshipsCount();

		QList<BaseRelationship *> getRelationships();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

		/*! \brief Computed fresh from line/label_bg's current geometry instead of relying on
		 * QGraphicsItemGroup's own boundingRect(), which is cached at addToGroup() time and never
		 * updated afterward -- without this override, hit-testing (selection, drag) only works
		 * over the sliver the line occupied when it was first added to the group */
		QRectF boundingRect() const override;

		//! \brief Repositions the marker relative to table's current title geometry
		void updatePosition();
};

#endif
