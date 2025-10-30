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
\class AttributesTogglerItem
\brief Implements a basic table's attributes toggler/pagination used by BaseTableView.
*/

#ifndef ATTRIBUTES_TOGGLER_ITEM_H
#define ATTRIBUTES_TOGGLER_ITEM_H

#include <QObject>
#include <QGraphicsPolygonItem>
#include "roundedrectitem.h"
#include "basetable.h"

class __libcanvas AttributesTogglerItem: public QObject, public RoundedRectItem {
	Q_OBJECT

	private:
		static QPolygonF btn_polygons[7];

		//! \brief Stores the selection rectangle of the item's internal elements
		QGraphicsRectItem *sel_rect;

		//! \brief Stores the current collasping mode related to the table associated to this toggler
		BaseTable::CollapseMode collapse_mode;

		//! \brief Stores the polygonal items denoting the control buttons of the toggler
		QGraphicsPolygonItem *buttons[7];

		//! \brief Stores the selected status of the buttons
		bool btns_selected[7],

		/*! \brief Indicates if the table associated to this toggler has extended attributes
		 * This attribute changes the way some buttons are rendered */
		has_ext_attribs,

		//! \brief Indicates if the pagination is enabled for the table associated to this toggler
		pagination_enabled;

		//! \brief Stores the width of all visible buttons
		double btns_width,

		//! \brief Stores the height of all visible buttons
		btns_height;

		//! \brief Store the current page visible on the table associated to this toggler
		unsigned current_page[2],

		//! \brief Store the maximum pages allowed for the table associated to this toggler
		max_pages[2];

		//! \brief The minimum opacity factor used to fade buttons
		static constexpr double ButtonMinOpacity = 0.40;

		enum TogglerButton: unsigned {
			//! \brief Constant used to reference the attributes expand button
			AttribsExpandBtn,

			//! \brief Constant used to reference the attributes collapse button
			AttribsCollapseBtn,

			//! \brief Constant used to reference the next page button
			NextAttribsPageBtn,

			//! \brief Constant used to reference the previous page button
			PrevAttribsPageBtn,

			//! \brief Constant used to reference the next page button
			NextExtAttribsPageBtn,

			//! \brief Constant used to reference the previous page button
			PrevExtAttribsPageBtn,

			//! \brief Constant used to reference the pagination toggler button
			PaginationTogglerBtn
		};

		/*! \brief Configure the buttons visibility and opacity based upon the current values
		 * of pagination and collapse mode */
		void configureButtonsState();

		static void createButtonPolygons();

	public:
		AttributesTogglerItem(QGraphicsItem *parent = nullptr);

		~AttributesTogglerItem() override;

		/*! \brief Configures the postion and dimensions of the buttons based on the provided bounding rect
		 * If the provided rect is smaller than the total width/height of the buttons it will be assumed
		 * as bouding rect the latter dimensions */
		void configureButtons(const QRectF &rect);

		//! \brief Configures the buttons brush
		void setButtonsBrush(const QBrush &brush);

		//! \brief Configures the buttons pen
		void setButtonsPen(const QPen &pen);

		//! \brief Configures the current collapse mode of the toggler
		void setCollapseMode(BaseTable::CollapseMode coll_mode);

		/*! \brief Set a button selected (highlighted by the selection rectangle) if the provided
		 * point is within a button's bouding rect. The parameter 'clicked' indicates if the button
		 * is also clicked and not only hovered (the default behaviour) which causes addtional actions to be executed */
		void setButtonSelected(const QPointF &pnt, bool clicked = false);

		//! \brief Defines if the toggles is controlling extended attributes pagination/collapsing
		void setHasExtAttributes(bool value);

		/*! \brief Toggles the pagination. If the hide_pag_toggler is set then the button that enables/disables
		 * the pagination will be hidden permanently and the pagination automatically disabled */
		void setPaginationEnabled(bool value, bool hide_pag_toggler = false);

		/*! \brief Defines the current values of the pagination (current page and maximum allowed pages)
		 * Thes values are used to control the buttons fading when the page navigation reaches one of the limits (min/max) */
		void setPaginationValues(BaseTable::TableSection section_id, unsigned curr_page, unsigned max_page);

		//! \brief Clears the selection status of the buttons
		void clearButtonsSelection();

		//! \brief Returns the width of the area occupied by all buttons (including spacing between them)
		double getButtonsWidth();

		//! \brief Returns the height of the area occupied by all buttons (including a vertical spacing)
		double getButtonsHeight();

		//! \brief Paints the toggler and its internal componets
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

	signals:
		//! \brief Signal emitted when the current collapse mode changes (the user clicks the collpase/expand buttons)
		void s_collapseModeChanged(BaseTable::CollapseMode);

		//! \brief Signal emitted when the current page changes (the user clicks the page navigation buttons)
		void s_currentPageChanged(BaseTable::TableSection, unsigned);

		//! \brief Signal emitted when the user clicks the pagination toggler buttons
		void s_paginationToggled(bool);
};

#endif
