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

#include "basegraphicobject.h"

bool BaseGraphicObject::updates_enabled { true };

BaseGraphicObject::BaseGraphicObject()
{
	is_modified=true;
	is_faded_out=false;
	attributes[Attributes::XPos]="";
	attributes[Attributes::YPos]="";
	attributes[Attributes::Position]="";
	attributes[Attributes::FadedOut]="";
	attributes[Attributes::Layers]="";
	attributes[Attributes::ZValue]="";
	receiver_object=nullptr;
	z_value=0;
	resetLayers();
}

void BaseGraphicObject::setProtected(bool value)
{
	BaseObject::setProtected(value);
	emit s_objectProtected(this->isProtected());
}

void BaseGraphicObject::setSystemObject(bool value)
{
	BaseObject::setSystemObject(value);
}

void BaseGraphicObject::setModified(bool value)
{
	is_modified=value;

	if(is_modified && updates_enabled)
		emit s_objectModified();
}

void BaseGraphicObject::setSQLDisabled(bool value)
{
	BaseObject::setSQLDisabled(value);
}

void BaseGraphicObject::setFadedOut(bool value)
{
	setCodeInvalidated(is_faded_out != value);
	is_faded_out = value;
}

bool BaseGraphicObject::isModified()
{
	return is_modified;
}

bool BaseGraphicObject::isFadedOut()
{
	return is_faded_out;
}

void BaseGraphicObject::setFadedOutAttribute()
{
	attributes[Attributes::FadedOut]=(is_faded_out ? Attributes::True : "");
}

void BaseGraphicObject::setLayersAttribute()
{
	QStringList layers_str;

	for(auto &id : layers)
		layers_str.append(QString::number(id));

	attributes[Attributes::Layers] = layers_str.join(',');
}

void BaseGraphicObject::setPositionAttribute()
{
	attributes[Attributes::XPos]=QString("%1").arg(position.x());
	attributes[Attributes::YPos]=QString("%1").arg(position.y());
	attributes[Attributes::Position]=schparser.getSourceCode(Attributes::Position, attributes, SchemaParser::XmlCode);
}

void  BaseGraphicObject::setPosition(const QPointF &pos)
{
	setCodeInvalidated(position != pos);
	position=pos;
}

QPointF BaseGraphicObject::getPosition()
{
	return position;
}

void BaseGraphicObject::operator = (BaseGraphicObject &obj)
{
	*(dynamic_cast<BaseObject *>(this)) = dynamic_cast<BaseObject &>(obj);
	this->position = obj.position;
	this->z_value = obj.z_value;
}

void BaseGraphicObject::setReceiverObject(QObject *obj)
{
	if(!obj && receiver_object)
		disconnect(this, nullptr, receiver_object, nullptr);

	receiver_object=obj;
}

QObject *BaseGraphicObject::getOverlyingObject()
{
	return receiver_object;
}

bool BaseGraphicObject::isGraphicObject(ObjectType type)
{
	return (type==ObjectType::Table || type==ObjectType::View || type==ObjectType::Relationship ||
				 type==ObjectType::BaseRelationship || type==ObjectType::Textbox || type==ObjectType::Schema ||
				 type==ObjectType::ForeignTable);
}

void BaseGraphicObject::addToLayer(unsigned layer_id)
{
	if(!layers.contains(layer_id))
		layers.append(layer_id);

	setCodeInvalidated(true);
}

void BaseGraphicObject::removeFromLayer(unsigned layer_id)
{
	layers.removeAll(layer_id);
	setCodeInvalidated(true);
}

void BaseGraphicObject::resetLayers()
{
	layers.clear();
	layers.append(0);
}

void BaseGraphicObject::setLayers(QStringList list)
{
	setCodeInvalidated(true);

	//Sanitizing the string list by removing non-numbers
	list.replaceInStrings(QRegularExpression("^(.)*(\\D)+(.)*$"), "0");
	list.removeDuplicates();
	layers.clear();

	for(auto &id : list)
		addToLayer(id.toUInt());
}

void BaseGraphicObject::setLayers(QList<unsigned> list)
{
	setCodeInvalidated(true);
	layers = list;
}

QList<unsigned> BaseGraphicObject::getLayers()
{
	return layers;
}

bool BaseGraphicObject::isInLayer(unsigned layer_id)
{
	return layers.contains(layer_id);
}

int BaseGraphicObject::getLayersCount()
{
	return layers.size();
}

void BaseGraphicObject::setZValue(int z_value)
{
	if(z_value < MinZValue)
		z_value = MinZValue;
	else if(z_value > MaxZValue)
		z_value = MaxZValue;

	this->z_value = z_value;
}

int BaseGraphicObject::getZValue()
{
	return z_value;
}

void BaseGraphicObject::setUpdatesEnabled(bool value)
{
	updates_enabled = value;
}

bool BaseGraphicObject::isUpdatesEnabled()
{
	return updates_enabled;
}
