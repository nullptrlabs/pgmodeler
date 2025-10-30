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
\class OperatorClassElement
\brief Implements the operations to manipulate operator class
 elements (storage, operator, function).
\note <strong>Creation date:</strong> 08/07/2008
*/

#ifndef OPERATOR_CLASS_ELEMENT_H
#define OPERATOR_CLASS_ELEMENT_H

#include "operator.h"
#include "function.h"
#include "operatorfamily.h"

class __libcore OperatorClassElement {
	public:
		//! \brief Constants used to reference the element types
		enum ElementType: unsigned {
			OperatorElem,
			FunctionElem,
			StorageElem
		};

	private:
		/*! \brief Type of the operator class element.
		 This can have 3 possible values:
		 0 -> OPERATOR_ELEM
		 1 -> FUNCTION_ELEM
		 2 -> STORAGE_ELEM */
		ElementType element_type;

		//! \brief Function used by the element (only for type FUNCTION_ELEM)
		Function *function;

		//! \brief Operator used by the element (only for type OPERATOR_ELEM)
		Operator *_operator;

		//! \brief Operator family used by the element (only for type OPERATOR_ELEM)
		OperatorFamily *op_family;

		/*! \brief PostgreSQL type used in the indexing method of operator class.
		 (only for type STORAGE_ELEM) */
		PgSqlType storage;

		/*! \brief Strategy number (or support number for functions). This attribute
		 must have a value greater than 0 (only for OPERATOR_ELEM and FUNCTION_ELEM) */
		unsigned strategy_number;

	public:
		OperatorClassElement();
		~OperatorClassElement() = default;

		//! \brief Defines the element as a function clause
		void setFunction(Function *func, unsigned stg_number);

		//! \brief Defines the element as an operator clause
		void setOperator(Operator *oper, unsigned stg_number);

		//! \brief Defines operator family used by the element (only for Operator elements)
		void setOperatorFamily(OperatorFamily *op_family);

		//! \brief Defines the element as a storage clause
		void setStorage(PgSqlType storage);

		//! \brief Returns the element type
		ElementType getElementType();

		/*! \brief Returns the current assigned function.
		 This method returns nullptr when the element is not an FUNCTION_ELEM */
		Function *getFunction();

		/*! \brief Returns the current assigned operator.
		 This method returns nullptr when the element is not an OPERATOR_ELEM */
		Operator *getOperator();

		/*! \brief Returns the operator family used by element.
		This method returns nullptr when the element is not an OPERATOR_ELEM */
		OperatorFamily *getOperatorFamily();

		//! \brief Storage type of the element
		PgSqlType getStorage();

		//! \brief Returns the strategy (support) number of the element
		unsigned getStrategyNumber();

		//! \brief Returns the SQL / XML code definition for the element
		virtual QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Operator to compare two elements, returns true when all atributes has the same configuration
		bool operator == (OperatorClassElement &elem);

		std::vector<BaseObject *> getDependencies();
};

/* Registering the OperatorClassElement class as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(OperatorClassElement)

#endif
