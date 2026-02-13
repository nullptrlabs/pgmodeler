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
\ingroup libcore
\class Reference
\brief Implements the operations to manipulate GenericSQL object references.
*/

#ifndef REFERENCE_H
#define REFERENCE_H

#include "baseobject.h"

class __libcore Reference {
	// Name of the reference (in SQL it be used between {} in order to be parsed)
	QString ref_name,

			// The alias of the reference (in SQL it be used between @{} in order to be parsed)
			ref_alias;

 // The object being referenced
	BaseObject *object;

	 // Indicates that the signature of the object must be used instead of the name
	bool use_signature,

				 // Indicates that the columns of the object (for table-like objects) must be used as view's columns
			use_columns,

				 // Indicates that the name of the object need to be automatically quoted or the schema name appended
			format_name;

public:
	Reference();

	Reference(BaseObject *_object, const QString &_ref_name, const QString &_ref_alias,
						 bool _use_signature, bool _format_name, bool _use_columns);

	BaseObject *getObject() const;
	QString getRefName() const;
	QString getRefAlias() const;
	bool isUseSignature() const;
	bool isFormatName() const;
	bool isUseColumns() const;
	bool isValid() const;
	QString getXmlCode() const;
};

#endif
