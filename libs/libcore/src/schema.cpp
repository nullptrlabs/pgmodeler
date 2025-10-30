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

#include "schema.h"

unsigned Schema::schema_id {3000};

Schema::Schema()
{
	object_id = Schema::schema_id++;
	obj_type = ObjectType::Schema;
	fill_color = QColor(225,225,225, 80);
	rect_visible = false;
	attributes[Attributes::FillColor] = "";
	attributes[Attributes::NameColor] = "";
	attributes[Attributes::RectVisible] = "";
}

void Schema::setName(const QString &name)
{
	/* Schema names starting with pg_ is reserved to PostgreSQL if its the case
		raises an error */
	if(name.startsWith("pg_"))
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgReservedName)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Schema)),
						ErrorCode::AsgReservedName,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObject::setName(name);
}

void Schema::setNameColor(const QColor &color)
{
	setCodeInvalidated(name_color != color);
	name_color = color;
}

QColor Schema::getNameColor()
{
 return name_color;
}

void Schema::setFillColor(const QColor &color)
{
	setCodeInvalidated(fill_color != color);
	this->fill_color=color;
}

QColor Schema::getFillColor()
{
	return fill_color;
}

void Schema::setRectVisible(bool value)
{
	setCodeInvalidated(rect_visible != value);
	rect_visible=value;
}

bool Schema::isRectVisible()
{
	return rect_visible;
}

QString Schema::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::FillColor]=fill_color.name();
	attributes[Attributes::NameColor]=name_color.name();
	attributes[Attributes::RectVisible]=(rect_visible ? Attributes::True : "");

	setFadedOutAttribute();
	setLayersAttribute();

	return BaseObject::__getSourceCode(def_type);
}
