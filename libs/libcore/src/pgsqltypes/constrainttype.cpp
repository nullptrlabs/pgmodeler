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

#include "constrainttype.h"

const QStringList ConstraintType::type_names {
	"", // Reserved for Class::Null
	"PRIMARY KEY",
	"FOREIGN KEY",
	"CHECK",
	"UNIQUE",
	"EXCLUDE",
};

ConstraintType::ConstraintType(unsigned type_id)
{
	setType(type_id);
}

ConstraintType::ConstraintType(const QString &type_name)
{
	setType(type_name);
}

ConstraintType::ConstraintType()
{
	type_idx = PrimaryKey;
}

QStringList ConstraintType::getTypes()
{
	return TemplateType<ConstraintType>::getTypes(type_names);
}

unsigned ConstraintType::setType(unsigned type_id)
{
	return TemplateType<ConstraintType>::setType(type_id, type_names);
}

unsigned ConstraintType::setType(const QString &type_name)
{
	return TemplateType<ConstraintType>::setType(type_name, type_names);
}

QString ConstraintType::getTypeName(unsigned type_id)
{
	return TemplateType<ConstraintType>::getTypeName(type_id, type_names);
}
