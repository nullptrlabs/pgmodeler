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
\class ForeignObject
\brief Implements the basic operations to handle common attributes of foreign data wrapper, foreign servers and foreign tables.
*/

#ifndef FOREIGN_OBJECT_H
#define FOREIGN_OBJECT_H

#include "coreglobal.h"
#include "attribsmap.h"

class __libcore ForeignObject {
	protected:
		//! \brief A set of key/value options associated to the foreign object
		attribs_map options;

		QString getOptionsAttribute(unsigned def_type);

	public:
		//! \brief Store the character used to separate options/values in the XML code
		static const QString OptionsSeparator,
		OptionValueSeparator;

		ForeignObject();
		~ForeignObject() = default;

		void setOption(const QString &opt, const QString &value);
		void setOptions(const attribs_map &options);
		void removeOption(const QString &opt);
		void removeOptions();
		attribs_map getOptions();

		void getAlteredAttributes(ForeignObject *fobj, attribs_map &fo_attribs);
};

#endif
