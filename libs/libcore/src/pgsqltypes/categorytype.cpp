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

#include "categorytype.h"

const QStringList CategoryType::type_names {
	"", // Reserved for null value

	//See table 44-43 on PostgreSQL documentation
	"U", //User-defined types
	"A", //Array types
	"B", //Boolean types
	"C", //Composite types
	"D", //Date/time types
	"E", //Enum types
	"G", //Geometric types
	"I", //Network address types
	"N", //Numeric types
	"P", //Pseudo-types
	"S", //String types
	"T", //Timespan types
	"V", //Bit-string types
	"X", //Unknown type
};

CategoryType::CategoryType(const QString &type_name)
{
	setType(type_name);
}

CategoryType::CategoryType(unsigned type_id)
{
	setType(type_id);
}

CategoryType::CategoryType()
{
	type_idx = UserDefined;
}

QStringList CategoryType::getTypes()
{
	return TemplateType<CategoryType>::getTypes(type_names);
}

unsigned CategoryType::setType(unsigned type_id)
{
	return TemplateType<CategoryType>::setType(type_id, type_names);
}

unsigned CategoryType::setType(const QString &type_name)
{
	return TemplateType<CategoryType>::setType(type_name, type_names);
}

QString CategoryType::getTypeName(unsigned type_id)
{
	return TemplateType<CategoryType>::getTypeName(type_id, type_names);
}
