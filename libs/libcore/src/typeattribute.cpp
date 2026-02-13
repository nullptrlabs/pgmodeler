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

#include "typeattribute.h"

TypeAttribute::TypeAttribute()
{
	obj_type=ObjectType::TypeAttribute;
}

TypeAttribute::TypeAttribute(const TypeAttribute &type_attr) : TypeAttribute()
{
	type = type_attr.type;
	obj_name = type_attr.obj_name;
	collation = type_attr.collation;
}

void TypeAttribute::setType(PgSqlType type)
{
	setCodeInvalidated(this->type != type);
	this->type=type;
}

PgSqlType TypeAttribute::getType()
{
	return type;
}

QString TypeAttribute::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::Name]=BaseObject::formatName(obj_name);
	else
		attributes[Attributes::Name]=obj_name;

	attributes[Attributes::Type]=type.getSourceCode(def_type);

	if(collation)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Collation]=collation->getName(true);
		else
			attributes[Attributes::Collation]=collation->getSourceCode(def_type, true);
	}

	return BaseObject::__getSourceCode(def_type);
}

void TypeAttribute::operator = (const TypeAttribute &tpattrib)
{
    this->obj_name = tpattrib.obj_name;
		this->type = tpattrib.type;
		this->collation = tpattrib.collation;
}
