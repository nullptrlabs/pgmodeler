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
\class Permission
\brief Implements the operations to manipulate object permissions.
\note <strong>Creation date:</strong> 16/09/2010
*/

#ifndef PERMISSION_H
#define PERMISSION_H

#include "baseobject.h"
#include "role.h"
#include <algorithm>
#include <QTextStream>
#include <QRegularExpression>

class __libcore Permission: public BaseObject {
		/*! \brief Permissions on PostgreSQL are only applied to the following
				object type:

				* table
				* column
				* view
				* sequence
				* database
				* foreign-data wrapper
				* foreign server
				* large objects (not implemented)
				* function
				* aggregate
				* linguage
				* schema
				* tablespace */
	private:
		static const QString priv_codes;

		//! \brief Object which the permission is applied
		BaseObject *object;

		/*! \brief Roles that has permissions over the object. This vector can be
			empty indicating that all roles on the cluster has permission over
			the object. */
		std::vector<Role *> roles;

		//! \brief Privileges set applied to the object (Accessed via constants PRIV_???)
		bool privileges[13],

		//! \brief Indicates if the permission treats a REVOKE command.
		revoke,

		//! \brief Indicates if the revoke is applied in a cascade way.
		cascade;

		/*! \brief Indicates whether a privilege with given index can be assigned to other roles
			over the same object (WITH GRANT OPTION). This attribute is not applicable
			when there is no specified role (PUBLIC). This attribute is ignored
			when there is no defined role as holder of the privilege. */
		bool grant_option[13];

		/*! \brief Generates a unique identifier for permission using the attribute
			'name' of base class BaseObject. This is only used to avoid
			 duplicate permissions in the model */
		void generatePermissionId();

	public:
		//! \brief Constants used to reference the privileges
		enum PrivilegeId: unsigned {
			PrivSelect,
			PrivInsert,
			PrivUpdate,
			PrivDelete,
			PrivTruncate,
			PrivReferences,
			PrivTrigger,
			PrivCreate,
			PrivConnect,
			PrivTemporary,
			PrivExecute,
			PrivUsage
		};

		/*! \brief In the constructor is required to specify which object will receive
		 the permissions this can not be changed after the object instance of
		 the class is created. */
		Permission(BaseObject *obj);
		~Permission() override = default;

		//! \brief Adds a role that will have privileges over the object
		void addRole(Role *role);

		//! \brief Sets the state of one permission's privilege (Accessed via constants PRIV_???)
		void setPrivilege(PrivilegeId priv_id, bool value, bool grant_op);

		void setRevoke(bool value);

		void setCascade(bool value);

		//! \brief Remove a role using its index
		void removeRole(unsigned role_idx);

		//! \brief Remove all roles from the permission
		void removeRoles();

		//! \brief Gets the role count associated to the permission
		unsigned getRoleCount();

		//! \brief Gets one role from permission using its index
		Role *getRole(unsigned role_idx);

		//! \brief Returns all the roles that is used by the permission
		std::vector<Role *> getRoles();

		//! \brief Gets the object that is subject to the privileges
		BaseObject *getObject();

		//! \brief Gets the actual state of the GRANT OPTION for the given privilege
		bool getGrantOption(PrivilegeId priv_id);

		//! \brief Gets the current state for the given privilege
		bool getPrivilege(PrivilegeId priv_id);

		/*! \brief Returns a string containing all the privileges
		 configured as the internal format of permissions
		 as documented on PostgreSQL GRANT command */
		QString getPermissionString();

		/*! \brief Parses the permission string (e.g. postgres=arwdDxt/postgres) and returns the role name
		which owns the permission. The parameter vectors stores the ordinary privileges as well the GRANT OPTION privileges */
		static QString parsePermissionString(QString perm_str, std::vector<PrivilegeId> &privs, std::vector<PrivilegeId> &gop_privs);

		//! \brief Indicates whether the role is present on the permission
		bool isRoleExists(Role *role);
		bool isRevoke();
		bool isCascade();

		//! \brief Returns the the specified permission is semantically the same as this permission
		bool isSimilarTo(Permission *perm);

		//! \brief Returns if the passed object type accepts permission
		static bool acceptsPermission(ObjectType obj_type, int privilege=-1);

		//! \brief Returns the SQL / XML definition for the permission
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getSignature(bool = false) final;

		QString getDropCode(bool cascade) final;

		void updateDependencies() override;
};

#endif
