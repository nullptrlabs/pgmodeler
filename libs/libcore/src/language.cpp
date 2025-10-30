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

#include "language.h"
#include "defaultlanguages.h"

Language::Language()
{
	obj_type=ObjectType::Language;
	is_trusted=false;

	for(unsigned i=ValidatorFunc; i <= InlineFunc; i++)
		functions[i]=nullptr;

	attributes[Attributes::Trusted]="";
	attributes[Attributes::HandlerFunc]="";
	attributes[Attributes::ValidatorFunc]="";
	attributes[Attributes::InlineFunc]="";
}

void Language::setName(const QString &name)
{
	//Raises an error if the user try to set an system reserved language name (C, SQL)
	if(name.toLower() == DefaultLanguages::C || name.toLower() == DefaultLanguages::Sql)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgReservedName)
						.arg(this->getName())
						.arg(BaseObject::getTypeName(ObjectType::Language)),
						ErrorCode::AsgReservedName,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObject::setName(name);
}

void Language::setTrusted(bool value)
{
	setCodeInvalidated(is_trusted != value);
	is_trusted=value;
}

void Language::setFunction(Function *func, FunctionId func_id)
{
	if(!func ||
			(func &&
			 /* The handler function must be written in C and have
								  'language_handler' as return type */
			 ((func_id==HandlerFunc &&
				 func->getReturnType()=="language_handler" &&
			   func->getParameterCount()==0 &&
				 func->getLanguage()->getName().toLower() == DefaultLanguages::C) ||
			  /* The validator function must be written in C and return 'void' also
									   must have only one parameter of the type 'oid' */
				(func_id==ValidatorFunc &&
				 func->getReturnType()=="void" &&
			   func->getParameterCount()==1 &&
				 func->getParameter(0).getType() == "oid" &&
				 func->getLanguage()->getName().toLower() == DefaultLanguages::C) ||
			  /* The inline function must be written in C and return 'void' also
									   must have only one parameter of the type 'internal' */
				(func_id==InlineFunc &&
				 func->getReturnType()=="void" &&
			   func->getParameterCount()==1 &&
				 func->getParameter(0).getType() == "internal" &&
				 func->getLanguage()->getName().toLower() == DefaultLanguages::C) )))
	{
		setCodeInvalidated(functions[func_id] != func);
		this->functions[func_id]=func;
	}
	//Raises an error in case the function return type doesn't matches the required by each rule
	else if((func_id==HandlerFunc && func->getReturnType()!="language_handler") ||
			((func_id==ValidatorFunc || func_id==InlineFunc) && func->getReturnType()!="void"))
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidReturnType)
						.arg(this->getName(true))
						.arg(BaseObject::getTypeName(ObjectType::Language)),
						ErrorCode::AsgFunctionInvalidReturnType,PGM_FUNC,PGM_FILE,PGM_LINE);
	else
		//Raises an error in case the function has invalid parameters (count and types)
		throw Exception(ErrorCode::AsgFunctionInvalidParameters,PGM_FUNC,PGM_FILE,PGM_LINE);
}

Function * Language::getFunction(FunctionId func_id)
{
	if(func_id > InlineFunc)
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return functions[func_id];
}

bool Language::isTrusted()
{
	return is_trusted;
}

QString Language::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

void Language::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &func : functions)
		deps.push_back(func);

	BaseObject::updateDependencies(deps);
}

QString Language::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	unsigned i;
	QString attribs_func[3]={Attributes::ValidatorFunc,
							 Attributes::HandlerFunc,
							 Attributes::InlineFunc};

	attributes[Attributes::Trusted]=(is_trusted ? Attributes::True : "");

	if(!reduced_form && def_type==SchemaParser::XmlCode)
		reduced_form=(!functions[ValidatorFunc] && !functions[HandlerFunc] && !functions[InlineFunc] && !this->getOwner());

	for(i=0; i < 3; i++)
	{
		if(functions[i])
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[attribs_func[i]]=functions[i]->getName(true);
			else
			{
				functions[i]->setAttribute(Attributes::RefType, attribs_func[i]);
				attributes[attribs_func[i]]=functions[i]->getSourceCode(def_type, true);
			}
		}
	}

	return BaseObject::getSourceCode(def_type, reduced_form);
}

