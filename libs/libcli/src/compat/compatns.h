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

#ifndef COMPATNS_H
#define COMPATNS_H

#include "view.h"
#include "xmlparser.h"
#include "databasemodel.h"

namespace CompatNs {
	//! \brief Create a view object taking as input the legacy XML code for views
	extern View *createLegacyView(const QString &xml_code, DatabaseModel *dbmodel);

	//! \brief Converts a legacy view object to the new one introduced by pgModeler 1.1.0-beta
	extern QString convertToNewView(View *old_view);

	/*! \brief Auxiliary method used by createLegacyView to fill basic attributes of the object
	 *  by reading values from the XML code */
	extern void setBasicAttributes(BaseObject *object, DatabaseModel *dbmodel);
}

#endif
