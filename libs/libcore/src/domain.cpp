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

#include "domain.h"

Domain::Domain()
{
	obj_type=ObjectType::Domain;
	not_null=false;
	attributes[Attributes::DefaultValue]="";
	attributes[Attributes::NotNull]="";
	attributes[Attributes::Type]="";
	attributes[Attributes::Constraints]="";
}

void Domain::addCheckConstraint(const QString &name, const QString &expr)
{
	//Raises an error if the constraint name is invalid
	if(!name.isEmpty() && !BaseObject::isValidName(name))
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(expr.isEmpty())
		throw Exception(ErrorCode::AsgInvalidExpressionObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(chk_constrs.count(name))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
						.arg(name)
						.arg(BaseObject::getTypeName(ObjectType::Constraint))
						.arg(this->getName(true))
						.arg(this->getTypeName()),
						ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	chk_constrs[name] = expr;
	setCodeInvalidated(true);
}

void Domain::removeCheckConstraints()
{
	chk_constrs.clear();
}

attribs_map Domain::getCheckConstraints()
{
	return chk_constrs;
}

void Domain::setName(const QString &name)
{
	QString prev_name, new_name;

	prev_name=this->getName(true);
	BaseObject::setName(name);
	new_name=this->getName(true);

	//Renames the PostgreSQL type represented by the domain
	PgSqlType::renameUserType(prev_name, this, new_name);
}

void Domain::setSchema(BaseObject *schema)
{
	QString prev_name;

	prev_name=this->getName(true);
	BaseObject::setSchema(schema);

	//Renames the PostgreSQL type represented by the domain
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Domain::setDefaultValue(const QString &default_val)
{
	QString def=default_val.trimmed();

	setCodeInvalidated(default_value != def);
	this->default_value=def;
}

void Domain::setNotNull(bool value)
{
	setCodeInvalidated(not_null != value);
	not_null=value;
}

void Domain::setType(PgSqlType type)
{
	setCodeInvalidated(this->type != type);
	this->type=type;
}

QString Domain::getDefaultValue()
{
	return default_value;
}

bool Domain::isNotNull()
{
	return not_null;
}

PgSqlType Domain::getType()
{
	return type;
}

QString Domain::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	attribs_map aux_attribs;

	attributes[Attributes::NotNull]=(not_null ? Attributes::True : "");
	attributes[Attributes::DefaultValue]=default_value;

	for(auto itr : chk_constrs)
	{
		aux_attribs[Attributes::Name] = itr.first;
		aux_attribs[Attributes::Expression] = itr.second;
		attributes[Attributes::Constraints]+=schparser.getSourceCode(Attributes::DomConstraint, aux_attribs, def_type);
	}

	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::Type]=(*type);
	else
		attributes[Attributes::Type]=type.getSourceCode(def_type);

	return BaseObject::__getSourceCode(def_type);
}

void Domain::operator = (Domain &domain)
{
	QString prev_name=this->getName(true);

	*(dynamic_cast<BaseObject *>(this))=dynamic_cast<BaseObject &>(domain);
	this->not_null=domain.not_null;
	this->default_value=domain.default_value;
	this->type=domain.type;
	this->chk_constrs=domain.chk_constrs;
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

QString Domain::getAlterCode(BaseObject *object)
{
	Domain *domain=dynamic_cast<Domain *>(object);

	if(!domain)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		QString alter_def=BaseObject::getAlterCode(object);
		attribs_map orig_constrs, aux_constrs, aux_attribs;
		QString orig_expr, aux_expr;

		attributes[Attributes::DefaultValue]="";
		attributes[Attributes::NotNull]="";
		attributes[Attributes::Constraints]="";
		attributes[Attributes::Expression]="";
		attributes[Attributes::OldName]="";
		attributes[Attributes::NewName]="";

		if(this->default_value.simplified().toLower()!=domain->default_value.simplified().toLower())
			attributes[Attributes::DefaultValue]=(!domain->default_value.isEmpty() ? domain->default_value : Attributes::Unset);

		if(this->not_null!=domain->not_null)
			attributes[Attributes::NotNull]=(domain->not_null ? Attributes::True : Attributes::Unset);

		orig_constrs = this->chk_constrs;
		aux_constrs = domain->chk_constrs;
		aux_attribs[Attributes::SqlObject] = this->getSQLName();
		aux_attribs[Attributes::Signature] = this->getSignature();

		//Generating the DROP for check constraints that does not exists anymore
		for(auto constr : orig_constrs)
		{
			orig_expr = QString(constr.second).remove(QChar(' ')).toLower();
			aux_expr = QString(aux_constrs[constr.first]).remove(QChar(' ')).toLower();

			//If the check constraint expression exists and the expressions differ from source to destination we drop it
			if(aux_constrs.count(constr.first) == 0 ||
				 (aux_constrs.count(constr.first) && orig_expr != aux_expr))
			{
				aux_attribs[Attributes::Name]=constr.first;
				aux_attribs[Attributes::Expression]=Attributes::Unset;
				attributes[Attributes::Constraints]+=BaseObject::getAlterCode(Attributes::DomConstraint, aux_attribs, false, true);
			}

			//We should include a command to recreate the check constraint with the new expression
			if(aux_constrs.count(constr.first) && orig_expr != aux_expr)
			{
				aux_attribs[Attributes::Name]=constr.first;
				aux_attribs[Attributes::Expression]=aux_constrs[constr.first];
				attributes[Attributes::Constraints]+=BaseObject::getAlterCode(Attributes::DomConstraint, aux_attribs, false, true);
			}
		}

		//Generating the ADD for new check constraints
		for(auto constr : aux_constrs)
		{
			if(orig_constrs.count(constr.first) == 0)
			{
				aux_attribs[Attributes::Name]=constr.first;
				aux_attribs[Attributes::Expression]=constr.second;
				attributes[Attributes::Constraints]+=BaseObject::getAlterCode(Attributes::DomConstraint, aux_attribs, false, true);
			}
		}

		alter_def+=BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
		return alter_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Domain::updateDependencies()
{
	BaseObject::updateDependencies({ type.getObject() });
}
