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
\class Relationship
\brief This class implements the operations to create relationship between tables
also generates the SQL code definition to represente the table link on PostgreSQL.
\note <strong>Creation date:</strong> 20/11/2006
*/

#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

#include "baserelationship.h"
#include "table.h"
#include "pgsqltypes/actiontype.h"
#include <stack>

/*
### Relationship implementation rules ###

[One to One relationship (1:1)]

(0,1) ---<>--- (0,1)
	 >> Columns are added on the destination table in order to represent the relationship.
	 >> The columns that represents the foreign key must accept null values.
	 >> The policy for DELETE and UPDATE events on the foreign key must be SET NULL.
	 >> The relationship attributes and constraints are added on same table where the
			foreign key columns are added.

(1,1) ---<>--- (0,1)
(0,1) ---<>--- (1,1)
	 >> Columns ared added on the table which minimum cardinality is 0.
	 >> The columns that represents the foreign key must accept null values.
	 >> The policy for DELETE and UPDATE events on the foreign key must be SET NULL.
	 >> The relationship attributes and constraints are added on same table where the
			foreign key columns are added.

(1,1) ---<>--- (1,1)
	 >> Not implemented because breaks the user modeling since its necessary to
			merge the envolved tables. If the user persists in creation of this type
			of relationship an error is raised.

------

[One to Many relationship (1:n)]

(0,1) ---<>--- (0,n)
(0,1) ---<>--- (1,n)
(1,1) ---<>--- (0,n)
(1,1) ---<>--- (1,n)

	>> Columns are added on the destination table in any cardinality combination.
	>> When the minimum cardinality is 0 on the "One" side (e.g. (0,1)) the foreign key columns
		 must accept null values in order to represent the optionality of the relationship. Also
		 the foreign key ON DELETE and ON UPDATE policy must be SET NULL.
	>> When the minimum cardinality is 1 on the "One" size (e.g. (1,1)) the foreign key columns
		 mustn't accept null values (NOT NULL) to represent the entity mandatory participation. Also
		 the foreign key ON DELETE and ON UPDATE policy must be RESTRICT.
	>> For identifier relationships the ON DELETE and ON UPDATE for the foreign key must be CASCADE
		 because the weak entity only exists if the strong exists too.

------

[Many to Many relationship (n:n)]

(n) ---<>--- (n)

	>> One table is created to represent the relationship. In the relationship type there is no
		 cardinality this means that the relationship is alway represented as (n) ---<>--- (n).
	>> Two foreign key is created on the new table each pointing to one of the envolved tables.
	>> The ON DELETE policy for the foreign keys must be RESTRICT.
	>> The ON UPDATE policy for the foreign keys must be UPDATE.
	>> The created table primary key is the union of the two foreign key columns.

------

[Relationships of degree > 2]

Must be manually simulated using the available binary relationships.

------

[NOTE]

* Case one relationship attribute is the identifier (primary key) for the "Many to Many" table
	relationship the fisrt will be merge with the automaticaly created primary key.

* "Many to Many" cannot be created as identifier because the two entities (tables)
	are always strong (in pgModeler's implementation).

* Case the relationship is identifier, this means, there is a weak entity
	in one side of the relatioship must be used the following rule:

	 1) The weak entity primary key must be composed with the columns used
			to reference the strong entity (foreign key columns)

	 2) The ON DELETE and ON UPDATE policy for the foreign key must be CASCADE, because
			for instance, in the case of one tuple on the strong entity is deleted the
			weak entity tuples must be destroyed also because this latter exists only because
			the first. */

class __libcore Relationship: public BaseRelationship {
	public:
		//! \brief Patterns ids
		enum PatternId: unsigned {
			SrcColPattern,
			DstColPattern,
			SrcFkPattern,
			DstFkPattern,
			PkPattern,
			UqPattern,
			PkColPattern,
			FkIdxPattern
		};

	private:
		/*! \brief Indicates that the relationship invalid because one or more critical attributes
		 where modified needing to be revalidated */
		bool invalidated,

		/*! \brief Indicates that the generated table primary key must have only a single column instead
		of the two from the foreign keys (only for n-n relationships) */
		single_pk_column;

		/*! \brief Stores the number of columns which were rejected at the time of relationship
		 connection. This is used only for generalization relationships because, according
		 to the rule a column is rejected in inheritance when both tables have the same column
		 of the same name and type. This attribute is used as an support to check if the relationship
		 is invalidated or not. */
		unsigned rejected_col_count;

		/*! \brief Stores the columns which are created to be referenced by foreign keys
			in order to represent the relationship */
		std::vector<Column *> gen_columns,

		/*! \brief Stores the primary key columns which from the referenced table
				This vector is used by the relationship validation method. */
		pk_columns;

		/*! \brief Stores the names of the columns referenced earlier, where
			the key of the map is the identifier of each column created by
			the relationship. This map is filled when the relationship is
			of type 1-1 and 1-n and when it is disconnected and its referenced
			columns are deallocated, because in this the column name history
			is lost and may cause the breakdown of references. Yet at the time
			of connection the old names stored on this map are assigned to columns
			related to each primary key column that generates the referenced columns
			and finally this map is cleared */
		std::map<unsigned, QString> prev_ref_col_names;

		//! \brief This map stores the name patterns for objects generated by relationship
		std::map<unsigned, QString> name_patterns;

		//! \brief Relationship attributes (used as columns on the receiver table)
		std::vector<TableObject *> rel_attributes;

		//! \brief Relationship constraint (used as constraints on the receiver table)
		std::vector<TableObject *> rel_constraints;

		//! \brief The index that is created on foreign key columns (only for 1-1, 1-n and n-n relationships)
		Index *fk_index;

		//! \brief The indexing type (access method) that is used by the FK index (only for 1-1, 1-n and n-n relationships)
		IndexingType fk_idx_type;

		//! \brief Foreign key that represents the 1-n relationship
		Constraint *fk_rel1n,

		/*! \brief Stores reference to the primary key automatically created when the
								relationship is identifier and the weak entity does not have
								a primary key */
		*pk_relident,

		/*! \brief Stores the reference to the special primary key. This constraint is only available
								to generalization / copy relationships */
		*pk_special,

		*pk_original,

		//! \brief Stores the unique key that represents the 1-1 relationship (including the fk_rel1n)
		*uq_rel11;

		//! \brief Stores the CHECK constraints copied from the parent table to its child
		std::vector<Constraint *> ck_constraints;

		//! \brief Table created by the relationship n-n
		Table *table_relnn;

		//! \brief Indicates if the relationship is identifier (when there is a strong and a weak entity)
		bool identifier;

		//! \brief Indicates if the foreign key (for 1-1, 1-n relationships) is deferrable
		bool deferrable;

		ActionType del_action,

		upd_action;

		//! \brief Deferral type used by the foreign key when this is deferrable
		DeferralType deferral_type;

		//! \brief Copy options assinged to receiver table (only copy relationship)
		CopyOptions copy_options;

		/*! \brief This vector allows the user to define which columns inherited / copied (via its indexes)
		 will be used on the special primary key in the receiver table */
		std::vector<unsigned> column_ids_pk_rel;

		//! \brief Name assigned to the generated table on n-n relationships
		QString tab_name_relnn,
		src_tab_prev_name,
		dst_tab_prev_name;

		//! \brief The partition bounding expression
		QString part_bounding_expr;

		std::stack<Column *> cols_stack;

		std::stack<Constraint *> constrs_stack;

		std::stack<Index *> indexes_stack;

		/*! \brief This method creates via "new" operator the object of the type Class, or,
		 * return an object from the stack of discarded objects if available for reused.
		 * Valid object types are Column, Constraint and Index. Returns null when Class is
		 * any other db object class */
		template<class Class>
		Class *createObject();

		/*! \brief This method stores the object in a stack of discarded objects instead of using delete operator
		 *  to free memory so the object can be resused the next time createObject() is called */
		void discardObject(TableObject *object);

		//! \brief Indicates if the column exists on the referenced column list
		bool isColumnExists(Column *column);

		//! \brief Executes the column addition for 1-1 relationship
		void addColumnsRel11();

		//! \brief Executes the column addition for 1-n relationship
		void addColumnsRel1n();

		//! \brief Executes the column addition for n-n relationship
		void addColumnsRelNn();

		/*! \brief Copy columns from one table to another. This operation is done in
		 *  relationships of type copy / generalization. It is necessary
		 *  to check duplicate names and incompatible types of columns. */
		void addColumnsRelGenPart(bool missing_only = false);

		/*! \brief Copy constraints from the parent table to the child. Currently, only
		check constraints are copied only if the NO INHERIT attribute is not set and
		there are no conflicting constraints (name or expression) on the child table */
		void addCheckConstrsRelGenPart();

		//! \brief Creates the special primary key (if in use) and other relationship constraints in the receiver table.
		void addConstraintsRelGenPart();

		/*! \brief Creates the foreign key that represents the relationship and adds it
		 to the receiver table. Must be specified the actions ON DELETE and UPDATE. */
		void addForeignKey(PhysicalTable *ref_tab, PhysicalTable *recv_tab, ActionType del_act, ActionType upd_act);

		/*! \brief Creates the unique key that represents the 1-1 relationship and adds it to
		 the receiver table */
		void addUniqueKey(PhysicalTable *recv_tab);

		/*! \brief Creates the index on the foreign key index hat represents the 1-* and n-n relationships
		 * and adds it to the receiver table */
		void addForeignKeyIndex(PhysicalTable *recv_tab);

		//! \brief Adds the relationship attributes (columns) into receiver table
		void addAttributes(PhysicalTable *recv_tab);

		/*! \brief Adds relationship constraints on the receiver table. If the relationship is
		 of type n-n, constraints will be added to the created table. If among the constraints
		 there is a primary key, then it will be merged with the primary key of receiver table */
		void addConstraints(PhysicalTable *recv_tab);

		/*! \brief Executes adicional configurations on receiver table primary key when the
		 relationship is identifier */
		void configureIndentifierRel(PhysicalTable *recv_tab);

		/*! \brief Copy the columns from the reference table to the receiver table. The parameter not_null indicates
		 that the columns must not accept null values. The parameter is_dst_table is used to force the usage of destination table
		and destination name pattern when creating a self many-to-many relationship */
		void copyColumns(PhysicalTable *ref_tab, PhysicalTable *recv_tab, bool not_null, bool is_dst_table = false, bool missing_only = false);

		/*! \brief This method is always executed before disconnection of the relationship.
		 Its function is to remove from the specified table all the attributes which
		 references any relationship generated column avoiding reference break */
		void removeTableObjectsRefCols(PhysicalTable *table);

		//! \brief Creates the special primary key using the names stored in the 'column_ids_pk_rel' vector
		void createSpecialPrimaryKey();

		//! \brief Removes all the columns created by the relationship from the specified table primary key if exists.
		void removeColumnsFromTablePK(PhysicalTable *table);

		//! \brief Removes a single column created by the relationship from the specified table primary key if exists.
		void removeColumnFromTablePK(PhysicalTable *table, Column *column);

		//! \brief Generates the object name according to the specified name pattern
		QString generateObjectName(PatternId pat_id, Column *id_col=nullptr, bool use_alias=false);

		void setOriginalPrimaryKey(Constraint *pk);

		//! \brief Adds the generated columns and relationshp attributes to the special primary key (if in use)
		void addGeneratedColsToSpecialPk();

		QString getSqlCode();

		QString getXmlCode();

	protected:
		//! \brief Destroy all the relationship attributes and constraints
		void destroyObjects();

		//! \brief Returns all the refenced columns of the relationship
		std::vector<Column *> getGeneratedColumns();

		//! \brief Returns the table generated by the n-n relationship
		Table *getGeneratedTable();

		/*! \brief Returns the generated foreign key that represents the 1-1, 1-n relationship,
		 as well the primary key generated by the identifier relationship and the
		 unique key used in 1-1 relationships */
		std::vector<Constraint *> getGeneratedConstraints();

		//! \brief Returns the index generated by the 1-* and n-n relationships
		Index *getGeneratedIndex();

		void configureSearchAttributes() override;

	public:
		//! \brief String used as the name suffix separator. Default '_'
		static const QString	SuffixSeparator,
		SrcTabToken,
		DstTabToken,
		GenTabToken,
		SrcColToken;

		Relationship(Relationship *rel);

		Relationship(BaseRelationship::RelType rel_type,
								 PhysicalTable *src_tab, PhysicalTable *dst_tab,
								 bool src_mdtry = false, bool dst_mdtry = false,
								 bool identifier = false);

		//! \brief  Connects the relationship making the configuration according to its type
		void connectRelationship();

		/*! \brief  Updates the columns of the relationship. This methods operates only when the
		 *  relationship is connected but is still in an invalidated state */
		bool updateGeneratedObjects();

		/*! \brief Disconnects the relationship from the tables removing all the attributes / constraints
			deallocating all the created object. */
		void disconnectRelationship(bool rem_tab_objs=true);

		//! \brief Defines the mandatory participation for the specified table
		void setMandatoryTable(TableId table_id, bool value);

		/*! \brief Warning: This method is effective only when the relationship is connected. Only
			in this situation you obtain the columns generated by connecting relationship.

			This method defines what columns inherited / copied will be used
			as special primary key on receiver table. When the relationship is connected
			the special primary key is created using the columns inherited / copied.

			This special primary key is always destroyed when the relationship is disconnected and
			recreated when connected. This is a way available to the user create primary keys
			using the relationship included columns because on the current modeling, its not possibl
			directly create primary keys in the receiver table using columns created by the relationship.

			** This feature can only be used in generalization / copy relationships ** */
		void setSpecialPrimaryKeyCols(std::vector<unsigned> &cols);

		//! \brief Returns the column indexes used by the special primary key
		std::vector<unsigned> getSpecialPrimaryKeyCols();

		//! \brief Defines the name of the auto generated table on the n-n relationship
		void setTableNameRelNN(const QString &name);

		//! \brief Returns the name of the table auto generated by the n-n relationship
		QString getTableNameRelNN();

		/*! \brief Defines the partition bounding expression associated to the partition table (receiver table)
		 * when the relationship is connected (only for partitioning relationship) */
		void setPartitionBoundingExpr(const QString &part_bound_expr);

		//! \brief Returns the partition bouding expression configured for the relationship (only for partitioning relationship)
		QString getPartitionBoundingExpr();

		//! \brief Defines if the created foreign key  is deferrable (only for 1-1, 1-n relationships)
		void setDeferrable(bool value);

		//! \brief Returns if the created foreign key is deferrable (only for 1-1, 1-n relationships)
		bool isDeferrable();

		//! \brief Defines the deferral type for the created foreign key (only for 1-1, 1-n relationships)
		void setDeferralType(DeferralType defer_type);

		/*! \brief Defines the type of action for generated foreign keys (ON DELETE and ON UPDATE)
		 User must use Constraint::[DELETE_ACTION|UPDATE_ACTION] (only for 1-1, 1-n relationships) */
		void setActionType(ActionType act_type, Constraint::ActionEvent act_id);

		//! \brief Returns the deferral tyep for the created foreign key (only for 1-1, 1-n relationships)
		DeferralType getDeferralType();

		//! \brief Defines the FK index access method (indexing type)
		void setFKIndexType(IndexingType idx_type);

		//! \brief Returns the FK index access method (indexing type)
		IndexingType getFKIndexType();

		//! \brief Defines if the relationship is identifier
		void setIdentifier(bool value);

		//! \brief Returns if the relationship is identifier
		bool isIdentifier();

		//! \brief Set the copy options (only for copy relationships)
		void setCopyOptions(CopyOptions copy_op);

		//! \brief Defines the format for the specified pattern
		void setNamePattern(PatternId pat_id, const QString &pattern);

		//! \brief Gets the current format for the specified pattern
		QString getNamePattern(PatternId pat_id);

		//! \brief Returns the current copy options
		CopyOptions getCopyOptions();

		//! \brief Retuns the action type (ON DELETE or ON UPDATE) of the generated foreign keys
		ActionType getActionType(unsigned act_id);

		/*! \brief Returns if the relationship is invalidated in relation to propagation of columns.
			This method makes a series of verifications for each type of relationship,
			and if in any condition this method returns 'true' indicates that the relationship
			is no longer valid and must be reconnected. The reconnection operation is
			made on de model class ​​only because it treats all cases of invalidity at once. */
		bool isInvalidated();

		/*! \brief Forces the relationship to go into invalidated state. This method is useful to invalidate the
		relationship without add/remove attributes from it. Calling this method will cause the model to revalidate
		the relationship even it's structure does not reflect an invalid state (see isInvalidate).
		Use this wisely or it can cause huge slow downs or unexpected results. */
		void forceInvalidate();

		//! \brief Adds an attribute or constaint to the relationship.
		void addObject(TableObject *tab_obj, int obj_idx=-1);

		//! \brief Returns one attribute or constraint using its name
		TableObject *getObject(const QString &name, ObjectType obj_type);

		//! \brief Returns one attribute or constraint using its index
		TableObject *getObject(unsigned obj_idx, ObjectType obj_type);

		//! \brief Removes on attribute or constraint using its index
		void removeObject(unsigned obj_id, ObjectType obj_type);

		//! \brief Removes the specified attribute or constraint from the relationship
		void removeObject(TableObject *object);

		//! \brief Remove one attribute using its index
		void removeAttribute(unsigned attrib_idx);

		//! \brief Remove one constraint using its index
		void removeConstraint(unsigned constr_idx);

		//! \brief Gets an attribute using its index
		Column *getAttribute(unsigned attrib_idx);

		//! \brief Gets an attribute using its name
		Column *getAttribute(const QString &name);

		//! \brief Returns the list of user added attributes
		std::vector<TableObject *> getAttributes();

		//! \brief Gets an constraint using its index
		Constraint *getConstraint(unsigned constr_idx);

		//! \brief Gets an constraint using its name
		Constraint *getConstraint(const QString &name);

		//! \brief Returns the list of user added constraints
		std::vector<TableObject *> getConstraints();

		/*! \brief Returns the index of a relationship attribute or constraint. Returns -1
		 when the object doesn't exists */
		int getObjectIndex(TableObject *object);

		//! \brief Returns the attribute count
		unsigned getAttributeCount();

		//! \brief Returns the constraint count
		unsigned getConstraintCount();

		//! \brief Returns the attribute or constraint count
		unsigned getObjectCount(ObjectType obj_type);

		/*! \brief Returns if the relationship has one or more identifier attributes.
		 Identifier attributes are attributes which belongs to a primary key
		 added by the user. This method only scans the list of constraints
		 on the relationship searching for primary keys created by the user.
		 If found returns true. */
		bool hasIndentifierAttribute();

		/*! \brief Returns table that receives a copy of the columns that represent the relationship
		 according to its configuration.

		 WARNING: Not necessarily this method returns the destination table this because not in all
		 relationship configuration the receiver is the destination table */
		PhysicalTable *getReceiverTable();

		/*! \brief Returns table which serves as a reference when coping the columns to the
		 receiver table.

		 WARNING: Not necessarily this method returns the source table
		 because not in all relationship configuration the reference is the source table

		 For n-n relationships this method returns nullptr as this type of relationship
		 has 2 reference tables, which may be obtained by the method BaseRelationship::getTable() */
		PhysicalTable *getReferenceTable();

		void setSiglePKColumn(bool value);

		bool isSiglePKColumn();

		//! \brief Returns SQL / XML definition for the relationship.
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Copies the attributes from one relationship to another
		void operator = (Relationship &rel);

		QString getAlterRelationshipDefinition(bool undo_inh_part);

		//! \brief Returns true when the reference table is mandatory in the relationship
		bool isReferenceTableMandatory();

		//! \brief Returns true when the receiver table is mandatory in the relationship
		bool isReceiverTableMandatory();

		void setSQLDisabled(bool value) override;

		friend class DatabaseModel;
		friend class ModelWidget;
		friend class RelationshipWidget;
		friend class ModelExportHelper;
		friend class ModelsDiffHelper;
		friend class ModelDatabaseDiffForm;
};

#endif
