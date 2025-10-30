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
\class Domain
\brief Implements the operations to manipulate domains on the database.
\note <strong>Creation date:</strong> 07/04/2008
*/

#ifndef DOMAIN_H
#define DOMAIN_H

#include "baseobject.h"
#include "pgsqltypes/pgsqltype.h"

class __libcore Domain: public BaseObject{
	private:
		//! \brief Domain default data type value
		QString default_value;

		//! \brief Indicates that the domains accepts null values or not
		bool not_null;

		//! \brief Domain data type
		PgSqlType type;

		//! \brief Store the check constraint expressions (key:name value:expression)
		attribs_map chk_constrs;

	public:
		Domain();
		~Domain() override = default;

		void addCheckConstraint(const QString &name, const QString &expr);

		void removeCheckConstraints();

		attribs_map getCheckConstraints();

		//! \brief Sets the default value of the domain
		void setDefaultValue(const QString &default_val);

		//! \brief Defines whether the domain accepts null values or not
		void setNotNull(bool value);

		//! \brief Defines the domain data type
		void setType(PgSqlType type);

		/*! \brief Overloaded BaseObject name definition method.
		 Updates the reference of the domain as a PostgreSQL data type */
		void setName(const QString &name) override;

		/*! \brief Overloaded BaseObject schema definition method.
		 Updates the reference of the domain as a PostgreSQL data type */
		void setSchema(BaseObject *schema) override;

		//! \brief Methods to access domain's attributes
		QString getConstraintName();
		QString getExpression();
		QString getDefaultValue();
		bool isNotNull();
		PgSqlType getType();

		//! \brief Returns the SQL / XML code definition for the domain
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getAlterCode(BaseObject *object) final;

		void operator = (Domain &domain);

		void updateDependencies() override;
};

#endif
