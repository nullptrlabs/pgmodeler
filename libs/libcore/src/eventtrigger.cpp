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

#include "eventtrigger.h"
#include "defaultlanguages.h"

EventTrigger::EventTrigger()
{
	obj_type=ObjectType::EventTrigger;
	function=nullptr;
	attributes[Attributes::Event]="";
	attributes[Attributes::Filter]="";
	attributes[Attributes::Function]="";
}

void EventTrigger::setEvent(EventTriggerType evnt_type)
{
	setCodeInvalidated(event != evnt_type);
	this->event=evnt_type;
}

void EventTrigger::setFunction(Function *func)
{
	if(!func)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedFunction)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::EventTrigger)),
										ErrorCode::AsgNotAllocatedFunction, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
	//Functions with return type other that event_trigger are not accepted
	if(func->getReturnType()!="event_trigger")
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidTriggerFunction)
										.arg("event_trigger"), PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	//Functions with one or more parameters are not accepted
	if(func->getParameterCount()!=0)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgFunctionInvalidParamCount)
										.arg(this->getName())
										.arg(BaseObject::getTypeName(ObjectType::EventTrigger)),
										ErrorCode::AsgFunctionInvalidParamCount,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	//Functions coded in SQL lang. is not accepted by event triggers
	if(func->getLanguage()->getName().toLower() == DefaultLanguages::Sql)
		throw Exception(ErrorCode::AsgEventTriggerFuncInvalidLang,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(function != func);
	function=func;
}

void EventTrigger::setFilter(const QString &variable, const QStringList &values)
{
	if(variable.toLower()!=Attributes::Tag)
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidEventTriggerVariable).arg(variable),PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!values.isEmpty())
	{
		filter[variable].append(values);
		setCodeInvalidated(true);
	}
}

void EventTrigger::setFilter(const QString &variable, const QString &value)
{
	setFilter(variable, QStringList{ value });
}

void EventTrigger::removeFilter(const QString &variable)
{
	filter.erase(variable);
	setCodeInvalidated(true);
}

void EventTrigger::clearFilter()
{
	filter.clear();
	setCodeInvalidated(true);
}

EventTriggerType EventTrigger::getEvent()
{
	return event;
}

Function *EventTrigger::getFunction()
{
	return function;
}

QStringList EventTrigger::getFilter(const QString &variable)
{
	if(filter.count(variable))
		return filter.at(variable);

	return {};
}

QString EventTrigger::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def = getCachedCode(def_type, false);

	if(!code_def.isEmpty())
		return code_def;

	attributes[Attributes::Event] = ~event;

	if(def_type == SchemaParser::SqlCode)
	{
		QStringList str_list;

		if(function)
			attributes[Attributes::Function]=function->getSignature();

		for(auto &flt : filter)
			str_list.push_back(QString("%1 IN ('%2')").arg(flt.first).arg(flt.second.join("','")));

		attributes[Attributes::Filter]=str_list.join("\n\t AND ");
	}
	else
	{
		if(function)
			attributes[Attributes::Function]=function->getSourceCode(def_type, true);

		for(auto &flt : filter)
		{
			//Creating an element <filter variable="" values=""/>
			attributes[Attributes::Filter]+=QString("\t<%1 %2=\"%3\" %4=\"%5\"/>\n")
												   .arg(Attributes::Filter)
												   .arg(Attributes::Variable).arg(flt.first)
												   .arg(Attributes::Values).arg(flt.second.join(','));
		}
	}

	return BaseObject::__getSourceCode(def_type);
}

void EventTrigger::updateDependencies()
{
	BaseObject::updateDependencies({ function });
}
