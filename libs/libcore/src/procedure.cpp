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

#include "procedure.h"

Procedure::Procedure()
{
	obj_type = ObjectType::Procedure;
}

QString Procedure::getSourceCode(SchemaParser::CodeType def_type, bool)
{
	QString code_def = getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setBasicFunctionAttributes(def_type);

	return BaseObject::getSourceCode(def_type, false);
}

QString Procedure::getSourceCode(SchemaParser::CodeType def_type)
{
	return getSourceCode(def_type, false);
}

QString Procedure::getAlterCode(BaseObject *object)
{
	Procedure *proc = dynamic_cast<Procedure *>(object);

	if(!proc)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs;
		attribs = BaseFunction::getAlterCodeAttributes(proc);

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Procedure::updateDependencies()
{
	BaseFunction::updateDependencies({});
}
