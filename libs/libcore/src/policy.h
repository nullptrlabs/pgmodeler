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
\class Policy
\brief Implements the operations to manipulate row level security (RLS).
*/

#ifndef POLICY_H
#define POLICY_H

#include "tableobject.h"
#include "role.h"
#include "pgsqltypes/policycmdtype.h"

class __libcore Policy : public TableObject {
	private:
		/*! \brief Roles that has permissions over the object. This vector can be
			empty indicating that all roles on the cluster has permission over
			the object. */
		std::vector<Role *> roles;

		//! \brief Defines the USING expression applied to queries referencing the table which policy is applied
		QString using_expr,

		//! \brief Defines the CHECK expression applied to INSERT/UPDATE queries
		check_expr;

		/*! \brief Indicates if the policy is permissive. Permissive policies are combined using OR operator by PostgreSQL
		 and reducing the set of records accessed by a query. When the policy is restrictive the policies are combined using AND.
		By default, a policy is permissive. */
		bool permissive;

		PolicyCmdType policy_cmd;

	public:
		Policy();
		~Policy() override = default;

		void setParentTable(BaseTable *table) final;

		//! \brief Defines if the the policy is permissive or restrictive
		void setPermissive(bool value);

		//! \brief Returns if the policy is permissive or restrictive
		bool isPermissive();

		//! \brief Defines the command affected by the policy.
		void setPolicyCommand(PolicyCmdType cmd);

		//! \brief Returns the policy affected command
		PolicyCmdType getPolicyCommand();

		//! \brief Defines the USING expresion of the policy
		void setUsingExpression(const QString &expr);

		//! \brief Returns the USING expresion of the policy
		QString getUsingExpression();

		//! \brief Defines the CHECK expresion of the policy
		void setCheckExpression(const QString &expr);

		//! \brief Returns the CHECK expresion of the policy
		QString getCheckExpression();

		//! \brief Adds a role that will have privileges over the object
		void addRole(Role *role);

		//! \brief Remove all roles from the policy
		void removeRoles();

		//! \brief Returns all the roles that is used by the policy
		std::vector<Role *> getRoles();

		//! \brief Returns the SQL / XML definition for the policy
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getAlterCode(BaseObject *object) final;

		bool isRoleExists(Role *role);

		void updateDependencies() override;

		void generateHashCode() override;
};

#endif
