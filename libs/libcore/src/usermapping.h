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
\class Server
\brief Implements the operations to manipulate user mappings on the database.
*/

#ifndef USER_MAPPING_H
#define USER_MAPPING_H

#include "baseobject.h"
#include "foreignserver.h"

class __libcore UserMapping: public BaseObject, public ForeignObject {
	private:
		//! \brief The foreign server which is managed by this user mapping the server
		ForeignServer *foreign_server;

	public:
		UserMapping();

		virtual ~UserMapping(){}

		void setForeignServer(ForeignServer *server);
		ForeignServer *getForeignServer();

		//! \brief This method has a hardcoded way to generated the usermapping's name. It'll reject any value passed by its parameter
		virtual void setName(const QString &) override;

		virtual void setOwner(BaseObject *role) override;
		virtual QString getName(bool = false, bool = false) override;
		virtual QString getSignature(bool = false)  override;
		virtual QString getSourceCode(SchemaParser::CodeType def_type)  override;
		virtual QString getAlterCode(BaseObject *object)  override;
		virtual QString getDropCode(bool) override;

		virtual void updateDependencies() override;

};

#endif
