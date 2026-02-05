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

#include "simplecolumn.h"
#include "schemaparser.h"
#include "attribsmap.h"
#include "attributes.h"

SimpleColumn::SimpleColumn()
{

}

SimpleColumn::SimpleColumn(const SimpleColumn &col) : SimpleColumn(col.name, col.type, col.alias)
{

}

SimpleColumn::SimpleColumn(const QString &_name, const QString &_type, const QString &_alias)
{
	name = _name;
	type = _type;
	alias = _alias;
}

QString SimpleColumn::getName() const
{
	return name;
}

QString SimpleColumn::getType() const
{
	return type;
}

QString SimpleColumn::getAlias() const
{
	return alias;
}

bool SimpleColumn::operator == (const SimpleColumn &col) const
{
	return name == col.name && type == col.type && alias == col.alias;
}

SimpleColumn &SimpleColumn::operator = (const SimpleColumn &col)
{
	this->name = col.name;
	this->type = col.type;
	this->alias = col.alias;
	return *this;
}

bool SimpleColumn::isValid() const
{
	return !name.isEmpty() && !type.isEmpty();
}

QString SimpleColumn::getXmlCode() const
{
	SchemaParser schparser;
	attribs_map attribs;

	attribs[Attributes::Name] = name;
	attribs[Attributes::Alias] = alias;
	attribs[Attributes::Type] = type;

	return schparser.getSourceCode(Attributes::SimpleCol, attribs, SchemaParser::XmlCode);
}
