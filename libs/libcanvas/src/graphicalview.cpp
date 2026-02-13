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

#include "graphicalview.h"
#include "utilsns.h"

GraphicalView::GraphicalView(View *view) : BaseTableView(view)
{
	connect(view, &View::s_objectModified, this, &GraphicalView::configureObject);

	columns=new QGraphicsItemGroup;
	columns->setZValue(1);
	this->addToGroup(columns);
	configurePlaceholder();
	this->configureObject();
}

void GraphicalView::configureObject()
{
	View *view = dynamic_cast<View *>(this->getUnderlyingObject());

	if(!BaseGraphicObject::isUpdatesEnabled() ||
			(!pending_geom_update && !curr_hash_code.isEmpty() && curr_hash_code == view->getHashCode()))
		return;

	curr_hash_code = view->getHashCode();

	/* If the table isn't visible we abort the current configuration
	 * and mark its geometry update as pending so in the next call to
	 * setVisible(true) the geometry can be updated (see BaseObjectView::itemChange()) */
	if(!this->isVisible())
	{
		pending_geom_update = true;
		return;
	}

	int i = 0, count = 0;
	unsigned start_col = 0, end_col = 0, start_ext = 0, end_ext = 0;
	QPen pen;
	TableObjectView *graph_ref=nullptr;
	QList<QGraphicsItem *> subitems;
	std::vector<TableObject *> tab_objs, ext_tab_objs;
	std::vector<SimpleColumn> view_cols;
	QGraphicsItemGroup *groups[]={ columns, ext_attribs };
	RoundedRectItem *bodies[]={ body, ext_attribs_body };
	QString attribs[]={ Attributes::ViewBody, Attributes::ViewExtBody },
			tag_attribs[]={ Attributes::TableBody, Attributes::TableExtBody };
	double width, type_width=0, px=0, old_width = 0, old_height;
	TableObjectView *col_item=nullptr;
	QList<TableObjectView *> col_items;
	TableObject *tab_obj=nullptr;
	Tag *tag=view->getTag();
	BaseTable::CollapseMode collapse_mode = view->getCollapseMode();
	bool has_col_pag = false, has_ext_pag = false;

	old_width = bounding_rect.width();
	old_height = bounding_rect.height();

	// Clear the selected children objects vector since we'll (re)configure the whole view
	sel_child_objs.clear();

	//Configures the view's title
	title->configureObject(view);

	for(auto &obj : view->getObjects())
		ext_tab_objs.push_back(dynamic_cast<TableObject *>(obj));

	attribs_toggler->setHasExtAttributes(!hide_ext_attribs && !ext_tab_objs.empty());

	view_cols = view->getColumns();
	has_col_pag = configurePaginationParams(BaseTable::AttribsSection, view_cols.size(), start_col, end_col);
	has_ext_pag = configurePaginationParams(BaseTable::ExtAttribsSection,
																						collapse_mode != BaseTable::ExtAttribsCollapsed ? ext_tab_objs.size() : 0,
																						start_ext, end_ext);

	//Moves the references group to the origin to be moved latter
	columns->moveBy(-columns->scenePos().x(),	-columns->scenePos().y());
	columns->setVisible(view->getCollapseMode() != BaseTable::AllAttribsCollapsed && start_col < static_cast<unsigned>(view_cols.size()));
	body->setVisible(columns->isVisible());

	if(!columns->isVisible())
	{
		for(auto &item : columns->childItems())
		{
			columns->removeFromGroup(item);
			delete item;
		}
	}
	else
	{
		std::vector<SimpleColumn> aux_view_cols;

		if(has_col_pag)
			aux_view_cols.assign(view_cols.begin() + start_col, view_cols.begin() + end_col);
		else
			aux_view_cols = view_cols;

		count = aux_view_cols.size();
		subitems=columns->childItems();

		for(i=0; i < count; i++)
		{
			//Reuses the subitem if it was allocated before
			if(!subitems.isEmpty() && i < subitems.size())
			{
				graph_ref=dynamic_cast<TableObjectView *>(subitems[i]);

				//Moves the reference to the origin to be moved latter
				graph_ref->moveBy(-graph_ref->scenePos().x(),
													-graph_ref->scenePos().y());
			}
			else
				graph_ref=new TableObjectView;

			graph_ref->configureObject(aux_view_cols[i]);
			graph_ref->moveBy(HorizSpacing, (i * graph_ref->boundingRect().height()) + VertSpacing);

			/* Calculates the width of the name + type of the object. This is used to align all
			the constraint labels on table */
			width=graph_ref->getChildObject(TableObjectView::ObjDescriptor)->boundingRect().width() +
						graph_ref->getChildObject(TableObjectView::NameLabel)->boundingRect().width() + (8 * HorizSpacing);
			if(px < width)  px=width;

			//Gets the maximum width of the column type label to align all at same horizontal position
			if(type_width < graph_ref->getChildObject(TableObjectView::TypeLabel)->boundingRect().width())
				type_width=graph_ref->getChildObject(TableObjectView::TypeLabel)->boundingRect().width() + (3 * HorizSpacing);

			col_items.push_back(graph_ref);
		}

		//Destroy the graphical references not used
		i=subitems.size()-1;
		while(i > count-1)
		{
			graph_ref=dynamic_cast<TableObjectView *>(subitems[i]);
			columns->removeFromGroup(graph_ref);
			delete graph_ref;
			i--;
		}

		//Set all items position
		while(!col_items.isEmpty())
		{
			col_item=dynamic_cast<TableObjectView *>(col_items.front());
			columns->removeFromGroup(col_item);
			col_items.pop_front();

			//Positioning the type label
			col_item->setChildObjectXPos(TableObjectView::TypeLabel, px);

			//Positioning the constraints label			
			col_item->setChildObjectXPos(TableObjectView::ConstrAliasLabel, px + type_width);
			columns->addToGroup(col_item);
		}
	}

	if(!hide_ext_attribs && view->getCollapseMode() == BaseTable::NotCollapsed)
	{
		if(view->isPaginationEnabled() && has_ext_pag)
			tab_objs.assign(ext_tab_objs.begin() + start_ext, ext_tab_objs.begin() + end_ext);
		else
			tab_objs.assign(ext_tab_objs.begin(), ext_tab_objs.end());
	}

	ext_attribs->setVisible(!tab_objs.empty() && view->getCollapseMode() == BaseTable::NotCollapsed);
	ext_attribs_body->setVisible(ext_attribs->isVisible());

	if(tab_objs.empty())
	{
		for(auto &item : ext_attribs->childItems())
		{
			ext_attribs->removeFromGroup(item);
			delete item;
		}
	}
	else
	{
		count=tab_objs.size();

		//Gets the subitems of the current group
		subitems=ext_attribs->childItems();
		ext_attribs->moveBy(-ext_attribs->scenePos().x(), -ext_attribs->scenePos().y());
		for(i=0; i < count; i++)
		{
			tab_obj=tab_objs.at(i);

			//Reusing the subitem if it was allocated before
			if(!subitems.isEmpty() && i < subitems.size())
			{
				col_item=dynamic_cast<TableObjectView *>(subitems[i]);
				col_item->setSourceObject(tab_obj);
				col_item->configureObject();
				col_item->moveBy(-col_item->scenePos().x(),
												 -col_item->scenePos().y());
			}
			else
				col_item=new TableObjectView(tab_obj);

			//Configures the item and set its position
			col_item->configureObject();
			col_item->moveBy(HorizSpacing, (i * col_item->boundingRect().height()) + VertSpacing);

			/* Calculates the width of the name + type of the object. This is used to align all
			the constraint labels on table */
			width=col_item->getChildObject(TableObjectView::ObjDescriptor)->boundingRect().width() +
						col_item->getChildObject(TableObjectView::NameLabel)->boundingRect().width() + (3 * HorizSpacing);
			if(px < width)  px=width;

			//Gets the maximum width of the column type label to align all at same horizontal position
			if(type_width < col_item->getChildObject(TableObjectView::TypeLabel)->boundingRect().width())
				type_width=col_item->getChildObject(TableObjectView::TypeLabel)->boundingRect().width() + (3 * HorizSpacing);

			col_items.push_back(col_item);
		}

		//Destroy the unused items
		i=subitems.size()-1;
		while(i > count-1)
		{
			col_item=dynamic_cast<TableObjectView *>(subitems[i]);
			ext_attribs->removeFromGroup(col_item);
			delete col_item;
			i--;
		}

		//Set all items position
		while(!col_items.isEmpty())
		{
			col_item=dynamic_cast<TableObjectView *>(col_items.front());
			ext_attribs->removeFromGroup(col_item);
			col_items.pop_front();

			//Positioning the type label
			col_item->setChildObjectXPos(TableObjectView::TypeLabel, px);

			//Positioning the constraints label
			col_item->setChildObjectXPos(TableObjectView::ConstrAliasLabel, px + type_width);
			ext_attribs->addToGroup(col_item);
		}
	}

	width = calculateWidth();

	//Resizes the title using the new width
	title->resizeTitle(width, title->boundingRect().height());

	//Resizes the columns/extended attributes using the new width
	for(int idx=0; idx < 2; idx++)
	{
		/* Configuring the brush and pen of the bodies even if they aren't visible
		 * the attributes toggler at the bottom of the view uses the color of the attributes body
		 * this will avoid the creation of a transparent toggler */
		if(!tag)
		{
			bodies[idx]->setBrush(this->getFillStyle(attribs[idx]));
			pen=this->getBorderStyle(attribs[idx]);
		}
		else
		{
			bodies[idx]->setBrush(tag->getFillStyle(tag_attribs[idx]));
			pen.setColor(tag->getElementColor(tag_attribs[idx], ColorId::BorderColor));
		}

		pen.setStyle(Qt::DashLine);
		bodies[idx]->setPen(pen);

		// We avoid the construction of the rect related to the current body item if the related group isn't visible
		if(!groups[idx]->isVisible())
			continue;

		bodies[idx]->setRect(QRectF(0,0, width, groups[idx]->boundingRect().height() + (2 * VertSpacing)));

		if(idx==0)
			bodies[idx]->setPos(title->pos().x(), title->boundingRect().height() - 1);
		else
		{
			if(bodies[0]->isVisible())
				bodies[idx]->setPos(title->pos().x(),
														title->boundingRect().height() +
														bodies[0]->boundingRect().height() - 2);
			else
				bodies[idx]->setPos(title->pos().x(), title->boundingRect().height()-1);
		}

		groups[idx]->setPos(bodies[idx]->pos());

		subitems=groups[idx]->childItems();
		while(!subitems.isEmpty())
		{
			col_item=dynamic_cast<TableObjectView *>(subitems.front());
			subitems.pop_front();
			col_item->setChildObjectXPos(TableObjectView::ConstrAliasLabel,
																	 width - col_item->getChildObject(TableObjectView::ConstrAliasLabel)
																	 ->boundingRect().width() - (2 * HorizSpacing));
		}
	}

	this->bounding_rect.setTopLeft(title->boundingRect().topLeft());
	this->bounding_rect.setWidth(title->boundingRect().width());

	BaseTableView::__configureObject(width);

	pen = attribs_toggler->pen();
	pen.setStyle(Qt::DashLine);
	attribs_toggler->setPen(pen);

	//if(!view->getAlias().isEmpty())
	//	table_tooltip += QString("\n%1 Alias: %2").arg(UtilsNs::DataSeparator, view->getAlias());

	//if(!view->getComment().isEmpty())
	//	table_tooltip += QString("\n\n%1").arg(view->getComment());

	BaseObjectView::__configureObject();
	BaseTableView::configureObjectShadow();
	BaseObjectView::configureObjectSelection();
	configureTag();
	configureSQLDisabledInfo();
	requestRelationshipsUpdate();

	if((old_width != 0 && this->bounding_rect.width() != old_width) ||
		 (old_height != 0 && this->bounding_rect.height()!= old_height))
		emit s_objectDimensionChanged();
}

