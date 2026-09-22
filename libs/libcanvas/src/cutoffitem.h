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

		/*! \brief Guards itemChange() against persisting the table's cutoff position attribute
		 * when a position change comes from updatePosition() itself, not from the user dragging
		 * the marker */
		bool repositioning;

		//! \brief Refreshes the label text/size to reflect the current relationship count
		void updateLabel();

		//! \brief Redraws the line connecting the table's center point to the marker box's center point
		void updateLine();

		//! \brief Resizes obj_selection to the item's current bounding rect
		void configureObjectSelection();

		//! \brief Applies (or reverts, when selected is false) the selection color blend to line's pen
		void configureLineSelection(bool selected);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

		/*! \brief Computed fresh from line/label_bg's current geometry instead of relying on
		 * QGraphicsItemGroup's own boundingRect(), which is cached at addToGroup() time and never
		 * updated afterward -- without this override, hit-testing (selection, drag) only works
		 * over the sliver the line occupied when it was first added to the group */
		QRectF boundingRect() const override;

	private slots:
		//! \brief Repositions the marker relative to table's current title geometry
		void updatePosition();

	public:
		CutoffItem(BaseTableView *table);

		void addRelationship(BaseRelationship *rel);

		void removeRelationship(BaseRelationship *rel);

		int getRelationshipsCount();

		QList<BaseRelationship *> getRelationships();
};

#endif
