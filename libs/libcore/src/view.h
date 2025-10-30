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

#ifndef VIEW_H
#define VIEW_H

#include "basetable.h"
#include "trigger.h"
#include "rule.h"
#include "index.h"
#include "genericsql.h"
#include "pgsqltypes/checkoptiontype.h"

class __libcore View: public BaseTable {
	private:
		std::vector<TableObject *> triggers;
		std::vector<TableObject *> rules;
		std::vector<TableObject *> indexes;

		std::vector<SimpleColumn> gen_columns, custom_cols;

		std::vector<Reference> references;

		QString sql_definition;

		//! \brief Indicates that the view is a materialized one. This setting is auto exclusive with 'recursive'
		bool materialized,

		//! \brief Indicates that the materialized view should not be initialized at creation time (default=false)
		with_no_data,

		//! \brief Indicates that the view is a a recursive one. This setting is auto exclusive with 'materialized'
		recursive,

		security_invoker,

		security_barrier;

		CheckOptionType check_option;

		void setSQLObjectAttribute();

		void setOptionsAttributes(SchemaParser::CodeType def_type);

		//! \brief Returns a unique name for a columns comparing it to the existent columns. In case of duplication the name receives a suffix
		QString getUniqueColumnName(const QString &name);

	public:
		static const QString ExtraSCRegExp;

		View();

		~View() override;

		void setName(const QString &name) override;
		void setSchema(BaseObject *schema) override;
		void setProtected(bool value) override;

		void setMaterialized(bool value);
		void setRecursive(bool value);
		void setWithNoData(bool value);
		void setSecurityBarrier(bool value);
		void setSecurityInvoker(bool value);

		void setCheckOption(CheckOptionType check_opt);
		CheckOptionType getCheckOption();

		bool isMaterialized();
		bool isRecursive();
		bool isWithNoData();
		bool isSecurityInvoker();
		bool isSecurityBarrier();

		void setReferences(const std::vector<Reference> &obj_refs);
		void setCustomColumns(const std::vector<SimpleColumn> &cols);
		void setSqlDefinition(const QString &sql_def);

		QString getSqlDefinition();

		/*! \brief Returns a simple column by searching by the name
		 *  This method will return an invalid SimpleColumn instance if there's
		 *  no matching column in the view */
		SimpleColumn getColumn(const QString &name);

		/*! \brief Adds a trigger or rule into the view. If the index is specified ( obj_idx >= 0)
		inserts the object at the position */
		void addObject(BaseObject *obj, int obj_idx = -1) override;

		//! \brief Adds a trigger into the view
		void addTrigger(Trigger *trig, int obj_idx=-1);

		//! \brief Adds a rule into the view
		void addRule(Rule *rule, int obj_idx=-1);

		//! \brief Adds an index into the view
		void addIndex(Index *index, int obj_idx=-1);

		//! \brief Remove a object from view using its reference
		void removeObject(BaseObject *obj) override;

		//! \brief Removes the object using the index and type
		void removeObject(unsigned obj_idx, ObjectType obj_type) override;

		//! \brief Removes the object using the name and type
		void removeObject(const QString &name, ObjectType obj_type) override;

		//! \brief Remove a trigger from view using its index
		void removeTrigger(unsigned idx);

		//! \brief Remove a rule from view using its index
		void removeRule(unsigned idx);

		//! \brief Remove an index from view using its index
		void removeIndex(unsigned idx);

		//! \brief Returns the object index searching by its reference
		int getObjectIndex(BaseObject *obj) override;

		//! \brief Returns the object index searching by its index and type
		int getObjectIndex(const QString &name, ObjectType obj_type) override;

		//! \brief Returns the children objects of the view excluding the provided children types (does not include references)
		std::vector<BaseObject *> getObjects(const std::vector<ObjectType> &excl_types = {}) override;

		//! \brief Returns the view's child object using its index and type
		TableObject *getObject(unsigned obj_idx, ObjectType obj_type) override;

		//! \brief Returns the view's child object using its name and type
		TableObject *getObject(const QString &name, ObjectType obj_type) override;

		//! \brief Returns a trigger searching by its index
		Trigger *getTrigger(unsigned obj_idx);

		//! \brief Returns a rule searching by its index
		Rule *getRule(unsigned obj_idx);

		//! \brief Returns a index searching by its index
		Index *getIndex(unsigned obj_idx);

		//! \brief Returns the view's child object count
		unsigned getObjectCount(ObjectType obj_type, bool = false) override;

		//! \brief Returns the view's trigger count
		unsigned getTriggerCount();

		//! \brief Returns the view's rule count
		unsigned getRuleCount();

		//! \brief Returns the view's index count
		unsigned getIndexCount();

		//! \brief Removes all objects (triggers / roles) from view
		void removeObjects();

		//! \brief Returns the object list according to specified type
		std::vector<TableObject *> *getObjectList(ObjectType obj_type);

		//! \brief Returns the SQL / XML definition for the view
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getDropCode(bool cascade) final;

		/*! \brief Returns whether the view references columns added
		 by relationship. This method is used as auxiliary
		 to control which view reference columns added by the
		 relationship in order to avoid referece breaking due constants
		 connections and disconnections of relationships */
		bool isReferRelationshipAddedColumn();

		/*! \brief Returns the list of all columns that is created by relationships.
		 * This method is slower than isReferRelationshipAddedColumn() so it's not
		 * recommended to use it only check if the object is referencing columns
		 * added by relationship */
		std::vector<Column *> getRelationshipAddedColumns();

		//! \brief Returns if the view is referencing the specified table
		bool isReferencingTable(BaseTable *tab);

		void setObjectListsCapacity(unsigned capacity) override;

		unsigned getMaxObjectCount() override;

		/*! \brief Returns a list of deduced names for view's colums (useful for recursive views).
		 *	The names are retrieved, first, from columns aliases and lastly from table's columns
		 * 	when TABLE.* syntax is used. For expressions, if aliases aren't defined, a column name in the
		 *  for _expr#_ is used. */
		void generateColumns();

		//! \brief Returns the deduced columns of the view
		std::vector<SimpleColumn> getColumns();

		//! \brief Returns the user-defined columns of the view
		std::vector<SimpleColumn> getCustomColumns();

		std::vector<Reference> getObjectReferences();

		std::vector<BaseTable *> getReferencedTables();

		QString getDataDictionary(bool split, bool md_format, const attribs_map &extra_attribs = {}) override;

		QString getAlterCode(BaseObject *object) final;

		//! \brief Copy the attributes between two views
		void operator = (View &view);

		void updateDependencies() override;

		bool acceptsReplaceCommand() override;
};

#endif
