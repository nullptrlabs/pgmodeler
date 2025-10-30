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
\class Role
\brief Implements the operations to manipulate roles on the database.
\note <strong>Creation date:</strong> 12/05/2008
*/

#ifndef ROLE_H
#define ROLE_H

#include "baseobject.h"

class __libcore Role: public BaseObject {
	private:
		static unsigned role_id;

		/*! \brief Options for the role (SUPERUSER, CREATEDB, CREATEROLE,
		 INHERIT, LOGIN, REPLICATION, BYPASSRLS) */
		bool options[7];

		//! \brief Connection limit for the role
		int conn_limit;

		//! \brief Validity date for the role
		QString validity,

		//! \brief Authentication password
		password;

		//! \brief Member roles of 'this' role
		std::vector<Role *>	member_roles,

		//! \brief Member roles of 'this' role with admin privileges
		admin_roles;

		//! \brief Formats the role attribute to be used by the SchemaParser
		void setRoleAttribute(unsigned role_type);

		std::vector<Role *> *getRoleList(unsigned role_type);

		QString getAlterMembershipCommands(Role *imp_role, Role *ref_role, bool revoke);

	public:
		//! \brief Constants used to reference the available options for the role
		enum RoleOpts: unsigned {
			OpSuperuser,
			OpCreateDb,
			OpCreateRole,
			OpInherit,
			OpLogin,
			OpReplication,
			OpBypassRls
		};

		//! \brief Constants used to reference the internal role lists of the class
		enum RoleType: unsigned {
			MemberRole,
			AdminRole
		};

		Role();
		~Role() override = default;

		//! \brief Sets one option for the role (Via OP_??? constants)
		void setOption(RoleOpts op_type, bool value);

		//! \brief Adds one role to the internal role list (Via ???_ROLE constants)
		void addRole(RoleType role_type, Role *role);

		//! \brief Defines the connection limit for the role
		void setConnectionLimit(int limit);

		//! \brief Defines the validity date for the role
		void setValidity(const QString &date);

		//! \brief Sets the password for the role
		void setPassword(const QString &passwd);

		//! \brief Gets on option for the role (Via OP_??? constants)
		bool getOption(RoleOpts op_type);

		//! \brief Remove one role from internal role list (Via ???_ROLE constants)
		void removeRole(RoleType role_type, unsigned role_idx);

		//! \brief Remove all roles from one iternal list (Via ???_ROLE constants)
		void removeRoles(RoleType role_type);

		/*! \brief Gets one role from internal list (Via ???_ROLE constants) referencing
		 the object by its index */
		Role *getRole(RoleType role_type, unsigned role_idx);

		//! \brief Returns whether the role exists in the internal lists (Via ???_ROLE constants)
		bool isRoleExists(RoleType role_type, Role *role);

		//! \brief Returns whether the role name exists in the internal lists (Via ???_ROLE constants)
		bool isRoleExists(RoleType role_type, const QString &rl_name);

		//! \brief Gets the role count on the specified internal list (Via ???_ROLE constants)
		unsigned getRoleCount(RoleType role_type);

		//! \brief Returns the connection limit for the role
		unsigned getConnectionLimit();

		//! \brief Returns the validity date for the role
		QString getValidity();

		//! \brief Returns the role password
		QString getPassword();

		//! \brief Returns the SQL / XML definition for the role
		QString getSourceCode(SchemaParser::CodeType def_type) final;
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;

		QString getAlterCode(BaseObject *object) final;

		void updateDependencies() override;
};

#endif
