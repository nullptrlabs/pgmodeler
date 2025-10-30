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

#include "sequence.h"
#include "coreutilsns.h"

const QString Sequence::MaxPositiveValue {"+2147483647"};
const QString Sequence::MaxNegativeValue {"-2147483648"};
const QString Sequence::MaxSmallPositiveValue {"+32767"};
const QString Sequence::MaxSmallNegativeValue {"-32768"};
const QString Sequence::MaxBigPositiveValue {"+9223372036854775807"};
const QString Sequence::MaxBigNegativeValue {"-9223372036854775808"};

Sequence::Sequence()
{
	obj_type=ObjectType::Sequence;
	cycle=false;
	setDefaultValues(PgSqlType("serial"));
	owner_col=nullptr;

	attributes[Attributes::Increment]="";
	attributes[Attributes::MinValue]="";
	attributes[Attributes::MaxValue]="";
	attributes[Attributes::Start]="";
	attributes[Attributes::Cache]="";
	attributes[Attributes::Cycle]="";
	attributes[Attributes::OwnerColumn]="";
	attributes[Attributes::Table]="";
	attributes[Attributes::Column]="";
	attributes[Attributes::ColIsIdentity]="";
}

bool Sequence::isZeroValue(const QString &value)
{
	if(value.isEmpty())
		return false;

	unsigned i, count;
	bool is_zero;

	i=0;
	is_zero=true;
	count=value.size();

	while(i < count && is_zero)
	{
		is_zero=(value[i]=='0' || value[i]=='+' || value[i]=='-');
		i++;
	}

	return is_zero;
}

bool Sequence::isValidValue(const QString &value)
{
	if(value.isEmpty())
		return false;

	/* To be valid the value can be start with + or -, have only numbers and
		it's length must not exceed the MAX_POSITIVE_VALUE length */
	if(value.size() > MaxBigPositiveValue.size())
		return false;

	unsigned i = 0, count = 0;
	bool is_oper=false, is_num=false, is_valid=true;

	count = value.size();
	for(i=0; i < count && is_valid; i++)
	{
		if((value[i]=='-' || value[i]=='+') && !is_num)
		{
			if(!is_oper) is_oper=true;
		}
		else if((value[i]>='0' && value[i]<='9'))
		{
			if(!is_num) is_num=true;
		}
		else is_valid=false;
	}

	if(!is_num)
		is_valid = false;

	return is_valid;
}

QString Sequence::formatValue(const QString &value)
{
	QString fmt_value;

	//Before format the value checks if it is valid
	if(isValidValue(value))
	{
		unsigned i, count, neg_cnt;

		i=neg_cnt=0;
		count=value.size();

		/* Counts the number of negative operator because
		 the quantity can interfere on the final result
		 of formating */
		while((value[i]=='+' || value[i]=='-') && i < count)
		{
			if(value[i]=='-') neg_cnt++;
			i++;
		}

		//When the negative signal count is odd the number is negative
		if(neg_cnt % 2 != 0) fmt_value+="-";

		fmt_value+=value.mid(i, count);
	}

	return fmt_value;
}

int Sequence::compareValues(QString value1, QString value2)
{
	if(value1==value2 || value1.isEmpty() || value2.isEmpty())
		return 0;

	char ops[2]={'\0','\0'};
	unsigned i, idx, count;
	QString *vet_values[2]={&value1, &value2}, aux_value;

	if(value1.size() < value2.size())
		value1=value1.rightJustified(value1.size() + (value2.size()-value1.size()),'0');
	else if(value1.size() > value2.size())
		value2=value2.rightJustified(value2.size() + (value1.size()-value2.size()),'0');

	for(i=0; i < 2; i++)
	{
		//Gets the value signal
		ops[i]=vet_values[i]->at(0).toLatin1();

		//Case the value doesn't has a + it will be append
		if(ops[i]!='-' && ops[i]!='+') ops[i]='+';

		idx=0;
		count=vet_values[i]->size();
		while(idx < count)
		{
			if(vet_values[i]->at(idx)!='+' &&
					vet_values[i]->at(idx)!='-')
				aux_value+=vet_values[i]->at(idx);
			else
				aux_value+='0';

			idx++;
		}

		(*vet_values[i])=aux_value;
		aux_value="";
	}

	if(ops[0]==ops[1] && value1==value2)
		return 0;

	if((ops[0]=='-' && ops[1]=='-' && value1 > value2) ||
			(ops[0]=='+' && ops[1]=='+' && value1 < value2) ||
			(ops[0]=='-' && ops[1]=='+'))
		return -1;

	return 1;
}

void Sequence::setDefaultValues(PgSqlType serial_type)
{
	QString min, max;

	if(serial_type=="smallserial" ||
		 serial_type.isEquivalentTo(PgSqlType("smallint")))
	{
		min=MaxSmallNegativeValue;
		max=MaxSmallPositiveValue;
	}
	else if(serial_type=="bigserial" ||
					serial_type.isEquivalentTo(PgSqlType("bigint")))
	{
		min=MaxBigNegativeValue;
		max=MaxBigPositiveValue;
	}
	else
	{
		min=MaxNegativeValue;
		max=MaxPositiveValue;
	}

	setValues(min, max, "1", "1", "1");
}


void Sequence::setName(const QString &name)
{
	QString prev_name=this->getName(true);

	BaseObject::setName(name);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Sequence::setSchema(BaseObject *schema)
{
	PhysicalTable *table=nullptr;
	QString prev_name=this->getName(true);

	if(owner_col)
	{
		//Gets the table that owns the column
		table=dynamic_cast<PhysicalTable *>(owner_col->getParentTable());

		//Raises an error when the passed schema differs from the table schema
		if(table && table->getSchema()!=schema)
			throw Exception(ErrorCode::AsgSchemaSequenceDiffersTableSchema,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	BaseObject::setSchema(schema);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void Sequence::setCycle(bool value)
{
	setCodeInvalidated(cycle != value);
	cycle=value;
}

void Sequence::setValues(QString minv, QString maxv, QString inc, QString start, QString cache)
{
	minv=formatValue(minv);
	maxv=formatValue(maxv);
	inc=formatValue(inc);
	start=formatValue(start);
	cache=formatValue(cache);

	if(compareValues(minv,maxv) > 0)
		throw Exception(ErrorCode::AsgInvalidSequenceMinValue,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error when the start value is less that min value or grater than max value
	if(compareValues(start, minv) < 0 ||	compareValues(start, maxv) > 0)
		throw Exception(ErrorCode::AsgInvalidSequenceStartValue,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error when the increment value is null (0)
	if(isZeroValue(inc))
		throw Exception(ErrorCode::AsgInvalidSequenceIncrementValue,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error when the cache value is null (0)
	if(isZeroValue(cache))
		throw Exception(ErrorCode::AsgInvalidSequenceCacheValue,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->min_value=minv;
	this->max_value=maxv;
	this->increment=inc;
	this->cache=cache;
	this->start=start;

	setCodeInvalidated(true);
}

void Sequence::setOwnerColumn(PhysicalTable *table, const QString &col_name)
{
	if(!table || col_name.isEmpty())
		this->owner_col=nullptr;
	else if(table)
	{
		//Raises an error if the table schema differs from the sequence schema
		if(table->getSchema()!=this->schema)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgSeqOwnerTableDifferentSchema)
							.arg(this->getName(true)),
							ErrorCode::AsgSeqOwnerTableDifferentSchema,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error when the table owner role differs from the sequence owner
		if(table->getOwner()!=this->owner)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgSeqOwnerTableDifferentRole)
							.arg(this->getName(true)),
							ErrorCode::AsgSeqOwnerTableDifferentRole,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Gets the column with the passed name
		this->owner_col=table->getColumn(col_name);

		//Raises an error if the column doesn't exists
		if(!this->owner_col)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgInexistentSeqOwnerColumn)
							.arg(this->getName(true)),
							ErrorCode::AsgInexistentSeqOwnerColumn,PGM_FUNC,PGM_FILE,PGM_LINE);

		/* If the onwer column was added by relationship and the column id is greater than
		 sequence id, change the sequence id to be greater to avoid reference errors */
		if(this->owner_col && this->owner_col->isAddedByRelationship() &&
				this->owner_col->getObjectId() > this->object_id)
			this->object_id=BaseObject::getGlobalId();
	}

	setCodeInvalidated(true);
}

void Sequence::setOwnerColumn(Column *column)
{
	PhysicalTable *table=nullptr;

	if(!column)
		this->owner_col=nullptr;
	else
	{
		table=dynamic_cast<PhysicalTable *>(column->getParentTable());

		//Raises an error when the column doesn't has a parent table
		if(!table)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidSeqOwnerColumn)
							.arg(this->getName(true)),
							ErrorCode::AsgInvalidSeqOwnerColumn,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error if the table schema differs from the sequence schema
		if(table->getSchema()!=this->schema)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgSeqOwnerTableDifferentSchema)
							.arg(this->getName(true)),
							ErrorCode::AsgSeqOwnerTableDifferentSchema,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Raises an error when the table owner role differs from the sequence owner
		if(table->getOwner()!=this->owner)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgSeqOwnerTableDifferentRole)
							.arg(this->getName(true)),
							ErrorCode::AsgSeqOwnerTableDifferentRole,PGM_FUNC,PGM_FILE,PGM_LINE);

		this->owner_col=column;

		/* If the onwer column was added by relationship and the column id is greater than
		 sequence id, change the sequence id to be greater to avoid reference errors */
		if(column && column->isAddedByRelationship() &&
				column->getObjectId() > this->object_id)
			this->object_id=BaseObject::getGlobalId();
	}

	setCodeInvalidated(true);
}

bool Sequence::isReferRelationshipAddedColumn()
{
	return (owner_col && owner_col->isAddedByRelationship());
}

bool Sequence::isCycle()
{
	return cycle;
}

QString Sequence::getMaxValue()
{
	return max_value;
}

QString Sequence::getMinValue()
{
	return min_value;
}

QString Sequence::getCache()
{
	return cache;
}

QString Sequence::getIncrement()
{
	return increment;
}

QString Sequence::getStart()
{
	return start;
}

Column *Sequence::getOwnerColumn()
{
	return owner_col;
}

QString Sequence::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	PhysicalTable *table=nullptr;

	if(owner_col)
	{
		attributes[Attributes::OwnerColumn]=owner_col->getSignature();
		table=dynamic_cast<PhysicalTable *>(owner_col->getParentTable());
	}

	attributes[Attributes::Table]=(table ? table->getName(true) : "");
	attributes[Attributes::Column]=(owner_col ? owner_col->getName(true) : "");

	attributes[Attributes::ColIsIdentity]=
			(owner_col && owner_col->getIdentityType() != IdentityType::Null ? Attributes::True : "");

	attributes[Attributes::Increment]=increment;
	attributes[Attributes::MinValue]=min_value;
	attributes[Attributes::MaxValue]=max_value;
	attributes[Attributes::Start]=start;
	attributes[Attributes::Cache]=cache;
	attributes[Attributes::Cycle]=(cycle ? Attributes::True : "");

	return BaseObject::__getSourceCode(def_type);
}

QString Sequence::getAlterCode(BaseObject *object)
{
	Sequence *seq=dynamic_cast<Sequence *>(object);

	if(!seq)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		PhysicalTable *table=nullptr;
		attribs_map attribs;

		attributes[Attributes::AlterCmds]=BaseObject::getAlterCode(object);

		if((this->owner_col && !seq->owner_col) ||
				(!this->owner_col && seq->owner_col) ||
				(this->owner_col && seq->owner_col &&
				 this->owner_col->getSignature()!=seq->owner_col->getSignature()))
		{
			if(seq->owner_col)
			{
				attribs[Attributes::OwnerColumn]=seq->owner_col->getSignature();
				table=dynamic_cast<PhysicalTable *>(seq->owner_col->getParentTable());

				if(table)
				{
					attribs[Attributes::Table]=table->getName(true);
					attribs[Attributes::Column]=seq->owner_col->getName(true);
				}
			}
			else
				attribs[Attributes::OwnerColumn]=Attributes::Unset;
		}

		if(!seq->increment.isEmpty() && this->increment!=seq->increment)
			attribs[Attributes::Increment]=seq->increment;

		if(!seq->min_value.isEmpty() && this->min_value!=seq->min_value)
			attribs[Attributes::MinValue]=seq->min_value;

		if(!seq->max_value.isEmpty() && this->max_value!=seq->max_value)
			attribs[Attributes::MaxValue]=seq->max_value;

		if(!seq->start.isEmpty() && this->start!=seq->start)
			attribs[Attributes::Start]=seq->start;

		if(!seq->cache.isEmpty() && this->cache!=seq->cache)
			attribs[Attributes::Cache]=seq->cache;

		if(this->cycle!=seq->cycle)
			attribs[Attributes::Cycle]=(seq->cycle ? Attributes::True : Attributes::Unset);

		copyAttributes(attribs);
		return BaseObject::getAlterCode(this->getSchemaName(), attributes, false, true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void Sequence::operator = (Sequence &seq)
{
	QString prev_name=this->getName(true);

	*(dynamic_cast<BaseObject *>(this))=dynamic_cast<BaseObject &>(seq);

	this->cycle=seq.cycle;
	this->max_value=seq.max_value;
	this->min_value=seq.min_value;
	this->start=seq.start;
	this->increment=seq.increment;
	this->cache=seq.cache;
	this->owner_col=seq.owner_col;

	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

QString Sequence::getDataDictionary(bool md_format, const attribs_map &extra_attribs)
{
	try
	{
		attribs_map attribs;

		attribs.insert(extra_attribs.begin(), extra_attribs.end());
		attribs[Attributes::Name] = getSignature();
		attribs[Attributes::Cycle] = cycle ? CoreUtilsNs::DataDictCheckMark : "";
		attribs[Attributes::MinValue] = min_value;
		attribs[Attributes::MaxValue] = max_value;
		attribs[Attributes::Comment] = comment;

		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, getSchemaName()), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Sequence::updateDependencies()
{
	BaseObject::updateDependencies({ owner_col });
}
