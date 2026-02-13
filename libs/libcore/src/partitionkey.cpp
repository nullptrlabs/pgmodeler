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

#include "partitionkey.h"

PartitionKey::PartitionKey()
{
	collation = nullptr;
}

void PartitionKey::setCollation(Collation *collation)
{
	this->collation=collation;
}

Collation *PartitionKey::getCollation()
{
	return collation;
}

QString PartitionKey::getSourceCode(SchemaParser::CodeType def_type)
{
	attribs_map attribs;
	schparser.setPgSQLVersion(BaseObject::getPgSQLVersion(), BaseObject::isDbVersionIgnored());

	attribs[Attributes::Collation]="";
	configureAttributes(attribs, def_type);

	if(collation)
	{
		if(def_type==SchemaParser::SqlCode)
			attribs[Attributes::Collation]=collation->getName(true);
		else
			attribs[Attributes::Collation]=collation->getSourceCode(def_type, true);
	}

	return schparser.getSourceCode(Attributes::PartitionKey, attribs, def_type);
}

std::vector<BaseObject *> PartitionKey::getDependencies()
{
	std::vector<BaseObject *> deps = Element::getDependencies();

	if(collation)
		deps.push_back(collation);

	return deps;
}
