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
\class DeferralType
\brief Implements the operations to manipulate constraints defferal modes/types.
**/

#ifndef DEFERRAL_TYPE
#define DEFERRAL_TYPE

#include "templatetype.h"

class __libcore DeferralType: public TemplateType<DeferralType>{
	private:
		static const QStringList type_names;

	public:
		static constexpr unsigned Immediate = 1;
		static constexpr unsigned Deferred = 2;

		DeferralType(unsigned type_id);
		DeferralType(const QString &type_name);
		DeferralType();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id ) override;
};

#endif
