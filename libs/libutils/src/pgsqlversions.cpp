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

#include "pgsqlversions.h"
#include "exception.h"

namespace PgSqlVersions {
	const QString
	PgSqlVersion100("10.0"),
	PgSqlVersion110("11.0"),
	PgSqlVersion120("12.0"),
	PgSqlVersion130("13.0"),
	PgSqlVersion140("14.0"),
	PgSqlVersion150("15.0"),
	PgSqlVersion160("16.0"),
	PgSqlVersion170("17.0"),
	PgSqlVersion180("18.0"),
	DefaulVersion = PgSqlVersion180,
	MinimumVersion = PgSqlVersion100;

	const QStringList
	AllVersions = {
		PgSqlVersion180, PgSqlVersion170, PgSqlVersion160,
		PgSqlVersion150, PgSqlVersion140, PgSqlVersion130,
		PgSqlVersion120, PgSqlVersion110, PgSqlVersion100
	};

	QString parseString(const QString &pgsql_ver, bool ignore_legacy_ver)
	{
		unsigned curr_ver = QString(pgsql_ver).remove('.').toUInt(),
				min_ver = QString(MinimumVersion).remove('.').toUInt(),
				default_ver = QString(DefaulVersion).remove('.').toUInt();

		if(!ignore_legacy_ver && curr_ver != 0 && (curr_ver < min_ver))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvPostgreSQLVersion)
											.arg(pgsql_ver, PgSqlVersions::MinimumVersion, PgSqlVersions::DefaulVersion),
											ErrorCode::InvPostgreSQLVersion, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		if(curr_ver > 0)
		{
			// If valid version just returns the version
			if(curr_ver <= default_ver)
				return pgsql_ver;

			/* If it's a new version greater than the default
			 * we fallback to the default one */
			if(curr_ver > default_ver)
				return DefaulVersion;
		}

		return PgSqlVersions::MinimumVersion;
	}
}
