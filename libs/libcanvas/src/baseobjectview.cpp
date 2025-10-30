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

#include "baseobjectview.h"
#include "roundedrectitem.h"
#include "objectsscene.h"
#include "utilsns.h"
#include <QApplication>
#include <QScreen>
#include <QTimeLine>

unsigned BaseObjectView::global_sel_order {1};

bool BaseObjectView::use_placeholder {true};
bool BaseObjectView::compact_view {false};
bool BaseObjectView::hide_shadow {false};

std::map<QString, QTextCharFormat> BaseObjectView::font_config;
std::map<QString, std::vector<QColor>> BaseObjectView::color_config;

BaseObjectView::BaseObjectView(BaseObject *object)
{
	sel_order=0;
	protected_icon=nullptr;
	obj_shadow=nullptr;
	obj_selection=nullptr;
	pos_info_item=nullptr;
	sql_disabled_item=nullptr;
	placeholder=nullptr;
	setSourceObject(object);

	fade_tl.setFrameRange(0, 100);
	fade_tl.setEasingCurve(QEasingCurve::Linear);

	connect(&fade_tl, &QTimeLine::frameChanged, this, [this](int frame){
		setOpacity(frame / 100.0);
	});

	connect(&fade_tl, &QTimeLine::finished, this, [this]() {
		setOpacity(fade_tl.property(FinalOpacity).toReal());
	});
}

void BaseObjectView::blink()
{
	fade(true, 200, 3);
}

void BaseObjectView::fade(bool fd_in, int duration, int loop_cnt, qreal final_opacity)
{
	fade_tl.setDirection(fd_in ? QTimeLine::Forward : QTimeLine::Backward);
	fade_tl.setLoopCount(loop_cnt);
	fade_tl.setDuration(duration);
	fade_tl.setProperty(FinalOpacity, final_opacity);
	fade_tl.start();
}

void BaseObjectView::fadeIn()
{
	if(opacity() >= 1)
		return;

	fade(true, 200);
}

void BaseObjectView::fadeOut(qreal min_opacity)
{
	if(opacity() <= min_opacity)
		return;

	fade(false, 200, 1, min_opacity);
}

BaseObjectView::~BaseObjectView()
{  
	setSourceObject(nullptr);
}

void BaseObjectView::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	if(event->button()==Qt::RightButton && !this->isSelected())
	{
		//Faking an left-click in order to force the object selection using the right button.
		QGraphicsSceneMouseEvent *m_event=new QGraphicsSceneMouseEvent;
		m_event->setPos(event->pos());
		m_event->setScenePos(event->scenePos());
		m_event->setScreenPos(event->screenPos());
		m_event->setButton(Qt::LeftButton);

		QGraphicsItemGroup::mousePressEvent(m_event);
		event->ignore();
	}
	else if(event->button()==Qt::LeftButton)
	{
		QGraphicsItemGroup::mousePressEvent(event);
	}
}

void BaseObjectView::setSourceObject(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	//Stores the reference to the source object as the data of graphical object
	this->setData(0, QVariant::fromValue<void *>(object));

	if(!graph_obj)
	{
		if(obj_shadow)
		{
			this->removeFromGroup(obj_shadow);
			delete obj_shadow;
			obj_shadow=nullptr;
		}

		if(protected_icon)
		{
			this->removeFromGroup(protected_icon);
			delete protected_icon;
			protected_icon=nullptr;
		}

		if(pos_info_item)
		{
			this->removeFromGroup(pos_info_item);
			delete pos_info_item;
			pos_info_item=nullptr;
		}

		if(sql_disabled_item)
		{
			this->removeFromGroup(sql_disabled_item);
			delete sql_disabled_item;
			sql_disabled_item=nullptr;
		}

		if(placeholder)
		{
			delete placeholder;
			placeholder=nullptr;
		}
	}
	else
	{
		QGraphicsPolygonItem *pol_item=nullptr;

		graph_obj->disconnect();
		graph_obj->setReceiverObject(this);
		connect(graph_obj, &BaseGraphicObject::s_objectProtected, this, &BaseObjectView::toggleProtectionIcon);

		//By default the item can be selected and send geometry changes to the scene
		this->setFlags(QGraphicsItem::ItemIsSelectable |
					   QGraphicsItem::ItemSendsGeometryChanges);
		//The object is only movable if is not protected
		this->setFlag(QGraphicsItem::ItemIsMovable, !graph_obj->isProtected());

		if(!protected_icon)
		{
			protected_icon=new QGraphicsItemGroup;
			protected_icon->setVisible(graph_obj->isProtected());
			protected_icon->setZValue(3);

			pol_item=new QGraphicsPolygonItem;
			protected_icon->addToGroup(pol_item);

			pol_item=new QGraphicsPolygonItem;
			protected_icon->addToGroup(pol_item);

			this->addToGroup(protected_icon);
		}

		if(!pos_info_item)
		{
			pos_info_item=new TextPolygonItem;
			pos_info_item->setZValue(10);
			this->addToGroup(pos_info_item);
		}

		if(!sql_disabled_item && object->getObjectType()!=ObjectType::Textbox)
		{
			sql_disabled_item=new TextPolygonItem;
			sql_disabled_item->setZValue(100);
			this->addToGroup(sql_disabled_item);
		}
	}
}

BaseObject *BaseObjectView::getUnderlyingObject()
{
	return reinterpret_cast<BaseObject *>(this->data(0).value<void *>());
}

void BaseObjectView::setFontStyle(const QString &id, QTextCharFormat font_fmt)
{
	/* If the font style is not the global one we use the settings
	 * of the global (if available) */
	if(id != Attributes::Global)
	{
		QFont font = font_config[Attributes::Global].font();
		font.setItalic(font_fmt.font().italic());
		font.setBold(font_fmt.font().bold());
		font.setUnderline(font_fmt.font().underline());
		font_fmt.setFont(font);
	}
	else
	{
		// If changing the global font settings, we apply it to all other font styles alread configured
		QFont font = font_fmt.font();
		for(auto &itr : font_config)
		{
			font.setItalic(itr.second.font().italic());
			font.setBold(itr.second.font().bold());
			font.setUnderline(itr.second.font().underline());
			itr.second.setFont(font);
		}
	}

	font_config[id] = font_fmt;
}

void BaseObjectView::setElementColor(const QString &id, QColor color, ColorId color_id)
{
	if(color_id > ColorId::BorderColor)
		return;

	// If the provided element id does not exist we initialize it
	if(color_config.count(id) == 0)
		color_config[id] = { QColor(0,0,0), QColor(0,0,0), QColor(0,0,0) };

	color_config[id][enum_t(color_id)] = color;
}

QColor BaseObjectView::getElementColor(const QString &id, ColorId color_id)
{
	if(color_config.count(id) > 0 && color_id <= ColorId::BorderColor)
		return color_config[id][enum_t(color_id)];

	return { 0, 0, 0 };
}

void BaseObjectView::getFillStyle(const QString &id, QColor &color1, QColor &color2)
{
	if(color_config.count(id) == 0)
		return;

	color1 = color_config[id][enum_t(ColorId::FillColor1)];
	color2 = color_config[id][enum_t(ColorId::FillColor2)];
}

QLinearGradient BaseObjectView::getFillStyle(const QString &id)
{
	std::vector<QColor> colors;
	QLinearGradient grad(QPointF(0,0),QPointF(0,1));

	if(color_config.count(id) > 0)
	{
		int alpha = 255;
		colors=color_config[id];

		if(!colors.empty())
		{
			if(id==Attributes::ObjSelection || id==Attributes::Placeholder)
				alpha = ObjectAlphaChannel;
			else if(id == Attributes::ObjShadow)
				alpha = ObjectShadowAlphaChannel;

			colors[0].setAlpha(alpha);
			colors[1].setAlpha(alpha);
			grad.setCoordinateMode(QGradient::ObjectBoundingMode);
			grad.setColorAt(0, colors[0]);
			grad.setColorAt(1, colors[1]);
		}
	}

	return grad;
}

QPen BaseObjectView::getBorderStyle(const QString &id)
{
	QPen pen(Qt::NoPen);

	if(id != Attributes::ObjShadow && color_config.count(id) > 0)
	{
		std::vector<QColor> colors = color_config[id];

		if(!colors.empty())
		{
			if(id==Attributes::ObjSelection)
				colors[2].setAlpha(ObjectAlphaChannel);

			pen.setWidthF(ObjectBorderWidth * getScreenDpiFactor());
			pen.setColor(colors[2]);
			pen.setStyle(Qt::SolidLine);
		}
	}

	return pen;
}

QTextCharFormat BaseObjectView::getFontStyle(const QString &id)
{
	if(font_config.count(id))
		return font_config[id];
	
	return { };
}

void BaseObjectView::setPlaceholderEnabled(bool value)
{
	use_placeholder=value;
}

bool BaseObjectView::isPlaceholderEnabled()
{
	return use_placeholder;
}

void BaseObjectView::setCompactViewEnabled(bool value)
{
	compact_view = value;
}

bool BaseObjectView::isCompactViewEnabled()
{
	return compact_view;
}

void BaseObjectView::setShadowHidden(bool value)
{
	hide_shadow = value;
}

bool BaseObjectView::isShadowHidden()
{
	return hide_shadow;
}

QVariant BaseObjectView::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if(change==ItemPositionHasChanged)
	{
		BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

		if(graph_obj && !graph_obj->isProtected())
		{
			if(ObjectsScene::isAlignObjectsToGrid() && this->isSelected())
				this->setPos(ObjectsScene::alignPointToGrid(this->scenePos()));

			graph_obj->setPosition(this->scenePos());
			this->configurePositionInfo(this->pos());
		}
	}
	else if(change == ItemSelectedHasChanged && obj_selection)
	{
		this->setSelectionOrder(value.toBool());
		pos_info_item->setVisible(value.toBool());
		obj_selection->setVisible(value.toBool());

		this->configurePositionInfo(this->pos());
		emit s_objectSelected(dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject()), value.toBool());
	}

	return value;
}

void BaseObjectView::setSelectionOrder(bool selected)
{
	if(this->sel_order==0 && selected)
		this->sel_order=++BaseObjectView::global_sel_order;
	else if(!selected)
		this->sel_order=0;
}

QRectF BaseObjectView::boundingRect() const
{
	return bounding_rect;
}

void BaseObjectView::toggleProtectionIcon(bool value)
{
	BaseGraphicObject *obj_graf=dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	protected_icon->setVisible(value);
	this->setFlag(QGraphicsItem::ItemIsMovable, !value);

	if(obj_graf)
		obj_graf->setModified(true);
}

void BaseObjectView::configureObjectShadow()
{
	if(obj_shadow)
	{
		obj_shadow->setVisible(!hide_shadow);
		obj_shadow->setFlag(QGraphicsItem::ItemHasNoContents, hide_shadow);
		obj_shadow->setFlag(QGraphicsItem::ItemSendsGeometryChanges, !hide_shadow);
	}
}

void BaseObjectView::configureObjectSelection()
{
	RoundedRectItem *rect_item=dynamic_cast<RoundedRectItem *>(obj_selection);

	if(rect_item)
	{
		rect_item->setRect(this->boundingRect());
		rect_item->setPos(0, 0);
		rect_item->setBorderRadius(RoundedRectItem::DefaultRadius);
		rect_item->setBrush(this->getFillStyle(Attributes::ObjSelection));
		rect_item->setPen(this->getBorderStyle(Attributes::ObjSelection));
	}
}

void BaseObjectView::configurePositionInfo(QPointF pos)
{
	if(this->isSelected())
	{
		QFont fnt=font_config[Attributes::PositionInfo].font();

		pos_info_item->setBrush(BaseObjectView::getFillStyle(Attributes::PositionInfo));
		pos_info_item->setPen(BaseObjectView::getBorderStyle(Attributes::PositionInfo));

		fnt.setPointSizeF(fnt.pointSizeF() * 0.95);
		pos_info_item->setFont(fnt);
		pos_info_item->setTextColor(font_config[Attributes::PositionInfo].foreground().color());

		pos_info_item->setText(QString(" x:%1 y:%2 ").arg(round(pos.x())).arg(round(pos.y())));
		pos_info_item->setPolygon(QPolygonF(pos_info_item->getTextBoundingRect()));

		pos_info_item->setPos(-0.5, -pos_info_item->boundingRect().height()/2);
	}
}

void BaseObjectView::configureSQLDisabledInfo()
{
	if(sql_disabled_item)
	{
		double px=0, py=0;

		sql_disabled_item->setVisible(this->getUnderlyingObject()->isSQLDisabled());

		if(this->getUnderlyingObject()->isSQLDisabled())
		{
			QTextCharFormat char_fmt;
			char_fmt=BaseObjectView::getFontStyle(Attributes::PositionInfo);
			char_fmt.setFontPointSize(char_fmt.font().pointSizeF() * 0.80);

			sql_disabled_item->setFont(char_fmt.font());
			sql_disabled_item->setText(tr("SQL off"));
			sql_disabled_item->setTextColor(char_fmt.foreground().color());

			sql_disabled_item->setPolygon(QRectF(QPointF(0,0), sql_disabled_item->getTextBoundingRect().size() + QSizeF(1.5 * HorizSpacing, 1.5 * VertSpacing)));
			sql_disabled_item->setPen(BaseObjectView::getBorderStyle(Attributes::PositionInfo));
			sql_disabled_item->setBrush(BaseObjectView::getFillStyle(Attributes::PositionInfo));

			px=bounding_rect.width() - sql_disabled_item->boundingRect().width() + (1.5 * HorizSpacing);
			py=-(sql_disabled_item->boundingRect().height()/2);

			sql_disabled_item->setPos(px, py);
			sql_disabled_item->setTextPos(HorizSpacing * 0.75, VertSpacing * 0.75);
		}
	}
}

void BaseObjectView::configureProtectedIcon()
{
	if(protected_icon)
	{
		QGraphicsPolygonItem *pol_item=nullptr;
		QPolygonF pol;
		double factor;

		//Calculates the factor used to resize the protection icon accordding the font size
		factor=font_config[Attributes::Global].font().pointSizeF()/DefaultFontSize;

		pol.append(QPointF(2,5)); pol.append(QPointF(2,2));
		pol.append(QPointF(3,1)); pol.append(QPointF(4,0));
		pol.append(QPointF(7,0)); pol.append(QPointF(8,1));
		pol.append(QPointF(9,2)); pol.append(QPointF(9,5));
		pol.append(QPointF(7,5)); pol.append(QPointF(7,3));
		pol.append(QPointF(6,2)); pol.append(QPointF(5,2));
		pol.append(QPointF(4,3)); pol.append(QPointF(4,5));

		if(factor!=1.0)
			resizePolygon(pol, pol.boundingRect().width() * factor,
										pol.boundingRect().height() * factor);

		pol_item=dynamic_cast<QGraphicsPolygonItem *>(protected_icon->childItems().at(0));
		pol_item->setPolygon(pol);
		pol_item->setBrush(this->getFillStyle(Attributes::LockerArc));
		pol_item->setPen(this->getBorderStyle(Attributes::LockerArc));

		pol.clear();
		pol.append(QPointF(1,5));  pol.append(QPointF(10,5));
		pol.append(QPointF(11,6)); pol.append(QPointF(11,9));
		pol.append(QPointF(10,10)); pol.append(QPointF(1,10));
		pol.append(QPointF(0,9)); pol.append(QPointF(0,6));

		if(factor!=1.0)
			resizePolygon(pol, pol.boundingRect().width() * factor,
										pol.boundingRect().height() * factor);

		pol_item=dynamic_cast<QGraphicsPolygonItem *>(protected_icon->childItems().at(1));
		pol_item->setPolygon(pol);
		pol_item->setBrush(this->getFillStyle(Attributes::LockerBody));
		pol_item->setPen(this->getBorderStyle(Attributes::LockerBody));
	}
}

void BaseObjectView::configurePlaceholder()
{
	if(!placeholder)
	{
		placeholder=new RoundedRectItem();
		placeholder->setVisible(false);
		placeholder->setZValue(-1);
		placeholder->setFlag(QGraphicsItem::ItemIsMovable, false);
		placeholder->setFlag(QGraphicsItem::ItemIsSelectable, false);
	}
}

void BaseObjectView::__configureObject()
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
	{
		this->setPos(graph_obj->getPosition());
		this->setToolTip(UtilsNs::formatMessage(
											QString("`%1' (%2)").arg(graph_obj->getName(true), graph_obj->getTypeName()) +
											QString("\n%1 Id: %2").arg(UtilsNs::DataSeparator, QString::number(graph_obj->getObjectId()))));
		this->configurePositionInfo(graph_obj->getPosition());
		this->configureProtectedIcon();
	}
}

unsigned BaseObjectView::getSelectionOrder()
{
	return sel_order;
}

QPointF BaseObjectView::getCenter()
{
	return { this->pos().x() + (this->boundingRect().width() / 2.0),
			 		 this->pos().y() + (this->boundingRect().height() / 2.0) };
}

void BaseObjectView::togglePlaceholder(bool visible)
{
	if(use_placeholder && placeholder && this->scene())
	{
		if(!placeholder->scene())
			this->scene()->addItem(placeholder);

		if(visible)
		{
			QPen pen=BaseObjectView::getBorderStyle(Attributes::Placeholder);
			pen.setStyle(Qt::DashLine);

			placeholder->setZValue(this->zValue() - 1);
			placeholder->setBrush(BaseObjectView::getFillStyle(Attributes::Placeholder));
			placeholder->setPen(pen);
			placeholder->setRect(QRectF(QPointF(0,0),this->bounding_rect.size()));
		}

		placeholder->setPos(this->mapToScene(this->bounding_rect.topLeft()));
		placeholder->setVisible(visible);
	}
}

double BaseObjectView::getFontFactor()
{
	return font_config[Attributes::Global].font().pointSizeF()/DefaultFontSize;
}

void BaseObjectView::setLayers(const QList<unsigned> &list)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		graph_obj->setLayers(list);
}

void BaseObjectView::addToLayer(unsigned layer_id)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		graph_obj->addToLayer(layer_id);
}

void BaseObjectView::removeFromLayer(unsigned layer_id)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		graph_obj->removeFromLayer(layer_id);
}

void BaseObjectView::resetLayers()
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		graph_obj->resetLayers();
}

QList<unsigned> BaseObjectView::getLayers()
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		return graph_obj->getLayers();

	return {0};
}

int BaseObjectView::getLayersCount()
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		return graph_obj->getLayersCount();

	return 0;
}

bool BaseObjectView::isInLayer(unsigned layer_id)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(this->getUnderlyingObject());

	if(graph_obj)
		return graph_obj->isInLayer(layer_id);

	return false;
}

double BaseObjectView::getScreenDpiFactor()
{
	QScreen *screen = qApp->primaryScreen();
	double factor = screen->logicalDotsPerInch() / 96.0,
			pixel_ratio = screen->devicePixelRatio(),
			dpi_factor = factor * pixel_ratio;

	/* Avoiding returning a factor less than 1 because
	 * this can cause improper objects resizes */
	if(factor <= 1.005)
		return 1;

	/* Special case for 4K or superior screens: we need to cap the dpi factor
	 * to 1.4 since on this kind of screen the dpi_factor results in a value of 2 or more
	 * and this can cause a super resizing of graphical objects */
	if(dpi_factor > MaxDpiFactor)
		return MaxDpiFactor;

	return dpi_factor;
}

void BaseObjectView::resizePolygon(QPolygonF &pol, double width, double height)
{
	QVector<QPointF>::iterator itr,itr_end;
	double coef_a, coef_b;

	itr=pol.begin();
	itr_end=pol.end();

	//Calculates the resize factor
	coef_a=width / pol.boundingRect().width();
	coef_b=height / pol.boundingRect().height();

	//Applies the resize factor to all the polygon points
	while(itr!=itr_end)
	{
		itr->setX(itr->x() * coef_a);
		itr->setY(itr->y() * coef_b);
		itr++;
	}
}
