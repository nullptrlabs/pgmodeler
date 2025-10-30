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
\brief Implements the operations to manipulate views on database.
\note <strong>Creation date:</strong> 07/04/2008
*/

#ifndef COMPAT_VIEW_H
#define COMPAT_VIEW_H

/**
\ingroup pgmodeler-cli
\namespace CompatNs
\class View
\brief This is a minimal portion of the legacy class View (pgModeler 1.1.0-alpha1 and below) that is used to
convert views in older models to the new format introduced by pgModeler 1.1.0-beta.
*/

#include "reference.h"
#include "basetable.h"
#include "rule.h"
#include "index.h"

namespace CompatNs {

	class View: public BaseTable {
		private:
			//! \brief Stores the references to expressions and objects
			std::vector<Reference> references;

			/*! \brief Vectors that stores indexes to the view references in each
			 SQL part: SELECT-FROM, FROM-WHERE, after WHERE, expressions at the very end of definition (e.g. group by) */
			std::vector<unsigned>	exp_select,
			exp_from,
			exp_where,
			exp_end;

			std::vector<SimpleColumn> columns;

			/*! \brief Commom table expression. This is prepend on the views definition.
			CTE's are available since PostgreSQL 8.4:
				> http://www.postgresql.org/docs/8.4/interactive/queries-with.html */
			QString cte_expression;

			//! \brief Indicates that the view is a materialized one. This setting is auto exclusive with 'recursive'
			bool materialized,

			//! \brief Indicates that the materialized view should not be initialized at creation time (default=false)
			with_no_data,

			//! \brief Indicates that the view is a a recursive one. This setting is auto exclusive with 'materialized'
			recursive;

			//! \brief Sets the definition attribute used by the SchemaParser
			void setDefinitionAttribute();

			//! \brief Sets the references attribute used by the SchemaParser
			void setReferencesAttribute();

			/*! \brief Returns the reference index on the view. When it doesn't exists,
			 the method returns -1 */
			int getReferenceIndex(Reference &refer);

			//! \brief Returns the reference to internal expression list according to the SQL expression type
			std::vector<unsigned> *getExpressionList(Reference::SqlType sql_type);

			void setSQLObjectAttribute();

			//! \brief Returns a unique name for a columns comparing it to the existent columns. In case of duplication the name receives a suffix
			QString getUniqueColumnName(const QString &name);

		public:
			View();

			void setMaterialized(bool value);
			void setRecursive(bool value);
			void setWithNoData(bool value);

			bool isMaterialized();
			bool isRecursive();
			bool isWithNoData();

			/*! \brief Returns a simple column by searching by the name
			 *  This method will return an invalid SimpleColumn instance if there's
			 *  no matching column in the view */
			SimpleColumn getColumn(const QString &name);

			/*! \brief Adds a reference to the view specifying the SQL expression type for it
			 (refer to class Reference::SQL_??? constants). The 'expr_id' parameter is the
			 index where the reference must be inserted. By defaul the method always adds
			 new references at the end of the list */
			void addReference(Reference &refer, Reference::SqlType sql_type, int expr_id=-1);

			/*! \brief Adds a trigger or rule into the view. If the index is specified ( obj_idx >= 0)
			inserts the object at the position */
			void addObject(BaseObject *, int = -1) override {}

			//! \brief Remove a object from view using its reference
			void removeObject(BaseObject *) override {}

			//! \brief Removes the object using the index and type
			void removeObject(unsigned, ObjectType) override {}

			//! \brief Removes the object using the name and type
			void removeObject(const QString &, ObjectType) override {}

			//! \brief Returns the object index searching by its reference
			int getObjectIndex(BaseObject *) override
			{
				return -1;
			}

			//! \brief Returns the object index searching by its index and type
			int getObjectIndex(const QString &, ObjectType) override
			{
				return -1;
			}

			//! \brief Returns the children objects of the view excluding the provided children types (does not include references)
			std::vector<BaseObject *> getObjects(const std::vector<ObjectType> & = {}) override
			{
				return {};
			}

			//! \brief Returns the view's child object using its index and type
			TableObject *getObject(unsigned, ObjectType) override
			{
				return nullptr;
			}

			//! \brief Returns the view's child object using its name and type
			TableObject *getObject(const QString &, ObjectType) override
			{
				return nullptr;
			}

			//! \brief Returns the view's child object count
			unsigned getObjectCount(ObjectType, bool = false) override
			{
				return 0;
			}

			//! \brief Sets the commom table expression for the view
			void setCommomTableExpression(const QString &expr);

			//! \brief Removes all the references from the view
			void removeReferences();

			//! \brief Returns the commom table expression
			QString getCommomTableExpression();

			//! \brief Returns the reference count from view
			unsigned getReferenceCount();

			/*! \brief Returns the element count on the specified SQL expression type list (sql_type).
			 It possible to filter the reference type via 'ref_type' which must be filled
			 with the Reference::REFER_??? constants */
			unsigned getReferenceCount(Reference::SqlType sql_type, int ref_type = -1);

			//! \brief Returs one reference using its index
			Reference getReference(unsigned ref_id);

			/*! \brief Retuns one reference in the specified position (ref_id) on the
			 specified expression list (sql_type) */
			Reference getReference(unsigned ref_id, Reference::SqlType sql_type);

			//! \brief Returns the specified reference index on the specified expression list
			int getReferenceIndex(Reference &ref, Reference::SqlType sql_type);

			//! \brief Returns the SQL / XML definition for the view
			QString getSourceCode(SchemaParser::CodeType def_type) final;

			QString getDropCode(bool cascade) final;

			//! \brief Returns if the view is referencing the specified table
			bool isReferencingTable(PhysicalTable *tab);

			//! \brief Returns if the view is referencing the specified column
			bool isReferencingColumn(Column *col);

			//! \brief Returns if the view has an reference expression that is used as view definition
			bool hasDefinitionExpression();

			void setObjectListsCapacity(unsigned) override {}

			unsigned getMaxObjectCount() override
			{
				return DefMaxObjectCount;
			}

			/*! \brief Returns a list of deduced names for view's colums (useful for recursive views).
			 *	The names are retrieved, first, from columns aliases and lastly from table's columns
			 * 	when TABLE.* syntax is used. For expressions, if aliases aren't defined, a column name in the
			 *  for _expr#_ is used. */
			void generateColumns();

			//! \brief Returns the deduced columns of the view
			std::vector<SimpleColumn> getColumns();

			std::vector<Reference> getViewReferences();

			QString getDataDictionary(bool, bool, const attribs_map & = {}) override
			{
				return "";
			}

			QString getAlterCode(BaseObject *) final
			{
				return "";
			}

			void updateDependencies() override {}
	};
}
#endif
