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
\class Rule
\brief Implements the operations to manipulate table rules.
\note <strong>Creation date:</strong> 26/09/2006
*/

#ifndef RULE_H
#define RULE_H

#include "tableobject.h"
#include "pgsqltypes/executiontype.h"
#include "pgsqltypes/eventtype.h"

class __libcore Rule: public TableObject{
	private:
		//! \brief Commands executed by the rule when activated
		std::vector<QString> commands;

		//! \brief Conditional expression for the rule activation
		QString conditional_expr;

		//! \brief Rule execution type (ALSO or INSTEAD)
		ExecutionType execution_type;

		//! \brief Event when the rule is triggered (ON SELECT, NO UPDATE, ON INSERT, ON DELETE)
		EventType event_type;

		//! \brief Formats the commands string to be used by the SchemaParser
		void setCommandsAttribute();

	public:
		Rule();
		~Rule() override = default;

		//! \brief Adds the SQL command to be executed by the rule
		void addCommand(const QString &cmd);

		//! \brief Sets the conditional expression for the rule
		void setConditionalExpression(const QString &expr);

		//! \brief Sets the rule execution type (ALSO, INSTEAD)
		void setExecutionType(ExecutionType type);

		//! \brief Defines the event when the rule is triggered
		void setEventType(EventType type);

		//! \brief Returns one command executed by the rule using its index
		QString getCommand(unsigned cmd_idx);

		//! \brief Returns the SQL command count
		unsigned getCommandCount();

		//! \brief Returns the conditional expression for the rule
		QString getConditionalExpression();

		//! \brief Returns the event when the rule is triggered
		EventType getEventType();

		//! \brief Returns the execution type for the rule
		ExecutionType getExecutionType();

		//! \brief Removes one command form the rule using its index
		void removeCommand(unsigned cmd_idx);

		//! \brief Remove all commands from the rule
		void removeCommands();

		//! \brief Returns the SQL / XML definition for the rule
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		void generateHashCode() override;
};

#endif
