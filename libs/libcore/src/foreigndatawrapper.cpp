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

#include "foreigndatawrapper.h"

ForeignDataWrapper::ForeignDataWrapper()
{
	obj_type=ObjectType::ForeignDataWrapper;
	validator_func = handler_func = nullptr;
	attributes[Attributes::HandlerFunc] = "";
	attributes[Attributes::ValidatorFunc] = "";
	attributes[Attributes::Options] = "";
}

void ForeignDataWrapper::setHandlerFunction(Function *func)
{
	if(func)
	{
		if(func->getReturnType() != PgSqlType("fdw_handler"))
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidReturnType)
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgFunctionInvalidReturnType, PGM_FUNC, PGM_FILE, PGM_LINE);

		if(func->getParameterCount() != 0)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	handler_func = func;
}

void ForeignDataWrapper::setValidatorFunction(Function *func)
{
	if(func)
	{
		if(func->getParameterCount() != 2)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgFunctionInvalidParamCount, PGM_FUNC, PGM_FILE, PGM_LINE);

		if(!func->getParameter(0).getType().isExactTo(PgSqlType("text", 1)) ||
			 !func->getParameter(1).getType().isExactTo(PgSqlType("oid")))
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParameters)
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgFunctionInvalidParameters, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	validator_func = func;
}

Function *ForeignDataWrapper::getHandlerFunction()
{
	return handler_func;
}

Function *ForeignDataWrapper::getValidatorFunction()
{
	return validator_func;
}

QString ForeignDataWrapper::getSourceCode(SchemaParser::CodeType def_type)
{
	return getSourceCode(def_type, false);
}

QString ForeignDataWrapper::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	QStringList fmt_options;
	bool is_sql_def = (def_type == SchemaParser::SqlCode);

	if(handler_func)
	{
		handler_func->setAttribute(Attributes::RefType, Attributes::HandlerFunc);
		attributes[Attributes::HandlerFunc] = is_sql_def ? handler_func->getName(true) : handler_func->getSourceCode(def_type, true);
	}

	if(validator_func)
	{
		validator_func->setAttribute(Attributes::RefType, Attributes::ValidatorFunc);
		attributes[Attributes::ValidatorFunc] = is_sql_def ? validator_func->getName(true) : validator_func->getSourceCode(def_type, true);
	}

	attributes[Attributes::Options] = getOptionsAttribute(def_type);

	return this->BaseObject::getSourceCode(def_type, reduced_form);
}

QString ForeignDataWrapper::getAlterCode(BaseObject *object)
{
	try
	{
		ForeignDataWrapper *fdw=dynamic_cast<ForeignDataWrapper *>(object);
		attribs_map attribs;
		QStringList func_attribs = { Attributes::ValidatorFunc, Attributes::HandlerFunc };
		Function *this_funcs[2] = { this->getValidatorFunction(), this->getHandlerFunction() },
				*fdw_funcs[2] = { fdw->getValidatorFunction(), fdw->getHandlerFunction() },
				*this_func = nullptr, *fdw_func = nullptr;

		attributes[Attributes::AlterCmds] = BaseObject::getAlterCode(fdw);
		getAlteredAttributes(fdw, attribs);

		// Comparing FDW functions
		for(int i = 0; i < 2; i++)
		{
			this_func = this_funcs[i];
			fdw_func = fdw_funcs[i];

			if(!fdw_func && this_func)
				attribs[func_attribs[i]] = Attributes::Unset;
			else if(fdw_func &&
							(!this_func ||
							 (this_func && this_func->getSignature() != fdw_func->getSignature())))
				attribs[func_attribs[i]] = fdw_func->getName(true);
		}

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void ForeignDataWrapper::updateDependencies()
{
	BaseObject::updateDependencies({ validator_func, handler_func });
}
