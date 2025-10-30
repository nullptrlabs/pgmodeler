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

#include "coreutilsns.h"
#include <QDebug>
#include <QHash>
#include "relationship.h"
#include "cast.h"
#include "domain.h"
#include "aggregate.h"
#include "language.h"
#include "sequence.h"
#include "type.h"
#include "conversion.h"
#include "view.h"
#include "eventtrigger.h"
#include "genericsql.h"
#include "extension.h"
#include "foreignserver.h"
#include "foreigntable.h"
#include "transform.h"
#include "procedure.h"
#include "usermapping.h"
#include "tablespace.h"
#include "schema.h"

namespace CoreUtilsNs {

	std::vector<BaseObject *> filterObjectsByType(const std::vector<BaseObject *> &list, const std::vector<ObjectType> &excl_types)
	{
		if(excl_types.empty() || list.empty())
			return list;

		std::vector<BaseObject *> filtered_objs;

		/* Using std::copy_if to include in the filtered_obj list only the object
		 * which type is not in excl_types */
		std::copy_if(list.begin(), list.end(), std::back_inserter(filtered_objs),
								 [&excl_types](BaseObject *obj)
								 {
									return obj &&
												 std::find(excl_types.begin(), excl_types.end(),
																	 obj->getObjectType()) == excl_types.end();
								 });

		return filtered_objs;
	}

	template <class Class>
	void copyObject(BaseObject **psrc_obj, Class *copy_obj)
	{
		Class *orig_obj=nullptr;

		//Gets the objects stored in the pointer
		orig_obj=dynamic_cast<Class *>(*psrc_obj);

		//Raises an error if the copy object is not allocated
		if(!copy_obj)
			throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Allocates the source object if its not allocated
		if(!orig_obj)
		{
			orig_obj=new Class;
			(*psrc_obj)=orig_obj;
		}

		//Makes the copy between the objects
		(*orig_obj)=(*copy_obj);
	}

	void copyObject(BaseObject **psrc_obj, BaseObject *copy_obj, ObjectType obj_type)
	{
		switch(obj_type)
		{
			case ObjectType::Relationship:
				Relationship *rel1;
				rel1=new Relationship(dynamic_cast<Relationship *>(copy_obj));
				if(!(*psrc_obj))
					(*psrc_obj)=rel1;
				else
					(*(dynamic_cast<Relationship *>(*psrc_obj)))=(*rel1);
			break;
			case ObjectType::BaseRelationship:
				BaseRelationship *rel;
				rel=new BaseRelationship(dynamic_cast<BaseRelationship *>(copy_obj));
				if(!(*psrc_obj))
					(*psrc_obj)=rel;
				else
					(*(dynamic_cast<BaseRelationship *>(*psrc_obj)))=(*rel);
			break;
			case ObjectType::Column:
				copyObject(psrc_obj, dynamic_cast<Column *>(copy_obj));
			break;
			case ObjectType::Constraint:
				copyObject(psrc_obj, dynamic_cast<Constraint *>(copy_obj));
			break;
			case ObjectType::Trigger:
				copyObject(psrc_obj, dynamic_cast<Trigger *>(copy_obj));
			break;
			case ObjectType::Rule:
				copyObject(psrc_obj, dynamic_cast<Rule *>(copy_obj));
			break;
			case ObjectType::Index:
				copyObject(psrc_obj, dynamic_cast<Index *>(copy_obj));
			break;
			case ObjectType::Policy:
				copyObject(psrc_obj, dynamic_cast<Policy *>(copy_obj));
			break;
			case ObjectType::Table:
				copyObject(psrc_obj, dynamic_cast<Table *>(copy_obj));
			break;
			case ObjectType::Textbox:
				copyObject(psrc_obj, dynamic_cast<Textbox *>(copy_obj));
			break;
			case ObjectType::OpClass:
				copyObject(psrc_obj, dynamic_cast<OperatorClass *>(copy_obj));
			break;
			case ObjectType::Conversion:
				copyObject(psrc_obj, dynamic_cast<Conversion *>(copy_obj));
			break;
			case ObjectType::Cast:
				copyObject(psrc_obj, dynamic_cast<Cast *>(copy_obj));
			break;
			case ObjectType::Domain:
				copyObject(psrc_obj, dynamic_cast<Domain *>(copy_obj));
			break;
			case ObjectType::Tablespace:
				copyObject(psrc_obj, dynamic_cast<Tablespace *>(copy_obj));
			break;
			case ObjectType::Schema:
				copyObject(psrc_obj, dynamic_cast<Schema *>(copy_obj));
			break;
			case ObjectType::OpFamily:
				copyObject(psrc_obj, dynamic_cast<OperatorFamily *>(copy_obj));
			break;
			case ObjectType::Function:
				copyObject(psrc_obj, dynamic_cast<Function *>(copy_obj));
			break;
			case ObjectType::Aggregate:
				copyObject(psrc_obj, dynamic_cast<Aggregate *>(copy_obj));
			break;
			case ObjectType::Language:
				copyObject(psrc_obj, dynamic_cast<Language *>(copy_obj));
			break;
			case ObjectType::Operator:
				copyObject(psrc_obj, dynamic_cast<Operator *>(copy_obj));
			break;
			case ObjectType::Role:
				copyObject(psrc_obj, dynamic_cast<Role *>(copy_obj));
			break;
			case ObjectType::Sequence:
				copyObject(psrc_obj, dynamic_cast<Sequence *>(copy_obj));
			break;
			case ObjectType::Type:
				copyObject(psrc_obj, dynamic_cast<Type *>(copy_obj));
			break;
			case ObjectType::View:
				copyObject(psrc_obj, dynamic_cast<View *>(copy_obj));
			break;
			case ObjectType::Collation:
				copyObject(psrc_obj, dynamic_cast<Collation *>(copy_obj));
			break;
			case ObjectType::Extension:
				copyObject(psrc_obj, dynamic_cast<Extension *>(copy_obj));
			break;
			case ObjectType::Tag:
				copyObject(psrc_obj, dynamic_cast<Tag *>(copy_obj));
			break;
			case ObjectType::EventTrigger:
				copyObject(psrc_obj, dynamic_cast<EventTrigger *>(copy_obj));
			break;
			case ObjectType::GenericSql:
				copyObject(psrc_obj, dynamic_cast<GenericSQL *>(copy_obj));
			break;
			case ObjectType::ForeignDataWrapper:
				copyObject(psrc_obj, dynamic_cast<ForeignDataWrapper *>(copy_obj));
			break;
			case ObjectType::ForeignServer:
				copyObject(psrc_obj, dynamic_cast<ForeignServer *>(copy_obj));
			break;
			case ObjectType::UserMapping:
				copyObject(psrc_obj, dynamic_cast<UserMapping *>(copy_obj));
			break;
			case ObjectType::ForeignTable:
				copyObject(psrc_obj, dynamic_cast<ForeignTable *>(copy_obj));
			break;
			case ObjectType::Transform:
				copyObject(psrc_obj, dynamic_cast<Transform *>(copy_obj));
			break;
			case ObjectType::Procedure:
				copyObject(psrc_obj, dynamic_cast<Procedure *>(copy_obj));
			break;
			default:
				throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}

	bool isReservedKeyword(const QString &word)
	{
		static const QHash<QChar, QStringList> keywords {
			{QChar('A'), {"ALL", "ANALYSE", "ANALYZE", "AND",
										"ANY", "AS",      "ASC",     "AUTHORIZATION"}},

			{QChar('B'), {"BETWEEN", "BIGINT", "BINARY", "BIT",
										"BOOLEAN", "BOTH"}},

			{QChar('C'), {"CASE",         "CAST",         "CHAR",    "CHARACTER",
										"CHECK",        "COALESCE",     "COLLATE", "COLUMN",
										"CONSTRAINT",   "CONVERT",      "CREATE",  "CROSS",
										"CURRENT_DATE", "CURRENT_TIME", "CURRENT_TIMESTAMP", "CURRENT_USER"}},

			{QChar('D'), {"DEC",  "DECIMAL",  "DEFAULT", "DEFERRABLE",
										"DESC", "DISTINCT", "DO"}},

			{QChar('E'), {"ELSE", "END", "EXCEPT", "EXISTS",
										"EXTRACT"}},

			{QChar('F'), {"FALSE",  "FLOAT", "FOR", "FOREIGN",
										"FREEZE", "FROM",  "FULL"}},

			{QChar('G'), {"GRANT", "GROUP"}},

			{QChar('H'), {"HAVING"}},

			{QChar('I'), {"ILIKE", "IN",      "INITIALLY", "INNER",
										"INT",   "INTEGER", "INTERSECT", "INTERVAL",
										"INTO",  "IS",      "ISNULL"}},

			{QChar('J'), {"JOIN"}},

			{QChar('L'), {"LEADING",   "LEFT", "LIKE", "LIMIT",
										"LOCALTIME", "LOCALTIMESTAMP"}},

			{QChar('N'), {"NATURAL",      "NCHAR",   "NEW", "NOCREATEDB",
										"NOCREATEUSER", "NONE",    "NOT", "NOTHING",
										"NOTIFY",       "NOTNULL", "NULL", "NULLIF",
										"NUMERIC"}},

			{QChar('O'), {"OFF",   "OFFSET",  "OLD",   "ON",
										"ONLY",  "OR",      "ORDER", "OUTER",
										"OVERLAPS", "OVERLAY"}},

			{QChar('P'), {"PLACING", "POSITION", "PRIMARY"}},

			{QChar('R'), {"REAL", "REFERENCES", "RIGHT", "ROW"}},

			{QChar('S'), {"SELECT",   "SESSION_USER", "SETOF", "SIMILAR",
										"SMALLINT", "SOME",         "SUBSTRING"}},

			{QChar('T'), {"TABLE", "THEN",  "TIME",  "TIMESTAMP",
										"TO", "TRAILING", "TREAT", "TRIM",
										"TRUE"}},

			{QChar('U'), {"UNION", "UNIQUE", "USER", "USING"}},

			{QChar('V'), {"VARCHAR", "VERBOSE"}},

			{QChar('W'), {"WHEN", "WHERE"}}
		};

		if(word.isEmpty())
			return false;
		
		QChar chr = word.at(0).toUpper();

		if(!keywords.contains(chr))
			return false;
		
		return keywords[chr].contains(word.toUpper());
	}
}
