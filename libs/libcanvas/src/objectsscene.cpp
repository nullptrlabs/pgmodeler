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

#include "objectsscene.h"
#include "relationshipview.h"
#include "styledtextboxview.h"
#include "graphicalview.h"
#include "tableview.h"
#include "schemaview.h"
#include "databasemodel.h"
#include <QScrollBar>

const QColor ObjectsScene::DefaultGridColor { "#e1e1e1"};
const QColor ObjectsScene::DefaultCanvasColor {"#fff"};
const QColor ObjectsScene::DefaultDelimitersColor {"#4b73c3"};

ObjectsScene::GridPattern ObjectsScene::grid_pattern { ObjectsScene::SquarePattern };

QColor ObjectsScene::grid_color { DefaultGridColor };
QColor ObjectsScene::canvas_color { DefaultCanvasColor };
QColor ObjectsScene::delimiters_color { DefaultDelimitersColor };

bool ObjectsScene::align_objs_grid {false};
bool ObjectsScene::show_grid {true};
bool ObjectsScene::show_page_delim {true};
bool ObjectsScene::corner_move {true};
bool ObjectsScene::invert_rangesel_trigger {false};
bool ObjectsScene::lock_delim_scale {false};

unsigned ObjectsScene::grid_size {20};
unsigned ObjectsScene::expansion_factor {2};

QPageLayout ObjectsScene::page_layout {
	QPageSize(QPageSize::A4),
	QPageLayout::Landscape,
	QMarginsF(10,10,10,10)
};

double ObjectsScene::delimiter_scale {1};
double ObjectsScene::min_scene_width { page_layout.paintRect().width() };
double ObjectsScene::min_scene_height { page_layout.paintRect().height() };

ObjectsScene::ObjectsScene()
{		
	is_layer_rects_visible=is_layer_names_visible=false;
	moving_objs=move_scene=false;
	show_scene_limits=enable_range_sel=true;

	sel_ini_pnt.setX(DNaN);
	sel_ini_pnt.setY(DNaN);

	selection_rect=new QGraphicsPolygonItem;
	selection_rect->setVisible(false);
	selection_rect->setZValue(100);

	rel_line=new QGraphicsLineItem;
	rel_line->setVisible(false);
	rel_line->setEnabled(false);
	rel_line->setZValue(-1);

	this->addItem(selection_rect);
	this->addItem(rel_line);

	scene_move_dx=scene_move_dy=0;

	connect(&scene_move_timer, &QTimer::timeout, this, &ObjectsScene::moveObjectScene);

	connect(&corner_hover_timer, &QTimer::timeout, this, [this](){
		enableSceneMove();
	});

	connect(&object_move_timer, &QTimer::timeout, this, [this](){
		//If the timer reaches its timeout we execute the procedures to finish the objects movement
		finishObjectsMove(itemsBoundingRect(true, true).center());
		object_move_timer.stop();
	});

	scene_move_timer.setInterval(SceneMoveTimeout);
	corner_hover_timer.setInterval(SceneMoveTimeout * 10);
	object_move_timer.setInterval(SceneMoveTimeout * 10);

	setSceneRect(QRectF(0, 0, min_scene_width, min_scene_height));
}

ObjectsScene::~ObjectsScene()
{
	QGraphicsItemGroup *item=nullptr;
	QList<QGraphicsItem *> items;
	std::vector<ObjectType> obj_types={ ObjectType::Relationship, ObjectType::Textbox, ObjectType::View,
																 ObjectType::Table, ObjectType::ForeignTable, ObjectType::Schema };

	this->removeItem(selection_rect);
	this->removeItem(rel_line);

	delete selection_rect;
	delete rel_line;

	while(!layers_paths.isEmpty())
	{
		removeItem(layers_paths.front());
		delete layers_paths.front();
		layers_paths.pop_front();
	}

	//Destroy the objects in the order defined on obj_types vector
	for(auto &type : obj_types)
	{
		items=this->items();

		while(!items.isEmpty())
		{
			/* Try to convert the item to QGraphicsItemGroup because all the objects
			used to represent database object are derived from this class */
			item=dynamic_cast<QGraphicsItemGroup *>(items.front());

			/* Case the object is converted to a item group and can be converted to database
			objects, indicates that the object can be removed from the scene */
			if(item && !item->parentItem() &&
					((dynamic_cast<RelationshipView *>(item) && type==ObjectType::Relationship) ||
					 (dynamic_cast<TextboxView *>(item) && type==ObjectType::Textbox) ||
					 (dynamic_cast<StyledTextboxView *>(item) && type==ObjectType::Textbox) ||
					 (dynamic_cast<GraphicalView *>(item) && type==ObjectType::View) ||
					 (dynamic_cast<TableView *>(item) && (type==ObjectType::Table || type==ObjectType::ForeignTable)) ||
					 (dynamic_cast<SchemaView *>(item) && type==ObjectType::Schema)))

			{
				this->removeItem(item);
			}

			items.pop_front();
		}
	}

	//The graphical representation of db objects must be destroyed in a sorted way
	std::sort(removed_objs.begin(), removed_objs.end());
	while(!removed_objs.empty())
	{
		delete removed_objs.back();
		removed_objs.pop_back();
	}
}

QString ObjectsScene::formatLayerName(const QString &name)
{
	QString fmt_name;
	unsigned idx = 1;

	//Removing invalid chars
	for(auto &chr : name)
	{
		if(chr.isLetterOrNumber() || chr == ' ' || chr == '_')
			fmt_name.append(chr);
		else
			fmt_name.append('_');
	}

	//Doing the desambiguation (if needed)
	while(layers.contains(fmt_name))
		fmt_name = QString("%1 %2").arg(name).arg(QString::number(idx++));

	return fmt_name;
}

QString ObjectsScene::addLayer(const QString &name)
{
	if(name.isEmpty())
		return "";

	LayerItem *layer_item = new LayerItem;
	QString fmt_name = formatLayerName(name);

	layers.push_back(fmt_name);
	layers_paths.append(layer_item);

	layer_item->setZValue(-100 - layers.size());
	layer_item->setEnabled(false);
	layer_item->setVisible(false);
	addItem(layer_item);

	emit s_layersChanged();
	return fmt_name;
}

void ObjectsScene::addLayers(QStringList names, bool reset_objs_layers)
{
	if(names.isEmpty())
		return;

	removeLayers(reset_objs_layers);

	if(!layers.isEmpty())
	{
		/* Since we can't remove the default layer we set its name
		 * as the first item of layer names list */
		if(layers.at(DefaultLayer) != names.at(DefaultLayer))
			renameLayer(DefaultLayer, names.at(0));

		// Discarding the first element of the name lists in order to avoid duplication names
		names.removeFirst();
	}

	for(auto &name : names)
		addLayer(name);
}

QString ObjectsScene::renameLayer(unsigned idx, const QString &name)
{
	if(name.isEmpty() || idx >= static_cast<unsigned>(layers.size()))
		return "";

	if(name != layers[idx])
	{
		QString old_name = layers[idx],
				new_name = formatLayerName(name);

		layers[idx] = new_name;
		active_layers.replaceInStrings(QRegularExpression(QString("^(%1)$").arg(old_name)), new_name);

		updateLayerRects();
		emit s_layersChanged();
	}

	return layers[idx];
}

void ObjectsScene::removeLayer(const QString &name)
{
	int idx = layers.indexOf(name);

	if(idx > 0)
	{
		LayerItem *path_item = layers_paths.at(idx);

		validateLayerRemoval(idx);

		layers.removeAll(name);
		active_layers.removeAll(name);
		layers_paths.removeAt(idx);

		removeItem(path_item);
		delete path_item;

		updateLayerRects();
		emit s_layersChanged();
	}
}

void ObjectsScene::removeLayers()
{
	removeLayers(true);
}

void ObjectsScene::removeLayers(bool reset_obj_layers)
{
	if(layers.isEmpty())
		return;

	LayerItem *layer_path = nullptr;
	BaseObjectView *obj_view = nullptr;
	QString def_layer = layers[DefaultLayer];
	bool is_active = active_layers.contains(def_layer);

	layers.clear();
	active_layers.clear();

	// Deleting all layer paths except the one related to the default layer
	while(layers_paths.size() > 1)
	{
		layer_path = layers_paths.back();
		removeItem(layer_path);
		delete(layer_path);
		layers_paths.pop_back();
	}

	layers.append(def_layer);

	if(is_active)
		active_layers.push_back(def_layer);

	if(reset_obj_layers)
	{
		for(auto &item : this->items())
		{
			obj_view = dynamic_cast<BaseObjectView *>(item);

			if(obj_view && !obj_view->parentItem())
			{
				obj_view->resetLayers();
				obj_view->setVisible(is_active);
			}
		}
	}

	emit s_layersChanged();
	updateActiveLayers();
}

void ObjectsScene::setActiveLayers(QStringList act_layers)
{
	QList<unsigned> layers_idxs;
	int idx = -1;

	for(auto &layer : act_layers)
	{
		idx = layers.indexOf(layer);

		if(idx >= 0)
			layers_idxs.push_back(idx);
	}

	setActiveLayers(layers_idxs);
}

void ObjectsScene::setActiveLayers(QList<unsigned> layer_ids)
{
	BaseObjectView *obj_view = nullptr;
	active_layers.clear();

	if(!layer_ids.isEmpty())
	{
		bool is_in_layer = false;
		unsigned layer_cnt = static_cast<unsigned>(layers.size());
		SchemaView *sch_view = nullptr;

		for(auto &item : this->items())
		{
			obj_view = dynamic_cast<BaseObjectView *>(item);

			if(obj_view && !obj_view->parentItem())
			{
				sch_view = dynamic_cast<SchemaView *>(obj_view);
				is_in_layer = false;

				for(auto &idx : layer_ids)
				{
					if(obj_view->isInLayer(idx))
					{
						is_in_layer = true;
						break;
					}
				}

				if(!obj_view->isVisible() && is_in_layer)
				{
					if(!sch_view ||
						 (sch_view && dynamic_cast<Schema *>(sch_view->getUnderlyingObject())->isRectVisible()))
					 obj_view->setVisible(true);
				}
				else if(obj_view->isVisible() && !is_in_layer)
					obj_view->setVisible(false);
			}
		}

		for(auto &idx : layer_ids)
		{
			if(idx < layer_cnt)
				active_layers.push_back(layers[idx]);
		}
	}
	else
	{
		for(auto &item : this->items())
		{
			obj_view = dynamic_cast<BaseObjectView *>(item);

			if(obj_view && !obj_view->parentItem())
				obj_view->setVisible(false);
		}
	}

	updateLayerRects();
	emit s_activeLayersChanged();
}

void ObjectsScene::updateLayerRects()
{
	if(layers_paths.isEmpty())
		return;

	for(auto &path : layers_paths)
		path->setVisible(false);

	if(!is_layer_rects_visible)
		return;

	int idx = 0, act_layer_idx = 0;
	BaseObjectView *obj_view = nullptr;
	ObjectType obj_type;
	QRectF brect;
	QMap<int, QList<QRectF>> rects;
	QFontMetricsF fm(LayerItem::getDefaultFont());

	for(auto &item : this->items())
	{
		obj_view = dynamic_cast<BaseObjectView *>(item);

		if(obj_view && !obj_view->parentItem())
		{
			obj_type = 	obj_view->getUnderlyingObject()->getObjectType();

			/* Schemas and relationship are ignored when determining the paths for the layers
			 * because since these objects can have big bounding rects it may polute. For now
			 * only table-like objects and textboxes can display layer boxes. */
			if(obj_type == ObjectType::Schema ||
				 obj_type ==ObjectType::BaseRelationship ||
				 obj_type ==ObjectType::Relationship)
				continue;

			brect = obj_view->boundingRect();
			brect.moveTo(obj_view->pos());

			for(auto &layer_id : obj_view->getLayers())
			{
				if(static_cast<int>(layer_id) >= layers.size() ||
					 !active_layers.contains(layers.at(layer_id)))
					continue;

				/* We need to adjust the bounding rect dimension in such a way
				 * to take into account the font height (if the layer names are visible)
				 * as well as a default padding so the rectangles doesn't have the same size
				 * of the object's bounding rect */
				brect.adjust(-LayerItem::LayerPadding,
										 (is_layer_names_visible ? -fm.height() : -LayerItem::LayerPadding),
										 LayerItem::LayerPadding,
										 LayerItem::LayerPadding);

				rects[layer_id].append(brect);
			}
		}
	}

	//Based the active layers we reconfigure the graphical items of each layer
	for(auto &layer_name : active_layers)
	{
		idx = layers.indexOf(layer_name);
		layers_paths[idx]->setTextAlignment(act_layer_idx % 2 == 0 ? Qt::AlignLeft : Qt::AlignRight);
		layers_paths[idx]->setText(is_layer_names_visible ? layer_name : "");
		layers_paths[idx]->setRects(rects[idx]);
		layers_paths[idx]->setVisible(true);
		act_layer_idx++;
	}
}

void ObjectsScene::setLayerRectsVisible(bool value)
{
	is_layer_rects_visible = value;
	updateLayerRects();
}

void ObjectsScene::setLayerNamesVisible(bool value)
{
	is_layer_names_visible = value;
	updateLayerRects();
}

bool ObjectsScene::isLayerRectsVisible()
{
	return is_layer_rects_visible;
}

bool ObjectsScene::isLayerNamesVisible()
{
	return is_layer_names_visible;
}

void ObjectsScene::validateLayerRemoval(unsigned old_layer)
{
	BaseObjectView *obj_view = nullptr;
	unsigned total_layers = layers.size();
	QList<unsigned> obj_layers;

	if(old_layer == DefaultLayer || old_layer >= total_layers)
		return;

	for(auto &item : this->items())
	{
		obj_view = dynamic_cast<BaseObjectView *>(item);

		if(obj_view && !obj_view->parentItem())
		{
			// Remove the object from the layer to be deleted and add it to the default one
			if(obj_view->isInLayer(old_layer))
			{
				obj_view->removeFromLayer(old_layer);
				obj_view->addToLayer(DefaultLayer);
				obj_view->setVisible(isLayerActive(layers[DefaultLayer]));
			}

			/* Shifting the remainging layers ids if the layer to be removed is
			 * >= 1 or < layers.size(). For example, if we have the following layers:
			 * (0, 1, 2, 3). If the layer 1 is to be deleted, then there's the need to
			 * shift the ids (2, 3) to (1, 2) since after the delition of the layer 1
			 * the id 3 is invalid */
			obj_layers = obj_view->getLayers();

			for(auto &layer_id : obj_layers)
			{
				if(layer_id > old_layer)
					layer_id--;
			}

			obj_view->setLayers(obj_layers);
		}
	}

	emit s_objectsMovedLayer();
}

bool ObjectsScene::isLayerActive(const QString &name)
{
	return active_layers.contains(name);
}

bool ObjectsScene::isLayerActive(unsigned layer_id)
{
	if(layer_id >= static_cast<unsigned>(layers.size()))
		return false;

	return active_layers.contains(layers[layer_id]);
}

bool ObjectsScene::isLayersActive(const QList<unsigned> &list)
{
	for(auto &id : list)
	{
		if(id < static_cast<unsigned>(layers.size()) && active_layers.contains(layers[id]))
			return true;
	}

	return false;
}

QStringList ObjectsScene::getActiveLayers()
{
	return active_layers;
}

QList<unsigned> ObjectsScene::getActiveLayersIds()
{
	QList<unsigned> list;

	for(auto &layer : active_layers)
		list.push_back(layers.indexOf(layer));

	return list;
}

QStringList ObjectsScene::getLayers()
{
	return layers;
}

void ObjectsScene::updateActiveLayers()
{
	setActiveLayers(active_layers);
}

QStringList ObjectsScene::getLayerColorNames(LayerAttrColor color_id)
{
	QStringList colors;

	for(auto &path : layers_paths)
		colors.append(color_id == LayerNameColor ? path->getTextColor().name() : path->brush().color().name());

	return colors;
}

void ObjectsScene::setLayerColors(int layer_id, QColor txt_color, QColor bg_color)
{
	if(layer_id >= layers_paths.size())
		return;

	layers_paths[layer_id]->setTextColor(txt_color);
	layers_paths[layer_id]->setPen(QPen(bg_color, BaseObjectView::ObjectBorderWidth * BaseObjectView::getScreenDpiFactor()));

	bg_color.setAlpha(BaseObjectView::ObjectAlphaChannel * 0.80);
	layers_paths[layer_id]->setBrush(bg_color);

	layers_paths[layer_id]->update();
}

void ObjectsScene::setLayerColors(LayerAttrColor layer_attr_id, const QStringList &colors)
{
	int idx = 0;
	QColor color;

	for(auto &cl_name : colors)
	{
		if(idx >= layers_paths.size())
			break;

		color = QColor(cl_name);

		if(layer_attr_id == LayerNameColor)
			layers_paths[idx]->setTextColor(color);
		else
		{
			layers_paths[idx]->setPen(QPen(color, BaseObjectView::ObjectBorderWidth * BaseObjectView::getScreenDpiFactor()));

			color.setAlpha(BaseObjectView::ObjectAlphaChannel * 0.80);
			layers_paths[idx]->setBrush(color);
		}

		idx++;
	}
}

void ObjectsScene::setShowSceneLimits(bool show)
{
	show_scene_limits = show;
}

void ObjectsScene::setLockDelimiterScale(bool lock, double curr_scale)
{
	if(lock && curr_scale > 0 && curr_scale < 1)
		delimiter_scale = curr_scale;
	else
		delimiter_scale = 1;

	lock_delim_scale = lock;
}

bool ObjectsScene::isDelimiterScaleLocked()
{
	return lock_delim_scale;
}

void ObjectsScene::setEnableCornerMove(bool enable)
{
	ObjectsScene::corner_move=enable;
}

void ObjectsScene::setInvertRangeSelectionTrigger(bool invert)
{
	ObjectsScene::invert_rangesel_trigger=invert;
}

bool ObjectsScene::isCornerMoveEnabled()
{
	return ObjectsScene::corner_move;
}

void ObjectsScene::setGridPattern(GridPattern pattern)
{
	grid_pattern = pattern;
}

QPointF ObjectsScene::alignPointToGrid(const QPointF &pnt)
{
	double px = round(pnt.x() / static_cast<double>(grid_size)) * grid_size,
			   py = round(pnt.y() / static_cast<double>(grid_size)) * grid_size;

	return { QPointF(px, py).toPoint() };
}

void ObjectsScene::setSceneRect(const QRectF &rect)
{
	QSizeF sz = rect.size();

	if(sz.width() < min_scene_width)
		sz.setWidth(min_scene_width);

	if(sz.height() < min_scene_height)
		sz.setHeight(min_scene_height);

	QRectF old_rect = sceneRect(),
			new_rect = QRectF(rect.topLeft(), sz);

	QGraphicsScene::setSceneRect(new_rect);

	if(old_rect != new_rect)
		emit s_sceneRectChanged(new_rect);
}

QRectF ObjectsScene::itemsBoundingRect(bool seek_only_db_objs, bool selected_only, bool incl_layer_rects)
{
	if(!seek_only_db_objs)
		return QGraphicsScene::itemsBoundingRect();

	QRectF items_rect, rect;
	QList<QGraphicsItem *> items= (selected_only ? this->selectedItems() : this->items());
	BaseObjectView *obj_view = nullptr;
	QPointF pnt;
	QFontMetricsF fm(LayerItem::getDefaultFont());
	ObjectType obj_type;
	BaseGraphicObject *graph_obj = nullptr;

	for(auto &item : items)
	{
		obj_view = dynamic_cast<BaseObjectView *>(item);
		graph_obj = !obj_view ? nullptr : dynamic_cast<BaseGraphicObject *>(obj_view->getUnderlyingObject());

		if(!obj_view || !obj_view->isVisible() || !graph_obj)
			continue;

		obj_type = graph_obj->getObjectType();
		rect = obj_view->boundingRect();
		rect = obj_view->mapRectToScene(obj_view->boundingRect());

		if(graph_obj && incl_layer_rects && is_layer_rects_visible &&
			 obj_type != ObjectType::Schema &&
			 obj_type != ObjectType::BaseRelationship &&
			 obj_type != ObjectType::Relationship)
		{
			pnt = QPointF(LayerItem::LayerPadding * graph_obj->getLayersCount(),
										(is_layer_names_visible ? fm.height() : LayerItem::LayerPadding) * graph_obj->getLayersCount());

			rect.setTopLeft(rect.topLeft() - pnt);
			rect.setBottomRight(rect.bottomRight() + pnt);
		}

		items_rect = items_rect.united(rect);
	}

	return items_rect;
}

void ObjectsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
	double page_w = 0, page_h = 0,
			delim_factor = 1/delimiter_scale,
			pen_width = BaseObjectView::ObjectBorderWidth *
									BaseObjectView::getScreenDpiFactor();
	QSizeF aux_size;
	QRectF scn_rect = sceneRect();
	QPen pen = QPen(QColor(), pen_width);
	int start_x = 0, start_y = 0,
			end_x = 0, end_y = 0;

	// Retrieve the page rect considering the orientation, margin and page size
	aux_size = page_layout.paintRect(QPageLayout::Point).size() * delim_factor;

	//Calculates where the extreme width and height where delimiter lines will be drawn
	page_w = aux_size.width() / static_cast<double>(grid_size) * grid_size;
	page_h = aux_size.height() / static_cast<double>(grid_size) * grid_size;

	painter->save();
	painter->setClipping(true);
	painter->setClipRect(rect);
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setRenderHint(QPainter::TextAntialiasing, false);

	painter->fillRect(rect, canvas_color);

	start_x = round(scn_rect.left()/grid_size) * grid_size;
	start_y = round(scn_rect.top()/grid_size) * grid_size;
	end_x = scn_rect.right();
	end_y = scn_rect.bottom();

	if(show_grid && !move_scene)
	{
		int x1 = 0, y1 = 0,
				x2 = 0, y2 = 0;

		pen.setWidthF(pen_width *	(grid_pattern == GridPattern::DotPattern ? 1.60 : 1));
		pen.setColor(grid_color);
		painter->setPen(pen);

		//Draws the grid
		for(x1 = start_x; x1 < end_x; x1 += grid_size)
		{
			for(y1 = start_y; y1 < end_y; y1 += grid_size)
			{
				x2 = x1 + grid_size;
				y2 = y1 + grid_size;

				// Avoid drawing rectangle/dots beyond the scene limits
				if(y2 > end_y)
					y2 = y1;

				if(x2 > end_x)
					x2 = x1;

				if(grid_pattern == GridPattern::SquarePattern)
				{
					painter->drawLine(x1, y1, x2, y1);
					painter->drawLine(x2, y1, x2, y2);
					painter->drawLine(x1, y2, x2, y2);
					painter->drawLine(x1, y1, x1, y2);
				}
				else
				{
					painter->drawPoint(x1, y1);
					painter->drawPoint(x2, y1);
					painter->drawPoint(x2, y2);
					painter->drawPoint(x1, y2);
				}
			}
		}
	}

	//Creates the page delimiter lines
	if(show_page_delim && !move_scene)
	{
		pen.setWidthF(pen_width * 1.15);
		pen.setColor(delimiters_color);
		pen.setStyle(Qt::CustomDashLine);
		pen.setDashPattern({3, 5});
		painter->setPen(pen);

		double st_x = start_x, st_y = start_y;

		/* If the current origin point of the canvas is not at the positive origin (0,0)
		 * we need to calculate the number of page delimiter to be drawn before the (0,0).
		 * This avoid the delimiters to be shifted every time the canvas origin moves
		 * far to a negative point. */
		if(start_x < 0 || start_y < 0)
		{
			/* Due to the infinite canvas feature, we need to calculate
			 * the number of pages before origin (0,0) so the iteration
			 * can start in the right position of rendering */
			st_x = round(start_x/page_w) * page_w,
			st_y = round(start_y/page_h) * page_h;

			/* We decremente in one page the start rendering position
			 * so the round operation above doesn't cut any start page */
			st_x -= page_w;
			st_y -= page_h;
		}

		for(int px = st_x; px < end_x; px += page_w)
		{
			for(int py = st_y; py < end_y; py += page_h)
			{
				painter->drawLine(px, py, px + page_w, py);
				painter->drawLine(px, py, px, py + page_h);

				if(px + page_w >= end_x)
					painter->drawLine(px + page_w, py, px + page_w, py + page_h);

				if(py + page_h >= end_y)
					painter->drawLine(px, py + page_h, px + page_w, py + page_h);
			}
		}
	}

	// Drawing the scene boundaries
	if(show_scene_limits && !move_scene)
	{
		pen.setWidthF(pen_width * 1.30);
		pen.setColor(QColor(255, 0, 0));
		pen.setStyle(Qt::SolidLine);
		painter->setPen(pen);

		painter->drawLine(start_x, end_y, end_x, end_y);
		painter->drawLine(end_x, start_y, end_x, end_y);
	}

	painter->restore();
}

void ObjectsScene::setGridSize(unsigned size)
{
	if(size < 20)	size = 20;
	grid_size = size;
}

unsigned int ObjectsScene::getGridSize()
{
	return grid_size;
}

void ObjectsScene::showRelationshipLine(bool value, const QPointF &p_start)
{
	QList<QGraphicsItem *> items=this->items();
	QGraphicsItem::GraphicsItemFlags flags;
	BaseObjectView *object=nullptr;
	TableObjectView *tab_obj_view=nullptr;
	BaseGraphicObject *base_obj=nullptr;

	if(!std::isnan(p_start.x()) && !std::isnan(p_start.y()))
		rel_line->setLine(QLineF(p_start,p_start));

	rel_line->setPen(QPen(delimiters_color,
												BaseObjectView::ObjectBorderWidth * BaseObjectView::getScreenDpiFactor()));
	rel_line->setVisible(value);

	while(!items.isEmpty())
	{
		//When showing the relationship line all the objects cannot be moved
		flags=QGraphicsItem::ItemIsSelectable |
					QGraphicsItem::ItemSendsGeometryChanges;

		object = dynamic_cast<BaseObjectView *>(items.front());
		tab_obj_view = dynamic_cast<TableObjectView *>(object);

		// Discarding table objects views from checking since they can't be normally be selected by a single click
		if(object && !tab_obj_view && object->getUnderlyingObject())
		{
			BaseObject *ul_object = object->getUnderlyingObject();
			base_obj=dynamic_cast<BaseGraphicObject *>(ul_object);

			if(!value && base_obj &&
					base_obj->getObjectType()!=ObjectType::Relationship &&
					base_obj->getObjectType()!=ObjectType::BaseRelationship &&
					!base_obj->isProtected())
				flags=QGraphicsItem::ItemIsMovable |
					  QGraphicsItem::ItemIsSelectable |
					  QGraphicsItem::ItemSendsGeometryChanges;
			else
				flags=QGraphicsItem::ItemIsSelectable |
					  QGraphicsItem::ItemSendsGeometryChanges;
		}

		items.front()->setFlags(flags);
		items.pop_front();
	}
}

void ObjectsScene::setAlignObjectsToGrid(bool value)
{
	align_objs_grid = value;
}

bool ObjectsScene::isAlignObjectsToGrid()
{
	return align_objs_grid;
}

void ObjectsScene::setShowGrid(bool value)
{
	show_grid = value;
}

bool ObjectsScene::isShowGrid()
{
	return show_grid;
}

void ObjectsScene::setShowPageDelimiters(bool value)
{
	show_page_delim = value;
}

bool ObjectsScene::isShowPageDelimiters()
{
	return show_page_delim;
}

void ObjectsScene::setPageLayout(const QPageLayout &page_lt)
{
	page_layout = page_lt;
	min_scene_width = page_layout.paintRect().width() * 2;
	min_scene_height = page_layout.paintRect().height() * 2;
}

QPageLayout ObjectsScene::getPageLayout()
{
	return page_layout;
}

void ObjectsScene::handlePopupMenuRequested(TableObject *child_obj)
{
	emit s_popupMenuRequested(child_obj);
}

void ObjectsScene::handleObjectSelection(BaseGraphicObject *object, bool selected)
{
	if(object)
		emit s_objectSelected(object, selected);
}

void ObjectsScene::handleChildrenSelectionChanged()
{
	BaseTableView *tab_view = dynamic_cast<BaseTableView *>(sender());

	if(!tab_view)
		return;

	if(tab_view->getSelectedChidren().empty())		
		tabs_sel_children.removeAll(tab_view);
	else if(!tabs_sel_children.contains(tab_view))
		tabs_sel_children.append(tab_view);

	emit s_childrenSelectionChanged();
}

void ObjectsScene::addItem(QGraphicsItem *item, bool blink_new)
{
	if(!item)	return;

	RelationshipView *rel=dynamic_cast<RelationshipView *>(item);
	BaseTableView *tab=dynamic_cast<BaseTableView *>(item);
	BaseObjectView *obj=dynamic_cast<BaseObjectView *>(item);
	TextboxView *txtbox=dynamic_cast<TextboxView *>(item);

	if(rel)
		connect(rel, &RelationshipView::s_relationshipModified, this, &ObjectsScene::s_objectModified);
	else if(tab)
	{
		connect(tab, &BaseTableView::s_popupMenuRequested, this, &ObjectsScene::handlePopupMenuRequested);
		connect(tab, &BaseTableView::s_childrenSelectionChanged, this, &ObjectsScene::handleChildrenSelectionChanged);
		connect(tab, &BaseTableView::s_collapseModeChanged, this, &ObjectsScene::s_collapseModeChanged);
		connect(tab, &BaseTableView::s_paginationToggled, this, &ObjectsScene::s_paginationToggled);
		connect(tab, &BaseTableView::s_currentPageChanged, this, &ObjectsScene::s_currentPageChanged);
		connect(tab, &BaseTableView::s_sceneClearRequested, this, &ObjectsScene::clearSelection);
	}

	if(obj)
	{
		obj->setVisible(isLayersActive(obj->getLayers()));

		// Relationships and schemas don't have their z value changed
		if(!rel && !dynamic_cast<SchemaView *>(item))
			obj->setZValue(dynamic_cast<BaseGraphicObject *>(obj->getUnderlyingObject())->getZValue());

		connect(obj, &BaseObjectView::s_objectSelected, this, &ObjectsScene::handleObjectSelection);

		// Tables and textboxes are observed for dimension changes so the layers they are in are correctly updated
		if(tab || txtbox)
			connect(obj, &BaseObjectView::s_objectDimensionChanged, this, &ObjectsScene::updateLayerRects);
	}

	if(blink_new)
		item->setOpacity(0);

	QGraphicsScene::addItem(item);

	if(blink_new && obj)
		obj->blink();

	if(tab || txtbox)
		updateLayerRects();
}

void ObjectsScene::removeItem(QGraphicsItem *item)
{
	if(item)
	{
		BaseObjectView *obj_view=dynamic_cast<BaseObjectView *>(item);
		RelationshipView *rel=dynamic_cast<RelationshipView *>(item);

		if(rel)
			rel->disconnectTables();

		item->setVisible(false);
		item->setActive(false);
		QGraphicsScene::removeItem(item);

		if(obj_view)
		{
			BaseObject *obj = obj_view->getUnderlyingObject();

			if(BaseTable::isBaseTable(obj->getObjectType()) ||
				 obj->getObjectType() == ObjectType::Textbox)
				updateLayerRects();

			disconnect(obj_view, nullptr, this, nullptr);
			disconnect(obj_view, nullptr, dynamic_cast<BaseGraphicObject*>(obj_view->getUnderlyingObject()), nullptr);
			disconnect(dynamic_cast<BaseGraphicObject*>(obj_view->getUnderlyingObject()), nullptr, obj_view, nullptr);
			removed_objs.push_back(obj_view);
		}
	}
}

void ObjectsScene::blockItemsSignals(bool block)
{
	BaseObjectView *obj_view = nullptr;

	for(auto &item : this->items())
	{
		obj_view = dynamic_cast<BaseObjectView *>(item);
		if(obj_view)
			obj_view->blockSignals(block);
	}
}

void ObjectsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseDoubleClickEvent(event);

	if(this->selectedItems().size()==1 && event->buttons()==Qt::LeftButton && !rel_line->isVisible())
	{
		//Gets the selected graphical object
		BaseObjectView *obj=dynamic_cast<BaseObjectView *>(this->selectedItems().at(0));

		if(obj)
			emit s_objectDoubleClicked(dynamic_cast<BaseGraphicObject *>(obj->getUnderlyingObject()));
	}
	else
		//Emit a signal indicating that no object was selected
		emit s_objectDoubleClicked(nullptr);
}

void ObjectsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	//Gets the item at mouse position
	QGraphicsItem* item=this->itemAt(event->scenePos(), QTransform());
	bool is_deselection = !this->selectedItems().isEmpty() && !item;

	if(selectedItems().empty())
		emit s_scenePressed(event->buttons());

	/* If the relationship line is visible, indicates that the user is in the middle of
	 a relationship creation, thus is needed to inform to the scene to activate the
	 the multiselection to be able to select two tables and link them. By default,
	 the multiselection modifier is the Control key */
	if(rel_line->isVisible())
		event->setModifiers(Qt::ControlModifier);

	if(is_deselection)
		this->blockItemsSignals(true);

	/* If we're handling a deselection of the user selected another object whitout being holding Control
	 * we need to deselect the tables' children objects too */
	if(is_deselection || (event->buttons()==Qt::LeftButton && (event->modifiers() & Qt::ControlModifier) != Qt::ControlModifier))
		//Forcing the clear on all selected table children object
		clearTablesChildrenSelection();

	QGraphicsScene::mousePressEvent(event);

	if(is_deselection)
	{
		this->blockItemsSignals(false);
		emit s_objectSelected(nullptr, false);
	}

	if(event->buttons()==Qt::LeftButton)
	{
		sel_ini_pnt=event->scenePos();

		if((!invert_rangesel_trigger && event->modifiers()==Qt::ShiftModifier) ||
				(invert_rangesel_trigger && event->modifiers()==Qt::NoModifier))
		{
			if(enable_range_sel && this->selectedItems().isEmpty())
			{
				selection_rect->setVisible(true);
				emit s_objectSelected(nullptr,false);
			}
		}
		else
		{
			//Selects the object (without press control) if the user is creating a relationship
			if(item && item->isEnabled() && !item->isSelected() &&  rel_line->isVisible())
				item->setSelected(true);
		}
	}
	else if(event->buttons()==Qt::RightButton)
	{
		//Case there is no item at the mouse position clears the selection on the scene
		if(!item)
		{
			this->clearSelection();
			emit s_objectSelected(nullptr,false);
		}

		emit s_popupMenuRequested();
	}
}

void ObjectsScene::setGridColor(const QColor &value)
{
	grid_color = value;
}

QColor ObjectsScene::getGridColor()
{
	return grid_color;
}

void ObjectsScene::setCanvasColor(const QColor &value)
{
	canvas_color = value;
}

QColor ObjectsScene::getCanvasColor()
{
	return canvas_color;
}

void ObjectsScene::setPageDelimitersColor(const QColor &value)
{
	delimiters_color = value;
}

QColor ObjectsScene::getPageDelimitersColor()
{
	return delimiters_color;
}

void ObjectsScene::setExpansionFactor(unsigned factor)
{
	expansion_factor = factor > 10 ? 10 : factor;
}

unsigned int ObjectsScene::getExpansionFactor()
{
	return expansion_factor;
}

bool ObjectsScene::mouseIsAtCorner()
{
	QGraphicsView *view =getActiveViewport();

	if(view)
	{
		QPoint pos = view->mapFromGlobal(QCursor::pos());
		QRect rect = view->rect();

		if(rect.contains(pos))
		{
			if(pos.x() <= SceneMoveThreshold)
				scene_move_dx = -SceneMoveStep;
			else if(pos.x() >= (view->width() - view->verticalScrollBar()->width() - SceneMoveThreshold))
				scene_move_dx = SceneMoveStep;
			else
				scene_move_dx = 0;

			if(pos.y() <= SceneMoveThreshold)
				scene_move_dy = -SceneMoveStep;
			else if(pos.y() >= (view->height() - view->horizontalScrollBar()->height() - SceneMoveThreshold))
				scene_move_dy = SceneMoveStep;
			else
				scene_move_dy = 0;

			return scene_move_dx != 0 || scene_move_dy != 0;
		}
		
		return false;
	}
	
	return false;
}

QGraphicsView *ObjectsScene::getActiveViewport()
{
	QGraphicsView *view_p=nullptr;

	for(auto &view : this->views())
	{
		if(view && view->isActiveWindow())
		{
			view_p=view;
			break;
		}
	}

	return view_p;
}

void ObjectsScene::moveObjectScene()
{
	if(scene_move_dx!=0 || scene_move_dy!=0)
	{
		QGraphicsView *view=getActiveViewport();

		if(view && mouseIsAtCorner())
		{
			view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() + scene_move_dx);
			view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() + scene_move_dy);
			move_scene=true;
		}
		else
		{
			move_scene=false;
			scene_move_timer.stop();
		}
	}
}

void ObjectsScene::enableSceneMove(bool value)
{
	if(value)
	{
		scene_move_timer.start();
		corner_hover_timer.stop();
	}
	else
	{
		corner_hover_timer.stop();
		scene_move_timer.stop();
	}

	move_scene=value;
}

void ObjectsScene::enableRangeSelection(bool value)
{
	enable_range_sel=value;

	if(!value && selection_rect->isVisible())
		selection_rect->setVisible(value);
}

void ObjectsScene::adjustScenePositionOnKeyEvent(int key)
{
	QGraphicsView *view = getActiveViewport();

	if(view)
	{
		QRectF brect = itemsBoundingRect(true, true);
		QRectF view_rect = QRectF(view->mapToScene(view->rect().topLeft()),
															view->mapToScene(view->rect().bottomRight())),
				scene_rect = sceneRect();

		if(view_rect.right() < brect.right() && key == Qt::Key_Right)
		{
			/* If the objects are being moved right and the scene width is lesser than the items bounding rect's width
			we need to resize the scene prior the position adjustment */
			scene_rect.setRight(brect.right());
			setSceneRect(scene_rect);
			view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() + ((brect.right() - view_rect.right()) * 2));
		}
		else if(view_rect.left() > brect.left()  && key == Qt::Key_Left)
			view->horizontalScrollBar()->setValue(view->horizontalScrollBar()->value() - ((view_rect.left() - brect.left()) * 2));

		if(view_rect.bottom() < brect.bottom() && key == Qt::Key_Down)
		{
			/* If the objects are being moved down and the scene hight is lesser than the items bounding rect's height
			we need to resize the scene prior the position adjustment */
			scene_rect.setBottom(brect.bottom());
			setSceneRect(scene_rect);
			view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() + ((brect.bottom() - view_rect.bottom()) * 2));
		}
		else if(view_rect.top() > brect.top()  && key == Qt::Key_Up)
			view->verticalScrollBar()->setValue(view->verticalScrollBar()->value() - ((view_rect.top() - brect.top()) * 2));
	}
}

void ObjectsScene::keyPressEvent(QKeyEvent *event)
{
	if((event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
			event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) &&
		 !selectedItems().isEmpty())
	{
		double dx = 0, dy = 0;
		BaseObjectView *obj_view=nullptr;
		QRectF brect = itemsBoundingRect(true, true);

		if(!moving_objs)
		{
			sel_ini_pnt = brect.center();
			moving_objs = true;

			/* If the object move timer is not active we need to send the
			s_objectsMoved() signal in order to alert the classes like ModelWidget to
			save the current objects' position in the operation history */
			if(!object_move_timer.isActive())
				emit s_objectsMoved(false);

			for(auto item : selectedItems())
			{
				obj_view=dynamic_cast<BaseObjectView *>(item);

				if(obj_view && BaseObjectView::isPlaceholderEnabled())
					obj_view->togglePlaceholder(true);
			}
		}

		int move_step = static_cast<int>(grid_size);

		if(event->key() == Qt::Key_Up)
			dy = -move_step;
		else if(event->key() == Qt::Key_Down)
			dy = move_step;

		if(event->key() == Qt::Key_Left)
			dx = -move_step;
		else if(event->key() == Qt::Key_Right)
			dx = move_step;

		if(event->modifiers() == Qt::ControlModifier)
		{
			dx *= 10;
			dy *= 10;
		}

		for(auto item : selectedItems())
		{
			obj_view=dynamic_cast<BaseObjectView *>(item);

			if(obj_view && !dynamic_cast<RelationshipView *>(obj_view))
				obj_view->moveBy(dx, dy);
		}

		adjustScenePositionOnKeyEvent(event->key());
	}
	else
		QGraphicsScene::keyPressEvent(event);
}

void ObjectsScene::keyReleaseEvent(QKeyEvent *event)
{
	if((event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
			event->key() == Qt::Key_Left || event->key() == Qt::Key_Right) &&
		 !event->isAutoRepeat() && !selectedItems().isEmpty())
	{
		if(moving_objs)
		{
			object_move_timer.start();
			adjustScenePositionOnKeyEvent(event->key());
		}
	}
	else
		QGraphicsScene::keyReleaseEvent(event);
}

void ObjectsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->buttons()==Qt::LeftButton || rel_line->isVisible())
	{
		if(corner_move)
		{
			if(mouseIsAtCorner())
			{
				if(move_scene)
					scene_move_timer.start();
				else
					corner_hover_timer.start();
			}
			else
				enableSceneMove(false);
		}

		if(!rel_line->isVisible())
		{
			int sel_items_count = this->selectedItems().size();

			//Case the user starts a object moviment
			if(sel_items_count != 0 && !moving_objs)
			{
				if(BaseObjectView::isPlaceholderEnabled())
				{
					QList<QGraphicsItem *> items=this->selectedItems();
					BaseObjectView *obj_view=nullptr;

					for(QGraphicsItem *item : items)
					{
						obj_view=dynamic_cast<BaseObjectView *>(item);
						obj_view->togglePlaceholder(true);
					}
				}

				emit s_objectsMoved(false);
				moving_objs=true;
			}

			//If the alignment to grid is active, adjust the event scene position
			if(align_objs_grid && !selection_rect->isVisible() && sel_items_count <= 1)
				event->setScenePos(this->alignPointToGrid(event->scenePos()));
			else	if(selection_rect->isVisible())
			{
				QPolygonF pol;
				pol.append(sel_ini_pnt);
				pol.append(QPointF(event->scenePos().x(), sel_ini_pnt.y()));
				pol.append(QPointF(event->scenePos().x(), event->scenePos().y()));
				pol.append(QPointF(sel_ini_pnt.x(), event->scenePos().y()));
				selection_rect->setPolygon(pol);
				selection_rect->setBrush(BaseObjectView::getFillStyle(Attributes::ObjSelection));
				selection_rect->setPen(BaseObjectView::getBorderStyle(Attributes::ObjSelection));
			}
		}
	}

	if(rel_line->isVisible())
		rel_line->setLine(QLineF(rel_line->line().p1(), event->scenePos()));

	QGraphicsScene::mouseMoveEvent(event);
}

void ObjectsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
	QGraphicsScene::mouseReleaseEvent(event);

	if(event->button()==Qt::LeftButton && corner_move)
		enableSceneMove(false);

	//If there is selected object and the user ends the object moviment
	if(!this->selectedItems().isEmpty() && moving_objs && event->button()==Qt::LeftButton)
	{
		finishObjectsMove(event->scenePos());
	}
	else if(selection_rect->isVisible() && event->button()==Qt::LeftButton)
	{
		QPolygonF pol;
		QPainterPath sel_area;

		sel_area.addRect(selection_rect->polygon().boundingRect());

		this->blockItemsSignals(true);
		this->setSelectionArea(sel_area);
		this->blockItemsSignals(false);

		selection_rect->setVisible(false);
		selection_rect->setPolygon(pol);
		sel_ini_pnt.setX(DNaN);
		sel_ini_pnt.setY(DNaN);

		if(!this->selectedItems().isEmpty())
			emit s_objectsSelectedInRange();
	}
}

void ObjectsScene::finishObjectsMove(const QPointF &pnt_end)
{
	QList<QGraphicsItem *> items=this->selectedItems(), rel_list;
	double dx, dy;
	SchemaView *sch_view=nullptr;
	std::vector<QPointF> points;
	std::vector<QPointF>::iterator itr;
	std::vector<BaseObject *> rels, base_rels;
	QSet<Schema *> schemas;
	BaseRelationship *base_rel=nullptr;
	RelationshipView *rel=nullptr;
	BaseObjectView *obj_view=nullptr;
	BaseTableView *tab_view=nullptr;
	TableObjectView *tab_obj_view=nullptr;
	QSet<BaseObjectView *> tables;
	QRectF sel_rect;

	//Gathering the relationships inside the selected schemsa in order to move their points too
	for(auto &item : items)
	{
		obj_view=dynamic_cast<BaseObjectView *>(item);
		sch_view=dynamic_cast<SchemaView *>(item);
		tab_view=dynamic_cast<BaseTableView *>(item);
		tab_obj_view=dynamic_cast<TableObjectView *>(item);

		// Ignoring table objects items
		if(tab_obj_view)
			continue;

		if(obj_view)
			obj_view->togglePlaceholder(false);

		if(tab_view)
			tables.insert(tab_view);
		else if(sch_view)
		{
			//Get the schema object
			Schema *schema=dynamic_cast<Schema *>(sch_view->getUnderlyingObject());

			if(!schema->isProtected())
			{
				//Get the table-table and table-view relationships
				rels=dynamic_cast<DatabaseModel *>(schema->getDatabase())->getObjects(ObjectType::Relationship, schema);
				base_rels=dynamic_cast<DatabaseModel *>(schema->getDatabase())->getObjects(ObjectType::BaseRelationship, schema);
				rels.insert(rels.end(), base_rels.begin(), base_rels.end());

				for(auto &rel : rels)
				{
					base_rel=dynamic_cast<BaseRelationship *>(rel);

					/* If the relationship contains points and it is not selected then it will be included on the list
						 in order to move their custom line points */
					if(!dynamic_cast<RelationshipView *>(base_rel->getOverlyingObject())->isSelected() &&
							!base_rel->getPoints().empty())
						rel_list.push_back(dynamic_cast<QGraphicsItem *>(base_rel->getOverlyingObject()));
				}

				QList<BaseObjectView *> list = sch_view->getChildren();
				tables.unite(QSet<BaseObjectView *>(list.begin(), list.end()));
			}
		}
	}

	items.append(rel_list);
	dx=pnt_end.x() - sel_ini_pnt.x();
	dy=pnt_end.y() - sel_ini_pnt.y();

	for(auto &item : items)
	{
		// Ignoring table objects items
		if(dynamic_cast<TableObjectView *>(item))
			continue;

		rel=dynamic_cast<RelationshipView *>(item);

		if(!rel)
		{
			if(align_objs_grid)
				item->setPos(alignPointToGrid(item->pos()));

			/* Storing the object's bounding rect (with position) so we can
			 * request that the viewport adjust the current position to make
			 * the selection visible */
			sel_rect = sel_rect.united(QRectF(item->pos(), item->boundingRect().size()));
		}
		else
		{
			/* If the relationship has points added to the line is necessary to move the points
				 too. Since relationships cannot be moved naturally (by user) this will be done
				 by the scene. NOTE: this operation is done ONLY WHEN there is more than one object selected! */
			points = rel->getUnderlyingObject()->getPoints();
			if(items.size() > 1 && !points.empty())
			{
				itr=points.begin();
				while(itr!=points.end())
				{
					//Translate the points
					itr->setX(itr->x() + dx);
					itr->setY(itr->y() + dy);

					//Align to grid if the flag is set
					if(align_objs_grid)
						(*itr)=alignPointToGrid(*itr);

					itr++;
				}

				//Assing the new points to relationship and reconfigure its line
				rel->getUnderlyingObject()->setPoints(points);
				rel->configureLine();
			}

			/* Storing the relationship's bounding rect so we can
			 * request that the viewport adjust the current position to make
			 * the selection visible */
			sel_rect = sel_rect.united(rel->boundingRect());
		}
	}

	for(auto &obj : tables)
	{
		tab_view = dynamic_cast<BaseTableView *>(obj);

		//Realign tables if the parent schema had the position adjusted too
		if(align_objs_grid)
		{
			tab_view->setPos(alignPointToGrid(tab_view->pos()));
			schemas.insert(dynamic_cast<Schema *>(tab_view->getUnderlyingObject()->getSchema()));
		}

		if(BaseObjectView::isPlaceholderEnabled())
			tab_view->requestRelationshipsUpdate();
	}

	//Updating schemas bounding rects after moving objects
	for(auto &obj : schemas)
		obj->setModified(true);

	moving_objs=false;
	sel_ini_pnt.setX(DNaN);
	sel_ini_pnt.setY(DNaN);
	updateLayerRects();

	QRectF old_scene_rect = sceneRect(),
			rect = adjustSceneRect(true);

	/* We invalidate the entire scene if the old scene size differs from the new one
	 * calculated based upon the items bounding rects after objects movement */
	if(old_scene_rect.topLeft() != rect.topLeft() ||
		 old_scene_rect.height() != rect.height() ||
		 old_scene_rect.width() != rect.width())
	{
		emit s_ensureVisibleRequested(sel_rect);
	}

	emit s_objectsMoved(true);
}

QRectF ObjectsScene::adjustSceneRect(bool expand_only)
{
	QRectF rect = this->itemsBoundingRect(true, false, true),
				scn_rect = sceneRect();

	// Create a default-sized rectangle if there are no items on the scene
	if(rect.isNull())
		rect = QRectF(0, 0, min_scene_width, min_scene_height);

	/* If the expand_only is false, we always adjust the scene rect
	 * to the items bounding rect with a padding (using grid_size).
	 * In this case, the rectangle can be expanded or shrinked but never
	 * shrinks in such a way to be lesser that the
	 * default scene rect (0,0, min_scene_width, min_scene_height) */
	else if(!expand_only)
	{
		double lp = 0, tp = 0;

		if(rect.left() <= 0)
			lp = -static_cast<double>(grid_size);
		else
			lp = -rect.left();

		if(rect.top() <= 0)
			tp = -static_cast<double>(grid_size);
		else
			tp = -rect.top();

		rect.adjust(lp, tp, grid_size, grid_size);
	}
	/* In this case, the scene rect always expands.
	 * This means that the items rectangle coordinates that
	 * are beyond (in any direction, positive/negative) are always
	 * used as the new scene rectangle coordinates */
	else
	{
		double left = std::min(scn_rect.left(), rect.left() - grid_size),
				top = std::min(scn_rect.top(), rect.top() - grid_size),
				right = std::max(scn_rect.right(), rect.right() + grid_size),
				bottom = std::max(scn_rect.bottom(), rect.bottom() + grid_size);

		rect = QRectF(QPointF(left, top), QPointF(right, bottom));
	}

	setSceneRect(rect);
	return rect;
}

void ObjectsScene::alignObjectsToGrid()
{
	QList<QGraphicsItem *> items=this->items();
	RelationshipView *rel=nullptr;
	BaseTableView *tab=nullptr;
	TextboxView *lab=nullptr;
	std::vector<QPointF> points;
	std::vector<Schema *> schemas;
	unsigned i1, count1;

	for(auto &item : items)
	{
		if(dynamic_cast<QGraphicsItemGroup *>(item) && !item->parentItem())
		{
			tab = dynamic_cast<BaseTableView *>(item);
			rel = dynamic_cast<RelationshipView *>(item);

			if(tab)
				tab->setPos(this->alignPointToGrid(tab->pos()));
			else if(rel)
			{
				//Align the relationship points
				points = rel->getUnderlyingObject()->getPoints();
				count1 = points.size();

				for(i1 = 0; i1 < count1; i1++)
					points[i1] = this->alignPointToGrid(points[i1]);

				if(count1 > 0)
				{
					rel->getUnderlyingObject()->setPoints(points);
					rel->configureLine();
				}

				//Align the labels
				for(i1 = BaseRelationship::SrcCardLabel; i1 <= BaseRelationship::RelNameLabel; i1++)
				{
					lab = rel->getLabel(static_cast<BaseRelationship::LabelId>(i1));

					if(lab)
						lab->setPos(this->alignPointToGrid(lab->pos()));
				}
			}
			else if(!dynamic_cast<SchemaView *>(item))
				item->setPos(this->alignPointToGrid(item->pos()));
			else
				schemas.push_back(dynamic_cast<Schema *>(dynamic_cast<BaseObjectView *>(item)->getUnderlyingObject()));
		}
	}

	//Updating schemas dimensions
	while(!schemas.empty())
	{
		schemas.back()->setModified(true);
		schemas.pop_back();
	}

	updateLayerRects();
}

void ObjectsScene::update()
{
	QGraphicsScene::update(this->sceneRect());
}

void ObjectsScene::clearTablesChildrenSelection()
{
	for(auto &tab_obj_view : tabs_sel_children)
		tab_obj_view->clearChildrenSelection();

	tabs_sel_children.clear();
}

void ObjectsScene::clearSelection()
{
	clearTablesChildrenSelection();
	QGraphicsScene::clearSelection();
}

QList<QRectF> ObjectsScene::getPagesForPrinting(const QPageLayout &page_lt, unsigned &h_page_cnt, unsigned &v_page_cnt, double scale)
{
	QList<QRectF> pages;
	QRectF page_rect, scn_rect = sceneRect(),
			items_rect = itemsBoundingRect(true);
	double page_w = 0, page_h = 0;
	int start_h = 0, start_v = 0,
			end_h = 0, end_v = 0;

	if(scale < MinScaleFactor)
		scale = MinScaleFactor;
	else if(scale > MaxScaleFactor)
		scale = MaxScaleFactor;

	page_w = page_lt.paintRect(QPageLayout::Point).width();
	page_w /= scale;

	page_h = page_lt.paintRect(QPageLayout::Point).height();
	page_h /= scale;

	/* If the origin point is before (0,0) we have to calculate the number
	 * pages are before that coordinate */
	if(scn_rect.top() < 0 || scn_rect.left() < 0)
	{
		start_h = round(scn_rect.left() / page_w) - 1;
		start_v = round(scn_rect.top() / page_h) - 1;
	}

	/* Calculates the horizontal and vertical page count to iterate
	 * based upon the passed paper size and scene rect.
	 * This is not the real page count since we still need to detect
	 * empty pages that don't need to be printed */
	h_page_cnt = round(scn_rect.width() / page_w) + 1;
	v_page_cnt = round(scn_rect.height() / page_h) + 1;

	end_h = start_h + h_page_cnt;
	end_v = start_v + v_page_cnt;

	QList<QPoint> pg_ids;

	for(int curr_v = start_v; curr_v < end_v; curr_v++)
	{
		for(int curr_h = start_h; curr_h < end_h; curr_h++)
		{
			// Calculates the current page rectangle
			page_rect = QRectF(QPointF(curr_h * page_w, curr_v * page_h),
													QSizeF(page_w + 1, page_h + 1));

			/* We consider only page rects that intersect the items bounding rect.
			 * This will avoid printing extra/uneeded pages */
			if(items_rect.intersects(page_rect))
			{
				/* Store the page position so we can calculate the exact number
				 * of pages to be printed */
				pg_ids.append(QPoint(curr_h, curr_v));
				pages.append(page_rect);
			}
		}
	}

	if(pg_ids.isEmpty())
		h_page_cnt = v_page_cnt = 0;
	else
	{
		QPoint f_id = pg_ids.first(), l_id = pg_ids.last();
		h_page_cnt = (l_id.x() - f_id.x()) + 1;
		v_page_cnt = (l_id.y() - f_id.y()) + 1;
	}

	return pages;
}

QList<QRectF> ObjectsScene::getPagesForPrinting(unsigned &h_page_cnt, unsigned &v_page_cnt)
{
	return getPagesForPrinting(page_layout, h_page_cnt, v_page_cnt, 1);
}

bool ObjectsScene::isRangeSelectionEnabled()
{
	return enable_range_sel;
}

bool ObjectsScene::isRangeSelectionTriggerInverted()
{
	return invert_rangesel_trigger;
}

bool ObjectsScene::isRelationshipLineVisible()
{
	return rel_line->isVisible();
}

bool ObjectsScene::isMovingObjects()
{
	return moving_objs;
}

QList<QGraphicsItem *> ObjectsScene::selectedItems() const
{
	if(tabs_sel_children.empty())
		return QGraphicsScene::selectedItems();

	QList<QGraphicsItem *> items = QGraphicsScene::selectedItems();

	for(auto &tab_view :tabs_sel_children)
	{
		for(auto &tab_obj : tab_view->getSelectedChidren())
			items.append(tab_obj);
	}

	return items;
}

bool ObjectsScene::hasOnlyTableChildrenSelection() const
{
	return QGraphicsScene::selectedItems().isEmpty() && !tabs_sel_children.isEmpty();
}

void ObjectsScene::expandSceneRect(ObjectsScene::ExpandDirection exp_dir)
{
	QRectF scn_rect = sceneRect(),
			pg_rect = getPageLayout().paintRect();

	switch(exp_dir)
	{
		case ExpandTop:
			scn_rect.adjust(0, -pg_rect.height() * expansion_factor, 0, 0);
		break;

		case ExpandBottom:
			scn_rect.adjust(0, 0, 0, pg_rect.height() * expansion_factor);
		break;

		case ExpandLeft:
			scn_rect.adjust(-pg_rect.width() * expansion_factor, 0, 0, 0);
		break;

		case ExpandRight:
		default:
			scn_rect.adjust(0, 0, pg_rect.width() * expansion_factor, 0);
		break;
	}

	setSceneRect(scn_rect);
}
