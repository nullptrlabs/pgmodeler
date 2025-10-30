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

#include "foreignserver.h"

ForeignServer::ForeignServer()
{
	obj_type = ObjectType::ForeignServer;
	fdata_wrapper = nullptr;

	attributes[Attributes::Options] = "";
	attributes[Attributes::Version] = "";
	attributes[Attributes::Type] = "";
	attributes[Attributes::Object] = "";
}

void ForeignServer::setType(const QString &type)
{
	this->type = type;
}

void ForeignServer::setVersion(const QString &version)
{
	this->version = version;
}

void ForeignServer::setForeignDataWrapper(ForeignDataWrapper *fdw)
{
	fdata_wrapper = fdw;
}

QString ForeignServer::getType()
{
	return type;
}

QString ForeignServer::getVersion()
{
	return version;
}

ForeignDataWrapper *ForeignServer::getForeignDataWrapper()
{
	return fdata_wrapper;
}

QString ForeignServer::getSourceCode(SchemaParser::CodeType def_type)
{
	return getSourceCode(def_type, false);
}

QString ForeignServer::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Version] = version;
	attributes[Attributes::Type] = type;
	attributes[Attributes::Fdw] = "";

	if(fdata_wrapper)
	{
		if(def_type == SchemaParser::SqlCode)
			attributes[Attributes::Fdw] = fdata_wrapper->getName(true);
		else
			attributes[Attributes::Fdw] = fdata_wrapper->getSourceCode(def_type, true);
	}

	attributes[Attributes::Options] = getOptionsAttribute(def_type);

	return this->BaseObject::getSourceCode(def_type, reduced_form);
}

QString ForeignServer::getAlterCode(BaseObject *object)
{
	try
	{
		ForeignServer *server=dynamic_cast<ForeignServer *>(object);
		attribs_map attribs;

		attributes[Attributes::AlterCmds] = BaseObject::getAlterCode(server);
		getAlteredAttributes(server, attribs);

		if(this->version != server->version)
			attribs[Attributes::Version] = server->version;

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void ForeignServer::updateDependencies()
{
	BaseObject::updateDependencies({ fdata_wrapper });
}
