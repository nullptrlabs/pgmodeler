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
\class LayerItem
\brief This specialization of QGraphicsPathItem draws rects and texts on custom positions in a path item mainly to denote layer boxes in a canvas.
*/

#ifndef LAYER_ITEM_H
#define LAYER_ITEM_H

#include "canvasglobal.h"
#include <QGraphicsPathItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

class __libcanvas LayerItem : public QGraphicsPathItem {
	private:
		//! \brief Stores the text color of the item
		QColor txt_color;

		//! \brief Stores the rects used to generate the path that represents the layer
		QList<QRectF> layer_rects;

		/*! \brief Stores the text alignment of the layer. The always displayed at the top
		 * of the rectagles but can be aligned to left or right */
		Qt::Alignment text_align;

		//! \brief The text to be displayed on every rectagle that composes the layer
		QString text;

	public:
		static constexpr double LayerPadding = 10.0;

		LayerItem(QGraphicsItem *parent = nullptr);

		/*! \brief Configures the layer item by generating an internal
		 * QPainterPath that is composed by the rects in the list.
		 * The rectangles must come already with their desired dimensions configured.
		 * This method will keep untouched the rects */
		void setRects(const QList<QRectF> &rects);

		//! \brief Defines the layer text alignment (Only Qt::AlignLeft and Qt::AlignRight are accepted)
		void setTextAlignment(Qt::Alignment align);

		//! \brief The text that is displayed o every rectagle of the layer
		void setText(const QString &txt);

		void setTextColor(const QColor &color);

		QColor getTextColor(void);

		/*! \brief Forcing the non-usage of the setPath method in this class.
		 * For the correct configuration of a LayerPathItem instance, use
		 * setRects() instead */
		void setPath(const QPainterPath &) = delete;

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

		//! \brief Returns the default font settings of the layer item
		static QFont getDefaultFont();
};

#endif
