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
\class EventTrigger
\brief Implements the operations to manipulate event triggers on the database.
*/

#ifndef EVENT_TRIGGER_H
#define EVENT_TRIGGER_H

#include "baseobject.h"
#include "function.h"
#include "pgsqltypes/eventtriggertype.h"

class __libcore EventTrigger: public BaseObject {
	private:

		//! \brief Function that is excuted when the event trigger is activated
		Function *function;

		//! \brief The event that fires the trigger
		EventTriggerType event;

		/*! \brief This map contains the values used as condition on WHEN clause.
				Currently the only supported variable is TAG, and the object will raise
				error is other variable name is used on setFilter() call */
		std::map<QString, QStringList> filter;

	public:
		EventTrigger();
		~EventTrigger() override = default;

		void setEvent(EventTriggerType evnt_type);
		void setFunction(Function *func);
		void setFilter(const QString &variable, const QStringList &values);
		void setFilter(const QString &variable, const QString &value);

		void removeFilter(const QString &variable);
		void clearFilter();

		EventTriggerType getEvent();
		Function *getFunction();
		QStringList getFilter(const QString &variable);

		QString getSourceCode(SchemaParser::CodeType def_type) final;

		void updateDependencies() override;
};

#endif
