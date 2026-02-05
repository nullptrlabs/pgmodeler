/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
		static constexpr unsigned Year = 1,
		Month = 2,
		Day = 3,
		Hour= 4,
		Minute = 5,
		Second = 6,
		YearToMonth = 7,
		DayToHour = 8,
		DayToMinute = 9,
		DayToSecond = 10,
		HourToMinute = 11,
		HourToSecond = 12,
		MinuteToSecond = 13;

		IntervalType(const QString &type_name);
		IntervalType(unsigned type_id);
		IntervalType();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id) override;
};

#endif 
