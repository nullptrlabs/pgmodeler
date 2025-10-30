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

#include "trigger.h"
#include "utilsns.h"
#include "coreutilsns.h"

Trigger::Trigger()
{
	unsigned i;
	EventType tipos[4]={EventType::OnInsert, EventType::OnDelete,
						EventType::OnUpdate, EventType::OnTruncate};

	function=nullptr;
	is_exec_per_row=is_constraint=is_deferrable=false;
	obj_type=ObjectType::Trigger;
	referenced_table=nullptr;

	for(i=0; i < 4; i++)
		events[tipos[i]]=false;

	attributes[Attributes::Arguments]="";
	attributes[Attributes::Events]="";
	attributes[Attributes::TriggerFunc]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::Columns]="";
	attributes[Attributes::FiringType]="";
	attributes[Attributes::PerRow]="";
	attributes[Attributes::InsEvent]="";
	attributes[Attributes::DelEvent]="";
	attributes[Attributes::UpdEvent]="";
	attributes[Attributes::TruncEvent]="";
	attributes[Attributes::Condition]="";
	attributes[Attributes::RefTable]="";
	attributes[Attributes::DeferType]="";
	attributes[Attributes::Deferrable]="";
	attributes[Attributes::DeclInTable]="";
	attributes[Attributes::Constraint]="";
	attributes[Attributes::OldTableName]="";
	attributes[Attributes::NewTableName]="";
}

void Trigger::addArgument(const QString &arg)
{
	arguments.append(arg);
}

void Trigger::addArguments(const QStringList &args)
{
	arguments.clear();
	arguments = args;
}

void Trigger::setArgumentAttribute(unsigned def_type)
{
	QStringList str_args;

	for(auto &arg : arguments)
	{
		if(def_type==SchemaParser::SqlCode)
			str_args.append("'" + arg + "'");
		else
			str_args.append(arg);
	}

	attributes[Attributes::Arguments] =
			str_args.join(def_type == SchemaParser::SqlCode ? "," : UtilsNs::DataSeparator);
}

void Trigger::setFiringType(FiringType firing_type)
{
	setCodeInvalidated(this->firing_type != firing_type);
	this->firing_type=firing_type;
}

void Trigger::setEvent(EventType event, bool value)
{
	if(event==EventType::OnSelect)
		throw Exception(ErrorCode::RefInvalidTriggerEvent,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(events[event] != value);
	events[event]=value;
}

void Trigger::setFunction(Function *func)
{
	//Case the function is null an error is raised
	if(!func)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::Trigger)),
										ErrorCode::AsgNotAllocatedFunction,PGM_FUNC,PGM_FILE,PGM_LINE);
	}


	//Case the function doesn't returns 'trigger' it cannot be used with the trigger thus raise an error
	if(func->getReturnType()!="trigger")
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidTriggerFunction)
										.arg("trigger"),PGM_FUNC,PGM_FILE,PGM_LINE);

	//Case the function has some parameters raise an error
	if(func->getParameterCount()!=0)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::Trigger)),
										ErrorCode::AsgFunctionInvalidParamCount,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	setCodeInvalidated(function != func);
	this->function=func;
}

void Trigger::setCondition(const QString &cond)
{
	setCodeInvalidated(condition != cond);
	this->condition=cond;
}

void Trigger::addColumn(Column *column)
{
	if(!column)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedColumn)
										.arg(this->getName(true))
										.arg(this->getTypeName()),
										ErrorCode::AsgNotAllocatedColumn,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(!column->getParentTable())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgColumnNoParent)
										.arg(this->getName(true))
										.arg(this->getTypeName()),
										ErrorCode::AsgNotAllocatedColumn,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(this->getParentTable() &&
		 column->getParentTable() != this->getParentTable())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidColumnTrigger)
										.arg(column->getName(true))
										.arg(this->getName(true)),
										ErrorCode::AsgInvalidColumnTrigger,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	upd_columns.push_back(column);
	setCodeInvalidated(true);
}

void Trigger::addColumns(const std::vector<Column *> &cols)
{
	try
	{
		for(auto &col : cols)
			addColumn(col);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Trigger::editArgument(unsigned arg_idx, const QString &new_arg)
{
	//Raises an error if the argument index is invalid (out of bound)
	if(static_cast<int>(arg_idx) >= arguments.size())
		throw Exception(ErrorCode::RefArgumentInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	arguments[arg_idx] = new_arg;
	setCodeInvalidated(true);
}

void Trigger::setExecutePerRow(bool value)
{
	setCodeInvalidated(is_exec_per_row != value);
	is_exec_per_row=value;
}

bool Trigger::isExecuteOnEvent(EventType event)
{
	if(event==EventType::OnSelect)
		throw Exception(ErrorCode::RefInvalidTriggerEvent,PGM_FUNC,PGM_FILE,PGM_LINE);

	return events.at(!event);
}

bool Trigger::isExecutePerRow()
{
	return is_exec_per_row;
}

QString Trigger::getArgument(unsigned arg_idx)
{
	//Raises an error if the argument index is invalid (out of bound)
	if(static_cast<int>(arg_idx) >= arguments.size())
		throw Exception(ErrorCode::RefArgumentInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return arguments[arg_idx];
}

Column *Trigger::getColumn(unsigned col_idx)
{
	//Raises an error if the column index is invalid (out of bound)
	if(col_idx>=upd_columns.size())
		throw Exception(ErrorCode::RefColumnInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return upd_columns[col_idx];
}

std::vector<Column *> Trigger::getColumns()
{
	return upd_columns;
}

unsigned Trigger::getArgumentCount()
{
	return arguments.size();
}

unsigned Trigger::getColumnCount()
{
	return upd_columns.size();
}

Function *Trigger::getFunction()
{
	return function;
}

QString Trigger::getCondition()
{
	return condition;
}

FiringType Trigger::getFiringType()
{
	return firing_type;
}

void Trigger::removeArgument(unsigned arg_idx)
{
	//Raises an error if the argument index is invalid (out of bound)
	if(static_cast<int>(arg_idx) >= arguments.size())
		throw Exception(ErrorCode::RefArgumentInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	arguments.removeAt(arg_idx);
	setCodeInvalidated(true);
}

void Trigger::removeArguments()
{
	arguments.clear();
	setCodeInvalidated(true);
}

void Trigger::removeColumns()
{
	upd_columns.clear();
	setCodeInvalidated(true);
}

void Trigger::setReferecendTable(BaseTable *ref_table)
{
	//If the referenced table isn't valid raises an error
	if(ref_table && ref_table->getObjectType()!=ObjectType::Table)
		throw Exception(ErrorCode::AsgObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(referenced_table != ref_table);
	this->referenced_table=ref_table;
}

void Trigger::setDeferralType(DeferralType type)
{
	setCodeInvalidated(deferral_type != type);
	deferral_type=type;
}

void Trigger::setDeferrable(bool value)
{
	setCodeInvalidated(is_deferrable != value);
	is_deferrable=value;
}

BaseTable *Trigger::getReferencedTable()
{
	return referenced_table;
}

DeferralType Trigger::getDeferralType()
{
	return deferral_type;
}

bool Trigger::isDeferrable()
{
	return is_deferrable;
}

void Trigger::setConstraint(bool value)
{
	setCodeInvalidated(is_constraint != value);
	is_constraint=value;
}

void Trigger::setTransitionTableName(TransitionTableId tab_idx, const QString &name)
{
	if(tab_idx > NewTableName)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(transition_tabs_names[tab_idx] != name);
	transition_tabs_names[tab_idx] = name;
}

QString Trigger::getTransitionTableName(TransitionTableId tab_idx)
{
	if(tab_idx > NewTableName)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return transition_tabs_names[tab_idx];
}

bool Trigger::isConstraint()
{
	return is_constraint;
}

bool Trigger::isReferRelationshipAddedColumn()
{
	std::vector<Column *>::iterator itr, itr_end;
	Column *col=nullptr;
	bool found=false;

	itr=upd_columns.begin();
	itr_end=upd_columns.end();

	while(itr!=itr_end && !found)
	{
		col=(*itr);
		found=col->isAddedByRelationship();
		itr++;
	}

	return found;
}

std::vector<Column *> Trigger::getRelationshipAddedColumns()
{
	std::vector<Column *> cols;

	for(auto &col : upd_columns)
	{
		if(col->isAddedByRelationship())
			cols.push_back(col);
	}

	return cols;
}

void Trigger::setBasicAttributes(SchemaParser::CodeType def_type)
{
	QString str_aux,
			attribs[4]={Attributes::InsEvent, Attributes::DelEvent,
									Attributes::TruncEvent, Attributes::UpdEvent },
			sql_event[4]={"INSERT OR ", "DELETE OR ", "TRUNCATE OR ", "UPDATE   "};
	unsigned count, i, i1, event_types[4]={EventType::OnInsert, EventType::OnDelete,
										   EventType::OnTruncate, EventType::OnUpdate};

	setArgumentAttribute(def_type);

	for(i=0; i < 4; i++)
	{
		if(events.at(event_types[i]))
		{
			str_aux+=sql_event[i];
			attributes[attribs[i]]=Attributes::True;

			if(event_types[i]==EventType::OnUpdate)
			{
				count=upd_columns.size();
				attributes[Attributes::Columns]="";

				for(i1=0; i1 < count; i1++)
				{
					attributes[Attributes::Columns]+=upd_columns.at(i1)->getName(true);
					if(i1 < count-1)
						attributes[Attributes::Columns]+=",";
				}
			}
		}
	}

	if(!str_aux.isEmpty()) str_aux.remove(str_aux.size()-3,3);

	if(def_type==SchemaParser::SqlCode && !attributes[Attributes::Columns].isEmpty())
		str_aux+=" OF " + attributes[Attributes::Columns];

	attributes[Attributes::Events]=str_aux;

	if(function)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::TriggerFunc]=function->getName(true);
		else
			attributes[Attributes::TriggerFunc]=function->getSourceCode(def_type, true);
	}
}

QString Trigger::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setBasicAttributes(def_type);

	/* Case the trigger doesn't referece some column added by relationship it will be declared
		inside the parent table construction by the use of 'decl-in-table' schema attribute */
	if(!isReferRelationshipAddedColumn())
		attributes[Attributes::DeclInTable]=Attributes::True;

	if(getParentTable())
		attributes[Attributes::Table]=getParentTable()->getName(true);

	attributes[Attributes::Constraint]=(is_constraint ? Attributes::True : "");
	attributes[Attributes::FiringType]=(~firing_type);

	//** Constraint trigger MUST execute per row **
	attributes[Attributes::PerRow]=((is_exec_per_row && !is_constraint) || is_constraint ? Attributes::True : "");

	attributes[Attributes::Condition]=condition;

	if(referenced_table)
		attributes[Attributes::RefTable]=referenced_table->getName(true);

	attributes[Attributes::Deferrable]=(is_deferrable ? Attributes::True : "");
	attributes[Attributes::DeferType]=(~deferral_type);

	if(def_type == SchemaParser::XmlCode)
	{
		attributes[Attributes::OldTableName]=transition_tabs_names[OldTableName];
		attributes[Attributes::NewTableName]=transition_tabs_names[NewTableName];
	}
	else
	{
		attributes[Attributes::OldTableName]=BaseObject::formatName(transition_tabs_names[OldTableName]);
		attributes[Attributes::NewTableName]=BaseObject::formatName(transition_tabs_names[NewTableName]);
	}

	return BaseObject::__getSourceCode(def_type);
}

void Trigger::validateTrigger()
{
	if(getParentTable())
	{
		ObjectType parent_type=getParentTable()->getObjectType();

		if(!is_constraint)
		{
			//The INSTEAD OF mode cannot be used on triggers that belongs to tables! This is available only for view triggers
			if(firing_type==FiringType::InsteadOf && parent_type != ObjectType::View)
				throw Exception(ErrorCode::InvTableTriggerInsteadOfFiring,PGM_FUNC,PGM_FILE,PGM_LINE);

			//The INSTEAD OF mode cannot be used on view triggers that executes for each statement
			if(firing_type==FiringType::InsteadOf && parent_type==ObjectType::View && !is_exec_per_row)
				throw Exception(ErrorCode::InvUsageInsteadOfOnTrigger,PGM_FUNC,PGM_FILE,PGM_LINE);

			//A trigger cannot make reference to columns when using INSTEAD OF mode and UPDATE event
			if(firing_type==FiringType::InsteadOf && events[EventType::OnUpdate] && !upd_columns.empty())
				throw Exception(ErrorCode::InvUsageInsteadOfUpdateTrigger,PGM_FUNC,PGM_FILE,PGM_LINE);

			//The TRUNCATE event can only be used when the trigger executes for each statement and belongs to a table
			if(events[EventType::OnTruncate] && (is_exec_per_row || parent_type==ObjectType::View))
				throw Exception(ErrorCode::InvUsageTruncateOnTrigger,PGM_FUNC,PGM_FILE,PGM_LINE);

			//A view trigger cannot be AFTER/BEFORE when it executes for each row
			if(parent_type==ObjectType::View && is_exec_per_row && (firing_type==FiringType::After || firing_type==FiringType::Before))
				throw Exception(ErrorCode::InvUsageAfterBeforeViewTrigger,PGM_FUNC,PGM_FILE,PGM_LINE);

			//Only constraint triggers can be deferrable or reference another table
			if(referenced_table || is_deferrable)
				throw Exception(ErrorCode::InvUseConstraintTriggerAttribs,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
		//Constraint triggers can only be executed on AFTER events and for each row
		else
		{
			if(firing_type!=FiringType::After && !is_exec_per_row)
				throw Exception(ErrorCode::InvConstrTriggerNotAfterRow,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}
}

bool Trigger::acceptsReplaceCommand()
{
	// Constraint triggers don't accept CREATE OR REPLACE command
	return !is_constraint;
}

QString Trigger::getDataDictionary(bool md_format, const attribs_map &extra_attribs)
{
	try
	{
		attribs_map attribs;
		QStringList aux_list;
		std::vector<EventType> events = { EventType::OnInsert, EventType::OnDelete,
																			EventType::OnTruncate, EventType::OnUpdate };

		attribs.insert(extra_attribs.begin(), extra_attribs.end());
		attribs[Attributes::Name] = obj_name;
		attribs[Attributes::Comment] = comment;
		attribs[Attributes::RefTable] = referenced_table ? referenced_table->getSignature().remove('"') : "";
		attribs[Attributes::Function] = function ? function->getSignature() : "";
		attribs[Attributes::FiringType] = ~firing_type;
		attribs[Attributes::Condition] = condition;
		attribs[Attributes::PerRow] = is_exec_per_row ? CoreUtilsNs::DataDictCheckMark : "";

		if(is_constraint)
			aux_list.append(Attributes::Constraint.toUpper());

		aux_list.clear();
		if(is_deferrable)
			aux_list.append(Attributes::Deferrable.toUpper() + QString(" (%1)").arg(~deferral_type));
		else
			aux_list.append("NOT " + Attributes::Deferrable.toUpper());

		attribs[Attributes::Attributes] = aux_list.join(", ");

		aux_list.clear();
		for(auto &event : events)
		{
			if(!isExecuteOnEvent(event))
				continue;

			aux_list.append(~event);
		}

		attribs[Attributes::Events] = aux_list.join(", ");

		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, getSchemaName()), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Trigger::updateDependencies()
{
	std::vector<BaseObject *> deps = { function, referenced_table };

	for(auto &col : upd_columns)
		deps.push_back(col);

	TableObject::updateDependencies(deps);
}

void Trigger::generateHashCode()
{
	QString str_events;
	TableObject::generateHashCode();

	for(auto &itr : events)
		str_events.append(QString::number(isExecuteOnEvent(itr.first)));

	hash_code = UtilsNs::getStringHash(hash_code + ~firing_type + str_events);
}
