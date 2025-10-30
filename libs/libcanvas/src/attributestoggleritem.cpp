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

#include "attributestoggleritem.h"
#include <QApplication>
#include "baseobjectview.h"

QPolygonF AttributesTogglerItem::btn_polygons[7];

AttributesTogglerItem::AttributesTogglerItem(QGraphicsItem *parent) : RoundedRectItem(parent)
{
	createButtonPolygons();

	this->setRoundedCorners(RoundedRectItem::BottomLeftCorner | RoundedRectItem::BottomRightCorner);
	sel_rect = new QGraphicsRectItem;

	for(unsigned arr_id = 0; arr_id < 7; arr_id++)
	{
		buttons[arr_id] = new QGraphicsPolygonItem;
		buttons[arr_id]->setPolygon(btn_polygons[arr_id]);
		btns_selected[arr_id] = false;
	}

	buttons[AttribsExpandBtn]->setToolTip(tr("Expand the object's collapsed section"));
	buttons[AttribsCollapseBtn]->setToolTip(tr("Collapse the object's expanded section"));
	buttons[NextAttribsPageBtn]->setToolTip(tr("Show next attributes page"));
	buttons[PrevAttribsPageBtn]->setToolTip(tr("Show previous attributes page"));
	buttons[NextExtAttribsPageBtn]->setToolTip(tr("Show next extended attributes page"));
	buttons[PrevExtAttribsPageBtn]->setToolTip(tr("Show previous extended attributes page"));
	buttons[PaginationTogglerBtn]->setToolTip(tr("Toggle attributes pagination"));

	has_ext_attribs = false;
	pagination_enabled = false;
	collapse_mode = BaseTable::NotCollapsed;
	btns_width = btns_height = 0;

	for(unsigned idx = 0; idx < 2; idx++)
		current_page[idx] = max_pages[idx] = 0;

	configureButtonsState();
}

AttributesTogglerItem::~AttributesTogglerItem()
{
	for(auto &button : buttons)
		delete button;

	delete sel_rect;
}

void AttributesTogglerItem::setButtonsBrush(const QBrush &brush)
{
	for(auto & button : buttons)
		button->setBrush(brush);
}

void AttributesTogglerItem::setButtonsPen(const QPen &pen)
{
	for(auto & button : buttons)
		button->setPen(pen);
}

void AttributesTogglerItem::setCollapseMode(BaseTable::CollapseMode coll_mode)
{
	//Avoiding setting up extended attributes collapsed when the toggler is configured to not having extended attribs
	if(!has_ext_attribs && coll_mode == BaseTable::ExtAttribsCollapsed)
		collapse_mode = BaseTable::NotCollapsed;
	else
		collapse_mode = coll_mode;

	configureButtonsState();
}

void AttributesTogglerItem::setButtonSelected(const QPointF &pnt, bool clicked)
{
	QRectF rect;
	double h_spacing = 4 * BaseObjectView::HorizSpacing;
	unsigned coll_mode = static_cast<unsigned>(collapse_mode);
	BaseTable::TableSection section_id;

	this->setToolTip("");
	clearButtonsSelection();

	for(unsigned arr_id = 0; arr_id < 7; arr_id++)
	{
		rect.setSize(QSizeF(buttons[arr_id]->boundingRect().width() + h_spacing, this->boundingRect().height()));
		rect.moveTo(buttons[arr_id]->pos().x() - (h_spacing/2), 0);
		btns_selected[arr_id] = rect.contains(pnt) && buttons[arr_id]->isVisible();

		if(btns_selected[arr_id])
		{
			this->setToolTip(buttons[arr_id]->toolTip());

			if(clicked)
			{
				if(arr_id == AttribsExpandBtn ||
					 arr_id == AttribsCollapseBtn)
				{
					if(arr_id == AttribsExpandBtn)
						coll_mode++;
					else if(arr_id == AttribsCollapseBtn)
						coll_mode--;

					if(!has_ext_attribs && coll_mode == enum_t(BaseTable::ExtAttribsCollapsed))
						coll_mode += (arr_id == AttribsExpandBtn ? 1 : -1);

					if(coll_mode > enum_t(BaseTable::NotCollapsed))
					{
						collapse_mode = (arr_id == AttribsExpandBtn ?
															 BaseTable::NotCollapsed : BaseTable::AllAttribsCollapsed);
					}
					else
						collapse_mode = static_cast<BaseTable::CollapseMode>(coll_mode);
				}
				else if(arr_id == PaginationTogglerBtn)
				{
					pagination_enabled = !pagination_enabled;
				}
				else
				{
					if(arr_id == PrevAttribsPageBtn ||
						 arr_id == NextAttribsPageBtn)
						section_id = BaseTable::AttribsSection;
					else
						section_id = BaseTable::ExtAttribsSection;

					if(max_pages[section_id] != 0)
					{
						if(arr_id == PrevAttribsPageBtn ||
							 arr_id == PrevExtAttribsPageBtn)
							current_page[section_id]--;
						else
							current_page[section_id]++;

						if(current_page[section_id] >= max_pages[section_id])
							current_page[section_id] = (arr_id == PrevAttribsPageBtn ||
																					arr_id == PrevExtAttribsPageBtn ? 0 : max_pages[section_id] - 1);
					}
				}

				configureButtons(this->rect);
				clearButtonsSelection();
				configureButtonsState();

				if(arr_id == PaginationTogglerBtn)
					emit s_paginationToggled(pagination_enabled);
				else if(arr_id == AttribsExpandBtn ||
								arr_id == AttribsCollapseBtn)
					emit s_collapseModeChanged(collapse_mode);
				else
					emit s_currentPageChanged(section_id, current_page[section_id]);
			}
			else
			{
				//Configuring the selection rectangle if the arrows isn't clicked
				QRectF rect;
				QSizeF size = QSizeF(buttons[AttribsExpandBtn]->boundingRect().size().width() + (2 * BaseObjectView::HorizSpacing),
														 btns_height + BaseObjectView::VertSpacing);
				double px = 0, py = 0, arr_x = buttons[arr_id]->pos().x();

				rect.setSize(size);
				px = arr_x - (((arr_x + size.width()) - (arr_x + buttons[arr_id]->boundingRect().width()))/2);
				py = (this->boundingRect().size().height() - size.height())/2.5;

				sel_rect->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));
				sel_rect->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
				sel_rect->setRect(rect);
				sel_rect->setPos(px, py);
			}

			break;
		}
	}
}

void AttributesTogglerItem::configureButtonsState()
{
	buttons[AttribsExpandBtn]->setOpacity(collapse_mode == BaseTable::ExtAttribsCollapsed ||
																															 collapse_mode == BaseTable::AllAttribsCollapsed? 1 : ButtonMinOpacity);

	buttons[AttribsCollapseBtn]->setOpacity(collapse_mode == BaseTable::ExtAttribsCollapsed ||
																																 collapse_mode == BaseTable::NotCollapsed ? 1 : ButtonMinOpacity);

	buttons[PrevAttribsPageBtn]->setOpacity(max_pages[BaseTable::AttribsSection] != 0 &&
																																 current_page[BaseTable::AttribsSection] > 0 ? 1 : ButtonMinOpacity);

	buttons[NextAttribsPageBtn]->setOpacity(max_pages[BaseTable::AttribsSection] != 0 &&
																																 current_page[BaseTable::AttribsSection] < max_pages[BaseTable::AttribsSection] - 1 ? 1 : ButtonMinOpacity);

	buttons[PrevExtAttribsPageBtn]->setOpacity(has_ext_attribs &&
																																		max_pages[BaseTable::ExtAttribsSection] != 0 &&
																																		current_page[BaseTable::ExtAttribsSection] > 0 ? 1 : ButtonMinOpacity);

	buttons[NextExtAttribsPageBtn]->setOpacity(has_ext_attribs && max_pages[BaseTable::ExtAttribsSection] != 0 &&
																																		current_page[BaseTable::ExtAttribsSection] < max_pages[BaseTable::ExtAttribsSection] - 1 ? 1 : ButtonMinOpacity);

	buttons[PrevAttribsPageBtn]->setVisible(pagination_enabled);
	buttons[NextAttribsPageBtn]->setVisible(pagination_enabled);
	buttons[PrevExtAttribsPageBtn]->setVisible(pagination_enabled);
	buttons[NextExtAttribsPageBtn]->setVisible(pagination_enabled);
}

void AttributesTogglerItem::setHasExtAttributes(bool value)
{
	has_ext_attribs = value;
	configureButtonsState();
}

void AttributesTogglerItem::setPaginationEnabled(bool value, bool hide_pag_toggler)
{
	buttons[PaginationTogglerBtn]->setVisible(!hide_pag_toggler);
	pagination_enabled = value;
	configureButtons(this->boundingRect());
	configureButtonsState();
}

void AttributesTogglerItem::setPaginationValues(BaseTable::TableSection section_id, unsigned curr_page, unsigned max_page)
{
	if(!pagination_enabled || section_id > BaseTable::ExtAttribsSection)
		return;

	if(curr_page > max_page)
		current_page[section_id] = max_pages[section_id] = max_page;
	else
	{
		current_page[section_id] = curr_page;
		max_pages[section_id] = max_page;
	}
}

void AttributesTogglerItem::clearButtonsSelection()
{
	for(bool & btn_sel : btns_selected)
		btn_sel = false;

	this->update();
}

double AttributesTogglerItem::getButtonsWidth()
{
	return btns_width;
}

double AttributesTogglerItem::getButtonsHeight()
{
	return btns_height;
}

void AttributesTogglerItem::configureButtons(const QRectF &rect)
{
	double arr_width = 0, px = 0,
			h_spacing = 6 * BaseObjectView::HorizSpacing,
			height =  4 * BaseObjectView::VertSpacing;
	QRectF new_rect = rect;
	QPolygonF pol;
	double factor = BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();

	// Resizing the buttons polygons to reflect any font size change
	for(auto arr_id = 0; arr_id < 7; arr_id++)
	{
		pol = btn_polygons[arr_id];
		BaseObjectView::resizePolygon(pol, pol.boundingRect().width() * factor, pol.boundingRect().height() * factor);
		buttons[arr_id]->setPolygon(pol);
	}

	btns_height = buttons[PrevAttribsPageBtn]->boundingRect().height();
	height += btns_height;

	if(pagination_enabled)
	{
		arr_width = buttons[PrevAttribsPageBtn]->boundingRect().width() +
								buttons[NextAttribsPageBtn]->boundingRect().width() +
								buttons[PrevExtAttribsPageBtn]->boundingRect().width() +
								buttons[NextExtAttribsPageBtn]->boundingRect().width() +
								(4 * h_spacing);
	}

	arr_width += buttons[AttribsCollapseBtn]->boundingRect().width() +
							 buttons[AttribsExpandBtn]->boundingRect().width() + (2 * h_spacing);

	if(buttons[PaginationTogglerBtn]->isVisible())
		arr_width += buttons[AttribsExpandBtn]->boundingRect().width() + h_spacing;

	btns_width = arr_width;
	new_rect.setHeight(height);
	RoundedRectItem::setRect(new_rect);

	px = (new_rect.width() - arr_width + h_spacing)/2;

	if(buttons[PaginationTogglerBtn]->isVisible())
	{
		buttons[PaginationTogglerBtn]->setPos(px, (new_rect.height() -
																																			buttons[PaginationTogglerBtn]->boundingRect().height())/2);
		px += buttons[PaginationTogglerBtn]->boundingRect().width() + h_spacing;

		if(pagination_enabled)
		{
			buttons[PrevExtAttribsPageBtn]->setPos(px, (new_rect.height() -
																																				 buttons[PrevExtAttribsPageBtn]->boundingRect().height())/2);
			px += buttons[PrevExtAttribsPageBtn]->boundingRect().width() + h_spacing;

			buttons[PrevAttribsPageBtn]->setPos(px, (new_rect.height() -
																																			buttons[PrevAttribsPageBtn]->boundingRect().height())/2);
			px += buttons[PrevAttribsPageBtn]->boundingRect().width() + h_spacing;

			buttons[NextAttribsPageBtn]->setPos(px, (new_rect.height() -
																																			buttons[NextAttribsPageBtn]->boundingRect().height())/2);
			px += buttons[PrevExtAttribsPageBtn]->boundingRect().width() + h_spacing;

			buttons[NextExtAttribsPageBtn]->setPos(px, (new_rect.height() -
																																				 buttons[NextExtAttribsPageBtn]->boundingRect().height())/2);
			px += buttons[NextExtAttribsPageBtn]->boundingRect().width() + h_spacing;
		}
	}

	buttons[AttribsCollapseBtn]->setPos(px, (new_rect.height() -
																																	buttons[AttribsCollapseBtn]->boundingRect().height())/2);
	px += buttons[AttribsCollapseBtn]->boundingRect().width() + (h_spacing * 0.80);

	buttons[AttribsExpandBtn]->setPos(px, (new_rect.height() -
																																buttons[AttribsExpandBtn]->boundingRect().height())/2);
}

void AttributesTogglerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	QGraphicsItem *parent = this->parentItem();
	RoundedRectItem::paint(painter, option, widget);

	for(unsigned arr_id = 0; arr_id < 7; arr_id++)
	{
		if(!buttons[arr_id]->isVisible())
			continue;

		painter->save();
		painter->translate(buttons[arr_id]->pos());
		painter->setOpacity(buttons[arr_id]->opacity() * (parent ? parent->opacity() : 1));
		buttons[arr_id]->paint(painter, option, widget);
		painter->restore();

		// Drawing the selection rectangle over the button if it isn't faded
		if(btns_selected[arr_id] && buttons[arr_id]->opacity() > ButtonMinOpacity)
		{
			painter->save();
			painter->translate(sel_rect->pos());
			sel_rect->paint(painter, option, widget);
			painter->restore();
		}
	}
}

void AttributesTogglerItem::createButtonPolygons()
{
	if(!btn_polygons[0].isEmpty())
		return;

	QPolygonF *pol = nullptr;

	pol = &btn_polygons[PrevAttribsPageBtn];
	pol->append(QPointF(0, 5 ));
	pol->append(QPointF(8 , 0));
	pol->append(QPointF(8 , 10 ));

	pol = &btn_polygons[NextAttribsPageBtn];
	pol->append(QPointF(0, 0));
	pol->append(QPointF(8 , 5 ));
	pol->append(QPointF(0, 10 ));

	pol = &btn_polygons[PrevExtAttribsPageBtn];
	pol->append(QPointF(0, 0));
	pol->append(QPointF(2 , 0));
	pol->append(QPointF(2 , 4 ));
	pol->append(QPointF(8 , 0));
	pol->append(QPointF(8 , 10 ));
	pol->append(QPointF(2 , 6 ));
	pol->append(QPointF(2 , 10 ));
	pol->append(QPointF(0, 10 ));

	pol = &btn_polygons[NextExtAttribsPageBtn];
	pol->append(QPointF(0, 0));
	pol->append(QPointF(6 , 4 ));
	pol->append(QPointF(6 , 0 ));
	pol->append(QPointF(8 , 0));
	pol->append(QPointF(8 , 10 ));
	pol->append(QPointF(6 , 10 ));
	pol->append(QPointF(6 , 6 ));
	pol->append(QPointF(0, 10 ));

	pol = &btn_polygons[AttribsCollapseBtn];
	pol->append(QPointF(5 , 0));
	pol->append(QPointF(0, 8 ));
	pol->append(QPointF(10 , 8 ));

	pol = &btn_polygons[AttribsExpandBtn];
	pol->append(QPointF(0, 0));
	pol->append(QPointF(10 , 0));
	pol->append(QPointF(5 , 8 ));

	pol = &btn_polygons[PaginationTogglerBtn];
	pol->append(QPointF(4 , 0));
	pol->append(QPointF(8 , 4 ));
	pol->append(QPointF(4 , 8 ));
	pol->append(QPointF(0, 4 ));
}
