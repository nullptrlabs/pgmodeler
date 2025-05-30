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
\class CategoryType
\brief Implements the operations to manipulate base types categories.
**/

#ifndef CATEGORY_TYPE
#define CATEGORY_TYPE

#include "templatetype.h"

class __libcore CategoryType: public TemplateType<CategoryType>{
	private:
		static const QStringList type_names;

	public:
		static constexpr unsigned UserDefined = 1;
		static constexpr unsigned Array = 2;
		static constexpr unsigned Boolean = 3;
		static constexpr unsigned Composite = 4;
		static constexpr unsigned DateTime = 5;
		static constexpr unsigned Enumeration = 6;
		static constexpr unsigned Geometric = 7;
		static constexpr unsigned NetworkAddr = 8;
		static constexpr unsigned Numeric = 9;
		static constexpr unsigned PseudoTypes = 10;
		static constexpr unsigned Stringt = 11;
		static constexpr unsigned Timespan = 12;
		static constexpr unsigned BitString = 13;
		static constexpr unsigned Unknown = 14;

		CategoryType(unsigned type_id);
		CategoryType(const QString &type_name);
		CategoryType();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id) override;
};

#endif
