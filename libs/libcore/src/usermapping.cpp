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

#include "usermapping.h"

UserMapping::UserMapping()
{
	obj_type = ObjectType::UserMapping;
	foreign_server = nullptr;
	setName("");
	attributes[Attributes::Server] = "";
	attributes[Attributes::Options] = "";
}

void UserMapping::setForeignServer(ForeignServer *server)
{
	setCodeInvalidated(foreign_server != server);
	foreign_server = server;
	setName("");
}

ForeignServer *UserMapping::getForeignServer()
{
	return foreign_server;
}

void UserMapping::setOwner(BaseObject *role)
{
	BaseObject::setOwner(role);
	setName("");
}

void UserMapping::setName(const QString &)
{
	//Configures a fixed name for the user mapping (in form: role@server)
	this->obj_name=QString("%1@%2").arg(owner ? owner->getName() : "public")
								 .arg(foreign_server ? foreign_server->getName() : "");
}

QString UserMapping::getName(bool, bool)
{
	return this->obj_name;
}

QString UserMapping::getSignature(bool)
{
	return QString("FOR %1 SERVER %2").arg(owner ? owner->getName() : "public")
																		.arg(foreign_server ? foreign_server->getName() : "");
}

QString UserMapping::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Role] = "";
	attributes[Attributes::Server] = "";

	if(foreign_server)
	{
		if(def_type == SchemaParser::SqlCode)
			attributes[Attributes::Server] = foreign_server->getName(true);
		else
			attributes[Attributes::Server] = foreign_server->getSourceCode(def_type, true);
	}

	attributes[Attributes::Options] = getOptionsAttribute(def_type);

	return this->BaseObject::__getSourceCode(def_type);
}

QString UserMapping::getAlterCode(BaseObject *object)
{
	try
	{
		attributes[Attributes::AlterCmds] = BaseObject::getAlterCode(object);
		getAlteredAttributes(dynamic_cast<ForeignObject *>(object), attributes);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString UserMapping::getDropCode(bool)
{
	return BaseObject::getDropCode(false);
}

void UserMapping::updateDependencies()
{
	BaseObject::updateDependencies({ foreign_server });
}
