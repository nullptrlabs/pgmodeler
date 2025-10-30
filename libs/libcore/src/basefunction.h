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

/**
\ingroup libcore
\brief Implements the basic operations to manipulate functions/stored procedures on the database.
*/

#ifndef BASE_FUNCTION_H
#define BASE_FUNCTION_H

#include "parameter.h"
#include "pgsqltypes/securitytype.h"

class __libcore BaseFunction: public BaseObject {
	protected:	
		//! \brief Function unique signature
		QString signature;

		//! \brief Shared library (commonly developed in C language) that stores the function routine
		QString library;

		//! \brief Symbol that identify the function on the shared library
		QString symbol;

		//! \brief Function source code (except for C language defined functions)
		QString func_source;

		//! \brief Language that is used to construct the language
		BaseObject *language;

		//! \brief Function parameters
		std::vector<Parameter> parameters;

		//! \brief Function security type. It can be SECURITY [INVOKER | DEFINER]
		SecurityType security_type;

		//! \brief Transforms types that are applied to this function
		std::vector<PgSqlType> transform_types;

		//! \brief The user-defined configuration parameters of the function
		attribs_map config_params;

		//! \brief Formats the function parameter attribute to be used by the SchemaParser
		void setParametersAttribute(SchemaParser::CodeType def_type);

		//! \brief Formats the basic function's attributes to used in the code generation
		void setBasicFunctionAttributes(SchemaParser::CodeType def_type);

		//! \brief Returns a code snippet that is used to configure atler commands in order to change configuration parameters
		attribs_map getAlterCodeAttributes(BaseFunction *func);

		void configureSearchAttributes() override;

	public:
		static const QRegularExpression ConfigParamPattern;

		BaseFunction();
		~BaseFunction() override = default;

		//! \brief Sets the function name updating its signature
		void setName(const QString &name) override;

		//! \brief Sets the function schema updating its signature
		void setSchema(BaseObject *schema) override;

		//! \brief Adds a parameter to the function
		virtual void addParameter(Parameter param);

		//! \brief Defines the function source code (if its not use the C language)
		void setFunctionSource(const QString &src_code);

		//! \brief Defines the shared library that stores the function routine
		void setLibrary(const QString &library);

		//! \brief Defines the symbol that identifies the function on shared library
		void setSymbol(const QString &symbol);

		//! \brief Defines the language used to construct the function
		void setLanguage(BaseObject *language);

		//! \brief Defines the security type of the function
		void setSecurityType(SecurityType sec_type);

		//! \brief Add a transform type to the function
		void addTransformType(PgSqlType type);

		/*! \brief Add a set of transform types to the function. The type names in the list
		 * must not contain any length, dimension or any other specialization of a type,
		 * just the correct type name */
		void addTransformTypes(const QStringList &types);

		/*! \brief Defines a value to the provided configuration parameter. The configuration parameter name is
		 * validated according to PostgreSQL naming convention and the method will raise an exception in case of
		 * a invalid name is being used. Empty values are considered setting the default value for the provided
		 * configuration parameter in the SQL code (e.g.: SET config_param = DEFAULT). */
		void setConfigurationParam(const QString &cfg_param, const QString &value);

		//! \brief Clears all the configuration parameters
		void removeConfigurationParams();

		//! \brief Returns the current configuration paramenters
		attribs_map getConfigurationParams();

		//! \brief Returns the function's source code
		QString getFunctionSource();

		//! \brief Returns the shared library that stores the function definition
		QString getLibrary();

		//! \brief Returns the symbol that identifies the function on the shared library
		QString getSymbol();

		//! \brief Returns the language used to construct the function
		BaseObject *getLanguage();

		//! \brief Returns the function parameter count
		unsigned getParameterCount();

		//! \brief Returns a parameter using its index
		Parameter getParameter(unsigned param_idx);

		//! \brief Returns the security type used by the function
		SecurityType getSecurityType();

		//! \brief Returns the transform types of the function
		std::vector<PgSqlType> getTransformTypes();

		//! \brief Removes a parameter using its name and type
		void removeParameter(const QString &name, PgSqlType type);

		//! \brief Removes a parameter using its index
		void removeParameter(unsigned param_idx);

		//! \brief Removes all the parameters from the function
		void removeParameters();

		//! \brief Removes all the transform types from the function
		void removeTransformTypes();

		//! \brief Returns true when a transform type is already in use by the function
		bool isTransformTypeExists(PgSqlType type);

		//! \brief Returns the complete function signature
		QString getSignature(bool = true) override;

		/*! \brief Generates the function's signature. The 'format' parameter is used
		 to adequately format the function and parameters names. By default
		 this formating is always done. */
		void createSignature(bool format = true, bool prepend_schema = true);

		void updateDependencies() override = 0 ;

		void updateDependencies(const std::vector<BaseObject *> &deps, const std::vector<BaseObject *> &old_deps = {});

		static bool isBaseFunction(ObjectType obj_tp);

		QString getSourceCode(SchemaParser::CodeType def_type, bool) override = 0;
		QString getSourceCode(SchemaParser::CodeType def_type) override = 0;
		QString getAlterCode(BaseObject *object) override = 0;
};

#endif
