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
\class Trigger
\brief Implements the operations to manipulate triggers on the database.
\note <strong>Creation date:</strong> 11/10/2006
*/

#ifndef TRIGGER_H
#define TRIGGER_H

#include "tableobject.h"
#include "function.h"
#include "pgsqltypes/firingtype.h"
#include "pgsqltypes/eventtype.h"
#include "pgsqltypes/deferraltype.h"

class __libcore Trigger: public TableObject{
	private:
		//! \brief Stores the transtion tables names (OLD and NEW) [REFERENCING { OLD | NEW } TABLE name]
		QString transition_tabs_names[2];

		//! \brief Arguments passed to the function that trigger executes
		QStringList arguments;

		/*! \brief Column list used as the trigger firing condition. This attribute was
		 introduced in PostgreSQL 9.1 and it is used only when the UPDATE event
		 is assigned to trigger. */
		std::vector<Column *> upd_columns;

		//! \brief Function that is excuted when the trigger is activated
		Function *function;

		//! \brief Condition that guarantees the trigger's execution
		QString condition;

		//! \brief Trigger firing mode (BEFORE, AFTER, INSTEAD OF)
		FiringType firing_type;

		//! \brief Map that marks which events activates the trigger
		std::map<EventType, bool> events;

		//! \brief Flag that indicates whether the function must be executed by row
		bool is_exec_per_row;

		//! \brief Table referecend by the trigger (only for constraint trigger)
		BaseTable *referenced_table;

		//! \brief Indicates if the trigger is a constraint trigger
		bool is_constraint,

		//! \brief Indicates whether the trigger is deferrable (only for constraint trigger)
		is_deferrable;

		//! \brief Deferral type for the trigger (only for constraint trigger)
		DeferralType deferral_type;

		//! \brief Formats the basic trigger attributes to be used by SchemaParser
		void setBasicAttributes(SchemaParser::CodeType def_type);

		//! \brief Format the function arguments to be used by the SchemaParser
		void setArgumentAttribute(unsigned def_type);

	public:
		enum TransitionTableId: unsigned {
			OldTableName,
			NewTableName
		};

		Trigger();
		~Trigger() override = default;

		/*! \brief Adds a column as a firing condition (only when the event UPDATE is used).
		 The columns added by this method must belongs to the trigger owner table. */
		void addColumn(Column *column);

		void addColumns(const std::vector<Column *> &cols);

		//! \brief Adds an argument to the trigger
		void addArgument(const QString &arg);

		//! \brief Adds a list of arguments to the trigger
		void addArguments(const QStringList &args);

		//! \brief Defines in which events the trigger is executed
		void setEvent(EventType event, bool value);

		//! \brief Defines the function to be executed by the trigger
		void setFunction(Function *func);

		//! \brief Defines the firing condition for trigger
		void setCondition(const QString &cond);

		//! \brief Defines the referenced table (only for constraint trigger)
		void setReferecendTable(BaseTable *ref_table);

		//! \brief Defines the deferral type
		void setDeferralType(DeferralType type);

		//! \brief Defines whether the trigger is deferrable or not
		void setDeferrable(bool value);

		//! \brief Changes the specified trigger agument replacing the current argument by the 'new_arg'
		void editArgument(unsigned arg_idx, const QString &new_arg);

		//! \brief Defines the moment when the trigger must be executed
		void setFiringType(FiringType firing_type);

		//! \brief Defines wheter the trigger executes per row
		void setExecutePerRow(bool value);

		//! \brief Defines if the trigger is constraint
		void setConstraint(bool value);

		//! \brief Defines the transition table name (OLD|NEW) referenced by the trigger
		void setTransitionTableName(TransitionTableId tab_idx, const QString &name);

		//! \brief Returns the transition table name (OLD|NEW) referenced by the trigger
		QString getTransitionTableName(TransitionTableId tab_idx);

		//! \brief Returns true if the trigger executes on the passed event
		bool isExecuteOnEvent(EventType event);

		//! \brief Returns the current EXECUTE PER ROW state
		bool isExecutePerRow();

		//! \brief Gets one reference column by its index
		Column *getColumn(unsigned col_idx);

		std::vector<Column *> getColumns();

		//! \brief Gets one argument by its index
		QString getArgument(unsigned arg_idx);

		//! \brief Gets the trigger firing condition
		QString getCondition();

		//! \brief Returns the function executed by the trigger
		Function *getFunction();

		//! \brief Returns the trigger argument count
		unsigned getArgumentCount();

		//! \brief Returns the reference column count
		unsigned getColumnCount();

		//! \brief Returns when the trigger executes
		FiringType getFiringType();

		//! \brief Remove an argument using its index
		void removeArgument(unsigned arg_idx);

		//! \brief Remove all arguments
		void removeArguments();

		//! \brief Remove all referenced columns
		void removeColumns();

		//! \brief Returns the referenced table
		BaseTable *getReferencedTable();

		//! \brief Returns the deferral type of the constraint trigger
		DeferralType getDeferralType();

		//! \brief Returns if the constraint trigger is deferrable or not
		bool isDeferrable();

		//! \brief Returns if the trigger is configured as a constraint trigger
		bool isConstraint();

		/*! \brief Returns whether the trigger references columns added
		 * by relationship. This method is used as auxiliary
		 * to control which triggers reference columns added by the
		 * relationship in order to avoid referece breaking due constants
		 * connections and disconnections of relationships */
		bool isReferRelationshipAddedColumn();

		/*! \brief Returns the list of all columns that is created by relationships.
		 * This method is slower than isReferRelationshipAddedColumn() so it's not
		 * recommended to use it only check if the object is referencing columns
		 * added by relationship */
		std::vector<Column *> getRelationshipAddedColumns();

		//! \brief Returns the SQL / XML definition for the trigger
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		/*! \brief Validates the trigger attributes according to the docs.
		 * This method is executed whenever the trigger is added to a table or view.
		 * Normally the user don't need to call it explicitly */
		void validateTrigger();

		bool acceptsReplaceCommand() override;

		QString getDataDictionary(bool md_format, const attribs_map &extra_attribs);

		void updateDependencies() override;
		void generateHashCode() override;
};

#endif
