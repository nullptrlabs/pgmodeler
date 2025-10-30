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

#include "tabletitleview.h"
#include "schema.h"
#include "tag.h"
#include "physicaltable.h"

TableTitleView::TableTitleView() : BaseObjectView(nullptr)
{
	schema_name=new QGraphicsSimpleTextItem;
	schema_name->setZValue(1);

	obj_name=new QGraphicsSimpleTextItem;
	obj_name->setZValue(1);

	box=new RoundedRectItem;
	box->setRoundedCorners(RoundedRectItem::TopLeftCorner | RoundedRectItem::TopRightCorner);
	box->setZValue(0);
}

TableTitleView::~TableTitleView()
{
	delete schema_name;
	delete obj_name;
	delete box;
}

void TableTitleView::configureObject(BaseGraphicObject *object)
{
	QTextCharFormat fmt;
	QString name_attrib, schema_name_attrib, title_color_attrib;
	QPen pen;
	Schema *schema=nullptr;
	QFont font;
	Tag *tag=nullptr;
	PhysicalTable *table = dynamic_cast<PhysicalTable *>(object);

	//Raises an error if the object related to the title is not allocated
	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	//Raises an error if the object is invalid
	if(!BaseTable::isBaseTable(object->getObjectType()))
		throw Exception(ErrorCode::OprObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);

	schema=dynamic_cast<Schema *>(object->getSchema());
	tag=dynamic_cast<BaseTable *>(object)->getTag();

	if(object->getObjectType()==ObjectType::View && !tag)
	{
		name_attrib=Attributes::ViewName;
		schema_name_attrib=Attributes::ViewSchemaName;
		title_color_attrib=Attributes::ViewTitle;
	}
	else if(object->getObjectType()==ObjectType::ForeignTable && !tag)
	{
		name_attrib=Attributes::ForeignTableName;
		schema_name_attrib=Attributes::ForeignTableSchemaName;
		title_color_attrib=Attributes::ForeignTableTitle;
	}
	else
	{
		name_attrib=Attributes::TableName;
		schema_name_attrib=Attributes::TableSchemaName;
		title_color_attrib=Attributes::TableTitle;
	}

	fmt=font_config[schema_name_attrib];
	font=fmt.font();
	schema_name->setFont(font);

	if(!tag)
		schema_name->setBrush(fmt.foreground());
	else
		schema_name->setBrush(tag->getElementColor(schema_name_attrib, ColorId::FillColor1));

	if(schema->isRectVisible())
		schema_name->setText(" ");
	else
	{
		if(compact_view && !schema->getAlias().isEmpty())
			schema_name->setText(schema->getAlias() + ".");
		else
			schema_name->setText(schema->getName() + ".");
	}

	fmt=font_config[name_attrib];
	font=fmt.font();

	obj_name->setFont(font);
	obj_name->setText(compact_view && !object->getAlias().isEmpty() ? object->getAlias() : object->getName());

	if(!tag)
	{
		obj_name->setBrush(fmt.foreground());
		box->setBrush(this->getFillStyle(title_color_attrib));
	}
	else
	{
		obj_name->setBrush(tag->getElementColor(name_attrib, ColorId::FillColor1));
		box->setBrush(tag->getFillStyle(title_color_attrib));
	}

	pen=this->getBorderStyle(title_color_attrib);

	if(tag)
		pen.setColor(tag->getElementColor(title_color_attrib, ColorId::BorderColor));

	if(object->getObjectType()==ObjectType::View ||
		 (table && table->isPartition()))
		pen.setStyle(Qt::DashLine);

	box->setPen(pen);

	if(schema->isRectVisible())
		this->resizeTitle(obj_name->boundingRect().width()  + (2 * HorizSpacing),
											obj_name->boundingRect().height() + (2 * VertSpacing));
	else
		this->resizeTitle(obj_name->boundingRect().width() + schema_name->boundingRect().width() + (2 * HorizSpacing),
											schema_name->boundingRect().height() + (2 * VertSpacing));
}

void TableTitleView::resizeTitle(double width, double height)
{
	double py = height / 1.5;

	box->setPos(0,0);
	box->setRect(QRectF(0,0, width, height));

	if(schema_name->text()==" ")
		obj_name->setPos((box->boundingRect().width() - obj_name->boundingRect().width())/2.0, py);
	else
	{
		schema_name->setPos((box->boundingRect().width() - (schema_name->boundingRect().width() + obj_name->boundingRect().width()))/2.0, py);
		obj_name->setPos(schema_name->pos().x() + schema_name->boundingRect().width(), py);
	}

	this->bounding_rect.setSize(QSizeF(box->boundingRect().width(), box->boundingRect().height()));
}

void TableTitleView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	box->paint(painter, option, widget);

	painter->setFont(schema_name->font());
	painter->setPen(schema_name->brush().color());
	painter->drawText(schema_name->pos(), schema_name->text());

	painter->setFont(obj_name->font());
	painter->setPen(obj_name->brush().color());
	painter->drawText(obj_name->pos(), obj_name->text());
}
