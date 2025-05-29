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
\class EventType
\brief Implements the operations to manipulate trigger events.
**/

#ifndef EVENT_TYPE
#define EVENT_TYPE

#include "templatetype.h"

class __libcore EventType: public TemplateType<EventType>{
	private:
		static const QStringList type_names;

	public:
		static constexpr unsigned OnSelect = 1;
		static constexpr unsigned OnInsert = 2;
		static constexpr unsigned OnDelete = 3;
		static constexpr unsigned OnUpdate = 4;
		static constexpr unsigned OnTruncate = 5;

		EventType(const QString &type_name);
		EventType(unsigned type_id);
		EventType();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id) override;

		/*! \brief These two operators were created to permit the use of the
		 class EventType on STL containers (specially maps) */
		bool operator < (EventType type) const;
		bool operator < (unsigned type_id) const;
};

#endif 
