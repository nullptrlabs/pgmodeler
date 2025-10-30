/*
# PostgreSQL Database Modeler (pgModeler)
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
\class BaseObject
\brief Implements the most important operations to define, maintain and generate code (SQL or XML) of database objects
\note <strong>Creation date:</strong> 12/09/2006
\ingroup libcore
*/

#ifndef BASE_OBJECT_H
#define BASE_OBJECT_H

#include "coreglobal.h"
#include "attributes.h"
#include "globalattributes.h"
#include "schemaparser.h"
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>
#include "enumtype.h"
#include "exception.h"
#include "pgsqlversions.h"

enum class ObjectType: unsigned {
	Column,
	Constraint,
	Function,
	Trigger,
	Index,
	Rule,
	Table,
	View,
	Domain,
	Schema,
	Aggregate,
	Operator,
	Sequence,
	Role,
	Conversion,
	Cast,
	Language,
	Type,
	Tablespace,
	OpFamily,
	OpClass,
	Database,
	Collation,
	Extension,
	EventTrigger,
	Policy,
	ForeignDataWrapper,
	ForeignServer,
	ForeignTable,
	UserMapping,
	Transform,
	Procedure,
	Relationship,
	Textbox,
	Permission,
	Parameter,
	TypeAttribute,
	Tag,
	GenericSql,
	BaseRelationship,
	BaseObject,
	BaseTable
};

/* Registering the ObjectType struct as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(ObjectType)

class __libcore BaseObject {
	private:
		//! \brief Current PostgreSQL version used in SQL code generation
		static QString pgsql_ver;

		static bool escape_comments,

		//! \brief Indicates if the dependences/references of the object must be erased on the destructor
		clear_deps_in_dtor;

		//! \brief Stores the set of special (valid) chars that forces the object's name quoting
		static const QByteArray special_chars;

		//! \brief Stores the database wich the object belongs
		BaseObject *database;

		//! \brief Stores the objects that references the "this" object
		std::vector<BaseObject *> object_refs,

		//! \brief Stores the objects that "this" object depends on to create a valid SQL code
		object_deps;

		/*! \brief Indicates if the dependences/references of the object must be erased on the destructor
		 *  This is useful to avoid calling the method clearAllDepsRefs() when destroying the entire
		 *  database model. See more in BaseObject::~BaseObject() */
		static void setClearDepsInDtor(bool value);

	protected:
		SchemaParser schparser;

		/*! \brief Indicates if the PostgreSQL version checking must be ignored during code generation.
		 * This flag allows generating code (poorly!) for older versions ( < 10). */
		static bool ignore_db_version;

		/*! \brief This static attribute is used to generate the unique identifier for objects.
		 * As object instances are created this value is incremented. In some classes
		 * like Schema, DatabaseModel, Tablespace, Role, Type and Function id generators are
		 * used each with a custom different numbering range.
		 *
		 * Initializes the global id which is shared between instances
		 * of classes derived from the this class. The value of global_id
		 * starts at 5k because the id ranges 0, 1k, 2k, 3k, 4k
		 * are respectively assigned to objects of classes Role, Tablespace
		 * DatabaseModel, Schema, Tag */
		static unsigned global_id;

		/*! \brief Stores the unique identifier for the object. This id is nothing else
		 * than the current value of global_id. This identifier is used
		 * to know the chronological order of the creation of each object in the model
		 * because the generation and reading of the XML code is completely tied to the order
		 * in which the objects were created */
		unsigned object_id,

		/*! \brief This is a transient field that is filled up during the reverse engineering process.
		 *  It holds the OID of the object in a PostgreSQL database. It is completely different from
		 *  object_id attribute. Currently, this attribute is used by DatabaseModel::getObjectByOid. */
		pg_oid;

		//! \brief Objects type count declared on enum ObjectType
		static constexpr unsigned ObjectTypeCount=enum_t(ObjectType::BaseTable) + 1;

		/*! \brief Indicates whether the object is protected or not.
		 A protected object indicates that it can not suffer changes in position
		 (e.g. can not be moved or rotated) can not have your name / text changed,
		 and deleted. This is only a flag, the cited operations are controled in a
		 upper class layer */
		bool is_protected,

		/*! \brief This property indicates that the object is a system protected object and cannot be modified
		by the user. Additionally, the this attribute is true the SQL/XML code for the object is not generated */
		system_obj,

		/*! \brief Indicates if the generated SQL code is disable. When this flag is true
		the object's SQL code is created normally but is commented. This is useful when using
		the role only as a reference since it already exists on the destination server. */
		sql_disabled,

		/*! \brief Indicates if the cached code is invalidated. Some key attributes of this class and other base classes
				automatically change the value of this attribute when the respective set[Attribute]() is called. For all the
				rest the method setCodeInvalidated() should be explicitly called if you need to invalidate the code and
				generate it again */
		code_invalidated;

		//! \brief Stores the cached xml and sql code
		QString cached_code[2],

		//! \brief Stores the xml code in reduced form
		cached_reduced_code,

		/*! \brief Store the cached names of the object (raw name, formated name, signature)
		 *  This will avoid calling the name validation/formatting everytime the object name
		 *  need to be retrieved, improving the overall perfomance */
		cached_names[3];

		//! \brief References the cached names entries
		enum CachedNameId: unsigned {
			RawName, // Original name without formatting (double-quotes)
			FmtName, // Original name with formatting (double-quotes)
			Signature // Original name prefixed by the schema's name (both formatted)
		};

		/*! \brief This list stores the name of each object type associated to a schema file
		 * that generates the object's code definition.
		 *
		 * CAUTION: If both amount and order of the object type enumerations are modified
		 * then the order and amount of the elements of this vector must also be modified */
		static const QString objs_schemas[ObjectTypeCount],

		/*! \brief This map associates the object type to a keyword on
		 SQL language that represents the object */
		objs_sql[ObjectTypeCount],

		/*! \brief Stores the name of the type of objects to be used in error messages formatting
		 and others operations that envolves object type name */
		obj_type_names[ObjectTypeCount];

		//! \brief This map stores the translate human readable names of each search attribute use by the object
		static const attribs_map search_attribs_i18n;

		static const QStringList search_attribs_names;

		/*! \brief Role that is owner of the object. Some objects cannot be associated to a role
		 so if one is assigned to the object an error will be raised */
		BaseObject *owner;

		/*! \brief Schema the objects belongs. Some objects cannot be associated to a schema
		 so if one is assigned to the object an error will be raised */
		BaseObject *schema;

		/*! \brief Tablespace to which the object is part. Some objects cannot be associated to a tablespace
		 so if one is assigned to the object an error will be raised */
		BaseObject *tablespace;

		/*! \brief Collation referenced by the object. Some objects cannot be associated to a collation
		 so if one is assigned to the object an error will be raised */
		BaseObject *collation;

		//! \brief Comments related to object
		QString comment,

		//! \brief Object's name (in PostgreSQL accepted format)
		obj_name,

		//! \brief Object's alias (human readable / friendly) name
		alias,

		//! \brief The set of SQL commands appended on the objectc's definition
		appended_sql,

		//! \brief The set of SQL commands prepended on the objectc's definition
		prepended_sql;

		/*! \brief Stores the attributes and their values ​​shaped in strings to be used
		 by SchemaParser on the object's code definition creation. The attribute
		 name related to model objects are defined in ParsersAttributes namespace. */
		attribs_map attributes,

		/*! \brief Stores the attributes and their vales which can be used by the
		 * searching mechanism to match patters */
		search_attribs;

		/*! \brief Type of object, may have one of the values ​​of the enum ObjectType
		 It was used a numeric type to avoid the use excessive of RTTI. */
		ObjectType obj_type;

		/*! \brief This method calls the getSourceCode(SchemaParser::CodeType, bool) method with the 'reduced_form' defined as 'false',
		 This is the real implementation of the virtual method getSourceCode(SchemaParser::CodeType). */
		QString __getSourceCode(SchemaParser::CodeType def_type);

		/*! \brief Set the database that owns the object
		ATTENTION: calling this method with a nullptr parameter doesn't means that the object will
							 be removed from the database, only the attribute will be set as nullptr and
							 if the user calls getDatabase() in further operations may result in crash */
		void setDatabase(BaseObject *db);

		/*! \brief Swap the the ids of the specified objects. The method will raise errors if the objects are the same,
		or some of them are system object. The boolean param enables the id swap between ordinary object and
		cluster level objects (database, tablespace and roles). */
		static void swapObjectsIds(BaseObject *obj1, BaseObject *obj2, bool enable_cl_obj_swap);

		//! \brief Changes the current object id to the most recent global id value.
		static void updateObjectId(BaseObject *obj);

		//! \brief Clears all the attributes used by the SchemaParser
		void clearAttributes();

		/*! \brief Returns the cached code for the specified code type. This method returns an empty
		 string in case of no code is cached */
		QString getCachedCode(unsigned def_type, bool reduced_form);

		/*! \brief Configures the DIF_SQL attribute depending on the type of the object. This attribute is used to know how
		ALTER, COMMENT and DROP commands must be generated. Refer to schema files for comments, drop and alter. */
		void setBasicAttributes(bool format_name);

		/*! \brief Compares two xml buffers and returns if they differs from each other. The user can specify which attributes
	and tags must be ignored when makin the comparison. NOTE: only the name for attributes and tags must be informed */
		bool isCodeDiffersFrom(const QString &xml_def1, const QString &xml_def2, const QStringList &ignored_attribs, const QStringList &ignored_tags);

		/*! \brief Copies the non-empty attributes on the map at parameter to the own object attributes map. This method is used
		as an auxiliary when generating alter definition for some objects. When one or more attributes are copied an especial
		attribute is inserted (HAS_CHANGES) in order to help the atler generatin process to identify which attributes are
		products of comparison */
		void copyAttributes(attribs_map &attribs);

		static QString getAlterCode(QString sch_name, attribs_map &attribs, bool ignore_ukn_attribs=false, bool ignore_empty_attribs=false);

		QString getAlterCommentDefinition(BaseObject *object, attribs_map attributes);

		/*! \brief This version, called inside updateDependencies(), just run through the provided
		 *  dep_objs list and sets the dependency link between the "this" object and the items
		 *  in the list. Additionally, a list of dependencies that must be undone can be provided.
		 *  This method clears the old dependencies by default if old_deps is empty.
		 *
		 *  The dependency link breaking operation made using old_deps runs first before creating the
		 *  dependency link using the objects in dep_objs.
		 *
		 *  NOTE: this method must be called only in specific points of the code (currently only in
		 *  the operator = due to the need in OperationList class )
		 *  because it can be expensive in terms of processing if lots of objects calls it */
		void updateDependencies(const std::vector<BaseObject *> &dep_objs, const std::vector<BaseObject *> &old_deps = {});

		//! \brief Register an object as a reference to the "this" object
		void setReference(BaseObject *ref_obj);

		//! \brief Unregister an object as a reference to the "this" object
		void unsetReference(BaseObject *ref_obj);

		//! \brief Defines the dep_obj as a dependency of the "this" object.
		void setDependency(BaseObject *dep_obj);

		/*! \brief Unregister the dep_obj as a dependency of the "this" object.
		 *  This method also marks that the "this" object is not a reference to dep_obj anymore */
		void unsetDependency(BaseObject *dep_obj);

		//! \brief Link type used to determine the kind of objects retrived by the functions getLinkedObjects()
		enum ObjLinkType: unsigned {
			ObjDependencies,
			ObjReferences
		};

		/*! \brief Returns the list of objects linked to "this".
		 * The lkn_type determines the modality of the objects to be retrieved (see ObjLinkType enum).
		 * The parameter incl_ind_links will include in the resulting list all the indirect links of the object
		 * The parameter excl_types is used to exclude the objects of the types in it from the resulting list.
		 * The parameter rem_duplicates is used to return a list without duplicate elements */
		std::vector<BaseObject *> getLinkedObjects(ObjLinkType lnk_type, bool incl_ind_links, const std::vector<ObjectType> &excl_types, bool rem_duplicates);

		//! \brief This is the recursive version of the getLinkedObjects method
		void __getLinkedObjects(ObjLinkType lnk_type, const std::vector<BaseObject *> &objs, std::vector<BaseObject *> &ind_links);

	public:
		//! \brief Maximum number of characters that an object name on PostgreSQL can have
		static constexpr int ObjectNameMaxLength=63;

		/*! \brief The default number of objects supposed to be stored in objects list.
		 * This values is just a reference (hint) and is used to preallocate (reserve) space on vectors which handle objects
		 * to avoid excessive allocation/deallocation by resizing the vectors due to insert operation */
		static constexpr unsigned DefMaxObjectCount=20;

		BaseObject();

		virtual ~BaseObject();

		//! \brief Returns the reference to the database that owns the object
		BaseObject *getDatabase();

		/*! \brief Defines a specific attribute in the attribute list used to generate the code definition.
		 This method can be used when a class needs to directly write some attributes of
		 another class but does not have permission. */
		void setAttribute(const QString &attrib, const QString &value);

		/*! \brief Returns whether the object name is in conformity with the PostgreSQL object naming rule.
		 (e.g. 63 bytes long and chars in set [a-zA-z0-9_] */
		static bool isValidName(const QString &name);

		/*! \brief Formats the passed name following the PostgreSQL object naming rule.
		 The 'is_operator' parameter indicates that the passed name is a for an operator
		 this is the only type of object that accepts characters outside of the alphabet
		 (e.g. mathematical signs) on the composition of its name.
		 In this case, the formatting function just ignores some validations if the parameter is checked */
		static QString formatName(const QString &name, bool is_operator=false);

		//! \brief Returns the object's type translated name related to the passed object type
		static QString getTypeName(ObjectType obj_type);

		/*! \brief Returns the object's type translated name related to the passed object type id (in string format).
		The string parameter is the value returned by getSchemaName() */
		static QString getTypeName(const QString &type_str);

		/*! \brief Returns the object's type related to the passed type name.
		 *  If the flag is_sql_name is true then the method tries to identify the type
		 *  from the SQL name of the object otherwise it tries to identify by comparing
		 *  with objects' schema names */
		static ObjectType getObjectType(const QString &type_name, bool is_sql_name = false);

		/*! \brief Returns the schema identifier used to generate the code definition related to the
		 passed object type */
		static QString getSchemaName(ObjectType obj_type);

		//! \brief Returns the keyword related to the object on SQL language
		static QString getSQLName(ObjectType obj_type);

		//! \brief Returns the current value of the global object id counter
		static unsigned getGlobalId();

		static void setEscapeComments(bool value);

		static bool isEscapeComments();

		//! \brief Defines the comment of the object that will be attached to its SQL definition
		virtual void setComment(const QString &comment);

		//! \brief Defines the objects name. If the passed name isn't valid it'll raise an error
		virtual void setName(const QString &name);

		/*! \brief Defines the object's alias (human readable / friendly name). An alias is used when the database model is being
		 * displayed in compact view. This method raises an error when the provided name is larger than 63 bytes
		 * (the same rule for PostgreSQL names length) */
		virtual void setAlias(const QString &alias);

		//! \brief Toggles the object's modify protection
		virtual void setProtected(bool value);

		/*! \brief Defines the schema that the object belongs. An error is raised if the
		 passed schema is not valid or the object does not accepts the use of schemas. */
		virtual void setSchema(BaseObject *schema);

		/*! \brief Defines the owner of the object. An error is raised if the
		 passed owner is not valid or the object does not accepts the use of owners. */
		virtual void setOwner(BaseObject *owner);

		/*! \brief Defines the tablespace which the objects will use. An error is raised if the
		 passed tablespace is not valid or the object does not accepts the use of tablespaces. */
		virtual void setTablespace(BaseObject *tablespace);

		/*! \brief Defines the collation which the objects will use. An error is raised if the
		 passed collation is not valid or the object does not accepts the use of collations. */
		virtual void setCollation(BaseObject *collation);

		//! \brief Disables the SQL code commenting it on generation
		virtual void setSQLDisabled(bool value);

		//! \brief Assign to the object a set of SQL commands to be appended to it's definition
		void setAppendedSQL(const QString &sql);

		//! \brief Assign to the object a set of SQL commands to be prepended to it's definition
		void setPrependedSQL(const QString &sql);

		//! \brief Returns if the generated SQL is commented
		bool isSQLDisabled();

		//! \brief Defines if the object is a system protected object
		virtual void setSystemObject(bool value);

		//! \brief Returns if the object is a system protected object
		bool isSystemObject();

		/*! \brief Returns the object's name. The parameter 'format' is used to get
		 the name properly formated (using quotes when there is uppercase char or extended utf-8),
		 the parameter 'prepend_schema' includes the schema name on the objects name (defult) */
		virtual QString getName(bool format=false, bool prepend_schema=true);

		//! \brief Returns the object's alias (user friendly) name
		virtual QString getAlias();

		//! \brief Returns the name of the object with schema name (when available) prepended by default
		virtual QString getSignature(bool format=true);

		//! \brief Returns the object's comment (in raw form)
		QString getComment();

		/*! \brief Returns the object's comment in such way that the quotes are escaped as well,
		 * if escape_special_chars is true, any line break and tabulation is returned in form \n and \t */
		QString getEscapedComment(bool escape_special_chars);

		//! \brief Returns the object's type
		ObjectType getObjectType();

		//! \brief Returns the object's type name
		QString getTypeName();

		//! \brief Returns the object's schema name used to generate code definition
		QString getSchemaName();

		//! \brief Returns the keyword related to the object type
		QString getSQLName();

		//! \brief Returns the schema that the objects is part
		BaseObject *getSchema();

		//! \brief Returns the owner of the object
		BaseObject *getOwner();

		//! \brief Returns the tablespace that the object is part
		BaseObject *getTablespace();

		//! \brief Returns the collation that the object makes use
		BaseObject *getCollation();

		QString getAppendedSQL();

		QString getPrependedSQL();

		//! \brief Returns the object's generated id
		unsigned getObjectId();

		//! \brief Returns if the object is protected or not
		bool isProtected();

		//! \brief Assigns an object to other copiyng all the attributes correctly
		virtual void operator = (BaseObject &obj);

		/*! \brief Forcing the class to be virtual. This means that derivated classes may
		 override this method in order to be possible its instatiation. */
		virtual QString getSourceCode(SchemaParser::CodeType)=0;

		/*! \brief Returns the object's SQL or XML code definition. The attribute 'reduced_form'
		 indicates that the code generation will be an XML minimum representation
		 of the object. See schema file for: functions, schemas, domains, types. */
		virtual QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form);

		/*! \brief Returns the SQL definition in form of ALTER commands containing the differences between the this and 'object'.
		This form do the camparison considering the difference on the objects' names (ignore_name_diff=false). This method
		is used in cases when the objects' name differences are important and can't be discarded */
		virtual QString getAlterCode(BaseObject *object);

		/*! \brief Returns the SQL definition in form of ALTER commands containing the differences between the this and 'object'.
		The paramenter ignore_name_diff when true will cause the method to not generate a ALTER ... RENAME TO when the name of
		objects differs. */
		virtual QString getAlterCode(BaseObject *object, bool ignore_name_diff);

		//!brief Returns the DROP statement for the object
		virtual QString getDropCode(bool cascade);

		//! \brief Returns if the specified type accepts to have a schema assigned
		static bool acceptsSchema(ObjectType obj_type);

		//! \brief Returns if the specified type accepts to have an owner assigned
		static bool acceptsOwner(ObjectType obj_type);

		//! \brief Returns if the specified type accepts to have a tablespace assigned
		static bool acceptsTablespace(ObjectType obj_type);

		//! \brief Returns if the specified type accepts to have a collation assigned
		static bool acceptsCollation(ObjectType obj_type);

		//! \brief Returns if the specified type accepts to have appended sql commands
		static bool acceptsCustomSQL(ObjectType obj_type);

		/*! \brief Returns if the specified type accepts the use of ALTER commands in order to change their attributes
		This is different from PostgreSQL implementation. In pgModeler, an object accepts ALTER when an attribute
		other than name, schema or owner can be changed. */
		static bool acceptsAlterCommand(ObjectType obj_type);

		//! \brief Returns if the specified type accepts the use of DROP commands
		static bool acceptsDropCommand(ObjectType obj_type);

		//! \brief Returns if the object accepts the use of CREATE OR REPLACE commands
		static bool acceptsReplaceCommand(ObjectType obj_type);

		//! \brief Returns if the specified type accepts an alias (friendly name)
		static bool acceptsAlias(ObjectType obj_type);

		//! \brief Returns if the specified type accepts comment
		static bool acceptsComment(ObjectType obj_type);

		//! \brief Returns if the object accepts to have a schema assigned
		bool acceptsSchema();

		//! \brief Returns if the object accepts to have an owner assigned
		bool acceptsOwner();

		//! \brief Returns if the object accepts to have a tablespace assigned
		bool acceptsTablespace();

		//! \brief Returns if the object accepts to have a collation assigned
		bool acceptsCollation();

		//! \brief Returns if the object accepts to have appended sql commands
		bool acceptsCustomSQL();

		//! \brief Returns if the object accepts the use of ALTER commands to have its attributes changed
		bool acceptsAlterCommand();

		//! \brief Returns if the object accepts the use of DROP commands
		bool acceptsDropCommand();

		//! \brief Returns if the object accepts the use of CREATE OR REPLACE commands
		virtual bool acceptsReplaceCommand();

		//! \brief Returns if the object accepts the use of alias
		bool acceptsAlias();

		//! \brief Returns if the object accepts comment
		bool acceptsComment();

		//! \brief Defines the OID of the object in the PostgreSQL database
		void setPgOid(unsigned oid);

		//! \brief Returns the OID of the object in the PostgreSQL database
		unsigned getPgOid();

		/*! \brief Marks the current cached code as invalid and forces its regenaration.
				Some key attributes / setters in the base classes BaseObject, BaseTable and BaseRelationship
				will automatically invalidate the code but for all other setters / attributes the user must call
				this method explicitly in order to force the regeneration of the code.
				This method has no effect when the cached code support is disables. See enableCachedCode() */
		virtual void setCodeInvalidated(bool value);

		virtual void configureSearchAttributes();

		//! \brief Returns if the code (sql and xml) is invalidated
		bool isCodeInvalidated();

		/*! \brief Compares the xml code between the "this" object and another one. The user can specify which attributes
		and tags must be ignored when makin the comparison. NOTE: only the name for attributes and tags must be informed */
		virtual bool isCodeDiffersFrom(BaseObject *object, const QStringList &ignored_attribs={}, const QStringList &ignored_tags={});

		/*! \brief Returns the valid object types in a vector. The types
		ObjectType::ObjBaseObject, TYPE_ATTRIBUTE and ObjectType::ObjBaseTable aren't included in return vector.
		By default table objects (columns, trigger, constraints, etc) are included. To
		avoid the insertion of these types set the boolean param to false. */
		static std::vector<ObjectType> getObjectTypes(bool inc_table_objs=true, std::vector<ObjectType> exclude_types={});

		/*! \brief Returns the valid object types that are child or grouped under the specified type.
	This method works a litte different from getObjectTypes() since this latter returns all valid types
	and this one returns only the valid types for the current specified type. For now the only accepted
	types are ObjectType::Database, ObjectType::Schema, ObjectType::Table, ObjectType::ForeignTable */
		static std::vector<ObjectType> getChildObjectTypes(ObjectType obj_type);

		//! \brief Returns true when the child_type is in the list of children types of the parent_type
		static bool isChildObjectType(ObjectType parent_type, ObjectType child_type);

		//! \brief Sets the default version when generating the SQL code. This affects all instances of classes that is based upon this one
		static void setPgSQLVersion(const QString &version);

		//! \brief Returns the current version for SQL code generation
		static QString getPgSQLVersion();

		//! \brief Returns the set of attributes used by the search mechanism
		attribs_map getSearchAttributes();

		/*! \brief Returns all the objects that the this object depends on.
		 * The boolean paramenter inc_indirect_deps is used to include the indirect dependencies
		 * in the returned list. Indirect dependencies are the dependencies of the objects that are
		 * dependencies of the this object, e.g., view V that depends on a table T that dependes on a schema S.
		 * The parameter excl_types is used to exclude the objects of the types in it from the resulting list.
		 * The parameter rem_duplicates is used to return a list without duplicate elements */
		virtual std::vector<BaseObject *> getDependencies(bool inc_indirect_deps = false, const std::vector<ObjectType> &excl_types = {}, bool rem_duplicates = false);

		/*! \brief Returns all the objects that in which references the this object.
		 * The boolean paramenter inc_indirect_refs is used to include the indirect references
		 * in the returned list. Indirect references are the references of the objects that are
		 * references to the this object, e.g., column C that references a type T that references a schema S.
		 * The parameter excl_types is used to exclude the objects of the types in it from the resulting list.
		 * The parameter rem_duplicates is used to return a list without duplicate elements */
		virtual std::vector<BaseObject *> getReferences(bool inc_indirect_refs = false, const std::vector<ObjectType> &excl_types = {}, bool rem_duplicates = false);

		/*! \brief Ignores the PostgreSQL version checking during code generation.
		 *  When false (the default behavior), when generating code which db version is < 10, an error
		 *  is raised. When true, the error is not raised, but the overall usage of the tool may be affected
		 *  since pgModeler is strictly tied to newer version. */
		static void setIgnoreDbVersion(bool ignore);

		//! \brief Indicates if the database version is ignored in the code generation
		static bool isDbVersionIgnored();

		//! \brief Returns the translated human readable search attribute name
		static QString getSearchAttributeI18N(const QString &search_attr);

		static QStringList getSearchAttributesNames();

		//! \brief Unset all dependecies at once
		void clearDependencies();

		//! \brief Unset all dependecies at once
		void clearReferences();

		//! \brief Clears both dependencies and references
		void clearAllDepsRefs();

		/*! \brief Returns true when the object has at least one object referencing it
		 *  This method is useful to know if the object is being referenced wihtout the
		 *  need to get the references themselves like getReference() */
		bool isReferenced();

		//! \brief Returns true when the provided object is a referrer of the "this" object
		bool isReferencedBy(BaseObject *ref_obj);

		/*! \brief Returns true when the object has at least one dependency to another object
		 *  This method is useful to know if the object has dependencies wihtout the
		 *  need to get the dependencies themselves like getDependencies() */
		bool hasDependencies();

		//! \brief Returns true when the provided object is one of the dependencies of the "this" object
		bool isDependingOn(BaseObject *dep_obj);

		/*! \brief Updates the dependencies list based upon the current relationship between
		 *  the "this" object and its dependencies. This method clears the old dependencies
		 *  relationships and creates other ones.
		 *
		 *  NOTE: this version of the method only produces a dependency relationship between the
		 *  schema, tablespace, owner and collation. Derived classes must reimplement thi one and include
		 *  other dependency objects, e.g., data types, functions, and etc.
		 *
		 *  NOTE: this method must be called only in specific points of the code because it can be expensive
		 *  in terms of processing if lots of objects calls it. */
		virtual void updateDependencies();

		friend class DatabaseModel;
		friend class ModelValidationHelper;
		friend class DatabaseImportHelper;
		friend class SwapObjectsIdsWidget;
		friend class ModelWidget;
};

#endif
