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

#include "relationshipview.h"
#include "relationship.h"
#include "tableview.h"
#include "utilsns.h"

bool RelationshipView::hide_name_label {false};
bool RelationshipView::use_curved_lines {true};
bool RelationshipView::use_crows_foot {true};

RelationshipView::LineConnectionMode RelationshipView::line_conn_mode { RelationshipView::ConnectFkToPk };

RelationshipView::RelationshipView(BaseRelationship *rel) : BaseObjectView(rel)
{
	if(!rel)
		throw Exception(ErrorCode::AsgNotAllocattedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	for(unsigned i=BaseRelationship::SrcCardLabel; i <= BaseRelationship::RelNameLabel; i++)
	{
		if(rel->getLabel(static_cast<BaseRelationship::LabelId>(i)))
		{
			labels[i] = new TextboxView(rel->getLabel(static_cast<BaseRelationship::LabelId>(i)), true);
			labels[i]->setZValue(i == BaseRelationship::RelNameLabel ? 1 : 2);
			this->addToGroup(labels[i]);
		}
		else
			labels[i]=nullptr;
	}

	sel_object=nullptr;
	sel_object_idx=-1;
	configuring_line=false;
	using_placeholders=BaseObjectView::isPlaceholderEnabled();

	descriptor=new QGraphicsPolygonItem;
	descriptor->setZValue(0);
	this->addToGroup(descriptor);

	obj_shadow=new QGraphicsPolygonItem;
	obj_shadow->setZValue(-1);
	this->addToGroup(obj_shadow);

	obj_selection=new QGraphicsPolygonItem;
	obj_selection->setZValue(4);
	obj_selection->setVisible(false);
	this->addToGroup(obj_selection);

	tables[0]=tables[1]=nullptr;

	for(unsigned i=0; i < 2; i++)
	{
		round_cf_descriptors[i] = nullptr;
		cf_descriptors[i] = nullptr;

		line_circles[i]=new QGraphicsEllipseItem;
		line_circles[i]->setRect(QRectF(0,0,GraphicPointRadius,GraphicPointRadius));
		line_circles[i]->setZValue(0);
		line_circles[i]->setVisible(false);
		this->addToGroup(line_circles[i]);
	}

	this->setZValue(-50);
	this->configureObject();
}

RelationshipView::~RelationshipView()
{
	QGraphicsItem *item=nullptr;
	std::vector<std::vector<QGraphicsLineItem *> *> rel_lines = { &lines, &fk_lines, &pk_lines, &src_cf_lines, &dst_cf_lines };

	while(!curves.empty())
	{
		this->removeFromGroup(curves.back());
		item = curves.back();
		curves.pop_back();
		delete item;
	}

	for(auto & line_circle : line_circles)
	{
		this->removeFromGroup(line_circle);
		delete line_circle;
	}

	for(auto & label : labels)
	{
		if(label)
		{
			this->removeFromGroup(label);
			delete label;
		}
	}

	while(!attributes.empty())
	{
		item=attributes.back();
		this->removeFromGroup(item);
		attributes.pop_back();
		delete item;
	}

	for(auto &lines : rel_lines)
	{
		while(!lines->empty())
		{
			item = lines->back();
			this->removeFromGroup(item);
			lines->pop_back();
			delete item;
		}
	}

	this->removeFromGroup(descriptor);
	delete descriptor;

	for(auto & cf_descriptor : cf_descriptors)
	{
		if(!cf_descriptor)
			continue;
		
		for(auto &item : cf_descriptor->childItems())
		{
			cf_descriptor->removeFromGroup(item);
			this->removeFromGroup(item);
			delete item;
		}

		this->removeFromGroup(cf_descriptor);
		delete cf_descriptor;
	}
}

void RelationshipView::setHideNameLabel(bool value)
{
	hide_name_label=value;
}

bool RelationshipView::isNameLabelHidden()
{
	return hide_name_label;
}

void RelationshipView::setCurvedLines(bool value)
{
	use_curved_lines = value;
}

bool RelationshipView::isCurvedLines()
{
	return use_curved_lines;
}

void RelationshipView::setCrowsFoot(bool value)
{
	use_crows_foot = value;

	if(value)
		line_conn_mode=RelationshipView::ConnectTableEdges;
}

bool RelationshipView::isCrowsFoot()
{
	return use_crows_foot;
}

void RelationshipView::setLineConnectionMode(LineConnectionMode mode)
{
	if(use_crows_foot)
		line_conn_mode=ConnectTableEdges;
	else
	{
		if(mode > ConnectTableEdges)
			mode=ConnectTableEdges;

		line_conn_mode=mode;
	}
}

unsigned RelationshipView::getLineConnectinMode()
{
	return line_conn_mode;
}

QPointF RelationshipView::getConnectionPoint(BaseRelationship::TableId table_idx)
{
	if(table_idx > 2)
		throw Exception(ErrorCode::RefElementInvalidIndex ,PGM_FUNC,PGM_FILE,PGM_LINE);

	return conn_points[table_idx];
}

BaseRelationship *RelationshipView::getUnderlyingObject()
{
	return dynamic_cast<BaseRelationship *>(this->BaseObjectView::getUnderlyingObject());
}

TextboxView *RelationshipView::getLabel(BaseRelationship::LabelId lab_idx)
{
	if(lab_idx > BaseRelationship::RelNameLabel)
		return nullptr;

	return labels[lab_idx];
}

bool RelationshipView::isTableVisible(BaseRelationship::TableId table_idx)
{
	if(table_idx > 2)
		throw Exception(ErrorCode::RefElementInvalidIndex ,PGM_FUNC,PGM_FILE,PGM_LINE);

	return tables[table_idx]->isVisible();
}

QVariant RelationshipView::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change==ItemPositionChange)
	{
		this->setFlag(QGraphicsItem::ItemIsMovable, false);
	}
	else if(change==ItemSelectedHasChanged)
	{
		unsigned i, count;
		QPen pen;
		QColor color, line_color=this->getUnderlyingObject()->getCustomColor();
		std::vector<QGraphicsLineItem *> rel_lines;

		this->setSelectionOrder(value.toBool());
		pos_info_item->setVisible(value.toBool());
		obj_selection->setVisible(value.toBool() && descriptor->isVisible());
		this->configurePositionInfo();

		for(i=0; i < 3; i++)
		{
			if(labels[i])
				labels[i]->itemChange(change, value);
		}

		//Show tha graphical points if 'value' is true
		count=graph_points.size();
		for(i=0; i < count; i++)
			graph_points[i]->setVisible(value.toBool());

		//Alter the relationship line color when it is selected
		if(line_color==Qt::transparent)
			line_color=BaseObjectView::getBorderStyle(Attributes::Relationship).color();

		if(value.toBool())
		{
			QColor cor1=BaseObjectView::getBorderStyle(Attributes::ObjSelection).color(),
					cor2=line_color;

			color.setRedF((cor1.redF() + cor2.greenF())/2.0);
			color.setGreenF((cor1.greenF() + cor2.greenF())/2.0);
			color.setBlueF((cor1.blueF() + cor2.blueF())/2.0);
			color.setAlphaF((cor1.alphaF() + cor2.alphaF())/2.0);
		}
		else
			color=line_color;

		color.setAlpha(255);
		rel_lines=lines;
		rel_lines.insert(rel_lines.end(), fk_lines.begin(), fk_lines.end());
		rel_lines.insert(rel_lines.end(), pk_lines.begin(), pk_lines.end());

		for(auto &lin : rel_lines)
		{
			pen=lin->pen();
			pen.setColor(color);
			lin->setPen(pen);
		}

		for(auto &curve : curves)
		{
			pen = curve->pen();
			pen.setColor(color);
			curve->setPen(pen);
		}

		//If the crow's foot descriptors are allocated we change their border color
		if(cf_descriptors[0] != nullptr)
		{
			std::vector<QGraphicsLineItem *> lines;
			QVector<QGradientStop> grad_stops = descriptor->brush().gradient()->stops();
			QColor sel_color = BaseObjectView::getBorderStyle(Attributes::ObjSelection).color();
			QLinearGradient grad(QPointF(0,0),QPointF(0,1));
			int color_id = 0;

			grad.setCoordinateMode(QGradient::ObjectBoundingMode);
			lines.assign(src_cf_lines.begin(), src_cf_lines.end());
			lines.insert(lines.end(), dst_cf_lines.begin(), dst_cf_lines.end());

			for(auto &line : lines)
				line->setPen(pen);

			for(auto &round_cf_descriptor : round_cf_descriptors)
			{
				/* If we have a circle that describes optional cardinality we should
				 * merge its color with the object selection color in order to simulate the
				 * that the descriptor is selected as well */
				if(value.toBool())
				{
					color_id = 0;
					for(auto &stop : grad_stops)
					{
						color = stop.second;
						color.setRedF((color.redF() + sel_color.greenF())/2.0);
						color.setGreenF((color.greenF() + sel_color.greenF())/2.0);
						color.setBlueF((color.blueF() + sel_color.blueF())/2.0);
						grad.setColorAt(color_id++, color);
					}

					round_cf_descriptor->setBrush(grad);
				}
				else
					round_cf_descriptor->setBrush(descriptor->brush());

				if(this->getUnderlyingObject()->getRelationshipType() == BaseRelationship::RelationshipFk)
					pen.setStyle(Qt::DashLine);

				round_cf_descriptor->setPen(pen);
			}
		}

		//Shows/hides the attribute's selection
		count=attributes.size();
		for(i=0; i < count; i++)
			attributes[i]->childItems().at(3)->setVisible(value.toBool());

		emit s_objectSelected(dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject()),	value.toBool());
	}

	return value;
}

void RelationshipView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	//Marks the flag ItemIsMovable in order to permit the relationship to be selected
	this->setFlag(QGraphicsItem::ItemIsMovable);
	BaseObjectView::mousePressEvent(event);
	this->setFlag(QGraphicsItem::ItemIsMovable, false);

	if(!this->getUnderlyingObject()->isProtected())
	{
		BaseRelationship *base_rel=this->getUnderlyingObject();

		//Resets the labels position when mid-button is pressed
		if(event->buttons()==Qt::LeftButton && event->modifiers()==(Qt::AltModifier | Qt::ShiftModifier))
		{
			base_rel->resetLabelsDistance();
			this->configureLabels();
		}
		else if(event->modifiers()==Qt::ShiftModifier)
		{
			QRectF rect;
			unsigned i = 0, count = 0;
			bool pnt_rem=false;
			std::vector<QPointF> points = base_rel->getPoints();

			if(!base_rel->isSelfRelationship())
			{
				//When the user press SHIFT and clicks on a point this is removed
				if(event->buttons()==Qt::LeftButton)
				{
					emit s_relationshipModified(base_rel);

					count=graph_points.size();
					for(i=0; i < count; i++)
					{
						//Calculating the graphical point rect
						rect.setTopLeft(graph_points[i]->pos());
						rect.setSize(graph_points[i]->boundingRect().size());

						//Case the mouse pos is inside the point rect
						if(rect.contains(event->pos()))
						{
							points.erase(points.begin()+i);
							base_rel->setPoints(points);
							this->configureLine();
							pnt_rem=true;
							break;
						}
					}

										/* Create a path stroke based on the relationship lines so
						 when the user press SHIFT and clicks next to the line (based on a threshold)
						 a new point is inserted */
					QPainterPathStroker ps;
					QPainterPath stroke;
					ps.setWidth(LineSelStrokeWidth);

					count = lines.size();
					for(i=0; i < count && !pnt_rem; i++)
					{
						if(use_curved_lines)
							stroke = ps.createStroke(curves[i]->shape());
						else
							stroke = ps.createStroke(lines[i]->shape());

						if(stroke.contains(event->pos()))
						{
							//Inserts the point to the line
							if(i >= points.size())
								points.push_back(event->pos());
							else
								points.insert(points.begin() + i, event->pos());

							base_rel->setPoints(points);
							this->configureLine();

							break;
						}
					}
				}
			}
		}
		//Clicking only with the left button, checks if some child object can be selected
		else if(event->button()==Qt::LeftButton)
		{
			QRectF rect;
			unsigned count, i;

			//Checks if there is some point selected
			count=graph_points.size();
			for(i=0; i < count && !sel_object; i++)
			{
				rect.setTopLeft(graph_points[i]->pos());
				rect.setSize(graph_points[i]->boundingRect().size());

				if(rect.contains(event->pos()))
				{
					sel_object=graph_points[i];
					sel_object_idx=i;
				}
			}

			//Checks if there is some label selected
			for(i=0; i < 3 && !sel_object; i++)
			{
				if(labels[i])
				{
					rect.setTopLeft(labels[i]->pos());
					rect.setSize(labels[i]->boundingRect().size());

					if(rect.contains(event->pos()))
					{
						sel_object=labels[i];
						sel_object_idx=i;
					}
				}
			}
		}
	}
}

void RelationshipView::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if(this->isSelected() && event->buttons()==Qt::LeftButton &&
			!this->getUnderlyingObject()->isProtected())
	{
		if(dynamic_cast<QGraphicsPolygonItem *>(sel_object))
		{
			QRectF brect = QRectF(tables[0]->pos() - QPointF(20,20),
														tables[0]->pos() + QPointF(tables[0]->boundingRect().width() + 20, tables[0]->boundingRect().height()  + 20)),
					brect1 = QRectF(tables[1]->pos() - QPointF(20,20),
													tables[1]->pos() + QPointF(tables[1]->boundingRect().width() + 20, tables[1]->boundingRect().height() + 20));

			//We only include the point if it is not inside the tables' bounding rect
			if(!brect.contains(event->pos()) && !brect1.contains(event->pos()))
			{
				BaseRelationship *rel_base=this->getUnderlyingObject();
				std::vector<QPointF> points=rel_base->getPoints();

				points[sel_object_idx]=event->pos();
				rel_base->setPoints(points);
				this->configureLine();
			}
		}
		else if(dynamic_cast<TextboxView *>(sel_object))
		{
			sel_object->setPos(event->pos());
			configureBoundingRect();
		}
	}

	BaseObjectView::mouseMoveEvent(event);
}

QPainterPath RelationshipView::shape() const
{
	return rel_shape;
}

void RelationshipView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	BaseRelationship *base_rel=this->getUnderlyingObject();

	if(event->button() == Qt::LeftButton)
	{
		if(dynamic_cast<TextboxView *>(sel_object))
		{
			//Calculates the displacement of the label from the initial pos to the current
			base_rel->setLabelDistance(static_cast<BaseRelationship::LabelId>(sel_object_idx),
																 QPointF(sel_object->pos() - labels_ini_pos[sel_object_idx]));
		}

		sel_object_idx = -1;
		sel_object = nullptr;
	}

	BaseObjectView::mouseReleaseEvent(event);
}

void RelationshipView::disconnectTables()
{
	if(tables[0] && tables[1])
	{
		BaseRelationship *rel_base=this->getUnderlyingObject();

		tables[0]->removeConnectedRelationship(rel_base);

		if(!rel_base->isSelfRelationship())
			tables[1]->removeConnectedRelationship(rel_base);

		for(auto & table : tables)
		{
			table->disconnect(this);
			table = nullptr;
		}
	}
}

void RelationshipView::connectTables()
{
	if(!tables[0] || !tables[1])
		return;

	for(auto & table : tables)
	{
		table->disconnect(this);

		if(BaseObjectView::isPlaceholderEnabled())
			connect(table, &BaseTableView::s_relUpdateRequest, this, &RelationshipView::configureLine);
		else
			connect(table, &BaseTableView::s_objectMoved, this, &RelationshipView::configureLine);

		connect(table, &BaseTableView::s_objectDimensionChanged, this, &RelationshipView::configureLine);
	}
}

void RelationshipView::configureObject()
{
	BaseRelationship *rel_base = this->getUnderlyingObject();
	BaseRelationship::TableId src_tab = BaseRelationship::SrcTable,
			dst_tab = BaseRelationship::DstTable;

	tables[src_tab] =
		dynamic_cast<BaseTableView *>(rel_base->getTable(src_tab)->getOverlyingObject());

	tables[dst_tab] =
		dynamic_cast<BaseTableView *>(rel_base->getTable(dst_tab)->getOverlyingObject());

	tables[src_tab]->addConnectedRelationship(rel_base);

	if(!rel_base->isSelfRelationship())
		tables[dst_tab]->addConnectedRelationship(rel_base);

	configureLine();
	connectTables();

	connect(rel_base, &BaseRelationship::s_objectModified, this, &RelationshipView::configureLine);
}

void RelationshipView::configurePositionInfo()
{
	if(this->isSelected())
	{
		BaseObjectView::configurePositionInfo(descriptor->pos());
		pos_info_item->setPos(descriptor->pos().x(),
													descriptor->pos().y() - (pos_info_item->boundingRect().height() * 1.5));
	}
}

void RelationshipView::configureLine()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::TableId src_tab = BaseRelationship::SrcTable,
			dst_tab = BaseRelationship::DstTable;

	configureToolTip();

	/* We skip the line configuration if the line is still being configured, or the bounding rectangles
	 * of the involved tables are not valid (they wasn't rendered/configured yet) or the
	 * hash codes of both tables are unchanged since the last call to this method */
	if(configuring_line ||

			!BaseGraphicObject::isUpdatesEnabled() ||

			!tables[src_tab]->boundingRect().isValid() ||

			!tables[dst_tab]->boundingRect().isValid() ||

			((!tab_hashes[src_tab].isEmpty() &&
					tab_hashes[src_tab] == base_rel->getTable(src_tab)->getHashCode()) &&

			 (!tab_hashes[dst_tab].isEmpty() &&
					tab_hashes[dst_tab] == base_rel->getTable(dst_tab)->getHashCode())))
		return;

	tab_hashes[src_tab] = base_rel->getTable(src_tab)->getHashCode();
	tab_hashes[dst_tab] = base_rel->getTable(dst_tab)->getHashCode();

	//Reconnect the tables if the placeholder usage changes
	if(using_placeholders!=BaseObjectView::isPlaceholderEnabled())
	{
		connectTables();
		using_placeholders=BaseObjectView::isPlaceholderEnabled();
	}

	std::vector<QPointF> points, fk_points, pk_points;
	QGraphicsLineItem *lin=nullptr;
	QPointF pos, p_central[2], pk_pnt, fk_pnt;
	QRectF rect;
	QPen pen;
	QGraphicsPolygonItem *pol=nullptr;
	QPolygonF pol_aux;
	QString tool_tip;
	QGraphicsItem *item=nullptr;
	int i, i1, count;
	bool conn_same_sides = false,
			conn_horiz_sides[2] = { false, false }, conn_vert_sides[2] = { false, false };
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();

	configuring_line=true;
	pen.setCapStyle(Qt::RoundCap);

	if(base_rel->isSelfRelationship())
	{
		double fnt_factor=font_config[Attributes::Global].font().pointSizeF()/DefaultFontSize,
				pos_factor = 0, offset = 0;
		unsigned rel_cnt = tables[0]->getConnectedRelsCount(base_rel->getTable(BaseRelationship::SrcTable),
																												base_rel->getTable(BaseRelationship::DstTable));

		/* Sefl-relationshihp line format:

					+----<>
					|     |
	 ----------   |
	 | Table  |---+
	 ----------

	 This line cannot be moved or have points added */
		pos=tables[0]->pos();
		rect=tables[0]->boundingRect();

		if(rel_cnt > 1)
		{
			int idx = tables[0]->getConnectedRelationshipIndex(base_rel, true);
			double min_val = std::min<double>(rect.width(), rect.height());

			if(idx < 0) idx =0;
			pos_factor = min_val * 0.08 * idx;
		}

		p_central[0].setX(pos.x() + rect.width());
		p_central[0].setY(pos.y() + (rect.height() / 3.0) + pos_factor);
		p_central[1].setX(pos.x() + (rect.width() / 1.5) - pos_factor);
		p_central[1].setY(pos.y());

		if(p_central[0].y() > pos.y() + rect.height())
			p_central[0].setY(pos.y() + rect.height());

		if(p_central[1].x() < pos.x())
			p_central[1].setX(pos.x());

		offset = use_crows_foot ? 30 : 20;

		points.push_back(QPointF(p_central[0].x() + (offset * fnt_factor) + pos_factor,  p_central[0].y()));
		points.push_back(QPointF(p_central[0].x() + (offset * fnt_factor) + pos_factor,  p_central[1].y() - (offset * fnt_factor) - pos_factor));
		points.push_back(QPointF(p_central[1].x(),  p_central[1].y() - (offset * fnt_factor) - pos_factor));

		base_rel->setPoints(points);
	}
	else
	{
		Relationship *rel=dynamic_cast<Relationship *>(base_rel);
		bool rel_1n= (rel_type==Relationship::Relationship11 ||
									rel_type==Relationship::Relationship1n ||
									rel_type==Relationship::RelationshipFk);

		if(rel &&
			 rel->getRelationshipType()==Relationship::Relationship11 &&
			 rel->isIdentifier())
		{
			tables[0]=dynamic_cast<BaseTableView *>(rel->getReferenceTable()->getOverlyingObject());
			tables[1]=dynamic_cast<BaseTableView *>(rel->getReceiverTable()->getOverlyingObject());
		}

		if(line_conn_mode==ConnectCenterPoints || line_conn_mode==ConnectTableEdges || !rel_1n)
		{
			std::vector<std::vector<QGraphicsLineItem *> *> ref_lines={ &fk_lines, &pk_lines };

			for(i=0; i < 2; i++)
				p_central[i]=tables[i]->getCenter();

			//Destroying the fk and pk connection lines when the line mode changes
			for(i=0; i < 2; i++)
			{
				while(!ref_lines[i]->empty())
				{
					item=ref_lines[i]->back();
					ref_lines[i]->pop_back();
					this->removeFromGroup(item);
					delete item;
				}
			}
		}
		else if(line_conn_mode==ConnectFkToPk && rel_1n)
		{
			QPointF pnt;
			QRectF rec_tab_rect, ref_tab_rect;
			double fk_py=0, pk_py=0, fk_px=0, pk_px=0;
			std::vector<Constraint *> fks;
			Table *ref_tab=nullptr, *rec_tab=nullptr;
			TableView *ref_tab_view=nullptr, *rec_tab_view=nullptr;
			unsigned cnt=0, i=0;
			BaseTableView::ConnectionPoint pk_pnt_type = BaseTableView::LeftConnPoint,
					fk_pnt_type = BaseTableView::LeftConnPoint;

			if(!rel)
			{
				ref_tab=dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::DstTable));
				rec_tab=dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::SrcTable));
			}
			else
			{
				ref_tab=dynamic_cast<Table *>(rel->getReferenceTable());
				rec_tab=dynamic_cast<Table *>(rel->getReceiverTable());
			}

			ref_tab_view=dynamic_cast<TableView *>(ref_tab->getOverlyingObject());
			rec_tab_view=dynamic_cast<TableView *>(rec_tab->getOverlyingObject());

			/* We determine the fk to pk connections only if both tables are not fully collapsed.
			 * Otherwise, the tables' center points are used as connection point of the relationship */
			if(rec_tab->getCollapseMode() != BaseTable::AllAttribsCollapsed &&
				 ref_tab->getCollapseMode() != BaseTable::AllAttribsCollapsed)
			{
				rec_tab->getForeignKeys(fks, true, ref_tab);

				//Create the table's rectangles to detect where to connect the relationship
				ref_tab_rect=QRectF(ref_tab_view->pos(), ref_tab_view->boundingRect().size());

				//In this case the receiver table rect Y must be equal to reference table Y in order to do the correct comparison
				rec_tab_rect=QRectF(QPointF(rec_tab_view->pos().x(),
																		ref_tab_view->pos().y()), rec_tab_view->boundingRect().size());

				if(ref_tab_rect.intersects(rec_tab_rect))
				{
					//Connects the rectangle at the same sides on both tables
					conn_same_sides = true;

					if(rec_tab_rect.center().x() >= ref_tab_rect.center().x())
						pk_pnt_type=fk_pnt_type=BaseTableView::LeftConnPoint;
					else if(rec_tab_rect.center().x() < ref_tab_rect.center().x())
						pk_pnt_type=fk_pnt_type=BaseTableView::RightConnPoint;
				}
				else
				{
					//Connecting the relationship on the opposite sides depending on the tables' position
					if(ref_tab_rect.right() <= rec_tab_rect.left())
					{
						pk_pnt_type=BaseTableView::RightConnPoint;
						fk_pnt_type=BaseTableView::LeftConnPoint;
					}
					else
					{
						pk_pnt_type=BaseTableView::LeftConnPoint;
						fk_pnt_type=BaseTableView::RightConnPoint;
					}
				}

				for(auto &constr : fks)
				{
					cnt=constr->getColumnCount(Constraint::SourceCols);

					for(i=0; i < cnt; i++)
					{
						pnt=rec_tab_view->getConnectionPoints(constr->getColumn(i, Constraint::SourceCols), fk_pnt_type);
						fk_py+=pnt.y();
						fk_px=pnt.x();
						fk_points.push_back(this->mapFromItem(rec_tab_view, pnt));

						pnt=ref_tab_view->getConnectionPoints(constr->getColumn(i, Constraint::ReferencedCols), pk_pnt_type);
						pk_py+=pnt.y();
						pk_px=pnt.x();
						pk_points.push_back(this->mapFromItem(ref_tab_view, pnt));
					}
				}
			}

			if(!fks.empty())
			{
				double pk_dx=(pk_pnt_type==BaseTableView::LeftConnPoint ? -ConnLineLength : ConnLineLength),
						fk_dx=(fk_pnt_type==BaseTableView::LeftConnPoint ? -ConnLineLength : ConnLineLength);

				pk_pnt=this->mapFromItem(ref_tab_view, QPointF(pk_px + pk_dx, pk_py/pk_points.size()));
				fk_pnt=this->mapFromItem(rec_tab_view, QPointF(fk_px + fk_dx, fk_py/fk_points.size()));

				if(rel_type==Relationship::RelationshipFk)
				{
					p_central[1]=pk_pnt;
					p_central[0]=fk_pnt;
				}
				else
				{
					p_central[0]=pk_pnt;
					p_central[1]=fk_pnt;
				}
			}
			else
			{
				/* Fallback configuration: if one of the tables are collapsed or if no fk was found in the
				 * receiver table uses the tables' center points to configure the line in order to avoid glitched lines.
				 * The second situation may happen when the relationship is being validated and the needed fk was not
				 * created yet. In a second interaction of the rel. validation they are created
				 * and the relationship line is properly configured */
				if(rel_type==Relationship::RelationshipFk)
				{
					p_central[1]=pk_pnt=ref_tab_view->getCenter();
					p_central[0]=fk_pnt=rec_tab_view->getCenter();
				}
				else
				{
					p_central[0]=pk_pnt=ref_tab_view->getCenter();
					p_central[1]=fk_pnt=rec_tab_view->getCenter();
				}
			}
		}

		points=base_rel->getPoints();
		count=points.size();
		pol_aux.append(QPointF(0,0)); pol_aux.append(QPointF(6,0));
		pol_aux.append(QPointF(6,6)); pol_aux.append(QPointF(0,6));

		for(i=0; i < count; i++)
		{
			if(i >= static_cast<int>(graph_points.size()))
			{
				pol=new QGraphicsPolygonItem;
				graph_points.push_back(pol);
				pol->setZValue(0);
				pol->setPolygon(pol_aux);
				pol->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));
				pol->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
				this->addToGroup(pol);
			}
			else
				pol=graph_points[i];

			pol->setPos(points[i]);
			pol->moveBy(-GraphicPointRadius/2, -GraphicPointRadius/2);
			pol->setVisible(this->isSelected());
		}

		//Destroy the graphical points not used
		i=graph_points.size()-1;
		while(i > count-1)
		{
			item=graph_points.back();
			graph_points.pop_back();
			this->removeFromGroup(item);
			delete item;
			i--;
		}
	}

	if(base_rel->isSelfRelationship() || line_conn_mode != ConnectTableEdges)
	{
		conn_points[0]=p_central[0];
		conn_points[1]=p_central[1];

		points.insert(points.begin(),p_central[0]);
		points.push_back(p_central[1]);
	}
	else if(line_conn_mode == ConnectTableEdges)
	{
		QRectF brect;
		QPolygonF pol;
		QLineF edge, line = QLineF(tables[0]->getCenter(), tables[1]->getCenter());
		QPointF pi, center, p_aux[2];
		double font_factor=BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor(),
				size_factor = 1,
				border_factor = ConnLineLength * 0.30,
				min_lim = 0, max_lim = 0,
				conn_rels_factors[2] = { 0, 0 };
		unsigned conn_rels_cnt[2] = { 0, 0 };
		QLineF::IntersectType inter_type;

		for(int tab_idx = 0; tab_idx < 2; tab_idx++)
		{
			conn_rels_cnt[tab_idx] = tables[tab_idx]->getConnectedRelsCount(base_rel->getTable(BaseRelationship::SrcTable),
																																			base_rel->getTable(BaseRelationship::DstTable));
			conn_rels_factors[tab_idx] = conn_rels_cnt[tab_idx] == 1 ? 1 : 0.08 * (tables[tab_idx]->getConnectedRelationshipIndex(base_rel));

			if(!points.empty())
			{
				if(tab_idx == 0)
					line = QLineF(tables[0]->getCenter(), points[0]);
				else
					line = QLineF(tables[1]->getCenter(), points[points.size() - 1]);
			}

			if(rel_type==BaseRelationship::RelationshipGen || rel_type==BaseRelationship::RelationshipDep || rel_type==BaseRelationship::RelationshipPart)
				size_factor = 0.40;
			else if(use_crows_foot)
			{
				if(rel_type==BaseRelationship::RelationshipNn ||
					 (tab_idx == 1 && rel_type==BaseRelationship::RelationshipFk) ||
					 (tab_idx == 0 &&	rel_type==BaseRelationship::Relationship1n && base_rel->isTableMandatory(BaseRelationship::SrcTable)) ||
					 (tab_idx == 0 && rel_type==BaseRelationship::Relationship11 && base_rel->isTableMandatory(BaseRelationship::SrcTable)) ||
					 (tab_idx == 1 && rel_type==BaseRelationship::Relationship11 && base_rel->isTableMandatory(BaseRelationship::DstTable)))
					size_factor = 1 * font_factor;
				else
					size_factor = 1.5 * font_factor;

				if(rel_type == BaseRelationship::RelationshipPart ||
					 rel_type == BaseRelationship::RelationshipGen ||
					 rel_type == BaseRelationship::RelationshipDep ||
					 rel_type == BaseRelationship::Relationship11 ||
					 (tab_idx == 0 && rel_type == BaseRelationship::Relationship1n) ||
					 (tab_idx == 1 && rel_type == BaseRelationship::RelationshipFk))
					border_factor = ConnLineLength * 0.30;
				else
					border_factor = ConnLineLength * 0.75;
			}
			else
				size_factor = 0.65 * font_factor;

			brect = QRectF(tables[tab_idx]->pos(), tables[tab_idx]->boundingRect().size());
			pol = QPolygonF(brect);
			center = tables[tab_idx]->getCenter();

			for(int idx = 0; idx < pol.size() - 1; idx++)
			{
				edge.setP1(pol.at(idx));
				edge.setP2(pol.at(idx + 1));
				inter_type = line.intersects(edge, &pi);

				if(inter_type == QLineF::BoundedIntersection)
				{
					/* Adjusting the intersection point if there're more than one relationship connected the current table
					 * this will cause all relationships to be aligned together */
					if(conn_rels_cnt[tab_idx] > 1)
					{
						double max_dim = std::max<double>(brect.height(), brect.width());
						int signal = 0;

						if(edge.dx() == 0)
						{
							signal = edge.dy()/fabs(edge.dy()) * (tab_idx == 0 ? 1 : -1);
							pi.setY(pi.y() - ((conn_rels_factors[tab_idx] * max_dim) * signal));

							// Adjusting the position of the interesection point to make is as close to the center of the edge as possible
							pi.setY(pi.y() + ((max_dim * 0.05) * signal));
						}

						if(edge.dy() == 0)
						{
							signal = edge.dx()/fabs(edge.dx()) * (tab_idx == 0 ? 1 : -1);
							pi.setX(pi.x() - ((conn_rels_factors[tab_idx] * max_dim) * signal));

							// Adjusting the position of the interesection point to make is as close to the center of the edge as possible
							pi.setX(pi.x() + ((max_dim * 0.05) * signal));
						}
					}

					//Avoiding the line to be exposed in the rounded corners of the tables
					if(edge.dx() == 0)
					{
						min_lim = brect.top() + border_factor;
						max_lim = brect.bottom() - border_factor;

						if(pi.y() < min_lim)
							pi.setY(min_lim);
						else if(pi.y() > max_lim)
							pi.setY(max_lim);
					}
					else
					{
						min_lim = brect.left() + border_factor;
						max_lim = brect.right() - border_factor;

						if(pi.x() < min_lim)
							pi.setX(min_lim);
						else if(pi.x() > max_lim)
							pi.setX(max_lim);
					}

					conn_points[tab_idx] = p_central[tab_idx] = pi;

					if(edge.dx() == 0)
					{
						if(pi.x() < center.x())
							pi.setX(pi.x() - (ConnLineLength * size_factor));
						else
							pi.setX(pi.x() + (ConnLineLength * size_factor));

						conn_vert_sides[tab_idx] = true;
					}
					else
					{
						if(pi.y() < center.y())
							pi.setY(pi.y() - (ConnLineLength * size_factor));
						else
							pi.setY(pi.y() + (ConnLineLength * size_factor));

						conn_horiz_sides[tab_idx] = true;
					}

					p_aux[tab_idx] = pi;
					break;
				}
				
				p_aux[tab_idx] = p_central[tab_idx];
			}
		}

		points.insert(points.begin(), p_central[0]);
		points.insert(points.begin() + 1, p_aux[0]);

		points.push_back(p_aux[1]);
		points.push_back(p_central[1]);
	}

	//If the relationship is selected we do not change the lines colors
	if(this->isSelected() && !lines.empty())
		pen = lines[0]->pen();
	else
	{
		//Configuring the relationship line color
		if(base_rel->getCustomColor()!=Qt::transparent)
			//Using custom color
			pen.setColor(base_rel->getCustomColor());
		else
			//Using the default color
			pen=BaseObjectView::getBorderStyle(Attributes::Relationship);
	}

	//For dependency/partition relationships the line is dashed
	if(rel_type==BaseRelationship::RelationshipDep ||
		 rel_type == BaseRelationship::RelationshipPart)
		pen.setStyle(Qt::DashLine);

	if(line_conn_mode==ConnectFkToPk)
	{
		std::vector<QPointF> ref_points={ fk_pnt, pk_pnt };
		std::vector<std::vector<QPointF> *> ref_pnt_vects={ &fk_points, &pk_points };
		std::vector<std::vector<QGraphicsLineItem *> *> ref_lines={ &fk_lines, &pk_lines };
		std::vector<QPointF> *ref_pnt=nullptr;
		std::vector<QGraphicsLineItem *> *ref_lin=nullptr;

		for(unsigned vet_idx=0; vet_idx < 2; vet_idx++)
		{
			ref_pnt=ref_pnt_vects[vet_idx];
			ref_lin=ref_lines[vet_idx];
			count=ref_pnt->size();

			for(i=0; i < count; i++)
			{
				if(i >= static_cast<int>(ref_lin->size()))
				{
					lin=new QGraphicsLineItem;
					lin->setZValue(-1);
					ref_lin->push_back(lin);
					this->addToGroup(lin);
				}
				else
					lin=ref_lin->at(i);

				pen.setWidthF(getDefaultPenWidth());
				lin->setLine(QLineF(ref_pnt->at(i), ref_points[vet_idx]));
				lin->setPen(pen);
			}

			//Destroy the unused pk or fk lines
			i = ref_lin->size()-1;
			while(i > count - 1)
			{
				item = ref_lin->back();
				ref_lin->pop_back();
				this->removeFromGroup(item);
				delete item;
				i--;
			}
		}
	}

	//Create the relationship lines
	count=points.size();
	for(i=0; i < count-1; i++)
	{
		if(i >= static_cast<int>(lines.size()))
		{
			lin=new QGraphicsLineItem;
			lin->setZValue(-1);
			lines.push_back(lin);
			this->addToGroup(lin);
		}
		else
			lin=lines[i];

		pen.setWidthF(getDefaultPenWidth());
		lin->setLine(QLineF(points[i], points[i+1]));
		lin->setPen(pen);
	}

	//Removing unused lines
	if(!base_rel->isSelfRelationship())
	{
		i=points.size()-1;
		i1=lines.size();
		while(i1 > i)
		{
			item=lines.back();
			lines.pop_back();
			this->removeFromGroup(item);
			delete item;
			i1--;
		}
	}

	//Exposing the line ending circles
	if((!base_rel->isSelfRelationship() && line_conn_mode==ConnectCenterPoints && !use_crows_foot) ||
		 (!base_rel->isSelfRelationship() &&
			((line_conn_mode != ConnectTableEdges && rel_type==BaseRelationship::RelationshipDep) ||
			 (line_conn_mode != ConnectTableEdges && rel_type==BaseRelationship::RelationshipGen) ||
			 (line_conn_mode != ConnectTableEdges && rel_type==BaseRelationship::RelationshipPart) ||
			 (line_conn_mode != ConnectTableEdges && rel_type==BaseRelationship::RelationshipNn  && !use_crows_foot))))
	{
		for(i=0; i < 2; i++)
		{
			line_circles[i]->setVisible(true);
			line_circles[i]->setPen(pen);
			line_circles[i]->setBrush(pen.color());
			line_circles[i]->setPos(p_central[i]-QPointF(GraphicPointRadius/2, GraphicPointRadius/2));
		}
	}
	else
	{
		line_circles[0]->setVisible(false);
		line_circles[1]->setVisible(false);
	}

	//Using bezier curves instead of straight lines to denote the relationship line
	if(use_curved_lines && !base_rel->isSelfRelationship())
	{
		BezierCurveItem * curve = nullptr;
		bool invert_cpoints = false, simple_curve = false;
		i = 0;

		for(auto &line : lines)
		{
			if(i >= static_cast<int>(curves.size()))
			{
				curve = new BezierCurveItem;
				curve->setZValue(line->zValue());
				this->addToGroup(curve);
				curves.push_back(curve);
			}
			else
				curve = curves[i];

			i++;

			/* Creates simple a curved line in the following situations:
			 * 1) The crow's foot is not enabled but the relationship connects in the same sides on tables (conn_same_sides)
			 * 2) The crow's foot is enabled and one of the connection point is not in the horizontal edges of one of the tables (conn_horiz_sides) */

			/* We invert the curve's bounding rect when crow's foot is enabled and the relationship connects
			 * at the top/bottom edges of both tables */
			simple_curve = (conn_same_sides && lines.size() == 1) || (conn_horiz_sides[0] != conn_horiz_sides[1] && lines.size() == 3);

			if(!simple_curve)
				invert_cpoints = (conn_horiz_sides[0] && conn_horiz_sides[1] && lines.size() == 3);
			else
				invert_cpoints = (!conn_horiz_sides[0] && conn_horiz_sides[1] && conn_vert_sides[0] && !conn_vert_sides[1]);

			curve->setLine(line->line(), simple_curve, invert_cpoints);
			curve->setPen(line->pen());
			line->setVisible(false);
		}

		//Removing unused curves
		i=lines.size();
		i1=curves.size();

		while(i1 > i)
		{
			curve=curves.back();
			curves.pop_back();
			this->removeFromGroup(curve);
			delete curve;
			i1--;
		}
	}
	else if(!use_curved_lines && !lines.empty() && !curves.empty())
	{
		BezierCurveItem *curve = nullptr;

		for(auto &line : lines)
			line->setVisible(true);

		while(!curves.empty())
		{
			curve=curves.back();
			curves.pop_back();
			this->removeFromGroup(curve);
			delete curve;
		}
	}

	this->configureDescriptor();

	this->configureCrowsFootDescriptors();

	this->configureLabels();

	this->configureProtectedIcon();

	configureBoundingRect();

	configuring_line=false;
}

void RelationshipView::configureToolTip()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	QString tool_tip;

	/* Making a little tweak on the foreign key type name. Despite being of class BaseRelationship,
	for semantics purposes shows the type of this relationship as "Relationship" unlike "Link" */
	if(base_rel->getRelationshipType() == BaseRelationship::RelationshipFk)
		tool_tip = QString("`%1' (%2)").arg(base_rel->getName(), BaseObject::getTypeName(ObjectType::Relationship));
	else
		tool_tip = QString("`%1' (%2)").arg(base_rel->getName(), base_rel->getTypeName());

	tool_tip +=	QString("\n%1 Id: %2").arg(UtilsNs::DataSeparator, QString::number(base_rel->getObjectId())) +
							QString("\n%1 Src table: `%2'").arg(UtilsNs::DataSeparator, base_rel->getTable(BaseRelationship::SrcTable)->getSignature()) +
							QString("\n%1 Dst table: `%2'").arg(UtilsNs::DataSeparator, base_rel->getTable(BaseRelationship::DstTable)->getSignature());

	if(!base_rel->getAlias().isEmpty())
		tool_tip += QString("\n%1 Alias: %2").arg(UtilsNs::DataSeparator, base_rel->getAlias());

	tool_tip += QString("\n%1 %2 %3").arg(TableObjectView::ConstrDelimStart, base_rel->getRelationshipTypeName(), TableObjectView::ConstrDelimEnd);
	tool_tip = UtilsNs::formatMessage(tool_tip);

	this->setToolTip(tool_tip);

	for(auto & label : labels)
	{
		if(label)
			label->setToolTip(tool_tip);
	}

	if(descriptor)
		descriptor->setToolTip(tool_tip);

	for(auto &curve : curves)
	{
		if(curve)
			curve->setToolTip(tool_tip);
	}

	for(int i = 0; i < 2; i++)
	{
		if(cf_descriptors[i])
			cf_descriptors[i]->setToolTip(tool_tip);

		if(round_cf_descriptors[i])
			round_cf_descriptors[i]->setToolTip(tool_tip);
	}
}

void RelationshipView::configureDescriptor()
{
	QLineF lin;
	QPolygonF pol;
	BaseRelationship *base_rel=this->getUnderlyingObject();
	unsigned rel_type=base_rel->getRelationshipType();
	double x, y, x1, y1, angle = 0,
			factor=BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();
	QPen pen;
	QPointF pnt;
	std::vector<QPointF> points=base_rel->getPoints();
	QColor line_color=base_rel->getCustomColor();
	QGraphicsPolygonItem *pol_item=nullptr;

	//Configuring the relationship descriptor color
	if(base_rel->getCustomColor()!=Qt::transparent)
		//Using custom color
		pen.setColor(base_rel->getCustomColor());
	else
		//Using the default color
		pen=BaseObjectView::getBorderStyle(Attributes::Relationship);

	if(rel_type==BaseRelationship::RelationshipDep ||
		 rel_type == BaseRelationship::RelationshipPart)
		pen.setStyle(Qt::DashLine);

	pen.setWidthF(getDefaultPenWidth());
	descriptor->setPen(pen);

	if(line_color!=Qt::transparent)
	{
		QColor colors[2];
		QLinearGradient grad;
		BaseObjectView::getFillStyle(Attributes::Relationship, colors[0], colors[1]);

		for(unsigned i=0; i < 2; i++)
		{
			colors[i].setRed((colors[i].red() + line_color.red() + 255)/3);
			colors[i].setGreen((colors[i].green() + line_color.green() + 255)/3);
			colors[i].setBlue((colors[i].blue() + line_color.blue() + 255)/3);
			grad.setColorAt(i, colors[i]);
		}

		grad.setCoordinateMode(QGradient::ObjectBoundingMode);
		descriptor->setBrush(grad);
	}
	else
		descriptor->setBrush(BaseObjectView::getFillStyle(Attributes::Relationship));

	if(rel_type==BaseRelationship::RelationshipDep ||
	   rel_type==BaseRelationship::RelationshipGen)
	{
	  pol.append(QPointF(0,0)); pol.append(QPointF(18,10));
	  pol.append(QPointF(0,20)); pol.append(QPointF(0,10));
	}
	else if(rel_type==BaseRelationship::RelationshipPart)
	{
	  pol.append(QPointF(0,4)); pol.append(QPointF(4,0));
	  pol.append(QPointF(18,12)); pol.append(QPointF(4,24));
	  pol.append(QPointF(0,20)); pol.append(QPointF(10,12));
	}
	else
	{
		pol.append(QPointF(11,0)); pol.append(QPointF(22,11));
		pol.append(QPointF(11,22)); pol.append(QPointF(0,11));
	}

	resizePolygon(pol,
								(pol.boundingRect().width() + (HorizSpacing * 2)) * factor ,
								(pol.boundingRect().height() + (VertSpacing * 2)) * factor);

	if(base_rel->isSelfRelationship())
		pnt = points.at(points.size()/2);
	else
	{
		if(!curves.empty())
		{
			int idx = curves.size()/2;
			BezierCurveItem *curve =  curves.at(idx);
			QPainterPath path = curve->path();

			/* Workaround to avoid the inheritance / dependency relationship to get the descriptor rotated to the wrong side
			 * We create and auxiliary line with points from the position at 65% of the curve to the 45% and use the
			 * angle of that line instead of the angle at 50% of the curve */
			if((rel_type == BaseRelationship::RelationshipDep ||
				rel_type == BaseRelationship::RelationshipGen ||
				rel_type == BaseRelationship::RelationshipPart) &&
				 curve->isControlPointsInverted() && !curve->isSimpleCurve() && !curve->isStraightLine())
			{
				QLineF lin_aux = QLineF(path.pointAtPercent(0.65), path.pointAtPercent(0.45));
				angle = -lin_aux.angle();
				pnt = path.pointAtPercent(0.5);
			}
			else
			{
				double percent = path.percentAtLength(path.length()/2);
				angle = -path.angleAtPercent(percent);
				pnt = path.pointAtPercent(percent);
			}
		}
		else
		{
			lin=lines.at(lines.size()/2)->line();
			pnt.setX((lin.p1().x() + lin.p2().x()) / 2.0);
			pnt.setY((lin.p1().y() + lin.p2().y()) / 2.0);
			angle = -lin.angle();
		}

		descriptor->setRotation(angle);
		obj_selection->setRotation(angle);
		obj_shadow->setRotation(angle);
	}

	x=x1=pnt.x() - (pol.boundingRect().width()/2.0);
	y=y1=pnt.y() - (pol.boundingRect().height()/2.0);

	protected_icon->setPos(x + ((pol.boundingRect().width()/2.0) * 0.60),
												 y + ((pol.boundingRect().height()/2.0) * 0.55));

	configureSQLDisabledInfo();
	x1+=6 * HorizSpacing;
	y1-=3 * VertSpacing;
	sql_disabled_item->setPos(x1, y1);

	descriptor->setPolygon(pol);
	descriptor->setTransformOriginPoint(descriptor->boundingRect().center());
	descriptor->setPos(x, y);

	pol_item=dynamic_cast<QGraphicsPolygonItem *>(obj_selection);
	pol_item->setPolygon(pol);
	pol_item->setTransformOriginPoint(obj_selection->boundingRect().center());
	pol_item->setPos(x,y);
	pol_item->setBrush(this->getFillStyle(Attributes::ObjSelection));
	pol_item->setPen(this->getBorderStyle(Attributes::ObjSelection));

	pol_item=dynamic_cast<QGraphicsPolygonItem *>(obj_shadow);
	pol_item->setPolygon(pol);
	pol_item->setTransformOriginPoint(obj_shadow->boundingRect().center());
	pol_item->setPos(x + ObjectShadowXPos, y + ObjectShadowYPos);
	pol_item->setPen(getBorderStyle(Attributes::ObjShadow));
	pol_item->setBrush(getFillStyle(Attributes::ObjShadow));

	this->configureAttributes();
	this->configurePositionInfo();

	/* If the crow's feet is enabled the relationship descriptor is hidden
	 * for 1:1, 1:n, n:n and fk relationship. For generalization, dependency and partitioning
	 * relationships the descriptor is still displayed. */
	bool visible = !use_crows_foot ||
								 (use_crows_foot && (
										rel_type == BaseRelationship::RelationshipDep ||
										rel_type == BaseRelationship::RelationshipGen ||
										rel_type == BaseRelationship::RelationshipPart));
	descriptor->setVisible(visible);
	obj_shadow->setVisible(visible);
}

void RelationshipView::configureCrowsFootDescriptors()
{
	BaseRelationship * base_rel = this->getUnderlyingObject();
	Relationship *rel=dynamic_cast<Relationship *>(base_rel);

	//Hiding all descriptors related to crow's foot when the notation is not being used
	if(!use_crows_foot && cf_descriptors[BaseRelationship::SrcTable])
	{
		for(unsigned tab_id = BaseRelationship::SrcTable; tab_id <= BaseRelationship::DstTable; tab_id++)
		{
			for(auto &item : cf_descriptors[tab_id]->childItems())
			{
				cf_descriptors[tab_id]->setRotation(0);
				cf_descriptors[tab_id]->removeFromGroup(item);
				this->removeFromGroup(item);
				item->setVisible(false);
				cf_descriptors[tab_id]->setVisible(false);
			}
		}
	}
	else if(use_crows_foot && base_rel &&
		 (base_rel->getRelationshipType() == BaseRelationship::Relationship11 ||
			base_rel->getRelationshipType() == BaseRelationship::Relationship1n ||
			base_rel->getRelationshipType() == BaseRelationship::RelationshipNn ||
			base_rel->getRelationshipType() == BaseRelationship::RelationshipFk))
	{
		QGraphicsLineItem *line_item = nullptr;
		QGraphicsEllipseItem *circle_item = nullptr;
		BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
		double factor=BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();
		int signal = 1;
		BaseTableView *tables[2] = { nullptr, nullptr };
		bool mandatory[2] = { false, false }, simulate_rel11 = false;

		if(rel_type == BaseRelationship::RelationshipNn || rel_type == BaseRelationship::RelationshipFk)
		{
			tables[BaseRelationship::SrcTable] = dynamic_cast<BaseTableView *>(base_rel->getTable(BaseRelationship::SrcTable)->getOverlyingObject());
			tables[BaseRelationship::DstTable] = dynamic_cast<BaseTableView *>(base_rel->getTable(BaseRelationship::DstTable)->getOverlyingObject());
			mandatory[BaseRelationship::SrcTable] = base_rel->isTableMandatory(BaseRelationship::SrcTable);
			mandatory[BaseRelationship::DstTable] = base_rel->isTableMandatory(BaseRelationship::DstTable);

			simulate_rel11 = base_rel->canSimulateRelationship11();
		}
		else
		{
			tables[BaseRelationship::SrcTable] = dynamic_cast<BaseTableView *>(rel->getReferenceTable()->getOverlyingObject());
			tables[BaseRelationship::DstTable] = dynamic_cast<BaseTableView *>(rel->getReceiverTable()->getOverlyingObject());
			mandatory[BaseRelationship::SrcTable] = rel->isReferenceTableMandatory();
			mandatory[BaseRelationship::DstTable] = rel->isReceiverTableMandatory();

			/* There's a special case for one-to-one relationships that will cause the crow's foot
			 * descriptors to be positioned in the oposite sides when compared to other relationships.
			 * This because pgModeler switches automatically the receiver table to be the destination one
			 * when the mandatory table of the relationship is the destination. So we use and auxiliary (signal inverter)
			 * variable to alter the descriptors position */
			if(mandatory[BaseRelationship::SrcTable] && rel_type == BaseRelationship::Relationship11 &&
				 rel->getTable(BaseRelationship::DstTable) == rel->getReferenceTable())
				signal = -1;
		}

		//Allocatting all objects related to the crow's foot descriptors
		if(cf_descriptors[BaseRelationship::SrcTable] == nullptr)
		{
			int src_zvalue = lines.front()->zValue() + 1,
					dst_zvalue = lines.back()->zValue() + 1;
			QGraphicsLineItem *line_item = nullptr;

			for(int idx = 0; idx < 2; idx++)
			{
				cf_descriptors[idx] = new QGraphicsItemGroup;
				round_cf_descriptors[idx] = new QGraphicsEllipseItem;
				this->addToGroup(cf_descriptors[idx]);
			}

			for(int idx = 0; idx < 4; idx++)
			{
				line_item = new QGraphicsLineItem;
				src_cf_lines.push_back(line_item);
				cf_descriptors[BaseRelationship::SrcTable]->setZValue(src_zvalue);

				line_item = new QGraphicsLineItem;
				dst_cf_lines.push_back(line_item);
				cf_descriptors[BaseRelationship::DstTable]->setZValue(dst_zvalue);
			}
		}
		else
		{
			cf_descriptors[BaseRelationship::SrcTable]->setVisible(true);
			cf_descriptors[BaseRelationship::DstTable]->setVisible(true);
		}

		QPointF pi;
		QRectF brect;
		QPen pen, pens[2] = { lines.front()->pen(), lines.back()->pen() };
		QLineF::IntersectType inter_type;
		QLineF line, line1, edge, rel_lines[2] = {(signal < 0 ? lines.back()->line() : lines.front()->line()),
															 (signal < 0 ? lines.front()->line() : lines.back()->line())};
		QPolygonF pol;
		std::vector<std::vector<QGraphicsLineItem *> *> cf_lines = { &src_cf_lines, &dst_cf_lines };
		unsigned lin_idx = 0;
		double px = 0, py = 0, min_x = 0, max_x = 0, min_y = 0, max_y = 0;

		for(unsigned tab_id = BaseRelationship::SrcTable; tab_id <= BaseRelationship::DstTable; tab_id++)
		{
			cf_descriptors[tab_id]->setRotation(0);

			for(auto &line : *cf_lines[tab_id])
			{
				cf_descriptors[tab_id]->removeFromGroup(line);
				this->removeFromGroup(line);
				line->setVisible(false);
			}

			//Configuring the minimum cardinality descriptor
			if(simulate_rel11 ||

				 ((tab_id == BaseRelationship::SrcTable &&
					rel_type != BaseRelationship::RelationshipNn && rel_type != BaseRelationship::RelationshipFk) ||

				 (tab_id == BaseRelationship::DstTable &&
					(rel_type == BaseRelationship::Relationship11 || rel_type == BaseRelationship::RelationshipFk))))
			{

				line_item = cf_lines[tab_id]->at(lin_idx++);
				cf_descriptors[tab_id]->addToGroup(line_item);
				line_item->setVisible(true);

				line_item->setLine(QLineF(QPointF(0, -8 * factor), QPointF(0, 8 * factor)));
				line_item->setPos((10 * signal) * (tab_id == BaseRelationship::DstTable ? -1 : 1), 0);
				line_item->setPen(pens[tab_id]);
			}
			else
			{
				//Configuring the crow's foot descriptor
				if(tab_id == BaseRelationship::SrcTable)
				{
					px = 0;
					line = QLineF(QPointF(18, 0), QPointF(0, - 10 * factor));
					line1 = QLineF(QPointF(18, 0), QPointF(0, 10 * factor));
				}
				else
				{
					line = QLineF(QPointF(0, 0), QPointF(18, - 10 * factor));
					line1 = QLineF(QPointF(0, 0), QPointF(18, 10 * factor));
					px = -line.dx();
				}

				line_item = cf_lines[tab_id]->at(lin_idx++);
				cf_descriptors[tab_id]->addToGroup(line_item);
				line_item->setVisible(true);
				line_item->setLine(line);
				line_item->setPos(px, 0);
				line_item->setPen(pens[tab_id]);

				line_item = cf_lines[tab_id]->at(lin_idx++);
				cf_descriptors[tab_id]->addToGroup(line_item);
				line_item->setVisible(true);
				line_item->setLine(line1);
				line_item->setPos(px, 0);
				line_item->setPen(pens[tab_id]);
			}

			cf_descriptors[tab_id]->removeFromGroup(round_cf_descriptors[tab_id]);
			this->removeFromGroup(round_cf_descriptors[tab_id]);
			round_cf_descriptors[tab_id]->setVisible(false);

			//Configuring the maximum cardinality descriptor
			if((tab_id == BaseRelationship::SrcTable && mandatory[tab_id]) ||
				 (tab_id == BaseRelationship::DstTable && mandatory[tab_id] && rel_type == BaseRelationship::RelationshipFk) ||
				 rel_type == BaseRelationship::RelationshipNn)
			{
				line_item = cf_lines[tab_id]->at(lin_idx++);
				cf_descriptors[tab_id]->addToGroup(line_item);
				line_item->setVisible(true);

				line_item->setLine(QLineF(QPointF(0, -8 * factor), QPointF(0, 8 * factor)));
				line_item->setPos(18 * signal * (tab_id == BaseRelationship::DstTable ? -1 : 1), 0);
				line_item->setPen(pens[tab_id]);
			}
			else if(!mandatory[tab_id] /*&&
							((rel_type != BaseRelationship::RelationshipFk) ||
							 (tab_id == BaseRelationship::SrcTable && rel_type == BaseRelationship::RelationshipFk))*/)
			{
				//Configuring the circle which describes the optional cardinality
				circle_item = round_cf_descriptors[tab_id];
				cf_descriptors[tab_id]->addToGroup(circle_item);
				circle_item->setVisible(true);
				circle_item->setRect(QRectF(0, 0, GraphicPointRadius * 2.75 * factor, GraphicPointRadius * 2.75 * factor));

				py = -(circle_item->boundingRect().height()/2.20);

				if(tab_id == BaseRelationship::SrcTable)
				{
					if(base_rel->isSelfRelationship() || rel_type != BaseRelationship::RelationshipNn)
						px = 19;
					else
						px = 11;
				}
				else
				{
					if(rel_type == BaseRelationship::Relationship11)
					{
						if(signal < 0)
							px = 15;
						else
							px = -15 - circle_item->boundingRect().width();
					}
					else
						px = -17 - circle_item->boundingRect().width();
				}

				pen = pens[tab_id];

				if(rel_type == BaseRelationship::RelationshipFk)
					pen.setStyle(Qt::DashLine);

				circle_item->setPos(px, py);
				circle_item->setPen(pen);
				circle_item->setBrush(descriptor->brush());
			}

			lin_idx = 0;
			brect = QRectF(tables[tab_id]->pos(), tables[tab_id]->boundingRect().size());
			pol = QPolygonF(brect);

			/* Configuring the position of the descriptor based upon the intersection
			 * point of the relationship line and the edge of the bounding rect of the table */
			cf_descriptors[tab_id]->setRotation(-rel_lines[tab_id].angle());

			for(int idx = 0; idx < pol.size() - 1; idx++)
			{
				edge.setP1(pol.at(idx));
				edge.setP2(pol.at(idx + 1));
				inter_type = rel_lines[tab_id].intersects(edge, &pi);

				if(inter_type == QLineF::BoundedIntersection)
				{
					cf_descriptors[tab_id]->setPos(pi);
					break;
				}

				/* There some cases in which the intersection point can't be determined in BoundedIntersection mode
					* so as a fallback we check if one of the coordinates of the identified intersection point
					* is between  one of the coordinates of the relationship line used in the operation.
					*
					* If it matches we'll use the use one of the extremes of the relationship line in the matching coordinate */
				min_x = qMin<double>(rel_lines[tab_id].p1().x(), rel_lines[tab_id].p2().x());
				max_x = qMax<double>(rel_lines[tab_id].p1().x(), rel_lines[tab_id].p2().x());
				min_y = qMin<double>(rel_lines[tab_id].p1().y(), rel_lines[tab_id].p2().y());
				max_y = qMax<double>(rel_lines[tab_id].p1().y(), rel_lines[tab_id].p2().y());

				if(pi.x() >= min_x && pi.x() <= max_x)
					cf_descriptors[tab_id]->setPos(QPointF(pi.x(), (rel_lines[tab_id].dy() >= 0 ? max_y : min_y)));
				else if(pi.y() >= min_y && pi.y() <= max_y)
					cf_descriptors[tab_id]->setPos(QPointF((rel_lines[tab_id].dx() >= 0 ? max_x : min_x), pi.y()));				
			}
		}
	}
}

void RelationshipView::configureAttributes()
{
	Relationship *rel=dynamic_cast<Relationship *>(this->getUnderlyingObject());

	if(!rel)
		return;

	int i = 0, count;
	Column *col=nullptr;
	QGraphicsItemGroup *attrib=nullptr;
	QGraphicsLineItem *lin=nullptr;
	QGraphicsEllipseItem *desc=nullptr;
	QGraphicsPolygonItem *sel_attrib=nullptr;
	QGraphicsSimpleTextItem *text=nullptr;
	QGraphicsItemGroup *item=nullptr;
	QPointF p_aux;
	QTextCharFormat fmt;
	QFont font;
	QRectF rect;
	QPolygonF pol;
	double py, px,
			factor=font_config[Attributes::Global].font().pointSizeF()/DefaultFontSize;

	fmt=font_config[Attributes::Attribute];
	font=fmt.font();
	font.setPointSizeF(font.pointSizeF() * 0.90);

	//Configures the rectangle used as base for creation of attribute descriptor
	rect.setTopLeft(QPointF(0,0));
	rect.setSize(QSizeF(12 * factor, 12 * factor));

	//Calculates the first attribute position based upon the attribute count and descriptor size
	count=rel->getAttributeCount();
	px=descriptor->pos().x() + descriptor->boundingRect().width() + (4 * HorizSpacing * factor);
	py=descriptor->pos().y() - (count * rect.height()/(4 * factor));

	for(i=0; i < count; i++)
	{
		col=rel->getAttribute(i);

		if(i >= static_cast<int>(attributes.size()))
		{
			attrib=new QGraphicsItemGroup;
			attrib->setZValue(-1);

			//Creates the line that connects the attribute to the relationship descriptor
			lin=new QGraphicsLineItem;
			lin->setZValue(-1);
			attrib->addToGroup(lin);

			//Creates the attribute descriptor
			desc=new QGraphicsEllipseItem;
			desc->setZValue(0);
			attrib->addToGroup(desc);

			//Creates the attribute text
			text=new QGraphicsSimpleTextItem;
			text->setZValue(0);
			attrib->addToGroup(text);

			sel_attrib=new QGraphicsPolygonItem;
			sel_attrib->setZValue(1);
			sel_attrib->setVisible(false);
			attrib->addToGroup(sel_attrib);

			this->addToGroup(attrib);
			attributes.push_back(attrib);
		}
		else
		{
			attrib=attributes[i];
			lin=dynamic_cast<QGraphicsLineItem *>(attrib->childItems().at(0));
			desc=dynamic_cast<QGraphicsEllipseItem *>(attrib->childItems().at(1));
			text=dynamic_cast<QGraphicsSimpleTextItem *>(attrib->childItems().at(2));
			sel_attrib=dynamic_cast<QGraphicsPolygonItem *>(attrib->childItems().at(3));
		}

		desc->setRect(rect);
		desc->setPen(BaseObjectView::getBorderStyle(Attributes::Attribute));
		desc->setBrush(BaseObjectView::getFillStyle(Attributes::Attribute));
		lin->setPen(descriptor->pen());
		text->setBrush(fmt.foreground());
		text->setFont(font);
		sel_attrib->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
		sel_attrib->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));

		attrib->setPos(px, py);

		text->setText(compact_view && !col->getAlias().isEmpty() ? col->getAlias() : col->getName());
		text->setPos(QPointF(desc->pos().x() + desc->boundingRect().width() + (HorizSpacing * factor),
												 (desc->boundingRect().height() - text->boundingRect().height())/2.5));
		desc->setPos(0, VertSpacing * factor);

		pol.clear();
		pol.append(QPointF(-HorizSpacing, 0));
		pol.append(QPointF(desc->boundingRect().width() + text->boundingRect().width() + (2 * HorizSpacing), 0));
		pol.append(QPointF(desc->boundingRect().width() + text->boundingRect().width() + (2 * HorizSpacing), text->boundingRect().height()));
		pol.append(QPointF(-HorizSpacing, text->boundingRect().height()));
		sel_attrib->setPolygon(pol);

		p_aux=this->mapToItem(attrib, descriptor->pos().x() + (descriptor->boundingRect().width()/2.0),
													descriptor->pos().y() + (descriptor->boundingRect().height()/2.0));
		lin->setLine(QLineF(p_aux, desc->boundingRect().center()));

		py+=desc->boundingRect().height() + (5 * VertSpacing);
	}

	i=attributes.size()-1;
	while(i > count-1)
	{
		item=attributes.back();
		attributes.pop_back();
		this->removeFromGroup(item);
		delete item;
		i--;
	}

}

void RelationshipView::configureLabels()
{
	double x=0,y=0;
	QPointF pnt;
	BaseRelationship *base_rel=this->getUnderlyingObject();
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
	QPointF label_dist;

	label_dist=base_rel->getLabelDistance(BaseRelationship::RelNameLabel);

	pnt=descriptor->pos();
	x=pnt.x() -
		((labels[BaseRelationship::RelNameLabel]->boundingRect().width() -
		 descriptor->boundingRect().width())/2.0);

	if(base_rel->isSelfRelationship())
		y=pnt.y() -	labels[BaseRelationship::RelNameLabel]->boundingRect().height() - (2 * VertSpacing);
	else
		y=pnt.y() + descriptor->boundingRect().height() + VertSpacing;

	labels[BaseRelationship::RelNameLabel]->setVisible(!hide_name_label);
	configureLabelPosition(BaseRelationship::RelNameLabel, x, y);

	if(!hide_name_label)
	{
		Textbox *txtbox = dynamic_cast<Textbox *>(labels[BaseRelationship::RelNameLabel]->getUnderlyingObject());

		if(compact_view && !base_rel->getAlias().isEmpty())
		{
			txtbox->setComment(base_rel->getAlias());
			txtbox->setModified(true);
		}
		else if(txtbox->getComment() != base_rel->getName(true))
		{
			txtbox->setComment(base_rel->getName(true));
			txtbox->setModified(true);
		}
	}

	//Hides the cardinality labels when crow's feet is enabled
	if(labels[BaseRelationship::SrcCardLabel] && labels[BaseRelationship::DstCardLabel])
	{
		labels[BaseRelationship::SrcCardLabel]->setVisible(!use_crows_foot);
		labels[BaseRelationship::DstCardLabel]->setVisible(!use_crows_foot);
	}

	if(!use_crows_foot &&
		 rel_type!=BaseRelationship::RelationshipGen &&
		 rel_type!=BaseRelationship::RelationshipDep &&
		 rel_type!=BaseRelationship::RelationshipPart)
	{
		QPointF pi, pf, p_int, pos;
		unsigned idx, i1;
		double dl, da, v_space=VertSpacing * 2.5, h_space=HorizSpacing * 2.5;
		QLineF lins[2], borders[2][4];
		QRectF tab_rect, rect;
		BaseRelationship::LabelId label_ids[2]={ BaseRelationship::SrcCardLabel,
																											 BaseRelationship::DstCardLabel };

		if(!base_rel->isSelfRelationship() &&
				line_conn_mode==ConnectFkToPk && rel_type!=BaseRelationship::RelationshipNn)
		{
			for(idx=0; idx < 2; idx++)
			{
				pos=conn_points[idx];
				da=labels[idx]->boundingRect().height()/2.0;

				if((rel_type!=BaseRelationship::RelationshipFk && pos.x() < tables[idx]->pos().x()) ||
						(rel_type==BaseRelationship::RelationshipFk && pos.x() >= tables[idx]->pos().x()))
				{
					x=pos.x() - (labels[idx]->boundingRect().width() * 0.55);
				}
				else
				{
					x=pos.x() - (labels[idx]->boundingRect().width() * 0.45);
				}

				configureLabelPosition(label_ids[idx], x, pos.y() - da);
			}
		}
		else
		{
			QLineF::IntersectType inter_type;

			lins[0]=lines[0]->line();
			lins[1]=lines[lines.size()-1]->line();

			/* Creating lines that represents the tables border in order to calculate the
				 position of the cardinality labels via intersection point */
			for(idx=0; idx < 2; idx++)
			{
				rect=tables[idx]->boundingRect();
				pos=tables[idx]->pos();

				borders[idx][0].setPoints(pos, QPointF(pos.x(), pos.y() + rect.height()));
				borders[idx][1].setPoints(QPointF(pos.x(), pos.y() + rect.height()),
											QPointF(pos.x() + rect.width(), pos.y() + rect.height()));
				borders[idx][2].setPoints(QPointF(pos.x() + rect.width(), pos.y()),
											QPointF(pos.x() + rect.width(), pos.y() + rect.height()));
				borders[idx][3].setPoints(pos, QPointF(pos.x() + rect.width(), pos.y()));
			}

			for(idx=0; idx < 2; idx++)
			{
				for(i1=0; i1 < 4; i1++)
				{
					inter_type = lins[idx].intersects(borders[idx][i1], &p_int);

					if(inter_type == QLineF::BoundedIntersection)
					{
						if(idx==0)
							lins[idx].setP1(p_int);
						else
							lins[idx].setP2(p_int);
						break;
					}
				}
			}

			for(idx=0; idx < 2; idx++)
			{
				if(idx==0)
				{
					pi=lins[idx].p1();
					pf=lins[idx].p2();
				}
				else
				{
					pi=lins[idx].p2();
					pf=lins[idx].p1();
				}

				dl=labels[label_ids[idx]]->boundingRect().width()/2.0;
				da=labels[label_ids[idx]]->boundingRect().height()/2.0;

				x=pi.x() - dl;
				y=pi.y() - da;

				tab_rect.setTopLeft(tables[idx]->pos());
				tab_rect.setSize(tables[idx]->boundingRect().size());

				rect.setTopLeft(QPointF(x,y));
				rect.setSize(labels[idx]->boundingRect().size());

				if(rect.contains(tab_rect.bottomRight()))
				{ x+=dl + h_space; y+=da + v_space; }
				else if(rect.contains(tab_rect.bottomLeft()))
				{ x-=dl + h_space; y+=da + v_space; }
				else if(rect.contains(tab_rect.topLeft()))
				{ x-=dl + h_space; y-=da + v_space; }
				else if(rect.contains(tab_rect.topRight()))
				{ x+=dl + h_space; y-=da + v_space; }
				else
				{
					if(tab_rect.contains(rect.bottomLeft()) && tab_rect.contains(rect.bottomRight()))
						y-=da + v_space;
					else if(tab_rect.contains(rect.topLeft()) && tab_rect.contains(rect.topRight()))
						y+=da + v_space;

					if(tab_rect.contains(rect.topRight()) && tab_rect.contains(rect.bottomRight()))
						x-=dl + h_space;
					else if(tab_rect.contains(rect.topLeft()) && tab_rect.contains(rect.bottomLeft()))
						x+=dl + h_space;
				}

				configureLabelPosition(label_ids[idx], x, y);
			}
		}
	}
}

void RelationshipView::configureLabelPosition(BaseRelationship::LabelId label_id, double x, double y)
{
	if(label_id > BaseRelationship::RelNameLabel)
		throw Exception(ErrorCode::RefObjectInvalidIndex ,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(labels[label_id])
	{
		BaseRelationship *base_rel=this->getUnderlyingObject();
		QTextCharFormat char_fmt;
		QPointF label_dist;

		labels_ini_pos[label_id]=QPointF(x,y);
		label_dist=base_rel->getLabelDistance(label_id);

		if(!std::isnan(label_dist.x()))
		{
			x+=label_dist.x();
			y+=label_dist.y();
		}

		labels[label_id]->setPos(x,y);
		labels[label_id]->setToolTip(this->toolTip());

		char_fmt=BaseObjectView::getFontStyle(Attributes::Label);
		char_fmt.setFontPointSize(char_fmt.fontPointSize() * 0.90);
		labels[label_id]->setFontStyle(char_fmt);
		labels[label_id]->setColorStyle(BaseObjectView::getFillStyle(Attributes::Label),
										BaseObjectView::getBorderStyle(Attributes::Label));
		dynamic_cast<Textbox *>(labels[label_id]->getUnderlyingObject())->setModified(true);
	}
}

double RelationshipView::getDefaultPenWidth()
{
	Relationship *rel = dynamic_cast<Relationship *>(getUnderlyingObject());

	// Adjusting the relationship lines thickness according to the screen dpi
	if(BaseObjectView::getScreenDpiFactor() > 1)
	{
		if(rel && rel->isIdentifier())
			return  ObjectBorderWidth * BaseObjectView::getScreenDpiFactor() * 2;

		return  ObjectBorderWidth * BaseObjectView::getScreenDpiFactor() * 1.25;
	}

	if(rel && rel->isIdentifier())
		return ObjectBorderWidth * 2.25;

	return ObjectBorderWidth * 1.50;
}

void RelationshipView::configureBoundingRect()
{
	rel_shape.clear();
	setFlag(QGraphicsItem::ItemClipsToShape);
	prepareGeometryChange();

	for(auto & label : labels)
	{
		if(!label || !label->isVisible())
			continue;

		rel_shape.addRect(label->mapRectToScene(label->boundingRect()));
	}

	QPainterPathStroker ps;
	ps.setWidth(LineSelStrokeWidth);

	if(!use_curved_lines)
	{
		QPainterPath ln_path;

		for(auto &line : lines)
		{
			if(!line->isVisible())
				continue;

			ln_path = QPainterPath(line->line().p1());
			ln_path.lineTo(line->line().p2());
			rel_shape.addPath(ps.createStroke(ln_path));
		}
	}
	else
	{
		for(auto &curve : curves)
		{
			if(!curve->isVisible())
				continue;

			rel_shape.addPath(ps.createStroke(curve->path()));
		}
	}

	bounding_rect = rel_shape.boundingRect();
}

void RelationshipView::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	/* Workaround: For some unknown reason, an artifact rectangle is being draw
	 * around the relationship everytime it is selected. To avoid that we force
	 * the painter to have no pen and the clipping area to be the exposed area
	 * of the object */
	painter->save();
	painter->setPen(Qt::NoPen);
	painter->setClipRect(option->exposedRect);
	BaseObjectView::paint(painter, option, widget);
	painter->restore();
}
