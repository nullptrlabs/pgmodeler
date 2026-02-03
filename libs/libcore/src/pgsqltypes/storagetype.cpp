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

#include "storagetype.h"

const QStringList StorageType::type_names {
	"", // Reserved for Class::Null
	"plain",
	"external",
	"extended",
	"main"
};

StorageType::StorageType()
{
	type_idx = Plain;
}

StorageType::StorageType(unsigned type_id)
{
	setType(type_id);
}

QStringList StorageType::getTypes()
{
	return TemplateType<StorageType>::getTypes(type_names);
}

StorageType::StorageType(const QString &type_name)
{
	setType(type_name);
}

bool StorageType::operator == (const char *type_name)
{
	return ((*this)==QString(type_name));
}

bool StorageType::operator == (const QString &type_name)
{
	return (type_idx == static_cast<unsigned>(type_names.indexOf(type_name)));
}

bool StorageType::operator != (const QString &type_name)
{
	return (!((*this)==type_name));
}

bool StorageType::operator != (StorageType type)
{
	return (this->type_idx!=type.type_idx);
}

unsigned StorageType::setType(unsigned type_id)
{
	return TemplateType<StorageType>::setType(type_id, type_names);
}

unsigned StorageType::setType(const QString &type_name)
{
	return TemplateType<StorageType>::setType(type_name, type_names);
}

QString StorageType::getTypeName(unsigned type_id)
{
	return TemplateType<StorageType>::getTypeName(type_id, type_names);
}
