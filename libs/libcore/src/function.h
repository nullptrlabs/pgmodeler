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
\brief Implements the operations to manipulate stored procedures (function) on the database.
\note <strong>Creation date:</strong> 28/09/2006
*/

#ifndef FUNCTION_H
#define FUNCTION_H

#include "basefunction.h"
#include "pgsqltypes/behaviortype.h"
#include "pgsqltypes/functiontype.h"
#include "pgsqltypes/paralleltype.h"

class __libcore Function: public BaseFunction {
	private:
		//! \brief Indicates whether the function returns a set of data (RETURNS SET OF)
		bool returns_setof,

		//! \brief Indicate whether the object is a windows function or not
		is_wnd_function,

		//! \brief Indicate whether the object is leak proof or not
		is_leakproof;

		//! \brief Type of execution behavior applied to the function
		BehaviorType behavior_type;

		//! \brief Function type. It can be VOLATILE, IMMUTABLE, STABLE
		FunctionType function_type;

		//! \brief Function return type
		PgSqlType return_type;

		//! \brief The parallelism type used by the function
		ParallelType parallel_type;

		/*! \brief Stores the table columns returned by the clause RETURNS TABLE. This clause instead
		 of return a specific element returns a whole table. This structure is available
		 only on PostgreSQL 8.4 and later */
		std::vector<Parameter> ret_table_columns;

		//! \brief Execution cost for the function
		unsigned execution_cost;

		//! \brief Estimated returned row (tuple) amount
		unsigned row_amount;

		//! \brief Formats the function return type to be used by the SchemaParser
		void setTableReturnTypeAttribute(SchemaParser::CodeType def_type);

	protected:
		void configureSearchAttributes() override;

	public:
		Function();
		~Function() override = default;

		//! \brief Adds a column to the function returned table
		void addReturnedTableColumn(const QString &name, PgSqlType type);

		//! \brief Defines the execution cost for the function
		void setExecutionCost(unsigned exec_cost);

		//! \brief Defines the estimated amount of rows (tuples) returned by the function
		void setRowAmount(unsigned row_amount);

		//! \brief Defines whether returns a set of values or not
		void setReturnSetOf(bool value);

		//! \brief Defines whether the function is a windows function or not
		void setWindowFunction(bool value);

		//! \brief Defines whether the function is leak proof or not
		void setLeakProof(bool value);

		//! \brief Defines the function's execution behavior
		void setBehaviorType(BehaviorType behav_type);

		//! \brief Defines the function type (VOLATILE, IMMUTABLE, STABLE)
		void setFunctionType(FunctionType func_type);

		//! \brief Defines the function return type
		void setReturnType(PgSqlType type);

		//! \brief Defines the function's parallelism type
		void setParalleType(ParallelType type);

		//! \brief Returns the returned table column count
		unsigned getReturnedTableColumnCount();

		//! \brief Returns a column from returned table using its index
		Parameter getReturnedTableColumn(unsigned column_idx);

		//! \brief Indicates whether the function returns setof or not
		bool isReturnSetOf();

		//! \brief Indicates whether the fucntion returns a table or not
		bool isReturnTable();

		//! \brief Indicates whether the function is window or not
		bool isWindowFunction();

		//! \brief Indicates whether the function is leak proof or not
		bool isLeakProof();

		//! \brief Returns the function execution behavior
		BehaviorType getBehaviorType();

		//! \brief Returns the function type
		FunctionType getFunctionType();

		//! \brief Returns the function returned type
		PgSqlType getReturnType();

		//! \brief Returns the function's parallelism type
		ParallelType getParallelType();

		//! \brief Returns the function's execution cost
		unsigned getExecutionCost();

		//! \brief Returns the estimated returned row amount
		unsigned getRowAmount();

		//! \brief Removes a column from returned table using its index
		void removeReturnedTableColumn(unsigned column_idx);

		//! \brief Removes all the columns from returned table column
		void removeReturnedTableColumns();

		//! \brief Returns the SQL / XML code definition for the function
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;

		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getAlterCode(BaseObject *object) final;

		void updateDependencies() override;
};

#endif
