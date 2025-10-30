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
#include "catalog.h"
#include "utilsns.h"
#include "tableobject.h"

const QString Catalog::PgModelerTempDbObj {"__pgmodeler_tmp"};
const QString Catalog::EscapedNullChar {"\\000"};
const QString Catalog::QueryList {"list"};
const QString Catalog::QueryAttribs {"attribs"};
const QString Catalog::PgSqlTrue {"t"};
const QString Catalog::PgSqlFalse {"f"};
const QString Catalog::BoolField {"_bool"};
const QString Catalog::ArrayPattern {"((\\[)[0-9]+(\\:)[0-9]+(\\])=)?(\\{)((.)+(,)*)*(\\})$"};
const QString Catalog::InvFilterPattern {"__invalid__pattern__"};
const QString Catalog::AliasPlaceholder{"$alias$"};

const QString Catalog::GetExtensionObjsSql {
	"SELECT d.objid AS oid, e.extname AS name FROM pg_depend AS d \
		LEFT JOIN pg_extension AS e ON e.oid = d.refobjid \
		WHERE objid > 0 AND refobjid > 0 AND deptype='e' AND classid::regclass::text != 'pg_type'\
		ORDER BY extname;"
};

const std::map<ObjectType, QString> Catalog::oid_fields {
	{ObjectType::Database, "oid"}, {ObjectType::Role, "oid"}, {ObjectType::Schema,"oid"},
	{ObjectType::Language, "oid"}, {ObjectType::Tablespace, "oid"}, {ObjectType::Extension, "ex.oid"},
	{ObjectType::Function, "pr.oid"}, {ObjectType::Aggregate, "pr.oid"}, {ObjectType::Operator, "op.oid"},
	{ObjectType::OpClass, "op.oid"}, {ObjectType::OpFamily, "op.oid"}, {ObjectType::Collation, "cl.oid"},
	{ObjectType::Conversion, "cn.oid"}, {ObjectType::Cast, "cs.oid"}, {ObjectType::View, "vw.oid"},
	{ObjectType::Sequence, "sq.oid"}, {ObjectType::Domain, "dm.oid"}, {ObjectType::Type, "tp.oid"},
	{ObjectType::Table, "tb.oid"}, {ObjectType::Column, "cl.oid"}, {ObjectType::Constraint, "cs.oid"},
	{ObjectType::Rule, "rl.oid"}, {ObjectType::Trigger, "tg.oid"}, {ObjectType::Index, "id.indexrelid"},
	{ObjectType::EventTrigger, "et.oid"}, {ObjectType::Policy, "pl.oid"}, {ObjectType::ForeignDataWrapper, "fw.oid"},
	{ObjectType::ForeignServer, "sv.oid"}, {ObjectType::UserMapping, "um.umid"}, {ObjectType::ForeignTable, "ft.oid"},
	{ObjectType::Transform, "tr.oid"}, {ObjectType::Procedure, "pr.oid"}
};

const std::map<ObjectType, QString> Catalog::obj_relnames {
	{ObjectType::Aggregate, "pg_aggregate"},	{ObjectType::Cast, "pg_cast"},
	{ObjectType::Collation, "pg_collation"},	{ObjectType::Column, "pg_attribute"},
	{ObjectType::Constraint, "pg_constraint"},	{ObjectType::Conversion, "pg_conversion"},
	{ObjectType::Database, "pg_database"},	{ObjectType::Domain, "pg_type"},
	{ObjectType::Extension, "pg_extension"},	{ObjectType::EventTrigger, "pg_event_trigger"},
	{ObjectType::ForeignDataWrapper, "pg_foreign_data_wrapper"},	{ObjectType::ForeignTable, "pg_foreign_table"},
	{ObjectType::Function, "pg_proc"},	{ObjectType::Index, "pg_class"},
	{ObjectType::Operator, "pg_operator"},	{ObjectType::OpClass, "pg_opclass"},
	{ObjectType::OpFamily, "pg_opfamily"},	{ObjectType::Policy, "pg_policy"},
	{ObjectType::Language, "pg_language"},	{ObjectType::Procedure, "pg_proc"},
	{ObjectType::Role, "pg_authid"},	{ObjectType::Rule, "pg_rewrite"},
	{ObjectType::Schema, "pg_namespace"},	{ObjectType::Sequence, "pg_class"},
	{ObjectType::ForeignServer, "pg_foreign_server"},	{ObjectType::Table, "pg_class"},
	{ObjectType::Tablespace, "pg_tablespace"},	{ObjectType::Transform, "pg_transform"},
	{ObjectType::Trigger, "pg_trigger"},	{ObjectType::Type, "pg_type"},
	{ObjectType::View, "pg_class"}, {ObjectType::UserMapping, "pg_user_mapping"}
};

const std::map<ObjectType, QString> Catalog::name_fields {
	{ObjectType::Database, "datname"}, {ObjectType::Role, "rolname"}, {ObjectType::Schema,"nspname"},
	{ObjectType::Language, "lanname"}, {ObjectType::Tablespace, "spcname"}, {ObjectType::Extension, "extname"},
	{ObjectType::Function, "proname"}, {ObjectType::Aggregate, "proname"}, {ObjectType::Operator, "oprname"},
	{ObjectType::OpClass, "opcname"}, {ObjectType::OpFamily, "opfname"}, {ObjectType::Collation, "collname"},
	{ObjectType::Conversion, "conname"}, {ObjectType::Cast, ""}, {ObjectType::View, "relname"},
	{ObjectType::Sequence, "relname"}, {ObjectType::Domain, "typname"}, {ObjectType::Type, "typname"},
	{ObjectType::Table, "relname"}, {ObjectType::Column, "attname"}, {ObjectType::Constraint, "conname"},
	{ObjectType::Rule, "rulename"}, {ObjectType::Trigger, "tgname"}, {ObjectType::Index, "cl.relname"},
	{ObjectType::EventTrigger, "evtname"}, {ObjectType::Policy, "polname"}, {ObjectType::ForeignDataWrapper, "fdwname"},
	{ObjectType::ForeignServer, "srvname"}, {ObjectType::ForeignTable, "relname"}, {ObjectType::Transform, ""},
	{ObjectType::Procedure, "proname"}
};

const std::map<ObjectType, QString> Catalog::ext_oid_fields {
	{ObjectType::Constraint, "cs.conrelid"},
	{ObjectType::Index, "id.indexrelid"},
	{ObjectType::Trigger, "tg.tgrelid"},
	{ObjectType::Rule, "rl.ev_class"},
	{ObjectType::Policy, "pl.polrelid"}
};

const std::map<ObjectType, QString> Catalog::parent_aliases {
	{ObjectType::Constraint, "tb"},
	{ObjectType::Index, "tb"},
	{ObjectType::Trigger, "tb"},
	{ObjectType::Rule, "cl"},
	{ObjectType::Policy, "tb"}
};

attribs_map Catalog::catalog_queries {};

Catalog::Catalog()
{
	match_signature = true;
	last_sys_oid=0;
	setQueryFilter(ExclExtensionObjs | ExclSystemObjs);
}

Catalog::Catalog(const Catalog &catalog)
{
	(*this)=catalog;
}

void Catalog::setConnection(Connection &conn)
{
	try
	{
		ResultSet res;
		QStringList obj_oids;

		connection.close();
		connection.setConnectionParams(conn.getConnectionParams());
		connection.connect();

		//Retrieving the last system oid
		executeCatalogQuery(QueryList, ObjectType::Database, res, true,
		{{Attributes::Name, conn.getConnectionParam(Connection::ParamDbName)}});

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			attribs_map attribs=changeAttributeNames(res.getTupleValues());
			last_sys_oid=attribs[Attributes::LastSysOid].toUInt();
		}

		//Retrieving the list of objects created by extensions
		ext_objects.clear();
		ext_objs_oids = "";
		this->connection.executeDMLCommand(GetExtensionObjsSql, res);

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				obj_oids.append(res.getColumnValue(Attributes::Oid));
				ext_objects[res.getColumnValue(Attributes::Name)].append(res.getColumnValue(Attributes::Oid));
			}
			while(res.accessTuple(ResultSet::NextTuple));

			ext_objs_oids = obj_oids.join(',');
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Catalog::closeConnection()
{
	connection.close();
}

bool Catalog::isConnectionValid()
{
	return connection.isConfigured();
}

void Catalog::setQueryFilter(QueryFilter filter)
{
	bool list_all=(ListAllObjects & filter) == ListAllObjects;

	this->filter=filter;
	list_only_sys_objs=false;
	exclude_array_types=(ExclBuiltinArrayTypes & filter) == ExclBuiltinArrayTypes;
	exclude_ext_objs=(ExclExtensionObjs & filter) == ExclExtensionObjs;
	exclude_sys_objs=(ExclSystemObjs & filter) == ExclSystemObjs;

	if(!list_all)
	{
		list_only_sys_objs=(ListOnlySystemObjs & filter) == ListOnlySystemObjs;

		if(list_only_sys_objs)
		{
			exclude_ext_objs=true;
			exclude_sys_objs=false;
		}
	}
}

void Catalog::setObjectFilters(QStringList filters, bool only_matching, bool match_signature, QStringList tab_obj_types)
{
	this->match_signature = match_signature;
	obj_filters.clear();
	extra_filter_conds.clear();

	if(filters.isEmpty())
		return;

	QStringList any_filters, filterable_types = getFilterableObjectNames();
	QRegularExpression any_flt_regex = QRegularExpression(QString("^(%1)(.)+").arg(Attributes::Any));

	any_filters = filters.filter(any_flt_regex);

	for(auto &filter : any_filters)
	{
		/* If there's at least one "any" filter we create filters
		 * for all object types using the pattern associated to the "any" filter */
		for(auto &type : filterable_types)
			filters.append(QString(filter).replace(Attributes::Any, type));
	}

	// Remove the any filters after converting them to object filters
	for(auto &any_flt : any_filters)
		filters.removeAll(any_flt);

	ObjectType obj_type;
	QString pattern, mode, parent_alias_ref, tab_filter = "^(%1)(.)+";
	QStringList values,	modes = { UtilsNs::FilterWildcard, UtilsNs::FilterRegExp };
	std::map<ObjectType, QStringList> tab_patterns;
	std::map<ObjectType, QStringList> parsed_filters;

	bool has_tab_filter = filters.indexOf(QRegularExpression(tab_filter.arg(BaseObject::getSchemaName(ObjectType::Table)))) >= 0,
			 has_view_filter = filters.indexOf(QRegularExpression(tab_filter.arg(BaseObject::getSchemaName(ObjectType::View)))) >= 0,
			 has_ftab_filter = filters.indexOf(QRegularExpression(tab_filter.arg(BaseObject::getSchemaName(ObjectType::ForeignTable)))) >= 0;

	/* If we have at least one table (view or foreign table) filter
	 * and the forced object types list we configure filters to force the
	 * listing of table children objects, tied to the filters that list
	 * the parent tables */
	if(!tab_obj_types.isEmpty() && (has_tab_filter || has_ftab_filter || has_view_filter))
	{
		/* If the match_signature is set we need to use this extra string to reference parent's schema
		 * in order to format its signature in the children catalog queries */
		QString parent_sch_ref("ns.nspname || '.' ||");

		/* Configuring the placeholder for the parent table name used in the construction of the creteria that filter
		 * table names in forced table children objects filters.
		 * This one comes in form of a regexp matching on oid::regclass::text */
		parent_alias_ref = QString("%1 %2 ~* '(#)'")
											 .arg(match_signature ? parent_sch_ref : "")
											 .arg(AliasPlaceholder + ".relname")
											 .replace("#", "%1");

		// Validating the provided table children objects types
		for(auto &type_name : tab_obj_types)
		{
			if(!TableObject::isTableObject(BaseObject::getObjectType(type_name)))
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::InvChildObjectTypeFilter).arg(type_name),
												ErrorCode::InvChildObjectTypeFilter,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		tab_obj_types.removeDuplicates();
	}

	// The only matching filter is generated if there're filters configured
	if(!filters.isEmpty() && only_matching)
	{
		for(auto &type : getFilterableObjectTypes())
		{
			/* We do not create exclusion filter for table objects if they were specified
			 * by the users forced objects to be filtered */
			if(tab_obj_types.contains(BaseObject::getSchemaName(type)))
				continue;

			if(filters.indexOf(QRegularExpression(QString("(%1)(.)+").arg(BaseObject::getSchemaName(type)))) < 0)
				parsed_filters[type].append(QString("(%1)").arg(InvFilterPattern));
		}
	}

	for(auto &filter : filters)
	{
		values = filter.split(UtilsNs::FilterSeparator);

		// Raises an error if the filter has an invalid field count
		if(values.size() != 3)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvalidObjectFilter).arg(filter).arg(modes.join('|')),
											ErrorCode::InvalidObjectFilter,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		obj_type = BaseObject::getObjectType(values[0]);
		pattern = values[1];
		mode = values[2];

		// Raises an error if the filter has an invalid object type, pattern or mode
		if(obj_type == ObjectType::BaseObject || pattern.isEmpty() || !modes.contains(mode))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvalidObjectFilter).arg(filter).arg(modes.join('|')),
											ErrorCode::InvalidObjectFilter,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		// Converting wildcard patterns into regexp syntax
		if(mode == UtilsNs::FilterWildcard)
		{
			pattern.replace('.', "\\.");

			// If the pattern has wildcard chars we replace them by (.)*
			if(pattern.contains(UtilsNs::WildcardChar))
			{
				QStringList list = pattern.split(UtilsNs::WildcardChar, Qt::KeepEmptyParts);
				QString any_str = "(.)*";
				pattern.clear();

				for(auto &word : list)
				{
					if(!word.isEmpty())
						word = QString("(%1)").arg(word);
				}

				pattern = list.join(any_str);
			}

			/* In wildcard mode the pattern matching must be exact, so we prepend ^ and append $
			 * in order to force a regular expression with exact match. */
			pattern.prepend('^');
			pattern.append('$');
		}

		parsed_filters[obj_type].append(QString("(%1)").arg(pattern));

		// Storing the table/view/foreign table patters if there're forced children objects to filter
		if(!tab_obj_types.isEmpty() && BaseTable::isBaseTable(obj_type))
			tab_patterns[obj_type].append(parent_alias_ref.arg(pattern));
	}

	if(!tab_obj_types.isEmpty())
	{
		std::map<ObjectType, QString> fmt_tab_patterns;
		QStringList fmt_conds;

		for(auto &itr : tab_patterns)
			fmt_tab_patterns[itr.first] = QString("(%1) AND %2.relkind ")
																		.arg(tab_patterns[itr.first].join(" OR "))
																		.arg(AliasPlaceholder);

		for(auto &type_name : tab_obj_types)
		{
			obj_type = BaseObject::getObjectType(type_name);

			/* Configuring the "relkind" criteria according to the
			 * flags indicating the presence of table/view/foreign table filters.
			 * This relkinds will be used to filter table children objects specifically for
			 * "relation kind" in pg_class in order to avoid bringing children object of
			 * table types not filtered */
			if(has_tab_filter && BaseObject::isChildObjectType(ObjectType::Table, obj_type))
				fmt_conds.append(QString("(%1)").arg(fmt_tab_patterns[ObjectType::Table] + "IN ('r','p')"));

			if(has_view_filter && BaseObject::isChildObjectType(ObjectType::View, obj_type))
				fmt_conds.append(QString("(%1)").arg(fmt_tab_patterns[ObjectType::View] + "IN ('v','m')"));

			if(has_ftab_filter && BaseObject::isChildObjectType(ObjectType::ForeignTable, obj_type))
				fmt_conds.append(QString("(%1)").arg(fmt_tab_patterns[ObjectType::ForeignTable] + "= 'f'"));

			if(!fmt_conds.isEmpty())
			{
				extra_filter_conds[obj_type] = fmt_conds.join(" OR ").replace(AliasPlaceholder, parent_aliases.at(obj_type));
				fmt_conds.clear();
			}
		}
	}

	// Joining all configured filters in a single regexp
	for(auto &itr : parsed_filters)
		obj_filters[itr.first] = itr.second.join('|');
}

void Catalog::clearObjectFilter(ObjectType type)
{
	obj_filters.erase(type);
	extra_filter_conds.erase(type);
}

void Catalog::clearObjectFilters()
{
	obj_filters.clear();
	extra_filter_conds.clear();
}

unsigned Catalog::getLastSysObjectOID()
{
	return last_sys_oid;
}

bool Catalog::isSystemObject(unsigned oid)
{
	return (oid <= last_sys_oid);
}

bool Catalog::isExtensionObject(unsigned oid, const QString &ext_name)
{
	if(!ext_name.isEmpty() && ext_objects.count(ext_name) == 0)
		return false;

	if(ext_name.isEmpty())
	{
		for(auto &itr : ext_objects)
		{
			if(itr.second.contains(QString::number(oid)))
				return true;
		}

		return false;
	}

	return ext_objects[ext_name].contains(QString::number(oid));
}

void Catalog::loadCatalogQuery(const QString &qry_id)
{
	if(catalog_queries.count(qry_id)==0)
		catalog_queries[qry_id] = UtilsNs::loadFile(GlobalAttributes::getSchemaFilePath(GlobalAttributes::CatalogSchemasDir, qry_id));

	schparser.loadBuffer(catalog_queries[qry_id]);
}

QString Catalog::getCatalogQuery(const QString &qry_type, ObjectType obj_type, bool single_result, attribs_map attribs)
{
	QString sql, custom_filter;

	/* Escaping apostrophe (') in the attributes values to avoid SQL errors
	 * due to support to this char in the middle of objects' names. The only exception
	 * is for custom filter attribute and comment retrive queries that remain unchaged */
	for(auto &attr : attribs)
	{	
		if(attr.first != Attributes::CustomFilter &&
			 attr.first != Attributes::Comment &&
			 attr.second.contains(QChar('\'')))
		{
			attr.second.replace(QChar('\''), "''");
		}
	}

	schparser.setPgSQLVersion(connection.getPgSQLVersion(true),
														Connection::isDbVersionIgnored());
	attribs[qry_type]=Attributes::True;

	if(exclude_sys_objs || list_only_sys_objs)
		attribs[Attributes::LastSysOid]=QString::number(last_sys_oid);

	if(list_only_sys_objs)
		attribs[Attributes::OidFilterOp]="<=";
	else
		attribs[Attributes::OidFilterOp]=">";

	if(obj_type==ObjectType::Type && exclude_array_types)
		attribs[Attributes::ExcBuiltinArrays]=Attributes::True;

	// If there's a name filter configured for the object type
	if(obj_filters.count(obj_type))
	{
		attribs[Attributes::UseSignature] = match_signature ? Attributes::True : "";
		attribs[Attributes::NameFilter] = obj_filters[obj_type];
	}

	// If there's a name filter configured for the object type
	if(extra_filter_conds.count(obj_type))
		attribs[Attributes::ExtraCondition] = extra_filter_conds[obj_type];

	//Checking if the custom filter expression is present
	if(attribs.count(Attributes::CustomFilter))
	{
		custom_filter=attribs[Attributes::CustomFilter];
		attribs.erase(Attributes::CustomFilter);
	}

	if(exclude_ext_objs && obj_type!=ObjectType::Database &&	obj_type!=ObjectType::Role && obj_type!=ObjectType::Tablespace && obj_type!=ObjectType::Extension)
	{
		if(ext_oid_fields.count(obj_type)==0)
			attribs[Attributes::NotExtObject]=getNotExtObjectQuery(oid_fields.at(obj_type));
		else
			attribs[Attributes::NotExtObject]=getNotExtObjectQuery(ext_oid_fields.at(obj_type));
	}

	loadCatalogQuery(BaseObject::getSchemaName(obj_type));
	schparser.ignoreUnkownAttributes(true);
	schparser.ignoreEmptyAttributes(true);

	attribs[Attributes::PgSqlVersion]=schparser.getPgSQLVersion();
	sql=schparser.getSourceCode(attribs).simplified();

	//Appeding the custom filter to the whole catalog query
	if(!custom_filter.isEmpty())
	{
		int order_by_idx = sql.lastIndexOf("ORDER BY", -1, Qt::CaseInsensitive),
				where_idx = sql.lastIndexOf("WHERE", -1, Qt::CaseInsensitive),
				pos = -1;

		// If there's no where in the catalog query
		if(where_idx < 0)
		{
			// Adding the custom filter with a WHERE statement
			custom_filter.prepend(" WHERE ");

			/* If we have and order by then the where statement will
			 * be placed before the order by */
			if(order_by_idx > 0)
				pos = order_by_idx;
			// Otherwise, it will be placed at query end
			else
				pos = sql.length();
		}
		// If we have an order by and a where
		else if(where_idx > 0)
		{
			custom_filter = QString(" AND (%1) ").arg(custom_filter);

			// If the order by at left of the where (inside a subquery for example)
			if(order_by_idx < 0 || order_by_idx < where_idx)
				// The custom filter will be placed at the end of the query
				pos = sql.length();
			else
				//Otherwise, before the order by
				pos = order_by_idx;
		}

		sql.insert(pos, custom_filter);
	}

	//Append a LIMIT clause when the single_result is set
	if(single_result)
	{
		if(sql.endsWith(';'))
			sql.remove(sql.size()-1, 1);

		sql+=" LIMIT 1";
	}

	return sql;
}

void Catalog::executeCatalogQuery(const QString &qry_type, ObjectType obj_type, ResultSet &result, bool single_result, attribs_map attribs)
{
	try
	{
		connection.executeDMLCommand(getCatalogQuery(qry_type, obj_type, single_result, attribs), result);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
						QString("catalog: %1").arg(BaseObject::getSchemaName(obj_type)));
	}
}

unsigned Catalog::getObjectsCount(std::vector<ObjectType> obj_types, bool incl_sys_objs, const QString &sch_name, const QString &tab_name, attribs_map extra_attribs)
{
	try
	{
		QStringList queries;
		QString sql, extra_info_token = Attributes::ExtraInfo;

		extra_info_token.replace('-', '_');

		if(!incl_sys_objs)
			extra_attribs[Attributes::LastSysOid] = QString::number(last_sys_oid);

		extra_attribs[Attributes::Schema] = sch_name;
		extra_attribs[Attributes::Table] = tab_name;

		for(auto &obj_type : obj_types)
		{
			//Build the catalog query for the specified object type
			sql = getCatalogQuery(QueryList, obj_type, false, extra_attribs);

			/* An catalog query will be empty if the object type doesn't exist in the
			 * current database version being queried */
			if(sql.isEmpty())
				continue;

			/* Removing the section of the query where columns are selected until the column extra_info,
			 * and injecting a count(oid) instruction */
			sql.remove(sql.indexOf("SELECT"), sql.indexOf(extra_info_token) + extra_info_token.length());
			sql.prepend(QString("SELECT count(%1) ").arg(oid_fields.at(obj_type)));
			queries.append(sql);
		}

		sql = QString("SELECT (") +  queries.join(") + (") + QChar(')');

		ResultSet res;
		connection.executeDMLCommand(sql, res);

		if(res.accessTuple(ResultSet::FirstTuple))
			return QString(res.getColumnValue(0)).toUInt();

		return 0;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Catalog::QueryFilter Catalog::getQueryFilter()
{
	return filter;
}

std::map<ObjectType, QString> Catalog::getObjectFilters()
{
	return obj_filters;
}

std::vector<ObjectType> Catalog::getFilteredObjectTypes()
{
	std::vector<ObjectType> types;
	QRegularExpression regexp = QRegularExpression(QString("(.)*(%1)(.)*").arg(InvFilterPattern));

	for(auto &flt : obj_filters)
	{
		if(flt.second.indexOf(QRegularExpression(regexp)) < 0)
			types.push_back(flt.first);
	}

	for(auto &ext_flt : extra_filter_conds)
		types.push_back(ext_flt.first);

	return types;
}

void Catalog::getObjectsOIDs(std::map<ObjectType, std::vector<unsigned> > &obj_oids, std::map<unsigned, std::vector<unsigned> > &col_oids, attribs_map extra_attribs)
{
	try
	{
		std::vector<ObjectType> types=BaseObject::getObjectTypes(true, { ObjectType::Database, ObjectType::Relationship, ObjectType::BaseRelationship,
																																ObjectType::Textbox, ObjectType::Tag, ObjectType::Column, ObjectType::Permission,
																																ObjectType::GenericSql });
		attribs_map attribs, col_attribs, sch_names;
		std::vector<attribs_map> tab_attribs;
		unsigned tab_oid=0;

		for(ObjectType type : types)
		{
			/* We retrieve the object's attributes only if there're no filters configured
			 * for the current type or in case of having filters, the type is registered in one of the
			 * two filter structures */
			if((obj_filters.empty() && extra_filter_conds.empty()) ||
				 (!obj_filters.empty() && obj_filters.count(type) != 0) ||
				 (!extra_filter_conds.empty() && TableObject::isTableObject(type) && extra_filter_conds.count(type) != 0))
				attribs = getObjectsNames(type, "", "", extra_attribs);

			for(auto &attr : attribs)
			{
				obj_oids[type].push_back(attr.first.toUInt());

				//Store the schemas names in order to retrieve the tables' columns correctly
				if(type==ObjectType::Schema)
					sch_names[attr.first]=attr.second;
				else if(type==ObjectType::Table)
				{
					//Get the full set of attributes of the table
					tab_oid=attr.first.toUInt();
					tab_attribs=getObjectsAttributes(type, "", "", { tab_oid });

					//Retrieve the oid and names of the table's columns
					col_attribs=getObjectsNames(ObjectType::Column, sch_names[tab_attribs[0][Attributes::Schema]], attr.second);

					for(auto &col_attr : col_attribs)
						col_oids[tab_oid].push_back(col_attr.first.toUInt());
				}
			}

			attribs.clear();
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

attribs_map Catalog::getObjectsNames(ObjectType obj_type, const QString &sch_name, const QString &tab_name, attribs_map extra_attribs)
{
	try
	{
		ResultSet res;
		attribs_map objects;

		extra_attribs[Attributes::Schema]=sch_name;
		extra_attribs[Attributes::Table]=tab_name;
		executeCatalogQuery(QueryList, obj_type, res, false, extra_attribs);

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				objects[res.getColumnValue(Attributes::Oid)]=res.getColumnValue(Attributes::Name);
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		return objects;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

std::vector<attribs_map> Catalog::getObjectsNames(std::vector<ObjectType> obj_types, const QString &sch_name, const QString &tab_name, attribs_map extra_attribs, bool sort_results)
{
	try
	{
		ResultSet res;
		std::vector<attribs_map> objects;
		QString sql, select_kw="SELECT";
		QStringList queries;
		attribs_map attribs;

		extra_attribs[Attributes::Schema]=sch_name;
		extra_attribs[Attributes::Table]=tab_name;

		for(auto &obj_type : obj_types)
		{	
			//Build the catalog query for the specified object type
			sql=getCatalogQuery(QueryList, obj_type, false, extra_attribs);

			/* For certain objects the catalog query will be empty due to the
			absence of that kind of element in the version of the database.
			E.g.: Event triggers does not exists in PgSQL < 9.3 */
			if(!sql.isEmpty())
			{
				//Injecting the object type integer code in order to sort the final result
				sql.replace(sql.indexOf(select_kw), select_kw.size(),
										QString("%1 %2 AS object_type, ").arg(select_kw).arg(enum_t(obj_type)));

				sql+=QChar('\n');
				queries.push_back(sql);
			}
		}

		//Joining the generated queries by using union in order to retrieve all results at once
		sql = QChar('(') +  queries.join(") UNION (") + QChar(')');

		if(sort_results)
			sql += " ORDER BY oid, object_type";

		connection.executeDMLCommand(sql, res);

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			QString fmt_attr_name;

			do
			{
				for(auto &col_name : res.getColumnNames())
				{
					fmt_attr_name = QString(col_name).replace('_', '-');
					attribs[fmt_attr_name] = res.getColumnValue(col_name);
				}

				// Creating a signature attribute by joining the parent name and the object name.
				if(!attribs[Attributes::Parent].isEmpty())
					attribs[Attributes::Signature] = attribs[Attributes::Parent] + ".";

				attribs[Attributes::Signature] += attribs[Attributes::Name];

				objects.push_back(attribs);
				attribs.clear();
				fmt_attr_name.clear();
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		return objects;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

attribs_map Catalog::getAttributes(const QString &obj_name, ObjectType obj_type, attribs_map extra_attribs)
{
	try
	{
		ResultSet res;
		attribs_map obj_attribs;

		//Add the name of the object as extra attrib in order to retrieve the data only for it
		extra_attribs[Attributes::Name]=obj_name;
		executeCatalogQuery(QueryAttribs, obj_type, res, true, extra_attribs);

		if(res.accessTuple(ResultSet::FirstTuple))
			obj_attribs=changeAttributeNames(res.getTupleValues());

		/* Insert the object type as an attribute of the query result to facilitate the
		import process on the classes that uses the Catalog */
		obj_attribs[Attributes::ObjectType]=QString("%1").arg(enum_t(obj_type));

		return obj_attribs;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

std::vector<attribs_map> Catalog::getMultipleAttributes(ObjectType obj_type, attribs_map extra_attribs)
{
	try
	{
		ResultSet res;
		attribs_map tuple;
		std::vector<attribs_map> obj_attribs;

		executeCatalogQuery(QueryAttribs, obj_type, res, false, extra_attribs);
		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				tuple=changeAttributeNames(res.getTupleValues());

				/* Insert the object type as an attribute of the query result to facilitate the
				import process on the classes that uses the Catalog */
				tuple[Attributes::ObjectType]=QString("%1").arg(enum_t(obj_type));

				obj_attribs.push_back(tuple);
				tuple.clear();
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		return obj_attribs;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

std::vector<attribs_map> Catalog::getMultipleAttributes(const QString &catalog_sch, attribs_map attribs)
{
	try
	{
		ResultSet res;
		attribs_map tuple;
		std::vector<attribs_map> obj_attribs;

		loadCatalogQuery(catalog_sch);
		schparser.ignoreUnkownAttributes(true);
		schparser.ignoreEmptyAttributes(true);

		attribs[Attributes::PgSqlVersion]=schparser.getPgSQLVersion();
		connection.executeDMLCommand(schparser.getSourceCode(attribs).simplified(), res);

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				tuple=changeAttributeNames(res.getTupleValues());
				obj_attribs.push_back(tuple);
				tuple.clear();
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		return obj_attribs;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString Catalog::getCommentQuery(const QString &oid_field, ObjectType obj_type, bool is_shared_obj)
{
	QString query_id=Attributes::Comment;

	try
	{
		attribs_map attribs={{ Attributes::Oid, oid_field },
												 { Attributes::SharedObj, (is_shared_obj ? Attributes::True : "") },
												 { Attributes::ObjRelationName, obj_relnames.at(obj_type) }};

		loadCatalogQuery(query_id);
		return schparser.getSourceCode(attribs).simplified();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
						QString("catalog: %1").arg(query_id));
	}
}

QString Catalog::getNotExtObjectQuery(const QString &oid_field)
{
	QString query_id="notextobject";

	try
	{
		attribs_map attribs={{Attributes::Oid, oid_field},
												 {Attributes::ExtObjOids, ext_objs_oids}};


		loadCatalogQuery(query_id);
		return schparser.getSourceCode(attribs).simplified();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
						QString("catalog: %1").arg(query_id));
	}
}

attribs_map Catalog::changeAttributeNames(const attribs_map &attribs)
{
	attribs_map::const_iterator itr=attribs.begin();
	attribs_map new_attribs;
	QString attr_name, value;

	while(itr!=attribs.end())
	{
		attr_name=itr->first;
		value=itr->second;

		if(attr_name.endsWith(BoolField))
		{
			attr_name.remove(BoolField);
			if(value==PgSqlFalse) value.clear();
			else value=Attributes::True;
		}

		attr_name.replace('_','-');
		new_attribs[attr_name]=value;
		itr++;
	}

	return new_attribs;
}

QString Catalog::createOidFilter(const std::vector<unsigned> &oids)
{
	QString filter;

	for(auto &oid : oids)
		filter += QString("%1,").arg(oid);

	if(!filter.isEmpty())
		filter.remove(filter.size() - 1, 1);

	return filter;
}

std::vector<attribs_map> Catalog::getObjectsAttributes(ObjectType obj_type, const QString &schema, const QString &table, const std::vector<unsigned> &filter_oids, attribs_map extra_attribs)
{
	try
	{
		bool is_shared_obj=(obj_type==ObjectType::Database ||	obj_type==ObjectType::Role ||
												obj_type==ObjectType::Tablespace || obj_type==ObjectType::Language ||
												obj_type==ObjectType::Cast);

		extra_attribs[Attributes::Schema]=schema;
		extra_attribs[Attributes::Table]=table;

		if(!filter_oids.empty())
			extra_attribs[Attributes::FilterOids]=createOidFilter(filter_oids);

		//Retrieve the comment catalog query. Only columns need to retreive comments in their own catalog query file
		if(obj_type != ObjectType::Column)
			extra_attribs[Attributes::Comment] = getCommentQuery(oid_fields.at(obj_type), obj_type, is_shared_obj);

		return getMultipleAttributes(obj_type, extra_attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
						QString("catalog: %1").arg(BaseObject::getSchemaName(obj_type)));
	}
}

attribs_map Catalog::getObjectAttributes(ObjectType obj_type, unsigned oid, const QString sch_name, const QString tab_name, attribs_map extra_attribs)
{
	try
	{
		std::vector<attribs_map> attribs_vect=getObjectsAttributes(obj_type, sch_name, tab_name, { oid }, extra_attribs);
		return (attribs_vect.empty() ? attribs_map() : attribs_vect[0]);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
										qApp->translate("Catalog","Object type: %1","", -1).arg(BaseObject::getSchemaName(obj_type)));
	}
}

QString Catalog::getObjectOID(const QString &name, ObjectType obj_type, const QString &schema, const QString &table)
{
	try
	{
		attribs_map attribs;
		ResultSet res;

		attribs[Attributes::CustomFilter] = QString("%1 = E'%2'").arg(name_fields.at(obj_type)).arg(name);
		attribs[Attributes::Schema] = schema;
		attribs[Attributes::Table] = table;
		executeCatalogQuery(QueryList, obj_type, res, false, attribs);

		if(res.getTupleCount() > 1)
			throw Exception(qApp->translate("Catalog","The catalog query returned more than one OID!","", -1),
											ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(res.getTupleCount() == 0)
			return "0";

		res.accessTuple(ResultSet::FirstTuple);
		return res.getColumnValue(Attributes::Oid);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
										qApp->translate("Catalog","Object type: %1","", -1).arg(BaseObject::getSchemaName(obj_type)));
	}
}

attribs_map Catalog::getServerAttributes()
{
	attribs_map attribs;

	try
	{
		ResultSet res = ResultSet();
		QString sql, attr_name;
		attribs_map tuple, attribs_aux;

		loadCatalogQuery("server");
		schparser.ignoreUnkownAttributes(true);
		schparser.ignoreEmptyAttributes(true);
		sql = schparser.getSourceCode(attribs).simplified();
		connection.executeDMLCommand(sql, res);

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				tuple=res.getTupleValues();
				attr_name = tuple[Attributes::Attribute];
				attr_name.replace('_','-');
				attribs[attr_name]=tuple[Attributes::Value];
			}
			while(res.accessTuple(ResultSet::NextTuple));

			attribs[Attributes::Connection] = connection.getConnectionId();
			attribs_aux = connection.getServerInfo();
			attribs.insert(attribs_aux.begin(), attribs_aux.end()) ;
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
										qApp->translate("Catalog","Object type: server","", -1));
	}

	return attribs;
}

bool Catalog::isServerSupported()
{
	try
	{
		return connection.isServerSupported();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QString Catalog::getServerVersion(bool major_only)
{
	try
	{
		return connection.getPgSQLVersion(major_only);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QStringList Catalog::parseArrayValues(const QString &array_val)
{
	QStringList list;
	QRegularExpression regexp(QRegularExpression::anchoredPattern(ArrayPattern));

	if(regexp.match(array_val).hasMatch())
	{
		//Detecting the position of { and }
		int start=array_val.indexOf('{')+1,
				end=array_val.lastIndexOf('}')-1;
		QString value=array_val.mid(start, (end - start)+1);

		if(value.contains('"'))
			list = parseDefaultValues(value, "\"", ",");
		else
			list = value.split(',', Qt::SkipEmptyParts);
	}

	return list;
}

QStringList Catalog::parseDefaultValues(const QString &def_values, const QString &str_delim, const QString &val_sep)
{
	int idx = 0, delim_start, delim_end, sep_idx, pos = -1;
	QStringList values;
	QString array_expr = "ARRAY[", aux_def_vals = def_values, array_val;

	/* Special case for ARRAY[M, .. , N] values:
	 *
	 * We need to replace temporarily the element separators (generally comma)
	 * by another character in order to avoid splitting the string in a vector
	 * greater than the passed default values define. *
	 *
	 * Example:
	 * Supposing the default value string "0, '*', ARRAY[0, 1, 2 ,3], 'foo', ARRAY['a', 'b', 'c']"
	 * If the elements ARRAY[] are not treated properly, in the final result we'll have
	 * a QStringList with 10 elements but the correct must be 5 since the commas inside ARRAY[]
	 * must not be considered when splitting the provided string. */

	do
	{
		pos = aux_def_vals.indexOf(array_expr, pos + 1);

		if(pos >= 0)
		{
			idx = aux_def_vals.indexOf("], ", pos + 1);

			if(idx < 0)
				idx = aux_def_vals.indexOf(']', pos + 1);

			array_val = aux_def_vals.mid(pos, (idx - pos) + 1);
			array_val.replace(",", UtilsNs::DataSeparator);
			aux_def_vals.replace(pos, array_val.size(), array_val);
		}
	}
	while(pos >= 0);

	pos = idx = 0;
	while(idx < aux_def_vals.size())
	{
		//Get the index of string delimiters (default: ')
		delim_start = aux_def_vals.indexOf(str_delim, idx);
		delim_end = aux_def_vals.indexOf(str_delim, delim_start + 1);

		/* Get the index of value separator on default value string
			 (by default the pg_get_expr separates values by comma and space (, ) */
		sep_idx = aux_def_vals.indexOf(val_sep, idx);

		/* If there is no separator on string (only one value) or the is
			 beyond the string delimiters or even there is no string delimiter on string */
		if(sep_idx < 0 ||
				(sep_idx >=0 && delim_start >= 0 && delim_end >= 0 &&
				(sep_idx < delim_start || sep_idx > delim_end)) ||
				(sep_idx >= 0 && (delim_start < 0 || delim_end < 0)))
		{
			//Extract the value from the current position
			values.push_back(aux_def_vals.mid(pos, sep_idx - pos).trimmed());

			//If there is no separator on string indicates that it contains only one value
			if(sep_idx < 0)
				//Forcing the loop abort
				idx = aux_def_vals.size();
			else
			{
				//Passing to the next value right after the separator
				pos = sep_idx+1;
				idx = pos;
			}
		}
		/* If the separator is between a string delimitation e.g.'abc, def' it will be ignored
		and the current postion will be moved to the first char after string delimiter */
		else if(delim_start >= 0 && delim_end >= 0 &&
						sep_idx >= delim_start && sep_idx <= delim_end)
		{
			idx = delim_end + 1;

			/* If the index reaches the end of string but the cursor (pos) isn't at end
			indicates that the last values wasn't retrieved, this way, the value will be
			pushed to list of values */
			if(idx >= aux_def_vals.size() && pos < aux_def_vals.size())
				values.push_back(aux_def_vals.mid(pos, aux_def_vals.size()));
		}
		else
			idx++;
	}

	// Converting back the separators of elements in ARRAY[] values
	for(auto &val : values)
	{
		if(val.contains(array_expr))
			val.replace(UtilsNs::DataSeparator, ",");
	}

	return values;
}

QStringList Catalog::parseRuleCommands(const QString &cmds)
{
	int start=-1, end=-1;
	QRegularExpression cmd_regexp("(DO)( )*(INSTEAD)*( )+");
	QRegularExpressionMatch match;
	QString fmt_cmd;

	match = cmd_regexp.match(cmds);
	start = match.capturedStart() + match.capturedLength();
	end = cmds.lastIndexOf(";");

	fmt_cmd = cmds.mid(start,(end - start)).simplified();

	if(fmt_cmd.startsWith('(') && fmt_cmd.endsWith(')'))
	{
		fmt_cmd.remove(0, 1);
		fmt_cmd.remove(fmt_cmd.length() - 1, 1);
		fmt_cmd = fmt_cmd.trimmed();
	}

	return fmt_cmd.split(';', Qt::SkipEmptyParts);
}

QStringList Catalog::parseIndexExpressions(const QString &expr)
{
	int open_paren = 0, close_paren = 0, pos = 0;
	QStringList expressions;
	QChar chr;
	QString word;
	bool open_apos = false;

	if(!expr.isEmpty())
	{
		while(pos < expr.length())
		{
			chr = expr[pos++];
			word += chr;

			if(chr == QChar('\''))
				open_apos = !open_apos;

			if(!open_apos && chr == QChar('('))
				open_paren++;
			else if(!open_apos && chr == QChar(')'))
				close_paren++;

			if(chr == QChar(',') || pos == expr.length())
			{
				if(open_paren == close_paren)
				{
					if(word.endsWith(QChar(',')))
						word.remove(word.length() - 1, 1);

					if(word.contains('(') && word.contains(')'))
						expressions.push_back(word.trimmed());
					else
						expressions.push_back(word);

					word.clear();
					open_paren = close_paren = 0;
				}
			}
		}
	}

	return expressions;
}

std::vector<ObjectType> Catalog::getFilterableObjectTypes()
{
	static std::vector<ObjectType> types = BaseObject::getObjectTypes(true, { ObjectType::Relationship,
																																			 ObjectType::BaseRelationship,
																																			 ObjectType::Textbox,
																																			 ObjectType::GenericSql,
																																			 ObjectType::Permission,
																																			 ObjectType::Database,
																																			 ObjectType::Column,
																																			 ObjectType::Tag });

	return types;
}

QStringList Catalog::getFilterableObjectNames()
{
	static QStringList names;

	if(names.isEmpty())
	{
		QStringList aux_names = {
			BaseObject::getSchemaName(ObjectType::View),
			BaseObject::getSchemaName(ObjectType::Table),
			BaseObject::getSchemaName(ObjectType::Schema)
		};

		for(auto &type : getFilterableObjectTypes())
		{
			if(type == ObjectType::Table ||
				 type == ObjectType::View ||
				 type == ObjectType::Schema)
				continue;

			names.append(BaseObject::getSchemaName(type));
		}

		names.sort();

		//Placing table types and schema at the start of the list
		for(auto &name : aux_names)
			names.prepend(name);
	}

	return names;
}

void Catalog::operator = (const Catalog &catalog)
{
	try
	{
		this->ext_objects=catalog.ext_objects;
		this->ext_objs_oids=catalog.ext_objs_oids;
		this->connection.setConnectionParams(catalog.connection.getConnectionParams());
		this->last_sys_oid=catalog.last_sys_oid;
		this->filter=catalog.filter;
		this->exclude_ext_objs=catalog.exclude_ext_objs;
		this->exclude_sys_objs=catalog.exclude_sys_objs;
		this->exclude_array_types=catalog.exclude_array_types;
		this->list_only_sys_objs=catalog.list_only_sys_objs;
		this->obj_filters=catalog.obj_filters;
		this->extra_filter_conds=catalog.extra_filter_conds;
		this->connection.connect();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}
