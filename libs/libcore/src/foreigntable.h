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
\brief Implements the operations to manipulate foreign tables on the database.
*/

#ifndef FOREIGN_TABLE_H
#define FOREIGN_TABLE_H

#include "physicaltable.h"
#include "foreignobject.h"
#include "foreignserver.h"

class __libcore ForeignTable: public PhysicalTable, public ForeignObject {
	private:
		//! \brief The foreign server in which the foreign table resides
		ForeignServer *foreign_server;

	public:
		ForeignTable();

		~ForeignTable() override;

		void setForeignServer(ForeignServer *server);
		ForeignServer *getForeignServer();

		/*! \brief Adds an child object to the foreign table.
		 * This will raise an error if the user try to add constraints other than CHECK,
		 * indexes, rules and policies. This because foreign tables only accepts columns, check constraints, triggers */
		void addObject(BaseObject *object, int obj_idx = -1) override;

		/*! \brief This method ignores any partitioning type provided for the foreign table.
		 * It always set partitioning type as null since foreign tables doesn't support partitioning */
		void setPartitioningType(PartitioningType) override;

		//! \brief Returns the SQL / XML definition for table
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Copy the attributes between two tables
		void operator = (ForeignTable &table);

		//! \brief Returns the alter definition comparing the this table against the one provided via parameter
		QString getAlterCode(BaseObject *object) final;

		/*! \brief Generates the table's SQL code considering adding the relationship added object or not.
		 * Note if the method is called with incl_rel_added_objs = true it can produce an SQL/XML code
		 * that does not reflect the real semantics of the table. So take care to use this method and always
		 * invalidate the tables code (see setCodeInvalidated()) after retrieving the resulting code */
		QString __getSourceCode(SchemaParser::CodeType def_type, bool incl_rel_added_objs, bool incl_constraints = true);

		void updateDependencies() override;

		friend class Relationship;
		friend class OperationList;
};

#endif
