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

#include "role.h"

unsigned Role::role_id {0};

Role::Role()
{
	obj_type=ObjectType::Role;
	object_id=Role::role_id++;

	for(unsigned i=0; i <= OpBypassRls; i++)
		options[i]=false;

	conn_limit=-1;

	attributes[Attributes::Superuser]="";
	attributes[Attributes::CreateDb]="";
	attributes[Attributes::CreateRole]="";
	attributes[Attributes::Inherit]="";
	attributes[Attributes::Login]="";
	attributes[Attributes::ConnLimit]="";
	attributes[Attributes::Password]="";
	attributes[Attributes::Validity]="";
	attributes[Attributes::MemberRoles]="";
	attributes[Attributes::AdminRoles]="";
	attributes[Attributes::Replication]="";
	attributes[Attributes::Group]="";
	attributes[Attributes::BypassRls]="";
	attributes[Attributes::EmptyPassword]="";
	attributes[Attributes::HasOptions]="";
}

void Role::setOption(RoleOpts op_type, bool value)
{
	if(op_type > OpBypassRls)
		//Raises an error if the option type is invalid
		throw Exception(ErrorCode::AsgValueInvalidRoleOptionType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(options[op_type] != value);
	options[op_type]=value;
}

void Role::addRole(RoleType role_type, Role *role)
{
	//Raises an error if the role to be added is not allocated
	if(!role)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the role to be added is the 'this' role
	if(role && this==role)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgRoleMemberItself)
						.arg(role->getName()),
						ErrorCode::AsgRoleMemberItself,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	bool role_mem = false, role_adm = false,
			role_mem1 = false, role_adm1 = false;

	//Check if the role to be added already exists in one of the internal role list
	role_mem=this->isRoleExists(MemberRole, role);
	role_adm=this->isRoleExists(AdminRole, role);

	/* Check if the role 'this' is referenced in one of the internal role list
	 of the role to be added */
	role_mem1=role->isRoleExists(MemberRole, this);
	role_adm1=role->isRoleExists(AdminRole, this);

	//Raises an error if the role already exists in one of the internal list
	if((role_type==MemberRole && (role_mem || role_adm)) ||
			(role_type==AdminRole && (role_adm || role_mem)))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InsDuplicatedRole)
						.arg(role->getName())
						.arg(this->getName()),
						ErrorCode::InsDuplicatedRole,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	// Checking for redundant reference between roles.
	if(role_mem1 || role_adm1)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgRoleReferenceRedundancy)
						.arg(role->getName(), this->getName(), this->getName(), role->getName()),
						ErrorCode::AsgRoleReferenceRedundancy,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	getRoleList(role_type)->push_back(role);
	setCodeInvalidated(true);
}

void Role::setConnectionLimit(int limit)
{
	setCodeInvalidated(conn_limit != limit);
	conn_limit=limit;
}

void Role::setValidity(const QString &date)
{
	setCodeInvalidated(validity != date);
	validity=date.mid(0,19);
}

void Role::setPassword(const QString &passwd)
{
	setCodeInvalidated(password != passwd);
	this->password=passwd;
}

void Role::setRoleAttribute(unsigned role_type)
{
	QString attrib;
	QStringList rol_names;
	std::vector<Role *> *roles_vect = getRoleList(role_type);

	if(role_type == MemberRole)
		attrib = Attributes::MemberRoles;
	else
		attrib = Attributes::AdminRoles;

	for(auto &rl : *roles_vect)
		rol_names.append(rl->getName(true));

	attributes[attrib]= rol_names.join(',');
}

std::vector<Role *> *Role::getRoleList(unsigned role_type)
{
	if(role_type == MemberRole)
	 return &member_roles;

	if(role_type == AdminRole)
		return &admin_roles;

	throw Exception(ErrorCode::RefInvalidRoleType,PGM_FUNC,PGM_FILE,PGM_LINE);
}

void Role::removeRole(RoleType role_type, unsigned role_idx)
{
	std::vector<Role *> *list = getRoleList(role_type);
	std::vector<Role *>::iterator itr;

	if(role_idx >= list->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr = list->begin() + role_idx;
	list->erase(itr);
	setCodeInvalidated(true);
}

void Role::removeRoles(RoleType role_type)
{
	std::vector<Role *> *list = getRoleList(role_type);
	list->clear();
	setCodeInvalidated(true);
}

bool Role::isRoleExists(RoleType role_type, Role *role)
{
	std::vector<Role *> *list = getRoleList(role_type);
	std::vector<Role *>::iterator itr, itr_end;
	bool found=false;

	itr = list->begin();
	itr_end = list->end();
	while(!found && itr != itr_end)
	{
		found=((*itr)==role);
		itr++;
	}

	return found;
}

bool Role::isRoleExists(RoleType role_type, const QString &rl_name)
{
	std::vector<Role *> *list = getRoleList(role_type);

	for(auto &rl : *list)
	{
		if(rl->getName() == rl_name)
			return true;
	}

	return false;
}

bool Role::getOption(RoleOpts op_type)
{
	if(op_type > OpBypassRls)
		throw Exception(ErrorCode::AsgValueInvalidRoleOptionType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return options[op_type];
}

Role *Role::getRole(RoleType role_type, unsigned role_idx)
{
	std::vector<Role *> *list = getRoleList(role_type);

	//Raises an error if the role index is invalid (out of bound)
	if(role_idx > list->size())
		throw Exception(ErrorCode::RefRoleInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return list->at(role_idx);
}

unsigned Role::getRoleCount(RoleType role_type)
{
	return getRoleList(role_type)->size();
}

unsigned Role::getConnectionLimit()
{
	return conn_limit;
}

QString Role::getValidity()
{
	return validity;
}

QString Role::getPassword()
{
	return password;
}

QString Role::getSourceCode(SchemaParser::CodeType def_type)
{
	return getSourceCode(def_type, false);
}

QString Role::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	unsigned op_idx = 0;
	QStringList op_attribs = { Attributes::Superuser, Attributes::CreateDb,
														 Attributes::CreateRole, Attributes::Inherit,
														 Attributes::Login, Attributes::Replication,
														 Attributes::BypassRls },
			other_attribs = { Attributes::ConnLimit, Attributes::Password ,
												Attributes::Validity,	Attributes::MemberRoles,
												Attributes::AdminRoles };

	setRoleAttribute(MemberRole);
	setRoleAttribute(AdminRole);

	for(auto &op_attr : op_attribs)
		attributes[op_attr] = (options[op_idx++] ? Attributes::True : "");

	attributes[Attributes::Password] = password;
	attributes[Attributes::Validity] = validity;

	if(conn_limit >= 0)
		attributes[Attributes::ConnLimit] = QString("%1").arg(conn_limit);

	other_attribs.append(op_attribs);

	for(auto &attr : other_attribs)
	{
		if(!attributes[attr].isEmpty())
		{
			attributes[Attributes::HasOptions] = Attributes::True;
			break;
		}
	}

	return BaseObject::getSourceCode(def_type, reduced_form);
}

QString Role::getAlterMembershipCommands(Role *imp_role, Role *ref_role, bool revoke)
{
	Role::RoleType role_types[] = { MemberRole, AdminRole };
	QStringList rl_names;
	attribs_map member_attrs;
	QString cmds;

	if(!imp_role || !ref_role)
		throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	for(auto &rl_type : role_types)
	{
		for(auto &rl : *ref_role->getRoleList(rl_type))
		{
			if(!imp_role->isRoleExists(rl_type, rl->getName()))
				rl_names.append(rl->getName(true));
		}

		if(!rl_names.isEmpty())
		{
			member_attrs[Attributes::Revoke] = (revoke ? Attributes::True : "");
			member_attrs[Attributes::Role] = ref_role->getName(true);
			member_attrs[Attributes::Roles] = rl_names.join(',');
			member_attrs[Attributes::AdminOption] = (rl_type == AdminRole ? Attributes::True : "");

			try
			{
				cmds += schparser.getSourceCode(
									GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, Attributes::RoleMembers),
									member_attrs);
			}
			catch(Exception &e)
			{
				throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
			}

			member_attrs.clear();
			rl_names.clear();
		}
	}

	return cmds;
}

QString Role::getAlterCode(BaseObject *object)
{
	Role *role=dynamic_cast<Role *>(object);

	if(!role)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs;
		QString op_attribs[]={ Attributes::Superuser, Attributes::CreateDb,
							   Attributes::CreateRole, Attributes::Inherit,
								 Attributes::Login, Attributes::Replication,
								 Attributes::BypassRls };

		attributes[Attributes::AlterCmds]=BaseObject::getAlterCode(object);

		if(this->password!=role->password)
		{
			attribs[Attributes::EmptyPassword]=role->password.isEmpty() ? Attributes::True : "";
			attribs[Attributes::Password]=role->password;
		}

		if(this->validity!=role->validity)
			attribs[Attributes::Validity]=role->validity;

		for(unsigned i=0; i <= OpBypassRls; i++)
		{
			if(this->options[i]!=role->options[i])
				attribs[op_attribs[i]]=(role->options[i] ? Attributes::True : Attributes::Unset);
		}

		attributes[Attributes::MemberRoles] = getAlterMembershipCommands(this, role, false);
		attributes[Attributes::MemberRoles] += getAlterMembershipCommands(role, this, true);

		copyAttributes(attribs);

		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Role::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &rl : member_roles)
		deps.push_back(rl);

	for(auto &rl : admin_roles)
		deps.push_back(rl);

	BaseObject::updateDependencies(deps);
}
