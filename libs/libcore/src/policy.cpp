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
#include "policy.h"
#include "utilsns.h"

Policy::Policy()
{
	obj_type=ObjectType::Policy;
	permissive = false;
	policy_cmd = PolicyCmdType::All;

	attributes[Attributes::Permissive] = "";
	attributes[Attributes::Command] = "";
	attributes[Attributes::UsingExp] = "";
	attributes[Attributes::CheckExp] = "";
	attributes[Attributes::Roles] = "";
}

void Policy::setParentTable(BaseTable *table)
{
	if(table && table->getObjectType() != ObjectType::Table)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidObjectType)
						.arg(this->obj_name)
						.arg(this->getTypeName())
						.arg(BaseObject::getTypeName(ObjectType::Table)),
						ErrorCode::AsgInvalidObjectType,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	TableObject::setParentTable(table);
}

void Policy::setPermissive(bool value)
{
	setCodeInvalidated(permissive != value);
	permissive = value;
}

bool Policy::isPermissive()
{
	return permissive;
}

void Policy::setPolicyCommand(PolicyCmdType cmd)
{
	setCodeInvalidated(policy_cmd != cmd);
	policy_cmd = cmd;
}

PolicyCmdType Policy::getPolicyCommand()
{
	return policy_cmd;
}

void Policy::setUsingExpression(const QString &expr)
{
	setCodeInvalidated(using_expr != expr);
	using_expr = expr;
}

QString Policy::getUsingExpression()
{
	return using_expr;
}

void Policy::setCheckExpression(const QString &expr)
{
	setCodeInvalidated(check_expr != expr);
	check_expr = expr;
}

QString Policy::getCheckExpression()
{
	return check_expr;
}

void Policy::addRole(Role *role)
{
	if(!role)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(std::find(roles.begin(), roles.end(), role) == roles.end())
		roles.push_back(role);
}

void Policy::removeRoles()
{
	roles.clear();
	setCodeInvalidated(true);
}

std::vector<Role *> Policy::getRoles()
{
	return roles;
}

QString Policy::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	QStringList rol_names;

	if(getParentTable())
		attributes[Attributes::Table]=getParentTable()->getName(true);

	attributes[Attributes::Command] = ~policy_cmd;

	for(auto role : roles)
		rol_names.append(role->getName(true));

	attributes[Attributes::Permissive] = (permissive ? Attributes::True : "");
	attributes[Attributes::UsingExp] = using_expr;
	attributes[Attributes::CheckExp] = check_expr;
	attributes[Attributes::Roles] = rol_names.join(", ");

	return BaseObject::__getSourceCode(def_type);
}

QString Policy::getAlterCode(BaseObject *object)
{
	Policy *policy=dynamic_cast<Policy *>(object);

	if(!policy)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		QStringList rol_names, aux_rol_names;
		attribs_map attribs;

		attributes[Attributes::AlterCmds] = TableObject::getAlterCode(object);

		if(this->using_expr.simplified() != policy->using_expr.simplified())
			attribs[Attributes::UsingExp] = policy->using_expr;

		if(this->check_expr.simplified() != policy->check_expr.simplified())
			attribs[Attributes::CheckExp] = policy->check_expr;

		for(auto role : this->roles)
			rol_names.append(role->getName(true));

		for(auto role : policy->roles)
			aux_rol_names.append(role->getName(true));

		rol_names.sort();
		aux_rol_names.sort();

		if(!rol_names.isEmpty() && aux_rol_names.isEmpty())
			attribs[Attributes::Roles] = Attributes::Unset;
		else if(rol_names.join(", ") != aux_rol_names.join(", "))
			attribs[Attributes::Roles] = aux_rol_names.join(", ");

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

bool Policy::isRoleExists(Role *role)
{
	if(!role)	return false;
	return (std::find(roles.begin(), roles.end(), role) != roles.end());
}

void Policy::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &rl : roles)
		deps.push_back(rl);

	TableObject::updateDependencies(deps);
}

void Policy::generateHashCode()
{
	TableObject::generateHashCode();
	hash_code = UtilsNs::getStringHash(hash_code + QString::number(permissive) + ~policy_cmd);
}
