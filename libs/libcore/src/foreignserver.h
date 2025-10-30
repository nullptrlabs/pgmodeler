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
\brief Implements the operations to manipulate foreign servers on the database.
*/

#ifndef FOREIGN_SERVER_H
#define FOREIGN_SERVER_H

#include "baseobject.h"
#include "foreigndatawrapper.h"

class __libcore ForeignServer: public BaseObject, public ForeignObject {
	private:
		//! \brief The foreign data wrapper which manages the server
		ForeignDataWrapper *fdata_wrapper;

		QString type, version;

	public:
		ForeignServer();
		~ForeignServer() override = default;

		void setType(const QString &type);
		void setVersion(const QString &version);
		void setForeignDataWrapper(ForeignDataWrapper *fdw);

		QString getType();
		QString getVersion();
		ForeignDataWrapper *getForeignDataWrapper();

		QString getSourceCode(SchemaParser::CodeType def_type) override;
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) override;
		QString getAlterCode(BaseObject *object) override;

		void updateDependencies() override;
};

#endif
