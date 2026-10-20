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

#include "relationshipview.h"
#include "relationship.h"
#include "tableview.h"
#include "utilsns.h"
#include "objectsscene.h"
#include <queue>
#include <limits>

bool RelationshipView::hide_name_label {false};
bool RelationshipView::use_crows_foot {true};

RelationshipView::LineConnectionMode RelationshipView::line_conn_mode { RelationshipView::ConnectFkToPk };

RelationshipView::LineStyle RelationshipView::line_style { RelationshipView::OrthogonalLines };

template<class T>
void RelationshipView::trimGroupItems(QGraphicsItemGroup *group, std::vector<T *> &items, size_t count)
{
	T *item = nullptr;

	while(items.size() > count)
	{
		item = items.back();
		items.pop_back();
		group->removeFromGroup(item);
		delete item;
	}
}

void RelationshipView::getFkPkConnSides(const QRectF &ref_rect, const QRectF &rec_rect,
																				BaseTableView::ConnectionPoint &pk_type, BaseTableView::ConnectionPoint &fk_type,
																				bool &same_sides)
{
	if(ref_rect.intersects(rec_rect))
	{
		same_sides = true;

		if(rec_rect.center().x() >= ref_rect.center().x())
			pk_type = fk_type = BaseTableView::LeftConnPoint;
		else
			pk_type = fk_type = BaseTableView::RightConnPoint;
	}
	else if(ref_rect.right() <= rec_rect.left())
	{
		pk_type = BaseTableView::RightConnPoint;
		fk_type = BaseTableView::LeftConnPoint;
	}
	else
	{
		pk_type = BaseTableView::LeftConnPoint;
		fk_type = BaseTableView::RightConnPoint;
	}
}

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
	curve_end_lens[0]=curve_end_lens[1]=0;
	route_stubs[0]=route_stubs[1]=0;
	zone_retries=0;

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

	// The bumps of the lines that crossed this one must be updated
	if(!route_points.empty())
		notifyRouteChanged();

	for(auto &curve : curves)
	{
		this->removeFromGroup(curve);
		delete curve;
	}

	for(auto &line_circle : line_circles)
	{
		this->removeFromGroup(line_circle);
		delete line_circle;
	}

	for(auto &label : labels)
	{
		if(label)
		{
			this->removeFromGroup(label);
			delete label;
		}
	}

	for(auto &attr : attributes)
	{
		this->removeFromGroup(attr);
		delete attr;
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

	//round_cf_descriptors aren't always parented under cf_descriptors, so they need their own cleanup
	for(auto &round_cf_descriptor : round_cf_descriptors)
		delete round_cf_descriptor;

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

void RelationshipView::setLineStyle(LineStyle style)
{
	line_style = style;
}

RelationshipView::LineStyle RelationshipView::getLineStyle()
{
	return line_style;
}

void RelationshipView::invalidateRoute(bool reset_retries)
{
	if(reset_retries)
		zone_retries = 0;

	tab_hashes[BaseRelationship::SrcTable].clear();
	tab_hashes[BaseRelationship::DstTable].clear();
	configureLine();
}

void RelationshipView::notifyRouteChanged()
{
	ObjectsScene *obj_scene = dynamic_cast<ObjectsScene *>(scene());

	if(obj_scene)
		obj_scene->requestBumpsUpdate();
}

void RelationshipView::setBumps(const std::vector<QPointF> &new_bumps)
{
	if(new_bumps == bumps || curves.empty() || route_points.empty())
		return;

	bumps = new_bumps;
	curves[0]->setPoints(route_points, RoundedCornerRadius * BaseObjectView::getScreenDpiFactor(),
											 bumps, BumpRadius * BaseObjectView::getScreenDpiFactor(),
											 curve_end_lens[0], curve_end_lens[1]);
	configureBoundingRect();
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
	/* The first configuration of the line is done in the constructor, when there's no scene yet to
	 * look for the tables in the way of an orthogonal line. So we redo it when the scene is assigned */
	else if(change==ItemSceneHasChanged)
	{
		if(scene() && line_style == OrthogonalLines && tables[0] && tables[1])
			invalidateRoute();
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

			if(!base_rel->isSelfRelationship() && line_style != OrthogonalLines)
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
						if(line_style == CurvedLines)
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

bool RelationshipView::isLineUpdateNeeded()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::TableId src_tab = BaseRelationship::SrcTable,
			dst_tab = BaseRelationship::DstTable;

	/* We skip the line configuration if the line is still being configured, or the bounding rectangles
	 * of the involved tables are not valid (they wasn't rendered/configured yet) or the
	 * hash codes of both tables are unchanged since the last call to this method */
	return !(configuring_line ||

					 BaseTable::isGeomChangesIgnored() ||

					 !BaseGraphicObject::isUpdatesEnabled() ||

					 !tables[src_tab]->boundingRect().isValid() ||

					 !tables[dst_tab]->boundingRect().isValid() ||

					 ((!tab_hashes[src_tab].isEmpty() &&
						 tab_hashes[src_tab] == base_rel->getTable(src_tab)->getHashCode()) &&

						(!tab_hashes[dst_tab].isEmpty() &&
						 tab_hashes[dst_tab] == base_rel->getTable(dst_tab)->getHashCode())));
}

void RelationshipView::updateTablesHashes()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();

	tab_hashes[BaseRelationship::SrcTable] = base_rel->getTable(BaseRelationship::SrcTable)->getHashCode();
	tab_hashes[BaseRelationship::DstTable] = base_rel->getTable(BaseRelationship::DstTable)->getHashCode();
}

void RelationshipView::syncPlaceholderConnection()
{
	//Reconnect the tables if the placeholder usage changes
	if(using_placeholders != BaseObjectView::isPlaceholderEnabled())
	{
		connectTables();
		using_placeholders = BaseObjectView::isPlaceholderEnabled();
	}
}

void RelationshipView::configureLine()
{
	configureToolTip();

	if(!isLineUpdateNeeded())
		return;

	updateTablesHashes();
	syncPlaceholderConnection();
	discardOutdatedSegmentShifts();

	configuring_line=true;

	LineGeometry geom = computeLineGeometry();

	//Configuring the relationship line, its connectors, end circles and curves
	QPen pen = getLinePen();
	configureConnectorLines(geom, pen);
	configureLineSegments(geom.points, pen);
	configureLineCircles(geom.p_central, pen);
	configureCurves(geom);

	this->configureDescriptor();

	this->configureCrowsFootDescriptors();

	this->configureLabels();

	this->configureProtectedIcon();

	configureBoundingRect();

	configuring_line=false;
}

void RelationshipView::swapIdentifierTables()
{
	Relationship *rel = dynamic_cast<Relationship *>(this->getUnderlyingObject());

	if(rel &&
		 rel->getRelationshipType() == Relationship::Relationship11 &&
		 rel->isIdentifier())
	{
		tables[0] = dynamic_cast<BaseTableView *>(rel->getReferenceTable()->getOverlyingObject());
		tables[1] = dynamic_cast<BaseTableView *>(rel->getReceiverTable()->getOverlyingObject());
	}
}

RelationshipView::LineGeometry RelationshipView::computeLineGeometry()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
	LineGeometry geom;

	end_zones.clear();

	if(base_rel->isSelfRelationship())
	{
		configureSelfRelGeometry(geom);
		addEndPoints(geom);
	}
	else if(line_style == OrthogonalLines)
		configureOrthogonalGeometry(geom);
	else
	{
		bool rel_1n = (rel_type == Relationship::Relationship11 ||
									 rel_type == Relationship::Relationship1n ||
									 rel_type == Relationship::RelationshipFk);

		swapIdentifierTables();

		if(line_conn_mode == ConnectCenterPoints || line_conn_mode == ConnectTableEdges || !rel_1n)
			configureCenterGeometry(geom);
		else
			configureFkToPkGeometry(geom);

		geom.points = base_rel->getPoints();
		configureGraphicPoints(geom.points);

		if(line_conn_mode != ConnectTableEdges)
			addEndPoints(geom);
		else
			configureTableEdgesGeometry(geom);
	}

	return geom;
}

void RelationshipView::configureSelfRelGeometry(LineGeometry &geom)
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	double fnt_factor = font_config[Attributes::Global].font().pointSizeF() / DefaultFontSize,
			pos_factor = 0, offset = 0;
	unsigned rel_cnt = tables[0]->getConnectedRelsCount(base_rel->getTable(BaseRelationship::SrcTable),
																											base_rel->getTable(BaseRelationship::DstTable));
	QPointF pos = tables[0]->pos();
	QRectF rect = tables[0]->boundingRect();

	/* Sefl-relationshihp line format:

				+----<>
				|     |
 ----------   |
 | Table  |---+
 ----------

 This line cannot be moved or have points added */
	if(rel_cnt > 1)
	{
		int idx = tables[0]->getConnectedRelationshipIndex(base_rel, true);
		double min_val = std::min<double>(rect.width(), rect.height());

		if(idx < 0) idx =0;
		pos_factor = min_val * 0.08 * idx;
	}

	geom.p_central[0].setX(pos.x() + rect.width());
	geom.p_central[0].setY(pos.y() + (rect.height() / 3.0) + pos_factor);
	geom.p_central[1].setX(pos.x() + (rect.width() / 1.5) - pos_factor);
	geom.p_central[1].setY(pos.y());

	if(geom.p_central[0].y() > pos.y() + rect.height())
		geom.p_central[0].setY(pos.y() + rect.height());

	if(geom.p_central[1].x() < pos.x())
		geom.p_central[1].setX(pos.x());

	offset = use_crows_foot ? 30 : 20;

	geom.points.push_back(QPointF(geom.p_central[0].x() + (offset * fnt_factor) + pos_factor,  geom.p_central[0].y()));
	geom.points.push_back(QPointF(geom.p_central[0].x() + (offset * fnt_factor) + pos_factor,  geom.p_central[1].y() - (offset * fnt_factor) - pos_factor));
	geom.points.push_back(QPointF(geom.p_central[1].x(),  geom.p_central[1].y() - (offset * fnt_factor) - pos_factor));

	base_rel->setPoints(geom.points);
}

void RelationshipView::configureCenterGeometry(LineGeometry &geom)
{
	geom.p_central[BaseRelationship::SrcTable] = tables[BaseRelationship::SrcTable]->getCenter();
	geom.p_central[BaseRelationship::DstTable] = tables[BaseRelationship::DstTable]->getCenter();

	//Destroying the fk and pk connection lines when the line mode changes
	removeConnectorLines();
}

void RelationshipView::configureFkToPkGeometry(LineGeometry &geom)
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	Relationship *rel = dynamic_cast<Relationship *>(base_rel);
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
	QPointF pnt;
	QRectF rec_tab_rect, ref_tab_rect;
	double fk_py = 0, pk_py = 0, fk_px = 0, pk_px = 0;
	std::vector<Constraint *> fks;
	Table *ref_tab = nullptr, *rec_tab = nullptr;
	TableView *ref_tab_view = nullptr, *rec_tab_view = nullptr;
	unsigned cnt = 0;
	BaseTableView::ConnectionPoint pk_pnt_type = BaseTableView::LeftConnPoint,
			fk_pnt_type = BaseTableView::LeftConnPoint;

	if(!rel)
	{
		ref_tab = dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::DstTable));
		rec_tab = dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::SrcTable));
	}
	else
	{
		ref_tab = dynamic_cast<Table *>(rel->getReferenceTable());
		rec_tab = dynamic_cast<Table *>(rel->getReceiverTable());
	}

	ref_tab_view = dynamic_cast<TableView *>(ref_tab->getOverlyingObject());
	rec_tab_view = dynamic_cast<TableView *>(rec_tab->getOverlyingObject());

	/* We determine the fk to pk connections only if both tables are not fully collapsed.
	 * Otherwise, the tables' center points are used as connection point of the relationship */
	if(rec_tab->getCollapseMode() != BaseTable::AllAttribsCollapsed &&
		 ref_tab->getCollapseMode() != BaseTable::AllAttribsCollapsed)
	{
		rec_tab->getForeignKeys(fks, true, ref_tab);

		//Create the table's rectangles to detect where to connect the relationship
		ref_tab_rect = QRectF(ref_tab_view->pos(), ref_tab_view->boundingRect().size());

		//In this case the receiver table rect Y must be equal to reference table Y in order to do the correct comparison
		rec_tab_rect = QRectF(QPointF(rec_tab_view->pos().x(),
																	ref_tab_view->pos().y()), rec_tab_view->boundingRect().size());

		getFkPkConnSides(ref_tab_rect, rec_tab_rect, pk_pnt_type, fk_pnt_type, geom.conn_same_sides);

		for(auto &constr : fks)
		{
			cnt = constr->getColumnCount(Constraint::SourceCols);

			for(unsigned i = 0; i < cnt; i++)
			{
				pnt = rec_tab_view->getConnectionPoints(constr->getColumn(i, Constraint::SourceCols), fk_pnt_type);
				fk_py += pnt.y();
				fk_px = pnt.x();
				geom.fk_points.push_back(this->mapFromItem(rec_tab_view, pnt));

				pnt = ref_tab_view->getConnectionPoints(constr->getColumn(i, Constraint::ReferencedCols), pk_pnt_type);
				pk_py += pnt.y();
				pk_px = pnt.x();
				geom.pk_points.push_back(this->mapFromItem(ref_tab_view, pnt));
			}
		}
	}

	if(!fks.empty())
	{
		double pk_dx = (pk_pnt_type == BaseTableView::LeftConnPoint ? -ConnLineLength : ConnLineLength),
				fk_dx = (fk_pnt_type == BaseTableView::LeftConnPoint ? -ConnLineLength : ConnLineLength);

		geom.pk_pnt = this->mapFromItem(ref_tab_view, QPointF(pk_px + pk_dx, pk_py / geom.pk_points.size()));
		geom.fk_pnt = this->mapFromItem(rec_tab_view, QPointF(fk_px + fk_dx, fk_py / geom.fk_points.size()));
	}
	else
	{
		/* Fallback configuration: if one of the tables are collapsed or if no fk was found in the
		 * receiver table uses the tables' center points to configure the line in order to avoid glitched lines.
		 * The second situation may happen when the relationship is being validated and the needed fk was not
		 * created yet. In a second interaction of the rel. validation they are created
		 * and the relationship line is properly configured */
		geom.pk_pnt = ref_tab_view->getCenter();
		geom.fk_pnt = rec_tab_view->getCenter();
	}

	if(rel_type == Relationship::RelationshipFk)
	{
		geom.p_central[1] = geom.pk_pnt;
		geom.p_central[0] = geom.fk_pnt;
	}
	else
	{
		geom.p_central[0] = geom.pk_pnt;
		geom.p_central[1] = geom.fk_pnt;
	}

	//The connector lines leave the tables horizontally, but only when the fk/pk columns were found
	if(!fks.empty())
	{
		QPointF pk_dir((pk_pnt_type == BaseTableView::LeftConnPoint ? -1 : 1), 0),
				fk_dir((fk_pnt_type == BaseTableView::LeftConnPoint ? -1 : 1), 0);

		if(rel_type == Relationship::RelationshipFk)
		{
			geom.conn_dirs[1] = pk_dir;
			geom.conn_dirs[0] = fk_dir;
		}
		else
		{
			geom.conn_dirs[0] = pk_dir;
			geom.conn_dirs[1] = fk_dir;
		}
	}
}

void RelationshipView::addEndPoints(LineGeometry &geom)
{
	conn_points[0] = geom.p_central[0];
	conn_points[1] = geom.p_central[1];

	geom.points.insert(geom.points.begin(), geom.p_central[0]);
	geom.points.push_back(geom.p_central[1]);
}

bool RelationshipView::hasEndDescriptors()
{
	BaseRelationship::RelType rel_type = this->getUnderlyingObject()->getRelationshipType();

	return rel_type == BaseRelationship::Relationship11 ||
				 rel_type == BaseRelationship::Relationship1n ||
				 rel_type == BaseRelationship::RelationshipNn ||
				 rel_type == BaseRelationship::RelationshipFk;
}

double RelationshipView::getEdgeSizeFactor(unsigned tab_idx)
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
	double font_factor = BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();

	if(rel_type == BaseRelationship::RelationshipGen ||
		 rel_type == BaseRelationship::RelationshipDep ||
		 rel_type == BaseRelationship::RelationshipPart)
		return 0.40;

	if(!use_crows_foot)
		return 0.65 * font_factor;

	if(rel_type == BaseRelationship::RelationshipNn ||
		 (tab_idx == 1 && rel_type == BaseRelationship::RelationshipFk) ||
		 (tab_idx == 0 && rel_type == BaseRelationship::Relationship1n && base_rel->isTableMandatory(BaseRelationship::SrcTable)) ||
		 (tab_idx == 0 && rel_type == BaseRelationship::Relationship11 && base_rel->isTableMandatory(BaseRelationship::SrcTable)) ||
		 (tab_idx == 1 && rel_type == BaseRelationship::Relationship11 && base_rel->isTableMandatory(BaseRelationship::DstTable)))
		return 1 * font_factor;

	return 1.5 * font_factor;
}

double RelationshipView::getEdgeBorderFactor(unsigned tab_idx)
{
	BaseRelationship::RelType rel_type = this->getUnderlyingObject()->getRelationshipType();

	if(use_crows_foot &&
		 !(rel_type == BaseRelationship::RelationshipPart ||
			 rel_type == BaseRelationship::RelationshipGen ||
			 rel_type == BaseRelationship::RelationshipDep ||
			 rel_type == BaseRelationship::Relationship11 ||
			 (tab_idx == 0 && rel_type == BaseRelationship::Relationship1n) ||
			 (tab_idx == 1 && rel_type == BaseRelationship::RelationshipFk)))
		return ConnLineLength * 0.75;

	return ConnLineLength * 0.30;
}

QPointF RelationshipView::configureEdgeConnection(unsigned tab_idx, const QLineF &line, LineGeometry &geom)
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	QRectF brect(tables[tab_idx]->pos(), tables[tab_idx]->boundingRect().size());
	QPolygonF pol(brect);
	QLineF edge;
	QPointF pi, center = tables[tab_idx]->getCenter();
	double size_factor = getEdgeSizeFactor(tab_idx),
			border_factor = getEdgeBorderFactor(tab_idx),
			min_lim = 0, max_lim = 0, conn_rels_factor = 0;
	unsigned conn_rels_cnt = tables[tab_idx]->getConnectedRelsCount(base_rel->getTable(BaseRelationship::SrcTable),
																																	base_rel->getTable(BaseRelationship::DstTable));

	conn_rels_factor = conn_rels_cnt == 1 ? 1 : 0.08 * (tables[tab_idx]->getConnectedRelationshipIndex(base_rel));

	for(int idx = 0; idx < pol.size() - 1; idx++)
	{
		edge.setP1(pol.at(idx));
		edge.setP2(pol.at(idx + 1));

		if(line.intersects(edge, &pi) != QLineF::BoundedIntersection)
			continue;

		/* Adjusting the intersection point if there're more than one relationship connected the current table
		 * this will cause all relationships to be aligned together */
		if(conn_rels_cnt > 1)
		{
			double max_dim = std::max<double>(brect.height(), brect.width());
			int signal = 0;

			if(edge.dx() == 0)
			{
				signal = edge.dy()/fabs(edge.dy()) * (tab_idx == 0 ? 1 : -1);
				pi.setY(pi.y() - ((conn_rels_factor * max_dim) * signal));

				// Adjusting the position of the interesection point to make is as close to the center of the edge as possible
				pi.setY(pi.y() + ((max_dim * 0.05) * signal));
			}

			if(edge.dy() == 0)
			{
				signal = edge.dx()/fabs(edge.dx()) * (tab_idx == 0 ? 1 : -1);
				pi.setX(pi.x() - ((conn_rels_factor * max_dim) * signal));

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

		conn_points[tab_idx] = geom.p_central[tab_idx] = pi;

		if(edge.dx() == 0)
		{
			if(pi.x() < center.x())
				pi.setX(pi.x() - (ConnLineLength * size_factor));
			else
				pi.setX(pi.x() + (ConnLineLength * size_factor));

			geom.conn_vert_sides[tab_idx] = true;
		}
		else
		{
			if(pi.y() < center.y())
				pi.setY(pi.y() - (ConnLineLength * size_factor));
			else
				pi.setY(pi.y() + (ConnLineLength * size_factor));

			geom.conn_horiz_sides[tab_idx] = true;
		}

		return pi;
	}

	return geom.p_central[tab_idx];
}

void RelationshipView::configureTableEdgesGeometry(LineGeometry &geom)
{
	QPointF p_aux[2];
	QLineF line;

	for(unsigned tab_idx = 0; tab_idx < 2; tab_idx++)
	{
		line = QLineF(tables[0]->getCenter(), tables[1]->getCenter());

		//If the user added points, the line starts at the table's center and goes to the nearest point
		if(!geom.points.empty())
		{
			if(tab_idx == 0)
				line = QLineF(tables[0]->getCenter(), geom.points.front());
			else
				line = QLineF(tables[1]->getCenter(), geom.points.back());
		}

		p_aux[tab_idx] = configureEdgeConnection(tab_idx, line, geom);
	}

	geom.points.insert(geom.points.begin(), geom.p_central[0]);
	geom.points.insert(geom.points.begin() + 1, p_aux[0]);

	geom.points.push_back(p_aux[1]);
	geom.points.push_back(geom.p_central[1]);
}
void RelationshipView::configureOrthogonalGeometry(LineGeometry &geom)
{
	BaseRelationship::RelType rel_type = this->getUnderlyingObject()->getRelationshipType();
	bool rel_1n = (rel_type == Relationship::Relationship11 ||
								 rel_type == Relationship::Relationship1n ||
								 rel_type == Relationship::RelationshipFk);
	double stubs[2] = { 0, 0 }, mid_shift = 0;

	swapIdentifierTables();

	if(line_conn_mode == ConnectFkToPk && rel_1n)
		configureFkToPkGeometry(geom);
	else
	{
		configureCenterGeometry(geom);

		if(line_conn_mode == ConnectTableEdges)
			configureOrthogonalEdges(geom, stubs, mid_shift);
	}

	//The points added by the user are ignored in orthogonal lines
	configureGraphicPoints({});

	conn_points[0] = geom.p_central[0];
	conn_points[1] = geom.p_central[1];

	geom.points = getOrthogonalRoute(geom.p_central[0], geom.conn_dirs[0], stubs[0],
																	 geom.p_central[1], geom.conn_dirs[1], stubs[1], mid_shift);

	if(isObstacleAvoidanceEnabled())
		geom.points = avoidObstacles(geom.points, geom.conn_dirs, stubs);

	base_route = geom.points;
	route_stubs[0] = stubs[0];
	route_stubs[1] = stubs[1];
	geom.points = applySegmentShifts(geom.points, stubs);
}

void RelationshipView::discardOutdatedSegmentShifts()
{
	constexpr double Tolerance = 0.01;
	QRectF rect;
	bool changed = false;

	for(unsigned idx = 0; idx < 2; idx++)
	{
		rect = QRectF(tables[idx]->pos(), tables[idx]->boundingRect().size());

		if(!shift_tab_rects[idx].isNull() &&
			 (std::fabs(rect.x() - shift_tab_rects[idx].x()) > Tolerance ||
				std::fabs(rect.y() - shift_tab_rects[idx].y()) > Tolerance ||
				std::fabs(rect.width() - shift_tab_rects[idx].width()) > Tolerance ||
				std::fabs(rect.height() - shift_tab_rects[idx].height()) > Tolerance))
			changed = true;

		shift_tab_rects[idx] = rect;
	}

	if(changed)
		getUnderlyingObject()->resetSegmentShifts();
}

QRectF RelationshipView::getSegmentShiftLimits()
{
	double margin = ConnLineLength * BaseObjectView::getScreenDpiFactor();
	QRectF limits = getRouteRect(base_route);

	for(auto &tab : tables)
		limits = limits.united(QRectF(tab->pos(), tab->boundingRect().size()));

	return limits.adjusted(-margin, -margin, margin, margin);
}

double RelationshipView::clampSegmentShift(const std::vector<QPointF> &route, size_t idx, double shift,
																					 const double *stubs, const QRectF &limits)
{
	constexpr double Epsilon = 0.01;
	double min_len = RoundedCornerRadius * BaseObjectView::getScreenDpiFactor(),
			lo = 0, hi = 0, prev_len = 0, next_len = 0, prev_min = 0, next_min = 0;
	size_t seg_cnt = route.size() - 1;
	bool horiz = std::fabs(route[idx].y() - route[idx + 1].y()) < Epsilon;

	if(idx < 1 || idx + 1 >= seg_cnt)
		return 0;

	// The neighbor segments are perpendicular to the displaced one, so their lengths are measured along the displacement axis
	if(horiz)
	{
		lo = limits.top() - route[idx].y();
		hi = limits.bottom() - route[idx].y();
		prev_len = route[idx].y() - route[idx - 1].y();
		next_len = route[idx + 2].y() - route[idx + 1].y();
	}
	else
	{
		lo = limits.left() - route[idx].x();
		hi = limits.right() - route[idx].x();
		prev_len = route[idx].x() - route[idx - 1].x();
		next_len = route[idx + 2].x() - route[idx + 1].x();
	}

	prev_min = (idx == 1 ? std::max(min_len, stubs[0]) : min_len);
	next_min = (idx + 2 == seg_cnt ? std::max(min_len, stubs[1]) : min_len);

	// The previous segment ends at the displaced one and the next one starts at it
	if(prev_len > 0)
		lo = std::max(lo, prev_min - prev_len);
	else
		hi = std::min(hi, -prev_len - prev_min);

	if(next_len > 0)
		hi = std::min(hi, next_len - next_min);
	else
		lo = std::max(lo, next_len + next_min);

	if(lo > hi)
		return 0;

	return std::clamp(shift, lo, hi);
}

std::vector<QPointF> RelationshipView::applySegmentShifts(const std::vector<QPointF> &route, const double *stubs)
{
	constexpr double Epsilon = 0.01;
	std::map<unsigned, BaseRelationship::SegmentShift> shifts = getUnderlyingObject()->getSegmentShifts();
	std::vector<QPointF> result = route;
	QRectF limits;
	QPointF offset;
	double shift = 0;
	bool horiz = false;

	if(shifts.empty() || route.size() < 4)
		return route;

	limits = getSegmentShiftLimits();

	for(auto &itr : shifts)
	{
		// Only the inner segments are displaced, the first and the last ones are anchored to the tables
		if(itr.first < 1 || itr.first + 2 >= route.size())
			continue;

		horiz = std::fabs(result[itr.first].y() - result[itr.first + 1].y()) < Epsilon;

		if(horiz != (itr.second.orientation == Qt::Horizontal))
			continue;

		shift = clampSegmentShift(result, itr.first, itr.second.value, stubs, limits);
		offset = (horiz ? QPointF(0, shift) : QPointF(shift, 0));
		result[itr.first] += offset;
		result[itr.first + 1] += offset;
	}

	return result;
}

bool RelationshipView::getEdgeSide(const QRectF &rect, const QRectF &other_rect, bool &horiz, int &sign)
{
	double gap_x = std::max(other_rect.left() - rect.right(), rect.left() - other_rect.right()),
			gap_y = std::max(other_rect.top() - rect.bottom(), rect.top() - other_rect.bottom());

	//When the tables overlap there's no side to connect the line
	if(gap_x <= 0 && gap_y <= 0)
		return false;

	/* The line leaves the tables through the facing sides along the axis of the greatest
	 * gap between them: horizontal sides when they are side by side, vertical ones otherwise */
	horiz = (gap_x >= gap_y);

	if(horiz)
		sign = (other_rect.center().x() >= rect.center().x() ? 1 : -1);
	else
		sign = (other_rect.center().y() >= rect.center().y() ? 1 : -1);

	return true;
}

double RelationshipView::getEdgePreferredPos(const QRectF &rect, const QRectF &other_rect, bool horiz, double border)
{
	double lo = (horiz ? rect.top() : rect.left()),
			hi = (horiz ? rect.bottom() : rect.right()),
			overlap_lo = std::max(lo, (horiz ? other_rect.top() : other_rect.left())),
			overlap_hi = std::min(hi, (horiz ? other_rect.bottom() : other_rect.right()));

	/* If the tables overlap along the side's axis the line is connected inside the overlapping
	 * area, which makes it straight, otherwise it is connected at the middle of the side */
	if(overlap_hi - overlap_lo > border * 2)
		return (overlap_lo + overlap_hi) / 2;

	return (lo + hi) / 2;
}

double RelationshipView::getEdgeAnchorPos(unsigned tab_idx, bool horiz, int sign)
{
	struct Anchor {
		BaseRelationship *rel;
		double desired, other_pos;
	};

	BaseTableView *tab = tables[tab_idx], *other_tab = nullptr;
	QRectF rect(tab->pos(), tab->boundingRect().size()), other_rect;
	RelationshipView *rel_view = nullptr;
	std::vector<Anchor> anchors;
	std::vector<double> pos;
	BaseRelationship *base_rel = this->getUnderlyingObject();
	int rel_sign = 0, this_idx = -1;
	bool rel_horiz = false;
	double lo = (horiz ? rect.top() : rect.left()),
			hi = (horiz ? rect.bottom() : rect.right()),
			border = getEdgeBorderFactor(tab_idx),
			min_lim = lo + border, max_lim = hi - border,
			/* The crow's foot descriptors are about 20 units wide so their connection points need more room
			 * than the plain lines, otherwise the descriptors of neighbor relationships overlap */
			gap = ConnLineLength * (use_crows_foot ? 1.3 : 0.8) * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor(),
			sum_desired = 0, sum_pos = 0;
	size_t cnt = 0, cluster_start = 0;

	/* Gathering all relationships connected to the same side of the table, including this one,
	 * so they can be spread along the side instead of sharing the same connection point */
	for(auto &rel : tab->connected_rels)
	{
		rel_view = dynamic_cast<RelationshipView *>(rel->getOverlyingObject());

		if(!rel_view || rel->isSelfRelationship())
			continue;

		if(rel_view->tables[0] == tab)
			other_tab = rel_view->tables[1];
		else if(rel_view->tables[1] == tab)
			other_tab = rel_view->tables[0];
		else
			continue;

		other_rect = QRectF(other_tab->pos(), other_tab->boundingRect().size());

		if(!getEdgeSide(rect, other_rect, rel_horiz, rel_sign) || rel_horiz != horiz || rel_sign != sign)
			continue;

		anchors.push_back({ rel,
												getEdgePreferredPos(rect, other_rect, horiz,
																						rel_view->getEdgeBorderFactor(rel_view->tables[0] == tab ? 0 : 1)),
												(horiz ? other_rect.center().y() : other_rect.center().x()) });
	}

	/* Ordering the relationships by their preferred position and then by the position of the opposite
	 * table, so the lines tend to not cross each other. The object id makes the order stable */
	std::sort(anchors.begin(), anchors.end(), [](const Anchor &a1, const Anchor &a2) {
		if(a1.desired != a2.desired)
			return a1.desired < a2.desired;

		if(a1.other_pos != a2.other_pos)
			return a1.other_pos < a2.other_pos;

		return a1.rel->getObjectId() < a2.rel->getObjectId();
	});

	cnt = anchors.size();
	pos.resize(cnt);

	for(size_t idx = 0; idx < cnt; idx++)
	{
		if(anchors[idx].rel == base_rel)
			this_idx = idx;
	}

	if(this_idx < 0)
		return std::clamp(getEdgePreferredPos(rect, QRectF(tables[tab_idx == 0 ? 1 : 0]->pos(),
																											 tables[tab_idx == 0 ? 1 : 0]->boundingRect().size()),
																					horiz, border), std::min(min_lim, max_lim), std::max(min_lim, max_lim));

	if(max_lim < min_lim)
		min_lim = max_lim = (lo + hi) / 2;

	if(cnt > 1)
		gap = std::min(gap, (max_lim - min_lim) / (cnt - 1));

	/* Pushing apart the connection points that are too close to each other. Each group of points pushed
	 * together is centered around the original (preferred) position of its members, which keeps
	 * unrelated lines (e.g. straight ones) untouched */
	for(size_t idx = 0; idx <= cnt; idx++)
	{
		if(idx < cnt && idx > 0 && anchors[idx].desired < pos[idx - 1] + gap)
			pos[idx] = pos[idx - 1] + gap;
		else
		{
			if(idx > cluster_start)
			{
				sum_desired = sum_pos = 0;

				for(size_t cl_idx = cluster_start; cl_idx < idx; cl_idx++)
				{
					sum_desired += anchors[cl_idx].desired;
					sum_pos += pos[cl_idx];
				}

				for(size_t cl_idx = cluster_start; cl_idx < idx; cl_idx++)
					pos[cl_idx] += (sum_desired - sum_pos) / (idx - cluster_start);
			}

			if(idx < cnt)
			{
				cluster_start = idx;
				pos[idx] = anchors[idx].desired;
			}
		}
	}

	//Keeping the connection points inside the side's limits and apart from each other
	for(size_t idx = 0; idx < cnt; idx++)
		pos[idx] = std::max(pos[idx], (idx == 0 ? min_lim : pos[idx - 1] + gap));

	for(size_t idx = cnt; idx > 0; idx--)
		pos[idx - 1] = std::min(pos[idx - 1], (idx == cnt ? max_lim : pos[idx] - gap));

	return std::clamp(pos[this_idx], min_lim, max_lim);
}

void RelationshipView::configureOrthogonalEdges(LineGeometry &geom, double stubs[2], double &mid_shift)
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	QRectF rects[2] = { QRectF(tables[0]->pos(), tables[0]->boundingRect().size()),
											QRectF(tables[1]->pos(), tables[1]->boundingRect().size()) };
	QPointF dirs[2];
	bool horiz = false;
	int sign = 0, rel_idx = 0;
	unsigned conn_rels_cnt = tables[0]->getConnectedRelsCount(base_rel->getTable(BaseRelationship::SrcTable),
																														base_rel->getTable(BaseRelationship::DstTable));
	double ref_pos = 0, rel_shift = 0, perp_delta = 0, half_width = 0;
	QPointF end_pnt;

	//When the tables overlap the line keeps going through their centers
	if(!getEdgeSide(rects[0], rects[1], horiz, sign))
		return;

	dirs[0] = (horiz ? QPointF(sign, 0) : QPointF(0, sign));
	dirs[1] = -dirs[0];

	for(unsigned idx = 0; idx < 2; idx++)
	{
		ref_pos = getEdgeAnchorPos(idx, horiz, (horiz ? dirs[idx].x() : dirs[idx].y()));

		if(horiz)
			geom.p_central[idx] = QPointF((dirs[idx].x() > 0 ? rects[idx].right() : rects[idx].left()), ref_pos);
		else
			geom.p_central[idx] = QPointF(ref_pos, (dirs[idx].y() > 0 ? rects[idx].bottom() : rects[idx].top()));

		geom.conn_dirs[idx] = dirs[idx];
		stubs[idx] = ConnLineLength * getEdgeSizeFactor(idx);

		/* The crow's foot descriptors occupy the first part of the line segments at the ends. That part
		 * is kept straight and the segments are extended so the corner's curve fits right after it */
		if(use_crows_foot && hasEndDescriptors())
		{
			geom.end_lens[idx] = stubs[idx] * 1.2;
			stubs[idx] = geom.end_lens[idx] + (RoundedCornerRadius * BaseObjectView::getScreenDpiFactor());

			// The area of the descriptors: from the table's edge along the line, a little wider than the descriptor itself
			half_width = 11 * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor();
			end_pnt = geom.p_central[idx] + (dirs[idx] * geom.end_lens[idx]);

			if(horiz)
				end_zones.push_back(QRectF(QPointF(std::min(geom.p_central[idx].x(), end_pnt.x()), geom.p_central[idx].y() - half_width),
																	 QPointF(std::max(geom.p_central[idx].x(), end_pnt.x()), geom.p_central[idx].y() + half_width)));
			else
				end_zones.push_back(QRectF(QPointF(geom.p_central[idx].x() - half_width, std::min(geom.p_central[idx].y(), end_pnt.y())),
																	 QPointF(geom.p_central[idx].x() + half_width, std::max(geom.p_central[idx].y(), end_pnt.y()))));
		}
	}

	/* Relationships between the same tables have their middle segments displaced from each other.
	 * The displacement order depends on the direction of the line so the lines don't cross */
	if(conn_rels_cnt > 1)
	{
		for(auto &rel : tables[0]->connected_rels)
		{
			if(rel == base_rel)
				break;

			if((rel->getTable(BaseRelationship::SrcTable) == base_rel->getTable(BaseRelationship::SrcTable) &&
					rel->getTable(BaseRelationship::DstTable) == base_rel->getTable(BaseRelationship::DstTable)) ||
				 (rel->getTable(BaseRelationship::SrcTable) == base_rel->getTable(BaseRelationship::DstTable) &&
					rel->getTable(BaseRelationship::DstTable) == base_rel->getTable(BaseRelationship::SrcTable)))
				rel_idx++;
		}

		perp_delta = (horiz ? geom.p_central[1].y() - geom.p_central[0].y() :
													geom.p_central[1].x() - geom.p_central[0].x());
		rel_shift = rel_idx - ((conn_rels_cnt - 1) / 2.0);
		mid_shift = rel_shift * ConnLineLength * 0.5 * (perp_delta > 0 ? -1 : 1);
	}
}

std::vector<QPointF> RelationshipView::cleanRoute(const std::vector<QPointF> &route)
{
	std::vector<QPointF> pts;
	QPointF a, b;

	// Removing the duplicated points as well as the intermediate ones of collinear segments
	for(auto &pnt : route)
	{
		if(!pts.empty() && QLineF(pts.back(), pnt).length() < 0.01)
			continue;

		if(pts.size() >= 2)
		{
			a = pts[pts.size() - 2];
			b = pts.back();

			if(std::fabs(((b.x() - a.x()) * (pnt.y() - b.y())) - ((b.y() - a.y()) * (pnt.x() - b.x()))) < 0.01)
				pts.pop_back();
		}

		pts.push_back(pnt);
	}

	if(pts.size() < 2 && !route.empty())
		pts.push_back(route.back());

	return pts;
}

QRectF RelationshipView::getRouteRect(const std::vector<QPointF> &route)
{
	double left = 0, top = 0, right = 0, bottom = 0;

	if(route.empty())
		return QRectF();

	left = right = route.front().x();
	top = bottom = route.front().y();

	/* Not using QRectF::united() because it ignores the null rects (zero size),
	 * which are the ones that represent the isolated points */
	for(auto &pnt : route)
	{
		left = std::min(left, pnt.x());
		right = std::max(right, pnt.x());
		top = std::min(top, pnt.y());
		bottom = std::max(bottom, pnt.y());
	}

	return QRectF(QPointF(left, top), QPointF(right, bottom));
}

bool RelationshipView::isSegmentBlocked(const QPointF &p1, const QPointF &p2, const std::vector<QRectF> &obstacles)
{
	constexpr double Epsilon = 0.01;
	double left = std::min(p1.x(), p2.x()), right = std::max(p1.x(), p2.x()),
			top = std::min(p1.y(), p2.y()), bottom = std::max(p1.y(), p2.y());

	/* A horizontal/vertical segment is blocked when it crosses the inside of an obstacle.
	 * Segments that only touch the obstacle's borders are free, so they can run along them */
	for(auto &rect : obstacles)
	{
		if(left < rect.right() - Epsilon && right > rect.left() + Epsilon &&
			 top < rect.bottom() - Epsilon && bottom > rect.top() + Epsilon)
			return true;
	}

	return false;
}

bool RelationshipView::isRouteBlocked(const std::vector<QPointF> &route, const std::vector<QRectF> &obstacles)
{
	for(size_t idx = 1; idx < route.size(); idx++)
	{
		if(isSegmentBlocked(route[idx - 1], route[idx], obstacles))
			return true;
	}

	return false;
}

bool RelationshipView::isObstacleAvoidanceEnabled()
{
	ObjectsScene *obj_scene = dynamic_cast<ObjectsScene *>(scene());

	/* Without the scene there is no information about the other tables. Also, when the tables are being
	 * moved and the relationships are updated at every step (no placeholders), the obstacles are
	 * ignored to keep the movement fluid. The relationships are fixed when the movement ends */
	return obj_scene && !(obj_scene->isMovingObjects() && !BaseObjectView::isPlaceholderEnabled());
}

std::vector<QRectF> RelationshipView::getObstacles(const QRectF &region)
{
	std::vector<QRectF> obstacles;
	BaseTableView *tab = nullptr;

	if(!scene())
		return obstacles;

	for(auto &item : scene()->items(region, Qt::IntersectsItemBoundingRect))
	{
		tab = dynamic_cast<BaseTableView *>(item);

		if(tab && tab != tables[0] && tab != tables[1] && tab->isVisible())
			obstacles.push_back(QRectF(tab->pos(), tab->boundingRect().size()));
	}

	return obstacles;
}

std::vector<QRectF> RelationshipView::getEndZones(const QRectF &region)
{
	std::vector<QRectF> zones;
	RelationshipView *rel_view = nullptr;

	if(!scene())
		return zones;

	for(auto &item : scene()->items(region, Qt::IntersectsItemBoundingRect))
	{
		rel_view = dynamic_cast<RelationshipView *>(item);

		if(!rel_view || rel_view == this || !rel_view->isVisible())
			continue;

		for(auto &zone : rel_view->end_zones)
		{
			if(zone.intersects(region))
				zones.push_back(zone.adjusted(-ZoneClearance, -ZoneClearance, ZoneClearance, ZoneClearance));
		}
	}

	return zones;
}

std::vector<QPointF> RelationshipView::findOrthogonalPath(const QPointF &start, const QPointF &goal, int start_dir, int goal_dir,
																													const std::vector<QRectF> &obstacles)
{
	/* Search of the best path on the grid formed by the coordinates of the obstacles' borders (Hanan grid).
	 * The cost of a path is its length plus a penalty for each bend. The state of the search is the grid
	 * node plus the direction of the last move, which allows the bends to be accounted for. */
	constexpr double BendPenalty = 50;
	constexpr size_t MaxNodes = 40000;
	const int dx[4] = { 1, 0, -1, 0 }, dy[4] = { 0, 1, 0, -1 };

	struct QueueItem {
		double f, g;
		int state;

		bool operator < (const QueueItem &item) const {
			if(f != item.f)
				return f > item.f;

			if(g != item.g)
				return g < item.g;

			return state > item.state;
		}
	};

	auto unique_coords = [](std::vector<double> &coords) {
		std::sort(coords.begin(), coords.end());
		coords.erase(std::unique(coords.begin(), coords.end(), [](double c1, double c2) {
			return std::fabs(c1 - c2) < 0.01;
		}), coords.end());
	};

	auto index_of = [](const std::vector<double> &coords, double value) {
		return static_cast<int>(std::lower_bound(coords.begin(), coords.end(), value - 0.01) - coords.begin());
	};

	std::vector<double> xs { start.x(), goal.x() }, ys { start.y(), goal.y() }, dist;
	std::vector<int> parent;
	std::priority_queue<QueueItem> queue;
	std::vector<QPointF> path;

	for(auto &rect : obstacles)
	{
		xs.push_back(rect.left());
		xs.push_back(rect.right());
		ys.push_back(rect.top());
		ys.push_back(rect.bottom());
	}

	unique_coords(xs);
	unique_coords(ys);

	int nx = xs.size(), ny = ys.size(),
			start_x = index_of(xs, start.x()), start_y = index_of(ys, start.y()),
			goal_x = index_of(xs, goal.x()), goal_y = index_of(ys, goal.y());

	if(static_cast<size_t>(nx) * ny > MaxNodes)
		return path;

	dist.assign(nx * ny * 4, std::numeric_limits<double>::infinity());
	parent.assign(nx * ny * 4, -1);

	auto push_state = [&](int ix, int iy, int dir, double g, int from) {
		int state = ((iy * nx) + ix) * 4 + dir;

		if(g >= dist[state])
			return;

		dist[state] = g;
		parent[state] = from;
		queue.push({ g + std::fabs(xs[ix] - goal.x()) + std::fabs(ys[iy] - goal.y()), g, state });
	};

	if(start_dir >= 0)
		push_state(start_x, start_y, start_dir, 0, -1);
	else
	{
		for(int dir = 0; dir < 4; dir++)
			push_state(start_x, start_y, dir, 0, -1);
	}

	while(!queue.empty())
	{
		QueueItem item = queue.top();
		queue.pop();

		if(item.g > dist[item.state])
			continue;

		int cur_dir = item.state % 4, node = item.state / 4,
				ix = node % nx, iy = node / nx;

		if(ix == goal_x && iy == goal_y)
		{
			for(int state = item.state; state >= 0; state = parent[state])
				path.push_back(QPointF(xs[(state / 4) % nx], ys[(state / 4) / nx]));

			std::reverse(path.begin(), path.end());
			return path;
		}

		for(int dir = 0; dir < 4; dir++)
		{
			int new_x = ix + dx[dir], new_y = iy + dy[dir];
			double cost = 0;

			// Going back to where the line came from is never part of a best path
			if(dir == (cur_dir + 2) % 4 || new_x < 0 || new_x >= nx || new_y < 0 || new_y >= ny)
				continue;

			QPointF p1(xs[ix], ys[iy]), p2(xs[new_x], ys[new_y]);

			if(isSegmentBlocked(p1, p2, obstacles))
				continue;

			cost = item.g + QLineF(p1, p2).length() + (dir != cur_dir ? BendPenalty : 0);

			// The line must arrive at the goal in the direction that links it to the table
			if(new_x == goal_x && new_y == goal_y && goal_dir >= 0 && dir != goal_dir)
				cost += BendPenalty;

			push_state(new_x, new_y, dir, cost, item.state);
		}
	}

	return path;
}

std::vector<QPointF> RelationshipView::avoidObstacles(const std::vector<QPointF> &route, const QPointF *dirs, const double *stubs)
{
	constexpr size_t MaxObstacles = 80;
	const double regions_margins[2] = { 150, 600 };
	double clearance = ConnLineLength * 0.75, exit_len = 0;
	QRectF route_rect, region;
	std::vector<QRectF> obstacles, all_obstacles;
	std::vector<QPointF> path, result;
	QPointF ends[2];
	int dir_ids[2] = { -1, -1 };

	if(route.size() < 2)
		return route;

	// Converting the directions (unit vectors) to the indexes used by the path search
	for(int idx = 0; idx < 2; idx++)
	{
		if(dirs[idx].x() > 0)
			dir_ids[idx] = 0;
		else if(dirs[idx].y() > 0)
			dir_ids[idx] = 1;
		else if(dirs[idx].x() < 0)
			dir_ids[idx] = 2;
		else if(dirs[idx].y() < 0)
			dir_ids[idx] = 3;
	}

	/* The obstacles are the tables, with the clearance around them, plus the end zones
	 * (crow's foot areas) of the other relationships */
	auto get_all_obstacles = [&](const QRectF &rgn) {
		std::vector<QRectF> rects = getObstacles(rgn), zones = getEndZones(rgn);

		for(auto &rect : rects)
			rect.adjust(-clearance, -clearance, clearance, clearance);

		rects.insert(rects.end(), zones.begin(), zones.end());
		return rects;
	};

	route_rect = getRouteRect(route);

	for(int attempt = 0; attempt < 2; attempt++)
	{
		region = route_rect.adjusted(-regions_margins[attempt], -regions_margins[attempt],
																 regions_margins[attempt], regions_margins[attempt]);
		obstacles = get_all_obstacles(region);

		// If the route doesn't touch any obstacle there's nothing to do
		if(attempt == 0 && !isRouteBlocked(route, obstacles))
			return route;

		if(obstacles.size() > MaxObstacles)
			return route;

		/* The tables linked by the relationship are also obstacles for the search, otherwise the path could go
		 * through them. The exception is when the line is not connected to the table's border (no direction),
		 * which happens when it is connected to the table's center */
		all_obstacles = obstacles;

		for(int idx = 0; idx < 2; idx++)
		{
			if(dir_ids[idx] >= 0)
			{
				all_obstacles.push_back(QRectF(tables[idx]->pos(), tables[idx]->boundingRect().size()).
																adjusted(-clearance, -clearance, clearance, clearance));
			}
		}

		/* The search starts and ends away from the tables so there's room for the segments
		 * at the ends of the line (e.g. the crow's foot descriptors) */
		for(int idx = 0; idx < 2; idx++)
		{
			exit_len = (dir_ids[idx] >= 0 ? std::max(stubs[idx], clearance + 2) : 0);
			ends[idx] = (idx == 0 ? route.front() : route.back()) + (dirs[idx] * exit_len);
		}

		/* The line arrives at the second table moving against its direction, which is the reverse
		 * of the direction the line leaves that table */
		path = findOrthogonalPath(ends[0], ends[1], dir_ids[0], (dir_ids[1] >= 0 ? (dir_ids[1] + 2) % 4 : -1), all_obstacles);

		if(path.empty())
			continue;

		result.clear();
		result.push_back(route.front());
		result.insert(result.end(), path.begin(), path.end());
		result.push_back(route.back());
		result = cleanRoute(result);

		// The found path must not touch obstacles outside the region used in the search
		obstacles = get_all_obstacles(getRouteRect(result));

		if(!isRouteBlocked(result, obstacles))
			return result;
	}

	return route;
}

std::vector<QPointF> RelationshipView::getOrthogonalRoute(QPointF p0, QPointF dir0, double stub0,
																													 QPointF p1, QPointF dir1, double stub1, double mid_shift)
{
	auto swap_xy = [](const QPointF &pnt) { return QPointF(pnt.y(), pnt.x()); };
	bool horiz0 = (dir0.x() != 0 && dir0.y() == 0), horiz1 = (dir1.x() != 0 && dir1.y() == 0),
			vert0 = (dir0.x() == 0 && dir0.y() != 0), vert1 = (dir1.x() == 0 && dir1.y() != 0);
	std::vector<QPointF> route;
	double dist = 0, mid = 0, margin = 0;

	/* When the directions are unknown or incompatible (one horizontal and the other vertical)
	 * the line leaves the tables along the axis with the greatest distance between the points */
	if(!((horiz0 && horiz1) || (vert0 && vert1)))
	{
		double dx = p1.x() - p0.x(), dy = p1.y() - p0.y();

		if(std::fabs(dx) >= std::fabs(dy))
			dir0 = QPointF((dx >= 0 ? 1 : -1), 0);
		else
			dir0 = QPointF(0, (dy >= 0 ? 1 : -1));

		dir1 = -dir0;
		stub0 = stub1 = 0;
	}

	/* The route is always calculated for horizontal directions, the vertical
	 * ones are calculated by swapping the coordinates and swapping them back at the end */
	bool transposed = (dir0.x() == 0);

	if(transposed)
	{
		p0 = swap_xy(p0);
		p1 = swap_xy(p1);
		dir0 = swap_xy(dir0);
		dir1 = swap_xy(dir1);
	}

	dist = (p1.x() - p0.x()) * dir0.x();

	// The line leaves the tables in opposite directions: the middle segment lies between the points
	if(dir0.x() * dir1.x() < 0)
	{
		mid = (dist / 2) + mid_shift;

		// Keeps room for the segments at the ends of the line (crow's foot) when the points allow it
		if(dist >= stub0 + stub1)
			mid = std::clamp(mid, stub0, dist - stub1);

		mid = p0.x() + (dir0.x() * mid);
	}
	// The line leaves the tables at the same side: the middle segment goes around the points
	else
	{
		margin = std::max({ stub0, stub1, ConnLineLength * 0.5 });

		if(dir0.x() > 0)
			mid = std::max(p0.x(), p1.x()) + margin;
		else
			mid = std::min(p0.x(), p1.x()) - margin;

		mid += dir0.x() * mid_shift;
	}

	route = { p0, QPointF(mid, p0.y()), QPointF(mid, p1.y()), p1 };

	if(transposed)
	{
		for(auto &pnt : route)
			pnt = swap_xy(pnt);
	}

	return cleanRoute(route);
}

void RelationshipView::removeConnectorLines()
{
	trimGroupItems(this, fk_lines, 0);
	trimGroupItems(this, pk_lines, 0);
}

void RelationshipView::configureGraphicPoints(const std::vector<QPointF> &points)
{
	QGraphicsPolygonItem *pol = nullptr;
	QPolygonF pol_aux;
	unsigned count = points.size();

	pol_aux.append(QPointF(0,0)); pol_aux.append(QPointF(6,0));
	pol_aux.append(QPointF(6,6)); pol_aux.append(QPointF(0,6));

	for(unsigned i = 0; i < count; i++)
	{
		if(i >= graph_points.size())
		{
			pol = new QGraphicsPolygonItem;
			graph_points.push_back(pol);
			pol->setZValue(0);
			pol->setPolygon(pol_aux);
			pol->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));
			pol->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
			this->addToGroup(pol);
		}
		else
			pol = graph_points[i];

		pol->setPos(points[i]);
		pol->moveBy(-GraphicPointRadius/2, -GraphicPointRadius/2);
		pol->setVisible(this->isSelected());
	}

	//Destroy the graphical points not used
	trimGroupItems(this, graph_points, count);
}

QPen RelationshipView::getLinePen()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();
	QPen pen;

	pen.setCapStyle(Qt::RoundCap);

	//If the relationship is selected we do not change the lines colors
	if(this->isSelected() && !lines.empty())
		pen = lines[0]->pen();
	else
	{
		//Configuring the relationship line color
		if(base_rel->getCustomColor() != Qt::transparent)
			//Using custom color
			pen.setColor(base_rel->getCustomColor());
		else
			//Using the default color
			pen = BaseObjectView::getBorderStyle(Attributes::Relationship);
	}

	//For dependency/partition relationships the line is dashed
	if(rel_type == BaseRelationship::RelationshipDep ||
		 rel_type == BaseRelationship::RelationshipPart)
		pen.setStyle(Qt::DashLine);

	pen.setWidthF(getDefaultPenWidth());

	return pen;
}

void RelationshipView::configureConnectorLines(const LineGeometry &geom, const QPen &pen)
{
	if(line_conn_mode != ConnectFkToPk)
		return;

	QGraphicsLineItem *lin = nullptr;
	QPointF ref_points[2] = { geom.fk_pnt, geom.pk_pnt };
	const std::vector<QPointF> *ref_pnt_vects[2] = { &geom.fk_points, &geom.pk_points };
	std::vector<QGraphicsLineItem *> *ref_lines[2] = { &fk_lines, &pk_lines };
	const std::vector<QPointF> *ref_pnt = nullptr;
	std::vector<QGraphicsLineItem *> *ref_lin = nullptr;
	unsigned count = 0;

	for(unsigned vet_idx = 0; vet_idx < 2; vet_idx++)
	{
		ref_pnt = ref_pnt_vects[vet_idx];
		ref_lin = ref_lines[vet_idx];
		count = ref_pnt->size();

		for(unsigned i = 0; i < count; i++)
		{
			if(i >= ref_lin->size())
			{
				lin = new QGraphicsLineItem;
				lin->setZValue(-1);
				ref_lin->push_back(lin);
				this->addToGroup(lin);
			}
			else
				lin = ref_lin->at(i);

			lin->setLine(QLineF(ref_pnt->at(i), ref_points[vet_idx]));
			lin->setPen(pen);
		}

		//Destroy the unused pk or fk lines
		trimGroupItems(this, *ref_lin, count);
	}
}

void RelationshipView::configureLineSegments(const std::vector<QPointF> &points, const QPen &pen)
{
	QGraphicsLineItem *lin = nullptr;
	unsigned seg_cnt = points.size() - 1;

	//Create the relationship lines
	for(unsigned i = 0; i < seg_cnt; i++)
	{
		if(i >= lines.size())
		{
			lin = new QGraphicsLineItem;
			lin->setZValue(-1);
			lines.push_back(lin);
			this->addToGroup(lin);
		}
		else
			lin = lines[i];

		lin->setLine(QLineF(points[i], points[i+1]));
		lin->setPen(pen);
	}

	//Removing unused lines
	if(!this->getUnderlyingObject()->isSelfRelationship())
		trimGroupItems(this, lines, seg_cnt);
}

bool RelationshipView::isLineEndCirclesVisible()
{
	BaseRelationship *base_rel = this->getUnderlyingObject();
	BaseRelationship::RelType rel_type = base_rel->getRelationshipType();

	if(base_rel->isSelfRelationship())
		return false;

	if(line_conn_mode == ConnectCenterPoints && !use_crows_foot)
		return true;

	return line_conn_mode != ConnectTableEdges &&
				 (rel_type == BaseRelationship::RelationshipDep ||
					rel_type == BaseRelationship::RelationshipGen ||
					rel_type == BaseRelationship::RelationshipPart ||
					(rel_type == BaseRelationship::RelationshipNn && !use_crows_foot));
}

void RelationshipView::configureLineCircles(const QPointF *p_central, const QPen &pen)
{
	//Exposing the line ending circles
	if(isLineEndCirclesVisible())
	{
		for(unsigned i = 0; i < 2; i++)
		{
			line_circles[i]->setVisible(true);
			line_circles[i]->setPen(pen);
			line_circles[i]->setBrush(pen.color());
			line_circles[i]->setPos(p_central[i] - QPointF(GraphicPointRadius/2, GraphicPointRadius/2));
		}
	}
	else
	{
		line_circles[0]->setVisible(false);
		line_circles[1]->setVisible(false);
	}
}

void RelationshipView::configureCurves(const LineGeometry &geom)
{
	//Using bezier curves instead of straight lines to denote the relationship line
	if(line_style != StraightLines)
	{
		if(line_style == OrthogonalLines || this->getUnderlyingObject()->isSelfRelationship())
			configurePolylineCurve(geom.points, geom.end_lens[0], geom.end_lens[1]);
		else
			configureCurvedLines(geom);
	}
	else if(!lines.empty() && !curves.empty())
		removeCurves();
}

void RelationshipView::configureCurvedLines(const LineGeometry &geom)
{
	route_points.clear();
	bumps.clear();

	BezierCurveItem *curve = nullptr;
	bool invert_cpoints = false, simple_curve = false;
	unsigned i = 0;

	for(auto &line : lines)
	{
		if(i >= curves.size())
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
		simple_curve = (geom.conn_same_sides && lines.size() == 1) ||
									 (geom.conn_horiz_sides[0] != geom.conn_horiz_sides[1] && lines.size() == 3);

		if(!simple_curve)
			invert_cpoints = (geom.conn_horiz_sides[0] && geom.conn_horiz_sides[1] && lines.size() == 3);
		else
			invert_cpoints = (!geom.conn_horiz_sides[0] && geom.conn_horiz_sides[1] &&
												geom.conn_vert_sides[0] && !geom.conn_vert_sides[1]);

		curve->setLine(line->line(), simple_curve, invert_cpoints);
		curve->setPen(line->pen());
		line->setVisible(false);
	}

	//Removing unused curves
	trimGroupItems(this, curves, lines.size());
}

/* Keeps the straight horizontal/vertical segments of the line (self-relationship loops and
 * orthogonal lines) but rounds off the corners where they meet instead of drawing sharp right
 * angles: a single curve item traces the whole line, with a quadratic curve (control point at
 * the corner itself) taking over from the straight segments near each turn. */
void RelationshipView::configurePolylineCurve(const std::vector<QPointF> &points, double start_len, double end_len)
{
	BezierCurveItem *curve = nullptr;

	if(curves.empty())
	{
		curve = new BezierCurveItem;
		this->addToGroup(curve);
		curves.push_back(curve);
	}
	else
		curve = curves[0];

	/* The route of the orthogonal lines is stored so the bumps (hops over the lines crossing
	 * this one) can be applied by the scene later. They're discarded if the route changes */
	if(line_style == OrthogonalLines)
	{
		if(route_points != points)
			bumps.clear();

		route_points = points;
	}
	else
	{
		route_points.clear();
		bumps.clear();
	}

	curve_end_lens[0] = start_len;
	curve_end_lens[1] = end_len;

	curve->setZValue(lines[0]->zValue());
	curve->setPen(lines[0]->pen());
	curve->setPoints(points, RoundedCornerRadius * BaseObjectView::getScreenDpiFactor(),
									 bumps, BumpRadius * BaseObjectView::getScreenDpiFactor(),
									 curve_end_lens[0], curve_end_lens[1]);

	for(auto &line : lines)
		line->setVisible(false);

	//Removing any extra unused curves
	trimGroupItems(this, curves, 1);

	if(line_style == OrthogonalLines)
		notifyRouteChanged();
}

void RelationshipView::removeCurves()
{
	route_points.clear();
	bumps.clear();

	for(auto &line : lines)
		line->setVisible(true);

	trimGroupItems(this, curves, 0);
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

	if(line_style == StraightLines)
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
