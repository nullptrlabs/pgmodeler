/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\brief Implements the operations to manipulate operators on the database.
\note <strong>Creation date:</strong> 16/04/2008
*/

#ifndef OPERATOR_H
#define OPERATOR_H

#include "baseobject.h"
#include "function.h"

class __libcore Operator: public BaseObject {
	private:
		//! \brief Stores the functions that defines the operator
		Function *functions[3];

		//! \brief Stores the arguments types (left and right) used by the operator
		PgSqlType argument_types[2];

		//! \brief Stores the auxiliary operators
		Operator *operators[2];

		bool	hashes, //! \brief Indicates that the operator can execute a hash join
		merges;  //! \brief Indicates that the operator can execute a merge join

	protected:
		virtual void configureSearchAttributes() override;

	public:
		enum FunctionId: unsigned {
			FuncOperator,
			FuncJoin,
			FuncRestrict
		};

		enum ArgumentId: unsigned {
			LeftArg,
			RightArg
		};

		enum OperatorId: unsigned {
			OperCommutator,
			OperNegator
		};

		Operator();

		virtual ~Operator(){}

		//! \brief Defines the name of the operator
		virtual void setName(const QString &name) override;

		//! \brief Defines the function used by the operator (constants FUNC_[OPERATOR | JOIN | RESTRICT])
		void setFunction(Function *func, FunctionId func_id);

		//! \brief Defines the argument data type for operator (constants ARG_[LEFT | RIGHT])
		void setArgumentType(PgSqlType arg_type, ArgumentId arg_id);

		//! \brief Defines the auxiliary operators (constants OPER_[COMMUTATOR | NEGATOR])
		void setOperator(Operator *oper, OperatorId op_id);

		//! \brief Defines that the operator accepts hash join
		void setHashes(bool value);

		//! \brief Defines that the operator accepts merge join
		void setMerges(bool value);

		//! \brief Returns the function used by the operator referencing it by its type
		Function *getFunction(FunctionId func_id);

		//! \brief Returns the type of the passed argument id
		PgSqlType getArgumentType(ArgumentId arg_id);

		//! \brief Returns on of the auxiliary operators
		Operator *getOperator(OperatorId op_id);

		//! \brief Returns whether the operator accepts hash join
		bool isHashes();

		//! \brief Returns whether the operator accepts merge join
		bool isMerges();

		//! \brief Validates the passed name using the naming rule for operators
		static bool isValidName(const QString &name);

		//! \brief Returns the SQL / XML definition for the operator
		virtual QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;
		virtual QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Returns the operator signature
		virtual QString getSignature(bool format_name=true) override;

		virtual void updateDependencies() override;
};

#endif
