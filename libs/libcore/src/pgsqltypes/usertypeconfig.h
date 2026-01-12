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
\ingroup libcore/pgsqltypes
\struct UserTypeConfig
\brief Implements the operations to manipulate user-defined data type configurations (used by PgSqlType).
**/

#ifndef USER_TYPE_CONFIG
#define USER_TYPE_CONFIG

#include "../coreglobal.h"
#include <QString>

class BaseObject;

/*! \brief This class stores the user defined type configureation.
	 When the user creates a Type, Sequence, Domain, even a Table,
	 it can be used as a type on certain configurations so this
	 class implements a basic structure to control these types */
struct __libcore UserTypeConfig {
	public:
		enum TypeConf: unsigned {
			BaseType=1, //! \brief The type refers to a user-defined base type (class Type)
			DomainType=2, //! \brief The type refers to a domain
			TableType=4, //! \brief The type refers to a table
			SequenceType=8, //! \brief The type refers to a sequence
			ViewType=16, //! \brief The type refers to a view
			ForeignTableType=32, //! \brief The type refers to a extension used as datatype

			//! \brief This constant refers to all types above and must be used only on type searches
			AllUserTypes=63
		};

		UserTypeConfig()
		{
			name = "";
			ptype = nullptr;
			pmodel = nullptr;
			invalidated = false;
			type_conf = BaseType;
		}

	protected:
		//! \brief Pointer to the instance of the user defined type
		BaseObject *ptype;

		//! \brief Pointer to the model that the type belongs to
		BaseObject *pmodel;

		//! \brief Name of the type
		QString name;

		//! \brief Type configuration id (refer to ???_TYPE constants)
		TypeConf type_conf;

		/*! \brief Indicates that the type is invalidated after being removed from database model.
		This wll cause the type not to be listed or referenced. Actually the type is not removed from
		user type list but deactivated to avoid messing around with other user types */
		bool invalidated;

		friend class PgSqlType;
};

#endif 
