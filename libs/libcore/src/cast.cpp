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

#include "cast.h"

Cast::Cast()
{
	obj_type=ObjectType::Cast;
	cast_function=nullptr;
	cast_type=Explicit;
	is_in_out=false;
	attributes[Attributes::SourceType]="";
	attributes[Attributes::DestType]="";
	attributes[Attributes::CastType]="";
	attributes[Attributes::IoCast]="";
	attributes[Attributes::Function]="";
}

void Cast::setName(const QString &)
{
	//Configures the cast name (in form of signature: cast(src_type, dst_type) )
	this->obj_name=QString("cast(%1,%2)").arg(~types[SrcType]).arg(~types[DstType]);
}

void Cast::setDataType(DataTypeId type_idx, PgSqlType type)
{
	type.reset();

	if(type_idx > DstType)
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the passed data type is null
	if((*type).isEmpty())
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNullTypeObject)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Cast)),
						ErrorCode::AsgNullTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->types[type_idx] != type);
	this->types[type_idx]=type;

	setName("");
}

void Cast::setCastType(CastType cast_type)
{
	//Raises an error if the user tries to assign an invalid cast type
	if(cast_type > Implicit)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->cast_type != cast_type);
	this->cast_type = cast_type;
}

void Cast::setInOut(bool value)
{
	setCodeInvalidated(is_in_out != value);
	is_in_out=value;
}

void Cast::setCastFunction(Function *cast_func)
{
	unsigned param_count;
	bool error=false;
	PgSqlType ret_type;

	if(!cast_func)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Cast)),
						ErrorCode::AsgNotAllocatedFunction,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Retrieve the cast function parameter count for specific validations
	param_count=cast_func->getParameterCount();

	//Raises an error if the function don't have at least 1 parameter or a maximum of 3
	if(param_count == 0 || param_count > 3)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::Cast)),
										ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	/* Error condition 1: Check if the first function parameter data type differs
		 from cast source data type */
	error=(cast_func->getParameter(0).getType()!=this->types[SrcType] &&
				!cast_func->getParameter(0).getType().canCastTo(this->types[SrcType]));

	/* Error condition 2: Check if the second function parameter data type
		 is different from 'integer' */
	if(!error && param_count>=2)
		error=(cast_func->getParameter(1).getType()!="integer");

	/* Error condition 3: Check if the third function parameter data type is
		 different from 'boolean' */
	if(!error && param_count==3)
		error=(cast_func->getParameter(2).getType()!="boolean");

	//In case some error condition is reached raises an error
	if(error)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParameters)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::Cast)),
										ErrorCode::AsgFunctionInvalidParameters, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
	
	/* Raises an error if the return type of the function differs from the destination data type.
	   If the types can be casted between them no error is returned */
	ret_type=cast_func->getReturnType();

	if(ret_type != this->types[DstType] && !ret_type.canCastTo(this->types[DstType]))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidReturnType)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::Cast)),
										ErrorCode::AsgFunctionInvalidReturnType, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	setCodeInvalidated(cast_function != cast_func);
	this->cast_function=cast_func;
}

PgSqlType Cast::getDataType(DataTypeId type_idx)
{
	if(type_idx > DstType)
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return this->types[type_idx];
}

bool Cast::isInOut()
{
	return is_in_out;
}

Function *Cast::getCastFunction()
{
	return cast_function;
}

Cast::CastType Cast::getCastType()
{
	return cast_type;
}

QString Cast::getDropCode(bool cascade)
{
	attributes[Attributes::Signature].replace(",", " AS ");
	return BaseObject::getDropCode(cascade);
}

QString Cast::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	if(def_type==SchemaParser::SqlCode)
	{
		attributes[Attributes::SourceType]=(*types[SrcType]);
		attributes[Attributes::DestType]=(*types[DstType]);
	}
	else
	{
		attributes[Attributes::SourceType]=types[SrcType].getSourceCode(def_type);
		attributes[Attributes::DestType]=types[DstType].getSourceCode(def_type);
	}

	if(!is_in_out && cast_function)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Function]=cast_function->getSignature();
		else
			attributes[Attributes::Function]=cast_function->getSourceCode(def_type, true);
	}
	else
		attributes[Attributes::IoCast]=(is_in_out ? Attributes::True : "");

	if(cast_type==Assignment)
		attributes[Attributes::CastType]=Attributes::Assignment;
	else if(cast_type==Implicit)
		attributes[Attributes::CastType]=Attributes::Implicit;
	else
		attributes[Attributes::CastType]="";

	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::CastType]=attributes[Attributes::CastType].toUpper();

	return BaseObject::__getSourceCode(def_type);
}

QString Cast::getSignature(bool)
{
	attributes[Attributes::Signature]=this->getName().remove("cast");
	return BaseObject::getSignature(false);
}

void Cast::configureSearchAttributes()
{
	QStringList arg_types;

	BaseObject::configureSearchAttributes();

	arg_types += *types[SrcType];
	arg_types += *types[DstType];
	search_attribs[Attributes::Type] = arg_types.join("; ");
}

void Cast::updateDependencies()
{
	BaseObject::updateDependencies({ cast_function,
																	types[SrcType].getObject(),
																	types[DstType].getObject() });
}
