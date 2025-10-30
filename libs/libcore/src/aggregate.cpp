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

#include "aggregate.h"

Aggregate::Aggregate()
{
	obj_type=ObjectType::Aggregate;
	functions[0]=functions[1]=nullptr;
	sort_operator=nullptr;
	attributes[Attributes::Types]="";
	attributes[Attributes::TransitionFunc]="";
	attributes[Attributes::StateType]="";
	attributes[Attributes::BaseType]="";
	attributes[Attributes::FinalFunc]="";
	attributes[Attributes::InitialCond]="";
	attributes[Attributes::SortOp]="";
}

void Aggregate::setFunction(FunctionId func_id, Function *func)
{
	//Case the function index is invalid raises an error
	if(func_id > TransitionFunc)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Checks if the function is valid, if not the case raises an error
	if(!isValidFunction(func_id, func))
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidConfiguration)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Aggregate)),
						ErrorCode::AsgFunctionInvalidConfiguration,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(functions[func_id]!=func);
	functions[func_id]=func;
}

bool Aggregate::isValidFunction(unsigned func_idx, Function *func)
{
	if(func)
	{
		if(func_idx==FinalFunc)
		{
			/* According to docs the final function to be valid must have 1 parameter which type is the
					same as the state_type attribute. BUT when importing system aggregates some functions has
					different parameter counts so in order to import them correctly we remove the restriction
					of one paramenter for this function */

			return (func->getParameterCount() > 0 &&
							func->getParameter(0).getType().canCastTo(state_type));
		}

		unsigned qtd, i;
		bool cond1,cond2=true;

		/* The transition function must have n+1 parameters, where n is the accepted data types list size.
		Also, the first parameter of the function and the return type must be the same as the 'state_type'
		attribute. Lastly, the other parameters must be the same as the accepted data types (the appearece order
		is important here).

		IMPORTANT: this is not documented by aggregate docs but when trying to import some catalog aggregates the
		majority of the functions used by them has polymorphic parameters so in order to accept that situation
		and recreate aggregates in the model we enable the usage of polymorphic functions here */

		cond1=(func->getReturnType().canCastTo(state_type)) &&
				((func->getParameterCount()==data_types.size() + 1) ||
					(func->getParameterCount() > 0 &&
						func->getParameter(func->getParameterCount()-1).getType().isPolymorphicType())) &&
				(func->getParameter(0).getType().canCastTo(state_type));

		qtd=func->getParameterCount();
		for(i=1 ; i < qtd && cond2; i++)
			cond2=(func->getParameter(i).getType().isPolymorphicType() ||
							((i-1) < data_types.size() && func->getParameter(i).getType().canCastTo(data_types[i-1])));

		return (cond1 && cond2);
	}

	return true;
}

void Aggregate::setStateType(PgSqlType st_type)
{
	st_type.reset();
	setCodeInvalidated(state_type != st_type);
	state_type = st_type;
}

void Aggregate::setInitialCondition(const QString &cond)
{
	setCodeInvalidated(initial_condition != cond);
	initial_condition=cond;
}

void Aggregate::setSortOperator(Operator *sort_op)
{
	if(sort_op)
	{
		Function *func=nullptr;

		/* Accordingly to the documentation, a sort operator only can be assigned
		 to the aggregate when:

		 1) The aggregate accepts only one data type
		 2) The function that defines the operator has the parameter types identical
				as the input data type of the aggregate  */
		func = sort_op->getFunction(Operator::FuncOperator);
		
		//Validating the condition 1
		if(data_types.size()!=1)
			throw Exception(ErrorCode::AsgInvalidOperatorArguments,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Validating the condition 2
		if(func->getParameter(0).getType() != data_types[0] ||
			(func->getParameterCount()==2 && func->getParameter(1).getType()!=data_types[0]))
			throw Exception(ErrorCode::AsgInvalidOperatorTypes,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	setCodeInvalidated(sort_operator != sort_op);
	this->sort_operator=sort_op;
}

void Aggregate::setTypesAttribute(SchemaParser::CodeType def_type)
{
	QString str_types;
	unsigned i, count;

	count=data_types.size();
	for(i=0; i < count; i++)
	{
		if(def_type==SchemaParser::SqlCode)
		{
			str_types+=data_types[i].getSourceCode(SchemaParser::SqlCode);
			if(i < (count-1)) str_types+=',';
		}
		else str_types+=data_types[i].getSourceCode(def_type);
	}

	/* Case none data type is specified for the aggregate creates
		an aggregate that accepts any possible data '*' e.g. function(*) */
	if(def_type == SchemaParser::SqlCode && str_types.isEmpty()) str_types='*';

	attributes[Attributes::Types]=str_types;
}

void Aggregate::addDataType(PgSqlType type)
{
	type.reset();
	data_types.push_back(type);
	setCodeInvalidated(true);
}

void Aggregate::removeDataType(unsigned type_idx)
{
	//Raises an exception if the type index is out of bound
	if(type_idx >= data_types.size())
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Removes the type at the specified position
	auto type_itr = data_types.begin() + type_idx;
	data_types.erase(type_itr);
	setCodeInvalidated(true);
}

void Aggregate::removeDataTypes()
{
	data_types.clear();
	setCodeInvalidated(true);
}

unsigned Aggregate::getDataTypeCount()
{
	return data_types.size();
}

Function *Aggregate::getFunction(FunctionId func_id)
{
	//Raises an exception if the function index is invalid
	if(func_id > TransitionFunc)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return functions[func_id];
}

PgSqlType Aggregate::getStateType()
{
	return state_type;
}

QString Aggregate::getInitialCondition()
{
	return initial_condition;
}

Operator *Aggregate::getSortOperator()
{
	return sort_operator;
}

PgSqlType Aggregate::getDataType(unsigned type_idx)
{
	//Raises an exception if the type index is out of bound
	if(type_idx >= data_types.size())
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return data_types[type_idx];
}

QString Aggregate::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setTypesAttribute(def_type);

	if(functions[TransitionFunc])
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::TransitionFunc]=functions[TransitionFunc]->getName(true);
		else
		{
			functions[TransitionFunc]->setAttribute(Attributes::RefType, Attributes::TransitionFunc);
			attributes[Attributes::TransitionFunc]=functions[TransitionFunc]->getSourceCode(def_type,true);
		}
	}

	if(functions[FinalFunc])
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::FinalFunc]=functions[FinalFunc]->getName(true);
		else
		{
			functions[FinalFunc]->setAttribute(Attributes::RefType,
												Attributes::FinalFunc);
			attributes[Attributes::FinalFunc]=functions[FinalFunc]->getSourceCode(def_type,true);
		}
	}

	if(sort_operator)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::SortOp]=sort_operator->getName(true);
		else
			attributes[Attributes::SortOp]=sort_operator->getSourceCode(def_type,true);
	}

	if(!initial_condition.isEmpty())
		attributes[Attributes::InitialCond]=initial_condition;

	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::StateType]=*(state_type);
	else
		attributes[Attributes::StateType]=state_type.getSourceCode(def_type,Attributes::StateType);

	return BaseObject::__getSourceCode(def_type);
}

QString Aggregate::getDropCode(bool cascade)
{
	setTypesAttribute(SchemaParser::SqlCode);
	return BaseObject::getDropCode(cascade);
}

QString Aggregate::getAlterCode(BaseObject *object)
{
	try
	{
		setTypesAttribute(SchemaParser::SqlCode);
		return BaseObject::getAlterCode(object);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString Aggregate::getSignature(bool format)
{
	QStringList types;

	if(data_types.empty())
		types.push_back("*");
	else
	{
		for(auto &tp : data_types)
			types.push_back(tp.getSourceCode(SchemaParser::SqlCode));
	}

	return BaseObject::getSignature(format) + QString("(%1)").arg(types.join(','));
}

void Aggregate::configureSearchAttributes()
{
	QStringList list;

	BaseObject::configureSearchAttributes();

	for(auto &type : data_types)
		list += *type;

	search_attribs[Attributes::Type] = list.join("; ");
}

void Aggregate::updateDependencies()
{
	std::vector<BaseObject *> dep_objs = {
		functions[FinalFunc], functions[TransitionFunc],
		sort_operator, state_type.getObject()
	};

	for(auto &type : data_types)
		dep_objs.push_back(type.getObject());

	BaseObject::updateDependencies(dep_objs);
}
