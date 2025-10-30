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
		~UserMapping() override = default;

		void setForeignServer(ForeignServer *server);
		ForeignServer *getForeignServer();

		//! \brief This method has a hardcoded way to generated the usermapping's name. It'll reject any value passed by its parameter
		void setName(const QString &) override;

		void setOwner(BaseObject *role) override;
		QString getName(bool = false, bool = false) override;
		QString getSignature(bool = false) override;
		QString getSourceCode(SchemaParser::CodeType def_type) override;
		QString getAlterCode(BaseObject *object) override;
		QString getDropCode(bool) override;

		void updateDependencies() override;
};

#endif
