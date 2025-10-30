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

#include "column.h"
#include "coreutilsns.h"

const QString Column::NextValFuncTmpl {"nextval('%1'::regclass)"};

Column::Column()
{
	obj_type=ObjectType::Column;
	not_null=seq_cycle=generated=false;
	attributes[Attributes::Type]="";
	attributes[Attributes::DefaultValue]="";
	attributes[Attributes::NotNull]="";
	attributes[Attributes::Generated]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::Sequence]="";
	attributes[Attributes::DeclInTable]="";
	attributes[Attributes::IdentityType]="";
	attributes[Attributes::Increment]="";
	attributes[Attributes::MinValue]="";
	attributes[Attributes::MaxValue]="";
	attributes[Attributes::Start]="";
	attributes[Attributes::Cache]="";
	attributes[Attributes::Cycle]="";

	sequence=nullptr;
	identity_type = IdentityType::Null;
}

void Column::setName(const QString &name)
{
	try
	{
		QString prev_name;

		//The current column name will be used as the old name
		prev_name=this->obj_name;

		//Tries to define the new name to column
		BaseObject::setName(name);

		/* Case no error is raised stored the old name on the
		 respective column attribute */
		this->old_name=prev_name;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Column::setType(PgSqlType tp)
{
	//An error is raised if the column receive a pseudo-type as data type.
	if(tp.isPseudoType())
		throw Exception(ErrorCode::AsgPseudoTypeColumn,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(this->identity_type != IdentityType::Null && !tp.isIntegerType())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvalidIdentityColumn).arg(getSignature()),
										ErrorCode::InvalidIdentityColumn, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	setCodeInvalidated(this->type != tp);
	this->type = tp;
}

void Column::setIdentityType(IdentityType id_type)
{
	if(id_type != IdentityType::Null && !type.isIntegerType())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvalidIdentityColumn).arg(getSignature()),
										ErrorCode::InvalidIdentityColumn, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	setCodeInvalidated(identity_type != id_type);
	identity_type = id_type;
	default_value.clear();
	sequence = nullptr;
	generated = false;

	//Identity column implies NOT NULL constraint
	if(id_type != IdentityType::Null)
		setNotNull(true);
}

void Column::setDefaultValue(const QString &value)
{
	setCodeInvalidated(default_value != value);
	default_value = value.trimmed();
	sequence = nullptr;
	identity_type = IdentityType::Null;
}

void Column::setNotNull(bool value)
{
	setCodeInvalidated(not_null != value);
	not_null=value;
}

void Column::setGenerated(bool value)
{
	setCodeInvalidated(generated != value);
	generated = value;
	identity_type = IdentityType::Null;
	sequence = nullptr;
}

PgSqlType Column::getType()
{
	return type;
}

IdentityType Column::getIdentityType()
{
	return identity_type;
}

bool Column::isNotNull()
{
	return not_null;
}

bool Column::isGenerated()
{
	return generated;
}

bool Column::isIdentity()
{
	return (identity_type != IdentityType::Null);
}

QString Column::getTypeReference()
{
	if(getParentTable())
		return getParentTable()->getName(true) + QString(".") + this->getName(true) + QString("%TYPE");
	
	return "";
}

QString Column::getDefaultValue()
{
	return default_value;
}

QString Column::getOldName(bool format)
{
	if(format)
		return BaseObject::formatName(old_name);
	
	return old_name;
}

void Column::setSequence(BaseObject *seq)
{
	if(seq)
	{
		if(seq->getObjectType() != ObjectType::Sequence)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidObjectType)
											.arg(this->obj_name)
											.arg(this->getTypeName())
											.arg(BaseObject::getTypeName(ObjectType::Sequence)),
											ErrorCode::AsgInvalidObjectType, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		if(!type.isIntegerType() && !type.isNumericType())
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::IncompColumnTypeForSequence)
											.arg(seq->getName(true))
											.arg(this->obj_name),
											ErrorCode::IncompColumnTypeForSequence, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		default_value="";
		identity_type=IdentityType::Null;
		generated = false;
	}

	setCodeInvalidated(sequence != seq);
	sequence = seq;
}

BaseObject *Column::getSequence()
{
	return sequence;
}

bool Column::isIdSeqCycle()
{
	return seq_cycle;
}

QString Column::getIdSeqMaxValue()
{
	return seq_max_value;
}

QString Column::getIdSeqMinValue()
{
	return seq_min_value;
}

QString Column::getIdSeqIncrement()
{
	return seq_increment;
}

QString Column::getIdSeqStart()
{
	return seq_start;
}

QString Column::getIdSeqCache()
{
	return seq_cache;
}

void Column::setIdSeqAttributes(QString minv, QString maxv, QString inc, QString start, QString cache, bool cycle)
{
	seq_min_value = minv;
	seq_max_value = maxv;
	seq_increment = inc;
	seq_start = start;
	seq_cache = cache;
	seq_cycle = cycle;
}

QString Column::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	if(getParentTable())
		attributes[Attributes::Table]=getParentTable()->getName(true);

	attributes[Attributes::Type]=type.getSourceCode(def_type);
	attributes[Attributes::DefaultValue]="";
	attributes[Attributes::IdentityType]="";

	if(identity_type != IdentityType::Null)
	{
		attributes[Attributes::IdentityType] = ~identity_type;
		attributes[Attributes::Increment]=seq_increment;
		attributes[Attributes::MinValue]=seq_min_value;
		attributes[Attributes::MaxValue]=seq_max_value;
		attributes[Attributes::Start]=seq_start;
		attributes[Attributes::Cache]=seq_cache;
		attributes[Attributes::Cycle]=(seq_cycle ? Attributes::True : "");
	}
	else
	{
		if(!sequence)
			attributes[Attributes::DefaultValue]=default_value;
		else
		{
			//Configuring the default value of the column to get the next value of the sequence
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::DefaultValue]=NextValFuncTmpl.arg(sequence->getSignature());

			attributes[Attributes::Sequence]=sequence->getName(true);
		}
	}

	attributes[Attributes::NotNull]=(!not_null ? "" : Attributes::True);
	attributes[Attributes::Generated]=(generated ? Attributes::True : "");
	attributes[Attributes::DeclInTable]=(isDeclaredInTable() ? Attributes::True : "");

	return BaseObject::__getSourceCode(def_type);
}

QString Column::getAlterCode(BaseObject *object)
{
	Column *col=dynamic_cast<Column *>(object);

	if(!col)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs;
		QString def_val, alter_def;
		bool ident_seq_changed = false;

		if(!this->type.isEquivalentTo(col->type) ||
				(this->type.isEquivalentTo(col->type) &&

				 ((this->type.hasVariableLength() &&
					 this->type.getLength() != col->type.getLength()) ||

					(this->type.acceptsPrecision() &&
					 col->type.getPrecision() > 0 &&
					 this->type.getPrecision() != col->type.getPrecision()))))
			attribs[Attributes::Type]=col->type.getSourceCode(SchemaParser::SqlCode);

		if(col->sequence)
			def_val=NextValFuncTmpl.arg(col->sequence->getSignature());
		else
			def_val=col->default_value;

		// Generated columns can't have their default value changes after being created
		if(!this->isGenerated() && !col->isGenerated() &&
			 this->default_value.simplified().toLower() != def_val.simplified().toLower())
			attribs[Attributes::DefaultValue]=(def_val.isEmpty() ? Attributes::Unset : def_val);

		if(this->not_null!=col->not_null)
			attribs[Attributes::NotNull]=(!col->not_null ? Attributes::Unset : Attributes::True);

		attribs[Attributes::NewIdentityType] = "";

		if(this->identity_type == IdentityType::Null && col->identity_type != IdentityType::Null)
			attribs[Attributes::IdentityType] = ~col->identity_type;
		else if(this->identity_type != IdentityType::Null && col->identity_type == IdentityType::Null)
			attribs[Attributes::IdentityType] = Attributes::Unset;
		else if(this->identity_type != IdentityType::Null && col->identity_type != IdentityType::Null &&
						this->identity_type != col->identity_type)
			attribs[Attributes::NewIdentityType] = ~col->identity_type;

		attribs[Attributes::CurIdentityType] = "";
		attribs[Attributes::MinValue] = "";
		attribs[Attributes::MaxValue] = "";
		attribs[Attributes::Start] = "";
		attribs[Attributes::Increment] = "";
		attribs[Attributes::Cache] = "";
		attribs[Attributes::Cycle] = "";

		//Checking differences in the underlying sequence (identity col)
		if(attribs[Attributes::IdentityType] != Attributes::Unset)
		{
			if(!col->seq_min_value.isEmpty() && this->seq_min_value != col->seq_min_value)
			{
				attribs[Attributes::MinValue] = col->seq_min_value;
				ident_seq_changed = true;
			}

			if(!col->seq_max_value.isEmpty() && this->seq_max_value != col->seq_max_value)
			{
				attribs[Attributes::MaxValue] = col->seq_max_value;
				ident_seq_changed = true;
			}

			if(!col->seq_start.isEmpty() && this->seq_start != col->seq_start)
			{
				attribs[Attributes::Start] = col->seq_start;
				ident_seq_changed = true;
			}

			if(!col->seq_increment.isEmpty() && this->seq_increment != col->seq_increment)
			{
				attribs[Attributes::Increment] = col->seq_increment;
				ident_seq_changed = true;
			}

			if(!col->seq_cache.isEmpty() && this->seq_cache != col->seq_cache)
			{
				attribs[Attributes::Cache] = col->seq_cache;
				ident_seq_changed = true;
			}

			if(this->seq_cycle != col->seq_cycle)
			{
				attribs[Attributes::Cycle] = (col->seq_cycle ? Attributes::True : Attributes::False);
				ident_seq_changed = true;
			}

			if(ident_seq_changed)
				attribs[Attributes::CurIdentityType] = ~this->identity_type;
		}

		copyAttributes(attribs);
		alter_def = TableObject::getAlterCode(col);
		alter_def += BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);

		return alter_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Column::configureSearchAttributes()
{
	BaseObject::configureSearchAttributes();
	search_attribs[Attributes::Type] = *type;
}

void Column::operator = (Column &col)
{
	this->comment=col.comment;
	this->is_protected=col.is_protected;

	this->obj_name=col.obj_name;
	this->alias=col.alias;
	this->old_name=col.old_name;

	this->type=col.type;
	this->default_value=col.default_value;

	this->not_null=col.not_null;
	this->generated=col.generated;
	this->sequence=col.sequence;
	this->identity_type=col.identity_type;

	this->seq_cache = col.seq_cache;
	this->seq_cycle = col.seq_cycle;
	this->seq_increment = col.seq_increment;
	this->seq_max_value = col.seq_max_value;
	this->seq_min_value = col.seq_min_value;
	this->seq_start = col.seq_start;

	this->setParentTable(col.getParentTable());
	this->setAddedByCopy(false);
	this->setAddedByGeneralization(false);
	this->setAddedByLinking(false);
	this->setCodeInvalidated(true);
	this->setParentRelationship(col.getParentRelationship());
}

QString Column::getDataDictionary(bool md_format, const attribs_map &extra_attribs)
{
	try
	{
		attribs_map attribs;

		attribs.insert(extra_attribs.begin(), extra_attribs.end());
		attribs[Attributes::Parent] = getParentTable()->getSchemaName();
		attribs[Attributes::Name] = obj_name;
		attribs[Attributes::Type] = *type;
		attribs[Attributes::DefaultValue] = sequence ? NextValFuncTmpl.arg(sequence->getSignature()) : default_value;
		attribs[Attributes::Comment] = comment;
		attribs[Attributes::NotNull] = not_null ? CoreUtilsNs::DataDictCheckMark : "";

		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, getSchemaName()), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Column::updateDependencies()
{
	TableObject::updateDependencies({ sequence, type.getObject() });
}

void Column::generateHashCode()
{
	TableObject::generateHashCode();
	hash_code = UtilsNs::getStringHash(hash_code + type.getTypeSql() + QString::number(not_null));
}
