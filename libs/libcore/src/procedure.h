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
\brief Implements the operations to manipulate procedures on the database.
*/

#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "basefunction.h"

class __libcore Procedure: public BaseFunction {
	public:
		Procedure();

		virtual ~Procedure(){}

		virtual QString getSourceCode(SchemaParser::CodeType def_type, bool) final;
		virtual QString getSourceCode(SchemaParser::CodeType def_type) final;
		virtual QString getAlterCode(BaseObject *object) final;

		virtual void updateDependencies() override;
};

#endif
