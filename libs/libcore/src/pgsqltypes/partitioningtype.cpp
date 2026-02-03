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

#include "partitioningtype.h"

const QStringList PartitioningType::type_names {
	"", // Reserved for Class::Null
	"RANGE",
	"LIST",
	"HASH"
};

PartitioningType::PartitioningType()
{
	type_idx = Range;
}

QStringList PartitioningType::getTypes()
{
	return TemplateType<PartitioningType>::getTypes(type_names);
}

PartitioningType::PartitioningType(unsigned type_id)
{
	setType(type_id);
}

PartitioningType::PartitioningType(const QString &type_name)
{
	setType(type_name);
}

unsigned PartitioningType::setType(unsigned type_id)
{
	return TemplateType<PartitioningType>::setType(type_id, type_names);
}

unsigned PartitioningType::setType(const QString &type_name)
{
	return TemplateType<PartitioningType>::setType(type_name, type_names);
}

QString PartitioningType::getTypeName(unsigned type_id)
{
	return TemplateType<PartitioningType>::getTypeName(type_id, type_names);
}
