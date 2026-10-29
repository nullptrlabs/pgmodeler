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
\class OperatorFamily
\brief Implements the operations to manipulate operator family on the database.
\note <strong>Creation date:</strong> 08/07/2008
*/

#ifndef OPERATOR_FAMILY_H
#define OPERATOR_FAMILY_H

#include "baseobject.h"
#include "pgsqltypes/indexingtype.h"

class __libcore OperatorFamily: public BaseObject {
	private:
		//! \brief Indexing type used by the operator family
		IndexingType indexing_type;

	public:
		OperatorFamily();
		~OperatorFamily() override = default;

		//! \brief Sets the indexing type of the operator family
		void setIndexingType(IndexingType idx_type);

		//! \brief Returns the indexing type of the operator family
		IndexingType getIndexingType();

		//! \brief Returns the SQL / XML code definition for the operator family
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getSignature(bool format=true) final;
};

#endif
