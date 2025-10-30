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
\ingroup libutils
\namespace PgSQLVersions
\brief Definition of PgSQLVersions namespace wich stores a series of static strings constants used
 to reference the supported PostgreSQL version.
*/

#ifndef PGSQL_VERSIONS_H
#define PGSQL_VERSIONS_H

#include "utilsglobal.h"
#include <QStringList>

namespace PgSqlVersions {
	extern __libutils const QString
	PgSqlVersion100,
	PgSqlVersion110,
	PgSqlVersion120,
	PgSqlVersion130,
	PgSqlVersion140,
	PgSqlVersion150,
	PgSqlVersion160,
	PgSqlVersion170,
	PgSqlVersion180,
	MinimumVersion,
	DefaulVersion;

	extern __libutils const QStringList AllVersions;

	/*! \brief Parses and checks a string contaning a PostgreSQL version in the form [major.minor]
	 * When the flag ignore_legacy_ver is true, this method will raise an error if an invalid version
	 * is provided. An invalid version is considered any string in incorrect format or a valid string
	 * but refers to an obsolete version (< 10.0). If an invalid major version is proveded then the function
	 * will return the DefaultVersion as a fallback. */
	extern __libutils QString parseString(const QString &pgsql_ver, bool ignore_legacy_ver);
}

#endif
