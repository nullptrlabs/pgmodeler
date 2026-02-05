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
\class SimpleColumn
\brief This class acts like a rudimentary column used by Reference and Views to represent columns
			 Note that, as intended, the class expects that all received values are validated since they are immutable
*/

#ifndef SIMPLE_COLUMN_H
#define SIMPLE_COLUMN_H

#include "coreglobal.h"
#include <QString>
#include <qmetatype.h>

class __libcore SimpleColumn {
	private:
		QString name, type, alias;

	public:
		SimpleColumn();

		SimpleColumn(const SimpleColumn &col);

		SimpleColumn(const QString &_name, const QString &_type, const QString &_alias);

		QString getName() const;
		QString getType() const;
		QString getAlias() const;

		bool operator == (const SimpleColumn &col) const;
		SimpleColumn & operator = (const SimpleColumn &col);

		bool isValid() const;
		QString getXmlCode() const;
};

/* Registering the SimpleColumn class as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(SimpleColumn)

#endif
