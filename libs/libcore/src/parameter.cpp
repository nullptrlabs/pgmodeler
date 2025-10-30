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

#include "parameter.h"

Parameter::Parameter()
{
	obj_type=ObjectType::Parameter;
	is_in=is_out=is_variadic=false;
}

Parameter::Parameter(const Parameter &param) : Parameter()
{
	setName(param.obj_name);
	setType(param.type);
	setIn(param.is_in);
	setOut(param.is_out);
	setVariadic(param.is_variadic);
	setDefaultValue(param.default_value);
}

Parameter::Parameter(const QString &name, PgSqlType type, bool in, bool out, bool variadic) : Parameter()
{
	setName(name);
	setType(type);
	setIn(in);
	setOut(out);
	setVariadic(variadic);
}

void Parameter::setType(PgSqlType type)
{
	if(!type.isArrayType() && !type.isPolymorphicType() && is_variadic)
		throw Exception(ErrorCode::InvUsageVariadicParamMode ,PGM_FUNC,PGM_FILE,PGM_LINE);

	type.reset();
	setCodeInvalidated(this->type != type);
	this->type=type;
}

void Parameter::setIn(bool value)
{
	setCodeInvalidated(is_in != value);
	is_in=value;
	is_variadic=false;
}

void Parameter::setOut(bool value)
{
	setCodeInvalidated(is_out != value);
	is_out=value;
	is_variadic=false;
}

void Parameter::setVariadic(bool value)
{
	if(value && !type.isArrayType() && !type.isPolymorphicType())
		throw Exception(ErrorCode::InvUsageVariadicParamMode ,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(is_variadic != value);
	is_variadic=value;
	if(value)	is_in=is_out=false;
}

bool Parameter::isIn()
{
	return is_in;
}

bool Parameter::isOut()
{
	return is_out;
}

bool Parameter::isVariadic()
{
	return is_variadic;
}

QString Parameter::getModeString()
{
	QString mode;

	if(is_variadic)
		mode = "VARIADIC";
	else
	{
		if(is_in) mode = "IN";
		if(is_out) mode += "OUT";
	}

	return mode;
}

void Parameter::operator = (const Parameter &param)
{
	this->obj_name=param.obj_name;
	this->type=param.type;
	this->default_value=param.default_value;
	this->is_in=param.is_in;
	this->is_out=param.is_out;
	this->is_variadic=param.is_variadic;
	setCodeInvalidated(true);
}

QString Parameter::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	return this->getSourceCode(def_type, false);
}

QString Parameter::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::Name]=BaseObject::formatName(obj_name);
	else
		attributes[Attributes::Name]=obj_name;

	attributes[Attributes::ParamIn]=(is_in ? Attributes::True : "");
	attributes[Attributes::ParamOut]=(is_out ? Attributes::True : "");
	attributes[Attributes::ParamVariadic]=(is_variadic ? Attributes::True : "");
	attributes[Attributes::DefaultValue]=default_value;
	attributes[Attributes::Type]=type.getSourceCode(def_type);

	return BaseObject::getSourceCode(def_type, reduced_form);
}
