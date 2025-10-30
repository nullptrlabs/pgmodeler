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
\class TableObject
\brief This class serves as a base class for objects that are embedded on tables
like columns, rules, triggers, indexes, constraints.
\note <strong>Creation date:</strong> 24/07/2008
*/

#ifndef TABLE_OBJECT_H
#define TABLE_OBJECT_H

#include "basetable.h"

class __libcore TableObject: public BaseObject {
	private:
		//! \brief Stores the table that owns this object
		BaseTable *parent_table;

		//! \brief Stores a reference to the relationship that generates the object
		BaseObject *parent_rel;

		/*! \brief Indicates that the object was included automaticaly by "one to many",
		 "one to many", "many to many" relationships */
		bool add_by_linking;

		/*! \brief Indicates that the object was included automaticaly by "one to many",
		 "generalization" relationships */
		bool add_by_generalization;

		/*! \brief Indicates that the object was included automaticaly by "one to many",
		 "copy" relationships */
		bool add_by_copy;

		bool decl_in_table;

	protected:
		QString hash_code;

		//! \brief Defines that the object is included by relationship (1-1, 1-n, n-n)
		void setAddedByLinking(bool value);

		//! \brief Defines that the object is included by generalization relationship
		void setAddedByGeneralization(bool value);

		//! \brief Defines that the object is include by copy relationship
		void setAddedByCopy(bool value);

		/*! \brief Defines that the object's SQL code must be created inside parent's
		 * table declaration, this is true by default. This attribute is only changed
		 * on export operations. This attribute is used only by columns and constraints, other
		 * types of child objects will ignore it */
		void setDeclaredInTable(bool value);

	public:
		TableObject();
		~TableObject() override = default;

		//! \brief Defines the parent table for the object
		virtual void setParentTable(BaseTable *table);

		//! \brief Returns the object parent table
		BaseTable *getParentTable();

		virtual void setParentRelationship(BaseObject *parent_rel);

		BaseObject *getParentRelationship();

		/*! \brief This method is purely virtual to force the derived classes
		 *  overload this method. This also makes class TableObject
		 *  not instantiable */
		QString getSourceCode(SchemaParser::CodeType def_type) override = 0;

		QString getDropCode(bool cascade) override;

		/*! \brief This version compares only the comments of two TableObject instances
		 *  returning the proper command to modify the comment, when needed */
		QString getAlterCode(BaseObject *object) override;

		QString getSignature(bool format = true) override;

		//! \brief Returns whether the object was added by relationship 1-1, 1-n, n-n
		bool isAddedByLinking();

		//! \brief Returns whether the object was added by generalization
		bool isAddedByGeneralization();

		//! \brief Returns whether the object was added by copy relationship
		bool isAddedByCopy();

		/*! \brief Returns whether the object was added by relationship (covering all the
		 possible relationship types) */
		bool isAddedByRelationship();

		bool isDeclaredInTable();

		//! \brief Returns if the passed type is a table child object (column, constraint, index, rule, trigger)
		static bool isTableObject(ObjectType type);

		void setCodeInvalidated(bool value) override;

		void operator = (TableObject &object);

		virtual void generateHashCode();

		QString getHashCode();

		friend class Relationship;
		friend class PhysicalTable;
		friend class Table;
		friend class ModelsDiffHelper;
		friend class DatabaseModel;
};

#endif
