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
\ingroup libcore
\class DatabaseModel
\brief This class reunites and manages all the other object types.
Additionally, this class, saves, loads and generates the XML/SQL definition of a complete database.
\note <strong>Creation date:</strong> 20/10/2006
*/

#ifndef DATABASE_MODEL_H
#define DATABASE_MODEL_H

#include <QFile>
#include <QObject>
#include <QStringList>
#include <QDateTime>
#include "baseobject.h"
#include "table.h"
#include "function.h"
#include "schema.h"
#include "view.h"
#include "sequence.h"
#include "relationship.h"
#include "textbox.h"
#include "role.h"
#include "type.h"
#include "tablespace.h"
#include "language.h"
#include "aggregate.h"
#include "cast.h"
#include "conversion.h"
#include "operatorclass.h"
#include "xmlparser.h"
#include "permission.h"
#include "domain.h"
#include "collation.h"
#include "extension.h"
#include "tag.h"
#include "eventtrigger.h"
#include "genericsql.h"
#include "foreigndatawrapper.h"
#include "foreignserver.h"
#include "usermapping.h"
#include "foreigntable.h"
#include "transform.h"
#include "procedure.h"
#include <algorithm>
#include <locale.h>
#include "operation.h"

class ModelWidget;

class __libcore DatabaseModel:  public QObject, public BaseObject {
	Q_OBJECT

	private:
		//! \brief Constants used to access the tuple columns in the internal changelog
		enum LogFields: unsigned {
			LogDate,
			LogSinature,
			LogObjectType,
			LogAction
		};

		/*! \brief Stores all changes performed in the database model
		 * The only purpose of this structure is to be used by the partial diff to filter certain objects by operation/date and,
		 * differently from OperationList class, it's data persisted in the database model file. */
		std::vector<std::tuple<QDateTime,QString,ObjectType,QString>> changelog;

		/*! \brief Stores the references to all object lists of each type. This map is used by getObjectList() in order
		 * to return the list according to the provided type */
		std::map<ObjectType, std::vector<BaseObject *> *> obj_lists;

		/*! \brief Stores the references to the methods that create objects from XML code. This map is used by createObject() in order
		 * to return the created object */
		std::map<ObjectType, std::function<BaseObject*(void)>> create_methods;

		/*! \brief Stores the references to the methods that add objects from database. This map is used by addObject() in order
		 * to return the created object */
		std::map<ObjectType, std::function<void(BaseObject *, int)>> add_methods;

		/*! \brief Stores the references to the methods that remove objects from database. This map is used by removeObject() in order
		 * to return the created object */
		std::map<ObjectType, std::function<void(BaseObject *, int)>> remove_methods;

		static unsigned dbmodel_id;

		XmlParser xmlparser;

		//! \brief Stores the layers names and active layer to write them on XML code
		QStringList layers,

		//! \brief Stores the name colors of each layer to write them on XML code
		layer_name_colors,

		//! \brief Stores the rect colors of each layer to write them on XML code
		layer_rect_colors;

		QList<unsigned> active_layers;

		/*! \brief The current scene (canvas) geometry that is restored when the database
		 * model is rendered by a scene for the first time after loading the model. */
		QRectF scene_rect;

		//! \brief Stores the layer information of FK relationships during the model loading
		std::map<QString, QStringList> fk_rel_layers;

		//! \brief Stores the model widget that is managing this database model instance
		ModelWidget *model_wgt;

		//! \brief Database encoding
		EncodingType encoding;

		//! \brief Template database
		QString template_db,

		//! \brief Model's author
		author,

		//! \brief Database localizations (LC_CTYPE, LC_COLLATE)
		localizations[2];

		//! \brief Stores the objects of each type that are considered the default ones associated to new objects
		std::map<ObjectType, BaseObject *> default_objs;

		//! \brief Maximum number of connections
		int conn_limit;

		//! \brief Indicates if the database can be used as template
		bool is_template,

		//! \brief Indicates if the database accepts connection
		allow_conns,

		//! \brief Indicates if the internal changelog must be saved to the dbm file
		persist_changelog,

		is_layer_names_visible,

		is_layer_rects_visible,

		//! \brief Indicates that disabled objects' SQL code must appear in the database creation script
		gen_dis_objs_code,

		/*! \brief Indicates that system schemas (pg_catalog, information_schema, etc) must display their rectangles
		 *  Since these objects can't have their attributes changes via editing form (except for public schema)
		 *  this flag helps to persist the visibility state of the rectangles of that schemas */
		show_sys_sch_rects,

		/*! \brief This flag is used to notify the model to break the code generation/saving.
		 *  This is only used by the export helper to cancel a running export to file process */
		cancel_saving;

		//! \brief Vectors that stores all the objects types
		std::vector<BaseObject *> textboxes,
		relationships,
		base_relationships,
		functions,
		schemas,
		views,
		tables,
		types,
		roles,
		tablespaces,
		languages,
		aggregates,
		casts,
		conversions,
		operators,
		op_classes,
		op_families,
		domains,
		sequences,
		permissions,
		collations,
		extensions,
		tags,
		eventtriggers,
		genericsqls,
		fdata_wrappers,
		foreign_servers,
		usermappings,
		foreign_tables,
		transforms,
		procedures;

		/*! \brief Stores the xml definition for special objects. This map is used
		 when revalidating the relationships */
		std::map<unsigned, QString> xml_special_objs;

		/*! \brief Stores the special objects considered invalid after a relationships revalidation.
		 * This vector is destroyed only when the model is destroyed too in order to avoid segfaults */
		std::vector<BaseObject *> invalid_special_objs;

		//! \brief Indicates if the model is being loaded
		bool loading_model,

		/*! \brief Indicates if the model was invalidated due to operations like insert / remove objects.
		When this flag is set it's recommend to revalidate the model using the Model validation tool */
		invalidated,

		//! \brief Indicates that appended SQL commands must be put at the very end of model definition
		append_at_eod,

		//! \brief Indicates that prepended SQL commands must be put at the very beginning of model definition
		prepend_at_bod;

		//! \brief Stores the last position on the model where the user was editing objects
		QPoint last_pos;

		double last_zoom;

		//! \brief Returns an object seaching it by its name and type. The third parameter stores the object index
		BaseObject *getObject(const QString &name, ObjectType obj_type, int &obj_idx);

		//! \brief Generic method that adds an object to the model
		void __addObject(BaseObject *object, int obj_idx=-1);

		/*! \brief Generic method that removes an object to the model. The boolean param is used
		to enable/disable reference checking before remove the object from model. */
		void __removeObject(BaseObject *object, int obj_idx=-1, bool check_refs=true);

		//! \brief Recreates the special object from the passed xml code buffer
		void createSpecialObject(const QString &xml_def, unsigned obj_id=0);

		//! \brief Removes an user defined type (domain or type)
		void removeUserType(BaseObject *object, int obj_idx);

		//! \brief Creates a IndexElement or ExcludeElement from XML depending on type of the 'elem' param.
		void createElement(Element &elem, TableObject *tab_obj, BaseObject *parent_obj);

		//! \brief Returns extra error info when loading database models
		QString getErrorExtraInfo();

		/*! \brief This method forces the breaking of the code generation/saving in the methods getSourceCode, saveModel and saveSplitModel.
		 *  This method is used only by the export helper in such a way to allow the user to abort any export to file in a threaded operation. */
		void setCancelSaving(bool value);

		//! \brief Set the initial capacity of the objects list for a optimized memory usage
		void setObjectListsCapacity(unsigned capacity);

		/*! \brief Configures all the shell types related to base user-defined base. By default, this method will convert
		 * parameters of functions that are part of a user defined type and return the shell types SQL code. If the parameter reset_config
		 * the method will only restore the original configuration of the functions and return an empty string. */
		QString configureShellTypes(bool reset_config);

		/*! \brief Saves the appended/prepended code of the database model to a separated file.
		 * The parameter save_appended tells the method to save appended code instead of prepended code.
		 * The parameter path is where the file will be saved. The file_prefix is a string that is prepended
		 * to the filename. Returns true when the file could be saved. */
		bool saveSplitCustomSQL(bool save_appended, const QString &path, const QString &file_prefix);

		//! \brief Returns true if there is at least one relationship in an invalid state
		bool hasInvalidRelatioships();

		/*! \brief Tries to create all the special objects returning a vector of errors
		 *  if one or more special object can't be created */
		std::vector<Exception> createSpecialObjects();

		//! \brief Updates all the relationships in such a way to create the missing columns/constraints
		void updateRelsGeneratedObjects();

		//! \brief Restore the layer information of FK relationship during loading process
		void restoreFKRelationshipLayers();

	protected:
		//! \brief Set the layer names (only to be written in the XML definition)
		void setLayers(const QStringList &layers);

		//! \brief Set the active layer ids (only to be written in the XML definition)
		void setActiveLayers(const QList<unsigned> &layers);

		//! \brief Set the layer name colors (only to be written in the XML definition)
		void setLayerNameColors(const QStringList &color_names);

		//! \brief Set the layer rect colors (only to be written in the XML definition)
		void setLayerRectColors(const QStringList &color_names);

		//! \brief Set the layer names visibility (only to be written in the XML definition)
		void setLayerNamesVisible(bool value);

		//! \brief Set the layer rects visibility (only to be written in the XML definition)
		void setLayerRectsVisible(bool value);

		/*! \brief Register an object change in the internal changelog.
		 * If the provided object is derived from TableObject then the parent is registered instead.
		 * The op_type is one of the operations Operation::ObjectCreate, Operation::ObjRemoved, Operation::ObjModified, any other operation type is ignored.
		 * This method will validate all the provided parameters and in case of invalid values will raise and exception */
		void addChangelogEntry(BaseObject *object, Operation::OperType op_type, BaseObject *parent_obj = nullptr);

		/*! \brief Register an object change in the internal changelog.
		 * This version accepts string parameters to make the changelog loading from file more easy to handle.
		 * This method will validate all the provided parameters and in case of invalid values will raise and exception */
		void addChangelogEntry(const QString &signature, const QString &type, const QString &action, const QString &date);

		//! \brief Loads the basic attributes, common between all children of BaseObject, from XML code
		void setBasicAttributes(BaseObject *object);

		//! \brief Loads the basic attributes, common between all children of BaseFunction, from XML code
		void setBasicFunctionAttributes(BaseFunction *func);

		//! \brief Updates (creating/removing) in the model the extension children objects
		bool updateExtensionObjects(Extension *ext);

		//! \brief Removes from the model all the extension children objects
		void removeExtensionObjects(Extension *ext);

		//! \brief This convenience method forces the redrawn of the tables of a relationship as well as their respective schemas
		void setRelTablesModified(BaseRelationship *rel);

	public:
		/*! \brief Constants used to determine the code generation mode:
		 *  OriginalSql: generates the SQL for the object only (original behavior)
		 *  DependenciesSql: generates the original SQL code + dependencies SQL
		 *  ChildrenSql: generates the original SQL code + children SQL
		 *  GroupByType: generates the original SQL code but grouping them in a single file
		 *  for each object type. */
		enum CodeGenMode: unsigned {
			OriginalSql,
			DependenciesSql,
			ChildrenSql,
			GroupByType
		};

		enum MetaAttrOptions: unsigned {
			MetaNoOpts=0,
			MetaDbAttributes=1,	//! \brief Handle database model attribute when save/load metadata file
			MetaObjsPositioning=2,	//! \brief Handle objects' positioning when save/load metadata file
			MetaObjsProtection=4,	//! \brief Handle objects' protection status when save/load metadata file
			MetaObjsSqlDisabled=8,	//! \brief Handle objects' sql disabled status when save/load metadata file
			MetaObjsCustomSql=16,	//! \brief Handle object's custom sql when save/load metadata file
			MetaObjsCustomColors=32,	//! \brief Handle object's custom colors when save/load metadata file
			MetaObjsFadeOut=64,	//! \brief Handle graphical object's fade out status when save/load metadata file
			MetaObjsCollapseMode=128,	//! \brief Handle tables and views collapse mode when save/load metadata file
			MetaTextboxObjs=256,	//! \brief Handle textboxes object when save/load metadata file
			MetaTagObjs=512,	//! \brief Handle tags object when save/load metadata file
			MetaGenericSqlObjs=1024,	//! \brief Handle generic sql object when save/load metadata file
			MetaObjsAliases=2048,	//! \brief Handle the object's aliases (graphical objects and table children objects) when save/load metadata file
			MetaObjsZStackValue=4096,	//! \brief Handle the object's Z stack value
			MetaObjsLayersConfig=8192,	//! \brief Handle all the configuration related to layers
			MetaMergeDuplicatedObjs=16384,	//! \brief Merges duplicated textboxes, tags and generic SQL objects
			MetaAllInfo=32767	//! \brief Handle all metadata information about objects when save/load metadata file
		};

		DatabaseModel();

		//! \brief Creates a database model and assign the model widget which will manage this instance
		explicit DatabaseModel(ModelWidget *model_wgt);

		~DatabaseModel() override;

		//! \brief Returns the model widget that is managing the current database instance
		ModelWidget *getModelWidget();

		//! \brief Returns the complete object list according to the type
		std::vector<BaseObject *> *getObjectList(ObjectType obj_type);

		//! \brief Disconnects all the relationships in a ordered way
		void disconnectRelationships();

		/*! \brief Detects and stores the XML for special objects (that is referencing columns created
		 by relationship) in order to be reconstructed in a posterior moment */
		void storeSpecialObjectsXML();

		/*! \brief Validates all relationships, propagating all column modifications over the tables when needed.
		 *  This method returns false when no relationship was invalid and true when the method validated (reconnected)
		 *  at least one relationship */
		bool validateRelationships();

		/*! \brief Returns an object seaching it by its name and on the group objects specified by "types".
		 * If the types list is empty the method will return nullptr. */
		BaseObject *getObject(const QString &name, const std::vector<ObjectType> &types);

		//! \brief Returns the object by searching it by its pg_oid
		BaseObject *getObjectByOid(unsigned oid, ObjectType obj_type);

		//! \brief Returns the list of specified object type that belongs to the passed schema
		std::vector<BaseObject *> getObjects(ObjectType obj_type, BaseObject *schema=nullptr);

		//! \brief Returns the list of objects (all types) that belongs to the passed schema
		std::vector<BaseObject *> getObjects(BaseObject *schema);

		//! \brief Returns the object index searching by its name
		int getObjectIndex(const QString &name, ObjectType obj_type);

		//! \brief Retuns the passed object index
		int getObjectIndex(BaseObject *object);

		//! \brief Adds an object to the model
		void addObject(BaseObject *object, int obj_idx=-1);

		//! \brief Removes an object from the model
		void removeObject(BaseObject *object, int obj_idx=-1);

		//! \brief Returns an object from the model using its index and type
		BaseObject *getObject(unsigned obj_idx, ObjectType obj_type);

		/*! \brief Loads a database model from a file. In case of loading errors
		the objects in the model will not be destroyed automatically. The user need to call
		destroyObjects() or delete the entire model */
		void loadModel(const QString &filename);

		//! \brief Sets the database encoding
		void setEncoding(EncodingType encod);

		//! \brief Sets the database localizations
		void setLocalization(unsigned localiz_id, const QString &value);

		//! \brief Sets the connections limit
		void setConnectionLimit(int conn_lim);

		//! \brief Sets the template database
		void setTemplateDB(const QString &temp_db);

		//! \brief Sets the model's author
		void setAuthor(const QString &author);

		//! \brief Sets the protection for all objects on the model
		void setProtected(bool value) override;

		//! \brief Sets the sql appending at end of entire model definition
		void setAppendAtEOD(bool value);

		//! \brief Sets the sql prepeding at beginning of entire model definition
		void setPrependAtBOD(bool value);

		void setDefaultObject(BaseObject *object, ObjectType obj_type=ObjectType::BaseObject);

		void setIsTemplate(bool value);

		void setAllowConnections(bool value);

		//! \brief Returns the current state of the sql appeding at end of entire model definition
		bool isAppendAtEOD();

		//! \brief Returns the current state of the sql prepeding at beginning of entire model definition
		bool isPrependedAtBOD();

		bool isTemplate();

		bool isAllowConnections();

		//! \brief Destroys all the objects
		void destroyObjects();

		//! \brief Returns the object count for the specified type
		unsigned getObjectCount(ObjectType obj_type);

		//! \brief Returns the object count for all object types.
		unsigned getObjectCount();

		unsigned getObjectsCount(const std::vector<ObjectType> &obj_types);

		unsigned getMaxObjectCount();

		//! \brief Retuns the specified localization value
		QString getLocalization(unsigned localiz_id);

		//! \brief Returns the connection limit
		int getConnectionLimit();

		//! \brief Returns the template database
		QString getTemplateDB();

		//! \brief Returns the model's author
		QString getAuthor();

		//! \brief Returns the database enconding
		EncodingType getEncoding();

		BaseObject *getDefaultObject(ObjectType obj_type);

		//! \brief Returns if the model is invalidated. When true its recommended to validate model using Model validation tool
		bool isInvalidated();

		//! \brief Indicate if the model invalidated
		void setInvalidated(bool value);

		//! \brief Saves the specified code definition for the model on the specified filename
		void saveModel(const QString &filename, SchemaParser::CodeType def_type);

		/*! \brief Saves the model's SQL code definition by creating separated files for each object
		 * The provided path must be a directory. If it does not exists then the method will create
		 * it prior to the generation of the files. */
		void saveSplitSQLDefinition(const QString &path, CodeGenMode code_gen_mode = OriginalSql, bool gen_drop_file = false);

		/*! \brief Returns the complete SQL/XML defintion for the entire model (including all the other objects).
		 The parameter 'export_file' is used to format the generated code in a way that can be saved
		 in na SQL file and executed later on the DBMS server. This parameter is only used for SQL definition. */
		QString getSourceCode(SchemaParser::CodeType def_type, bool export_file) final;

		//! \brief Returns the complete SQL/XML definition for the entire model (including all the other objects).
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Returns the code definition only for the database (excluding the definition of the other objects)
		QString __getSourceCode(SchemaParser::CodeType def_type);

		/*! \brief Returns the code definition for the specified objects.
		 *  This method receives the code generation mode option which can be:
		 *  OriginalSql: generates only the original SQL code of the object.
		 *  DependenciesSql: generates the original code plus all dependencies needed to properly create the object.
		 *  ChildrenSql: generates the original code plus all object's children SQL code. This option is used only by schemas, tables and views.
		 */
		//QString getSQLDefinition(BaseObject *object, CodeGenMode code_gen_mode = OriginalSql);
		QString getSQLDefinition(const std::vector<BaseObject *> objects, CodeGenMode code_gen_mode);

		/*! \brief Returns the creation order of objects in each definition type (SQL or XML).

		The parameter incl_relnn_objs when 'true' includes the generated objects (table and constraint)
		of the many-to-many relationships instead of the relationships themselves. The incl_relnn_objs is
		is accepted only when the creation order for SQL code is being generated, for XML, it'll simply ignored.

		The parameter incl_rel1n_constr when 'true' includes the generated foreign and unique keys
		of one-to-one|many relationships instead of the relationships themselves. This parameter is
		is accepted only when the creation order for SQL code is being generated, for XML, it'll simply ignored.

		The parameter realloc_fk_perms causes foreign keys and permissions to have their position changed to the
		end of the creation order to avoid being created before their parent objects or referenced objects */
		std::map<unsigned, BaseObject *> getCreationOrder(SchemaParser::CodeType def_type,
																											bool incl_relnn_objs = false, bool incl_rel1n_constrs = false,
																											bool realloc_fk_perms = true);

		/*! \brief Returns a list containig all the object need to create the 'object' in the proper order.
		If 'only_children' is set only children objects will be included in the list (for tables, views or schemas).
		If 'only_children' is not set, the method will automatically include dependencies, children and permissions of
		the object. */
		std::vector<BaseObject *> getCreationOrder(BaseObject *object, bool only_children);

		void addRelationship(BaseRelationship *rel, int obj_idx=-1);
		void removeRelationship(BaseRelationship *rel, int obj_idx=-1);
		BaseRelationship *getRelationship(unsigned obj_idx, ObjectType rel_type);
		BaseRelationship *getRelationship(const QString &name);

		/*! \brief Searchs and returns the relationship between the specified tables. If the second parameter
		 is ommited (nullptr), the method returns the first relationship where the source table is
		 participating. The optional parameter ref_fk will search for foreign key relationships which the reference foreign key
		is the one provided */
		BaseRelationship *getRelationship(BaseTable *src_tab, BaseTable *dst_tab, Constraint *ref_fk = nullptr);

		//! \brief Searchs and returns all the relationships that the specified table participates
		std::vector<BaseRelationship *> getRelationships(BaseTable *tab);

		void addTextbox(Textbox *txtbox, int obj_idx=-1);
		void removeTextbox(Textbox *txtbox, int obj_idx=-1);
		Textbox *getTextbox(unsigned obj_idx);
		Textbox *getTextbox(const QString &name);

		void addFunction(Function *func, int obj_idx=-1);
		void removeFunction(Function *func, int obj_idx=-1);
		Function *getFunction(unsigned obj_idx);
		Function *getFunction(const QString &signature);

		void addSchema(Schema *schema, int obj_idx=-1);
		void removeSchema(Schema *schema, int obj_idx=-1);
		Schema *getSchema(unsigned obj_idx);
		Schema *getSchema(const QString &name);

		void addView(View *view, int obj_idx=-1);
		void removeView(View *view, int obj_idx=-1);
		View *getView(unsigned obj_idx);
		View *getView(const QString &name);

		void addTable(Table *table, int obj_idx=-1);
		void removeTable(Table *table, int obj_idx=-1);
		Table *getTable(unsigned obj_idx);
		Table *getTable(const QString &name);

		void addType(Type *type, int obj_idx=-1);
		void removeType(Type *type, int obj_idx=-1);
		Type *getType(unsigned obj_idx);
		Type *getType(const QString &name);

		void addRole(Role *role, int obj_idx=-1);
		void removeRole(Role *role, int obj_idx=-1);
		Role *getRole(unsigned obj_idx);
		Role *getRole(const QString &name);

		void addTablespace(Tablespace *tabspc, int obj_idx=-1);
		void removeTablespace(Tablespace *tabspc, int obj_idx=-1);
		Tablespace *getTablespace(unsigned obj_idx);
		Tablespace *getTablespace(const QString &name);

		void addLanguage(Language *lang, int obj_idx=-1);
		void removeLanguage(Language *lang, int obj_idx=-1);
		Language *getLanguage(unsigned obj_idx);
		Language *getLanguage(const QString &name);

		void addAggregate(Aggregate *aggreg, int obj_idx=-1);
		void removeAggregate(Aggregate *aggreg, int obj_idx=-1);
		Aggregate *getAggregate(unsigned obj_idx);
		Aggregate *getAggregate(const QString &name);

		void addCast(Cast *cast, int obj_idx=-1);
		void removeCast(Cast *cast, int obj_idx=-1);
		Cast *getCast(unsigned obj_idx);
		Cast *getCast(const QString &name);

		void addConversion(Conversion *conv, int obj_idx=-1);
		void removeConversion(Conversion *conv, int obj_idx=-1);
		Conversion *getConversion(unsigned obj_idx);
		Conversion *getConversion(const QString &name);

		void addOperator(Operator *oper, int obj_idx=-1);
		void removeOperator(Operator *oper, int obj_idx=-1);
		Operator *getOperator(unsigned obj_idx);
		Operator *getOperator(const QString &signature);

		void addOperatorClass(OperatorClass *op_class, int obj_idx=-1);
		void removeOperatorClass(OperatorClass *op_class, int obj_idx=-1);
		OperatorClass *getOperatorClass(unsigned obj_idx);
		OperatorClass *getOperatorClass(const QString &name);

		void addOperatorFamily(OperatorFamily *op_family, int obj_idx=-1);
		void removeOperatorFamily(OperatorFamily *op_family, int obj_idx=-1);
		OperatorFamily *getOperatorFamily(unsigned obj_idx);
		OperatorFamily *getOperatorFamily(const QString &name);

		void addDomain(Domain *domain, int obj_idx=-1);
		void removeDomain(Domain *domain, int obj_idx=-1);
		Domain *getDomain(unsigned obj_idx);
		Domain *getDomain(const QString &name);

		void addSequence(Sequence *sequence, int obj_idx=-1);
		void removeSequence(Sequence *sequence, int obj_idx=-1);
		Sequence *getSequence(unsigned obj_idx);
		Sequence *getSequence(const QString &name);

		void addCollation(Collation *collation, int obj_idx=-1);
		void removeCollation(Collation *collation, int obj_idx=-1);
		Collation *getCollation(unsigned obj_idx);
		Collation *getCollation(const QString &name);

		void addExtension(Extension *extension, int obj_idx=-1);
		void removeExtension(Extension *extension, int obj_idx=-1);
		Extension *getExtension(unsigned obj_idx);
		Extension *getExtension(const QString &name);

		void addTag(Tag *tag, int obj_idx=-1);
		void removeTag(Tag *tag, int obj_idx=-1);
		Tag *getTag(unsigned obj_idx);
		Tag *getTag(const QString &name);

		void addEventTrigger(EventTrigger *evnttrig, int obj_idx=-1);
		void removeEventTrigger(EventTrigger *evnttrig, int obj_idx=-1);
		EventTrigger *getEventTrigger(unsigned obj_idx);
		EventTrigger *getEventTrigger(const QString &name);

		void addGenericSQL(GenericSQL *genericsql, int obj_idx=-1);
		void removeGenericSQL(GenericSQL *genericsql, int obj_idx=-1);
		GenericSQL *getGenericSQL(unsigned obj_idx);
		GenericSQL *getGenericSQL(const QString &name);

		void addForeignDataWrapper(ForeignDataWrapper *fdata_wrapper, int obj_idx=-1);
		void removeForeignDataWrapper(ForeignDataWrapper *fdata_wrapper, int obj_idx=-1);
		ForeignDataWrapper *getForeignDataWrapper(unsigned obj_idx);
		ForeignDataWrapper *getForeignDataWrapper(const QString &name);

		void addForeignServer(ForeignServer *server, int obj_idx=-1);
		void removeForeignServer(ForeignServer *server, int obj_idx=-1);
		ForeignServer *getForeignServer(unsigned obj_idx);
		ForeignServer *getForeignServer(const QString &name);

		void addUserMapping(UserMapping *usrmap, int obj_idx=-1);
		void removeUserMapping(UserMapping *usrmap, int obj_idx=-1);
		UserMapping *getUserMapping(unsigned obj_idx);
		UserMapping *getUserMapping(const QString &name);

		void addForeignTable(ForeignTable *table, int obj_idx=-1);
		void removeForeignTable(ForeignTable *table, int obj_idx=-1);
		ForeignTable *getForeignTable(unsigned obj_idx);
		ForeignTable *getForeignTable(const QString &name);

		void addTransform(Transform *transf, int obj_idx=-1);
		void removeTransform(Transform *transf, int obj_idx=-1);
		Transform *getTransform(unsigned obj_idx);
		Transform *getTransform(const QString &name);

		void addProcedure(Procedure *proc, int obj_idx=-1);
		void removeProcedure(Procedure *proc, int obj_idx=-1);
		Procedure *getProcedure(unsigned obj_idx);
		Procedure *getProcedure(const QString &name);

		void addPermission(Permission *perm);
		void removePermission(Permission *perm);

		/*! \brief Returns the specified permission's index. If exact_match is true
		then all the contents of the permission are compared against other permission,
		otherwise checks only if the roles and the objects are the same between the specified
		permission and the ones existent in the model */
		int getPermissionIndex(Permission *perm, bool exact_match);

		//! \brief Inserts a list of permissions into the model
		void addPermissions(const std::vector<Permission *> &perms);

		//! \brief Removes all the permission related to the passed object
		void removePermissions(BaseObject *object);

		//! \brief Returns all the permissions related to the passed object
		void getPermissions(BaseObject *object, std::vector<Permission *> &perms);

		/*! \brief Returns the object searching by its signature and type.
		 *  If the signature does not match the provided name then the method
		 *  tries to match the name parameter against the object's name */
		BaseObject *getObject(const QString &name, ObjectType obj_type);

		void configureDatabase(attribs_map &attribs);
		PgSqlType createPgSQLType();
		BaseObject *createObject(ObjectType obj_type);
		Role *createRole();
		Tablespace *createTablespace();
		Schema *createSchema();
		Language *createLanguage();
		Function *createFunction();
		Parameter createParameter();
		TypeAttribute createTypeAttribute();
		Type *createType();
		Domain *createDomain();
		Cast *createCast();
		Conversion *createConversion();
		Operator *createOperator();
		OperatorFamily *createOperatorFamily();
		OperatorClass *createOperatorClass();
		Aggregate *createAggregate();
		Table *createTable();
		Column *createColumn();
		Sequence *createSequence(bool ignore_onwer=false);
		View *createView();
		Collation *createCollation();
		Extension *createExtension();
		Tag *createTag();
		Permission *createPermission();
		Textbox *createTextbox();
		BaseRelationship *createRelationship();
		Constraint *createConstraint(BaseObject *parent_obj);
		Rule *createRule();
		Index *createIndex();
		Trigger *createTrigger();
		Policy *createPolicy();
		EventTrigger *createEventTrigger();
		GenericSQL *createGenericSQL();
		ForeignDataWrapper *createForeignDataWrapper();
		ForeignServer *createForeignServer();
		UserMapping *createUserMapping();
		ForeignTable *createForeignTable();
		Transform *createTransform();
		Procedure *createProcedure();

		template<class TableClass>
		TableClass *createPhysicalTable();

		//! \brief Update views that reference the provided table forcing the column name deduction and redraw of the former objects
		void updateViewsReferencingTable(PhysicalTable *table);

		//! \brief Creates/removes the relationship between the passed view and the referecend tables
		void updateViewRelationships(View *view, bool force_rel_removal=false);

		//! \brief Creates/removes the relationship between the passed table and the referecend tables on its foreign keys
		void updateTableFKRelationships(Table *table);

		//! \brief Updates the fk relationships for all table on the model
		void updateTablesFKRelationships();

		//! \brief Validates the relationship to reflect the modifications on the column/constraint of the passed table
		void validateRelationships(TableObject *object, Table *parent_tab);

		/*! \brief Checks if from the passed relationship some redundacy is found. Redundancy generates infinite column
		 propagation over the tables. This method raises an error when found some. */
		void checkRelationshipRedundancy(Relationship *rel);

		/*! \brief Marks the graphical objects of the provided types as modified forcing their redraw. User can specify only a set of
	 graphical objects to be marked */
		void setObjectsModified(std::vector<ObjectType> types={});

		//! \brief Marks the graphical objects in the list as modified forcing their redraw.
		void setObjectsModified(std::vector<BaseObject *> &objects);

		/*! \brief Marks the objects with code invalidated forcing their code regeneration. User can specify only a set of
		 graphical objects to be marked */
		void setCodesInvalidated(std::vector<ObjectType> types={});

		/*! \brief Updates the user type names which belongs to the passed schema. This method must be executed whenever
		 the schema is renamed to propagate the new name to the user types on the PgSQLTypes list. Additionally
		 the previous schema name must be informed in order to rename the types correctly */
		void validateSchemaRenaming(Schema *schema, const QString &prev_sch_name);

		/*! \brief Creates the system objects: public schema and languages C, SQL and plpgsql. This method ignores one of these
		objects if some of them already exists */
		void createSystemObjects(bool create_public);

		/*! \brief Returns a list of object searching them using the specified pattern. The search can be delimited by filtering the object's types.
		The additional bool params are: case sensitive name search, name pattern is a regexp, exact match for names. */
		std::vector<BaseObject *> findObjects(const QString &pattern, std::vector<ObjectType> types,
																					bool case_sensitive, bool is_regexp, bool exact_match,
																					const QString &search_attr = Attributes::Name);

		/*! \brief Returns a list of objects searching them by using the filter syntax [type]:[pattern]:[mode]
		 * The provided list of filter strings is composed by:
		 * > The object type (schema name, see BaseObject::getSchemaName())
		 * > The search pattern itself
		 * > The pattern mode (regexp | wildcard)
		 * Additionally the search attribute can be provided so the search may occurr in other attributes instead of the default (name)
		 * The parameter any_incl_cols indicates if the any filter type must filter columns too */
		std::vector<BaseObject *> findObjects(const QStringList &filters, const QString &search_attr = Attributes::Name, bool any_incl_cols = false);

		void setLastPosition(const QPoint &pnt);
		QPoint getLastPosition();

		void setLastZoomFactor(double zoom);
		double getLastZoomFactor();

		/*! \brief This method exposes the XML parser for the outside world. In order to create objects from xml code inside the current
		 database model you need first get the parser (through this method), populate the parser with the desired XML and then call
		 the create* method.

		\note: This is not the better approach and certainly will be changed in future releases */
		XmlParser *getXMLParser();

		//! \brief Returns the ALTER definition between the current model and the provided one
		QString getAlterCode(BaseObject *object) final;

		//! \brief Returns the data dictionary of all tables in a single HTML code
		void getDataDictionary(attribs_map &datadict, bool browsable, bool split, bool md_format);

		//! \brief Saves the data dictionary of all tables in a single HTML file or splitted in several files for each table
		void saveDataDictionary(const QString &path, bool browsable, bool split, bool md_format);

		/*! \brief Save the graphical objects positions, custom colors and custom points (for relationship lines) to an special file
				that can be loaded by another model in order to change their objects position */
		void saveObjectsMetadata(const QString &filename, MetaAttrOptions options=MetaAllInfo);

		//! \brief Load the file containing the objects positioning to be applied to the model
		void loadObjectsMetadata(const QString &filename, MetaAttrOptions options=MetaAllInfo);

		/*! \brief Returns a search filter from the objects in the change log.
		 * It's possible to specify a date interval to contrain the entries
		 * retrieved from changelog. */
		QStringList getFiltersFromChangelog(QDateTime start, QDateTime end);

		//! \brief Enable the persistence of the internal changelog
		void setPersistedChangelog(bool persist);

		//! \brief Returns true when the internal changelog is being persisted to the dbm file
		bool isPersistedChangelog();

		//! \brief Clears the changelog
		void clearChangelog();

		//! \brief Returns the last changelog entry date
		QDateTime getLastChangelogDate();

		//! \brief Returns the first changelog entry date
		QDateTime getFirstChangelogDate();

		//! \brief Returns the amount of entries in the changelog
		unsigned getChangelogLength(Operation::OperType op_type = Operation::NoOperation);

		QStringList getLayers();
		QStringList getLayerNameColors();
		QList<unsigned> getActiveLayers();
		QStringList getLayerRectColors();

		bool isLayerNamesVisible();
		bool isLayerRectsVisible();

		/*! \brief Fills the provided attributes map with the database model attributes only.
		 *  The result varies according to the code_type provided */
		void setDatabaseModelAttributes(attribs_map &attribs, SchemaParser::CodeType code_type);

		//! \brief Returns the XML code for the changelog
		QString getChangelogDefinition(bool csv_format = false);

		/*! \brief Defines the current scene rectangle in which the model is being rendered
		 *  This is used to restore the original scene geometry when the model is loaded from file */
		void setSceneRect(const QRectF &rect);
		QRectF getSceneRect();

		//! \brief Toggles the generation of SQL code of object with code disabled
		void setGenDisabledObjsCode(bool value);

		bool isGenDisabledObjsCode();

		//! \brief Toggles the display of system schemas rectangles
		void setShowSysSchemasRects(bool value);

	signals:
		//! \brief Signal emitted when a new object is added to the model
		void s_objectAdded(BaseObject *object);

		//! \brief Signal emitted when an object is removed from the model
		void s_objectRemoved(BaseObject *object);

		//! \brief Signal emitted when an object is created from a xml code
		void s_objectLoaded(int progress, QString object_id, unsigned obj_type);

	friend class DatabaseImportHelper;
	friend class ModelExportHelper;
	friend class ModelWidget;
	friend class PgModelerCliApp;
	friend class OperationList;
	friend class PermissionWidget;
	friend class ExtensionWidget;
};

#endif
