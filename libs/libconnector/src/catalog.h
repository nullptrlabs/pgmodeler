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
\ingroup libconnector
\class Catalog
\brief \brief Implements the operations to read data from information_schema as well from pg_catalog schema.
This class is the basis for the reverse engineering feature.
*/

#ifndef CATALOG_H
#define CATALOG_H

#include "connection.h"
#include "baseobject.h"
#include <QTextStream>
#include <QApplication>

class __libconnector Catalog {
	public:
		enum QueryFilter: unsigned {
			//! \brief This entry is only for bitwise operation purposes
			NoFilter = 0,

			//! \brief Excludes the system objects from listing
			ExclSystemObjs=1,

			//! \brief Excludes the extension generated objects from listing
			ExclExtensionObjs=2,

			//! \brief Excludes the builtin array types.
			ExclBuiltinArrayTypes=4,

			/*! \brief Shows only system objects. Using this filter will disable the other two filters.
			Using this filter implies the listing of extension objects */
			ListOnlySystemObjs=8,

			//! \brief Shows all objects including system objects and extension object.
			ListAllObjects=16
		};

	private:
		SchemaParser schparser;

		//! \brief Executes a list command on catalog
		static const QString QueryList,

		//! \brief Executes a attribute retrieving command on catalog
		QueryAttribs,

		//! \brief Replacement for true 't' boolean value
		PgSqlTrue,

		//! \brief Replacement for false 'f' boolean value
		PgSqlFalse,

		//! \brief Suffix for boolean fields.
		BoolField,

		/*! \brief Query used to retrieve extension objects.
		 * This query retrieve all extension child object except for data types because
		 * they are handled in extension catalog query */
		GetExtensionObjsSql,

		//! \brief This pattern matches the PostgreSQL array values in format [n:n]={a,b,c,d,...} or {a,b,c,d,...}
		ArrayPattern,

		//! \brief Holds a constant string used to mark invalid filter patterns
		InvFilterPattern,

		AliasPlaceholder;

		/*! \brief Stores the oid of objects that are created by extension.
		 * The keys of this map are the names of the extensions that hold objects in the database,
		 * The values of this map are the list of objects oids. This is used to speed up the checking
		 * if an certain object is owned by a certain extension (see isExtensionObject()) */
		std::map<QString, QStringList> ext_objects;

		/*! \brief Stores in comma seperated way the oids of all objects created by extensions. This
		 * 	attribute is use to create the catalog query that filters objects that are created or not
		 *  by extensions. */
		QString ext_objs_oids;

		//! \brief Stores the name filters for each type of object. (See setObjectFilters())
		std::map<ObjectType, QString> obj_filters;

		/*! \brief Stores the extra filters for table children objects.
		 * This one is used only when forced objects filtering is enabled (See setObjectFilters()) */
		std::map<ObjectType, QString> extra_filter_conds;

		/*! \brief This map stores the oid field name for each object type. The oid field name can be
		composed by the pg_[OBJECT_TYPE] table alias. Refer to catalog query schema files for details */
		static const std::map<ObjectType, QString> oid_fields,

		//! \brief This map stores the relation names in catalogs for each object type
		obj_relnames,
		
		/*! \brief This map stores the name field for each object type. Refer to catalog query schema files for details */
		name_fields,

		/*! \brief This map stores the oid field name that is used to check if the object (or its parent) is part of a extension
		(see getNotExtObjectQuery()). By default the attribute oid_fields is used instead for that purpose, but, for some objects,
		there are different fields that tells if the object (or its parent) is part of extension. */
		ext_oid_fields,

		/*! \brief This map stores the aliases that are used to reference the table (parent) on each table object catalog query.
		 * This is mainly used to force the filter of constraints/indexes/triggers/rules/policies in presence of one or more table
		 * filter (see setObjectFilter) */
		parent_aliases;

		//! \brief Store the cached catalog queries
		static attribs_map catalog_queries;

		//! \brief Connection used to query the pg_catalog
		Connection connection;

		//! \brief Stores the last system object identifier. This is used to filter system objects
		unsigned last_sys_oid;

		QueryFilter filter;

		//! \brief Indicates if the catalog must filter system objects
		bool exclude_sys_objs,

		//! \brief Indicates if the catalog must filter extension created objects
		exclude_ext_objs,

		//! \brief Indicates if the catalog must filter built-in array data types
		exclude_array_types,

		//! \brief Indicates if the catalog must list only system objects
		list_only_sys_objs,

		//! \brief Indicates that the name filtering should occur in the objects' signature instead of their names
		match_signature;

		/*! \brief Load the schema parser buffer with the catalog query using identified by qry_id.
		The method will cache the catalog query if it's not cached yet (only when use_cached_queries=true) */
		void loadCatalogQuery(const QString &qry_id);

		/*! \brief Executes a query on the catalog for the specified object type. If the parameter 'single_result' is true
		the query will return only one tuple on the result set. Additional attributes can be passed so that SchemaParser will
		use them when parsing the schema file for the object. A special extra attribute is accepted but not passed to SchemaParser:
		ParsersAttributes::CUSTOM_FILTER that will be appended to the current filter expression */
		void executeCatalogQuery(const QString &qry_type, ObjectType obj_type, ResultSet &result, bool single_result=false, attribs_map attribs=attribs_map());

		//! \brief Returns the catalog query according to the type of the object type provided
		QString getCatalogQuery(const QString &qry_type, ObjectType obj_type, bool single_result=false, attribs_map attribs=attribs_map());

		/*! \brief Recreates the attribute map in such way that attribute names that have
		underscores have this char replaced by dashes. Another special operation made is to replace
		the values of fiels which suffix is _bool to '1' when 't' and to empty when 'f', this is because
		the resultant attribs_map will be passed to XMLParser/SchemaParser which understands bool values as 1 (one) or '' (empty) */
		attribs_map changeAttributeNames(const attribs_map &attribs);

		//! \brief Returns a attribute set for the specified object type and name
		attribs_map getAttributes(const QString &obj_name, ObjectType obj_type, attribs_map extra_attribs=attribs_map());

		/*! \brief Returns the query to retrieve the information if the object (specified by its oid field) is part or not of
		 a extension. */
		QString getNotExtObjectQuery(const QString &oid_field);

		/*! \brief Returns the query that is used to retrieve an objects comment. The 'is_shared_obj' is used
		 * to query the pg_shdescription instead of pg_description */
		QString getCommentQuery(const QString &oid_field, ObjectType obj_type, bool is_shared_obj=false);

		//! \brief Creates a comma separated string containing all the oids to be filtered
		QString createOidFilter(const std::vector<unsigned> &oids);

	public:
		//! \brief Stores the prefix of any temp object (in pg_temp) created during catalog reading by pgModeler
		static const QString PgModelerTempDbObj,

		//! \brief Stores the null char escaped in format \000
		EscapedNullChar;

		Catalog();

		Catalog(const Catalog &catalog);

		//! \brief Changes the current connection used by the catalog
		void setConnection(Connection &conn);

		/*! \brief Closes the connection used by the catalog.
		 * Once this method is called the user must call setConnection() again or the
		 * catalog queries will fail */
		void closeConnection();

		bool isConnectionValid();

		//! \brief Configures the catalog query filter
		void setQueryFilter(QueryFilter filter);

		/*! \brief Configures the objects name filtering.
		 * The parameter only_matching creates extra filters for the other kind of objects not provided by the user in order to avoid listing them.
		 * The tab_obj_types contains a list of table children object type names in which should be forcibly listed
		 * if the user provides table/view/foreign table filters. This is useful to retrieve tables with their children objects avoiding the need of
		 * provide specific filters for each table children object. This list has effect only when discard_non_matches is set to true.
		 * This method raises an exception when detecting malformed filters */
		void setObjectFilters(QStringList filters, bool only_matching, bool match_signature, QStringList tab_obj_types = {});

		//! \brief Clears the filter configured for the provided object type
		void clearObjectFilter(ObjectType type);

		//! \brief Clears all configured filters
		void clearObjectFilters();

		//! \brief Returns the last system object oid registered on the database
		unsigned getLastSysObjectOID();

		//! \brief Returns if the specified oid is amongst the system objects' oids
		bool isSystemObject(unsigned oid);

		/*! \brief Returns if the specified oid is amongst the extension created objects' oids
		 * It is possible to check if the oid is owned by a specific extension by passing the extension name
		 * in the second parameter. */
		bool isExtensionObject(unsigned oid, const QString &ext_name = "");

		/*! \brief Returns the count for the specified object types. A schema name can be specified
		 * in order to filter only objects of the specifed schema. The parameter incl_sys_objs
		 * indicates if the system object need to be included in the resulting counting. */
		unsigned getObjectsCount(std::vector<ObjectType> types, bool incl_sys_objs, const QString &sch_name = "", const QString &tab_name = "", attribs_map extra_attribs = attribs_map());

		//! \brief Returns the current filter configuration for the catalog queries
		QueryFilter getQueryFilter();

		//! \brief Returns the configured objects a name filters
		std::map<ObjectType, QString> getObjectFilters();

		/*! \brief Returns a vector with all filtered object types.
		 * Invalid pattern filters containing the InvFilterPattern are discarded from the returning vector */
		std::vector<ObjectType> getFilteredObjectTypes();

		//! \brief Fills the specified maps with all object's oids querying the catalog with the specified filter
		void getObjectsOIDs(std::map<ObjectType, std::vector<unsigned> > &obj_oids, std::map<unsigned, std::vector<unsigned> > &col_oids, attribs_map extra_attribs=attribs_map());

		/*! \brief Returns a attributes map containing the oids (key) and names (values) of the objects from
		the specified type.	A schema name can be specified in order to filter only objects of the specifed schema */
		attribs_map getObjectsNames(ObjectType obj_type, const QString &sch_name="", const QString &tab_name="", attribs_map extra_attribs=attribs_map());

		/*! \brief Returns a vector of attributes map containing the oids (key) and names as well types of the objects from
		the specified list of types.	A schema name can be specified in order to filter only objects of the specifed schema */
		std::vector<attribs_map> getObjectsNames(std::vector<ObjectType> obj_types, const QString &sch_name="", const QString &tab_name="", attribs_map extra_attribs=attribs_map(), bool sort_results=false);

		//! \brief Returns a set of multiple attributes (several tuples) for the specified object type
		std::vector<attribs_map> getMultipleAttributes(ObjectType obj_type, attribs_map extra_attribs=attribs_map());

		/*! \brief Returns a set of multiple attributes (several tuples) for the specified catalog schema file.
		 * This version of the method differs from the one in which the user need to provide the object type.
		 * This one, the user is responsible to provide all attributes that will be parsed together with the
		 * catalog file. */
		std::vector<attribs_map> getMultipleAttributes(const QString &catalog_sch, attribs_map attribs=attribs_map());

		/*! \brief Retrieve all available objects attributes for the specified type. Internally this method calls the get method for the
		specified type. User can filter items by oids (except for table child objects), by schema (in the object type is suitable to accept schema)
		and by table name (only when retriving child objects for a specific table) */
		std::vector<attribs_map> getObjectsAttributes(ObjectType obj_type, const QString &schema="", const QString &table="", const std::vector<unsigned> &filter_oids={}, attribs_map extra_attribs=attribs_map());

		//! \brief Returns the attributes for the object specified by its type and OID
		attribs_map getObjectAttributes(ObjectType obj_type, unsigned oid, const QString sch_name="", const QString tab_name="", attribs_map extra_attribs=attribs_map());

		/*! \brief Returns the OID of the named object. User can filter items by schema (if the object type is suitable to accept schema)
		and by table name (only when retriving child objects for a specific table). The method will raise an exception if the catalog query
		used returns more than one result. A zero OID is returned when no suitable object is found. */
		QString getObjectOID(const QString &name, ObjectType obj_type, const QString &schema = "", const QString &table = "");

		//! \brief This special method returns some server's attributes read from pg_settings
		attribs_map getServerAttributes();

		/*! \brief This method returns true when the server version is supported.
		 * This is a convenience method that call Connection::isServerSupported()
		 * of the currently open connection of the catalog object. */
		bool isServerSupported();

		//! \brief Returns the version of the server in which the catalog object is connected to
		QString getServerVersion(bool major_only = false);

		//! \brief Parse a PostgreSQL array value and return the elements in a string list
		static QStringList parseArrayValues(const QString &array_val);

		/*! \brief Parse a function's default value and return the elements in a string list.
		It can be specified the string delimiter as well the value separator if the input default value
		contains several values */
		static QStringList parseDefaultValues(const QString &def_values, const QString &str_delim="'", const QString &val_sep=", ");

		//! \brief Parse the raw commands of a rule retrieved by the catalog and returns only the relevant parts
		static QStringList parseRuleCommands(const QString &cmd);

		/*! \brief Parse a set of expressions related to an index returned by the pg_get_expr(oid) and separates
		 * them as a string list. */
		static QStringList parseIndexExpressions(const QString &expr);

		/*! \brief Enable/disable the use of cached catalog queries. When enabled, the schema files read for the first are stored in memory
		so in the next time the same catalog query must be used it'll be read right from the memory and not from the disk anymore */
		static void enableCachedQueries(bool value);

		//! \brief Returns the current status of cached catalog queries
		static bool isCachedQueriesEnabled();

		//! \brief Returns the object types that are able to be filtered
		static std::vector<ObjectType> getFilterableObjectTypes();

		//! \brief Returns the object schema names that are able to be filtered
		static QStringList getFilterableObjectNames();

		//! \brief Performs the copy between two catalogs
		void operator = (const Catalog &catalog);
};

#endif
