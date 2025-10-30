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

#include "basetableview.h"
#include "schema.h"
#include "utilsns.h"

bool BaseTableView::hide_ext_attribs {false};
bool BaseTableView::hide_tags {false};
unsigned BaseTableView::attribs_per_page[2] { 10, 5};

BaseTableView::BaseTableView(BaseTable *base_tab) : BaseObjectView(base_tab)
{
	if(!base_tab)
		throw Exception(ErrorCode::AsgNotAllocattedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	pending_geom_update = false;
	body=new RoundedRectItem;
	body->setRoundedCorners(RoundedRectItem::BottomLeftCorner | RoundedRectItem::BottomRightCorner);

	title=new TableTitleView;
	title->setZValue(2);

	ext_attribs_body=new RoundedRectItem;
	ext_attribs_body->setRoundedCorners(RoundedRectItem::NoCorners);

	ext_attribs=new QGraphicsItemGroup;
	ext_attribs->setZValue(1);

	columns=new QGraphicsItemGroup;
	columns->setZValue(1);

	tag_item = new TextPolygonItem;
	tag_item->setZValue(3);

	obj_shadow=new RoundedRectItem;
	obj_shadow->setZValue(-1);

	obj_selection=new RoundedRectItem;
	obj_selection->setVisible(false);
	obj_selection->setZValue(4);

	attribs_toggler = new AttributesTogglerItem;
	attribs_toggler->setZValue(1);

	this->addToGroup(obj_selection);
	this->addToGroup(obj_shadow);
	this->addToGroup(columns);
	this->addToGroup(body);
	this->addToGroup(title);
	this->addToGroup(tag_item);
	this->addToGroup(ext_attribs);
	this->addToGroup(ext_attribs_body);
	this->addToGroup(attribs_toggler);

	this->setAcceptHoverEvents(true);
	sel_child_obj_view=nullptr;
	configurePlaceholder();

	sel_enabler_timer.setInterval(500);

	connect(attribs_toggler, &AttributesTogglerItem::s_collapseModeChanged, this, &BaseTableView::configureCollapsedSections);
	connect(attribs_toggler, &AttributesTogglerItem::s_paginationToggled, this, &BaseTableView::togglePagination);
	connect(attribs_toggler, &AttributesTogglerItem::s_currentPageChanged, this, &BaseTableView::configureCurrentPage);

	connect(&sel_enabler_timer, &QTimer::timeout, this, [this](){
		this->setFlag(QGraphicsItem::ItemIsSelectable, true);
	});
}

BaseTableView::~BaseTableView()
{
	this->removeFromGroup(body);
	this->removeFromGroup(title);
	this->removeFromGroup(attribs_toggler);
	this->removeFromGroup(ext_attribs_body);
	this->removeFromGroup(ext_attribs);
	this->removeFromGroup(columns);
	this->removeFromGroup(tag_item);
	delete attribs_toggler;
	delete ext_attribs_body;
	delete ext_attribs;
	delete body;
	delete title;
	delete columns;
	delete tag_item;
}

void BaseTableView::setHideExtAttributes(bool value)
{
	hide_ext_attribs=value;
}

void BaseTableView::setHideTags(bool value)
{
	hide_tags=value;
}

bool BaseTableView::isExtAttributesHidden()
{
	return hide_ext_attribs;
}

bool BaseTableView::isTagsHidden()
{
	return hide_tags;
}

QVariant BaseTableView::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change==ItemSelectedHasChanged)
	{
		this->setToolTip(this->table_tooltip);
		configureObjectSelection();
		attribs_toggler->clearButtonsSelection();
	}
	else if(change == ItemVisibleHasChanged)
	{
		if(value.toBool() && pending_geom_update)
		{
			this->configureObject();
			pending_geom_update = false;
		}
	}
	else if(change == ItemZValueHasChanged)
	{
		BaseTable *tab = dynamic_cast<BaseTable *>(getUnderlyingObject());
		tab->setZValue(this->zValue());
	}

	if(change==ItemPositionHasChanged)
		emit s_objectMoved();

	BaseObjectView::itemChange(change, value);
	return value;
}

void BaseTableView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	//Emit a signal containing the select child object if the user right-click the focused item
	if(!this->isSelected() && event->buttons()==Qt::RightButton && sel_child_obj_view)
	{
		// Avoiding clear selection when the focused child item is amongst the other selected children
		if(sel_child_obj_view->getUnderlyingObject() && !sel_child_objs.contains(sel_child_obj_view))
		{
			// Forcing the selection clearing when we right click an child object that is not selected yet
			emit s_sceneClearRequested();
			clearChildrenSelection();

			/* Deactivate the table in order not to hide the child object selection.
				 The table object is reativated when the context menu is hidden */
			this->setEnabled(false);
			emit s_popupMenuRequested(dynamic_cast<TableObject *>(sel_child_obj_view->getUnderlyingObject()));
		}
	}
	else
	{
		QPointF pnt = attribs_toggler->mapFromScene(event->scenePos());

		//If the user clicks the extended attributes toggler
		if(!this->isSelected() && event->buttons()==Qt::LeftButton && event->modifiers() == Qt::NoModifier &&
			 attribs_toggler->isVisible() && attribs_toggler->boundingRect().contains(pnt))
			attribs_toggler->setButtonSelected(pnt, true);

		/* We select children object only if the have a source object (column, constraint, trigger, etc). View references
		 * items should not be selected here because they do not have a source object */
		if(sel_child_obj_view && sel_child_obj_view->getUnderlyingObject() &&
			 event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier))
		{
			this->setFlag(QGraphicsItem::ItemIsSelectable, false);
			sel_child_obj_view->setFakeSelection(!sel_child_obj_view->hasFakeSelection());

			if(!sel_child_obj_view->hasFakeSelection())
					sel_child_objs.removeAll(sel_child_obj_view);
			else
				sel_child_objs.append(sel_child_obj_view);

			sel_child_obj_view = nullptr;
			event->ignore();
			emit s_childrenSelectionChanged();
			sel_enabler_timer.start();
		}
		else if((this->flags() & QGraphicsItem::ItemIsSelectable) == QGraphicsItem::ItemIsSelectable)
		{
			// Forcing the scene selection clearing when we right click the table itself directly
			if(event->buttons() == Qt::RightButton && !this->isSelected())
			{
				emit s_sceneClearRequested();
				this->setSelected(true);
			}

			clearChildrenSelection();
			BaseObjectView::mousePressEvent(event);
		}
	}
}

void BaseTableView::setAttributesPerPage(BaseTable::TableSection section_id, unsigned value)
{
	if(section_id > BaseTable::ExtAttribsSection)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(value > 0)
		attribs_per_page[section_id] = value;
}

unsigned BaseTableView::getAttributesPerPage(BaseTable::TableSection section_id)
{
	if(section_id > BaseTable::ExtAttribsSection)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return attribs_per_page[section_id];
}

void BaseTableView::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	if(!this->isSelected() && obj_selection->isVisible())
		obj_selection->setVisible(false);

	attribs_toggler->clearButtonsSelection();
	sel_child_obj_view=nullptr;
}

void BaseTableView::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
	/* Case the table itself is not selected shows the child selector
		at mouse position */
	if(!this->isSelected())
	{
		QList<QGraphicsItem *> items;
		double cols_height, item_idx, ext_height=0;
		QRectF rect, rect1;
		QPointF pnt = attribs_toggler->mapFromScene(event->scenePos());

		items.append(columns->childItems());

		if(!hide_ext_attribs &&
			 dynamic_cast<BaseTable *>(this->getUnderlyingObject())->getCollapseMode() == BaseTable::NotCollapsed)
		{
			items.append(ext_attribs->childItems());
			ext_height=ext_attribs->boundingRect().height();
		}

		//Calculates the default item height
		cols_height=(columns->boundingRect().height() + ext_height + (2*VertSpacing)) / static_cast<double>(items.size());

		//Calculates the item index based upon the mouse position
		rect=this->mapRectToItem(title, title->boundingRect());
		item_idx=(event->pos().y() - rect.bottom()) / cols_height;

		if(attribs_toggler->isVisible() && attribs_toggler->boundingRect().contains(pnt))
		{
			attribs_toggler->setButtonSelected(pnt);
		}
		//If the index is invalid clears the selection
		else if(item_idx < 0 || item_idx >= items.size())
		{
			this->hoverLeaveEvent(event);
			this->setToolTip(this->table_tooltip);
		}
		else if(!items.isEmpty())
		{
			TableObjectView *item=dynamic_cast<TableObjectView *>(items[static_cast<int>(item_idx)]);

			//Configures the selection with the item's dimension
			if(obj_selection->boundingRect().height() != item->boundingRect().height())
			{
				dynamic_cast<RoundedRectItem *>(obj_selection)->setBorderRadius(4);
				dynamic_cast<RoundedRectItem *>(obj_selection)->setRect(QRectF(0, 0,
																																			 title->boundingRect().width() - (2.5 * HorizSpacing),
																																			 item->boundingRect().height() - VertSpacing));
			}

			//Sets the selection position as same as item's position
			rect1=this->mapRectToItem(item, item->boundingRect());
			obj_selection->setVisible(true);
			obj_selection->setPos(QPointF(title->pos().x() + HorizSpacing, -rect1.top() + (VertSpacing/2)));

			//Stores the selected child object
			sel_child_obj_view = item;
			this->setToolTip(item->toolTip());
		}
	}
}

void BaseTableView::addConnectedRelationship(BaseRelationship *base_rel)
{
	BaseTable *tab = dynamic_cast<BaseTable *>(getUnderlyingObject());

	if(!base_rel ||
		 (base_rel &&
			base_rel->getTable(BaseRelationship::SrcTable) != tab &&
			base_rel->getTable(BaseRelationship::DstTable) != tab))
		return;

	connected_rels.push_back(base_rel);
}

void BaseTableView::removeConnectedRelationship(BaseRelationship *base_rel)
{
	connected_rels.erase(std::find(connected_rels.begin(), connected_rels.end(), base_rel));
}

int BaseTableView::getConnectedRelationshipIndex(BaseRelationship *base_rel, bool only_self_rels)
{
	std::vector<BaseRelationship *>::iterator itr;
	std::vector<BaseRelationship *> self_rels, *vet_rels = nullptr;

	if(!only_self_rels)
		vet_rels = &connected_rels;
	else
	{
		for(auto &rel : connected_rels)
		{
			if(rel->isSelfRelationship())
				self_rels.push_back(rel);
		}

		vet_rels = &self_rels;
	}

	itr = std::find(vet_rels->begin(), vet_rels->end(), base_rel);

	if(itr != vet_rels->end())
		return itr - vet_rels->begin();

	return -1;
}

unsigned BaseTableView::getConnectedRelsCount(BaseTable *src_tab, BaseTable *dst_tab)
{
	unsigned count = 0;

	for(auto &rel : connected_rels)
	{
		if((rel->getTable(BaseRelationship::SrcTable) == src_tab &&
				rel->getTable(BaseRelationship::DstTable) == dst_tab) ||
			 (rel->getTable(BaseRelationship::SrcTable) == dst_tab &&
				rel->getTable(BaseRelationship::DstTable) == src_tab))
			count++;
	}

	return count;
}

void BaseTableView::configureTag()
{
	BaseTable *tab=dynamic_cast<BaseTable *>(this->getUnderlyingObject());
	Tag *tag=tab->getTag();

	tag_item->setVisible(tag!=nullptr && !hide_tags);

	if(!hide_tags && tag)
	{
		QPolygonF pol;
		QPointF p1, p2;
		double bottom;
		QFont fnt=BaseObjectView::getFontStyle(Attributes::Tag).font();

		fnt.setPointSizeF(fnt.pointSizeF() * 0.90);
		tag_item->setFont(fnt);
		tag_item->setText(tag->getName());
		tag_item->setTextColor(BaseObjectView::getFontStyle(Attributes::Tag).foreground().color());

		p1=tag_item->getTextBoundingRect().topLeft();
		p2=tag_item->getTextBoundingRect().bottomRight();
		bottom=this->boundingRect().bottom();

		pol.append(QPointF(p1.x()- HorizSpacing, p1.y() - VertSpacing));
		pol.append(QPointF(p2.x(), p1.y() - VertSpacing));
		pol.append(QPointF(p2.x() + HorizSpacing + 5, p2.y()/2));
		pol.append(QPointF(p2.x(), p2.y() + VertSpacing));
		pol.append(QPointF(p1.x(), p2.y() + VertSpacing));
		pol.append(QPointF(p1.x()-HorizSpacing, p2.y() + VertSpacing));

		tag_item->setPolygon(pol);
		tag_item->setPen(BaseObjectView::getBorderStyle(Attributes::Tag));
		tag_item->setBrush(BaseObjectView::getFillStyle(Attributes::Tag));
		tag_item->setPos(-5, bottom - 1.5);
		tag_item->setTextPos(HorizSpacing/2, 0);
	}
}

void BaseTableView::__configureObject(double width)
{
	BaseTable *tab = dynamic_cast<BaseTable *>(getUnderlyingObject());
	QString togg_btn_color_id, togg_body_color_id;
	Tag *tag = tab->getTag();
	QBrush togg_brush, togg_btns_brush;
	QPen togg_pen, togg_btns_pen;
	double height = 0;

	prepareGeometryChange();

	if(tag)
	{
		togg_brush = tag->getFillStyle(Attributes::TableTogglerBody);
		togg_pen = tag->getElementColor(Attributes::TableTogglerBody, ColorId::BorderColor);
		togg_btns_brush = tag->getFillStyle(Attributes::TableTogglerButtons);
		togg_btns_pen = tag->getElementColor(Attributes::TableTogglerButtons, ColorId::BorderColor);
	}
	else
	{
		if(tab->getObjectType() == ObjectType::Table)
		{
			togg_btn_color_id = Attributes::TableTogglerButtons;
			togg_body_color_id = Attributes::TableTogglerBody;
		}
		else if(tab->getObjectType() == ObjectType::View)
		{
			togg_btn_color_id = Attributes::ViewTogglerButtons;
			togg_body_color_id = Attributes::ViewTogglerBody;
		}
		else
		{
			togg_btn_color_id = Attributes::ForeignTableTogglerButtons;
			togg_body_color_id = Attributes::ForeignTableTogglerBody;
		}

		togg_brush = BaseObjectView::getFillStyle(togg_body_color_id);
		togg_pen = BaseObjectView::getBorderStyle(togg_body_color_id);
		togg_btns_brush = BaseObjectView::getFillStyle(togg_btn_color_id);
		togg_btns_pen = BaseObjectView::getBorderStyle(togg_btn_color_id);
	}

	attribs_toggler->setBrush(togg_brush);
	attribs_toggler->setPen(togg_pen);
	attribs_toggler->setButtonsBrush(togg_btns_brush);
	attribs_toggler->setButtonsPen(togg_btns_pen);
	attribs_toggler->configureButtons(QRectF(0, 0, width, 12 * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor()));
	attribs_toggler->setCollapseMode(tab->getCollapseMode());

	//Set the protected icon position to the top-right on the title
	protected_icon->setPos(title->pos().x() + (2 * HorizSpacing), title->boundingRect().height() * 0.25);
	this->bounding_rect = title->boundingRect();

	body->setRoundedCorners(RoundedRectItem::NoCorners);

	height = title->boundingRect().height() + attribs_toggler->boundingRect().height() - VertSpacing;
	height += (body->isVisible() ? body->boundingRect().height() : 1);
	height += (ext_attribs_body->isVisible() ? ext_attribs_body->boundingRect().height() - VertSpacing + 1 : 0);

	this->bounding_rect.setHeight(height);

	attribs_toggler->setPos(title->pos().x(),
													height - attribs_toggler->boundingRect().height());

	this->table_tooltip =
						"`" + tab->getName(true) +
						"' (" + tab->getTypeName() + ")" +
						QString("\n%1 Id: %2").arg(UtilsNs::DataSeparator, QString::number(tab->getObjectId())) +
						tr("\n%1 Connected relationships: %2").arg(UtilsNs::DataSeparator, QString::number(this->getConnectRelsCount()));

	if(!tab->getAlias().isEmpty())
		table_tooltip += QString("\n%1 Alias: %2").arg(UtilsNs::DataSeparator, tab->getAlias());

	if(!tab->getComment().isEmpty())
		table_tooltip += QString("\n\n%1").arg(tab->getComment());

	table_tooltip = UtilsNs::formatMessage(table_tooltip);

	this->setToolTip(table_tooltip);
	this->setZValue(tab->getZValue());

	configureObjectSelection();
	configureObjectShadow();
}

double BaseTableView::calculateWidth()
{
	/* Calculating the maximum width between the title, columns, extended attributes and the attribs toggler.
	 * This width is used to set the uniform width of table */
	std::vector<double> widths = { columns->isVisible() ? columns->boundingRect().width() : 0,
														ext_attribs->isVisible() ? ext_attribs->boundingRect().width() : 0,
														attribs_toggler->isVisible() ? attribs_toggler->getButtonsWidth() : 0,
														title->boundingRect().width() };

	std::sort(widths.begin(), widths.end());
	return widths.back() + (2 * HorizSpacing);
}

int BaseTableView::getConnectRelsCount()
{
	return connected_rels.size();
}

void BaseTableView::requestRelationshipsUpdate()
{
	emit s_relUpdateRequest();
}

void BaseTableView::togglePlaceholder(bool value)
{
	BaseObjectView::togglePlaceholder(!connected_rels.empty() && value);
}

void BaseTableView::configureObjectShadow()
{
	RoundedRectItem *rect_item=dynamic_cast<RoundedRectItem *>(obj_shadow);
	rect_item->setPen(getBorderStyle(Attributes::ObjShadow));
	rect_item->setBrush(getFillStyle(Attributes::ObjShadow));
	rect_item->setRect(this->boundingRect());
	rect_item->setPos(ObjectShadowXPos, ObjectShadowYPos);

	BaseObjectView::configureObjectShadow();
}

QList<TableObjectView *> BaseTableView::getSelectedChidren()
{
	return sel_child_objs;
}

void BaseTableView::clearChildrenSelection()
{
	if(sel_child_objs.isEmpty())
		return;

	for(auto &tab_obj : sel_child_objs)
		tab_obj->setFakeSelection(false);

	sel_child_objs.clear();
	emit s_childrenSelectionChanged();
}

void BaseTableView::startGeometryUpdate()
{
	//We need to force the object to be not selectable so further calls to mousePressEvent doesn't select the object
	this->setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void BaseTableView::finishGeometryUpdate()
{
	//Updating the object's geometry to reflect the geometry change
	this->configureObject();
	obj_selection->setVisible(false);

	// Using a single shot time to restore the selectable flag
	QTimer::singleShot(300, [&]{ this->setFlag(QGraphicsItem::ItemIsSelectable, true); });

	//Updating the schema box that holds the object (if visible)
	dynamic_cast<Schema *>(this->getUnderlyingObject()->getSchema())->setModified(true);
}

bool BaseTableView::configurePaginationParams(BaseTable::TableSection section_id, unsigned total_attrs, unsigned &start_attr, unsigned &end_attr)
{
	if(section_id > BaseTable::ExtAttribsSection)
		return false;

	BaseTable *table = dynamic_cast<BaseTable *>(getUnderlyingObject());
	unsigned attr_per_page = attribs_per_page[section_id];

	start_attr = end_attr = 0;
	attribs_toggler->setPaginationEnabled(table->isPaginationEnabled());

	/* If the pagination is enabled for the table and the amount of objects is greater than the
	 * number of objects per page we configure the pagination parameter */
	if(table->isPaginationEnabled() && total_attrs > attr_per_page)
	{
		// Calculating the proportions of columns and extended attributes displayed per page
		unsigned max_pages = 0, curr_page = table->getCurrentPage(section_id);

		// Determining the maximum amount of pages
		max_pages = ceil(total_attrs / static_cast<double>(attr_per_page));

		// Validating the current page related to the maximum determined
		if(curr_page >= max_pages)
			curr_page = max_pages - 1;

		// Calculating the start and end columns/ext. attributes for the current page
		start_attr = curr_page * attr_per_page;
		end_attr = start_attr + attr_per_page;

		// Validating the determined start/end indexes avoiding the extrapolation of limits
		if(start_attr > total_attrs)
			start_attr = total_attrs;

		if(end_attr > total_attrs)
			end_attr = total_attrs;

		// Configure the attributes toggler item withe the calculated pagination parameters
		attribs_toggler->setPaginationValues(section_id, curr_page, max_pages);
		return true;
	}

	attribs_toggler->setPaginationValues(section_id, 0, 0);
	return false;
}

void BaseTableView::configureCollapsedSections(BaseTable::CollapseMode coll_mode)
{
	startGeometryUpdate();
	dynamic_cast<BaseTable *>(this->getUnderlyingObject())->setCollapseMode(coll_mode);
	finishGeometryUpdate();
	emit s_collapseModeChanged();
}

void BaseTableView::togglePagination(bool enabled)
{
	BaseTable *tab = dynamic_cast<BaseTable *>(this->getUnderlyingObject());

	startGeometryUpdate();
	tab->setPaginationEnabled(enabled);
	tab->resetCurrentPages();
	finishGeometryUpdate();
	emit s_paginationToggled();
}

void BaseTableView::configureCurrentPage(BaseTable::TableSection section_id, unsigned page)
{
	startGeometryUpdate();
	dynamic_cast<BaseTable *>(this->getUnderlyingObject())->setCurrentPage(section_id, page);
	finishGeometryUpdate();
	emit s_currentPageChanged();
}

void BaseTableView::selectRelationships()
{
	for(auto &rel : connected_rels)
		dynamic_cast<BaseObjectView *>(rel->getOverlyingObject())->setSelected(true);
}

void BaseTableView::setChildSelected(TableObject *tab_obj)
{
	if(!tab_obj)
		return;

	TableObjectView *tab_obj_view = nullptr;
	QList<QGraphicsItem *> items;

	items.append(columns->childItems());
	items.append(ext_attribs->childItems());

	for(auto &item : items)
	{
		tab_obj_view = dynamic_cast<TableObjectView *>(item);

		if(tab_obj_view && tab_obj_view->getUnderlyingObject() == tab_obj)
		{
			tab_obj_view->setFakeSelection(true);
			sel_child_objs.append(tab_obj_view);
			emit s_childrenSelectionChanged();
			break;
		}
	}
}
