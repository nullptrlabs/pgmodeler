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

#include "basefunction.h"
#include "defaultlanguages.h"
#include "utilsns.h"

const QRegularExpression BaseFunction::ConfigParamPattern {
	QRegularExpression::anchoredPattern("([a-z]+)([a-z]|(_))*"),
	QRegularExpression::CaseInsensitiveOption
};

BaseFunction::BaseFunction()
{
	language=nullptr;
	obj_type=ObjectType::BaseObject;

	attributes[Attributes::Parameters]="";
	attributes[Attributes::Language]="";
	attributes[Attributes::SecurityType]="";
	attributes[Attributes::Definition]="";
	attributes[Attributes::Signature]="";
	attributes[Attributes::ReturnTable]="";
	attributes[Attributes::Library]="";
	attributes[Attributes::Symbol]="";
	attributes[Attributes::TransformTypes]="";
	attributes[Attributes::ConfigParams]="";
}

void BaseFunction::setName(const QString &name)
{
	BaseObject::setName(name);
	createSignature();
}

void BaseFunction::setSchema(BaseObject *schema)
{
	BaseObject::setSchema(schema);
	createSignature();
}

void BaseFunction::addParameter(Parameter param)
{
	std::vector<Parameter>::iterator itr,itr_end;
	bool found=false;

	itr=parameters.begin();
	itr_end=parameters.end();

	//Checks the duplicity of parameter names
	while(itr!=itr_end && !found)
	{
		/* Compares the parameters name storing in the 'found' flag
		 if already exists in the function */
		found=(itr->getName()==param.getName());
		itr++;
	}

	//If a duplicated parameter is found an error is raised
	if(found)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedParameterFunction)
						.arg(param.getName())
						.arg(this->signature),
						ErrorCode::AsgDuplicatedParameterFunction,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Inserts the parameter in the function
	parameters.push_back(param);
	createSignature();
}

void BaseFunction::setParametersAttribute(SchemaParser::CodeType def_type)
{
	QString str_param;
	QStringList fmt_params;

	for(auto &param : parameters)
		fmt_params.append(param.getSourceCode(def_type));

	str_param = fmt_params.join("");

	if(def_type==SchemaParser::SqlCode)
		str_param.remove(str_param.size()-2,2);

	attributes[Attributes::Parameters]=str_param;
}

void BaseFunction::setBasicFunctionAttributes(SchemaParser::CodeType def_type)
{
	try
	{
		attribs_map attribs;
		setParametersAttribute(def_type);

		if(language)
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::Language]=language->getName(false);
			else
				attributes[Attributes::Language]=language->getSourceCode(def_type, true);

			if(language->getName().toLower() == DefaultLanguages::C)
			{
				attributes[Attributes::Symbol]=symbol;
				attributes[Attributes::Library]=library;
			}
		}

		QStringList types;
		for(auto &type : transform_types)
		{
			types.append(QString("%1%2")
									 .arg(def_type == SchemaParser::SqlCode ? UtilsNs::DataSeparator : "")
									 .arg(~type));
		}

		if(def_type==SchemaParser::SqlCode)
			types.replaceInStrings(UtilsNs::DataSeparator, " FOR TYPE ");

		attributes[Attributes::TransformTypes] = types.join(',');

		for(auto &cfg_param : config_params)
		{
			attribs[Attributes::Name] = cfg_param.first;
			attribs[Attributes::Value] = cfg_param.second;
			attributes[Attributes::ConfigParams] += schparser.getSourceCode(Attributes::ConfigParam, attribs, def_type);
		}

		attributes[Attributes::SecurityType]=~security_type;
		attributes[Attributes::Definition]=func_source;
		attributes[Attributes::Signature]=signature;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void BaseFunction::configureSearchAttributes()
{
	QStringList param_types;

	BaseObject::configureSearchAttributes();

	for(auto &param : parameters)
		param_types += *param.getType();

	search_attribs[Attributes::Type] = param_types.join("; ");
}

void BaseFunction::setLibrary(const QString &library)
{
	if(language->getName().toLower() != DefaultLanguages::C)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgRefLibraryFuncLanguageNotC)
						.arg(this->getSignature()),
						ErrorCode::AsgRefLibraryFuncLanguageNotC,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->library != library);
	this->library=library;
}

void BaseFunction::setSymbol(const QString &symbol)
{
	if(language->getName().toLower() != DefaultLanguages::C)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgRefLibraryFuncLanguageNotC)
						.arg(this->getSignature()),
						ErrorCode::AsgRefLibraryFuncLanguageNotC,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->symbol != symbol);
	this->symbol=symbol;
}

void BaseFunction::setLanguage(BaseObject *lang)
{
	//Raises an error if the language is not allocated
	if(!lang)
		throw Exception(ErrorCode::AsgNotAllocatedLanguage,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the language object is invalid
	if(lang->getObjectType()!=ObjectType::Language)
		throw Exception(ErrorCode::AsgInvalidLanguageObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->language != lang);
	this->language=lang;
}

void BaseFunction::setSecurityType(SecurityType sec_type)
{
	setCodeInvalidated(security_type != sec_type);
	security_type=sec_type;
}

void BaseFunction::addTransformType(PgSqlType type)
{
	type.reset();

	if(!isTransformTypeExists(type))
	{
		transform_types.push_back(type);
		setCodeInvalidated(true);
	}
}

void BaseFunction::addTransformTypes(const QStringList &types)
{
	try
	{
		for(auto &type : types)
			addTransformType(PgSqlType(type));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void BaseFunction::setConfigurationParam(const QString &cfg_param, const QString &value)
{
	if(!ConfigParamPattern.match(cfg_param).hasMatch())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvConfigParameterName).arg(cfg_param).arg(signature),
										ErrorCode::InvConfigParameterName, PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(value.isEmpty())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::EmptyConfigParameterValue).arg(cfg_param).arg(signature),
										ErrorCode::EmptyConfigParameterValue, PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	config_params[cfg_param] = value;
	setCodeInvalidated(true);
}

void BaseFunction::removeConfigurationParams()
{
	config_params.clear();
	setCodeInvalidated(true);
}

attribs_map BaseFunction::getConfigurationParams()
{
	return config_params;
}

void BaseFunction::setFunctionSource(const QString &src_code)
{
	if(language && language->getName().toLower() == DefaultLanguages::C)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgSourceCodeFuncCLanguage)
						.arg(this->getSignature()),
						ErrorCode::AsgSourceCodeFuncCLanguage,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->func_source != src_code);
	this->func_source=src_code;
}

BaseObject *BaseFunction::getLanguage()
{
	return language;
}

unsigned BaseFunction::getParameterCount()
{
	return parameters.size();
}

SecurityType BaseFunction::getSecurityType()
{
	return security_type;
}

std::vector<PgSqlType> BaseFunction::getTransformTypes()
{
	return transform_types;
}

QString BaseFunction::getFunctionSource()
{
	return func_source;
}

Parameter BaseFunction::getParameter(unsigned param_idx)
{
	//Raises an error if the parameter index is out of bound
	if(param_idx>=parameters.size())
		throw Exception(ErrorCode::RefParameterInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return parameters[param_idx];
}

QString BaseFunction::getLibrary()
{
	return library;
}

QString BaseFunction::getSymbol()
{
	return symbol;
}

void BaseFunction::removeParameters()
{
	parameters.clear();
	createSignature();
}

void BaseFunction::removeTransformTypes()
{
	transform_types.clear();
	setCodeInvalidated(true);
}

bool BaseFunction::isTransformTypeExists(PgSqlType type)
{
	return std::find(transform_types.begin(), transform_types.end(), type) != transform_types.end();
}

void BaseFunction::removeParameter(const QString &name, PgSqlType type)
{
	auto itr = parameters.begin(),
			itr_end=parameters.end();

	while(itr != itr_end)
	{
		//Compares the iterator name and type with the passed name an type
		if(itr->getName()==name && itr->getType()==(~type))
		{
			parameters.erase(itr);
			break;
		}

		itr++;
	}

	//After remove the parameter is necessary updated the signature
	createSignature();
}

void BaseFunction::removeParameter(unsigned param_idx)
{
	//Raises an error if parameter index is out of bound
	if(param_idx>=parameters.size())
		throw Exception(ErrorCode::RefParameterInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	auto itr = parameters.begin() + param_idx;
	parameters.erase(itr);
	createSignature();
}

QString BaseFunction::getSignature(bool)
{
	return signature;
}

void BaseFunction::createSignature(bool format, bool prepend_schema)
{
	QString aux_str;
	QStringList fmt_params;

	for(auto &param : parameters)
	{
		//OUT parameters is not part of function's signature
		if(!param.isOut() || param.isVariadic() ||
				(param.isIn() && param.isOut()) ||
				(param.isIn() && !param.isOut()))
		{
			/* Removing the arg mode IN from parameter signature because this is de default for any kind of parameter
			 * So in order to avoid signature conflicts (mainly whe diff functions) we remove it.
			 * The keyword OUT is also removed for IN OUT parameters, since removing the IN parameter the OUT keyword will remain which
			 * forms an invalid signature. */
			aux_str = param.getSourceCode(SchemaParser::SqlCode, true).replace(QRegularExpression("^(IN)?( )*(OUT)?( )"),"").trimmed();
			aux_str.remove(',');
			fmt_params.append(aux_str);
			param.setCodeInvalidated(true);
		}
	}

	signature = this->getName(format, prepend_schema) + "(" + fmt_params.join(",") + ")";
	this->setCodeInvalidated(true);
}

void BaseFunction::updateDependencies(const std::vector<BaseObject *> &deps, const std::vector<BaseObject *> &old_deps)
{
	std::vector<BaseObject *> aux_deps = { language };

	aux_deps.insert(aux_deps.end(), deps.begin(), deps.end());

	for(auto &param : parameters)
		aux_deps.push_back(param.getType().getObject());

	for(auto &type : transform_types)
		aux_deps.push_back(type.getObject());

	BaseObject::updateDependencies(aux_deps, old_deps);
}

bool BaseFunction::isBaseFunction(ObjectType obj_tp)
{
	return obj_tp == ObjectType::Function || obj_tp == ObjectType::Procedure;
}

attribs_map BaseFunction::getAlterCodeAttributes(BaseFunction *func)
{
	attribs_map attribs,
			cfg_params, aux_attrs;

	try
	{
		attributes[Attributes::AlterCmds] = BaseObject::getAlterCode(func);

		if(this->security_type != func->security_type)
			attribs[Attributes::SecurityType] = ~func->security_type;

		cfg_params = func->getConfigurationParams();

		// Checking if we need to create/update the configuration parameters
		for(auto &cfg : cfg_params)
		{
			if(config_params.count(cfg.first) == 0 ||
				 (config_params.count(cfg.first) && config_params[cfg.first] != cfg.second))
			{
				aux_attrs[Attributes::Name] = cfg.first;
				aux_attrs[Attributes::Value] = cfg.second;
				attribs[Attributes::ConfigParams] += BaseObject::getAlterCode(Attributes::ConfigParam, aux_attrs, false, true);
			}
		}

		// Resetting configuration parameter individually
		for(auto &cfg : config_params)
		{
			if(cfg_params.count(cfg.first) == 0)
			{
				aux_attrs[Attributes::Name] = cfg.first;
				aux_attrs[Attributes::Value] = Attributes::Unset;
				attribs[Attributes::ConfigParams] += BaseObject::getAlterCode(Attributes::ConfigParam, aux_attrs, false, true);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	return attribs;
}
