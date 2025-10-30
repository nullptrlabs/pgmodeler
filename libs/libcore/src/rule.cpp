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

#include "rule.h"
#include "utilsns.h"

Rule::Rule()
{
	execution_type=ExecutionType::Null;
	obj_type=ObjectType::Rule;
	attributes[Attributes::EventType]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::Condition]="";
	attributes[Attributes::ExecType]="";
	attributes[Attributes::Commands]="";
}

void Rule::setCommandsAttribute()
{
	QString str_cmds;
	unsigned i, qtd;

	qtd=commands.size();
	for(i=0; i < qtd; i++)
	{
		str_cmds+=commands[i];
		if(i < (qtd-1)) str_cmds+=";";
	}

	attributes[Attributes::Commands]=str_cmds;
}

void Rule::setEventType(EventType type)
{
	setCodeInvalidated(event_type != type);
	event_type=type;
}

void Rule::setExecutionType(ExecutionType type)
{
	setCodeInvalidated(execution_type != type);
	execution_type=type;
}

void Rule::setConditionalExpression(const QString &expr)
{
	setCodeInvalidated(conditional_expr != expr);
	conditional_expr=expr;
}

void Rule::addCommand(const QString &cmd)
{
	//Raises an error if the command is empty
	if(cmd.isEmpty())
		throw Exception(ErrorCode::InsEmptyRuleCommand,PGM_FUNC,PGM_FILE,PGM_LINE);

	QString cmd_aux=cmd;
	cmd_aux.remove(';');
	commands.push_back(cmd_aux);
	setCodeInvalidated(true);
}

EventType Rule::getEventType()
{
	return event_type;
}

ExecutionType Rule::getExecutionType()
{
	return execution_type;
}

QString Rule::getConditionalExpression()
{
	return conditional_expr;
}

QString Rule::getCommand(unsigned cmd_idx)
{
	//Raises an error if the command index is out of bound
	if(cmd_idx >= commands.size())
		throw Exception(ErrorCode::RefRuleCommandInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return commands[cmd_idx];
}

unsigned Rule::getCommandCount()
{
	return commands.size();
}

void Rule::removeCommand(unsigned cmd_idx)
{
	//Raises an error if the command index is out of bound
	if(cmd_idx>=commands.size())
		throw Exception(ErrorCode::RefRuleCommandInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	commands.erase(commands.begin() + cmd_idx);
	setCodeInvalidated(true);
}

void Rule::removeCommands()
{
	commands.clear();
	setCodeInvalidated(true);
}

QString Rule::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setCommandsAttribute();
	attributes[Attributes::Condition]=conditional_expr;
	attributes[Attributes::ExecType]=(~execution_type);
	attributes[Attributes::EventType]=(~event_type);

	if(getParentTable())
		attributes[Attributes::Table]=getParentTable()->getName(true);

	return BaseObject::__getSourceCode(def_type);
}

void Rule::generateHashCode()
{
	TableObject::generateHashCode();
	hash_code = UtilsNs::getStringHash(hash_code + ~execution_type + ~event_type);
}
