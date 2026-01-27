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
\namespace GlobalAttributes
\brief Definition of DefaultLanguages namespace which stores the default language names on a PostgreSQL installation.
*/

#ifndef DEFAULT_LANGS_H
#define DEFAULT_LANGS_H

#include <QString>

namespace DefaultLanguages {
	inline const QString Sql ("sql"),
	C("c"),
	PlPgsql("plpgsql"),
	PlTcl("pltcl"),
	PlPerl("plperl"),
	PlPythonU("plpythonu"),
	Internal("internal");
}

#endif
