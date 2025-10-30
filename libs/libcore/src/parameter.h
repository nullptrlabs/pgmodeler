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
\brief Implements the operations to manipulate stored procedures (function) parameters.
*/

#ifndef PARAMETER_H
#define PARAMETER_H

#include "column.h"

class __libcore Parameter: public Column {
	private:
		/*! \brief This attributes indicates whether the parameter
		is in, out or both (IN, OUT, INOUT, VARIADIC) */
		bool is_in, is_out, is_variadic;

		void updateDependencies() override {}

	public:
		Parameter();
		Parameter(const Parameter &param);
		Parameter(const QString &name, PgSqlType type, bool in = false, bool out = false, bool variadic = false);
		~Parameter() override = default;

		void setType(PgSqlType type);
		void setIn(bool value);
		void setOut(bool value);
		void setVariadic(bool value);

		bool isIn();
		bool isOut();
		bool isVariadic();

		QString getModeString();

		//! \brief Returns the SQL / XML code definition for the parameter
		QString getSourceCode(SchemaParser::CodeType def_type) final;
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;

		void operator = (const Parameter &param);
};

#endif
