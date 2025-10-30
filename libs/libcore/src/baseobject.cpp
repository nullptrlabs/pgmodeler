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

#include "baseobject.h"
#include "coreutilsns.h"
#include "exception.h"
#include <QApplication>

QString BaseObject::pgsql_ver { PgSqlVersions::DefaulVersion };

bool BaseObject::escape_comments {true};
bool BaseObject::clear_deps_in_dtor {true};
bool BaseObject::ignore_db_version {false};

unsigned BaseObject::global_id {5000};

const QByteArray BaseObject::special_chars {"'_-.@ $:()/<>+*\\=~!#%^&|?{}[]`;"};

const QString BaseObject::objs_schemas[ObjectTypeCount] {
	"column",  "constraint", "function", "trigger",
	"index", "rule", "table", "view",
	"domain", "schema", "aggregate", "operator",
	"sequence", "role", "conversion", "cast",
	"language", "usertype", "tablespace",
	"opfamily", "opclass", "database","collation",
	"extension", "eventtrigger", "policy", "foreigndatawrapper",
	"foreignserver", "foreigntable", "usermapping", "transform",
	"procedure", "relationship", "textbox", "permission", "parameter",
	"typeattribute", "tag", "genericsql", "relationship"
};

const QString BaseObject::objs_sql[ObjectTypeCount] {
	"COLUMN", "CONSTRAINT", "FUNCTION",
	"TRIGGER", "INDEX", "RULE", "TABLE",
	"VIEW", "DOMAIN", "SCHEMA", "AGGREGATE",
	"OPERATOR", "SEQUENCE", "ROLE", "CONVERSION",
	"CAST", "LANGUAGE", "TYPE", "TABLESPACE",
	"OPERATOR FAMILY", "OPERATOR CLASS", "DATABASE",
	"COLLATION", "EXTENSION", "EVENT TRIGGER",
	"POLICY", "FOREIGN DATA WRAPPER", "SERVER",
	"FOREIGN TABLE", "USER MAPPING", "TRANSFORM",
	"PROCEDURE"
};

const QString BaseObject::obj_type_names[ObjectTypeCount] {
	QT_TR_NOOP("Column"), QT_TR_NOOP("Constraint"), QT_TR_NOOP("Function"),
	QT_TR_NOOP("Trigger"), QT_TR_NOOP("Index"), QT_TR_NOOP("Rule"),
	QT_TR_NOOP("Table"), QT_TR_NOOP("View"),  QT_TR_NOOP("Domain"),
	QT_TR_NOOP("Schema"), QT_TR_NOOP("Aggregate"), QT_TR_NOOP("Operator"),
	QT_TR_NOOP("Sequence"), QT_TR_NOOP("Role"), QT_TR_NOOP("Conversion"),
	QT_TR_NOOP("Cast"), QT_TR_NOOP("Language"), QT_TR_NOOP("Type"), QT_TR_NOOP("Tablespace"),
	QT_TR_NOOP("Operator Family"), QT_TR_NOOP("Operator Class"),
	QT_TR_NOOP("Database"), QT_TR_NOOP("Collation"), QT_TR_NOOP("Extension"),
	QT_TR_NOOP("Event Trigger"), QT_TR_NOOP("Policy"),	QT_TR_NOOP("Foreign Data Wrapper"),
	QT_TR_NOOP("Foreign Server"),	QT_TR_NOOP("Foreign Table"), QT_TR_NOOP("User Mapping"),
	QT_TR_NOOP("Transform"), QT_TR_NOOP("Procedure"), QT_TR_NOOP("Relationship"),
	QT_TR_NOOP("Textbox"),	QT_TR_NOOP("Permission"),	QT_TR_NOOP("Parameter"),
	QT_TR_NOOP("Type Attribute"), QT_TR_NOOP("Tag"), QT_TR_NOOP("Generic SQL"),
	QT_TR_NOOP("Basic Relationship")
};

const attribs_map BaseObject::search_attribs_i18n {
	{ Attributes::Name, QT_TR_NOOP("Name") },
	{ Attributes::Comment, QT_TR_NOOP("Comment") },
	{ Attributes::Signature, QT_TR_NOOP("Signature") },
	{ Attributes::Schema, QT_TR_NOOP("Schema") },
	{ Attributes::Owner, QT_TR_NOOP("Owner") },
	{ Attributes::Tablespace, QT_TR_NOOP("Tablespace") },
	{ Attributes::Type, QT_TR_NOOP("Data type") },
	{ Attributes::ReturnType, QT_TR_NOOP("Return type") },
	{ Attributes::SrcTable, QT_TR_NOOP("Source table") },
	{ Attributes::DstTable, QT_TR_NOOP("Destination table") },
	{ Attributes::RelatedForeignKey, QT_TR_NOOP("Related foreign key") },
	{ Attributes::SrcColumns, QT_TR_NOOP("Source column(s)") },
	{ Attributes::RefColumns, QT_TR_NOOP("Referenced column(s)") }
};

const QStringList BaseObject::search_attribs_names {
	Attributes::Name, Attributes::Comment, Attributes::Signature,
	Attributes::Schema, Attributes::Owner, Attributes::Tablespace,
	Attributes::Type, Attributes::ReturnType, Attributes::SrcTable,
	Attributes::DstTable, Attributes::RelatedForeignKey, Attributes::SrcColumns,
	Attributes::RefColumns
};

BaseObject::BaseObject()
{
	pg_oid = 0;
	object_id=BaseObject::global_id++;
	is_protected=system_obj=sql_disabled=false;
	code_invalidated=true;
	obj_type=ObjectType::BaseObject;
	schema=nullptr;
	owner=nullptr;
	tablespace=nullptr;
	database=nullptr;
	collation=nullptr;
	attributes[Attributes::Name]="";
	attributes[Attributes::Alias]="";
	attributes[Attributes::Comment]="";
	attributes[Attributes::Owner]="";
	attributes[Attributes::Tablespace]="";
	attributes[Attributes::Schema]="";
	attributes[Attributes::Collation]="";
	attributes[Attributes::Protected]="";
	attributes[Attributes::SqlDisabled]="";
	attributes[Attributes::AppendedSql]="";
	attributes[Attributes::PrependedSql]="";
	attributes[Attributes::Drop]="";
	attributes[Attributes::Signature]="";
	attributes[Attributes::EscapeComment]="";
	this->setName(qApp->translate("BaseObject","new_object","", -1));
}

BaseObject::~BaseObject()
{
	if(clear_deps_in_dtor)
		clearAllDepsRefs();
}

unsigned BaseObject::getGlobalId()
{
	return global_id;
}

void BaseObject::setEscapeComments(bool value)
{
	escape_comments = value;
}

bool BaseObject::isEscapeComments()
{
	return escape_comments;
}

QString BaseObject::getTypeName(ObjectType obj_type)
{
	if(obj_type!=ObjectType::BaseObject)
		/* Due to the class BaseObject not be derived from QObject the function tr() is inefficient to
		 translate the type names thus the method called to do the translation is from the application
		 specifying the context (BaseObject) in the ts file and the text to be translated */
		return qApp->translate("BaseObject", obj_type_names[enum_t(obj_type)].toStdString().c_str(),"", -1);

	return "";
}

QString BaseObject::getTypeName(const QString &type_str)
{
	return getTypeName(getObjectType(type_str));
}

ObjectType BaseObject::getObjectType(const QString &type_name, bool is_sql_name)
{
	for(unsigned i = 0; i < BaseObject::ObjectTypeCount; i++)
	{
		if((is_sql_name && !objs_sql[i].isEmpty() && objs_sql[i] == type_name.toUpper()) ||
			 (!is_sql_name && objs_schemas[i] == type_name.toLower()))
		{
			return static_cast<ObjectType>(i);
		}
	}

	return ObjectType::BaseObject;
}

QString BaseObject::getSchemaName(ObjectType obj_type)
{
	return objs_schemas[enum_t(obj_type)];
}

QString BaseObject::getSQLName(ObjectType obj_type)
{
	return objs_sql[enum_t(obj_type)];
}

QString BaseObject::formatName(const QString &name, bool is_operator)
{
	bool is_formated=false;
	QString frmt_name;
	QByteArray raw_name;
	unsigned char chr, chr1, chr2;
	QRegularExpression fmt_name_regexp(QRegularExpression::anchoredPattern("(\")(.)+(\")"));

	//Checking if the name is already formated enclosed by quotes
	is_formated = fmt_name_regexp.match(name).hasMatch();

	/* If the name is not formatted or it symbolizes the name of an operator
		(which has characters invalid according to the rule and is the only exception
		 to which its name is formatted even being invalid) or if the name is valid according
		 with PostgreSQL rules for other types of objects */
	if(!is_formated && (is_operator || isValidName(name)))
	{
		bool needs_fmt=false;
		unsigned i = 0, qtd = 0;

		raw_name.append(name.toUtf8());

		/* Checks if the name has some upper case letter. If its the
		 case the name will be enclosed in quotes */
		needs_fmt = (!is_operator && name.contains(QRegularExpression("^[0-9]+")));

		for(int idx = 0; idx < special_chars.size() && !needs_fmt; idx++)
			needs_fmt = (!is_operator && special_chars.at(idx) != '_' && name.indexOf(special_chars.at(idx)) >= 0);

		qtd=name.size();
		i=0;
		while(i < qtd && !needs_fmt)
		{
			chr=raw_name[i];

			if(((i + 1) < (qtd-1)) &&
					((chr >= 0xC2 && chr <= 0xDF) ||
					 (chr >= 0xE0 && chr <= 0xEF)))
				chr1=raw_name[i+1];
			else
				chr1=0;

			if((i + 2) < (qtd-1) &&
					chr >= 0xE0 && chr <= 0xEF)
				chr2=raw_name[i+2];
			else
				chr2=0;

			if(chr1!=0 && chr2!=0)
				i+=3;
			else if(chr1!=0 && chr2==0)
				i+=2;
			else
				i++;

			//2 bytes UTF-8 character
			if((chr  >= 0xC2 && chr <= 0xDF &&
				chr1 >= 0x80 && chr1 <= 0xBF) ||

					//3 bytes UTF-8 character
					(chr  >= 0xE0 && chr <= 0xEF &&
					 chr1 >= 0x80 && chr1 <= 0xBF &&
					 chr2 >= 0x80 && chr2 <= 0xBF) ||

					QChar(chr).isUpper())
			{
				needs_fmt=true;
			}

		}

		if(needs_fmt || CoreUtilsNs::isReservedKeyword(name))
			frmt_name=QString("\"%1\"").arg(name);
		else
			frmt_name=name;
	}
	else if(is_formated)
		frmt_name=name;

	return frmt_name;
}

bool BaseObject::isValidName(const QString &name)
{
	QString aux_name=name;

	// Remove the quotes from the start/end of the name to calculate the exact length
	if(aux_name.contains(QRegularExpression::anchoredPattern("(\")(.)+(\")")))
	{
		aux_name.remove(0,1);
		aux_name.remove(aux_name.size()-1,1);
	}

	/* If the name is greater than the maximum size accepted
	by PostgreSQL (currently 63 bytes) the name is invalid.
	In this case the starting and ending quotes are discarded from
	the name in order to validate the length. */
	if(name.isEmpty() || aux_name.size() > ObjectNameMaxLength)
		return false;

	int i=0, len;
	bool valid=true;
	unsigned char chr='\0', chr1='\0', chr2='\0';
	QByteArray raw_name;
	bool is_sch_qualified = name.contains('.');

	raw_name.append(name.toUtf8());
	len=raw_name.size();

	chr=raw_name[0];
	if(len > 1)
		chr1=raw_name[len-1];

	//Checks if the name is enclosed in quotes
	if(chr=='\"' && chr1=='\"')
	{
		/* Validates the name but the validation will continue until the
		end of string (or the last quote) */
		valid=true; i++; len--;
	}

	while(valid && i < len)
	{
		chr = raw_name[i];

		/* If the name is schema qulified [schema].[name] we just ignore the
			* double quotes if they appear because double quote between dot which
			* qualifies hierarchy is completely accepatable */
		if(is_sch_qualified && chr == '"')
		{
			i++;
			continue;
		}

		/* Validation of simple ASCI characters.
			Checks if the name has the characters in the set [ a-z A-Z 0-9 _ . @ $ - : space () <>] */
		if((chr >= 'a' && chr <='z') || (chr >= 'A' && chr <='Z') ||
				(chr >= '0' && chr <='9') || special_chars.contains(chr))
		{
			valid=true;
			i++;
		}
		else valid=false;

		/* Validation of UTF8 charactes (2 and 3 bytes long).
		Reference: http://www.fileformat.info/info/unicode/utf8.htm
								http://en.wikipedia.org/wiki/UTF-8

		Snippet extracted from the above url:

		The value of each individual byte indicates its UTF-8 function, as follows:
		00 to 7F hex (0 to 127): first and only byte of a sequence.
		80 to BF hex (128 to 191): continuing byte in a multi-byte sequence.
		C2 to DF hex (194 to 223): first byte of a two-byte sequence.
		E0 to EF hex (224 to 239): first byte of a three-byte sequence.  */
		if(!valid && (i < len-1))
		{
			chr1=raw_name[i+1];

			if((i + 2) <= (len-1))
				chr2=raw_name[i+2];
			else
				chr2=0;

			//UTF-8 character with 2 bytes length
			if((chr  >= 0xC2 && chr <= 0xDF &&
				chr1 >= 0x80 && chr1 <= 0xBF) ||

					//UTF-8 character with 3 bytes length
					(chr  >= 0xE0 && chr <= 0xEF &&
						chr1 >= 0x80 && chr1 <= 0xBF &&
						chr2 >= 0x80 && chr2 <= 0xBF))
				valid=true;

			//Increments the counter in the size of the validated char
			if(chr >= 0xC2 && chr <= 0xDF)
				//2 bytes char
				i+=2;
			else
				//3 bytes char
				i+=3;
		}
	}

	return valid;
}

void BaseObject::setDatabase(BaseObject *db)
{
	if((db && db->getObjectType()==ObjectType::Database) || !db)
		this->database=db;
}

BaseObject *BaseObject::getDatabase()
{
	return this->database;
}

void BaseObject::setProtected(bool value)
{
	setCodeInvalidated(this->is_protected != value);
	is_protected=value;
}

void BaseObject::setName(const QString &name)
{
	QString aux_name=name;
	bool is_quoted=aux_name.contains(QRegularExpression("^(\")(.)+(\")$"));

	//Raises an error if the passed name is invalid
	if(!isValidName(aux_name))
	{
		if(aux_name.isEmpty())
			throw Exception(ErrorCode::AsgEmptyNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		//If the name is quoted we add 2 bytes to the maximum in order to check if it exceeds the limit
		if(aux_name.size() > (ObjectNameMaxLength + (is_quoted ? 2 : 0)))
			throw Exception(ErrorCode::AsgLongNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	aux_name.remove('"');
	setCodeInvalidated(this->obj_name!=aux_name);
	this->obj_name=aux_name;
}

void BaseObject::setAlias(const QString &alias)
{
	if(alias.size() > ObjectNameMaxLength)
		throw Exception(ErrorCode::AsgLongNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->alias = alias;
	setCodeInvalidated(this->alias != alias);
}

void BaseObject::setComment(const QString &comment)
{
	setCodeInvalidated(this->comment!=comment);
	this->comment=comment;
}

bool BaseObject::acceptsSchema(ObjectType obj_type)
{
	return (obj_type==ObjectType::Function || obj_type==ObjectType::Table ||
			 obj_type==ObjectType::View  || obj_type==ObjectType::Domain ||
			 obj_type==ObjectType::Aggregate || obj_type==ObjectType::Operator ||
			 obj_type==ObjectType::Sequence || obj_type==ObjectType::Conversion ||
			 obj_type==ObjectType::Type || obj_type==ObjectType::OpClass ||
			 obj_type==ObjectType::OpFamily || obj_type==ObjectType::Collation ||
			 obj_type==ObjectType::Extension || obj_type==ObjectType::ForeignTable ||
			 obj_type == ObjectType::Procedure);
}

bool BaseObject::acceptsSchema()
{
	return BaseObject::acceptsSchema(this->obj_type);
}

bool BaseObject::acceptsOwner(ObjectType obj_type)
{
	return (obj_type==ObjectType::Function || obj_type==ObjectType::Table ||
			 obj_type==ObjectType::Domain || obj_type==ObjectType::Schema ||
			 obj_type==ObjectType::Aggregate || obj_type==ObjectType::Operator ||
			 obj_type==ObjectType::Conversion || obj_type==ObjectType::Sequence ||
			 obj_type==ObjectType::Language || obj_type==ObjectType::Type ||
			 obj_type==ObjectType::Tablespace || obj_type==ObjectType::Database ||
			 obj_type==ObjectType::OpClass || obj_type==ObjectType::OpFamily ||
			 obj_type==ObjectType::Collation  || obj_type==ObjectType::View ||
			 obj_type==ObjectType::EventTrigger || obj_type==ObjectType::ForeignDataWrapper  ||
			 obj_type==ObjectType::ForeignServer || obj_type==ObjectType::UserMapping ||
			 obj_type==ObjectType::ForeignTable || obj_type==ObjectType::Procedure);
}

bool BaseObject::acceptsOwner()
{
	return BaseObject::acceptsOwner(this->obj_type);
}

bool BaseObject::acceptsTablespace(ObjectType obj_type)
{
	return (obj_type==ObjectType::Index ||
			 obj_type==ObjectType::Table ||
			 obj_type==ObjectType::View ||
			 obj_type==ObjectType::Constraint ||
			 obj_type==ObjectType::Database);
}

bool BaseObject::acceptsTablespace()
{
	return BaseObject::acceptsTablespace(this->obj_type);
}

bool BaseObject::acceptsCollation(ObjectType obj_type)
{
	return (obj_type==ObjectType::Domain || obj_type==ObjectType::Column  ||
			 obj_type==ObjectType::Collation || obj_type==ObjectType::Type ||
			 obj_type==ObjectType::TypeAttribute);
}

bool BaseObject::acceptsCollation()
{
	return BaseObject::acceptsCollation(this->obj_type);
}

bool BaseObject::acceptsCustomSQL(ObjectType obj_type)
{
	return (obj_type!=ObjectType::Column && obj_type!=ObjectType::Constraint &&
				 obj_type!=ObjectType::Relationship && obj_type!=ObjectType::Textbox && obj_type!=ObjectType::Parameter &&
				 obj_type!=ObjectType::TypeAttribute && obj_type!=ObjectType::BaseRelationship  &&
				 obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::BaseTable && obj_type!=ObjectType::Permission &&
				 obj_type!=ObjectType::Tag && obj_type!=ObjectType::GenericSql);
}

bool BaseObject::acceptsAlterCommand(ObjectType obj_type)
{
	return (obj_type==ObjectType::Collation || obj_type==ObjectType::Column ||
				 obj_type==ObjectType::Domain || obj_type==ObjectType::EventTrigger ||
				 obj_type==ObjectType::Extension || obj_type==ObjectType::Function ||
				 obj_type==ObjectType::Index || obj_type==ObjectType::Role ||
				 obj_type==ObjectType::Schema || obj_type==ObjectType::Sequence ||
				 obj_type==ObjectType::Table || obj_type==ObjectType::Tablespace ||
				 obj_type==ObjectType::Type || obj_type==ObjectType::Policy ||
				 obj_type==ObjectType::ForeignDataWrapper || obj_type==ObjectType::ForeignServer ||
				 obj_type==ObjectType::UserMapping || obj_type==ObjectType::ForeignTable ||
				 obj_type==ObjectType::Procedure);
}

bool BaseObject::acceptsDropCommand(ObjectType obj_type)
{
	return (obj_type!=ObjectType::Permission && obj_type!=ObjectType::Relationship &&
				obj_type!=ObjectType::Textbox && obj_type!=ObjectType::TypeAttribute &&
				obj_type!=ObjectType::Parameter && obj_type!=ObjectType::BaseObject &&
				obj_type!=ObjectType::Tag && obj_type!=ObjectType::BaseRelationship &&
				obj_type!=ObjectType::BaseTable);
}

bool BaseObject::acceptsReplaceCommand(ObjectType obj_type)
{
	return obj_type == ObjectType::Aggregate || obj_type == ObjectType::Function ||
				 obj_type == ObjectType::Language || obj_type == ObjectType::Procedure ||
				 obj_type == ObjectType::Rule || obj_type == ObjectType::Transform ||
				 obj_type == ObjectType::Trigger || obj_type == ObjectType::View;
}

bool BaseObject::acceptsAlias(ObjectType obj_type)
{
	return (obj_type==ObjectType::Relationship || obj_type==ObjectType::BaseRelationship ||
				 obj_type==ObjectType::Table || obj_type==ObjectType::Schema || obj_type==ObjectType::View ||
				 obj_type == ObjectType::Column || obj_type == ObjectType::Constraint ||
				 obj_type == ObjectType::Index || obj_type == ObjectType::Rule ||
				 obj_type == ObjectType::Trigger || obj_type == ObjectType::Policy ||
					obj_type==ObjectType::ForeignTable);
}

bool BaseObject::acceptsComment(ObjectType obj_type)
{
	return obj_type!=ObjectType::Relationship && obj_type!=ObjectType::Textbox &&
				 obj_type!=ObjectType::Parameter && obj_type!=ObjectType::UserMapping &&
				 obj_type!=ObjectType::Permission;
}

bool BaseObject::acceptsCustomSQL()
{
	return BaseObject::acceptsCustomSQL(this->obj_type);
}

bool BaseObject::acceptsAlterCommand()
{
	return BaseObject::acceptsAlterCommand(this->obj_type);
}

bool BaseObject::acceptsDropCommand()
{
	return BaseObject::acceptsDropCommand(this->obj_type);
}

bool BaseObject::acceptsReplaceCommand()
{
	return BaseObject::acceptsReplaceCommand(this->obj_type);
}

bool BaseObject::acceptsAlias()
{
	return BaseObject::acceptsAlias(this->obj_type);
}

bool BaseObject::acceptsComment()
{
	return BaseObject::acceptsComment(this->obj_type);
}

void BaseObject::setPgOid(unsigned int oid)
{
	pg_oid = oid;
}

unsigned int BaseObject::getPgOid()
{
	return pg_oid;
}

void BaseObject::setSchema(BaseObject *schema)
{
	if(!schema)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::AsgNotAllocatedSchema)
										.arg(this->obj_name, this->getTypeName()),
						ErrorCode::AsgNotAllocatedSchema, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
	
	if(schema && schema->getObjectType()!=ObjectType::Schema)
		throw Exception(ErrorCode::AsgInvalidSchemaObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!acceptsSchema())
		throw Exception(ErrorCode::AsgInvalidSchemaObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->schema != schema);
	this->schema = schema;
}

void BaseObject::setOwner(BaseObject *owner)
{
	if(owner && owner->getObjectType()!=ObjectType::Role)
		throw Exception(ErrorCode::AsgInvalidRoleObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!acceptsOwner())
		throw Exception(ErrorCode::AsgRoleObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->owner != owner);
	this->owner = owner;
}

void BaseObject::setTablespace(BaseObject *tablespace)
{
	if(tablespace && tablespace->getObjectType()!=ObjectType::Tablespace)
		throw Exception(ErrorCode::AsgInvalidTablespaceObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!acceptsTablespace())
		throw Exception(ErrorCode::AsgTablespaceInvalidObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->tablespace != tablespace);
	this->tablespace = tablespace;
}

void BaseObject::setCollation(BaseObject *collation)
{
	if(collation && !acceptsCollation())
		throw Exception(ErrorCode::AsgInvalidCollationObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	if(collation && collation->getObjectType()!=ObjectType::Collation)
		throw Exception(ErrorCode::AsgInvalidCollationObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->collation != collation);
	this->collation = collation;
}

void BaseObject::setAppendedSQL(const QString &sql)
{
	if(!acceptsCustomSQL())
		throw Exception(ErrorCode::AsgCustomSQLObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->appended_sql != sql);
	this->appended_sql=sql;
}

void BaseObject::setPrependedSQL(const QString &sql)
{
	if(!acceptsCustomSQL())
		throw Exception(ErrorCode::AsgCustomSQLObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(this->prepended_sql != sql);
	this->prepended_sql=sql;
}

QString BaseObject::getName(bool format, bool prepend_schema)
{
	CachedNameId name_id = RawName;

	if(format && !prepend_schema)
		name_id = FmtName;
	else if(format && prepend_schema)
		name_id = Signature;

	/* If the schema name have changed somewhere we need to invalidate
	 * the cache for the object's signature so it is generated again */
	if(schema && name_id == Signature &&
			!cached_names[name_id].isEmpty() &&
			!cached_names[name_id].startsWith(schema->getName(true)))
		cached_names[name_id].clear();

	if(!cached_names[name_id].isEmpty())
		return cached_names[name_id];

	if(format)
	{
		QString aux_name = formatName(obj_name, (obj_type == ObjectType::Operator));

		if(schema && prepend_schema)
			aux_name = formatName(schema->getName(format)) + QChar('.') + aux_name;

		if(!aux_name.isEmpty())
		{
			cached_names[name_id] = aux_name;
			return aux_name;
		}
	}

	cached_names[name_id] = obj_name;
	return obj_name;
}

QString BaseObject::getAlias()
{
	return this->alias;
}

QString BaseObject::getSignature(bool format)
{
	return this->getName(format, true);
}

QString BaseObject::getComment()
{
	return comment;
}

QString BaseObject::getEscapedComment(bool escape_special_chars)
{
	QString fmt_comm = comment.trimmed();

	if(escape_special_chars)
	{
		fmt_comm.replace(QChar('\\'), "\\\\");
		fmt_comm.replace(QChar::LineFeed, "\\n");
		fmt_comm.replace(QChar::Tabulation, "\\t");
	}

	fmt_comm.replace(QChar('\''), "''");
	return fmt_comm;
}

BaseObject *BaseObject::getSchema()
{
	return schema;
}

BaseObject *BaseObject::getOwner()
{
	return owner;
}

BaseObject *BaseObject::getTablespace()
{
	return tablespace;
}

BaseObject *BaseObject::getCollation()
{
	return collation;
}

QString BaseObject::getAppendedSQL()
{
	return appended_sql;
}

QString BaseObject::getPrependedSQL()
{
	return prepended_sql;
}

ObjectType BaseObject::getObjectType()
{
	return obj_type;
}

QString BaseObject::getTypeName()
{
	return BaseObject::getTypeName(this->obj_type);
}

QString BaseObject::getSchemaName()
{
	return BaseObject::getSchemaName(this->obj_type);
}

QString BaseObject::getSQLName()
{
	return BaseObject::getSQLName(this->obj_type);
}

bool BaseObject::isProtected()
{
	return is_protected;
}

unsigned BaseObject::getObjectId()
{
	return object_id;
}

void BaseObject::setSQLDisabled(bool value)
{
	setCodeInvalidated(this->sql_disabled != value);
	sql_disabled=value;
}

bool BaseObject::isSQLDisabled()
{
	return sql_disabled;
}

void BaseObject::setSystemObject(bool value)
{
	setCodeInvalidated(this->system_obj != value);
	system_obj=sql_disabled=value;
}

bool BaseObject::isSystemObject()
{
	return system_obj;
}

void BaseObject::setBasicAttributes(bool format_name)
{
	if(attributes[Attributes::Name].isEmpty())
		attributes[Attributes::Name] = getName(format_name);

	if(attributes[Attributes::Alias].isEmpty())
		attributes[Attributes::Alias] = getAlias();

	if(attributes[Attributes::Signature].isEmpty())
		attributes[Attributes::Signature] = getSignature(format_name);

	if(attributes[Attributes::SqlObject].isEmpty())
		attributes[Attributes::SqlObject] = objs_sql[enum_t(this->obj_type)];
}

QString BaseObject::__getSourceCode(SchemaParser::CodeType def_type)
{
	return BaseObject::getSourceCode(def_type, false);
}

QString BaseObject::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def;

	if((def_type==SchemaParser::SqlCode &&
		obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::BaseRelationship &&
		obj_type!=ObjectType::BaseTable && obj_type!=ObjectType::Textbox) ||

			(def_type==SchemaParser::XmlCode &&
			 obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::BaseTable))
	{
		bool format=false;

		schparser.setPgSQLVersion(BaseObject::pgsql_ver, ignore_db_version);
		attributes[Attributes::SqlDisabled]=(sql_disabled ? Attributes::True : "");

		//Formats the object's name in case the SQL definition is being generated
		format=((def_type==SchemaParser::SqlCode) ||
				(def_type==SchemaParser::XmlCode && reduced_form &&
				 obj_type!=ObjectType::Textbox && obj_type!=ObjectType::Relationship));

		setBasicAttributes(format);

		if(schema)
		{
			if(def_type==SchemaParser::XmlCode)
				attributes[Attributes::Schema]=schema->getSourceCode(def_type, true);
			else
				attributes[Attributes::Schema]=schema->getName(format);
		}

		if(def_type==SchemaParser::XmlCode)
			attributes[Attributes::Protected]=(is_protected ? Attributes::True : "");

		if(tablespace)
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::Tablespace]=tablespace->getName(format);
			else
				attributes[Attributes::Tablespace]=tablespace->getSourceCode(def_type, true);
		}

		if(collation && attributes[Attributes::Collation].isEmpty())
		{
			if(def_type==SchemaParser::SqlCode)
				attributes[Attributes::Collation]=collation->getName(format);
			else
				attributes[Attributes::Collation]=collation->getSourceCode(def_type, true);
		}

		if(owner)
		{
			if(def_type==SchemaParser::SqlCode)
			{
				attributes[Attributes::Owner] = owner->getName(format);

				/* Only tablespaces, database and user mapping do not have an ALTER OWNER SET
				 because the rule says that PostgreSQL tablespaces and database should be created
				 with just a command line isolated from the others */
				if(obj_type != ObjectType::Tablespace && obj_type != ObjectType::Database && obj_type != ObjectType::UserMapping)
				{
					SchemaParser sch_parser;
					QString filename=GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, Attributes::Owner);

					sch_parser.ignoreUnkownAttributes(true);
					attributes[Attributes::Owner]=sch_parser.getSourceCode(filename, attributes);
				}
			}
			else
				attributes[Attributes::Owner]=owner->getSourceCode(def_type, true);
		}

		if(!comment.isEmpty())
		{
			if(def_type==SchemaParser::SqlCode)
			{
				QString escape_comm = getEscapedComment(escape_comments);
				attributes[Attributes::EscapeComment]=escape_comments ? Attributes::True : "";
				attributes[Attributes::Comment]=escape_comm;
			}
			else
				attributes[Attributes::Comment]=comment;

			schparser.ignoreUnkownAttributes(true);

			attributes[Attributes::Comment]=
					schparser.getSourceCode(Attributes::Comment, attributes, def_type);
		}

		if(!appended_sql.isEmpty())
		{
			attributes[Attributes::AppendedSql]=appended_sql;

			if(def_type==SchemaParser::XmlCode)
			{
				schparser.ignoreUnkownAttributes(true);
				attributes[Attributes::AppendedSql]=
						schparser.getSourceCode(QString(Attributes::AppendedSql).remove('-'), attributes, def_type);
			}
			else
			{
				attributes[Attributes::AppendedSql]="\n-- Appended SQL commands --\n" + appended_sql;
			}
		}

		if(!prepended_sql.isEmpty())
		{
			attributes[Attributes::PrependedSql]=prepended_sql;

			if(def_type==SchemaParser::XmlCode)
			{
				schparser.ignoreUnkownAttributes(true);
				attributes[Attributes::PrependedSql]=
						schparser.getSourceCode(QString(Attributes::PrependedSql).remove('-'), attributes, def_type);
			}
			else
			{
				attributes[Attributes::PrependedSql]="\n-- Prepended SQL commands --\n" + prepended_sql;
			}
		}

		if(def_type==SchemaParser::SqlCode && this->acceptsDropCommand())
		{
			attributes[Attributes::Drop]=getDropCode(true);
			attributes[Attributes::Drop].remove(Attributes::DdlEndToken + '\n');
		}

		attributes[Attributes::ReducedForm]=(reduced_form ? Attributes::True : "");

		try
		{
			code_def+=schparser.getSourceCode(objs_schemas[enum_t(obj_type)], attributes, def_type);

			//Internally disabling the SQL definition
			if(sql_disabled && def_type==SchemaParser::SqlCode)
			{
				//Creates a text stream and insert an comment start token on each line
				QTextStream ts(&code_def);
				QString buf;

				while(!ts.atEnd())
					buf+=QString("-- %1\n").arg(ts.readLine());

				//The entire commented buffer will be returned
				code_def=buf;
			}

			clearAttributes();

			//Database object doesn't handles cached code.
			if(obj_type != ObjectType::Database)
			{
				if(def_type==SchemaParser::SqlCode ||	(!reduced_form && def_type==SchemaParser::XmlCode))
					cached_code[def_type] = code_def;
				else if(reduced_form)
					cached_reduced_code = code_def;
			}

			code_invalidated = false;
		}
		catch(Exception &e)
		{
			schparser.restartParser();
			clearAttributes();

			if(e.getErrorCode() == ErrorCode::UndefinedAttributeValue)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
												.arg(this->getName(true))
												.arg(this->getTypeName()),
												ErrorCode::AsgObjectInvalidDefinition, PGM_FUNC, PGM_FILE, PGM_LINE, &e);
			}

			throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
		}
	}

	return code_def;
}

void BaseObject::setAttribute(const QString &attrib, const QString &value)
{
	attributes[attrib]=value;
}

void BaseObject::clearAttributes()
{
	attribs_map::iterator itr, itr_end;

	itr=attributes.begin();
	itr_end=attributes.end();

	while(itr!=itr_end)
	{
		itr->second="";
		itr++;
	}
}

void BaseObject::swapObjectsIds(BaseObject *obj1, BaseObject *obj2, bool enable_cl_obj_swap)
{
	//Raises an error if some of the objects aren't allocated
	if(!obj1 || !obj2)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the involved objects are the same
	if(obj1==obj2)
		throw Exception(ErrorCode::InvIdSwapSameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Raises an error if the some of the objects are system objects
	if(obj1->isSystemObject())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
						.arg(obj1->getName())
						.arg(obj1->getTypeName()),
						ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(obj2->isSystemObject())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
						.arg(obj2->getName())
						.arg(obj2->getTypeName()),
						ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	//Raises an error if the object is object is cluster level and the swap of these types isn't enabled
	if(!enable_cl_obj_swap &&
		 (obj1->getObjectType()==ObjectType::Database ||
			obj1->getObjectType()==ObjectType::Tablespace ||
			obj1->getObjectType()==ObjectType::Role) &&

		 obj1->getObjectType() != obj2->getObjectType())
	{
		throw Exception(ErrorCode::InvIdSwapInvalidObjectType,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	unsigned id_bkp=obj1->object_id;
	obj1->object_id=obj2->object_id;
	obj2->object_id=id_bkp;
}

void BaseObject::updateObjectId(BaseObject *obj)
{
	//Raises an error if some of the objects aren't allocated
	if(!obj)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(obj->isSystemObject())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
										.arg(obj->getName())
										.arg(obj->getTypeName()),
										ErrorCode::OprReservedObject, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
	
	obj->object_id=++global_id;
}

std::vector<ObjectType> BaseObject::getObjectTypes(bool inc_table_objs, std::vector<ObjectType> exclude_types)
{
	std::vector<ObjectType> vet_types={ ObjectType::BaseRelationship, ObjectType::Aggregate, ObjectType::Cast, ObjectType::Collation,
									 ObjectType::Conversion, ObjectType::Database, ObjectType::Domain, ObjectType::Extension, ObjectType::EventTrigger,
									 ObjectType::ForeignDataWrapper, ObjectType::ForeignServer, ObjectType::Function, ObjectType::Procedure, ObjectType::GenericSql,
									 ObjectType::Language, ObjectType::OpClass, ObjectType::Operator, ObjectType::OpFamily, ObjectType::Permission,
									 ObjectType::Relationship, ObjectType::Role, ObjectType::Schema, ObjectType::Sequence, ObjectType::Table, ObjectType::Tablespace,
									 ObjectType::Tag, ObjectType::Textbox, ObjectType::Type, ObjectType::UserMapping, ObjectType::View, ObjectType::ForeignTable, ObjectType::Transform };
	std::vector<ObjectType>::iterator itr;

	if(inc_table_objs)
	{
		vet_types.push_back(ObjectType::Column);
		vet_types.push_back(ObjectType::Constraint);
		vet_types.push_back(ObjectType::Trigger);
		vet_types.push_back(ObjectType::Rule);
		vet_types.push_back(ObjectType::Index);
		vet_types.push_back(ObjectType::Policy);
	}

	for(auto &type : exclude_types)
	{
		itr=std::remove(vet_types.begin(), vet_types.end(), type);
		if(itr!=vet_types.end())
			vet_types.erase(itr);
	}

	return vet_types;
}

std::vector<ObjectType> BaseObject::getChildObjectTypes(ObjectType obj_type)
{
	if(obj_type == ObjectType::Database)
	{
		static std::vector<ObjectType> db_child { ObjectType::Cast, ObjectType::Role, ObjectType::Language,
																							ObjectType::Tablespace, ObjectType::Schema, ObjectType::Extension,
																							ObjectType::EventTrigger, ObjectType::ForeignDataWrapper, ObjectType::ForeignServer,
																							ObjectType::UserMapping, ObjectType::Transform };
		return db_child;
	}

	if(obj_type == ObjectType::Schema)
	{
		static std::vector<ObjectType> sch_child { ObjectType::Aggregate, ObjectType::Conversion, ObjectType::Collation,
																							 ObjectType::Domain, ObjectType::ForeignTable, ObjectType::Function, ObjectType::OpClass,
																							 ObjectType::Operator, ObjectType::OpFamily, ObjectType::Procedure, ObjectType::Sequence,
																							 ObjectType::Type, ObjectType::Table, ObjectType::View };
		return sch_child;
	}

	if(obj_type == ObjectType::Table)
	{
		static std::vector<ObjectType> tb_child { ObjectType::Column, ObjectType::Constraint, ObjectType::Rule,
																							ObjectType::Trigger, ObjectType::Index, ObjectType::Policy };
		return tb_child;
	}

	if(obj_type == ObjectType::ForeignTable)
	{
		static std::vector<ObjectType> ft_child {	ObjectType::Column, ObjectType::Constraint, ObjectType::Trigger };
		return ft_child;
	}

	if(obj_type==ObjectType::View)
	{
		static std::vector<ObjectType> vw_child {ObjectType::Rule, ObjectType::Trigger, ObjectType::Index};
		return vw_child;
	}

	return {};
}

bool BaseObject::isChildObjectType(ObjectType parent_type, ObjectType child_type)
{
	std::vector<ObjectType> types = getChildObjectTypes(parent_type);
	return std::find(types.begin(), types.end(), child_type) != types.end();
}

void BaseObject::setPgSQLVersion(const QString &version)
{
	try
	{
		pgsql_ver = PgSqlVersions::parseString(version, ignore_db_version);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString BaseObject::getPgSQLVersion()
{
	return pgsql_ver;
}

attribs_map BaseObject::getSearchAttributes()
{
	return search_attribs;
}

void BaseObject::setIgnoreDbVersion(bool ignore)
{
	ignore_db_version = ignore;
}

bool BaseObject::isDbVersionIgnored()
{
	return ignore_db_version;
}

QString BaseObject::getSearchAttributeI18N(const QString &search_attr)
{
	if(!search_attribs_i18n.count(search_attr))
		return "";

	return search_attribs_i18n.at(search_attr);
}

QStringList BaseObject::getSearchAttributesNames()
{
	return search_attribs_names;
}

void BaseObject::operator = (BaseObject &obj)
{
	//clearDependencies();
	this->owner=obj.owner;
	this->schema=obj.schema;
	this->tablespace=obj.tablespace;
	this->database=obj.database;
	this->comment=obj.comment;
	this->obj_name=obj.obj_name;
	this->alias=obj.alias;
	this->obj_type=obj.obj_type;
	this->is_protected=obj.is_protected;
	this->sql_disabled=obj.sql_disabled;
	this->system_obj=obj.system_obj;
	this->setCodeInvalidated(true);
	//updateDependencies();
}

void BaseObject::setCodeInvalidated(bool value)
{
	if(value != code_invalidated)
	{
		if(value)
		{
			cached_reduced_code.clear();
			cached_code[SchemaParser::SqlCode].clear();
			cached_code[SchemaParser::XmlCode].clear();
		}

		code_invalidated=value;
	}

	if(value)
	{
		cached_names[RawName].clear();
		cached_names[FmtName].clear();
		cached_names[Signature].clear();
	}
}

void BaseObject::configureSearchAttributes()
{
	search_attribs[Attributes::Name] = this->getName(false);
	search_attribs[Attributes::Signature] = this->getSignature(true);
	search_attribs[Attributes::Schema] = schema ? schema->getName(false) : "";
	search_attribs[Attributes::Tablespace] = tablespace ? tablespace->getName(false) : "";
	search_attribs[Attributes::Owner] = owner ? owner->getName(false) : "";
	search_attribs[Attributes::Comment] = comment;
}

bool BaseObject::isCodeInvalidated()
{
	return code_invalidated;
}

bool BaseObject::isCodeDiffersFrom(const QString &xml_def1, const QString &xml_def2, const QStringList &ignored_attribs, const QStringList &ignored_tags)
{
	QString xml, tag=QString("<%1").arg(this->getSchemaName()),
			attr_regex=QString("(%1=\")"),
			tag_regex=QString("<%1[^>]*((/>)|(>((?:(?!</%1>).)*)</%1>))");
	QStringList xml_defs{ xml_def1, xml_def2 };
	int start=0, end=-1, tag_end=-1;
	QRegularExpression regexp, tag_end_regexp("(\\\\)?(>)");
	QRegularExpressionMatch match;

	for(int i=0; i < 2; i++)
	{
		xml=xml_defs[i].simplified();

		//Removing ignored attributes
		for(auto &attr : ignored_attribs)
		{
			do
			{
				regexp.setPattern(attr_regex.arg(attr));
				tag_end = xml.indexOf(tag_end_regexp);

				match = regexp.match(xml);
				start = match.capturedStart();
				end=xml.indexOf('"', start + match.capturedLength());

				if(end > tag_end)
					end=-1;

				if(start >=0 && end >=0)
					xml.remove(start, (end - start) + 1);
			}
			while(start >= 0 && end >= 0);
		}

		//Removing ignored tags
		for(auto &tag : ignored_tags)
			xml.remove(QRegularExpression(tag_regex.arg(tag)));

		xml_defs[i]=xml.simplified();
	}

	return (xml_defs[0]!=xml_defs[1]);
}

bool BaseObject::isCodeDiffersFrom(BaseObject *object, const QStringList &ignored_attribs, const QStringList &ignored_tags)
{
	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(object->getObjectType() != this->getObjectType())
		throw Exception(ErrorCode::OprObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);

	try
	{
		return BaseObject::isCodeDiffersFrom(this->getSourceCode(SchemaParser::XmlCode),
											 object->getSourceCode(SchemaParser::XmlCode),
											 ignored_attribs, ignored_tags);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString BaseObject::getCachedCode(unsigned def_type, bool reduced_form)
{
	if(def_type==SchemaParser::SqlCode && schparser.getPgSQLVersion()!=BaseObject::pgsql_ver)
		code_invalidated=true;

	if(!code_invalidated &&
			((!reduced_form && !cached_code[def_type].isEmpty()) ||
			 (def_type==SchemaParser::XmlCode  && reduced_form && !cached_reduced_code.isEmpty())))
	{
		if(def_type==SchemaParser::XmlCode  && reduced_form)
			return cached_reduced_code;
		
		return cached_code[def_type];
	}

	return "";
}

QString BaseObject::getDropCode(bool cascade)
{
	try
	{
		if(acceptsDropCommand())
		{
			attribs_map attribs;

			setBasicAttributes(true);
			schparser.setPgSQLVersion(BaseObject::pgsql_ver, ignore_db_version);
			schparser.ignoreUnkownAttributes(true);
			schparser.ignoreEmptyAttributes(true);

			attribs=attributes;

			/* Creating an attribute that identifies the object type in order
		 to permit conditional code generation inside the DROP script */
			if(attribs.count(this->getSchemaName())==0)
				attribs[this->getSchemaName()]=Attributes::True;

			attribs[Attributes::Cascade]=(cascade ? Attributes::True : "");

			return schparser.getSourceCode(Attributes::Drop, attribs, SchemaParser::SqlCode);
		}
		
		return "";
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString BaseObject::getAlterCode(QString sch_name, attribs_map &attribs, bool ignore_ukn_attribs, bool ignore_empty_attribs)
{
	try
	{
		SchemaParser schparser;
		QString alter_sch_file=GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, sch_name);

		schparser.setPgSQLVersion(BaseObject::pgsql_ver, ignore_db_version);
		schparser.ignoreEmptyAttributes(ignore_empty_attribs);
		schparser.ignoreUnkownAttributes(ignore_ukn_attribs);
		return schparser.getSourceCode(alter_sch_file, attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void BaseObject::copyAttributes(attribs_map &attribs)
{
	if(!attribs.empty())
	{
		attributes[Attributes::HasChanges]=Attributes::True;
		for(auto &itr : attribs)
			attributes[itr.first]=itr.second;
	}
	else
		attributes[Attributes::HasChanges]="";
}

QString BaseObject::getAlterCode(BaseObject *object)
{
	return BaseObject::getAlterCode(object, false);
}

QString BaseObject::getAlterCode(BaseObject *object, bool ignore_name_diff)
{
	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	QString alter;

	if(object->obj_type!=this->obj_type)
		throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	setBasicAttributes(true);

	try
	{
		QStringList attribs={ Attributes::Owner, Attributes::Schema, Attributes::Tablespace };
		bool accepts_obj[3]={ acceptsOwner(), acceptsSchema(), acceptsTablespace() };
		BaseObject *dep_objs[3]={ this->getOwner(), this->getSchema(), this->getTablespace() },
				*aux_dep_objs[3]={ object->getOwner(), object->getSchema(), object->getTablespace() };

		if(!ignore_name_diff && this->getName()!=object->getName())
		{
			attributes[Attributes::NewName]=object->getName(true, false);
			alter+=BaseObject::getAlterCode(Attributes::Rename, attributes, true);
			attributes[Attributes::Name]=attributes[Attributes::NewName];
			attributes[Attributes::Signature]=object->getSignature(true);
		}

		for(unsigned i=0; i < 3; i++)
		{
			if(accepts_obj[i] && dep_objs[i] && aux_dep_objs[i] &&
					dep_objs[i]->getName(true)!=aux_dep_objs[i]->getName(true))
			{
				attributes[attribs[i]]=aux_dep_objs[i]->getName(true);
				alter+=BaseObject::getAlterCode(attribs[i], attributes, true);
			}
		}

		alter += getAlterCommentDefinition(object, attributes);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}

	return alter;
}

QString BaseObject::getAlterCommentDefinition(BaseObject *object, attribs_map attributes)
{
	try
	{
		QString comm_this = this->getEscapedComment(escape_comments),
				comm_obj = object->getEscapedComment(escape_comments);

		if(comm_this != comm_obj)
		{
			if(comm_obj.isEmpty())
				attributes[Attributes::Comment]=Attributes::Unset;
			else
			{
				attributes[Attributes::EscapeComment] = escape_comments ? Attributes::True : "";
				attributes[Attributes::Comment]=comm_obj;
			}

			schparser.ignoreUnkownAttributes(true);
			schparser.ignoreEmptyAttributes(true);
			return schparser.getSourceCode(Attributes::Comment, attributes, SchemaParser::SqlCode);
		}

		return "";
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

std::vector<BaseObject *> BaseObject::getDependencies(bool inc_indirect_deps, const std::vector<ObjectType> &excl_types, bool rem_duplicates)
{
	return getLinkedObjects(ObjDependencies, inc_indirect_deps, excl_types, rem_duplicates);
}

std::vector<BaseObject*> BaseObject::getReferences(bool inc_indirect_refs, const std::vector<ObjectType> &excl_types, bool rem_duplicates)
{
	return getLinkedObjects(ObjReferences, inc_indirect_refs, excl_types, rem_duplicates);
}

std::vector<BaseObject *> BaseObject::getLinkedObjects(ObjLinkType lnk_type, bool incl_ind_links, const std::vector<ObjectType> &excl_types, bool rem_duplicates)
{
	std::vector<BaseObject *> linked_objs,
			*obj_list =	(lnk_type == ObjDependencies ? &object_deps : &object_refs);

	if(incl_ind_links)
	{
		__getLinkedObjects(lnk_type, *obj_list, linked_objs);

		if(!excl_types.empty())
			linked_objs = CoreUtilsNs::filterObjectsByType(linked_objs, excl_types);
	}

	if(!excl_types.empty())
		linked_objs = CoreUtilsNs::filterObjectsByType(*obj_list, excl_types);
	else
		linked_objs = *obj_list;

	if(rem_duplicates)
	{
		std::sort(linked_objs.begin(), linked_objs.end());
		auto end = std::unique(linked_objs.begin(), linked_objs.end());
		linked_objs.erase(end, linked_objs.end());
	}

	return linked_objs;
}

void BaseObject::__getLinkedObjects(ObjLinkType lnk_type, const std::vector<BaseObject *> &objs, std::vector<BaseObject *> &ind_links)
{
	for(auto &obj : objs)
	{
		ind_links.push_back(obj);
		__getLinkedObjects(lnk_type,
											 lnk_type == ObjDependencies ? obj->getDependencies() : obj->getReferences(),
											 ind_links);
	}
}

void BaseObject::setDependency(BaseObject* dep_obj)
{
	if(!dep_obj)
		return;

	object_deps.push_back(dep_obj);
	dep_obj->setReference(this);
}

void BaseObject::setReference(BaseObject *ref_obj)
{
	if(!ref_obj)
		return;

	object_refs.push_back(ref_obj);
}

void BaseObject::unsetReference(BaseObject *ref_obj)
{
	if(!ref_obj)
		return;

	auto itr = std::find(object_refs.begin(), object_refs.end(), ref_obj);

	if(itr != object_refs.end())
		object_refs.erase(itr);
}

void BaseObject::unsetDependency(BaseObject *dep_obj)
{
	if(!dep_obj)
		return;

	auto itr = std::find(object_deps.begin(), object_deps.end(), dep_obj);

	if(itr != object_deps.end())
	{
		dep_obj->unsetReference(this);
		object_deps.erase(itr);
	}
}

void BaseObject::setClearDepsInDtor(bool value)
{
	clear_deps_in_dtor = value;
}

void BaseObject::clearDependencies()
{
	if(object_deps.empty() && object_refs.empty())
		return;

	for(auto &obj : object_deps)
		obj->unsetReference(this);

	object_deps.clear();
}

void BaseObject::clearReferences()
{
	for(auto &obj : object_refs)
		obj->unsetDependency(this);

	object_refs.clear();
}

void BaseObject::clearAllDepsRefs()
{
	clearDependencies();
	clearReferences();
}

bool BaseObject::isReferenced()
{
	return !object_refs.empty();
}

bool BaseObject::isReferencedBy(BaseObject *ref_obj)
{
	return std::find(object_refs.begin(), object_refs.end(), ref_obj) != object_refs.end();
}

bool BaseObject::hasDependencies()
{
	return !object_deps.empty();
}

bool BaseObject::isDependingOn(BaseObject *dep_obj)
{
	return std::find(object_deps.begin(), object_deps.end(), dep_obj) != object_deps.end();
}

void BaseObject::updateDependencies()
{
	updateDependencies({});
}

void BaseObject::updateDependencies(const std::vector<BaseObject *> &dep_objs, const std::vector<BaseObject *> &old_deps)
{
	/* If the custom list of old dependencies is emtpy, by default,
	 * the current dependencies will be unset */
	if(old_deps.empty())
		clearDependencies();
	// Otherwise we unset only the dependencies provided in old_deps
	else
	{
		for(auto &old_dep : old_deps)
			unsetDependency(old_dep);
	}

	std::vector<BaseObject *> aux_deps = {
		schema, tablespace, owner, collation
	};

	aux_deps.insert(aux_deps.end(), dep_objs.begin(), dep_objs.end());

	for(auto &obj : aux_deps)
	{
		if(!obj)
			continue;

		setDependency(obj);
	}
}
