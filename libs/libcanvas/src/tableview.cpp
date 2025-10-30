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

#include "tableview.h"
#include "utilsns.h"

TableView::TableView(PhysicalTable *table) : BaseTableView(table)
{
	connect(table, &PhysicalTable::s_objectModified, this, &TableView::configureObject);
	this->configureObject();
}

void TableView::configureObject()
{
	PhysicalTable *table = dynamic_cast<PhysicalTable *>(this->getUnderlyingObject());

	if(!BaseGraphicObject::isUpdatesEnabled() ||
			(!pending_geom_update &&
			 !curr_hash_code.isEmpty() &&
			 (curr_hash_code == table->getHashCode())))
		return;

	curr_hash_code = table->getHashCode();

	/* If the table isn't visible we abort the current configuration
	 * and mark its geometry update as pending so in the next call to
	 * setVisible(true) the geometry can be updated (see BaseObjectView::itemChange()) */
	if(!this->isVisible())
	{
		pending_geom_update = true;
		return;
	}

	int i, count, obj_idx;
	double width=0, px=0, cy=0, old_width=0, old_height=0;
	unsigned start_col = 0, end_col = 0, start_ext = 0, end_ext = 0;
	QPen pen;
	TableObjectView *col_item=nullptr;
	QList<QGraphicsItem *> subitems;
	QList<TableObjectView *> col_items;
	TableObject *tab_obj=nullptr;
	QGraphicsItemGroup *groups[]={ columns, ext_attribs };
	RoundedRectItem *bodies[]={ body, ext_attribs_body };
	std::vector<TableObject *> tab_objs, columns, ext_tab_objs;
	QStringList atribs, tag_attribs = { Attributes::TableBody, Attributes::TableExtBody };
	Tag *tag=table->getTag();
	BaseTable::CollapseMode collapse_mode = table->getCollapseMode();
	std::vector<ObjectType> ext_types = BaseObject::getChildObjectTypes(table->getObjectType());
	bool has_col_pag = false, has_ext_pag = false;

	if(table->getObjectType() == ObjectType::Table)
		atribs.append({ Attributes::TableBody, Attributes::TableExtBody });
	else
		atribs.append({ Attributes::ForeignTableBody, Attributes::ForeignTableExtBody });

	// Clear the selected children objects vector since we'll (re)configure the whole table
	sel_child_objs.clear();

	//Configures the table title
	title->configureObject(table);

	// We store the columns in a separated vector in order to paginate them (if enabled)
	columns.assign(table->getObjectList(ObjectType::Column)->begin(),
								 table->getObjectList(ObjectType::Column)->end());

	// We store the extended attributes in a separated vector in order to paginate them (if enabled)
	for(auto &type : ext_types)
	{
		if(type == ObjectType::Column)
			continue;

		ext_tab_objs.insert(ext_tab_objs.end(),
												table->getObjectList(type)->begin(),
												table->getObjectList(type)->end());
	}

	has_col_pag = configurePaginationParams(BaseTable::AttribsSection, columns.size(), start_col, end_col);

	has_ext_pag = configurePaginationParams(BaseTable::ExtAttribsSection,
																						collapse_mode != BaseTable::ExtAttribsCollapsed ? ext_tab_objs.size() : 0,
																						start_ext, end_ext);

	attribs_toggler->setHasExtAttributes(!hide_ext_attribs && !ext_tab_objs.empty());

	px=0;
	old_width=this->bounding_rect.width();
	old_height=this->bounding_rect.height();

	for(obj_idx=0; obj_idx < 2; obj_idx++)
	{
		tab_objs.clear();

		if(obj_idx==0)
		{
			if(collapse_mode != BaseTable::AllAttribsCollapsed)
			{
				if(table->isPaginationEnabled() && has_col_pag)
					tab_objs.assign(columns.begin() + start_col, columns.begin() + end_col);
				else
					tab_objs.assign(columns.begin(), columns.end());
			}
		}
		else
		{
			if(!hide_ext_attribs && collapse_mode == BaseTable::NotCollapsed)
			{
				if(table->isPaginationEnabled() && has_ext_pag)
					tab_objs.assign(ext_tab_objs.begin() + start_ext, ext_tab_objs.begin() + end_ext);
				else
					tab_objs.assign(ext_tab_objs.begin(), ext_tab_objs.end());
			}
		}

		//Gets the subitems of the current group
		subitems=groups[obj_idx]->childItems();
		groups[obj_idx]->moveBy(-groups[obj_idx]->scenePos().x(),
														-groups[obj_idx]->scenePos().y());
		count=tab_objs.size();
		groups[obj_idx]->setVisible(count > 0);
		bodies[obj_idx]->setVisible(count > 0);

		for(i=0; i < count; i++)
		{
			tab_obj=tab_objs.at(i);

			//Reusing the subitem if it was allocated before
			if(!subitems.isEmpty() && i < subitems.size())
			{
				col_item=dynamic_cast<TableObjectView *>(subitems[i]);
				col_item->setSourceObject(tab_obj);
				col_item->configureObject();
				col_item->moveBy(-col_item->scenePos().x(),-col_item->scenePos().y());
			}
			else
				col_item=new TableObjectView(tab_obj);

			//Configures the item and set its position
			col_item->configureObject();
			col_item->moveBy(HorizSpacing, (i * col_item->boundingRect().height()) + VertSpacing);

			/* Calculates the width of the name + type of the object. This is used to align all
			the constraint labels on table */
			width=col_item->getChildObject(TableObjectView::ObjDescriptor)->boundingRect().width() +
						col_item->getChildObject(TableObjectView::NameLabel)->boundingRect().width() + (5 * HorizSpacing);

			if(px < width)
				px=width;

			col_items.push_back(col_item);
		}

		//Destroy the unused items
		i=subitems.size()-1;
		while(i > count-1)
		{
			col_item=dynamic_cast<TableObjectView *>(subitems[i]);
			groups[obj_idx]->removeFromGroup(col_item);
			delete col_item;
			i--;
		}

		//Set all items position
		while(!col_items.isEmpty())
		{
			col_item=dynamic_cast<TableObjectView *>(col_items.front());
			groups[obj_idx]->removeFromGroup(col_item);
			col_items.pop_front();

			//Positioning the type label
			col_item->setChildObjectXPos(TableObjectView::TypeLabel, px);

			//Positioning the constraints label
			col_item->setChildObjectXPos(TableObjectView::ConstrAliasLabel,
																	 px + (col_item->getChildObject(TableObjectView::TypeLabel)->boundingRect().width() * 1.05));

			groups[obj_idx]->addToGroup(col_item);
		}
	}

	width = calculateWidth();

	//Resizes the title using the new width
	title->resizeTitle(width, title->boundingRect().height());

	//Resizes the columns/extended attributes using the new width
	for(obj_idx=0; obj_idx < 2; obj_idx++)
	{
		bodies[obj_idx]->setRect(QRectF(0,0, width, groups[obj_idx]->boundingRect().height() + (2 * VertSpacing)));
		pen=this->getBorderStyle(atribs[obj_idx]);

		if(table->isPartition())
		  pen.setStyle(Qt::DashLine);

		if(!tag)
			bodies[obj_idx]->setBrush(this->getFillStyle(atribs[obj_idx]));
		else
		{
			pen.setColor(tag->getElementColor(tag_attribs[obj_idx], ColorId::BorderColor));
			bodies[obj_idx]->setBrush(tag->getFillStyle(tag_attribs[obj_idx]));
		}

		bodies[obj_idx]->setPen(pen);

		if(obj_idx==0)
			bodies[obj_idx]->setPos(title->pos().x(), title->boundingRect().height()-1);
		else
		{
			if(bodies[0]->isVisible())
				bodies[obj_idx]->setPos(title->pos().x(),
										title->boundingRect().height() +
										bodies[0]->boundingRect().height() - 2);
			else
				bodies[obj_idx]->setPos(title->pos().x(), title->boundingRect().height()-1);
		}

		groups[obj_idx]->setPos(bodies[obj_idx]->pos());

		subitems=groups[obj_idx]->childItems();
		while(!subitems.isEmpty())
		{
			col_item=dynamic_cast<TableObjectView *>(subitems.front());
			subitems.pop_front();
			col_item->setChildObjectXPos(TableObjectView::ConstrAliasLabel,
																	 width - col_item->getChildObject(TableObjectView::ConstrAliasLabel)->boundingRect().width() - (2 * HorizSpacing) - 1);

			//Generating the connection points of the columns
			if(obj_idx==0)
			{
				tab_obj=dynamic_cast<TableObject *>(col_item->getUnderlyingObject());
				cy=title->boundingRect().height() + col_item->pos().y() + (col_item->boundingRect().height()/2);
				conn_points[tab_obj].resize(2);
				conn_points[tab_obj][BaseTableView::LeftConnPoint]=QPointF(col_item->pos().x() - 1.5, cy);
				conn_points[tab_obj][BaseTableView::RightConnPoint]=QPointF(col_item->pos().x() + width - 1.5  , cy);
			}
		}
	}

	BaseTableView::__configureObject(width);

	if(table->isPartitioned())
		table_tooltip += UtilsNs::formatMessage(QString("\n%1 %2 (%3)")
																						.arg(UtilsNs::DataSeparator,
																									tr("Partitioned"), ~table->getPartitioningType()));

	if(table->isPartition())
	{
		table_tooltip += UtilsNs::formatMessage(QString("\n%1 %2 of `%3'")
																						.arg(UtilsNs::DataSeparator, tr("Partition"),
																									table->getPartitionedTable()->getSignature(true)));
		pen = attribs_toggler->pen();
		pen.setStyle(Qt::DashLine);
		attribs_toggler->setPen(pen);
	}

	BaseObjectView::__configureObject();
	configureTag();
	configureSQLDisabledInfo();

	if((old_width!=0 && this->bounding_rect.width()!=old_width) ||
			(old_height!=0 && this->bounding_rect.height()!=old_height))
		emit s_objectDimensionChanged();
	else
		requestRelationshipsUpdate();
}

QPointF TableView::getConnectionPoints(TableObject *tab_obj, ConnectionPoint conn_pnt)
{
	if(!tab_obj)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(conn_pnt > RightConnPoint)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(conn_points.count(tab_obj)==0)
		//Returns the center point in case of the connection point of the table object wasn't calculated already
		return this->getCenter();

	return conn_points[tab_obj][conn_pnt];
}
