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

#include "operator.h"

Operator::Operator()
{
	unsigned i;

	obj_type=ObjectType::Operator;

	for(i=FuncOperator; i <= FuncRestrict; i++)
		functions[i]=nullptr;

	for(i=Operator::OperCommutator; i <= OperNegator; i++)
		operators[i]=nullptr;

	hashes=merges=false;
	argument_types[LeftArg]=PgSqlType("\"any\"");
	argument_types[RightArg]=PgSqlType("\"any\"");

	attributes[Attributes::LeftType]="";
	attributes[Attributes::RightType]="";
	attributes[Attributes::CommutatorOp]="";
	attributes[Attributes::NegatorOp]="";
	attributes[Attributes::RestrictionFunc]="";
	attributes[Attributes::JoinFunc]="";
	attributes[Attributes::OperatorFunc]="";
	attributes[Attributes::Hashes]="";
	attributes[Attributes::Merges]="";
	attributes[Attributes::Signature]="";
	attributes[Attributes::RefType]="";
}

bool Operator::isValidName(const QString &name)
{
	//Warning: Do not alter the sequence of characters or the validation will fail
	QString valid_chars=QString("+-*/<>=~!@#%^&|'?");
	int pos, len;
	bool valid=true;

	//Checks if the size of the name is valid
	valid=(name.size() <= static_cast<int>(ObjectNameMaxLength));

	/* Checking if the name is valid according the conditions:
		 1) The name has only the chars defined in 'valid_chars' */
	len=name.size();
	for(pos=0; pos < len && valid; pos++)
		valid=!(valid_chars.indexOf(name[pos]) < 0);

	//2) The name doesn't has sequences like -- or /* that defines SQL comments
	if(valid) valid=(name.indexOf(QLatin1String("--")) < 0);
	if(valid) valid=(name.indexOf(QLatin1String("/*")) < 0);

	//3) Case the name end with - or + it may own one or more chars in the set ~!@#%^&|'?
	if(name[name.size()-1]=='-' || name[name.size()-1]=='+')
	{
		//The set ~!@#%^&|' is defined from position 7  in the valid_chars string
		len=valid_chars.size();
		for(pos=7; pos < len && valid; pos++)
			valid|=(name.indexOf(valid_chars[pos]) >= 0);
	}

	return valid;
}

void Operator::setName(const QString &name)
{
	if(name.isEmpty())
		throw Exception(ErrorCode::AsgEmptyNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	
	if(!isValidName(name))
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->obj_name=name;
}

void Operator::setFunction(Function *func, FunctionId func_id)
{
	//Raises an error if the function type is invalid
	if(func_id > FuncRestrict)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
	
	if(func_id==FuncOperator)
	{
		//Raises an error if the function is not allocated
		if(!func)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
											.arg(this->getName(true))
											.arg(BaseObject::getTypeName(ObjectType::Operator)),
											ErrorCode::AsgNotAllocatedFunction,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		/* Raises an error if the parameter count is invalid. To be used by the operator
		 the function must own 1 or 2 parameters */
		if(func->getParameterCount()==0 || func->getParameterCount() > 2)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
							.arg(this->getName())
							.arg(BaseObject::getTypeName(ObjectType::Operator)),
							ErrorCode::AsgFunctionInvalidParamCount,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		unsigned param_count=func->getParameterCount();
		QString any_type = "\"any\"";
		PgSqlType param_type1=PgSqlType(any_type),
				param_type2=PgSqlType(any_type);

		//Get the function parameter to make validations
		param_type1=func->getParameter(0).getType();
		
		if(param_count==2)
			 param_type2=func->getParameter(1).getType();

		//Validates the function parameters according to the operator arguments

		//ERROR 1: The function have parameters of the type 'any'
		if((param_type1==any_type || (param_count==2 && param_type2==any_type)) ||

				//ERROR 2: The function parameter count is 1 and the type of operator argument is not 'any'
				(param_count==1 && argument_types[0]!=any_type && argument_types[1]!=any_type) ||

				//ERROR 3: The function parameter count is 2 and the operator arguments is not 'any'
				(param_count==2 && ((argument_types[0]==any_type && argument_types[1]!=any_type) ||
									(argument_types[0]!=any_type && argument_types[1]==any_type))) ||

				/* ERROR 4:  The function parameter count is 2 and the argument types differs from
												parameters type */
				(param_count==2 &&
					((argument_types[0]==any_type || argument_types[1]==any_type) ||
					(argument_types[0]!=param_type1 || argument_types[1]!=param_type2))) ||

				//ERROR 5:  When the function has 1 parameter the type differ from the operator argument
				(param_count==1 &&
					((argument_types[0]!=any_type && argument_types[0]!=param_type1) ||
					(argument_types[1]!=any_type && argument_types[1]!=param_type1))))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParameters)
							.arg(this->getName())
							.arg(BaseObject::getTypeName(ObjectType::Operator)),
							ErrorCode::AsgFunctionInvalidParameters,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}

	setCodeInvalidated(functions[func_id] != func);
	functions[func_id]=func;
}

void Operator::setArgumentType(PgSqlType arg_type, ArgumentId arg_id)
{
	//Raises an error if the argument id is invalid
	if(arg_id > RightArg)
		throw Exception(ErrorCode::RefOperatorArgumentInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	arg_type.reset();
	setCodeInvalidated(argument_types[arg_id] != arg_type);
	argument_types[arg_id]=arg_type;
}

void Operator::setOperator(Operator *oper, OperatorId op_id)
{
	//Raises an error if the operator type is invalid
	if(op_id > OperNegator)
		throw Exception(ErrorCode::RefOperatorInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	/* Validating the Commutator OP: According to the PostgreSQL documentation
	 the commutator must have its right argument as the same type of left argument
	 from the commuted operator. That is, if the operator ++(typeA, typeB)
	 is being defined and its commutator operator is +*+ then the signature
	 of the latter should be +*+ (typeB, typeA). Raises an error when this condition
	 is not satisfied. */
	if(oper && op_id==OperCommutator && argument_types[LeftArg]!=oper->argument_types[RightArg])
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidCommutatorOperator)
										.arg(oper->getSignature(true))
										.arg(this->getSignature(true)),
										ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	/* Validating Negator OP: According to the PostgreSQL documentation the negator
	 operator must have its arguments as the same type of arguments from the
	 operator to be defined. That is, if the operator !!(typeA) is being
	 set and its negator is !*! then the signature of the latter should be !*! (typeA).
	 Raises an error when this condition is not satisfied. */
	if(oper && op_id==OperNegator &&
					(argument_types[LeftArg]!=oper->argument_types[LeftArg] &&
					 argument_types[RightArg]!=oper->argument_types[RightArg]))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidNegatorOperator)
										.arg(oper->getSignature(true))
										.arg(this->getSignature(true)),
										ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	setCodeInvalidated(operators[op_id] != oper);
	operators[op_id]=oper;
}

void Operator::setHashes(bool value)
{
	setCodeInvalidated(hashes != value);
	hashes=value;
}

void Operator::setMerges(bool value)
{
	setCodeInvalidated(merges != value);
	merges=value;
}

Function *Operator::getFunction(FunctionId func_id)
{
	//Raises an error if the function type is invalid
	if(func_id > FuncRestrict)
		throw Exception(ErrorCode::RefOperatorInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return functions[func_id];
}

PgSqlType Operator::getArgumentType(ArgumentId arg_id)
{
	//Raises an error if the argument id is invalid
	if(arg_id > RightArg)
		throw Exception(ErrorCode::RefOperatorArgumentInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
	return argument_types[arg_id];
}

Operator *Operator::getOperator(OperatorId op_id)
{
	//Raises an error if the operator type is invalid
	if(op_id > OperNegator)
		throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return operators[op_id];
}

bool Operator::isHashes()
{
	return hashes;
}

bool Operator::isMerges()
{
	return merges;
}

QString Operator::getSignature(bool format_name)
{
	QString signature;
	QStringList args;
	unsigned i;

	signature=this->getName(format_name);

	for(i=0; i < 2; i++)
	{
		if(argument_types[i]=="\"any\"")
			args.push_back("NONE");
		else
			args.push_back(argument_types[i].getTypeName(true));
	}

	signature+="(" + args.join(',') + ")";
	return signature;
}

QString Operator::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

QString Operator::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	unsigned i;
	QString type_attribs[]={Attributes::LeftType, Attributes::RightType},
			op_attribs[]={ Attributes::CommutatorOp,
						   Attributes::NegatorOp },

			func_attribs[]={Attributes::OperatorFunc,
							Attributes::JoinFunc,
							Attributes::RestrictionFunc};

	for(i=Operator::LeftArg; i <= Operator::RightArg; i++)
	{
		if(def_type==SchemaParser::SqlCode)
		{
			if(argument_types[i]!="\"any\"")
				attributes[type_attribs[i]]=~argument_types[i];
		}
		else
		{
			attributes[type_attribs[i]]=argument_types[i].
																	getSourceCode(SchemaParser::XmlCode,type_attribs[i]);
		}
	}

	for(i=Operator::OperCommutator; i <= Operator::OperNegator; i++)
	{
		if(operators[i])
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[op_attribs[i]]=operators[i]->getName(true);
			else
			{
				operators[i]->attributes[Attributes::RefType]=op_attribs[i];
				attributes[op_attribs[i]]=operators[i]->getSourceCode(def_type, true);
			}
		}
	}

	for(i=Operator::FuncOperator; i <= Operator::FuncRestrict; i++)
	{
		if(functions[i])
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[func_attribs[i]]=functions[i]->getName(true);
			else
			{
				functions[i]->setAttribute(Attributes::RefType, func_attribs[i]);
				attributes[func_attribs[i]]=functions[i]->getSourceCode(def_type, true);
			}
		}
	}

	attributes[Attributes::Hashes]=(hashes ? Attributes::True : "");
	attributes[Attributes::Merges]=(merges ? Attributes::True : "");
	attributes[Attributes::Signature]=getSignature();

	return BaseObject::getSourceCode(def_type, reduced_form);
}

void Operator::configureSearchAttributes()
{
	QStringList arg_types;

	BaseObject::configureSearchAttributes();

	arg_types += *argument_types[0];
	arg_types += *argument_types[1];
	search_attribs[Attributes::Type] = arg_types.join("; ");
}

void Operator::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &func : functions)
	deps.push_back(func);

	for(auto &type : argument_types)
	deps.push_back(type.getObject());

	for(auto &oper : operators)
	deps.push_back(oper);

	BaseObject::updateDependencies(deps);
}
