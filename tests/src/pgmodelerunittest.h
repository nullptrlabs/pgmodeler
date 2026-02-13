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
\ingroup tests
\namespace PgModelerUnitTest
\brief This class is used to initialize the tests executables search path via GlobalAttributes.
So if a unit test requires the access to the schema files or any other file that lies in the pgModeler's GUI app
they must inherit from this class.
*/

#ifndef PGMODELER_UNIT_TEST_H
#define PGMODELER_UNIT_TEST_H

#include "globalattributes.h"

class PgModelerUnitTest {
	public:
		PgModelerUnitTest(const QString &search_path){
			GlobalAttributes::init(search_path, false);
	}
};

#endif
