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
\brief Implements the basic attributes and operations shared by tables and foreign tables.
*/

#ifndef PHYSICAL_TABLE_H
#define PHYSICAL_TABLE_H

#include <QStringList>
#include "basegraphicobject.h"
#include "basetable.h"
#include "column.h"
#include "constraint.h"
#include "trigger.h"
#include "partitionkey.h"
#include "copyoptions.h"
#include "pgsqltypes/partitioningtype.h"

class __libcore PhysicalTable: public BaseTable {
	protected:
		//! \brief Specifies the table from which columns are copied
		PhysicalTable *copy_table;

		//! \brief Specifies the copy table options
		CopyOptions copy_op;

		/*! \brief Stores the initial data of the table in CSV like form.
		This will produce a set of INSERT commands that is appended to the table's SQL definition */
		QString initial_data;

		//! \brief The partition bounding expression
		QString part_bounding_expr;

		//! \brief Vectors that store basic table attributes
		std::vector<TableObject *> columns;
		std::vector<TableObject *> constraints;
		std::vector<TableObject *> triggers;

		//! \brief Stores the tables that 'this' object inherits attributes
		std::vector<PhysicalTable *> ancestor_tables;

		//! \brief Stores the tables that 'this' object has as its partitions
		std::vector<PhysicalTable *> partition_tables;
		
		//! \brief Stores the partition keys of the table partitioning being used
		std::vector<PartitionKey> partition_keys;

		//! \brief Stores the table which this one is partition of
		PhysicalTable *partitioned_table;

		/*! \brief Indicates that constraints and columns are generated in for of ALTER commands.
		When true this will cause constraints and columsn to be created in a separated command
		outside tha table's declaration */
		bool gen_alter_cmds;

		//! \brief Stores the relationship added column / constraints indexes
		std::map<QString, unsigned> col_indexes,	constr_indexes;

		//! \brief The partitioning mode/type used by the table
		PartitioningType partitioning_type;

		/*! \brief Gets one table ancestor (ObjectType::Table) or copy (ObjectType::ObjBaseTable) using its name and stores
		 the index of the found object on parameter 'obj_idx' */
		BaseObject *getObject(const QString &name, ObjectType obj_type, int &obj_idx);

		//! \brief The methods below generates the table attributes used by the SchemaParser
		void setColumnsAttribute(SchemaParser::CodeType def_type, bool incl_rel_added_cols, bool incl_constraints);
		void setConstraintsAttribute(SchemaParser::CodeType def_type);
		void setCommentAttribute(TableObject *tab_obj);
		void setAncestorTableAttribute();
		void setRelObjectsIndexesAttribute();		

		//! \brief Adds an ancestor table
		void addAncestorTable(PhysicalTable *tab, int idx=-1);

		//! \brief Adds a partition table
		void addPartitionTable(PhysicalTable *tab);

		//! \brief Removes a partition table
		void removePartitionTable(PhysicalTable *tab);

		/*! \brief Returns the index of the partition table. If the
		 * compare_names is true then the search will compare the names if
		 * the object itself is not present in the list of partitions */
		int getPartitionTableIndex(PhysicalTable *tab, bool compare_names);
		
		//! \brief Removes an acestor table using its name
		void removeAncestorTable(const QString &name);

		//! \brief Removes an acestor table using its index
		void removeAncestorTable(unsigned idx);

		/*! \brief Updates the "decl_in_table" status for columns/constraints
		indicating if ALTER commands must be generated or not */
		void updateAlterCmdsStatus();

		void saveRelObjectsIndexes(ObjectType obj_type);
		void restoreRelObjectsIndexes(ObjectType obj_type);

		//! \brief Create an insert command from a list of columns and the values.
		QString createInsertCommand(const QStringList &col_names, const QStringList &values);

		//! \brief Performs the destruction of all children objects and internal lists clearing
		void destroyObjects();

		//! \brief Toggles the generation of columns and constraints in form of ALTER commands
		void __setGenerateAlterCmds(bool value);

	public:
		PhysicalTable();
		~PhysicalTable() override = default;

		//! \brief Returns true if the provided table is considered a physical table (Table, ForeignTable, PhysicalTable)
		static bool isPhysicalTable(ObjectType obj_type);

		//! \brief Defines the table's name. This method updates the type named after the table
		void setName(const QString &name) override;

		//! \brief Defines the table's schema. This method updates the type named after the table
		void setSchema(BaseObject *schema) override;

		//! \brief Configures the copy table
		void setCopyTable(PhysicalTable *tab);

		//! \brief Configures the copy table options
		void setCopyTableOptions(CopyOptions cpy_op);

		//! \brief Returns the copy table
		PhysicalTable *getCopyTable();

		//! \brief Get the copy table options
		CopyOptions getCopyTableOptions();

		//! \brief Defines if the partitioning type of the table
		virtual void setPartitioningType(PartitioningType part_type);

		//! \brief Returns the current partitioning type defined for the table
		PartitioningType getPartitioningType();

		//! \brief Returns the partitioned table which this table is a partition of
		PhysicalTable *getPartitionedTable();

		//! \brief Adds an object to the table. It can be inserted at a specified index 'obj_idx'.
		void addObject(BaseObject *obj, int obj_idx = -1) override;

		//! \brief Gets a object from table through its index and type
		BaseObject *getObject(unsigned obj_idx, ObjectType obj_type) override;

		//! \brief Gets a object from table through its name and type
		BaseObject *getObject(const QString &name, ObjectType obj_type) override;

		//! \brief Removes a object from table through its index and type
		void removeObject(unsigned obj_idx, ObjectType obj_type) override;

		//! \brief Removes a object from table through its name and type
		void removeObject(const QString &name, ObjectType obj_type) override;

		//! \brief Removes the specified object from table
		void removeObject(BaseObject *obj) override;

		//! \brief Adds a column to table (optionally the user can add the object at the specified index 'idx')
		void addColumn(Column *col, int idx=-1);

		//! \brief Adds a constraint to table (optionally the user can add the object at the specified index 'idx')
		void addConstraint(Constraint *constr, int idx=-1);

		//! \brief Adds a trigger to table (optionally the user can add the object at the specified index 'idx')
		void addTrigger(Trigger *trig, int idx=-1);

		//! \brief Adds a partition table
		void setPartionedTable(PhysicalTable *table);

		//! \brief Defines the partition bounding expression
		void setPartitionBoundingExpr(const QString part_bound_expr);

		//! \brief Returns the partition bounding expression
		QString getPartitionBoundingExpr();

		//! \brief Returns the partition tables
		std::vector<PhysicalTable *> getPartionTables();

		//! \brief Returs if the provided table is amongst the partitions
		bool isPartitionTableExists(PhysicalTable *table, bool compare_names);

		//! \brief Adds partition keys to the table. This method overrides the currently configure partition keys.
		void addPartitionKeys(std::vector<PartitionKey> &part_keys);

		//! \brief Remove all partition keys configured for the table
		void removePartitionKeys();

		/*! \brief Gets a column through its name. The boolean parameter is used
		 to search columns referencing their old names */
		Column *getColumn(const QString &name, bool ref_old_name=false);

		//! \brief Gets a column through its index
		Column *getColumn(unsigned idx);

		//! \brief Gets a constraint through its name.
		Constraint *getConstraint(const QString &name);

		//! \brief Gets a constraint through its index
		Constraint *getConstraint(unsigned idx);

		//! \brief Gets a trigger through its name.
		Trigger *getTrigger(const QString &name);

		//! \brief Gets a trigger through its index
		Trigger *getTrigger(unsigned idx);

		//! \brief Gets a ancestor table through its name
		PhysicalTable *getAncestorTable(const QString &name);

		//! \brief Gets a ancestor table through its index
		PhysicalTable *getAncestorTable(unsigned idx);

		//! \brief Returns the primary key of the table. Returns nullptr when it doesn't exists
		Constraint *getPrimaryKey();

		//! \brief Gets the column count
		unsigned getColumnCount();

		//! \brief Gets the constraint count
		unsigned getConstraintCount();

		//! \brief Gets the trigger count
		unsigned getTriggerCount();

		//! \brief Gets the ancestor table count
		unsigned getAncestorTableCount();

		/*! \brief Gets the the count for the specified object type. The boolean parameter indicates
		 that objects added by relationship must be counted */
		unsigned getObjectCount(ObjectType obj_type, bool inc_added_by_rel = true) override;

		//! \brief Removes a column through its name
		void removeColumn(const QString &name);

		//! \brief Removes a column through its index
		void removeColumn(unsigned idx);

		//! \brief Removes a constraint through its name
		void removeConstraint(const QString &name);

		//! \brief Removes a constraint through its index
		void removeConstraint(unsigned idx);

		//! \brief Removes a trigger through its name
		void removeTrigger(const QString &name);

		//! \brief Removes a trigger through its index
		void removeTrigger(unsigned idx);

		//! \brief Returns the SQL / XML definition for table
		QString getSourceCode(SchemaParser::CodeType def_type) override = 0;

		//! \brief Gets the object index using its name and type
		int getObjectIndex(const QString &name, ObjectType obj_type) override;

		/*! \brief Returns the index for the specified table object.
		If the object specified on the parameter owns to another table other than 'this'
		then the name of the objects are compared instead of the memory address */
		int getObjectIndex(BaseObject *obj) override;

		//! \brief Returns the children objects of the table excluding the provided children types
		std::vector<BaseObject *> getObjects(const std::vector<ObjectType> &excl_types = {}) override;

		//! \brief Returns all the partition keys used by the table
		std::vector<PartitionKey> getPartitionKeys();

		//! \brief Protects the table and its aggregated objects against modification
		void setProtected(bool value) override;

		/*! \brief Toggles the generation of columns and constraints in form of ALTER commands.
		 *  This method ignores the value when true if the table is a partitioned or partition. */
		void setGenerateAlterCmds(bool value);

		//! \brief Returns the current status of generation of ALTER commands for columns and constraints
		bool isGenerateAlterCmds();

		/*! \brief Returns if the specified column is referenced by one of the constraints on table.
		 The user must specify the constraint type */
		bool isConstraintRefColumn(Column *column, ConstraintType constr_type);

		/*! \brief Returns if the specified column is referenced by one of the constraints on table.
		 The user must specify the constraint type */
		bool isPartitionKeyRefColumn(Column *column);

		//! \brief Swaps two objects position
		void swapObjectsIndexes(ObjectType obj_type, unsigned idx1, unsigned idx2);

		//! \brief Returns if the table references objects added by relationship
		bool isReferRelationshipAddedObject();
		
		//! \brief Returns if the table is a partition of another table
		bool isPartition();

		//! \brief Returns if the table is a partitioned. This is the same as getPartitioningType() != null
		bool isPartitioned();

		//! \brief Copy the attributes between two tables
		void operator = (PhysicalTable &table);

		//! \brief Returns the specified object type list. Returns null if an invalid object type is provided
		virtual std::vector<TableObject *> *getObjectList(ObjectType obj_type);

		//! \brief Returns a list of objects that references the providede column.
		std::vector<TableObject *> getColumnReferences(Column *column);

		//! \brief Reset the current index of the objects created by relationship
		void resetRelObjectsIndexes();

		//! \brief Save the current index of the objects created by relationship
		void saveRelObjectsIndexes();

		/*! \brief Restore the position of the objects created by relationships.
		This method must be used with caution since it generate a new list of object replacing
		the original inside the table. Also this method can be slow in huge tables */
		void restoreRelObjectsIndexes();

		//! \brief Creates custom index from rel. created object using a name and index vectors as input.
		void setRelObjectsIndexes(const std::vector<QString> &obj_names, const std::vector<unsigned> &idxs, ObjectType obj_type);

		//! \brief Invalidates the cached code forcing the generation of both SQL and XML
		void setCodeInvalidated(bool value) override;

		/*! \brief Returns the alter definition by comparing the this table against the one provided via parameter
		 * This is a pure virtual method and must be implemented by children classes */
		QString getAlterCode(BaseObject *object) override = 0;

		/*! \brief Defines an initial set of data for the table in a CSV-like buffer.
		In order to separate columns and values use the DATA_SEPARATOR char and to separate
		rows use the DATA_LINE_BREAK */
		void setInitialData(const QString &value);

		//! \brief Returns the table's initial data in raw format
		QString getInitialData();

		/*! \brief Translate the CSV-like initial data to a set of INSERT commands.
		In invalid columns exist in the buffer they will be rejected when generating the commands */
		QString getInitialDataCommands();

		/*! \brief Generates the table's SQL code considering adding the relationship added object or not.
		 * Note if the method is called with incl_rel_added_objs = true it can produce an SQL/XML code
		 * that does not reflect the real semantics of the table. So take care to use this method and always
		 * invalidate the tables code (see setCodeInvalidated()) after retrieving the resulting code */
		void setTableAttributes(SchemaParser::CodeType def_type, bool incl_rel_added_objs, bool incl_contraints);

		void setObjectListsCapacity(unsigned capacity) override;

		unsigned getMaxObjectCount() override;

		QString getDataDictionary(bool split, bool md_format, const attribs_map &extra_attribs = {}) override;

		friend class Relationship;
		friend class OperationList;
		friend class ModelsDiffHelper;
};

#endif
