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
\ingroup libcore/pgsqltypes
\class IntervalType
\brief Implements the operations to manipulate time interval types.
**/

#ifndef INTERVAL_TYPE
#define INTERVAL_TYPE

#include "templatetype.h"

class __libcore IntervalType: public TemplateType<IntervalType>{
	private:
		static const QStringList type_names;

	public:
		static constexpr unsigned Year = 1;
		static constexpr unsigned Month = 2;
		static constexpr unsigned Day = 3;
		static constexpr unsigned Hour= 4;
		static constexpr unsigned Minute = 5;
		static constexpr unsigned Second = 6;
		static constexpr unsigned YearToMonth = 7;
		static constexpr unsigned DayToHour = 8;
		static constexpr unsigned DayToMinute = 9;
		static constexpr unsigned DayToSecond = 10;
		static constexpr unsigned HourToMinute = 11;
		static constexpr unsigned HourToSecond = 12;
		static constexpr unsigned MinuteToSecond = 13;

		IntervalType(const QString &type_name);
		IntervalType(unsigned type_id);
		IntervalType();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id) override;
};

#endif 
