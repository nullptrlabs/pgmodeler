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

#include "conversion.h"

Conversion::Conversion()
{
	obj_type=ObjectType::Conversion;
	conversion_func=nullptr;
	is_default=false;
	attributes[Attributes::Default]="";
	attributes[Attributes::SrcEncoding]="";
	attributes[Attributes::DstEncoding]="";
	attributes[Attributes::Function]="";
}

void Conversion::setEncoding(EncodingId encoding_id, EncodingType encoding_type)
{
	//Checks if the encoding index is valid
	if(encoding_id > DstEncoding)
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//If the passed enconding type is null an error is raised
	if((~encoding_type).isEmpty())
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNullTypeObject)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Conversion)),
						ErrorCode::AsgNullTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Assigns the encoding to the conversion in the specified index
	this->encodings[encoding_id]=encoding_type;
}

void Conversion::setConversionFunction(Function *conv_func)
{
	//Raises an error in case the passed conversion function is null
	if(!conv_func)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
						.arg(this->getName(true))
						.arg(BaseObject::getTypeName(ObjectType::Conversion)),
						ErrorCode::AsgNotAllocatedFunction, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	/* The conversion function must have 5 (or 6 in PG 14+) parameters if it's not the case
		raises an error. */
	if(conv_func->getParameterCount() < 5 ||
		 conv_func->getParameterCount() > 6)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
										.arg(this->getName(true))
										.arg(BaseObject::getTypeName(ObjectType::Conversion)),
										ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	/* Raises an error if the function parameters does not following the type order:
		interger, integer, cstring, internal, integer */
	if(conv_func->getParameter(0).getType()!="integer" ||
			conv_func->getParameter(1).getType()!="integer" ||
			conv_func->getParameter(2).getType()!="cstring" ||
			conv_func->getParameter(3).getType()!="internal" ||
			conv_func->getParameter(4).getType()!="integer" ||
			(conv_func->getParameterCount() == 6 &&
			 conv_func->getParameter(5).getType() != "boolean"))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParameters)
										.arg(this->getName(true))
										.arg(BaseObject::getTypeName(ObjectType::Conversion)),
										ErrorCode::AsgFunctionInvalidParameters, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	//Raises an error if the conversion function return type is not 'void' (or integer in PG14+)
	if(conv_func->getReturnType() != "void" &&
		 conv_func->getReturnType() != "integer")
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidReturnType)
										.arg(this->getName(true))
										.arg(BaseObject::getTypeName(ObjectType::Conversion)),
										ErrorCode::AsgFunctionInvalidReturnType, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	setCodeInvalidated(conversion_func != conv_func);
	this->conversion_func=conv_func;
}

void Conversion::setDefault(bool value)
{
	setCodeInvalidated(is_default != value);
	is_default=value;
}

EncodingType Conversion::getEncoding(EncodingId encoding_id)
{
	if(encoding_id > DstEncoding)
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return this->encodings[encoding_id];
}

Function *Conversion::getConversionFunction()
{
	return conversion_func;
}

bool Conversion::isDefault()
{
	return is_default;
}

QString Conversion::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attributes[Attributes::Default]=(is_default ? Attributes::True : "");
	attributes[Attributes::SrcEncoding]=(~encodings[SrcEncoding]);
	attributes[Attributes::DstEncoding]=(~encodings[DstEncoding]);

	if(conversion_func)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Function]=conversion_func->getName(true);
		else
			attributes[Attributes::Function]=conversion_func->getSourceCode(def_type, true);
	}

	return BaseObject::__getSourceCode(def_type);
}

void Conversion::updateDependencies()
{
	BaseObject::updateDependencies({ conversion_func });
}
