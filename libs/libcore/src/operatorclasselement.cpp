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

#include "operatorclasselement.h"

OperatorClassElement::OperatorClassElement()
{
	element_type=OperatorElem;
	function=nullptr;
	_operator=nullptr;
	strategy_number=0;
	op_family=nullptr;
}

void OperatorClassElement::setFunction(Function *func, unsigned stg_number)
{
	//Raises an exception case the function is not allocated
	if(!func)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error case the stratagy number is zero (invalid)
	if(stg_number == 0)
		throw Exception(ErrorCode::AsgInvalidSupportStrategyNumber,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Clear the attributes not related to the FUNCTION element type
	this->_operator=nullptr;
	this->op_family=nullptr;

	//Configure the attributes that belongs to the element type
	this->function=func;
	this->strategy_number=stg_number;
	this->element_type=FunctionElem;
}

void OperatorClassElement::setOperator(Operator *oper, unsigned stg_number)
{
	//Raises an error in case the operator is not allocated
	if(!oper)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error case the stratagy number is zero (invalid)
	if(stg_number==0)
		throw Exception(ErrorCode::AsgInvalidSupportStrategyNumber,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Clear the attributes not related to the OPERATOR element type
	this->function=nullptr;

	//Configure the attributes that belongs to the element type
	this->_operator=oper;
	this->strategy_number=stg_number;
	this->element_type=OperatorElem;
}

void OperatorClassElement::setOperatorFamily(OperatorFamily *op_family)
{
	if(this->element_type==OperatorElem)
	{
		if(op_family && op_family->getIndexingType()!=IndexingType::Btree)
			throw Exception(ErrorCode::AsgInvalidOpFamilyOpClassElem,PGM_FUNC,PGM_FILE,PGM_LINE);

		this->op_family=op_family;
	}
}

void OperatorClassElement::setStorage(PgSqlType storage)
{
	//Clear the attributes not related to the STORAGE element type
	this->function=nullptr;
	this->_operator=nullptr;
	this->op_family=nullptr;
	this->strategy_number=0;

	//Configure the attributes that belongs to the element type
	storage.reset();
	this->storage=storage;
	this->element_type=StorageElem;
}

OperatorClassElement::ElementType OperatorClassElement::getElementType()
{
	return element_type;
}

Function *OperatorClassElement::getFunction()
{
	return function;
}

Operator *OperatorClassElement::getOperator()
{
	return _operator;
}

OperatorFamily *OperatorClassElement::getOperatorFamily()
{
	return op_family;
}

PgSqlType OperatorClassElement::getStorage()
{
	return storage;
}

unsigned OperatorClassElement::getStrategyNumber()
{
	return strategy_number;
}

QString OperatorClassElement::getSourceCode(SchemaParser::CodeType def_type)
{
	SchemaParser schparser;
	attribs_map attributes;

	attributes[Attributes::Type]="";
	attributes[Attributes::StrategyNum]="";
	attributes[Attributes::Signature]="";
	attributes[Attributes::Function]="";
	attributes[Attributes::Operator]="";
	attributes[Attributes::Storage]="";
	attributes[Attributes::OpFamily]="";
	attributes[Attributes::Definition]="";

	if(element_type==FunctionElem && function && strategy_number > 0)
	{
		//FUNCTION support_number [ ( op_type [ , op_type ] ) ] funcname ( argument_type [, ...] )
		attributes[Attributes::Function]=Attributes::True;
		attributes[Attributes::StrategyNum]=QString("%1").arg(strategy_number);

		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Signature]=function->getSignature();
		else
			attributes[Attributes::Definition]=function->getSourceCode(def_type,true);
	}
	else if(element_type==OperatorElem && _operator && strategy_number > 0)
	{
		//OPERATOR strategy_number operator_name [ ( op_type, op_type ) ] [ FOR SEARCH | FOR ORDER BY sort_family_name ]
		attributes[Attributes::Operator]=Attributes::True;
		attributes[Attributes::StrategyNum]=QString("%1").arg(strategy_number);

		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Signature]=_operator->getSignature();
		else
			attributes[Attributes::Definition]=_operator->getSourceCode(def_type,true);

		if(op_family)
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::OpFamily]=op_family->getName(true);
			else
				attributes[Attributes::Definition]+=op_family->getSourceCode(def_type,true);
		}
	}
	else if(element_type==StorageElem && storage!=PgSqlType::Null)
	{
		//STORAGE storage_type
		attributes[Attributes::Storage]=Attributes::True;

		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Type]=(*storage);
		else
			attributes[Attributes::Definition]=storage.getSourceCode(def_type);
	}

	return schparser.getSourceCode(Attributes::Element,attributes, def_type);
}

bool OperatorClassElement::operator == (OperatorClassElement &elem)
{
	return (this->element_type == elem.element_type &&
					this->storage == elem.storage &&
					this->function == elem.function &&
					this->_operator == elem._operator &&
					this->strategy_number == elem.strategy_number &&
					this->op_family == elem.op_family);
}

std::vector<BaseObject *> OperatorClassElement::getDependencies()
{
	std::vector<BaseObject *> deps;

	if(function)
		deps.push_back(function);

	if(_operator)
		deps.push_back(_operator);

	if(op_family)
		deps.push_back(op_family);

	if(storage.getObject())
		deps.push_back(storage.getObject());

	return deps;
}
