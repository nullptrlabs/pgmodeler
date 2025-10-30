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

#include "databasemodel.h"
#include "coreutilsns.h"
#include "defaultlanguages.h"
#include <QtDebug>
#include <random>
#include "utilsns.h"
#include "doublenan.h"

unsigned DatabaseModel::dbmodel_id {2000};

DatabaseModel::DatabaseModel()
{
	this->model_wgt=nullptr;
	object_id=DatabaseModel::dbmodel_id++;
	obj_type=ObjectType::Database;

	layers.append(tr("Default layer"));
	active_layers.push_back(0);
	layer_name_colors.append(QColor(0,0,0).name());
	layer_rect_colors.append(QColor(180,180,180).name());

	is_layer_names_visible = is_layer_rects_visible = false;
	persist_changelog = false;
	is_template = false;
	allow_conns = true;
	cancel_saving = false;
	gen_dis_objs_code = false;
	show_sys_sch_rects = true;

	encoding=EncodingType::Null;
	BaseObject::setName(QObject::tr("new_database"));

	default_objs[ObjectType::Schema]=nullptr;
	default_objs[ObjectType::Role]=nullptr;
	default_objs[ObjectType::Tablespace]=nullptr;
	default_objs[ObjectType::Collation]=nullptr;

	conn_limit=-1;
	last_zoom=1;
	loading_model=invalidated=append_at_eod=prepend_at_bod=false;
	attributes[Attributes::Encoding]="";
	attributes[Attributes::TemplateDb]="";
	attributes[Attributes::ConnLimit]="";
	attributes[Attributes::LcCollate]="";
	attributes[Attributes::LcCtype]="";
	attributes[Attributes::AppendAtEod]="";
	attributes[Attributes::PrependAtBod]="";
	attributes[Attributes::AllowConns]="";
	attributes[Attributes::IsTemplate]="";
	attributes[Attributes::UseChangelog]="";
	attributes[Attributes::Changelog]="";
	attributes[Attributes::GenDisabledObjsCode]="";

	obj_lists = {
		{ ObjectType::Textbox, &textboxes },
		{ ObjectType::Table, &tables },
		{ ObjectType::Function, &functions },
		{ ObjectType::Aggregate, &aggregates },
		{ ObjectType::Schema, &schemas },
		{ ObjectType::View, &views },
		{ ObjectType::Type, &types },
		{ ObjectType::Role, &roles },
		{ ObjectType::Tablespace, &tablespaces },
		{ ObjectType::Language, &languages },
		{ ObjectType::Cast, &casts },
		{ ObjectType::Conversion, &conversions },
		{ ObjectType::Operator, &operators },
		{ ObjectType::OpClass, &op_classes },
		{ ObjectType::OpFamily, &op_families },
		{ ObjectType::Domain, &domains },
		{ ObjectType::Sequence, &sequences },
		{ ObjectType::BaseRelationship, &base_relationships },
		{ ObjectType::Relationship, &relationships },
		{ ObjectType::Permission, &permissions },
		{ ObjectType::Collation, &collations },
		{ ObjectType::Extension, &extensions },
		{ ObjectType::Tag, &tags },
		{ ObjectType::EventTrigger, &eventtriggers },
		{ ObjectType::GenericSql, &genericsqls },
		{ ObjectType::ForeignDataWrapper, &fdata_wrappers },
		{ ObjectType::ForeignServer, &foreign_servers },
		{ ObjectType::UserMapping, &usermappings },
		{ ObjectType::ForeignTable, &foreign_tables },
		{ ObjectType::Transform, &transforms },
		{ ObjectType::Procedure, &procedures }
	};

	// Binding the creation methods
	create_methods = {
		{ ObjectType::Role, std::bind(&DatabaseModel::createRole, this) },
		{ ObjectType::Tablespace, std::bind(&DatabaseModel::createTablespace, this) },
		{ ObjectType::Schema, std::bind(&DatabaseModel::createSchema, this) },
		{ ObjectType::Language, std::bind(&DatabaseModel::createLanguage, this) },
		{ ObjectType::Function, std::bind(&DatabaseModel::createFunction, this) },
		{ ObjectType::Type, std::bind(&DatabaseModel::createType, this) },
		{ ObjectType::Domain, std::bind(&DatabaseModel::createDomain, this) },
		{ ObjectType::Cast, std::bind(&DatabaseModel::createCast, this) },
		{ ObjectType::Conversion, std::bind(&DatabaseModel::createConversion, this) },
		{ ObjectType::Operator, std::bind(&DatabaseModel::createOperator, this) },
		{ ObjectType::OpFamily, std::bind(&DatabaseModel::createOperatorFamily, this) },
		{ ObjectType::OpClass, std::bind(&DatabaseModel::createOperatorClass, this) },
		{ ObjectType::Aggregate, std::bind(&DatabaseModel::createAggregate, this) },
		{ ObjectType::Table, std::bind(&DatabaseModel::createTable, this) },
		{ ObjectType::View, std::bind(&DatabaseModel::createView, this) },
		{ ObjectType::Textbox, std::bind(&DatabaseModel::createTextbox, this) },
		{ ObjectType::Trigger, std::bind(&DatabaseModel::createTrigger, this) },
		{ ObjectType::Index, std::bind(&DatabaseModel::createIndex, this) },
		{ ObjectType::Column, std::bind(&DatabaseModel::createColumn, this) },
		{ ObjectType::Rule, std::bind(&DatabaseModel::createRule, this) },
		{ ObjectType::Relationship, std::bind(&DatabaseModel::createRelationship, this) },
		{ ObjectType::BaseRelationship, std::bind(&DatabaseModel::createRelationship, this) },
		{ ObjectType::Collation, std::bind(&DatabaseModel::createCollation, this) },
		{ ObjectType::Extension, std::bind(&DatabaseModel::createExtension, this) },
		{ ObjectType::Tag, std::bind(&DatabaseModel::createTag, this) },
		{ ObjectType::Permission, std::bind(&DatabaseModel::createPermission, this) },
		{ ObjectType::EventTrigger, std::bind(&DatabaseModel::createEventTrigger, this) },
		{ ObjectType::GenericSql, std::bind(&DatabaseModel::createGenericSQL, this) },
		{ ObjectType::Policy, std::bind(&DatabaseModel::createPolicy, this) },
		{ ObjectType::ForeignDataWrapper, std::bind(&DatabaseModel::createForeignDataWrapper, this) },
		{ ObjectType::ForeignServer, std::bind(&DatabaseModel::createForeignServer, this) },
		{ ObjectType::UserMapping, std::bind(&DatabaseModel::createUserMapping, this) },
		{ ObjectType::ForeignTable, std::bind(&DatabaseModel::createForeignTable, this) },
		{ ObjectType::Transform, std::bind(&DatabaseModel::createTransform, this) },
		{ ObjectType::Procedure, std::bind(&DatabaseModel::createProcedure, this) },
		{ ObjectType::Sequence, [this](){ return createSequence(); } },
		{ ObjectType::Constraint, [this](){ return createConstraint(nullptr); } }
	};

	//Binding add methods
	add_methods = {
		{ ObjectType::Relationship, [this](BaseObject *obj, int idx){
				addRelationship(dynamic_cast<BaseRelationship *>(obj), idx); }},
		{ ObjectType::BaseRelationship, [this](BaseObject *obj, int idx){
				addRelationship(dynamic_cast<BaseRelationship *>(obj), idx); }},
		{ ObjectType::Textbox, [this](BaseObject *obj, int idx){
				addTextbox(dynamic_cast<Textbox *>(obj), idx); }},
		{ ObjectType::Table, [this](BaseObject *obj, int idx){
				addTable(dynamic_cast<Table *>(obj), idx); }},
		{ ObjectType::Function, [this](BaseObject *obj, int idx){
				addFunction(dynamic_cast<Function *>(obj), idx); }},
		{ ObjectType::Aggregate, [this](BaseObject *obj, int idx){
				addAggregate(dynamic_cast<Aggregate *>(obj), idx); }},
		{ ObjectType::Schema, [this](BaseObject *obj, int idx){
				addSchema(dynamic_cast<Schema *>(obj), idx); }},
		{ ObjectType::View, [this](BaseObject *obj, int idx){
				addView(dynamic_cast<View *>(obj), idx); }},
		{ ObjectType::Type, [this](BaseObject *obj, int idx){
				addType(dynamic_cast<Type *>(obj), idx); }},
		{ ObjectType::Role, [this](BaseObject *obj, int idx){
				addRole(dynamic_cast<Role *>(obj), idx); }},
		{ ObjectType::Tablespace, [this](BaseObject *obj, int idx){
				addTablespace(dynamic_cast<Tablespace *>(obj), idx); }},
		{ ObjectType::Language, [this](BaseObject *obj, int idx){
				addLanguage(dynamic_cast<Language *>(obj), idx); }},
		{ ObjectType::Cast,	[this](BaseObject *obj, int idx){
				addCast(dynamic_cast<Cast *>(obj), idx); }},
		{ ObjectType::Conversion, [this](BaseObject *obj, int idx){
				addConversion(dynamic_cast<Conversion *>(obj), idx); }},
		{ ObjectType::Operator, [this](BaseObject *obj, int idx){
				addOperator(dynamic_cast<Operator *>(obj), idx); }},
		{ ObjectType::OpClass, [this](BaseObject *obj, int idx){
				addOperatorClass(dynamic_cast<OperatorClass *>(obj), idx); }},
		{ ObjectType::OpFamily, [this](BaseObject *obj, int idx){
				addOperatorFamily(dynamic_cast<OperatorFamily *>(obj), idx); }},
		{ ObjectType::Domain,	[this](BaseObject *obj, int idx){
				addDomain(dynamic_cast<Domain *>(obj), idx); }},
		{ ObjectType::Sequence, [this](BaseObject *obj, int idx){
				addSequence(dynamic_cast<Sequence *>(obj), idx); }},
		{ ObjectType::Collation, [this](BaseObject *obj, int idx){
				addCollation(dynamic_cast<Collation *>(obj), idx); }},
		{ ObjectType::Extension, [this](BaseObject *obj, int idx){
				addExtension(dynamic_cast<Extension *>(obj), idx); }},
		{ ObjectType::Tag, [this](BaseObject *obj, int idx){
				addTag(dynamic_cast<Tag *>(obj), idx); }},
		{ ObjectType::EventTrigger, [this](BaseObject *obj, int idx){
				addEventTrigger(dynamic_cast<EventTrigger *>(obj), idx); }},
		{ ObjectType::GenericSql,[this](BaseObject *obj, int idx){
				addGenericSQL(dynamic_cast<GenericSQL *>(obj), idx); }},
		{ ObjectType::ForeignDataWrapper, [this](BaseObject *obj, int idx){
				addForeignDataWrapper(dynamic_cast<ForeignDataWrapper *>(obj), idx); }},
		{ ObjectType::ForeignServer, [this](BaseObject *obj, int idx){
				addForeignServer(dynamic_cast<ForeignServer *>(obj), idx); }},
		{ ObjectType::UserMapping, [this](BaseObject *obj, int idx){
				addUserMapping(dynamic_cast<UserMapping *>(obj), idx); }},
		{ ObjectType::ForeignTable, [this](BaseObject *obj, int idx){
				addForeignTable(dynamic_cast<ForeignTable *>(obj), idx); }},
		{ ObjectType::Transform, [this](BaseObject *obj, int idx){
				addTransform(dynamic_cast<Transform *>(obj), idx); }},
		{ ObjectType::Procedure, [this](BaseObject *obj, int idx){
				addProcedure(dynamic_cast<Procedure *>(obj), idx); }},
		{ ObjectType::Permission, [this](BaseObject *obj, int){
				addPermission(dynamic_cast<Permission *>(obj)); }},
	};

	//Binding remove methods
	remove_methods = {
		{ ObjectType::Relationship, [this](BaseObject *obj, int idx){
				removeRelationship(dynamic_cast<BaseRelationship *>(obj), idx);	}},
		{ ObjectType::BaseRelationship, [this](BaseObject *obj, int idx){
				removeRelationship(dynamic_cast<BaseRelationship *>(obj), idx);	}},
		{ ObjectType::Textbox, [this](BaseObject *obj, int idx){
				removeTextbox(dynamic_cast<Textbox *>(obj), idx);	}},
		{ ObjectType::Table, [this](BaseObject *obj, int idx){
				removeTable(dynamic_cast<Table *>(obj), idx);	}},
		{ ObjectType::Function, [this](BaseObject *obj, int idx){
				removeFunction(dynamic_cast<Function *>(obj), idx);	}},
		{ ObjectType::Aggregate, [this](BaseObject *obj, int idx){
				removeAggregate(dynamic_cast<Aggregate *>(obj), idx);	}},
		{ ObjectType::Schema, [this](BaseObject *obj, int idx){
				removeSchema(dynamic_cast<Schema *>(obj), idx);	}},
		{ ObjectType::View, [this](BaseObject *obj, int idx){
				removeView(dynamic_cast<View *>(obj), idx);	}},
		{ ObjectType::Type, [this](BaseObject *obj, int idx){
				removeType(dynamic_cast<Type *>(obj), idx);	}},
		{ ObjectType::Role, [this](BaseObject *obj, int idx){
				removeRole(dynamic_cast<Role *>(obj), idx);	}},
		{ ObjectType::Tablespace, [this](BaseObject *obj, int idx){
				removeTablespace(dynamic_cast<Tablespace *>(obj), idx);	}},
		{ ObjectType::Language, [this](BaseObject *obj, int idx){
				removeLanguage(dynamic_cast<Language *>(obj), idx);	}},
		{ ObjectType::Cast,	[this](BaseObject *obj, int idx){
				removeCast(dynamic_cast<Cast *>(obj), idx);	}},
		{ ObjectType::Conversion, [this](BaseObject *obj, int idx){
				removeConversion(dynamic_cast<Conversion *>(obj), idx);	}},
		{ ObjectType::Operator, [this](BaseObject *obj, int idx){
				removeOperator(dynamic_cast<Operator *>(obj), idx);	}},
		{ ObjectType::OpClass, [this](BaseObject *obj, int idx){
				removeOperatorClass(dynamic_cast<OperatorClass *>(obj), idx);	}},
		{ ObjectType::OpFamily, [this](BaseObject *obj, int idx){
				removeOperatorFamily(dynamic_cast<OperatorFamily *>(obj), idx);	}},
		{ ObjectType::Domain,	[this](BaseObject *obj, int idx){
			removeDomain(dynamic_cast<Domain *>(obj), idx);	}},
		{ ObjectType::Sequence, [this](BaseObject *obj, int idx){
				removeSequence(dynamic_cast<Sequence *>(obj), idx);	}},
		{ ObjectType::Collation, [this](BaseObject *obj, int idx){
				removeCollation(dynamic_cast<Collation *>(obj), idx);	}},
		{ ObjectType::Extension, [this](BaseObject *obj, int idx){
				removeExtension(dynamic_cast<Extension *>(obj), idx);	}},
		{ ObjectType::Tag, [this](BaseObject *obj, int idx){
				removeTag(dynamic_cast<Tag *>(obj), idx);	}},
		{ ObjectType::EventTrigger, [this](BaseObject *obj, int idx){
				removeEventTrigger(dynamic_cast<EventTrigger *>(obj), idx);	}},
		{ ObjectType::GenericSql,[this](BaseObject *obj, int idx){
				removeGenericSQL(dynamic_cast<GenericSQL *>(obj), idx);	}},
		{ ObjectType::ForeignDataWrapper, [this](BaseObject *obj, int idx){
				removeForeignDataWrapper(dynamic_cast<ForeignDataWrapper *>(obj), idx);	}},
		{ ObjectType::ForeignServer, [this](BaseObject *obj, int idx){
				removeForeignServer(dynamic_cast<ForeignServer *>(obj), idx);	}},
		{ ObjectType::UserMapping, [this](BaseObject *obj, int idx){
				removeUserMapping(dynamic_cast<UserMapping *>(obj), idx);	}},
		{ ObjectType::ForeignTable, [this](BaseObject *obj, int idx){
				removeForeignTable(dynamic_cast<ForeignTable *>(obj), idx);	}},
		{ ObjectType::Transform, [this](BaseObject *obj, int idx){
				removeTransform(dynamic_cast<Transform *>(obj), idx);	}},
		{ ObjectType::Procedure, [this](BaseObject *obj, int idx){
				removeProcedure(dynamic_cast<Procedure *>(obj), idx);	}},
		{ ObjectType::Permission, [this](BaseObject *obj, int){
				removePermission(dynamic_cast<Permission *>(obj)); }},
	};
}

DatabaseModel::DatabaseModel(ModelWidget *model_wgt):DatabaseModel()
{
	this->model_wgt=model_wgt;
}

DatabaseModel::~DatabaseModel()
{
	this->blockSignals(true);
	destroyObjects();
}

ModelWidget *DatabaseModel::getModelWidget()
{
	return model_wgt;
}

void DatabaseModel::setEncoding(EncodingType encod)
{
	encoding=encod;
}

void DatabaseModel::setLocalization(unsigned localiz_id, const QString &value)
{
	if(localiz_id > Collation::LcCollate)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	localizations[localiz_id]=value;
}

void DatabaseModel::setConnectionLimit(int conn_lim)
{
	if(conn_lim < -1) conn_lim=-1;
	this->conn_limit=conn_lim;
}

void DatabaseModel::setTemplateDB(const QString &temp_db)
{
	if(!temp_db.isEmpty() && !BaseObject::isValidName(temp_db))
		throw Exception(ErrorCode::AsgInvalidNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->template_db=temp_db;
}

void DatabaseModel::setAuthor(const QString &author)
{
	this->author=author;
}

std::vector<BaseObject *> *DatabaseModel::getObjectList(ObjectType obj_type)
{
	if(obj_lists.count(obj_type) == 0)
		return nullptr;

	return obj_lists[obj_type];
}

void DatabaseModel::addObject(BaseObject *object, int obj_idx)
{
	if(!object || add_methods.count(object->getObjectType()) == 0)
		return;

	try
	{
		add_methods[object->getObjectType()](object, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeObject(BaseObject *object, int obj_idx)
{
	if(!object || remove_methods.count(object->getObjectType()) == 0)
		return;

	try
	{
		remove_methods[object->getObjectType()](object, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::__addObject(BaseObject *object, int obj_idx)
{
	int idx;
	ObjectType obj_type;
	std::vector<BaseObject *> *obj_list=nullptr;
	std::vector<BaseObject *>::iterator itr, itr_end;

	if(!object)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	obj_type=object->getObjectType();

	if(obj_type==ObjectType::Tablespace)
	{
		Tablespace *tabspc=nullptr, *aux_tabspc=nullptr;

		obj_list=getObjectList(obj_type);
		itr=obj_list->begin();
		itr_end=obj_list->end();
		tabspc=dynamic_cast<Tablespace *>(object);

		while(itr!=itr_end)
		{
			aux_tabspc=dynamic_cast<Tablespace *>(*itr);

			/* Raises an error if the object to be added is a tablespace and
			there is some other tablespace pointing to the same directory */
			if(tabspc->getDirectory()==aux_tabspc->getDirectory())
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::AsgTablespaceDuplicatedDirectory)
								.arg(tabspc->getName())
								.arg(aux_tabspc->getName()),
								ErrorCode::AsgTablespaceDuplicatedDirectory,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			itr++;
		}
	}

	/* Raises an error if there is an object with the same name.
	 * The first checking is for duplicated functions/procedures */
	if(((obj_type==ObjectType::Function ||
			 obj_type==ObjectType::Procedure) &&
			(getObject(object->getSignature(), ObjectType::Function, idx) ||
			 getObject(object->getSignature(), ObjectType::Procedure, idx))) ||

		 /* If the object is a child of BaseTable we check if there're other
			* tables with the same name */
		 ((obj_type==ObjectType::View ||
			 obj_type==ObjectType::Table ||
			 obj_type==ObjectType::ForeignTable) &&
			(getObject(object->getName(true), ObjectType::View, idx) ||
			 getObject(object->getName(true), ObjectType::Table, idx) ||
			 getObject(object->getName(true), ObjectType::ForeignTable, idx))) ||

			(obj_type==ObjectType::Extension &&	(getObject(object->getName(false), obj_type, idx))) ||

			(getObject(object->getSignature(), obj_type, idx)))
	{
		QString str_aux;

		str_aux=Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
				.arg(object->getName(obj_type != ObjectType::Extension))
				.arg(object->getTypeName())
				.arg(this->getName(true))
				.arg(this->getTypeName());

		throw Exception(str_aux,ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	try
	{
		if(obj_type==ObjectType::Textbox || obj_type==ObjectType::BaseRelationship)
			object->getSourceCode(SchemaParser::XmlCode);
		else
			object->getSourceCode(SchemaParser::SqlCode);

		object->updateDependencies();

		BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(object);

		if(graph_obj)
		{
		/* Validating the object's layers. If one or more layers are invalid then the object will be
		 * moved to the default layer (idx = 0) */
			for(auto &layer_id : graph_obj->getLayers())
			{
				if(layer_id >= layers.size())
				{
					graph_obj->removeFromLayer(layer_id);
					graph_obj->addToLayer(0);
				}
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}

	obj_list=getObjectList(object->getObjectType());

	if(obj_idx < 0 || obj_idx >= static_cast<int>(obj_list->size()))
		obj_list->push_back(object);
	else
	{
		if(obj_idx >=0 && idx < 0)
			idx=obj_idx;
		else if(obj_idx < 0 && idx < 0)
			idx=0;

		if(obj_list->size() > 0)
			obj_list->insert((obj_list->begin() + idx), object);
		else
			obj_list->push_back(object);
	}

	object->setDatabase(this);
	emit s_objectAdded(object);
	this->setInvalidated(true);
}

void DatabaseModel::__removeObject(BaseObject *object, int obj_idx, bool check_refs)
{
	if(!object)
		throw Exception(ErrorCode::RemNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
	
	std::vector<BaseObject *> *obj_list = nullptr;
	ObjectType obj_type;

	obj_type=object->getObjectType();
	obj_list=getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	std::vector<BaseObject *> refs;

	//Get the table references
	if(check_refs)
		refs = object->getReferences();

	//If there are objects referencing the table
	if(!refs.empty())
	{
		ErrorCode err_code;

		//Raises an error indicating the object that is referencing the table
		if(!dynamic_cast<TableObject *>(refs[0]))
		{
			err_code = ErrorCode::RemDirectReference;
			throw Exception(Exception::getErrorMessage(err_code)
											.arg(object->getName(true))
											.arg(object->getTypeName())
											.arg(refs[0]->getName(true))
											.arg(refs[0]->getTypeName()),
											err_code, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		BaseObject *ref_obj_parent=dynamic_cast<TableObject *>(refs[0])->getParentTable();

		err_code = ErrorCode::RemInderectReference;
		throw Exception(Exception::getErrorMessage(err_code)
										.arg(object->getName(true))
										.arg(object->getTypeName())
										.arg(refs[0]->getName(true))
										.arg(refs[0]->getTypeName())
										.arg(ref_obj_parent->getName(true))
										.arg(ref_obj_parent->getTypeName()),
										err_code, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(obj_idx < 0 || obj_idx >= static_cast<int>(obj_list->size()))
		getObject(object->getSignature(), obj_type, obj_idx);

	if(obj_idx >= 0)
	{
		if(Permission::acceptsPermission(obj_type))
			removePermissions(object);

		obj_list->erase(obj_list->begin() + obj_idx);
	}

	object->clearAllDepsRefs();
	object->setDatabase(nullptr);
	emit s_objectRemoved(object);
}

std::vector<BaseObject *> DatabaseModel::getObjects(ObjectType obj_type, BaseObject *schema)
{
	std::vector<BaseObject *> *obj_list=nullptr, sel_list;
	std::vector<BaseObject *>::iterator itr, itr_end;
	BaseRelationship *rel=nullptr;

	obj_list=getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=obj_list->begin();
	itr_end=obj_list->end();

	while(itr!=itr_end)
	{
		rel=dynamic_cast<BaseRelationship *>(*itr);

		if((!rel && (*itr)->getSchema()==schema) ||
				(rel && (rel->getTable(BaseRelationship::SrcTable)->getSchema()==schema ||
						 rel->getTable(BaseRelationship::DstTable)->getSchema()==schema)))
			sel_list.push_back(*itr);

		itr++;
	}

	return sel_list;
}

std::vector<BaseObject *> DatabaseModel::getObjects(BaseObject *schema)
{
	std::vector<BaseObject *> *obj_list=nullptr, sel_list;
	std::vector<ObjectType> types = BaseObject::getChildObjectTypes(ObjectType::Schema);

	for(auto &type : types)
	{
		obj_list = getObjectList(type);

		for(auto &obj : *obj_list)
		{
			if(obj->getSchema() == schema)
				sel_list.push_back(obj);
		}
	}

	return sel_list;
}

BaseObject *DatabaseModel::getObject(const QString &name, ObjectType obj_type, int &obj_idx)
{
	BaseObject *object = nullptr;
	std::vector<BaseObject *> *obj_list=nullptr;
	std::vector<BaseObject *>::iterator itr, itr_end;
	bool found = false;
	QString aux_name1;

	obj_list = getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);
	
	itr = obj_list->begin();
	itr_end = obj_list->end();
	obj_idx = -1;
	aux_name1 = QString(name).remove('"');

	while(itr!=itr_end && !found)
	{
		if((*itr)->getSignature().remove("\"") == aux_name1 ||
				(*itr)->getName(false) == aux_name1)
		{
			found = true;
			break;
		}

		itr++;
	}

	if(found)
	{
		object = (*itr);
		obj_idx = (itr - obj_list->begin());
	}

	return object;
}

BaseObject *DatabaseModel::getObject(const QString &name, const std::vector<ObjectType> &types)
{
	BaseObject *object = nullptr;

	for(auto &type : types)
	{
		object = getObject(name, type);
		if(object) break;
	}

	return object;
}

BaseObject *DatabaseModel::getObjectByOid(unsigned int oid, ObjectType obj_type)
{
	auto list = getObjectList(obj_type);

	if(!list)
		return nullptr;

	for(auto &obj : *list)
	{
		if(obj->getPgOid() == oid)
			return obj;
	}

	return nullptr;
}

BaseObject *DatabaseModel::getObject(unsigned obj_idx, ObjectType obj_type)
{
	std::vector<BaseObject *> *obj_list=nullptr;

	obj_list=getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(obj_idx >= obj_list->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex, PGM_FUNC, PGM_FILE, PGM_LINE);

	return obj_list->at(obj_idx);
}

unsigned DatabaseModel::getObjectCount(ObjectType obj_type)
{
	std::vector<BaseObject *> *obj_list = nullptr;

	obj_list = getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return obj_list->size();
}

unsigned DatabaseModel::getObjectsCount(const std::vector<ObjectType> &obj_types)
{
	try
	{
		unsigned count = 0;

		for(auto &obj_typ : obj_types)
			count += getObjectCount(obj_typ);

		return count;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

unsigned DatabaseModel::getMaxObjectCount()
{
	std::vector<ObjectType> types = getObjectTypes(false, {ObjectType::Database});
  unsigned count = 0, max = 0;

  for(auto &type : types)
  {
	 count = getObjectList(type)->size();
	 if(count > max) max = count;
  }

	return max;
}

unsigned DatabaseModel::getObjectCount()
{
	std::vector<ObjectType> types= getObjectTypes(false, {ObjectType::Database});
	unsigned count=0;

	for(auto &type : types)
		count+=getObjectList(type)->size();

	return count;
}

QString DatabaseModel::getLocalization(unsigned localiz_id)
{
	if(localiz_id > Collation::LcCollate)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return localizations[localiz_id];
}

int DatabaseModel::getConnectionLimit()
{
	return conn_limit;
}

QString DatabaseModel::getTemplateDB()
{
	return template_db;
}

EncodingType DatabaseModel::getEncoding()
{
	return encoding;
}

BaseObject *DatabaseModel::getDefaultObject(ObjectType obj_type)
{
	if(default_objs.count(obj_type)==0)
		throw Exception(ErrorCode::RefObjectInvalidType, PGM_FUNC,PGM_FILE,PGM_LINE);

	return default_objs[obj_type];
}

QString DatabaseModel::getAuthor()
{
	return author;
}

void DatabaseModel::setProtected(bool value)
{
	for(auto &itr : obj_lists)
	{
		for(auto &object :  *itr.second)
		{
			object->setProtected(value);
		}
	}

	BaseObject::setProtected(value);
}

void DatabaseModel::destroyObjects()
{
	ObjectType graph_types[]={ ObjectType::Schema, ObjectType::BaseRelationship, ObjectType::Relationship,
														 ObjectType::Table, ObjectType::ForeignTable, ObjectType::View };
	BaseObject *object=nullptr;
	std::map<unsigned, BaseObject *> objects;
	std::map<unsigned, BaseObject *>::reverse_iterator ritr, ritr_end;
	std::vector<ObjectType> rem_obj_types;
	BaseGraphicObject *graph_obj = nullptr;

	//Blocking signals of all graphical objects to avoid uneeded updates in the destruction
	this->blockSignals(true);

	BaseObject::setClearDepsInDtor(false);
	BaseGraphicObject::setUpdatesEnabled(false);

	for(unsigned i=0; i < 5; i++)
	{
		for(auto &object : *this->getObjectList(graph_types[i]))
		{
			graph_obj = dynamic_cast<BaseGraphicObject *>(object);

			if(graph_obj)
				dynamic_cast<BaseGraphicObject *>(object)->blockSignals(true);
		}
	}

	try
	{
		//Removing the special objects first
		storeSpecialObjectsXML();
		disconnectRelationships();
	}
	catch(Exception &e)
	{
		/* DEBUG: An exception at this point shouldn't never occur but if
		 * it is raised, we spit out the error to the stdout in order to try to
		 * find out the problem! */
		qInfo() << "** FAIL TO DESTROY ALL OBJECTS **" << Qt::endl;
		qInfo() << e.getExceptionsText().toStdString().c_str() << Qt::endl;
	}

	objects = getCreationOrder(SchemaParser::XmlCode, true);
	ritr = objects.rbegin();
	ritr_end = objects.rend();

	while(ritr != ritr_end)
	{
		object = ritr->second;
		ritr++;

		// We ignore the database itself, permission objects (destroyed separetely) and table children objects
		if(object->getObjectType() == ObjectType::Database ||
			 object->getObjectType() == ObjectType::Permission ||
			 TableObject::isTableObject(object->getObjectType()))
			continue;

		// Register the type of the object being removed so the respective list can be cleaned in the end
		rem_obj_types.push_back(object->getObjectType());

		/* If the object is graphical destroy using the __removeObject in order
		emit the signal to object scene to remove the graphical representation
		of the to-be-destroyed object */
		if(dynamic_cast<BaseGraphicObject *>(object))
		{
			__removeObject(object,-1,false);

			if(object->getObjectType()==ObjectType::Relationship)
				dynamic_cast<Relationship *>(object)->destroyObjects();
		}

		delete object;
	}

	PgSqlType::removeUserTypes(this);

	for(auto &perm : 	permissions)
		delete perm;

	permissions.clear();

	for(auto &inv_obj : invalid_special_objs)
		delete inv_obj;

	invalid_special_objs.clear();

	//Cleaning out the list of removed objects to avoid segfaults while calling this method again
	if(!rem_obj_types.empty())
	{
		std::sort(rem_obj_types.begin(), rem_obj_types.end());
		std::vector<ObjectType>::iterator end = std::unique(rem_obj_types.begin(), rem_obj_types.end());
		rem_obj_types.erase(end, rem_obj_types.end());

		for(auto type : rem_obj_types)
			getObjectList(type)->clear();
	}

	BaseGraphicObject::setUpdatesEnabled(true);
	BaseObject::setClearDepsInDtor(true);
}

void DatabaseModel::addTable(Table *table, int obj_idx)
{
	try
	{
		__addObject(table, obj_idx);

		//PgSqlType::addUserType(table->getName(true), table, this, UserTypeConfig::TableType);
		PgSqlType::addUserType(table->getName(true), table, UserTypeConfig::TableType);

		updateTableFKRelationships(table);

		dynamic_cast<Schema *>(table->getSchema())->setModified(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Table *DatabaseModel::getTable(unsigned obj_idx)
{
	return dynamic_cast<Table *>(getObject(obj_idx, ObjectType::Table));
}

Table *DatabaseModel::getTable(const QString &name)
{
	return dynamic_cast<Table *>(getObject(name, ObjectType::Table));
}

void DatabaseModel::removeTable(Table *table, int obj_idx)
{
	try
	{
		__removeObject(table, obj_idx);
		PgSqlType::removeUserType(table->getName(true), table);
		updateTableFKRelationships(table);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addSequence(Sequence *sequence, int obj_idx)
{
	try
	{
		__addObject(sequence, obj_idx);
		//PgSqlType::addUserType(sequence->getName(true), sequence, this, UserTypeConfig::SequenceType);
		PgSqlType::addUserType(sequence->getName(true), sequence, UserTypeConfig::SequenceType);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Sequence *DatabaseModel::getSequence(unsigned obj_idx)
{
	return dynamic_cast<Sequence *>(getObject(obj_idx, ObjectType::Sequence));
}

Sequence *DatabaseModel::getSequence(const QString &name)
{
	return dynamic_cast<Sequence *>(getObject(name, ObjectType::Sequence));
}

void DatabaseModel::removeSequence(Sequence *sequence, int obj_idx)
{
	try
	{
		removeUserType(sequence, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addCollation(Collation *collation, int obj_idx)
{
	try
	{
		__addObject(collation, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Collation *DatabaseModel::getCollation(unsigned obj_idx)
{
	return dynamic_cast<Collation *>(getObject(obj_idx, ObjectType::Collation));
}

Collation *DatabaseModel::getCollation(const QString &name)
{
	return dynamic_cast<Collation *>(getObject(name, ObjectType::Collation));
}

void DatabaseModel::removeCollation(Collation *collation, int obj_idx)
{
	try
	{
		__removeObject(collation, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::addExtension(Extension *extension, int obj_idx)
{
	try
	{
		__addObject(extension, obj_idx);
		updateExtensionObjects(extension);
	}
	catch(Exception &e)
	{
		removeExtensionObjects(extension);
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeExtensionObjects(Extension *ext)
{
	if(!ext)
		throw Exception(ErrorCode::AsgNotAllocattedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	/* Checking if the user tries to remove an extension that
	 * has one or more children being referenced in the model */
	for(auto &obj : ext->getReferences())
	{
		/* We iterate over the references of each extension referenced object (child)
		 * to check if there are user-defined objects that is somehow linked to them */
		for(auto &ref_obj : obj->getReferences())
		{
			/* If the referenced object is not a child object of the
			 * extension being removed we raise an error indicating
			 * that there is an object created by the user which
			 * references the extension child object */
			if(!ref_obj->isDependingOn(ext))
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RemExtRefChildObject)
												.arg(ext->getSignature(), obj->getName(), obj->getTypeName(),
														 ref_obj->getSignature(), ref_obj->getTypeName()),
												 ErrorCode::RemExtRefChildObject, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}

		__removeObject(obj, -1, false);
	}
}

void DatabaseModel::setRelTablesModified(BaseRelationship *rel)
{
	if(!rel)
		return;

	BaseTable *src_table = rel->getTable(BaseRelationship::SrcTable),
			*dst_table = rel->getTable(BaseRelationship::DstTable);

	src_table->setModified(true);

	if(!rel->isSelfRelationship())
		dst_table->setModified(true);

	BaseObject *src_sch = src_table->getSchema(),
			*dst_sch = dst_table->getSchema();

	if(src_sch)
		dynamic_cast<BaseGraphicObject *>(src_sch)->setModified(true);

	if(dst_sch && dst_sch != src_sch)
		dynamic_cast<BaseGraphicObject *>(dst_sch)->setModified(true);
}

bool DatabaseModel::updateExtensionObjects(Extension *ext)
{
	if(!ext)
		throw Exception(ErrorCode::AsgNotAllocattedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	std::vector<BaseObject *> new_objs;

	try
	{
		/* Before inserting the extension, if it has child objects, we
		 * need to check if there are object with the same name in the model */
		BaseObject *obj = nullptr, *obj_sch = nullptr;
		QString obj_signature;
		bool objs_removed = true;

		for(auto ext_obj_type : { ObjectType::Schema, ObjectType::Type })
		{
			for(auto &ext_obj : ext->getObjects(ext_obj_type))
			{
				/* Checking if the extension child object has a conflicting name with another object
				 * of the same type in the model. For that, we get the first object that
				 * contains the extension's child object signature */
				if(ext_obj.getParent().isEmpty() && BaseObject::isChildObjectType(ObjectType::Schema, ext_obj_type))
					obj_signature = ext->getSchema()->getSignature() + "." + BaseObject::formatName(ext_obj.getName());
				else
					obj_signature = ext_obj.getSignature();

				obj = getObject(obj_signature, ext_obj_type);

				if(obj)
				{
					/* If the object exists and is not one that is linked to the current extension it means
					 * that if we try to create the object it'll be duplicated in the model, which is prohibited.
					 * The exception here is for duplicated schemas. The schema that is a child of the extension
					 * will not be created and the one in the model will be used by the other child objects of the
					 * extesion if needed */
					if(ext_obj_type != ObjectType::Schema && !obj->isDependingOn(ext))
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::AddExtDupChildObject)
														.arg(ext->getSignature(), obj->getSignature(), obj->getTypeName()),
														ErrorCode::AddExtDupChildObject,PGM_FUNC,PGM_FILE,PGM_LINE);
					}

					// If the retrieved type is one of the extension's child types we just skip it
					continue;
				}

				if(ext_obj_type == ObjectType::Schema)
				{
					obj = new Schema;
					dynamic_cast<Schema *>(obj)->setRectVisible(true);
				}
				else
				{
					obj = new Type;
					obj_sch = getSchema(ext_obj.getParent());
					obj->setSchema(!obj_sch ? ext->getSchema() : obj_sch);
					dynamic_cast<Type *>(obj)->setConfiguration(Type::EnumerationType);
				}

				/* Creating a new extension child object. Here, we force it to be a
				 * protected object that can't be handled by the user directly, and
				 * tie it to the extension itself, so when the extension is destroyed
				 * the child objects are destroyed as well */
				obj->setName(ext_obj.getName());
				obj->setSystemObject(true);
				obj->setSQLDisabled(true);
				obj->setDependency(ext);

				addObject(obj);

				/* Store the object in a temporary list so, in case of error,
				 * it will be removed in the catch section */
				new_objs.push_back(obj);
			}
		}

		/* In this step we remove potentially unused child object of the extesion that
		 * is in the model and add other ones that are still being referenced. This
		 * situation occurs when the extension is edited by the user by removing/adding
		 * other objects leaving behind the other ones previously created.*/
		for(auto &ref_obj : ext->getReferences())
		{
			Extension::ExtObject ext_obj { ref_obj->getName(),
																		 ref_obj->getObjectType(),
																		 ref_obj->getSchema() ? ref_obj->getSchema()->getName() : "" };

			if(!ext->containsObject(ext_obj))
			{
				// If the object is not being reference we can safely remove it
				if(!ref_obj->isReferenced())
					removeObject(ref_obj);
				else
				{
					// If it is still referenced we add it to the extension again
					ext->addObject(ext_obj);
					objs_removed = false;
				}
			}
		}

		return objs_removed;
	}
	catch(Exception &e)
	{
		for(auto itr = new_objs.rbegin();
						 itr != new_objs.rend(); itr++)
		{
			removeObject(*itr);
			delete *itr;
		}

		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

Extension *DatabaseModel::getExtension(unsigned obj_idx)
{
	return dynamic_cast<Extension *>(getObject(obj_idx, ObjectType::Collation));
}

Extension *DatabaseModel::getExtension(const QString &name)
{
	return dynamic_cast<Extension *>(getObject(name, ObjectType::Extension));
}

void DatabaseModel::addTag(Tag *tag, int obj_idx)
{
	try
	{
		__addObject(tag, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeTag(Tag *tag, int obj_idx)
{
	try
	{
		__removeObject(tag, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Tag *DatabaseModel::getTag(unsigned obj_idx)
{
	return dynamic_cast<Tag *>(getObject(obj_idx, ObjectType::Tag));
}

Tag *DatabaseModel::getTag(const QString &name)
{
	return dynamic_cast<Tag *>(getObject(name, ObjectType::Tag));
}

void DatabaseModel::addEventTrigger(EventTrigger *evnttrig, int obj_idx)
{
	try
	{
		__addObject(evnttrig, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeEventTrigger(EventTrigger *evnttrig, int obj_idx)
{
	try
	{
		__removeObject(evnttrig, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

EventTrigger *DatabaseModel::getEventTrigger(unsigned obj_idx)
{
	return dynamic_cast<EventTrigger *>(getObject(obj_idx, ObjectType::EventTrigger));
}

EventTrigger *DatabaseModel::getEventTrigger(const QString &name)
{
	return dynamic_cast<EventTrigger *>(getObject(name, ObjectType::EventTrigger));
}

void DatabaseModel::addGenericSQL(GenericSQL *genericsql, int obj_idx)
{
	try
	{
		__addObject(genericsql, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeGenericSQL(GenericSQL *genericsql, int obj_idx)
{
	try
	{
		__removeObject(genericsql, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

GenericSQL *DatabaseModel::getGenericSQL(unsigned obj_idx)
{
	return dynamic_cast<GenericSQL *>(getObject(obj_idx, ObjectType::GenericSql));
}

GenericSQL *DatabaseModel::getGenericSQL(const QString &name)
{
	return dynamic_cast<GenericSQL *>(getObject(name, ObjectType::GenericSql));
}

void DatabaseModel::addForeignDataWrapper(ForeignDataWrapper *fdata_wrapper, int obj_idx)
{
	try
	{
		__addObject(fdata_wrapper, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeForeignDataWrapper(ForeignDataWrapper *fdata_wrapper, int obj_idx)
{
	try
	{
		__removeObject(fdata_wrapper, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

ForeignDataWrapper *DatabaseModel::getForeignDataWrapper(unsigned obj_idx)
{
	return dynamic_cast<ForeignDataWrapper *>(getObject(obj_idx, ObjectType::ForeignDataWrapper));
}

ForeignDataWrapper *DatabaseModel::getForeignDataWrapper(const QString &name)
{
	return dynamic_cast<ForeignDataWrapper *>(getObject(name, ObjectType::ForeignDataWrapper));
}

void DatabaseModel::addForeignServer(ForeignServer *server, int obj_idx)
{
	try
	{
		__addObject(server, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeForeignServer(ForeignServer *server, int obj_idx)
{
	try
	{
		__removeObject(server, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

ForeignServer *DatabaseModel::getForeignServer(unsigned obj_idx)
{
	return dynamic_cast<ForeignServer *>(getObject(obj_idx, ObjectType::ForeignServer));
}

ForeignServer *DatabaseModel::getForeignServer(const QString &name)
{
	return dynamic_cast<ForeignServer *>(getObject(name, ObjectType::ForeignServer));
}

void DatabaseModel::addUserMapping(UserMapping *usrmap, int obj_idx)
{
	try
	{
		__addObject(usrmap, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeUserMapping(UserMapping *usrmap, int obj_idx)
{
	try
	{
		__removeObject(usrmap, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

UserMapping *DatabaseModel::getUserMapping(unsigned obj_idx)
{
	return dynamic_cast<UserMapping *>(getObject(obj_idx, ObjectType::UserMapping));
}

UserMapping *DatabaseModel::getUserMapping(const QString &name)
{
	return dynamic_cast<UserMapping *>(getObject(name, ObjectType::UserMapping));
}

void DatabaseModel::addForeignTable(ForeignTable *table, int obj_idx)
{
	try
	{
		__addObject(table, obj_idx);

//		PgSqlType::addUserType(table->getName(true), table, this, UserTypeConfig::ForeignTableType);
		PgSqlType::addUserType(table->getName(true), table, UserTypeConfig::ForeignTableType);
		dynamic_cast<Schema *>(table->getSchema())->setModified(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

ForeignTable *DatabaseModel::getForeignTable(unsigned obj_idx)
{
	return dynamic_cast<ForeignTable *>(getObject(obj_idx, ObjectType::ForeignTable));
}

ForeignTable *DatabaseModel::getForeignTable(const QString &name)
{
	return dynamic_cast<ForeignTable *>(getObject(name, ObjectType::ForeignTable));
}

void DatabaseModel::addTransform(Transform *transf, int obj_idx)
{
	try
	{
		__addObject(transf, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeTransform(Transform *transf, int obj_idx)
{
	try
	{
		__removeObject(transf, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Transform *DatabaseModel::getTransform(unsigned obj_idx)
{
	return dynamic_cast<Transform *>(getObject(obj_idx, ObjectType::Transform));
}

Transform *DatabaseModel::getTransform(const QString &name)
{
	return dynamic_cast<Transform *>(getObject(name, ObjectType::Transform));
}

void DatabaseModel::addProcedure(Procedure *proc, int obj_idx)
{
	try
	{
		__addObject(proc, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeProcedure(Procedure *proc, int obj_idx)
{
	try
	{
		__removeObject(proc, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Procedure *DatabaseModel::getProcedure(unsigned obj_idx)
{
	return dynamic_cast<Procedure *>(getObject(obj_idx, ObjectType::Procedure));
}

Procedure *DatabaseModel::getProcedure(const QString &name)
{
	return dynamic_cast<Procedure *>(getObject(name, ObjectType::Procedure));
}

void DatabaseModel::removeForeignTable(ForeignTable *table, int obj_idx)
{
	try
	{
		__removeObject(table, obj_idx);
		PgSqlType::removeUserType(table->getName(true), table);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removeExtension(Extension *extension, int obj_idx)
{
	if(!extension)
		throw Exception(ErrorCode::RemNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	try
	{
		removeExtensionObjects(extension);
		__removeObject(extension, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::addView(View *view, int obj_idx)
{
	try
	{
		__addObject(view, obj_idx);
		//PgSqlType::addUserType(view->getName(true), view, this, UserTypeConfig::ViewType);
		PgSqlType::addUserType(view->getName(true), view, UserTypeConfig::ViewType);

		updateViewRelationships(view);
		dynamic_cast<Schema *>(view->getSchema())->setModified(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

View *DatabaseModel::getView(unsigned obj_idx)
{
	return dynamic_cast<View *>(getObject(obj_idx, ObjectType::View));
}

View *DatabaseModel::getView(const QString &name)
{
	return dynamic_cast<View *>(getObject(name, ObjectType::View));
}

void DatabaseModel::removeView(View *view, int obj_idx)
{
	try
	{
		//The relationships that links tables to the view must be removed before erase the view itself
		updateViewRelationships(view, true);

		__removeObject(view, obj_idx);
		PgSqlType::removeUserType(view->getName(true), view);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::updateTableFKRelationships(Table *table)
{
	if(!table)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(table->getDatabase() != this)
		return;

	Table *ref_tab = nullptr;
	BaseRelationship *rel = nullptr;
	unsigned idx = 0;
	std::vector<Constraint *> fks;
	std::vector<BaseObject *>::iterator itr, itr_end;

	table->getForeignKeys(fks);

	/* First remove the invalid relationships (the foreign key that generates the
		relationship no longer exists) */
	itr = base_relationships.begin();
	itr_end = base_relationships.end();

	idx = 0;
	while(itr != itr_end)
	{
		rel = dynamic_cast<BaseRelationship *>(*itr);

		if(rel->getRelationshipType() == BaseRelationship::RelationshipFk &&
				(rel->getTable(BaseRelationship::SrcTable) == table))
		{
			Constraint *fk = rel->getReferenceForeignKey();
			ref_tab = dynamic_cast<Table *>(rel->getTable(BaseRelationship::DstTable));

			/* Removes the relationship if the following cases happen:
			 * 1) The foreign key references a table different from ref_tab, which means, the user
			 *		have changed the fk manually by setting a new referenced table but the relationship tied to the fk
			 *		does not reflect the new reference.
			 *
			 * 2) The fk references the correct table but the source table does not own the fk anymore, which means,
			 *		the fk as removed manually by the user. */
			if((table->getObjectIndex(fk) >= 0 && fk->getReferencedTable() != ref_tab) ||
				 (table->getObjectIndex(fk) < 0 && fk->getReferencedTable() == ref_tab))
			{
				removeRelationship(rel);
				itr = base_relationships.begin() + idx;
				itr_end = base_relationships.end();
			}
			else
			{
				rel->setModified(!loading_model);
				itr++; idx++;
			}
		}
		else
		{
			itr++; idx++;
		}
	}

	//Creating the relationships from the foreign keys
	for(auto &fk : fks)
	{
		ref_tab = dynamic_cast<Table *>(fk->getReferencedTable());

		//Only creates the relationship if doesn't exist one between the tables
		rel = getRelationship(table, ref_tab, fk);

		if(!rel && ref_tab->getDatabase()==this)
		{
			bool ref_mandatory = false;

			for(auto &col : fk->getColumns(Constraint::SourceCols))
			{
				if(col->isNotNull())
				{
					ref_mandatory = true;
					break;
				}
			}

			rel = new BaseRelationship(BaseRelationship::RelationshipFk, table, ref_tab, false, ref_mandatory);
			rel->setReferenceForeignKey(fk);
			rel->setCustomColor(Qt::transparent);

			/* Workaround: In some cases the combination of the two tablenames can generate a duplicated relationship
				 name so it`s necessary to check if a relationship with the same name already exists. If exists changes
				 the name of the new one */
			if(getObjectIndex(rel->getName(), ObjectType::BaseRelationship) >= 0)
				rel->setName(CoreUtilsNs::generateUniqueName(rel, base_relationships));

			addRelationship(rel);
		}
	}
}

void DatabaseModel::updateTablesFKRelationships()
{
	std::vector<BaseObject *>::iterator itr=tables.begin();

	while(itr!=tables.end())
	{
		updateTableFKRelationships(dynamic_cast<Table *>(*itr));
		itr++;
	}
}

void DatabaseModel::restoreFKRelationshipLayers()
{
	BaseRelationship *rel = nullptr;

	for(auto &obj : base_relationships)
	{
		rel = dynamic_cast<BaseRelationship *>(obj);

		if(rel->getRelationshipType() != BaseRelationship::RelationshipFk ||
			 !fk_rel_layers.count(rel->getName()))
			continue;

		rel->setLayers(fk_rel_layers.at(rel->getName()));
	}

	fk_rel_layers.clear();
}

void DatabaseModel::updateViewRelationships(View *view, bool force_rel_removal)
{
	PhysicalTable *table = nullptr;
	BaseRelationship *rel = nullptr;
	unsigned idx = 0;
	std::vector<BaseObject *>::iterator itr, itr_end;

	if(!view)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(getObjectIndex(view) < 0 || force_rel_removal)
	{
		//Remove all the relationship related to the view when this latter no longer exists
		itr = base_relationships.begin();
		itr_end = base_relationships.end();

		idx = 0;
		while(itr != itr_end)
		{
			rel = dynamic_cast<BaseRelationship *>(*itr);

			if(rel->getTable(BaseRelationship::SrcTable)==view ||
				 rel->getTable(BaseRelationship::DstTable)==view)
			{
				removeRelationship(rel);
				itr=base_relationships.begin() + idx;
				itr_end=base_relationships.end();
			}
			else
			{
				itr++; idx++;
			}
		}
	}
	else
	{
		/* Remove the relationships between tables and the view
		 when this latter doesn't reference the first */
		itr = base_relationships.begin();
		itr_end = base_relationships.end();

		idx = 0;
		while(itr != itr_end)
		{
			rel = dynamic_cast<BaseRelationship *>(*itr);

			if(rel->getTable(BaseRelationship::SrcTable)==view ||
					rel->getTable(BaseRelationship::DstTable)==view)
			{
				if(rel->getTable(BaseRelationship::SrcTable)->getObjectType()==ObjectType::Table)
					table=dynamic_cast<PhysicalTable *>(rel->getTable(BaseRelationship::SrcTable));
				else
					table=dynamic_cast<PhysicalTable *>(rel->getTable(BaseRelationship::DstTable));

				if(!view->isReferencingTable(table))
				{
					removeRelationship(rel);
					itr = base_relationships.begin() + idx;
					itr_end =base_relationships.end();
				}
				else
				{
					itr++; idx++;
				}
			}
			else
			{
				itr++; idx++;
			}
		}

		// Effectively creating the relationships
		for(auto &tab : view->getReferencedTables())
		{
			rel = getRelationship(view, tab);

			if(!rel)
			{
				rel=new BaseRelationship(BaseRelationship::RelationshipDep, view, tab, false, false);
				rel->setName(CoreUtilsNs::generateUniqueName(rel, base_relationships));
				addRelationship(rel);
			}
		}
	}
}

void DatabaseModel::disconnectRelationships()
{
	try
	{
		BaseRelationship *base_rel=nullptr;
		Relationship *rel=nullptr;
		std::vector<BaseObject *>::reverse_iterator ritr_rel, ritr_rel_end;

		//The relationships must be disconnected from the last to the first
		ritr_rel=relationships.rbegin();
		ritr_rel_end=relationships.rend();

		while(ritr_rel!=ritr_rel_end)
		{
			base_rel=dynamic_cast<BaseRelationship *>(*ritr_rel);
			ritr_rel++;
			base_rel->blockSignals(loading_model);

			if(base_rel->getObjectType()==ObjectType::Relationship)
			{
				rel=dynamic_cast<Relationship *>(base_rel);
				rel->disconnectRelationship();
			}
			else
				base_rel->disconnectRelationship();

			base_rel->blockSignals(false);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

bool DatabaseModel::hasInvalidRelatioships()
{
	Relationship *rel = nullptr;

	for(auto &itr : relationships)
	{
		rel = dynamic_cast<Relationship *>(itr);

		if(!rel) continue;

		if(rel && rel->isInvalidated())
			return true;
	}

	return false;
}

std::vector<Exception> DatabaseModel::createSpecialObjects()
{
	//The special objects are created only when the model is not being loaded
	if(loading_model)
		return {};

	std::vector<unsigned> created_objs_ids;
	std::vector<Exception> errors;

	for(auto &itr : xml_special_objs)
	{
		try
		{
			//Try to create the special object
			createSpecialObject(itr.second, itr.first);
			created_objs_ids.push_back(itr.first);
		}
		catch(Exception &e)
		{
			errors.push_back(e);
		}
	}

	while(!created_objs_ids.empty())
	{
		xml_special_objs.erase(created_objs_ids.back());
		created_objs_ids.pop_back();
	}

	return errors;
}

void DatabaseModel::updateRelsGeneratedObjects()
{
	try
	{
		Relationship *rel = nullptr;
		bool rels_updated = false;
		auto itr = relationships.begin(), itr_end = relationships.end();

		while(itr != itr_end)
		{
			rel = dynamic_cast<Relationship *>(*itr);
			itr++;

			rel->blockSignals(true);

			if(rel->updateGeneratedObjects())
				rels_updated = true;

			rel->blockSignals(false);

			if(itr == itr_end && rels_updated)
			{
				rels_updated = false;
				itr = relationships.begin();
			}
		}
	}
	catch (Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE);
	}
}

bool DatabaseModel::validateRelationships()
{
	Relationship *rel = nullptr;
	BaseRelationship *base_rel = nullptr;
	std::vector<Exception> errors;
	std::map<Relationship *, Exception> rel_errors;
	std::vector<Relationship *> failed_rels;

	if(!hasInvalidRelatioships())
		return false;

	if(!loading_model)
		BaseGraphicObject::setUpdatesEnabled(false);

	//Stores the special objects definition if there is some invalidated relationships
	if(!loading_model && xml_special_objs.empty())
		storeSpecialObjectsXML();

	// Disconnecting all relationships in order to force the correct propagation of columns/constraints
	disconnectRelationships();

	// Trying to connect all relatinships in the order they were created
	for(auto &rl : relationships)
	{
		try
		{
			rel = dynamic_cast<Relationship *>(rl);
			rel->blockSignals(true);
			rel->connectRelationship();
			rel->blockSignals(false);
		}
		catch(Exception &)
		{
			failed_rels.push_back(rel);
		}
	}

	// Trying to recreate relationships that failed to connect previously
	unsigned idx = 0, max_tries = failed_rels.size();

	while(idx < max_tries)
	{
		for(auto &rel : failed_rels)
		{
			try
			{
				rel->blockSignals(true);
				rel->connectRelationship();
				rel->blockSignals(false);
			}
			catch(Exception &e)
			{
				rel_errors[rel] = e;
			}
		}

		idx++;
	}

	/* Checking if some failed relationships have generated errors in the connection retry.
	 * If that's the case, we just remove the relationships from the model since
	 * they are permanentely invalidated and can't be reconnected */
	for(auto &re : rel_errors)
	{
		if(!re.first->isRelationshipConnected())
		{
			errors.push_back(re.second);
			__removeObject(re.first, -1, false);
		}
	}

	/* Updating the relationship generated objects.
	 * The columns and contraints not created in first connection are properly created */
	updateRelsGeneratedObjects();

	//Recreating the special objects that depends on the columns created by relationshps
	errors = createSpecialObjects();

	if(!loading_model)
	{
		for(auto &tab : tables)
			dynamic_cast<PhysicalTable *>(tab)->restoreRelObjectsIndexes();

		for(auto &tab : foreign_tables)
			dynamic_cast<PhysicalTable *>(tab)->restoreRelObjectsIndexes();

		xml_special_objs.clear();
	}

	//If errors were caught on the above executions they will be redirected to the user
	if(!errors.empty())
	{
		if(!loading_model)
			xml_special_objs.clear();

		/* Revalidates the fk relationships at this points because some fks must be removed due
		 * to special object invalidation */
		for(auto &itr : base_relationships)
		{
			base_rel = dynamic_cast<BaseRelationship *>(itr);

			if(base_rel->getRelationshipType() == BaseRelationship::RelationshipFk)
				this->updateTableFKRelationships(dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::SrcTable)));
		}
	}

	if(!loading_model)
	{
		//Set all the model objects as modified to force the redraw of the entire model
		BaseGraphicObject::setUpdatesEnabled(true);
		setObjectsModified();
	}

	if(!errors.empty())
		throw Exception(ErrorCode::RemInvalidatedObjects,PGM_FUNC,PGM_FILE,PGM_LINE, errors);

	return true;
}

void DatabaseModel::checkRelationshipRedundancy(Relationship *rel)
{
	try
	{
		//Raises an error if the user try to check the redundancy starting from a unnallocated relationship
		if(!rel)
			throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		BaseRelationship::RelType rel_type = rel->getRelationshipType();

		/* Only identifier relationships or relationship that has identifier
		 attributes (primary keys) are checked */
		if((!rel->isSelfRelationship() &&
				(rel->isIdentifier() ||
				 rel->hasIndentifierAttribute())) ||
				(rel_type==Relationship::RelationshipGen ||
				 rel_type==Relationship::RelationshipDep ||
				 rel_type==Relationship::RelationshipPart))
		{
			BaseTable *ref_table=nullptr, *src_table=nullptr;
			PhysicalTable *recv_table=nullptr;
			Relationship *rel_aux=nullptr;
			BaseRelationship *base_rel=nullptr;
			std::vector<BaseObject *>::iterator itr, itr_end;
			bool found_cycle=false;
			unsigned aux_rel_type;
			QString str_aux, msg;

			//Gets the tables from the relationship
			recv_table=rel->getReceiverTable();
			ref_table=rel->getReferenceTable();

			itr=relationships.begin();
			itr_end=relationships.end();

			/* Based on the obtained tables, scans the list of relationships in search of the cycle.
			A cycle is detected when a reference table from a relationship is the receiver table of
			the relationship used in the validation. */
			while(itr!=itr_end && !found_cycle)
			{
				base_rel=dynamic_cast<BaseRelationship *>(*itr);
				itr++;

				if(base_rel->getObjectType()==ObjectType::Relationship)
				{
					rel_aux=dynamic_cast<Relationship *>(base_rel);
					aux_rel_type=rel_aux->getRelationshipType();
					src_table=rel_aux->getReferenceTable();

					/* Case the reference table is equal to the receiver table of the relationship used
				as the start of validation and the current relationship type is the same as the
				latter relationship, this can indicate a principle of closing cycle, in this way
				the validation will proceed with the receiver table from the current relationship
				until que receiver table is the reference table of the initial relationship */
					if(recv_table==src_table &&  aux_rel_type==rel_type &&
							((!rel_aux->isSelfRelationship() &&
							  (rel_aux->isIdentifier() ||
							   rel_aux->hasIndentifierAttribute())) ||
							 (aux_rel_type==Relationship::RelationshipGen ||
								aux_rel_type==Relationship::RelationshipDep ||
								aux_rel_type==Relationship::RelationshipPart)))

					{
						//The receiver table will be the receiver from the current relationship
						recv_table=rel_aux->getReceiverTable();

						//Stores the relationship name to raise an error in case of closing cycle
						str_aux+=rel_aux->getName() + ", ";

						//Checking the closing cycle
						found_cycle=(recv_table==ref_table);

						//Restart the validation
						itr=relationships.begin();
					}
				}
			}

			//Raises an error indicating the relationship names that close the cycle
			if(found_cycle)
			{
				str_aux+=rel->getName();
				msg=Exception::getErrorMessage(ErrorCode::InsRelationshipRedundancy)
					.arg(rel->getName())
					.arg(str_aux);
				throw Exception(msg,ErrorCode::InsRelationshipRedundancy,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::storeSpecialObjectsXML()
{
	unsigned count = 0, i = 0, type_id = 0;
	std::vector<BaseObject *>::iterator itr, itr_end;
	Sequence *sequence = nullptr;
	Permission *permission = nullptr;
	PhysicalTable *table = nullptr;
	TableObject *tab_obj = nullptr;
	Constraint *constr = nullptr;
	Index *index = nullptr;
	Trigger *trigger = nullptr;
	View *view = nullptr;
	BaseRelationship *rel = nullptr;
	GenericSQL *generic_sql = nullptr;
	ObjectType tab_obj_type[] { ObjectType::Constraint, ObjectType::Trigger, ObjectType::Index };
	bool found=false;
	std::vector<BaseObject *> objects, rem_objects, upd_tables_rels, aux_tables;

	try
	{
		aux_tables = tables;
		aux_tables.insert(aux_tables.end(), foreign_tables.begin(), foreign_tables.end());

		/* Check on tables if there is some constraint/index/trigger that is referencing
		 some column added by relationship */
		for(auto &aux_tab : aux_tables)
		{
			table = dynamic_cast<PhysicalTable *>(aux_tab);

			for(auto &tp_id : tab_obj_type)
			{
				// Ignoring index type if we are dealing with foreign table
				if(table->getObjectType() == ObjectType::ForeignTable &&
					 tp_id == ObjectType::Index)
					continue;

				//Gets the table object count for the curret object type
				count = table->getObjectCount(tp_id);

				for(i = 0; i < count; i++)
				{
					tab_obj = dynamic_cast<TableObject *>(table->getObject(i, tp_id));
					//found = false;

					if(tp_id == ObjectType::Constraint)
					{
						constr = dynamic_cast<Constraint *>(tab_obj);

						/* A constraint is considered special in this case when it is referencing
							 relationship added column and the constraint itself was not added by
							 relationship (created manually by the user) */
						found = (!constr->isAddedByRelationship() &&
										 constr->isReferRelationshipAddedColumns() &&
										 constr->getConstraintType() != ConstraintType::PrimaryKey);

						//When found some special object, stores is xml definition
						if(found)
							xml_special_objs[constr->getObjectId()] = constr->getSourceCode(SchemaParser::XmlCode, true);
					}
					else if(tp_id == ObjectType::Trigger)
					{
						trigger = dynamic_cast<Trigger *>(tab_obj);
						found = trigger->isReferRelationshipAddedColumn();

						if(found)
							xml_special_objs[trigger->getObjectId()] = trigger->getSourceCode(SchemaParser::XmlCode);
					}
					else
					{
						index = dynamic_cast<Index *>(tab_obj);
						found = !index->isAddedByRelationship() &&
										index->isReferRelationshipAddedColumn();

						if(found)
							xml_special_objs[index->getObjectId()] = index->getSourceCode(SchemaParser::XmlCode);
					}

					if(found)
					{
						constr = dynamic_cast<Constraint *>(tab_obj);

						//When found the special object must be removed from the parent table
						table->removeObject(tab_obj->getName(), tab_obj->getObjectType());

						//We need to store the table which fk was referencing relationship added columns in order to update the fk relationships of that table
						if(constr && constr->getConstraintType() == ConstraintType::ForeignKey)
							upd_tables_rels.push_back(table);

						//Removes the permission from the table object
						removePermissions(tab_obj);

						i--; count--;
					}
				}
			}
		}

		//Updating fk rels of tables which had fks referencing relationship added columns
		if(!upd_tables_rels.empty())
		{
			std::vector<BaseObject *>::iterator end;

			if(upd_tables_rels.size() > 1)
			{
				std::sort(upd_tables_rels.begin(), upd_tables_rels.end());
				end = std::unique(upd_tables_rels.begin(), upd_tables_rels.end());
				upd_tables_rels.erase(end, upd_tables_rels.end());
			}

			for(auto &tab : upd_tables_rels)
				updateTableFKRelationships(dynamic_cast<Table *>(tab));
		}

		//Making a copy of the sequences list to avoid iterator invalidation when removing an object
		rem_objects.assign(sequences.begin(), sequences.end());

		for(auto &obj : rem_objects)
		{
			sequence = dynamic_cast<Sequence *>(obj);

			if(sequence->isReferRelationshipAddedColumn())
			{
				xml_special_objs[sequence->getObjectId()] = sequence->getSourceCode(SchemaParser::XmlCode);
				removeSequence(sequence);
				invalid_special_objs.push_back(sequence);
			}
		}

		//Making a copy of the view list to avoid iterator invalidation when removing an object
		rem_objects.assign(views.begin(), views.end());

		for(auto &obj : rem_objects)
		{
			view = dynamic_cast<View *>(obj);

			if(view->isReferRelationshipAddedColumn())
			{
				xml_special_objs[view->getObjectId()] = view->getSourceCode(SchemaParser::XmlCode);

				/* Relationships linking the view and the referenced tables are considered as
				 * special objects in this case only to be recreated more easely latter */
				for(auto &table : view->getReferencedTables())
				{
					//Get the relationship between the view and the referenced table
					rel = getRelationship(view, table);

					if(rel)
					{
						xml_special_objs[rel->getObjectId()] = rel->getSourceCode(SchemaParser::XmlCode);
						removeRelationship(rel);
						invalid_special_objs.push_back(rel);
					}
				}

				/* Removing child objects from view and including them in the list of objects to be recreated,
				 * this will avoid errors when removing the view from model */
				objects = view->getObjects();

				for(auto &obj : objects)
				{
					xml_special_objs[obj->getObjectId()] = obj->getSourceCode(SchemaParser::XmlCode);
					view->removeObject(obj);
					invalid_special_objs.push_back(obj);
				}

				removeView(view);
				invalid_special_objs.push_back(view);
			}
		}

		//Making a copy of the permissions list to avoid iterator invalidation when removing an object
		rem_objects.assign(permissions.begin(), permissions.end());

		for(auto &obj : rem_objects)
		{
			permission = dynamic_cast<Permission *>(obj);
			tab_obj = dynamic_cast<TableObject *>(permission->getObject());

			if(tab_obj)
			{
				xml_special_objs[permission->getObjectId()] = permission->getSourceCode(SchemaParser::XmlCode);
				removePermission(permission);
				invalid_special_objs.push_back(permission);
			}
		}

		//Making a copy of the generic SQL objects list to avoid iterator invalidation when removing an object
		rem_objects.assign(genericsqls.begin(), genericsqls.end());

		for(auto &obj : rem_objects)
		{
			generic_sql = dynamic_cast<GenericSQL *>(obj);

			if(generic_sql->isReferRelationshipAddedObject())
			{
				xml_special_objs[generic_sql->getObjectId()] = generic_sql->getSourceCode(SchemaParser::XmlCode);
				removeGenericSQL(generic_sql);
				invalid_special_objs.push_back(generic_sql);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::createSpecialObject(const QString &xml_def, unsigned obj_id)
{
	ObjectType obj_type;
	BaseObject *object=nullptr;

	try
	{
		//Restart the XML parser to read the passed xml buffer
		xmlparser.restartParser();
		xmlparser.loadXMLBuffer(xml_def);

		//Identifies the object type through the start element on xml buffer
		obj_type=BaseObject::getObjectType(xmlparser.getElementName());

		if(obj_type==ObjectType::Sequence)
			object=createSequence(true);
		else
			object=createObject(obj_type);

		if(obj_type==ObjectType::Sequence)
			addSequence(dynamic_cast<Sequence *>(object));
		else if(obj_type==ObjectType::View)
			addView(dynamic_cast<View *>(object));
		else if(obj_type==ObjectType::Permission)
			addPermission(createPermission());

		/* When the special object is recreated it receive a new id but to maintain
		 the correct creation order, the object has its id restored with the passed
		 id (obj_id) if it is specified */
		if(object && obj_id!=0)
			object->object_id=obj_id;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e, xml_def);
	}
}

void DatabaseModel::addRelationship(BaseRelationship *rel, int obj_idx)
{
	try
	{
		BaseTable *tab1=nullptr, *tab2=nullptr;
		QString msg;

		if(rel)
		{
			tab1=rel->getTable(BaseRelationship::SrcTable);
			tab2=rel->getTable(BaseRelationship::DstTable);

			//Raises an error if already exists an relationship between the tables
			if(rel->getRelationshipType() != Relationship::Relationship1n &&
				 rel->getRelationshipType() != Relationship::RelationshipNn &&
				 rel->getRelationshipType() != Relationship::RelationshipFk &&
				 getRelationship(tab1,tab2))
			{
				msg=Exception::getErrorMessage(ErrorCode::InsDuplicatedRelationship)
					.arg(tab1->getName(true))
					.arg(tab1->getTypeName())
					.arg(tab2->getName(true))
					.arg(tab2->getTypeName());
				throw Exception(msg,ErrorCode::InsDuplicatedRelationship,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		//Before add the relationship, checks if a redundancy can occur case the relationship is added
		if(rel->getObjectType()==ObjectType::Relationship)
			checkRelationshipRedundancy(dynamic_cast<Relationship *>(rel));

		rel->blockSignals(loading_model);
		__addObject(rel, obj_idx);

		if(rel->getObjectType()==ObjectType::Relationship)
		{
			dynamic_cast<Relationship *>(rel)->connectRelationship();

			/* If no relationship was validated means that the relationship we are adding
			 * is the first one in the model, so we force the modification of the involved
			 * tables so they can be properly updated graphically */
			if(!validateRelationships())
				setRelTablesModified(rel);
		}
		else
			rel->connectRelationship();

		rel->blockSignals(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeRelationship(BaseRelationship *rel, int obj_idx)
{
	try
	{
		if(getObjectIndex(rel) >= 0)
		{
			PhysicalTable *recv_tab=nullptr;

			if(rel->getObjectType()==ObjectType::Relationship)
			{
				/* If the relationship is not a many-to-many we store the receiver table in order to
				 * update the fk relationships (if there are any) */
				if(rel->getRelationshipType()!=Relationship::RelationshipNn)
					recv_tab=dynamic_cast<Relationship *>(rel)->getReceiverTable();

				storeSpecialObjectsXML();
				disconnectRelationships();
			}
			else if(rel->getObjectType()==ObjectType::BaseRelationship)
			{
				rel->blockSignals(loading_model);
				rel->disconnectRelationship();
				rel->blockSignals(false);
			}

			__removeObject(rel, obj_idx);

			/* If no relationship was validated means that the relationship we are removing
			 * was the only one in the model, so we force the modification of the involved
			 * tables so they can be properly updated graphically */
			if(rel->getObjectType() == ObjectType::Relationship &&
				 !validateRelationships())
				setRelTablesModified(rel);

			//Updating the fk relationships for the receiver table after removing the old relationship
			if(recv_tab && recv_tab->getObjectType() == ObjectType::Table)
				updateTableFKRelationships(dynamic_cast<Table *>(recv_tab));
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

BaseRelationship *DatabaseModel::getRelationship(unsigned obj_idx, ObjectType rel_type)
{
	//Raises an error if the object type used to get a relationship is invalid
	if(rel_type!=ObjectType::Relationship && rel_type!=ObjectType::BaseRelationship)
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return dynamic_cast<BaseRelationship *>(getObject(obj_idx, rel_type));
}

BaseRelationship *DatabaseModel::getRelationship(const QString &name)
{
	BaseRelationship *rel=dynamic_cast<BaseRelationship *>(getObject(name, ObjectType::BaseRelationship));

	if(!rel)
		rel=dynamic_cast<BaseRelationship *>(getObject(name, ObjectType::Relationship));

	return rel;
}

BaseRelationship *DatabaseModel::getRelationship(BaseTable *src_tab, BaseTable *dst_tab, Constraint *ref_fk)
{
	std::vector<BaseObject *>::iterator itr, itr_end;
	std::vector<BaseObject *> rel_list;
	BaseRelationship *rel=nullptr;
	bool found=false, search_uniq_tab=false;
	BaseTable *tab1=nullptr, *tab2=nullptr;

	if(src_tab)
	{
		if(!dst_tab)
		{
			dst_tab=src_tab;
			search_uniq_tab=true;
		}

		if(ref_fk || src_tab->getObjectType()==ObjectType::View || dst_tab->getObjectType()==ObjectType::View)
		{
			itr=base_relationships.begin();
			itr_end=base_relationships.end();
		}
		else
		{
			rel_list.assign(base_relationships.begin(), base_relationships.end());
			rel_list.insert(rel_list.end(), relationships.begin(), relationships.end());
			itr=rel_list.begin();
			itr_end=rel_list.end();
		}

		while(itr!=itr_end && !found)
		{
			rel=dynamic_cast<BaseRelationship *>(*itr);
			tab1=rel->getTable(BaseRelationship::SrcTable);
			tab2=rel->getTable(BaseRelationship::DstTable);

			found=((!ref_fk || (ref_fk && rel->getReferenceForeignKey() == ref_fk)) &&
						 ((tab1==src_tab && tab2==dst_tab) ||
							(tab2==src_tab && tab1==dst_tab) ||
							(search_uniq_tab && (tab1==src_tab || tab2==src_tab))));

			if(!found)
			{ rel=nullptr; itr++; }
		}
	}

	return rel;
}

std::vector<BaseRelationship *> DatabaseModel::getRelationships(BaseTable *tab)
{
	std::vector<BaseRelationship *> aux_rels;
	std::vector<BaseObject *> rels;
	BaseRelationship *base_rel=nullptr;

	rels=base_relationships;
	rels.insert(rels.end(), relationships.begin(), relationships.end());

	for(auto &obj : rels)
	{
		base_rel=dynamic_cast<BaseRelationship *>(obj);

		if(base_rel->getTable(BaseRelationship::SrcTable)==tab ||
			 base_rel->getTable(BaseRelationship::DstTable)==tab)
			aux_rels.push_back(base_rel);
	}

	return aux_rels;
}

void DatabaseModel::addTextbox(Textbox *txtbox, int obj_idx)
{
	try
	{
		__addObject(txtbox, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeTextbox(Textbox *txtbox, int obj_idx)
{
	try
	{
		__removeObject(txtbox, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Textbox *DatabaseModel::getTextbox(unsigned obj_idx)
{
	return dynamic_cast<Textbox *>(getObject(obj_idx, ObjectType::Textbox));
}

Textbox *DatabaseModel::getTextbox(const QString &name)
{
	return dynamic_cast<Textbox *>(getObject(name, ObjectType::Textbox));
}

void DatabaseModel::addSchema(Schema *schema, int obj_idx)
{
	try
	{
		if(schema && schema->isSystemObject() && schema->getName() != "public")
			schema->setRectVisible(show_sys_sch_rects);

		__addObject(schema, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Schema *DatabaseModel::getSchema(unsigned obj_idx)
{
	return dynamic_cast<Schema *>(getObject(obj_idx, ObjectType::Schema));
}

Schema *DatabaseModel::getSchema(const QString &name)
{
	return dynamic_cast<Schema *>(getObject(name, ObjectType::Schema));
}

void DatabaseModel::removeSchema(Schema *schema, int obj_idx)
{
	try
	{
		__removeObject(schema, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addRole(Role *role, int obj_idx)
{
	try
	{
		__addObject(role, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Role *DatabaseModel::getRole(unsigned obj_idx)
{
	return dynamic_cast<Role *>(getObject(obj_idx, ObjectType::Role));
}

Role *DatabaseModel::getRole(const QString &name)
{
	return dynamic_cast<Role *>(getObject(name, ObjectType::Role));
}

void DatabaseModel::removeRole(Role *role, int obj_idx)
{
	try
	{
		__removeObject(role, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addTablespace(Tablespace *tabspc, int obj_idx)
{
	try
	{
		__addObject(tabspc, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Tablespace *DatabaseModel::getTablespace(unsigned obj_idx)
{
	return dynamic_cast<Tablespace *>(getObject(obj_idx, ObjectType::Tablespace));
}

Tablespace *DatabaseModel::getTablespace(const QString &name)
{
	return dynamic_cast<Tablespace *>(getObject(name, ObjectType::Tablespace));
}

void DatabaseModel::removeTablespace(Tablespace *tabspc, int obj_idx)
{
	try
	{
		__removeObject(tabspc, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addCast(Cast *cast, int obj_idx)
{
	try
	{
		__addObject(cast, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeCast(Cast *cast, int obj_idx)
{
	try
	{
		__removeObject(cast, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Cast *DatabaseModel::getCast(unsigned obj_idx)
{
	return dynamic_cast<Cast *>(getObject(obj_idx, ObjectType::Cast));
}

Cast *DatabaseModel::getCast(const QString &name)
{
	return dynamic_cast<Cast *>(getObject(name, ObjectType::Cast));
}

void DatabaseModel::addConversion(Conversion *conv, int obj_idx)
{
	try
	{
		__addObject(conv, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeConversion(Conversion *conv, int obj_idx)
{
	try
	{
		__removeObject(conv, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Conversion *DatabaseModel::getConversion(unsigned obj_idx)
{
	return dynamic_cast<Conversion *>(getObject(obj_idx, ObjectType::Conversion));
}

Conversion *DatabaseModel::getConversion(const QString &name)
{
	return dynamic_cast<Conversion *>(getObject(name, ObjectType::Conversion));
}

void DatabaseModel::addLanguage(Language *lang, int obj_idx)
{
	try
	{
		__addObject(lang, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Language *DatabaseModel::getLanguage(unsigned obj_idx)
{
	return dynamic_cast<Language *>(getObject(obj_idx, ObjectType::Language));
}

Language *DatabaseModel::getLanguage(const QString &name)
{
	return dynamic_cast<Language *>(getObject(name, ObjectType::Language));
}

void DatabaseModel::removeLanguage(Language *lang, int obj_idx)
{
	try
	{
		__removeObject(lang, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addFunction(Function *func, int obj_idx)
{
	try
	{
		__addObject(func, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Function *DatabaseModel::getFunction(unsigned obj_idx)
{
	return dynamic_cast<Function *>(getObject(obj_idx, ObjectType::Function));
}

Function *DatabaseModel::getFunction(const QString &signature)
{
	return dynamic_cast<Function *>(getObject(signature, ObjectType::Function));
}

void DatabaseModel::removeFunction(Function *func, int obj_idx)
{
	try
	{
		__removeObject(func, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addAggregate(Aggregate *aggreg, int obj_idx)
{
	try
	{
		__addObject(aggreg, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Aggregate *DatabaseModel::getAggregate(unsigned obj_idx)
{
	return dynamic_cast<Aggregate *>(getObject(obj_idx, ObjectType::Aggregate));
}

Aggregate *DatabaseModel::getAggregate(const QString &name)
{
	return dynamic_cast<Aggregate *>(getObject(name, ObjectType::Aggregate));
}

void DatabaseModel::removeAggregate(Aggregate *aggreg, int obj_idx)
{
	try
	{
		__removeObject(aggreg, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addDomain(Domain *domain, int obj_idx)
{
	if(domain)
	{
		std::vector<BaseObject *>::iterator itr, itr_end;
		bool found=false;
		QString str_aux;

		/* Before insert the domain checks if there is some user defined type
		 with the same name as the domain. */
		itr=types.begin();
		itr_end=types.end();
		while(itr!=itr_end && !found)
		{
			found=((*itr)->getName(true)==domain->getName(true));
			itr++;
		}

		//Raises an error if found a type with the same name as the domain
		if(found)
		{
			str_aux=Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
					.arg(domain->getName(true))
					.arg(domain->getTypeName())
					.arg(this->getName(true))
					.arg(this->getTypeName());
			throw Exception(str_aux, ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		try
		{
			__addObject(domain, obj_idx);

			//When added to the model the domain is inserted on the pgsql base type list to be used as a column type
//			PgSqlType::addUserType(domain->getName(true), domain, this, UserTypeConfig::DomainType);
			PgSqlType::addUserType(domain->getName(true), domain, UserTypeConfig::DomainType);
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
		}
	}
}

void DatabaseModel::removeDomain(Domain *domain, int obj_idx)
{
	try
	{
		removeUserType(domain, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Domain *DatabaseModel::getDomain(unsigned obj_idx)
{
	return dynamic_cast<Domain *>(getObject(obj_idx, ObjectType::Domain));
}

Domain *DatabaseModel::getDomain(const QString &name)
{
	return dynamic_cast<Domain *>(getObject(name, ObjectType::Domain));
}

void DatabaseModel::addOperatorFamily(OperatorFamily *op_family, int obj_idx)
{
	try
	{
		__addObject(op_family, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

OperatorFamily *DatabaseModel::getOperatorFamily(unsigned obj_idx)
{
	return dynamic_cast<OperatorFamily *>(getObject(obj_idx, ObjectType::OpFamily));
}

OperatorFamily *DatabaseModel::getOperatorFamily(const QString &name)
{
	return dynamic_cast<OperatorFamily *>(getObject(name, ObjectType::OpFamily));
}

void DatabaseModel::removeOperatorFamily(OperatorFamily *op_family, int obj_idx)
{
	try
	{
		__removeObject(op_family, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addOperatorClass(OperatorClass *op_class, int obj_idx)
{
	try
	{
		__addObject(op_class, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeOperatorClass(OperatorClass *op_class, int obj_idx)
{
	try
	{
		__removeObject(op_class, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

OperatorClass *DatabaseModel::getOperatorClass(unsigned obj_idx)
{
	return dynamic_cast<OperatorClass *>(getObject(obj_idx, ObjectType::OpClass));
}

OperatorClass *DatabaseModel::getOperatorClass(const QString &name)
{
	return dynamic_cast<OperatorClass *>(getObject(name, ObjectType::OpClass));
}

void DatabaseModel::addOperator(Operator *oper, int obj_idx)
{
	try
	{
		__addObject(oper, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removeOperator(Operator *oper, int obj_idx)
{
	try
	{
		__removeObject(oper, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Operator *DatabaseModel::getOperator(unsigned obj_idx)
{
	return dynamic_cast<Operator *>(getObject(obj_idx, ObjectType::Operator));
}

Operator *DatabaseModel::getOperator(const QString &signature)
{
	return dynamic_cast<Operator *>(getObject(signature, ObjectType::Operator));
}

void DatabaseModel::addType(Type *type, int obj_idx)
{
	if(type)
	{
		std::vector<BaseObject *>::iterator itr, itr_end;
		bool found=false;
		QString str_aux;

		/* Before insert the type checks if there is some domain
		 with the same name as the type. */
		itr=domains.begin();
		itr_end=domains.end();
		while(itr!=itr_end && !found)
		{
			found=((*itr)->getName(true)==type->getName(true));
			itr++;
		}

		if(found)
		{
			str_aux=Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
					.arg(type->getName(true))
					.arg(type->getTypeName())
					.arg(this->getName(true))
					.arg(this->getTypeName());
			throw Exception(str_aux, ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		try
		{
			__addObject(type, obj_idx);

			//When added to the model the user type is inserted on the pgsql base type list to be used as a column type
			PgSqlType::addUserType(type->getName(true), type, UserTypeConfig::BaseType);
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
		}
	}
}

void DatabaseModel::removeType(Type *type, int obj_idx)
{
	try
	{
		removeUserType(type, obj_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Type *DatabaseModel::getType(unsigned obj_idx)
{
	return dynamic_cast<Type *>(getObject(obj_idx, ObjectType::Type));
}

Type *DatabaseModel::getType(const QString &name)
{
	return dynamic_cast<Type *>(getObject(name, ObjectType::Type));
}

void DatabaseModel::removeUserType(BaseObject *object, int obj_idx)
{
	try
	{
		__removeObject(object, obj_idx);

		//Removes the user type from the list of base types of pgsql
		PgSqlType::removeUserType(object->getName(true), object);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addPermissions(const std::vector<Permission *> &perms)
{
	std::vector<Permission *>::const_iterator itr=perms.cbegin(), itr_end=perms.cend();

	try
	{
		while(itr!=itr_end)
		{
			addPermission(*itr);
			itr++;
		}
	}
	catch(Exception &e)
	{
		//In case of errors removes the added permissions
		itr=perms.begin();
		itr_end=perms.end();

		while(itr!=itr_end)
		{
			removePermission(*itr);
			itr++;
		}

		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::addPermission(Permission *perm)
{
	try
	{
		if(!perm)
			throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		TableObject *tab_obj=dynamic_cast<TableObject *>(perm->getObject());

		if(getPermissionIndex(perm, false) >=0)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedPermission)
											.arg(perm->getObject()->getName())
											.arg(perm->getObject()->getTypeName()),
											ErrorCode::AsgDuplicatedPermission, PGM_FUNC, PGM_FILE, PGM_LINE);
		}
		
		//Raises an error if the permission is referencing an object that does not exists on model
		if(perm->getObject() != this &&
			((tab_obj && (getObjectIndex(tab_obj->getParentTable()) < 0)) ||
	 		(!tab_obj && (getObjectIndex(perm->getObject()) < 0))))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(perm->getName())
											.arg(perm->getObject()->getTypeName())
											.arg(perm->getObject()->getName())
											.arg(perm->getObject()->getTypeName()),
											ErrorCode::RefObjectInexistsModel, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		permissions.push_back(perm);
		perm->setDatabase(this);
		perm->updateDependencies();
	}
	catch(Exception &e)
	{
		if(e.getErrorCode()==ErrorCode::AsgDuplicatedObject)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedPermission)
									  	.arg(perm->getObject()->getName())
					  					.arg(perm->getObject()->getTypeName()),
											ErrorCode::AsgDuplicatedPermission, PGM_FUNC, PGM_FILE, PGM_LINE, &e);
		}

		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::removePermission(Permission *perm)
{
	try
	{
		__removeObject(perm);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::removePermissions(BaseObject *object)
{
	std::vector<BaseObject *>::iterator itr, itr_end;
	Permission *perm=nullptr;
	unsigned idx=0;

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=permissions.begin();
	itr_end=permissions.end();

	while(itr!=itr_end)
	{
		perm=dynamic_cast<Permission *>(*itr);

		if(perm->getObject()==object)
		{
			invalid_special_objs.push_back(perm);

			permissions.erase(itr);
			itr=itr_end=permissions.end();

			if(!permissions.empty())
				itr=permissions.begin() + idx;
		}
		else
		{ itr++; idx++; }
	}
}

void DatabaseModel::getPermissions(BaseObject *object, std::vector<Permission *> &perms)
{
	std::vector<BaseObject *>::iterator itr, itr_end;
	Permission *perm=nullptr;

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=permissions.begin();
	itr_end=permissions.end();

	perms.clear();

	while(itr!=itr_end)
	{
		perm=dynamic_cast<Permission *>(*itr);

		if(perm->getObject()==object)
			perms.push_back(perm);

		itr++;
	}
}

int DatabaseModel::getPermissionIndex(Permission *perm, bool exact_match)
{
	int perm_idx=-1;

	if(perm)
	{
		Permission *perm_aux=nullptr;
		std::vector<BaseObject *>::iterator itr, itr_end;

		itr=permissions.begin();
		itr_end=permissions.end();

		if(exact_match)
		{
			while(itr!=itr_end)
			{
				perm_aux=dynamic_cast<Permission *>(*itr);

				if(perm->isSimilarTo(perm_aux))
				{
					perm_idx=itr-permissions.begin();
					break;
				}

				itr++;
			}
		}
		else
		{
			BaseObject *object=nullptr;
			Role *role=nullptr;
			unsigned count, i;
			bool ref_role=false;

			object=perm->getObject();

			while(itr!=itr_end)
			{
				perm_aux=dynamic_cast<Permission *>(*itr);

				/* When the object of the auxiliary permission is the same as the
		   specified permission it will be check if the existant roles are
		   the same on both permissions */
				if(object==perm_aux->getObject())
				{
					count=perm->getRoleCount();

					for(i=0; i < count && !ref_role; i++)
					{
						role=perm->getRole(i);
						ref_role=perm_aux->isRoleExists(role);
					}
				}

				//If the permissions references the same roles but one is a REVOKE and other GRANT they a considered different
				if(perm==perm_aux || (ref_role && perm->isRevoke()==perm_aux->isRevoke()))
				{
					perm_idx=itr-permissions.begin();
					break;
				}

				itr++;
			}
		}
	}

	return perm_idx;
}

BaseObject *DatabaseModel::getObject(const QString &name, ObjectType obj_type)
{
	int idx;
	return getObject(name, obj_type, idx);
}

int DatabaseModel::getObjectIndex(const QString &name, ObjectType obj_type)
{
	int idx;
	getObject(name, obj_type, idx);
	return idx;
}

int DatabaseModel::getObjectIndex(BaseObject *object)
{
	if(!object)
		return -1;

	ObjectType obj_type=object->getObjectType();
	std::vector<BaseObject *> *obj_list=nullptr;
	std::vector<BaseObject *>::iterator itr, itr_end;
	bool found=false;

	obj_list=getObjectList(obj_type);

	if(!obj_list)
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
	
	itr = obj_list->begin();
	itr_end = obj_list->end();

	while(itr != itr_end && !found)
	{
		found=((*itr)==object);
		if(!found) itr++;
	}

	if(found)
		return itr - obj_list->begin();
		
	return -1;
}

void DatabaseModel::configureDatabase(attribs_map &attribs)
{
	encoding=attribs[Attributes::Encoding];
	template_db=attribs[Attributes::TemplateDb];
	localizations[0]=attribs[Attributes::LcCtype];
	localizations[1]=attribs[Attributes::LcCollate];
	append_at_eod=attribs[Attributes::AppendAtEod]==Attributes::True;
	prepend_at_bod=attribs[Attributes::PrependAtBod]==Attributes::True;
	is_template=attribs[Attributes::IsTemplate]==Attributes::True;
	allow_conns=attribs[Attributes::AllowConns] != Attributes::False;

	if(!attribs[Attributes::ConnLimit].isEmpty())
		conn_limit=attribs[Attributes::ConnLimit].toInt();

	setBasicAttributes(this);
}

void DatabaseModel::loadModel(const QString &filename)
{
	if(filename.isEmpty())
		return;

	BaseGraphicObject::setUpdatesEnabled(false);

	QString dtd_file, str_aux, elem_name;
	ObjectType obj_type;
	attribs_map attribs;
	BaseObject *object=nullptr;
	bool protected_model=false;
	QStringList pos_str;
	std::map<ObjectType, QString> def_objs;

	//Configuring the path to the base path for objects DTD
	dtd_file=GlobalAttributes::getSchemasRootPath() +
						 GlobalAttributes::DirSeparator +
						 GlobalAttributes::XMLSchemaDir +
						 GlobalAttributes::DirSeparator +
						 GlobalAttributes::ObjectDTDDir +
						 GlobalAttributes::DirSeparator;

	try
	{
		loading_model=true;
		xmlparser.restartParser();

		//Loads the root DTD
		xmlparser.setDTDFile(dtd_file + GlobalAttributes::RootDTD +
														 GlobalAttributes::ObjectDTDExt,
												 GlobalAttributes::RootDTD);

		//Loads the file validating it against the root DTD
		xmlparser.loadXMLFile(filename);

		//Gets the basic model information
		xmlparser.getElementAttributes(attribs);

		setObjectListsCapacity(attribs[Attributes::MaxObjCount].toUInt());

		this->author = attribs[Attributes::Author];

		pos_str = attribs[Attributes::LastPosition].split(',');

		if(pos_str.size() >= 2)
			this->last_pos = QPoint(pos_str[0].toInt(),pos_str[1].toInt());

		this->last_zoom = attribs[Attributes::LastZoom].toDouble();

		if(this->last_zoom <= 0)
			this->last_zoom=1;

		pos_str = attribs[Attributes::SceneRect].split(',');

		if(pos_str.size() == 4)
		{
			scene_rect.setLeft(pos_str[0].toDouble());
			scene_rect.setTop(pos_str[1].toDouble());
			scene_rect.setWidth(pos_str[2].toDouble());
			scene_rect.setHeight(pos_str[3].toDouble());
		}

		this->is_template = attribs[Attributes::IsTemplate] == Attributes::True;
		this->allow_conns = (attribs[Attributes::AllowConns].isEmpty() ||
												 attribs[Attributes::AllowConns] == Attributes::True);

		persist_changelog = attribs[Attributes::UseChangelog] == Attributes::True;
		gen_dis_objs_code = attribs[Attributes::GenDisabledObjsCode] == Attributes::True;
		show_sys_sch_rects = attribs[Attributes::ShowSysSchemasRects] == Attributes::True;

		/* Compatibility with models created prior the multiple layers features:
		 * We need to replace semi-colon by comma in the attribute Layers in order to split the
		 * string correctly, otherwise, the model will have only one layer no matter the amount of
		 * layers created preivously (in an older version) */
		layers = attribs[Attributes::Layers].replace(';',',').split(',', Qt::SkipEmptyParts);
		attribs[Attributes::ActiveLayers].replace(';',',');

		layer_name_colors = attribs[Attributes::LayerNameColors].split(',', Qt::SkipEmptyParts);
		layer_rect_colors = attribs[Attributes::LayerRectColors].split(',', Qt::SkipEmptyParts);

		is_layer_names_visible = attribs[Attributes::ShowLayerNames] == Attributes::True;
		is_layer_rects_visible = attribs[Attributes::ShowLayerRects] == Attributes::True;

		/*  Compatibility with models created prior the layers features:
		 * If the layer rect colors is empty (probably a model generated in an older version)
		 * we create random colors as fallback */

		// Forcing the creation of the default layer is not present
		if(layers.isEmpty())
			layers.push_back(tr("Default layer"));

		if(layer_rect_colors.size() != layers.size())
		{
			std::random_device rand_seed;
			std::default_random_engine rand_num_engine;
			std::uniform_int_distribution<unsigned> dist(0,255);

			layer_name_colors.clear();
			layer_rect_colors.clear();
			rand_num_engine.seed(rand_seed());

			for(int i = 0; i < layers.size(); i++)
			{
				layer_rect_colors.append(QColor(dist(rand_num_engine),
																				dist(rand_num_engine),
																				dist(rand_num_engine)).name());

				layer_name_colors.append(QColor(0,0,0).name());
			}
		}

		/* Compatibility with models created prior the layers features:
		 * If the "active-layers" is absent we make the default layer always visible */
		active_layers.clear();

		for(auto &layer_id : attribs[Attributes::ActiveLayers].split(',', Qt::SkipEmptyParts))
		{
			if(layer_id.toInt() >= layers.size())
				continue;

			active_layers.push_back(layer_id.toInt());
		}

		if(active_layers.isEmpty())
			active_layers.push_back(0);

		/* Perfoming size validations between the layer color lists and the layers lists
		 * The excessive items from both list are removed until their sizes matches
		 * the layers list */
		while(layer_name_colors.size() > layers.size())
			layer_name_colors.removeLast();

		while(layer_rect_colors.size() > layers.size())
			layer_rect_colors.removeLast();

		protected_model=(attribs[Attributes::Protected]==Attributes::True);

		def_objs[ObjectType::Schema]=attribs[Attributes::DefaultSchema];
		def_objs[ObjectType::Role]=attribs[Attributes::DefaultOwner];
		def_objs[ObjectType::Collation]=attribs[Attributes::DefaultCollation];
		def_objs[ObjectType::Tablespace]=attribs[Attributes::DefaultTablespace];

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem_name=xmlparser.getElementName();

					//Indentifies the object type to be load according to the current element on the parser
					obj_type=getObjectType(elem_name);

					if(elem_name == Attributes::Changelog)
					{
						attribs_map entry_attr;
						xmlparser.savePosition();

						if(xmlparser.accessElement(XmlParser::ChildElement))
						{
							do
							{
								xmlparser.getElementAttributes(entry_attr);
								addChangelogEntry(entry_attr[Attributes::Signature], entry_attr[Attributes::Type],
																	entry_attr[Attributes::Action], entry_attr[Attributes::Date]);
							}
							while(xmlparser.accessElement(XmlParser::NextElement));
						}

						xmlparser.restorePosition();
					}
					else if(obj_type==ObjectType::Database)
					{
						xmlparser.getElementAttributes(attribs);
						configureDatabase(attribs);
					}
					else
					{
						try
						{
							//Saves the current position of the parser before create any object
							xmlparser.savePosition();
							object=createObject(obj_type);

							if(object)
							{
								if(!dynamic_cast<TableObject *>(object) && obj_type!=ObjectType::Relationship && obj_type!=ObjectType::BaseRelationship)
									addObject(object);

								emit s_objectLoaded((xmlparser.getCurrentBufferLine()/static_cast<double>(xmlparser.getBufferLineCount()))*100,
																		tr("Loading: `%1' (%2)")
																				.arg(object->getName())
																				.arg(object->getTypeName()),
																		enum_t(obj_type));
							}

							xmlparser.restorePosition();
						}
						catch(Exception &e)
						{
							QString info_adicional=QString(QObject::tr("%1 (line: %2)")).arg(xmlparser.getLoadedFilename()).arg(xmlparser.getCurrentElement()->line);
							throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, info_adicional);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		this->BaseObject::setProtected(protected_model);

		//Validating default objects
		for(auto &itr : def_objs)
		{
			if(!itr.second.isEmpty())
			{
				object=this->getObject(itr.second, itr.first);

				if(!object)
					throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(this->getName())
															.arg(this->getTypeName())
															.arg(itr.second)
															.arg(BaseObject::getTypeName(itr.first)),
													ErrorCode::AsgDuplicatedPermission,PGM_FUNC,PGM_FILE,PGM_LINE);

				this->setDefaultObject(object);
			}
			else
				this->setDefaultObject(nullptr, itr.first);
		}

		loading_model=false;

		//If there are relationship make a relationship validation to recreate any special object left behind
		if(!relationships.empty())
		{
			emit s_objectLoaded(100, tr("Validating relationships..."), enum_t(ObjectType::Relationship));
			storeSpecialObjectsXML();
			disconnectRelationships();
			validateRelationships();
		}

		this->updateDependencies();
		this->setInvalidated(false);
		emit s_objectLoaded(100, tr("Validating relationships..."), enum_t(ObjectType::Relationship));

		updateTablesFKRelationships();
		restoreFKRelationshipLayers();

		BaseGraphicObject::setUpdatesEnabled(true);
		setObjectsModified();
		emit s_objectLoaded(100, tr("Rendering database model..."), enum_t(ObjectType::BaseObject));
	}
	catch(Exception &e)
	{
		QString extra_info;
		loading_model = false;

		if(xmlparser.getCurrentElement())
			extra_info = QObject::tr("%1 (line: %2)").arg(xmlparser.getLoadedFilename()).arg(xmlparser.getCurrentElement()->line);

		if(e.getErrorCode() != ErrorCode::FileDirectoryNotAccessed &&
			 e.getErrorCode() >= ErrorCode::InvalidSyntax)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvModelFileNotLoaded).arg(filename),
											ErrorCode::InvModelFileNotLoaded,PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
		}

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
	}
}

BaseObject *DatabaseModel::createObject(ObjectType obj_type)
{
	if(create_methods.count(obj_type) == 0)
		return nullptr;

	try
	{
		return create_methods[obj_type]();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseModel::setBasicAttributes(BaseObject *object)
{
	attribs_map attribs, attribs_aux;
	QString elem_name;
	BaseObject *tabspc=nullptr, *owner=nullptr, *collation=nullptr;
	Schema *schema=nullptr;
	ObjectType obj_type=ObjectType::BaseObject, obj_type_aux;
	bool has_error=false, protected_obj=false, sql_disabled=false;
	ForeignObject *frn_object = dynamic_cast<ForeignObject *>(object);

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	xmlparser.getElementAttributes(attribs);
	obj_type_aux=object->getObjectType();

	if(obj_type_aux!=ObjectType::Cast && obj_type_aux != ObjectType::UserMapping)
		object->setName(attribs[Attributes::Name]);

	if(BaseGraphicObject::isGraphicObject(obj_type_aux) && !attribs[Attributes::ZValue].isEmpty())
		dynamic_cast<BaseGraphicObject *>(object)->setZValue(attribs[Attributes::ZValue].toInt());

	if(BaseObject::acceptsAlias(obj_type_aux))
		object->setAlias(attribs[Attributes::Alias]);

	protected_obj=attribs[Attributes::Protected]==Attributes::True;
	sql_disabled=attribs[Attributes::SqlDisabled]==Attributes::True;

	if(frn_object)
	{
		QStringList opt_val;

		for(auto &option : attribs[Attributes::Options].split(ForeignObject::OptionsSeparator))
		{
			opt_val = option.split(UserMapping::OptionValueSeparator);

			if(opt_val.size() < 2)
				continue;

			frn_object->setOption(opt_val[0], opt_val[1]);
		}
	}

	xmlparser.savePosition();

	if(xmlparser.accessElement(XmlParser::ChildElement))
	{
		do
		{
			if(xmlparser.getElementType()==XML_ELEMENT_NODE)
			{
				elem_name=xmlparser.getElementName();

				//Defines the object's comment
				if(elem_name==Attributes::Comment)
				{
					xmlparser.savePosition();
					xmlparser.accessElement(XmlParser::ChildElement);
					object->setComment(xmlparser.getElementContent());
					xmlparser.restorePosition();
				}
				//Defines the object's schema
				else if(elem_name==Attributes::Schema)
				{
					obj_type=ObjectType::Schema;
					xmlparser.getElementAttributes(attribs_aux);
					schema=dynamic_cast<Schema *>(getObject(attribs_aux[Attributes::Name], obj_type));
					object->setSchema(schema);
					has_error=(!schema && !attribs_aux[Attributes::Name].isEmpty());
				}
				//Defines the object's tablespace
				else if(elem_name==Attributes::Tablespace)
				{
					obj_type=ObjectType::Tablespace;
					xmlparser.getElementAttributes(attribs_aux);
					tabspc=getObject(attribs_aux[Attributes::Name], obj_type);
					object->setTablespace(tabspc);
					has_error=(!tabspc && !attribs_aux[Attributes::Name].isEmpty());
				}
				//Defines the object's owner
				else if(elem_name==Attributes::Role)
				{
					obj_type=ObjectType::Role;
					xmlparser.getElementAttributes(attribs_aux);
					owner=getObject(attribs_aux[Attributes::Name], obj_type);
					object->setOwner(owner);
					has_error=(!owner && !attribs_aux[Attributes::Name].isEmpty());
				}
				//Defines the object's schema
				else if(elem_name==Attributes::Collation)
				{
					obj_type=ObjectType::Collation;
					xmlparser.getElementAttributes(attribs_aux);
					collation=getObject(attribs_aux[Attributes::Name], obj_type);
					object->setCollation(collation);
					has_error=(!collation && !attribs_aux[Attributes::Name].isEmpty());
				}
				else if(elem_name==Attributes::AppendedSql)
				{
					xmlparser.savePosition();
					xmlparser.accessElement(XmlParser::ChildElement);
					object->setAppendedSQL(xmlparser.getElementContent());
					xmlparser.restorePosition();
				}
				else if(elem_name==Attributes::PrependedSql)
				{
					xmlparser.savePosition();
					xmlparser.accessElement(XmlParser::ChildElement);
					object->setPrependedSQL(xmlparser.getElementContent());
					xmlparser.restorePosition();
				}
				//Defines the object's position (only for graphical objects)
				else if(elem_name==Attributes::Position)
				{
					xmlparser.getElementAttributes(attribs);

					if(elem_name==Attributes::Position &&
							(obj_type_aux!=ObjectType::Relationship &&
							 obj_type_aux!=ObjectType::BaseRelationship))
					{
						dynamic_cast<BaseGraphicObject *>(object)->setPosition(QPointF(attribs[Attributes::XPos].toDouble(),
																				 attribs[Attributes::YPos].toDouble()));


					}
				}
			}
		}
		while(!has_error && xmlparser.accessElement(XmlParser::NextElement));
	}

	xmlparser.restorePosition();
	object->setProtected(protected_obj);
	object->setSQLDisabled(sql_disabled);

	if(has_error)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
						.arg(object->getName())
						.arg(object->getTypeName())
						.arg(attribs_aux[Attributes::Name])
				.arg(BaseObject::getTypeName(obj_type)),
				ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
	}
	
	if(!object->getSchema() && (BaseObject::acceptsSchema(obj_type_aux) && obj_type_aux != ObjectType::Extension))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvObjectAllocationNoSchema)
										.arg(object->getName())
										.arg(object->getTypeName()),
										ErrorCode::InvObjectAllocationNoSchema,PGM_FUNC,PGM_FILE,PGM_LINE);
	}
}

QString DatabaseModel::getErrorExtraInfo()
{
	QString extra_info;

	if(!xmlparser.getLoadedFilename().isEmpty())
		extra_info=QString(QObject::tr("%1 (line: %2)")).arg(xmlparser.getLoadedFilename())
				   .arg(xmlparser.getCurrentElement()->line);
	else
		extra_info=xmlparser.getXMLBuffer();

	return extra_info;
}

/*void DatabaseModel::setLoadingModel(bool value)
{
	loading_model = value;
}*/

void DatabaseModel::setCancelSaving(bool value)
{
	cancel_saving = value;
}

void DatabaseModel::setObjectListsCapacity(unsigned capacity)
{
  if(capacity < BaseObject::DefMaxObjectCount ||
	 capacity > (BaseObject::DefMaxObjectCount * 1000))
	capacity = BaseObject::DefMaxObjectCount;

  unsigned half_cap = capacity/2, one_fourth_cap = capacity/4;

  views.reserve(capacity);
  tables.reserve(capacity);
  relationships.reserve(capacity);
  base_relationships.reserve(capacity);
  sequences.reserve(capacity);
  permissions.reserve(capacity);
  schemas.reserve(half_cap);
  roles.reserve(half_cap);
  functions.reserve(half_cap);
  types.reserve(half_cap);
  textboxes.reserve(half_cap);
  aggregates.reserve(half_cap);
  operators.reserve(half_cap);
  op_classes.reserve(half_cap);
  op_families.reserve(half_cap);
  domains.reserve(half_cap);
  collations.reserve(half_cap);
  extensions.reserve(half_cap);
  tags.reserve(half_cap);
  genericsqls.reserve(half_cap);
  tablespaces.reserve(one_fourth_cap);
  languages.reserve(one_fourth_cap);
  casts.reserve(one_fourth_cap);
  conversions.reserve(one_fourth_cap);
  eventtriggers.reserve(one_fourth_cap);
}

void DatabaseModel::setLastPosition(const QPoint &pnt)
{
	last_pos=pnt;
}

QPoint DatabaseModel::getLastPosition()
{
	return last_pos;
}

void DatabaseModel::setLastZoomFactor(double zoom)
{
	last_zoom=zoom;
}

double DatabaseModel::getLastZoomFactor()
{
	return last_zoom;
}

Role *DatabaseModel::createRole()
{
	attribs_map attribs, attribs_aux;
	Role *role=nullptr, *ref_role=nullptr;
	int i, len;
	bool marked;
	QStringList list;
	QString elem_name;
	Role::RoleType role_type;

	QString op_attribs[]={ Attributes::Superuser, Attributes::CreateDb,
												 Attributes::CreateRole, Attributes::Inherit,
												 Attributes::Login, Attributes::Replication,
												 Attributes::BypassRls };

	Role::RoleOpts op_vect[]={ Role::OpSuperuser, Role::OpCreateDb,
														 Role::OpCreateRole, Role::OpInherit,
														 Role::OpLogin, Role::OpReplication,
														 Role::OpBypassRls };

	try
	{
		role=new Role;
		setBasicAttributes(role);

		//Gets all the attributes values from the XML
		xmlparser.getElementAttributes(attribs);

		role->setPassword(attribs[Attributes::Password]);
		role->setValidity(attribs[Attributes::Validity]);

		if(!attribs[Attributes::ConnLimit].isEmpty())
			role->setConnectionLimit(attribs[Attributes::ConnLimit].toInt());

		//Setting up the role options according to the configured on the XML
		for(i=0; i < 7; i++)
		{
			marked=attribs[op_attribs[i]]==Attributes::True;
			role->setOption(op_vect[i], marked);
		}

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem_name=xmlparser.getElementName();

					//Getting the member roles
					if(elem_name==Attributes::Roles)
					{
						//Gets the member roles attributes
						xmlparser.getElementAttributes(attribs_aux);

						//The member roles names are separated by comma, so it is needed to split them
						list=attribs_aux[Attributes::Names].split(',');
						len=list.size();

						//Identifying the member role type
						if(attribs_aux[Attributes::RoleType]==Attributes::Member)
							role_type=Role::MemberRole;
						else
							role_type=Role::AdminRole;

						for(i=0; i < len; i++)
						{
							//Gets the role using the name from the model using the name from the list
							ref_role=dynamic_cast<Role *>(getObject(list[i].trimmed(),ObjectType::Role));

							//Raises an error if the roles doesn't exists
							if(!ref_role)
							{
								throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(role->getName())
												.arg(BaseObject::getTypeName(ObjectType::Role))
												.arg(list[i])
												.arg(BaseObject::getTypeName(ObjectType::Role)),
												ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
							}

							role->addRole(role_type, ref_role);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete role;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return role;
}

Tablespace *DatabaseModel::createTablespace()
{
	attribs_map attribs;
	Tablespace *tabspc=nullptr;

	try
	{
		tabspc=new Tablespace;
		setBasicAttributes(tabspc);
		xmlparser.getElementAttributes(attribs);
		tabspc->setDirectory(attribs[Attributes::Directory]);
	}
	catch(Exception &e)
	{
		delete tabspc;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return tabspc;
}

Schema *DatabaseModel::createSchema()
{
	Schema *schema=nullptr;
	attribs_map attribs;

	try
	{
		schema=new Schema;
		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(schema);
		schema->setFillColor(QColor(attribs[Attributes::FillColor]));
		schema->setNameColor(QColor(attribs[Attributes::NameColor]));
		schema->setRectVisible(attribs[Attributes::RectVisible] == Attributes::True);
		schema->setFadedOut(attribs[Attributes::FadedOut] == Attributes::True);
		schema->setLayers(attribs[Attributes::Layers].split(','));
	}
	catch(Exception &e)
	{
		delete schema;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return schema;
}

Language *DatabaseModel::createLanguage()
{
	attribs_map attribs;
	Language *lang=nullptr;
	BaseObject *func=nullptr;
	QString signature, ref_type;
	ObjectType obj_type;

	try
	{
		lang=new Language;
		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(lang);

		lang->setTrusted(attribs[Attributes::Trusted]==Attributes::True);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					obj_type=BaseObject::getObjectType(xmlparser.getElementName());

					if(obj_type==ObjectType::Function)
					{
						xmlparser.getElementAttributes(attribs);

						//Gets the function reference type
						ref_type=attribs[Attributes::RefType];

						//Only VALIDATOR, HANDLER and INLINE functions are accepted for the language
						if(ref_type==Attributes::ValidatorFunc ||
								ref_type==Attributes::HandlerFunc ||
								ref_type==Attributes::InlineFunc)
						{
							//Gets the function signature and tries to retrieve it from the model
							signature=attribs[Attributes::Signature];
							func=getObject(signature, ObjectType::Function);

							//Raises an error if the function doesn't exists
							if(!func)
								throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(lang->getName())
												.arg(lang->getTypeName())
												.arg(signature)
												.arg(BaseObject::getTypeName(ObjectType::Function)),
												ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

							if(ref_type==Attributes::ValidatorFunc)
								lang->setFunction(dynamic_cast<Function *>(func), Language::ValidatorFunc);
							else if(ref_type==Attributes::HandlerFunc)
								lang->setFunction(dynamic_cast<Function *>(func), Language::HandlerFunc);
							else
								lang->setFunction(dynamic_cast<Function *>(func), Language::InlineFunc);

						}
						else
							//Raises an error if the function type is invalid
							throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

	}
	catch(Exception &e)
	{
		delete lang;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return lang;
}

void DatabaseModel::setBasicFunctionAttributes(BaseFunction *func)
{
	if(!func)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		attribs_map attribs, attribs_aux;
		QString elem;
		BaseObject *object = nullptr;
		Parameter param;
		ObjectType obj_type;

		setBasicAttributes(func);
		xmlparser.getElementAttributes(attribs);

		if(!attribs[Attributes::SecurityType].isEmpty())
			func->setSecurityType(SecurityType(attribs[Attributes::SecurityType]));

		xmlparser.savePosition();

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem = xmlparser.getElementName();
					obj_type = BaseObject::getObjectType(elem);

					//Gets the function language
					if(obj_type==ObjectType::Language)
					{
						xmlparser.getElementAttributes(attribs);
						object = getObject(attribs[Attributes::Name], obj_type);

						//Raises an error if the function doesn't exisits
						if(!object)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(func->getName())
											.arg(func->getTypeName())
											.arg(attribs[Attributes::Name])
								.arg(BaseObject::getTypeName(ObjectType::Language)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						func->setLanguage(dynamic_cast<Language *>(object));
					}
					//Gets a function parameter
					else if(xmlparser.getElementName()==Attributes::Parameter)
					{
						param = createParameter();
						func->addParameter(param);
					}
					//Gets the function code definition
					else if(xmlparser.getElementName()==Attributes::Definition)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs_aux);

						if(!attribs_aux[Attributes::Library].isEmpty())
						{
							func->setLibrary(attribs_aux[Attributes::Library]);
							func->setSymbol(attribs_aux[Attributes::Symbol]);
						}
						else if(xmlparser.accessElement(XmlParser::ChildElement))
							func->setFunctionSource(xmlparser.getElementContent());

						xmlparser.restorePosition();
					}
					else if(xmlparser.getElementName() == Attributes::TransformTypes)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs_aux);
						func->addTransformTypes(attribs_aux[Attributes::Names].split(',', Qt::SkipEmptyParts));
						xmlparser.restorePosition();
					}
					else if(xmlparser.getElementName() == Attributes::Configuration)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs_aux);
						func->setConfigurationParam(attribs_aux[Attributes::Name], attribs_aux[Attributes::Value]);
						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		xmlparser.restorePosition();
	}
	catch(Exception &e)
	{
		if(e.getErrorCode()==ErrorCode::RefUserTypeInexistsModel)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
							.arg(func->getName())
							.arg(func->getTypeName()),
							ErrorCode::AsgObjectInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE,&e, getErrorExtraInfo());

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}
}

Function *DatabaseModel::createFunction()
{
	attribs_map attribs, attribs_aux;
	Function *func = nullptr;
	PgSqlType type;
	QString str_aux;
	Parameter param;

	try
	{
		func = new Function;
		setBasicFunctionAttributes(func);
		xmlparser.getElementAttributes(attribs);

		if(!attribs[Attributes::ReturnsSetOf].isEmpty())
			func->setReturnSetOf(attribs[Attributes::ReturnsSetOf]==
					Attributes::True);

		if(!attribs[Attributes::WindowFunc].isEmpty())
			func->setWindowFunction(attribs[Attributes::WindowFunc]==
					Attributes::True);

		if(!attribs[Attributes::LeakProof].isEmpty())
			func->setLeakProof(attribs[Attributes::LeakProof]==
					Attributes::True);

		if(!attribs[Attributes::BehaviorType].isEmpty())
			func->setBehaviorType(BehaviorType(attribs[Attributes::BehaviorType]));

		if(!attribs[Attributes::FunctionType].isEmpty())
			func->setFunctionType(FunctionType(attribs[Attributes::FunctionType]));

		if(!attribs[Attributes::ParallelType].isEmpty())
			func->setParalleType(ParallelType(attribs[Attributes::ParallelType]));

		if(!attribs[Attributes::ExecutionCost].isEmpty())
			func->setExecutionCost(attribs[Attributes::ExecutionCost].toInt());

		if(!attribs[Attributes::RowAmount].isEmpty())
			func->setRowAmount(attribs[Attributes::RowAmount].toInt());

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					//Gets the function return type from the XML
					if(xmlparser.getElementName() == Attributes::ReturnType)
					{
						xmlparser.savePosition();

						try
						{
							xmlparser.accessElement(XmlParser::ChildElement);

							do
							{
								if(xmlparser.getElementType()==XML_ELEMENT_NODE)
								{
									//when the element found is a TYPE indicates that the function return type is a single one
									if(xmlparser.getElementName()==Attributes::Type)
									{
										type=createPgSQLType();
										func->setReturnType(type);
									}
									//when the element found is a PARAMETER indicates that the function return type is a table
									else if(xmlparser.getElementName()==Attributes::Parameter)
									{
										param = createParameter();
										func->addReturnedTableColumn(param.getName(), param.getType());
									}
								}
							}
							while(xmlparser.accessElement(XmlParser::NextElement));

							xmlparser.restorePosition();
						}
						catch(Exception &e)
						{
							xmlparser.restorePosition();
							throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		if(func)
		{
			str_aux = func->getName();
			delete func;
		}

		if(e.getErrorCode()==ErrorCode::RefUserTypeInexistsModel)
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
							.arg(str_aux)
							.arg(BaseObject::getTypeName(ObjectType::Function)),
							ErrorCode::AsgObjectInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE,&e, getErrorExtraInfo());

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return func;
}

Parameter DatabaseModel::createParameter()
{
	Parameter param;
	attribs_map attribs;
	QString elem;

	try
	{
		xmlparser.savePosition();
		xmlparser.getElementAttributes(attribs);

		param.setName(attribs[Attributes::Name]);
		param.setDefaultValue(attribs[Attributes::DefaultValue]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Type)
					{
						param.setType(createPgSQLType());
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		param.setIn(attribs[Attributes::ParamIn]==Attributes::True);
		param.setOut(attribs[Attributes::ParamOut]==Attributes::True);
		param.setVariadic(attribs[Attributes::ParamVariadic]==Attributes::True);

		xmlparser.restorePosition();
	}
	catch(Exception &e)
	{
		QString extra_info=getErrorExtraInfo();
		xmlparser.restorePosition();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
	}

	return param;
}

TypeAttribute DatabaseModel::createTypeAttribute()
{
	TypeAttribute tpattrib;
	attribs_map attribs;
	QString elem;
	BaseObject *collation=nullptr;

	try
	{
		xmlparser.savePosition();
		xmlparser.getElementAttributes(attribs);

		tpattrib.setName(attribs[Attributes::Name]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Type)
					{
						tpattrib.setType(createPgSQLType());
					}
					else if(elem==Attributes::Collation)
					{
						xmlparser.getElementAttributes(attribs);

						collation=getObject(attribs[Attributes::Name], ObjectType::Collation);

						//Raises an error if the operator class doesn't exists
						if(!collation)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(tpattrib.getName())
											.arg(tpattrib.getTypeName())
											.arg(attribs[Attributes::Name])
									.arg(BaseObject::getTypeName(ObjectType::Collation)),
									ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						tpattrib.setCollation(collation);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		xmlparser.restorePosition();
	}
	catch(Exception &e)
	{
		QString extra_info=getErrorExtraInfo();
		xmlparser.restorePosition();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
	}

	return tpattrib;
}

PgSqlType DatabaseModel::createPgSQLType()
{
	attribs_map attribs;
	unsigned length=1, dimension=0, type_idx=0;
	int precision=-1;
	QString name;
	BaseObject *ptype=nullptr;
	bool with_timezone;
	IntervalType interv_type;
	SpatialType spatial_type;

	xmlparser.getElementAttributes(attribs);

	if(!attribs[Attributes::Length].isEmpty())
		length=attribs[Attributes::Length].toUInt();

	if(!attribs[Attributes::Dimension].isEmpty())
		dimension=attribs[Attributes::Dimension].toUInt();

	if(!attribs[Attributes::Precision].isEmpty())
		precision=attribs[Attributes::Precision].toInt();

	with_timezone=(attribs[Attributes::WithTimezone]==Attributes::True);
	interv_type=attribs[Attributes::IntervalType];

	if(!attribs[Attributes::SpatialType].isEmpty())
		spatial_type=SpatialType(attribs[Attributes::SpatialType],
				attribs[Attributes::Srid].toUInt(),
				static_cast<SpatialType::VariationId>(attribs[Attributes::Variation].toUInt()));

	name=attribs[Attributes::Name];

	/* A small tweak to detect a timestamp/date type which name contains the time zone modifier.
		 This situation can occur mainly on reverse engineering operation where the data type of objects
		 in most of times came as string form and need to be parsed */
	if(!with_timezone && attribs[Attributes::Name].contains("with time zone", Qt::CaseInsensitive))
	{
		with_timezone=true;
		name.remove(" with time zone", Qt::CaseInsensitive);
	}

	type_idx = PgSqlType::getBaseTypeIndex(name);

	if(type_idx != PgSqlType::Null)
		return { name, dimension, length, precision,
						 with_timezone, interv_type, spatial_type };

	//Raises an error if the referenced type name doesn't exists
	if(PgSqlType::getUserTypeIndex(name, nullptr, this) == PgSqlType::Null)
		throw Exception(ErrorCode::RefUserTypeInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

	type_idx = PgSqlType::getUserTypeIndex(name, ptype, this);

	return { type_idx, dimension, length, precision,
					 with_timezone, interv_type, spatial_type };
}

Type *DatabaseModel::createType()
{
	attribs_map attribs;
	std::map<QString, Type::FunctionId> func_types;
	Type *type=nullptr;
	QStringList enums;
	QString elem, str_aux;
	BaseObject *func=nullptr, *collation=nullptr;
	OperatorClass *op_class=nullptr;
	PgSqlType aux_type;

	try
	{
		type=new Type;
		setBasicAttributes(type);
		xmlparser.getElementAttributes(attribs);

		if(attribs[Attributes::Configuration]==Attributes::BaseType)
		{
			type->setConfiguration(Type::BaseType);
			type->setByValue(attribs[Attributes::ByValue]==Attributes::True);

			if(!attribs[Attributes::InternalLength].isEmpty())
				type->setInternalLength(attribs[Attributes::InternalLength].toUInt());

			if(!attribs[Attributes::Alignment].isEmpty())
				type->setAlignment(attribs[Attributes::Alignment]);

			if(!attribs[Attributes::Storage].isEmpty())
				type->setStorage(attribs[Attributes::Storage]);

			if(!attribs[Attributes::Element].isEmpty())
				type->setElement(attribs[Attributes::Element]);

			if(!attribs[Attributes::Delimiter].isEmpty())
				type->setDelimiter(attribs[Attributes::Delimiter][0].toLatin1());

			if(!attribs[Attributes::DefaultValue].isEmpty())
				type->setDefaultValue(attribs[Attributes::DefaultValue]);

			if(!attribs[Attributes::Category].isEmpty())
				type->setCategory(attribs[Attributes::Category]);

			if(!attribs[Attributes::Preferred].isEmpty())
				type->setPreferred(attribs[Attributes::Preferred]==Attributes::True);

			//Configuring an auxiliary map used to reference the functions used by base type
			func_types[Attributes::InputFunc]=Type::InputFunc;
			func_types[Attributes::OutputFunc]=Type::OutputFunc;
			func_types[Attributes::SendFunc]=Type::SendFunc;
			func_types[Attributes::RecvFunc]=Type::RecvFunc;
			func_types[Attributes::TpmodInFunc]=Type::TpmodInFunc;
			func_types[Attributes::TpmodOutFunc]=Type::TpmodOutFunc;
			func_types[Attributes::AnalyzeFunc]=Type::AnalyzeFunc;
		}
		else if(attribs[Attributes::Configuration]==Attributes::CompositeType)
			type->setConfiguration(Type::CompositeType);
		else if(attribs[Attributes::Configuration]==Attributes::EnumType)
			type->setConfiguration(Type::EnumerationType);
		else
		{
			type->setConfiguration(Type::RangeType);
			func_types[Attributes::CanonicalFunc]=Type::CanonicalFunc;
			func_types[Attributes::SubtypeDiffFunc]=Type::SubtypeDiffFunc;
		}

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					//Specific operations for ENUM type
					if(elem==Attributes::EnumType)
					{
						xmlparser.getElementAttributes(attribs);
						type->addEnumeration(attribs[Attributes::Label]);
					}
					//Specific operations for COMPOSITE types
					else if(elem==Attributes::TypeAttribute)
					{
						type->addAttribute(createTypeAttribute());
					}
					//Specific operations for BASE / RANGE type
					else if(elem==Attributes::Type)
					{
						aux_type=createPgSQLType();

						if(type->getConfiguration()==Type::RangeType)
							type->setSubtype(aux_type);
						else
							type->setLikeType(aux_type);
					}
					else if(elem==Attributes::Collation)
					{
						xmlparser.getElementAttributes(attribs);
						collation=getObject(attribs[Attributes::Name], ObjectType::Collation);

						//Raises an error if the operator class doesn't exists
						if(!collation)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(type->getName())
											.arg(type->getTypeName())
											.arg(attribs[Attributes::Name])
									.arg(BaseObject::getTypeName(ObjectType::Collation)),
									ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						type->setCollation(collation);
					}
					if(elem==Attributes::OpClass)
					{
						xmlparser.getElementAttributes(attribs);
						op_class=dynamic_cast<OperatorClass *>(getObject(attribs[Attributes::Name], ObjectType::OpClass));

						//Raises an error if the operator class doesn't exists
						if(!op_class)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(type->getName())
											.arg(type->getTypeName())
											.arg(attribs[Attributes::Name])
									.arg(BaseObject::getTypeName(ObjectType::OpClass)),
									ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						type->setSubtypeOpClass(op_class);
					}
					//Configuring the functions used by the type (only for BASE type)
					else if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);

						//Tries to get the function from the model
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists
						if(!func && !attribs[Attributes::Signature].isEmpty())
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(type->getName())
															.arg(type->getTypeName())
															.arg(attribs[Attributes::Signature])
															.arg(BaseObject::getTypeName(ObjectType::Function)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}
						
						//Raises an error if the function type is invalid
						if(func_types.count(attribs[Attributes::RefType]) == 0)
							throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

						type->setFunction(func_types[attribs[Attributes::RefType]],	dynamic_cast<Function *>(func));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		if(type)
		{
			str_aux=type->getName(true);
			delete type;
		}

		if(e.getErrorCode()==ErrorCode::RefUserTypeInexistsModel)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
											.arg(str_aux)
											.arg(type->getTypeName()),
											ErrorCode::AsgObjectInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE,&e, getErrorExtraInfo());
		}
		
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return type;
}

Domain *DatabaseModel::createDomain()
{
	attribs_map attribs;
	Domain *domain=nullptr;
	QString elem;

	try
	{
		domain=new Domain;
		setBasicAttributes(domain);
		xmlparser.getElementAttributes(attribs);

		if(!attribs[Attributes::DefaultValue].isEmpty())
			domain->setDefaultValue(attribs[Attributes::DefaultValue]);

		domain->setNotNull(attribs[Attributes::NotNull]==Attributes::True);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					//If a type element is found it'll be extracted an type which the domain is applied
					if(elem==Attributes::Type)
					{
						domain->setType(createPgSQLType());
					}
					else if(elem==Attributes::Constraint)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs);
						xmlparser.accessElement(XmlParser::ChildElement);
						xmlparser.accessElement(XmlParser::ChildElement);
						domain->addCheckConstraint(attribs[Attributes::Name], xmlparser.getElementContent());
						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(xmlparser.NextElement));
		}
	}
	catch(Exception &e)
	{
		delete domain;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return domain;
}

Cast *DatabaseModel::createCast()
{
	attribs_map attribs;
	Cast *cast=nullptr;
	QString elem;
	unsigned type_idx=0;
	PgSqlType type;
	BaseObject *func=nullptr;

	try
	{
		cast=new Cast;
		setBasicAttributes(cast);
		xmlparser.getElementAttributes(attribs);

		if(attribs[Attributes::CastType]==Attributes::Implicit)
			cast->setCastType(Cast::Implicit);
		else if(attribs[Attributes::CastType]==Attributes::Assignment)
			cast->setCastType(Cast::Assignment);
		else
			cast->setCastType(Cast::Explicit);

		cast->setInOut(attribs[Attributes::IoCast]==Attributes::True);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					//Extract one argument type from the XML
					if(elem==Attributes::Type)
					{
						type=createPgSQLType();
						if(type_idx==0)
							cast->setDataType(Cast::SrcType, type);
						else
							cast->setDataType(Cast::DstType, type);
						type_idx++;
					}
					//Extracts the conversion function
					else if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists
						if(!func && !attribs[Attributes::Signature].isEmpty())
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(cast->getName())
											.arg(cast->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Function)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						cast->setCastFunction(dynamic_cast<Function *>(func));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete cast;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return cast;
}

Conversion *DatabaseModel::createConversion()
{
	attribs_map attribs;
	Conversion *conv=nullptr;
	QString elem;
	BaseObject *func=nullptr;

	try
	{
		conv=new Conversion;
		setBasicAttributes(conv);
		xmlparser.getElementAttributes(attribs);

		conv->setEncoding(Conversion::SrcEncoding,
							EncodingType(attribs[Attributes::SrcEncoding]));

		conv->setEncoding(Conversion::DstEncoding,
							EncodingType(attribs[Attributes::DstEncoding]));

		conv->setDefault(attribs[Attributes::Default]==Attributes::True);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists
						if(!func && !attribs[Attributes::Signature].isEmpty())
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(conv->getName())
											.arg(conv->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Function)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						conv->setConversionFunction(dynamic_cast<Function *>(func));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete conv;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return conv;
}

Operator *DatabaseModel::createOperator()
{
	attribs_map attribs;
	std::map<QString, Operator::FunctionId> func_ids;
	std::map<QString, Operator::OperatorId> oper_ids;
	Operator *oper=nullptr;
	QString elem;
	BaseObject *func=nullptr,*oper_aux=nullptr;
	Operator::ArgumentId arg_id;
	PgSqlType type;

	try
	{
		oper=new Operator;
		setBasicAttributes(oper);
		xmlparser.getElementAttributes(attribs);

		oper->setMerges(attribs[Attributes::Merges]==Attributes::True);
		oper->setHashes(attribs[Attributes::Hashes]==Attributes::True);

		func_ids[Attributes::OperatorFunc]=Operator::FuncOperator;
		func_ids[Attributes::JoinFunc]=Operator::FuncJoin;
		func_ids[Attributes::RestrictionFunc]=Operator::FuncRestrict;

		oper_ids[Attributes::CommutatorOp]=Operator::OperCommutator;
		oper_ids[Attributes::NegatorOp]=Operator::OperNegator;

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==objs_schemas[enum_t(ObjectType::Operator)])
					{
						xmlparser.getElementAttributes(attribs);
						oper_aux=getObject(attribs[Attributes::Signature], ObjectType::Operator);

						//Raises an error if the auxiliary operator doesn't exists
						if(!oper_aux && !attribs[Attributes::Signature].isEmpty())
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(oper->getSignature(true))
											.arg(oper->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Operator)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						oper->setOperator(dynamic_cast<Operator *>(oper_aux),
											oper_ids[attribs[Attributes::RefType]]);
					}
					else if(elem==Attributes::Type)
					{
						xmlparser.getElementAttributes(attribs);

						if(attribs[Attributes::RefType]!=Attributes::RightType)
							arg_id=Operator::LeftArg;
						else
							arg_id=Operator::RightArg;

						type=createPgSQLType();
						oper->setArgumentType(type, arg_id);
					}
					else if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists on the model
						if(!func && !attribs[Attributes::Signature].isEmpty())
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(oper->getName())
											.arg(oper->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Function)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						oper->setFunction(dynamic_cast<Function *>(func),
											func_ids[attribs[Attributes::RefType]]);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete oper;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return oper;
}

OperatorClass *DatabaseModel::createOperatorClass()
{
	attribs_map attribs, attribs_aux;
	std::map<QString, unsigned> elem_types;
	BaseObject *object=nullptr;
	QString elem;
	PgSqlType type;
	OperatorClass *op_class=nullptr;
	OperatorClassElement class_elem;
	unsigned stg_number, elem_type;

	try
	{
		op_class=new OperatorClass;
		setBasicAttributes(op_class);
		xmlparser.getElementAttributes(attribs);

		op_class->setIndexingType(IndexingType(attribs[Attributes::IndexType]));
		op_class->setDefault(attribs[Attributes::Default]==Attributes::True);

		elem_types[Attributes::Function]=OperatorClassElement::FunctionElem;
		elem_types[Attributes::Operator]=OperatorClassElement::OperatorElem;
		elem_types[Attributes::Storage]=OperatorClassElement::StorageElem;

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==objs_schemas[enum_t(ObjectType::OpFamily)])
					{
						xmlparser.getElementAttributes(attribs);
						object=getObject(attribs[Attributes::Signature], ObjectType::OpFamily);

						//Raises an error if the operator family doesn't exists
						if(!object)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(op_class->getName())
											.arg(op_class->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::OpFamily)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						op_class->setFamily(dynamic_cast<OperatorFamily *>(object));
					}
					else if(elem==Attributes::Type)
					{
						xmlparser.getElementAttributes(attribs);
						type=createPgSQLType();
						op_class->setDataType(type);
					}
					else if(elem==Attributes::Element)
					{
						xmlparser.getElementAttributes(attribs);

						stg_number=attribs[Attributes::StrategyNum].toUInt();
						elem_type=elem_types[attribs[Attributes::Type]];

						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						xmlparser.getElementAttributes(attribs);

						if(elem_type==OperatorClassElement::StorageElem)
						{
							type=createPgSQLType();
							class_elem.setStorage(type);
						}
						else if(elem_type==OperatorClassElement::FunctionElem)
						{
							object=getObject(attribs[Attributes::Signature],ObjectType::Function);
							class_elem.setFunction(dynamic_cast<Function *>(object),stg_number);
						}
						else if(elem_type==OperatorClassElement::OperatorElem)
						{
							object=getObject(attribs[Attributes::Signature],ObjectType::Operator);
							class_elem.setOperator(dynamic_cast<Operator *>(object),stg_number);

							if(xmlparser.hasElement(XmlParser::NextElement))
							{
								xmlparser.savePosition();
								xmlparser.accessElement(XmlParser::NextElement);
								xmlparser.getElementAttributes(attribs_aux);

								object=getObject(attribs_aux[Attributes::Signature],ObjectType::OpFamily);

								if(!object && !attribs_aux[Attributes::Signature].isEmpty())
									throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
													.arg(op_class->getName())
													.arg(op_class->getTypeName())
													.arg(attribs_aux[Attributes::Signature])
										.arg(BaseObject::getTypeName(ObjectType::OpFamily)),
										ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

								class_elem.setOperatorFamily(dynamic_cast<OperatorFamily *>(object));
								xmlparser.restorePosition();
							}
						}

						op_class->addElement(class_elem);
						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete op_class;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return op_class;
}

OperatorFamily *DatabaseModel::createOperatorFamily()
{
	attribs_map attribs;
	OperatorFamily *op_family=nullptr;

	try
	{
		op_family=new OperatorFamily;
		setBasicAttributes(op_family);
		xmlparser.getElementAttributes(attribs);
		op_family->setIndexingType(IndexingType(attribs[Attributes::IndexType]));
	}
	catch(Exception &e)
	{
		delete op_family;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return op_family;
}

Aggregate *DatabaseModel::createAggregate()
{
	attribs_map attribs;
	BaseObject *func=nullptr;
	QString elem;
	PgSqlType type;
	Aggregate *aggreg=nullptr;

	try
	{
		aggreg=new Aggregate;
		setBasicAttributes(aggreg);
		xmlparser.getElementAttributes(attribs);

		aggreg->setInitialCondition(attribs[Attributes::InitialCond]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Type)
					{
						xmlparser.getElementAttributes(attribs);
						type=createPgSQLType();

						if(attribs[Attributes::RefType]==Attributes::StateType)
							aggreg->setStateType(type);
						else
							aggreg->addDataType(type);
					}
					else if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists on the model
						if(!func && !attribs[Attributes::Signature].isEmpty())
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(aggreg->getName())
											.arg(aggreg->getTypeName())
											.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Function)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						if(attribs[Attributes::RefType]==Attributes::TransitionFunc)
							aggreg->setFunction(Aggregate::TransitionFunc,
												dynamic_cast<Function *>(func));
						else
							aggreg->setFunction(Aggregate::FinalFunc,
												dynamic_cast<Function *>(func));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete aggreg;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return aggreg;
}

Table *DatabaseModel::createTable()
{
	try
	{
		Table *table = nullptr;
		attribs_map attribs;

		xmlparser.getElementAttributes(attribs);
		table = createPhysicalTable<Table>();
		table->setUnlogged(attribs[Attributes::Unlogged]==Attributes::True);
		table->setRLSEnabled(attribs[Attributes::RlsEnabled]==Attributes::True);
		table->setRLSForced(attribs[Attributes::RlsForced]==Attributes::True);
		table->setWithOIDs(attribs[Attributes::Oids]==Attributes::True);

		return table;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Column *DatabaseModel::createColumn()
{
	attribs_map attribs;
	Column *column=nullptr;
	BaseObject *seq=nullptr;
	QString elem;

	try
	{
		column=new Column;
		setBasicAttributes(column);

		xmlparser.getElementAttributes(attribs);
		column->setNotNull(attribs[Attributes::NotNull]==Attributes::True);
		column->setGenerated(attribs[Attributes::Generated]==Attributes::True);
		column->setDefaultValue(attribs[Attributes::DefaultValue]);
		column->setIdSeqAttributes(attribs[Attributes::MinValue], attribs[Attributes::MaxValue], attribs[Attributes::Increment],
																attribs[Attributes::Start], attribs[Attributes::Cache], attribs[Attributes::Cycle] == Attributes::True);

		if(!attribs[Attributes::IdentityType].isEmpty())
			column->setIdentityType(IdentityType(attribs[Attributes::IdentityType]));

		if(!attribs[Attributes::Sequence].isEmpty())
		{
			seq=getObject(attribs[Attributes::Sequence], ObjectType::Sequence);

			if(!seq)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
								.arg(attribs[Attributes::Name])
					.arg(BaseObject::getTypeName(ObjectType::Column))
					.arg(attribs[Attributes::Sequence])
					.arg(BaseObject::getTypeName(ObjectType::Sequence)),
					ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}


			column->setSequence(seq);
		}

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Type)
					{
						column->setType(createPgSQLType());
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete column;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return column;
}

Constraint *DatabaseModel::createConstraint(BaseObject *parent_obj)
{
	attribs_map attribs;
	Constraint *constr=nullptr;
	BaseObject *ref_table=nullptr;
	PhysicalTable *table=nullptr,*table_aux=nullptr;
	Column *column=nullptr;
	Relationship *rel=nullptr;
	QString elem, str_aux;
	bool deferrable, ins_constr_table=false;
	ConstraintType constr_type;
	QStringList col_list;
	int count, i;
	Constraint::ColumnsId cols_id;
	ObjectType obj_type;
	ExcludeElement exc_elem;

	try
	{
		xmlparser.getElementAttributes(attribs);

		//If the constraint parent is allocated
		if(parent_obj)
		{
			obj_type=parent_obj->getObjectType();

			//Identifies the correct parent type
			if(PhysicalTable::isPhysicalTable(obj_type))
				table=dynamic_cast<PhysicalTable *>(parent_obj);
			else if(obj_type==ObjectType::Relationship)
				rel=dynamic_cast<Relationship *>(parent_obj);
			else
				//Raises an error if the user tries to create a constraint in a invalid parent
				throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
		else
		{
			obj_type = ObjectType::Table;
			table = dynamic_cast<PhysicalTable *>(getObject(attribs[Attributes::Table], {ObjectType::Table, ObjectType::ForeignTable}));
			parent_obj=table;
			ins_constr_table=true;

			//Raises an error if the parent table doesn't exists
			if(!table)
			{
				str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
						.arg(attribs[Attributes::Name])
						.arg(BaseObject::getTypeName(ObjectType::Constraint))
						.arg(attribs[Attributes::Table])
						.arg(BaseObject::getTypeName(obj_type));

				throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		constr=new Constraint;
		constr->setParentTable(table);

		//Configuring the constraint type
		if(attribs[Attributes::Type]==Attributes::CkConstr)
			constr_type=ConstraintType::Check;
		else if(attribs[Attributes::Type]==Attributes::PkConstr)
			constr_type=ConstraintType::PrimaryKey;
		else if(attribs[Attributes::Type]==Attributes::FkConstr)
			constr_type=ConstraintType::ForeignKey;
		else if(attribs[Attributes::Type]==Attributes::UqConstr)
			constr_type=ConstraintType::Unique;
		else
			constr_type=ConstraintType::Exclude;

		constr->setConstraintType(constr_type);

		if(!attribs[Attributes::Factor].isEmpty())
			constr->setFillFactor(attribs[Attributes::Factor].toUInt());

		setBasicAttributes(constr);

		//Raises an error if the constraint is a primary key and no parent object is specified
		if(!parent_obj && constr_type==ConstraintType::PrimaryKey)
			throw Exception(Exception::getErrorMessage(ErrorCode::InvPrimaryKeyAllocation)
							.arg(constr->getName()),
							ErrorCode::InvPrimaryKeyAllocation,PGM_FUNC,PGM_FILE,PGM_LINE);

		deferrable=(attribs[Attributes::Deferrable]==Attributes::True);
		constr->setDeferrable(deferrable);

		if(deferrable && !attribs[Attributes::DeferType].isEmpty())
			constr->setDeferralType(attribs[Attributes::DeferType]);

		if(constr_type==ConstraintType::ForeignKey)
		{
			if(!attribs[Attributes::ComparisonType].isEmpty())
				constr->setMatchType(attribs[Attributes::ComparisonType]);

			if(!attribs[Attributes::DelAction].isEmpty())
				constr->setActionType(attribs[Attributes::DelAction], Constraint::DeleteAction);

			if(!attribs[Attributes::UpdAction].isEmpty())
				constr->setActionType(attribs[Attributes::UpdAction], Constraint::UpdateAction);

			ref_table=getObject(attribs[Attributes::RefTable], ObjectType::Table);

			if(!ref_table && table->getName(true)==attribs[Attributes::RefTable])
				ref_table=table;

			//Raises an error if the referenced table doesn't exists
			if(!ref_table)
			{
				str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
						.arg(constr->getName())
						.arg(constr->getTypeName())
						.arg(attribs[Attributes::RefTable])
						.arg(BaseObject::getTypeName(ObjectType::Table));

				throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			constr->setReferencedTable(dynamic_cast<BaseTable *>(ref_table));
		}
		else if(constr_type==ConstraintType::Check)
		{
			constr->setNoInherit(attribs[Attributes::NoInherit]==Attributes::True);
		}
		else if(constr_type==ConstraintType::Exclude &&	!attribs[Attributes::IndexType].isEmpty())
		{
			constr->setIndexType(attribs[Attributes::IndexType]);
		}
		else if(constr_type==ConstraintType::Unique)
			constr->setNullsNotDistinct(attribs[Attributes::NullsNotDistinct]==Attributes::True);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::ExcludeElement)
					{
						createElement(exc_elem, constr, parent_obj);
						constr->addExcludeElement(exc_elem);
					}
					else if(elem==Attributes::Expression)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						constr->setExpression(xmlparser.getElementContent());

						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Columns)
					{
						xmlparser.getElementAttributes(attribs);

						col_list=attribs[Attributes::Names].split(',');
						count=col_list.count();

						if(attribs[Attributes::RefType]==Attributes::SrcColumns)
							cols_id=Constraint::SourceCols;
						else
							cols_id=Constraint::ReferencedCols;

						for(i=0; i < count; i++)
						{
							if(cols_id==Constraint::SourceCols)
							{
								if(PhysicalTable::isPhysicalTable(obj_type))
								{
									column=table->getColumn(col_list[i]);

									//If the column doesn't exists tries to get it searching by the old name
									if(!column)
										column=table->getColumn(col_list[i], true);
								}
								else
									column=dynamic_cast<Column *>(rel->getObject(col_list[i], ObjectType::Column));
							}
							else
							{
								table_aux=dynamic_cast<PhysicalTable *>(ref_table);
								column=table_aux->getColumn(col_list[i]);

								//If the column doesn't exists tries to get it searching by the old name
								if(!column)
									column=table_aux->getColumn(col_list[i], true);
							}

							constr->addColumn(column, cols_id);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		if(ins_constr_table)
		{
			if(constr->getConstraintType()!=ConstraintType::PrimaryKey)
			{
				table->addConstraint(constr);

				if(this->getObjectIndex(table) >= 0)
					table->setModified(!loading_model);
			}
		}
	}
	catch(Exception &e)
	{
		delete constr;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return constr;
}

void DatabaseModel::createElement(Element &elem, TableObject *tab_obj, BaseObject *parent_obj)
{
	attribs_map attribs;
	Column *column=nullptr;
	OperatorClass *op_class=nullptr;
	Operator *oper=nullptr;
	Collation *collation=nullptr;
	QString xml_elem;
	bool is_part_key = false;

	xml_elem=xmlparser.getElementName();
	is_part_key = xml_elem == Attributes::PartitionKey;

	if(xml_elem==Attributes::IndexElement || xml_elem==Attributes::ExcludeElement || is_part_key)
	{
		xmlparser.getElementAttributes(attribs);

		elem.setColumn(nullptr);
		elem.setCollation(nullptr);
		elem.setOperator(nullptr);
		elem.setOperatorClass(nullptr);

		elem.setSortingAttribute(Element::AscOrder, attribs[Attributes::AscOrder]==Attributes::True);
		elem.setSortingAttribute(Element::NullsFirst, attribs[Attributes::NullsFirst]==Attributes::True);
		elem.setSortingEnabled(attribs[Attributes::UseSorting]!=Attributes::False);

		xmlparser.savePosition();
		xmlparser.accessElement(XmlParser::ChildElement);

		do
		{
			xml_elem=xmlparser.getElementName();

			if(xmlparser.getElementType()==XML_ELEMENT_NODE)
			{
				if(xml_elem==Attributes::OpClass)
				{
					xmlparser.getElementAttributes(attribs);
					op_class=dynamic_cast<OperatorClass *>(getObject(attribs[Attributes::Signature], ObjectType::OpClass));

					//Raises an error if the operator class doesn't exists
					if(!op_class)
					{
						if(!is_part_key)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(tab_obj->getName())
															.arg(tab_obj->getTypeName())
															.arg(attribs[Attributes::Signature])
															.arg(BaseObject::getTypeName(ObjectType::OpClass)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						throw Exception(Exception::getErrorMessage(ErrorCode::PartKeyObjectInexistsModel)
														.arg(parent_obj->getName())
														.arg(attribs[Attributes::Signature])
														.arg(BaseObject::getTypeName(ObjectType::OpClass)),
														ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
					}

					elem.setOperatorClass(op_class);
				}
				//Checking if elem is a ExcludeElement to be able to assign an operator to it
				else if(xml_elem==Attributes::Operator)
				{
					xmlparser.getElementAttributes(attribs);
					oper=dynamic_cast<Operator *>(getObject(attribs[Attributes::Signature], ObjectType::Operator));

					//Raises an error if the operator doesn't exists
					if(!oper)
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
										.arg(tab_obj->getName())
										.arg(tab_obj->getTypeName())
										.arg(attribs[Attributes::Signature])
								.arg(BaseObject::getTypeName(ObjectType::Operator)),
								ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
					}

					elem.setOperator(oper);
				}
				else if(xml_elem==Attributes::Collation)
				{
					xmlparser.getElementAttributes(attribs);
					collation=dynamic_cast<Collation *>(getObject(attribs[Attributes::Name], ObjectType::Collation));

					//Raises an error if the operator class doesn't exists
					if(!collation)
					{
						if(!is_part_key)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(tab_obj->getName())
															.arg(tab_obj->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::Collation)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}
						
						
						throw Exception(Exception::getErrorMessage(ErrorCode::PartKeyObjectInexistsModel)
														.arg(parent_obj->getName())
														.arg(attribs[Attributes::Name])
														.arg(BaseObject::getTypeName(ObjectType::Collation)),
														ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						
					}

					elem.setCollation(collation);
				}
				else if(xml_elem==Attributes::Column)
				{
					xmlparser.getElementAttributes(attribs);

					if(parent_obj->getObjectType() == ObjectType::Table)
					{
						Table *table = dynamic_cast<Table *>(parent_obj);

						column = table->getColumn(attribs[Attributes::Name]);

						if(!column)
							column = table->getColumn(attribs[Attributes::Name], true);

						elem.setColumn(column);
					}
					else if(parent_obj->getObjectType() == ObjectType::View)
					{
						View *view = dynamic_cast<View *>(parent_obj);
						elem.setSimpleColumn(view->getColumn(attribs[Attributes::Name]));
					}
					else
					{
						column = dynamic_cast<Column *>(dynamic_cast<Relationship *>(parent_obj)->getObject(attribs[Attributes::Name], ObjectType::Column));
						elem.setColumn(column);
					}

					//Raises an error if the column doesn't exists
					if((!elem.getColumn() && parent_obj->getObjectType() == ObjectType::Table) ||
						 (!elem.getSimpleColumn().isValid() && parent_obj->getObjectType() == ObjectType::View))
					{
						if(!is_part_key)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(tab_obj->getName())
															.arg(tab_obj->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::Column)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}
						
						throw Exception(Exception::getErrorMessage(ErrorCode::PartKeyObjectInexistsModel)
														.arg(parent_obj->getName())	
														.arg(attribs[Attributes::Name])
														.arg(BaseObject::getTypeName(ObjectType::Column)),
														ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						
					}
				}
				else if(xml_elem==Attributes::Expression)
				{
					xmlparser.savePosition();
					xmlparser.accessElement(XmlParser::ChildElement);
					elem.setExpression(xmlparser.getElementContent());
					xmlparser.restorePosition();
				}
			}
		}
		while(xmlparser.accessElement(XmlParser::NextElement));

		xmlparser.restorePosition();
	}
}

XmlParser *DatabaseModel::getXMLParser()
{
	return &xmlparser;
}

QString DatabaseModel::getAlterCode(BaseObject *object)
{
	DatabaseModel *db_aux=dynamic_cast<DatabaseModel *>(object);

	if(!db_aux)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		QString alter_def;
		attribs_map aux_attribs;

		aux_attribs[Attributes::Signature] = this->getSignature();
		aux_attribs[Attributes::SqlObject] = this->getSQLName();

		if(this->conn_limit!=db_aux->conn_limit)
			aux_attribs[Attributes::ConnLimit]=QString::number(db_aux->conn_limit);

		if(this->is_template != db_aux->is_template)
			aux_attribs[Attributes::IsTemplate] = (db_aux->is_template ? Attributes::True : Attributes::False);

		if(this->allow_conns != db_aux->allow_conns)
			aux_attribs[Attributes::AllowConns] = (db_aux->allow_conns ? Attributes::True : Attributes::False);

		alter_def+=BaseObject::getAlterCode(this->getSchemaName(), aux_attribs, true, false);
		alter_def+=BaseObject::getAlterCode(object);

		return alter_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

Index *DatabaseModel::createIndex()
{
	attribs_map attribs;
	Index *index=nullptr;
	QString elem, str_aux;
	IndexElement idx_elem;
	BaseTable *table=nullptr;

	try
	{
		xmlparser.getElementAttributes(attribs);

		table=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], ObjectType::Table));

		if(!table)
			table=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], ObjectType::View));

		//Raises an error if the parent table doesn't exists
		if(!table)
		{
			str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
					.arg(attribs[Attributes::Name])
					.arg(BaseObject::getTypeName(ObjectType::Index))
					.arg(attribs[Attributes::Table])
					.arg(BaseObject::getTypeName(ObjectType::Table));

			throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		index=new Index;
		setBasicAttributes(index);
		index->setParentTable(table);
		index->setIndexAttribute(Index::Concurrent, attribs[Attributes::Concurrent]==Attributes::True);
		index->setIndexAttribute(Index::Unique, attribs[Attributes::Unique]==Attributes::True);
		index->setIndexAttribute(Index::FastUpdate, attribs[Attributes::FastUpdate]==Attributes::True);
		index->setIndexAttribute(Index::Buffering, attribs[Attributes::Buffering]==Attributes::True);
		index->setIndexAttribute(Index::NullsNotDistinct, attribs[Attributes::NullsNotDistinct]==Attributes::True);
		index->setIndexingType(attribs[Attributes::IndexType]);
		index->setFillFactor(attribs[Attributes::Factor].toUInt());

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::IndexElement)
					{
						createElement(idx_elem, index, table);
						index->addIndexElement(idx_elem);
					}
					else if(elem==Attributes::Predicate)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						str_aux=xmlparser.getElementContent();
						xmlparser.restorePosition();
						index->setPredicate(str_aux);
					}
					else if(elem == Attributes::Columns)
					{
						xmlparser.getElementAttributes(attribs);
						QStringList col_names =  attribs[Attributes::Names].split(',', Qt::SkipEmptyParts);

						if(table->getObjectType() == ObjectType::Table)
						{
							for(auto &name : col_names)
								index->addColumn(dynamic_cast<Column *>(table->getObject(name, ObjectType::Column)));
						}
						else
						{
							View *view = dynamic_cast<View *>(table);
							SimpleColumn col;

							for(auto &name : col_names)
								index->addSimpleColumn(view->getColumn(name));
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		table->addObject(index);
		table->setModified(!loading_model);
	}
	catch(Exception &e)
	{
		delete index;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return index;
}

Rule *DatabaseModel::createRule()
{
	attribs_map attribs;
	QStringList cmd_list;
	Rule *rule=nullptr;
	QString elem, str_aux;
	int count, i;
	BaseTable *table=nullptr;

	try
	{
		rule=new Rule;
		setBasicAttributes(rule);

		xmlparser.getElementAttributes(attribs);

		table=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], ObjectType::Table));

		if(!table)
			table=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], ObjectType::View));

		if(!table)
			throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
							.arg(attribs[Attributes::Name])
				.arg(BaseObject::getTypeName(ObjectType::Rule))
				.arg(attribs[Attributes::Table])
				.arg(BaseObject::getTypeName(ObjectType::Table)),
				ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);


		rule->setExecutionType(attribs[Attributes::ExecType]);
		rule->setEventType(attribs[Attributes::EventType]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Commands ||
							elem==Attributes::Condition)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						str_aux=xmlparser.getElementContent();
						xmlparser.restorePosition();

						if(elem==Attributes::Commands)
						{
							cmd_list=str_aux.split(';');
							count=cmd_list.count();
							for(i=0; i < count; i++)
							{
								if(!cmd_list[i].isEmpty())
									rule->addCommand(cmd_list[i]);
							}
						}
						else
							rule->setConditionalExpression(str_aux);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		table->addObject(rule);
		table->setModified(!loading_model);
	}
	catch(Exception &e)
	{
		delete rule;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return rule;
}

Trigger *DatabaseModel::createTrigger()
{
	attribs_map attribs;
	Trigger *trigger=nullptr;
	QString elem, str_aux;
	QStringList list_aux;
	int count, i;
	BaseObject *ref_table=nullptr, *func=nullptr;
	Column *column=nullptr;
	BaseTable *table=nullptr;
	PhysicalTable *aux_table = nullptr;
	std::vector<ObjectType> table_types = { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View };

	try
	{
		xmlparser.getElementAttributes(attribs);

		for(auto &type : table_types)
		{
			table = dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], type));
			if(table) break;
		}

		if(!table)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
							.arg(attribs[Attributes::Name])
				.arg(BaseObject::getTypeName(ObjectType::Trigger))
				.arg(attribs[Attributes::Table])
				.arg(BaseObject::getTypeName(ObjectType::Table)),
				ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		trigger=new Trigger;
		trigger->setParentTable(table);

		setBasicAttributes(trigger);

		trigger->setConstraint(attribs[Attributes::Constraint]==Attributes::True);

		trigger->setEvent(EventType::OnInsert,
							(attribs[Attributes::InsEvent]==Attributes::True));

		trigger->setEvent(EventType::OnDelete,
							(attribs[Attributes::DelEvent]==Attributes::True));

		trigger->setEvent(EventType::OnUpdate,
							(attribs[Attributes::UpdEvent]==Attributes::True));

		trigger->setEvent(EventType::OnTruncate,
							(attribs[Attributes::TruncEvent]==Attributes::True));

		trigger->setExecutePerRow(attribs[Attributes::PerRow]==Attributes::True);

		trigger->setFiringType(FiringType(attribs[Attributes::FiringType]));

		trigger->setTransitionTableName(Trigger::OldTableName, attribs[Attributes::OldTableName]);
		trigger->setTransitionTableName(Trigger::NewTableName, attribs[Attributes::NewTableName]);

		trigger->addArguments(attribs[Attributes::Arguments].split(UtilsNs::DataSeparator, Qt::SkipEmptyParts));
		trigger->setDeferrable(attribs[Attributes::Deferrable]==Attributes::True);

		if(trigger->isDeferrable())
			trigger->setDeferralType(attribs[Attributes::DeferType]);

		if(!attribs[Attributes::RefTable].isEmpty())
		{
			ref_table=getObject(attribs[Attributes::RefTable], ObjectType::Table);

			if(!ref_table)
				ref_table=getObject(attribs[Attributes::RefTable], ObjectType::View);

			//Raises an error if the trigger is referencing a inexistent table
			if(!ref_table)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
								.arg(trigger->getName())
								.arg(trigger->getTypeName())
								.arg(attribs[Attributes::RefTable])
						.arg(BaseObject::getTypeName(ObjectType::Table)),
						ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			trigger->setReferecendTable(dynamic_cast<BaseTable *>(ref_table));
		}

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists
						if(!func && !attribs[Attributes::Signature].isEmpty())
						{
							str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
									.arg(trigger->getName())
									.arg(trigger->getTypeName())
									.arg(attribs[Attributes::Signature])
									.arg(BaseObject::getTypeName(ObjectType::Function));

							throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						trigger->setFunction(dynamic_cast<Function *>(func));
					}
					else if(elem==Attributes::Condition)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						str_aux=xmlparser.getElementContent();
						xmlparser.restorePosition();
						trigger->setCondition(str_aux);
					}
					else if(elem==Attributes::Columns)
					{
						xmlparser.getElementAttributes(attribs);

						list_aux=attribs[Attributes::Names].split(',');
						count=list_aux.count();

						for(i=0; i < count; i++)
						{
							column=dynamic_cast<Column *>(table->getObject(list_aux[i].trimmed(), ObjectType::Column));
							aux_table=dynamic_cast<PhysicalTable *>(table);

							if(!column && aux_table)
								column=aux_table->getColumn(list_aux[i].trimmed(), true);

							trigger->addColumn(column);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		table->addObject(trigger);
		table->setModified(!loading_model);
	}
	catch(Exception &e)
	{
		delete trigger;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return trigger;
}

Policy *DatabaseModel::createPolicy()
{
	attribs_map attribs;
	Policy *policy=nullptr;
	QString elem;
	BaseTable *table=nullptr;

	try
	{
		policy=new Policy;
		setBasicAttributes(policy);

		xmlparser.getElementAttributes(attribs);

		table=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::Table], ObjectType::Table));

		if(!table)
			throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(attribs[Attributes::Name])
											.arg(BaseObject::getTypeName(ObjectType::Policy))
											.arg(attribs[Attributes::Table])
											.arg(BaseObject::getTypeName(ObjectType::Table)),
				ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

		policy->setPermissive(attribs[Attributes::Permissive] == Attributes::True);
		policy->setPolicyCommand(PolicyCmdType(attribs[Attributes::Command]));

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Expression)
					{
						xmlparser.getElementAttributes(attribs);
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						if(attribs[Attributes::Type] == Attributes::UsingExp)
							policy->setUsingExpression(xmlparser.getElementContent());
						else if(attribs[Attributes::Type] == Attributes::CheckExp)
							policy->setCheckExpression(xmlparser.getElementContent());

						xmlparser.restorePosition();
					}
					else if(xmlparser.getElementName()==Attributes::Roles)
					{
						QStringList rol_names;
						Role *role = nullptr;

						xmlparser.getElementAttributes(attribs);

						rol_names = attribs[Attributes::Names].split(',');

						for(auto &name : rol_names)
						{
							role=dynamic_cast<Role *>(getObject(name.trimmed(), ObjectType::Role));

							//Raises an error if the referenced role doesn't exists
							if(!role)
							{
								throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
																.arg(policy->getName())
																.arg(policy->getTypeName())
																.arg(name)
																.arg(BaseObject::getTypeName(ObjectType::Role)),
																ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
							}

							policy->addRole(role);
						}
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		table->addObject(policy);
		table->setModified(!loading_model);
	}
	catch(Exception &e)
	{
		delete policy;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return policy;
}

EventTrigger *DatabaseModel::createEventTrigger()
{
	attribs_map attribs;
	EventTrigger *event_trig=nullptr;
	BaseObject *func=nullptr;
	QString elem;

	try
	{
		event_trig=new EventTrigger;
		setBasicAttributes(event_trig);
		xmlparser.getElementAttributes(attribs);
		event_trig->setEvent(EventTriggerType(attribs[Attributes::Event]));

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Function)
					{
						xmlparser.getElementAttributes(attribs);
						func=getObject(attribs[Attributes::Signature], ObjectType::Function);

						//Raises an error if the function doesn't exists
						if(!func && !attribs[Attributes::Signature].isEmpty())
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
											.arg(event_trig->getName())
											.arg(event_trig->getTypeName())
											.arg(attribs[Attributes::Signature])
									.arg(BaseObject::getTypeName(ObjectType::Function)),
									ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						event_trig->setFunction(dynamic_cast<Function *>(func));
					}
					else if(elem==Attributes::Filter)
					{
						xmlparser.getElementAttributes(attribs);
						event_trig->setFilter(attribs[Attributes::Variable], attribs[Attributes::Values].split(','));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

	}
	catch(Exception &e)
	{
		delete event_trig;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return event_trig;
}

GenericSQL *DatabaseModel::createGenericSQL()
{
	GenericSQL *genericsql=nullptr;
	attribs_map attribs;
	QString elem, parent_name, obj_name;
	ObjectType obj_type;
	PhysicalTable *parent_table = nullptr;
	BaseObject *object = nullptr;

	try
	{
		genericsql = new GenericSQL;
		setBasicAttributes(genericsql);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem = xmlparser.getElementName();

					if(elem == Attributes::Definition)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						genericsql->setDefinition(xmlparser.getElementContent());
						xmlparser.restorePosition();
					}
					else if(elem == Attributes::Reference)
					{
						xmlparser.getElementAttributes(attribs);

						obj_type = BaseObject::getObjectType(attribs[Attributes::Type]);
						obj_name = attribs[Attributes::Object];

						//If the object is a column its needed to get the parent table
						if(obj_type == ObjectType::Column)
						{
							QStringList names = obj_name.split('.');

							if(names.size() > 2)
							{
								parent_name = QString("%1.%2").arg(names[0]).arg(names[1]);
								obj_name = names[2];
							}

							parent_table = dynamic_cast<PhysicalTable *>(getObject(parent_name, {ObjectType::Table, ObjectType::ForeignTable}));

							if(parent_table)
								object = parent_table->getColumn(obj_name);
						}
						else
							object = getObject(obj_name, obj_type);

						//Raises an error if the generic object references an object that does not exist
						if(!object)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(genericsql->getName())
															.arg(genericsql->getTypeName())
															.arg(obj_name)
															.arg(BaseObject::getTypeName(obj_type)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						genericsql->addReference(Reference(object,
																								attribs[Attributes::RefName],
																								attribs[Attributes::RefAlias],
																								attribs[Attributes::UseSignature] == Attributes::True,
																								attribs[Attributes::FormatName] == Attributes::True,
																								attribs[Attributes::UseColumns] == Attributes::True));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete genericsql;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return genericsql;
}

ForeignDataWrapper *DatabaseModel::createForeignDataWrapper()
{
	attribs_map attribs;
	ForeignDataWrapper *fdw=nullptr;
	BaseObject *func=nullptr;
	QString signature, ref_type;
	ObjectType obj_type;

	try
	{
		fdw = new ForeignDataWrapper;

		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(fdw);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					obj_type = BaseObject::getObjectType(xmlparser.getElementName());

					if(obj_type == ObjectType::Function)
					{
						xmlparser.getElementAttributes(attribs);

						//Gets the function reference type
						ref_type = attribs[Attributes::RefType];

						//Try to retrieve one the functions of the fdw
						if(ref_type == Attributes::ValidatorFunc || ref_type == Attributes::HandlerFunc)
						{
							//Gets the function signature and tries to retrieve it from the model
							signature = attribs[Attributes::Signature];
							func = getObject(signature, ObjectType::Function);

							//Raises an error if the function doesn't exists
							if(!func)
								throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(fdw->getName())
												.arg(fdw->getTypeName())
												.arg(signature)
												.arg(BaseObject::getTypeName(ObjectType::Function)),
												ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

							if(ref_type == Attributes::ValidatorFunc)
								fdw->setValidatorFunction(dynamic_cast<Function *>(func));
							else if(ref_type == Attributes::HandlerFunc)
								fdw->setHandlerFunction(dynamic_cast<Function *>(func));
						}
						else
							//Raises an error if the function type is invalid
							throw Exception(ErrorCode::RefFunctionInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete fdw;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return fdw;
}

ForeignServer *DatabaseModel::createForeignServer()
{
	attribs_map attribs;
	ForeignServer *server = nullptr;
	BaseObject *fdw = nullptr;
	ObjectType obj_type;

	try
	{
		server = new ForeignServer;

		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(server);
		server->setType(attribs[Attributes::Type]);
		server->setVersion(attribs[Attributes::Version]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					obj_type = BaseObject::getObjectType(xmlparser.getElementName());

					if(obj_type == ObjectType::ForeignDataWrapper)
					{
						xmlparser.getElementAttributes(attribs);
						fdw = getObject(attribs[Attributes::Name], ObjectType::ForeignDataWrapper);

						//Raises an error if the fdw doesn't exists
						if(!fdw)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(server->getName())
															.arg(server->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::ForeignDataWrapper)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						server->setForeignDataWrapper(dynamic_cast<ForeignDataWrapper *>(fdw));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete server;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return server;
}

UserMapping *DatabaseModel::createUserMapping()
{
	attribs_map attribs;
	UserMapping *user_map = nullptr;
	ForeignServer *server = nullptr;
	ObjectType obj_type;

	try
	{
		user_map = new UserMapping;

		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(user_map);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					obj_type = BaseObject::getObjectType(xmlparser.getElementName());

					if(obj_type == ObjectType::ForeignServer)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs);
						server = dynamic_cast<ForeignServer *>(getObject(attribs[Attributes::Name], ObjectType::ForeignServer));

						//Raises an error if the server doesn't exists
						if(!server)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(user_map->getName())
															.arg(user_map->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::ForeignServer)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						user_map->setForeignServer(server);
						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete user_map;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return user_map;
}

ForeignTable *DatabaseModel::createForeignTable()
{
	ForeignTable *ftable = nullptr;

	try
	{
		ForeignServer *fserver = nullptr;
		attribs_map attribs;
		ObjectType obj_type;

		xmlparser.savePosition();
		ftable = createPhysicalTable<ForeignTable>();
		xmlparser.restorePosition();

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					obj_type = BaseObject::getObjectType(xmlparser.getElementName());

					if(obj_type == ObjectType::ForeignServer)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs);
						fserver = dynamic_cast<ForeignServer *>(getObject(attribs[Attributes::Name], ObjectType::ForeignServer));

						//Raises an error if the server doesn't exists
						if(!fserver)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(ftable->getName())
															.arg(ftable->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::ForeignServer)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						ftable->setForeignServer(fserver);
						xmlparser.restorePosition();
						break;
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		return ftable;
	}
	catch(Exception &e)
	{
		delete ftable;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Transform *DatabaseModel::createTransform()
{
	Transform *transf = nullptr;

	try
	{
		Function *func = nullptr;
		Language *lang = nullptr;
		attribs_map attribs;
		ObjectType obj_type;
		QString elem;

		transf = new Transform;
		xmlparser.savePosition();
		setBasicAttributes(transf);
		xmlparser.restorePosition();

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					elem = xmlparser.getElementName();
					obj_type = BaseObject::getObjectType(elem);

					if(elem == Attributes::Type)
					{
						transf->setType(createPgSQLType());
					}
					if(obj_type == ObjectType::Language)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs);
						lang = getLanguage(attribs[Attributes::Name]);

						if(!lang)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(transf->getName())
															.arg(transf->getTypeName())
															.arg(attribs[Attributes::Name])
															.arg(BaseObject::getTypeName(ObjectType::Language)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						transf->setLanguage(lang);
						xmlparser.restorePosition();
					}
					else if(obj_type == ObjectType::Function)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(attribs);
						func = getFunction(attribs[Attributes::Signature]);

						//Raises an error if the function doesn't exists
						if(!func)
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(transf->getName())
															.arg(transf->getTypeName())
															.arg(attribs[Attributes::Signature])
															.arg(BaseObject::getTypeName(ObjectType::Function)),
															ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

						if(attribs[Attributes::RefType] == Attributes::FromSqlFunc)
							transf->setFunction(func, Transform::FromSqlFunc);
						else
							transf->setFunction(func, Transform::ToSqlFunc);

						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		return transf;
	}
	catch(Exception &e)
	{
		delete transf;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Procedure *DatabaseModel::createProcedure()
{
	Procedure *proc = nullptr;

	try
	{
		proc = new Procedure;
		setBasicFunctionAttributes(proc);
	}
	catch(Exception &e)
	{
		delete proc;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return proc;
}

void DatabaseModel::updateViewsReferencingTable(PhysicalTable *table)
{
	BaseRelationship *rel = nullptr;
	View *view = nullptr;
	PhysicalTable *tab = nullptr;

	if(!table) return;

	for(auto obj : base_relationships)
	{
		rel = dynamic_cast<BaseRelationship *>(obj);

		if(rel->getRelationshipType() != BaseRelationship::RelationshipDep)
			continue;

		view = dynamic_cast<View *>(rel->getTable(BaseRelationship::SrcTable));
		tab = dynamic_cast<PhysicalTable *>(rel->getTable(BaseRelationship::DstTable));

		if(view && tab == table)
		{
			view->generateColumns();
			view->setCodeInvalidated(true);
			view->setModified(true);
			dynamic_cast<Schema *>(view->getSchema())->setModified(true);
		}
	}
}

Sequence *DatabaseModel::createSequence(bool ignore_onwer)
{
	attribs_map attribs;
	Sequence *sequence=nullptr;
	BaseObject *table=nullptr;
	Column *column=nullptr;
	QString str_aux, tab_name, col_name;
	QStringList elem_list;
	int count;

	try
	{
		sequence=new Sequence;
		setBasicAttributes(sequence);
		xmlparser.getElementAttributes(attribs);

		sequence->setValues(attribs[Attributes::MinValue],
				attribs[Attributes::MaxValue],
				attribs[Attributes::Increment],
				attribs[Attributes::Start],
				attribs[Attributes::Cache]);

		sequence->setCycle(attribs[Attributes::Cycle]==Attributes::True);

		//Getting the sequence's owner column
		if(!attribs[Attributes::OwnerColumn].isEmpty())
		{
			elem_list=attribs[Attributes::OwnerColumn].split('.');
			count=elem_list.count();

			if(count==3)
			{
				tab_name=elem_list[0] + "." + elem_list[1];
				col_name=elem_list[2];
			}
			else if(count==2)
			{
				tab_name=elem_list[0];
				col_name=elem_list[1];
			}

			table=getObject(tab_name, {ObjectType::Table, ObjectType::ForeignTable});

			//Raises an error if the column parent table doesn't exists
			if(!table)
			{
				str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
						.arg(sequence->getName())
						.arg(BaseObject::getTypeName(ObjectType::Sequence))
						.arg(tab_name)
						.arg(BaseObject::getTypeName(ObjectType::Table));

				throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			column=dynamic_cast<PhysicalTable *>(table)->getColumn(col_name);

			if(!column)
				column=dynamic_cast<PhysicalTable *>(table)->getColumn(col_name, true);

			//Raises an error if the column doesn't exists
			if(!column && !ignore_onwer)
				throw Exception(Exception::getErrorMessage(ErrorCode::AsgInexistentSeqOwnerColumn)
								.arg(sequence->getName(true)),
								ErrorCode::AsgInexistentSeqOwnerColumn,PGM_FUNC,PGM_FILE,PGM_LINE);

			sequence->setOwnerColumn(column);
		}
	}
	catch(Exception &e)
	{
		delete sequence;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return sequence;
}

View *DatabaseModel::createView()
{
	attribs_map attribs, aux_attribs;
	View *view = nullptr;
	BaseObject *tag = nullptr;
	std::vector<Reference> view_refs;
	std::vector<SimpleColumn> custom_cols;
	QString elem;

	try
	{
		view=new View;
		setBasicAttributes(view);

		xmlparser.getElementAttributes(attribs);
		view->setObjectListsCapacity(attribs[Attributes::MaxObjCount].toUInt());
		view->setMaterialized(attribs[Attributes::Materialized]==Attributes::True);
		view->setRecursive(attribs[Attributes::Recursive]==Attributes::True);
		view->setWithNoData(attribs[Attributes::WithNoData]==Attributes::True);
		view->setCheckOption(attribs[Attributes::CheckOption]);
		view->setSecurityBarrier(attribs[Attributes::SecurityBarrier] == Attributes::True);
		view->setSecurityInvoker(attribs[Attributes::SecurityInvoker] == Attributes::True);
		view->setCollapseMode(attribs[Attributes::CollapseMode].isEmpty() ? BaseTable::NotCollapsed : static_cast<BaseTable::CollapseMode>(attribs[Attributes::CollapseMode].toUInt()));
		view->setPaginationEnabled(attribs[Attributes::Pagination]==Attributes::True);
		view->setCurrentPage(BaseTable::AttribsSection, attribs[Attributes::AttribsPage].toUInt());
		view->setCurrentPage(BaseTable::ExtAttribsSection, attribs[Attributes::ExtAttribsPage].toUInt());
		view->setFadedOut(attribs[Attributes::FadedOut]==Attributes::True);
		view->setLayers(attribs[Attributes::Layers].split(','));

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					elem = xmlparser.getElementName();

					if(elem == Attributes::Definition)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						view->setSqlDefinition(xmlparser.getElementContent());
						xmlparser.restorePosition();
					}
					else if(elem == Attributes::SimpleCol)
					{
						xmlparser.getElementAttributes(attribs);
						custom_cols.push_back(SimpleColumn(attribs[Attributes::Name],
																							 attribs[Attributes::Type],
																							 attribs[Attributes::Alias]));
					}
					else if(elem == Attributes::Reference)
					{
						BaseObject *ref_object = nullptr;
						ObjectType ref_obj_type;

						xmlparser.getElementAttributes(attribs);

						ref_obj_type = BaseObject::getObjectType(attribs[Attributes::Type]);

						if(ref_obj_type == ObjectType::Column)
						{
							QStringList name_list = attribs[Attributes::Object].split('.');
							PhysicalTable *tab = nullptr;

							if(name_list.size() == 3)
							{
								QString tab_name = QString("%1.%2").arg(name_list[0], name_list[1]);
								tab = dynamic_cast<PhysicalTable *>(getObject(tab_name, { ObjectType::Table, ObjectType::ForeignTable }));

								if(tab)
									ref_object = tab->getObject(name_list[2], ref_obj_type);
							}
						}
						else
							ref_object = getObject(attribs[Attributes::Object], ref_obj_type);

						if(!ref_object)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(view->getSignature(),
																	 BaseObject::getTypeName(ObjectType::View),
																	 attribs[Attributes::Object],
																	 BaseObject::getTypeName(ref_obj_type)),
															 ErrorCode::RefObjectInexistsModel, PGM_FUNC, PGM_FILE, PGM_LINE);
						}

						view_refs.push_back(Reference(ref_object,
																											attribs[Attributes::RefName],
																											attribs[Attributes::RefAlias],
																											attribs[Attributes::UseSignature] == Attributes::True,
																											attribs[Attributes::FormatName] == Attributes::True,
																											attribs[Attributes::UseColumns] == Attributes::True));
					}
					else if(elem == BaseObject::getSchemaName(ObjectType::Tag))
					{
						xmlparser.getElementAttributes(aux_attribs);
						tag=getObject(aux_attribs[Attributes::Name] ,ObjectType::Tag);

						if(!tag)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
										.arg(attribs[Attributes::Name],
												 BaseObject::getTypeName(ObjectType::Table),
												 aux_attribs[Attributes::Table],
												BaseObject::getTypeName(ObjectType::Tag))
									, ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						view->setTag(dynamic_cast<Tag *>(tag));
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		view->setReferences(view_refs);
		view->setCustomColumns(custom_cols);
	}
	catch(Exception &e)
	{
		delete view;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return view;
}

Collation *DatabaseModel::createCollation()
{
	Collation *collation=nullptr;
	BaseObject *copy_coll=nullptr;
	attribs_map attribs;

	try
	{
		collation=new Collation;
		setBasicAttributes(collation);

		xmlparser.getElementAttributes(attribs);

		collation->setEncoding(EncodingType(attribs[Attributes::Encoding]));
		collation->setProvider(ProviderType(attribs[Attributes::Provider]));
		collation->setDeterministic(attribs[Attributes::Deterministic] == Attributes::True);

		//Creating a collation from a base locale
		if(!attribs[Attributes::Locale].isEmpty())
		{
			collation->setLocale(attribs[Attributes::Locale]);
			collation->setModifier(Collation::Locale, attribs[Attributes::LocaleMod]);
		}
		//Creating a collation from another collation
		else if(!attribs[Attributes::Collation].isEmpty())
		{
			copy_coll=this->getObject(attribs[Attributes::Collation], ObjectType::Collation);

			//Raises an error if the copy collation doesn't exists
			if(!copy_coll)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
								.arg(collation->getName())
								.arg(BaseObject::getTypeName(ObjectType::Collation))
								.arg(attribs[Attributes::Collation])
						.arg(BaseObject::getTypeName(ObjectType::Collation)),
						ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			collation->setCollation(dynamic_cast<Collation *>(copy_coll));
		}
		//Creating a collation using LC_COLLATE and LC_CTYPE params
		else
		{
			collation->setLocalization(Collation::LcCollate, attribs[Attributes::LcCollate]);
			collation->setLocalization(Collation::LcCtype, attribs[Attributes::LcCtype]);
			collation->setModifier(Collation::LcCtype, attribs[Attributes::LcCtypeMod]);
			collation->setModifier(Collation::LcCollate, attribs[Attributes::LcCollateMod]);
		}
	}
	catch(Exception &e)
	{
		delete collation;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return collation;
}

Extension *DatabaseModel::createExtension()
{
	Extension *extension=nullptr;
	attribs_map attribs;

	try
	{
		extension = new Extension;
		xmlparser.getElementAttributes(attribs);
		setBasicAttributes(extension);

		extension->setVersion(Extension::CurVersion, attribs[Attributes::CurVersion]);
		extension->setVersion(Extension::OldVersion, attribs[Attributes::OldVersion]);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			attribs.clear();

			do
			{
				if(xmlparser.getElementType() == XML_ELEMENT_NODE)
				{
					if(xmlparser.getElementName() == Attributes::Object)
					{
						xmlparser.getElementAttributes(attribs);
						extension->addObject(Extension::ExtObject { attribs[Attributes::Name],
																												BaseObject::getObjectType(attribs[Attributes::Type]),
																												attribs[Attributes::Parent] });
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		delete extension;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return extension;
}

Tag *DatabaseModel::createTag()
{
	Tag *tag=nullptr;
	attribs_map attribs;
	QString elem;

	try
	{
		tag=new Tag;
		setBasicAttributes(tag);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem==Attributes::Style)
					{
						xmlparser.getElementAttributes(attribs);
						tag->setElementColors(attribs[Attributes::Id],attribs[Attributes::Colors]);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		return tag;
	}
	catch(Exception &e)
	{
		delete tag;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}
}

Textbox *DatabaseModel::createTextbox()
{
	Textbox *txtbox=nullptr;
	attribs_map attribs;

	try
	{
		txtbox=new Textbox;
		setBasicAttributes(txtbox);

		xmlparser.getElementAttributes(attribs);

		txtbox->setFadedOut(attribs[Attributes::FadedOut]==Attributes::True);
		txtbox->setLayers(attribs[Attributes::Layers].split(','));
		txtbox->setTextAttribute(Textbox::ItalicText, attribs[Attributes::Italic]==Attributes::True);
		txtbox->setTextAttribute(Textbox::BoldText, attribs[Attributes::Bold]==Attributes::True);
		txtbox->setTextAttribute(Textbox::UnderlineText, attribs[Attributes::Underline]==Attributes::True);

		txtbox->setTextWidth(attribs[Attributes::Width].toDouble());

		if(!attribs[Attributes::Color].isEmpty())
			txtbox->setTextColor(QColor(attribs[Attributes::Color]));

		if(!attribs[Attributes::FontSize].isEmpty())
			txtbox->setFontSize(attribs[Attributes::FontSize].toDouble());
	}
	catch(Exception &e)
	{
		delete txtbox;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return txtbox;
}

BaseRelationship *DatabaseModel::createRelationship()
{
	std::vector<unsigned> cols_special_pk;
	attribs_map attribs, constr_attribs;
	BaseRelationship *base_rel=nullptr;
	Relationship *rel=nullptr;
	BaseTable *tables[2]={nullptr, nullptr};
	bool src_mand, dst_mand, identifier, protect, deferrable, sql_disabled, single_pk_col, faded_out;
	DeferralType defer_type;
	ActionType del_action, upd_action;
	BaseRelationship::RelType rel_type;
	unsigned i = 0;
	QStringList layers;
	ObjectType table_types[2]={ ObjectType::View, ObjectType::Table }, obj_rel_type;
	QString str_aux, elem, tab_attribs[2]={ Attributes::SrcTable, Attributes::DstTable };
	QColor custom_color=Qt::transparent;
	Table *table = nullptr;
	std::map<QString, BaseRelationship::LabelId> 	labels_id= {{ Attributes::NameLabel, BaseRelationship::RelNameLabel },
																														{ Attributes::SrcLabel, BaseRelationship::SrcCardLabel },
																														{ Attributes::DstLabel, BaseRelationship::DstCardLabel }};

	try
	{
		xmlparser.getElementAttributes(attribs);

		src_mand=attribs[Attributes::SrcRequired]==Attributes::True;
		dst_mand=attribs[Attributes::DstRequired]==Attributes::True;
		protect=(attribs[Attributes::Protected]==Attributes::True);
		faded_out=(attribs[Attributes::FadedOut]==Attributes::True);
		layers = attribs[Attributes::Layers].split(',');

		if(!attribs[Attributes::CustomColor].isEmpty())
			custom_color=QColor(attribs[Attributes::CustomColor]);

		if(attribs[Attributes::Type]!=Attributes::RelationshipTabView &&
				attribs[Attributes::Type]!=Attributes::RelationshipFk)
		{
			table_types[0]=ObjectType::Table;
			obj_rel_type=ObjectType::Relationship;
		}
		else
		{
			if(attribs[Attributes::Type]==Attributes::RelationshipFk)
				table_types[0]=ObjectType::Table;

			obj_rel_type=ObjectType::BaseRelationship;
		}

		//Gets the participant tables
		for(i=0; i < 2; i++)
		{
			tables[i]=dynamic_cast<BaseTable *>(getObject(attribs[tab_attribs[i]], table_types[i]));

			/* If the relationship is between a view and a table and the table is not found
			 * we try to find a foreign table instead */
			if(table_types[i] == ObjectType::Table && !tables[i])
			{
				tables[i]=dynamic_cast<BaseTable *>(getObject(attribs[tab_attribs[i]], ObjectType::ForeignTable));

				// In case of table-view relationship, as a last resort, we try to find a view matching the table name
				if(!tables[i] && attribs[Attributes::Type] == Attributes::RelationshipTabView)
					tables[i]=dynamic_cast<BaseTable *>(getObject(attribs[tab_attribs[i]], ObjectType::View));
			}

			//Raises an error if some table doesn't exists
			if(!tables[i])
			{
				str_aux=Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
						.arg(attribs[Attributes::Name])
						.arg(BaseObject::getTypeName(obj_rel_type))
						.arg(attribs[tab_attribs[i]])
						.arg(BaseObject::getTypeName(table_types[i]));

				throw Exception(str_aux,ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		if(obj_rel_type==ObjectType::BaseRelationship)
		{
			base_rel=getRelationship(tables[0], tables[1]);

			/* Creates the fk relationship if it not exists. This generally happens when a foreign key is
			added to the table after its creation. */
			if(attribs[Attributes::Type]==Attributes::RelationshipFk)
			{
				base_rel=new BaseRelationship(BaseRelationship::RelationshipFk, tables[0], tables[1], src_mand, dst_mand);
				base_rel->setName(attribs[Attributes::Name]);
				base_rel->setAlias(attribs[Attributes::Alias]);
				addRelationship(base_rel);

				/* Workaround: in very specific cases when the FK relationship is generated from a FK that references
				 * columns created by a relationship in the referenced table, we have to save the layers information
				 * of that relatinship because that one can be destroyed and recreated at the end of the loading process
				 * leading to the loss of the original layer configuration. This way, we store the layers in a special
				 * map which is read at DatabaseModel::restoreFKRelationshipLayer (called at the end of DatabaseModel::loadModel) */
				if(loading_model)
					fk_rel_layers[attribs[Attributes::Name]] = layers;

				/* If the source table doesn't have any fk that references the destination table indicates that the relationship
				is being created before the fk that represents it or the fk is invalid (inconsistence!). In this case an error is raised. */
				if(base_rel->getRelationshipType() == BaseRelationship::RelationshipFk &&
				   !base_rel->getReferenceForeignKey())
				{
					Table *src_tab = dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::SrcTable)),
							*dst_tab = dynamic_cast<Table *>(base_rel->getTable(BaseRelationship::DstTable));
					std::vector<Constraint *> fks;

					src_tab->getForeignKeys(fks, false, dst_tab);

					for(auto fk : fks)
					{
						if(!getRelationship(src_tab, dst_tab, fk))
						{
							base_rel->setReferenceForeignKey(fk);
							break;
						}
					}

					//Throws an error if the relationship was created without a valid foreign key attached to it
					if(!base_rel->getReferenceForeignKey())
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::InvAllocationFKRelationship)
										.arg(attribs[Attributes::Name])
									  .arg(src_tab->getName(true)),
										ErrorCode::InvAllocationFKRelationship,PGM_FUNC,PGM_FILE,PGM_LINE);
					}
				}
			}
			else if(base_rel)
				base_rel->setName(attribs[Attributes::Name]);

			if(!base_rel)
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
								.arg(this->getName())
								.arg(this->getTypeName())
								.arg(attribs[Attributes::Name])
					.arg(BaseObject::getTypeName(ObjectType::BaseRelationship)),
					ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

			base_rel->blockSignals(loading_model);
			base_rel->disconnectRelationship();
			base_rel->blockSignals(false);
		}
		else
		{
			QString pat_attrib[]= {
				Attributes::SrcColPattern, Attributes::DstColPattern,
				Attributes::SrcFkPattern, Attributes::DstFkPattern,
				Attributes::PkPattern, Attributes::UqPattern,
				Attributes::PkColPattern, Attributes::FkIdxPattern
			};

			std::vector<Relationship::PatternId>	pattern_ids= {
				Relationship::SrcColPattern, Relationship::DstColPattern,
				Relationship::SrcFkPattern, Relationship::DstFkPattern,
				Relationship::PkPattern, Relationship::UqPattern,
				Relationship::PkColPattern, Relationship::FkIdxPattern
			};

			sql_disabled=attribs[Attributes::SqlDisabled]==Attributes::True;
			identifier=attribs[Attributes::Identifier]==Attributes::True;
			deferrable=attribs[Attributes::Deferrable]==Attributes::True;
			defer_type=DeferralType(attribs[Attributes::DeferType]);
			del_action=ActionType(attribs[Attributes::DelAction]);
			upd_action=ActionType(attribs[Attributes::UpdAction]);
			single_pk_col=(attribs[Attributes::SinglePkColumn]==Attributes::True);

			if(attribs[Attributes::Type]==Attributes::Relationship11)
				rel_type=BaseRelationship::Relationship11;
			else if(attribs[Attributes::Type]==Attributes::Relationship1n)
				rel_type=BaseRelationship::Relationship1n;
			else if(attribs[Attributes::Type]==Attributes::RelationshipNn)
				rel_type=BaseRelationship::RelationshipNn;
			else if(attribs[Attributes::Type]==Attributes::RelationshipGen)
				rel_type=BaseRelationship::RelationshipGen;
			else if(attribs[Attributes::Type]==Attributes::RelationshipDep)
				rel_type=BaseRelationship::RelationshipDep;
			else
				rel_type=BaseRelationship::RelationshipPart;

			rel=new Relationship(rel_type,
													 dynamic_cast<PhysicalTable *>(tables[0]),
													 dynamic_cast<PhysicalTable *>(tables[1]),
													 src_mand, dst_mand, identifier);

			rel->setActionType(upd_action, Constraint::UpdateAction);
			rel->setActionType(del_action, Constraint::DeleteAction);
			rel->setSQLDisabled(sql_disabled);
			rel->setSiglePKColumn(single_pk_col);
			rel->setDeferrable(deferrable);
			rel->setDeferralType(defer_type);
			rel->setCopyOptions(CopyOptions(static_cast<CopyOptions::CopyMode>(attribs[Attributes::CopyMode].toUInt()),
																			static_cast<CopyOptions::CopyOpts>(attribs[Attributes::CopyOptions].toUInt())));
			rel->setFKIndexType(IndexingType(attribs[Attributes::FkIdxType]));

			if(!attribs[Attributes::TableName].isEmpty())
				rel->setTableNameRelNN(attribs[Attributes::TableName]);

			rel->setName(attribs[Attributes::Name]);
			rel->setAlias(attribs[Attributes::Alias]);
			base_rel = rel;

			//Configuring the name patterns
			for(auto &pat_id : pattern_ids)
				rel->setNamePattern(pat_id, attribs[pat_attrib[pat_id]]);
		}

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();

					if(elem == Attributes::Expression && rel)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						rel->setPartitionBoundingExpr(xmlparser.getElementContent());
						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Column && rel)
					{
						xmlparser.savePosition();
						rel->addObject(createColumn());
						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Constraint && rel)
					{
						xmlparser.savePosition();
						xmlparser.getElementAttributes(constr_attribs);

						/* If we find a primary key constraint at this point means that we're handling the original primary key stored by the relationship.
						 * Since relationships can't have primary keys created manually by the users we assume that
						 * the relationship contains a special primary key (created during relationship connection)
						 * and the current constraint is the original one owned by one of the tables prior the connection
						 * of the relationship. */
						if(constr_attribs[Attributes::Type] == Attributes::PkConstr)
						{
							table = getTable(constr_attribs[Attributes::Table]);
							rel->setOriginalPrimaryKey(createConstraint(table));
						}
						else
							rel->addObject(createConstraint(rel));

						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Line)
					{
						std::vector<QPointF> points;
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						do
						{
							xmlparser.getElementAttributes(attribs);
							points.push_back(QPointF(attribs[Attributes::XPos].toDouble(),
											 attribs[Attributes::YPos].toDouble()));
						}
						while(xmlparser.accessElement(XmlParser::NextElement));

						base_rel->setPoints(points);
						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Label)
					{
						xmlparser.getElementAttributes(attribs);
						str_aux=attribs[Attributes::RefType];

						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						xmlparser.getElementAttributes(attribs);
						xmlparser.restorePosition();

						base_rel->setLabelDistance(labels_id[str_aux],
													 QPointF(attribs[Attributes::XPos].toDouble(),
													 attribs[Attributes::YPos].toDouble()));
					}
					else if(elem==Attributes::SpecialPkCols && rel)
					{
						QList<QString> col_list;

						xmlparser.getElementAttributes(attribs);
						col_list=attribs[Attributes::Indexes].split(',');

						while(!col_list.isEmpty())
						{
							cols_special_pk.push_back(col_list.front().toUInt());
							col_list.pop_front();
						}

						rel->setSpecialPrimaryKeyCols(cols_special_pk);
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		if(base_rel && base_rel->getObjectType()==ObjectType::Relationship)
			delete base_rel;

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	if(rel)
	{
		storeSpecialObjectsXML();
		addRelationship(rel);
	}

	base_rel->setFadedOut(faded_out);
	base_rel->setProtected(protect);
	base_rel->setCustomColor(custom_color);
	base_rel->setLayers(layers);

	/* If the FK relationship does not reference a foreign key (models generated in older versions)
	 * we need to assign them to the respective relationships */
	if(base_rel && base_rel->getObjectType()==ObjectType::BaseRelationship)
	{
		base_rel->blockSignals(loading_model);
		base_rel->connectRelationship();
		base_rel->blockSignals(false);
	}

	return base_rel;
}

Permission *DatabaseModel::createPermission()
{
	Permission *perm=nullptr;
	BaseObject *object=nullptr;
	PhysicalTable *parent_table=nullptr;
	Role *role=nullptr;
	attribs_map priv_attribs, attribs;
	attribs_map::iterator itr, itr_end;
	ObjectType obj_type;
	QString parent_name, obj_name;
	QStringList list;
	bool priv_value, grant_op, revoke, cascade;
	std::map<QString, Permission::PrivilegeId> priv_ids = {
		{ Attributes::ConnectPriv, Permission::PrivConnect },
		{ Attributes::CreatePriv, Permission::PrivCreate },
		{ Attributes::DeletePriv, Permission::PrivDelete },
		{ Attributes::ExecutPriv, Permission::PrivExecute },
		{ Attributes::InsertPriv, Permission::PrivInsert },
		{ Attributes::ReferencesPriv, Permission::PrivReferences },
		{ Attributes::SelectPriv, Permission::PrivSelect },
		{ Attributes::TemporaryPriv, Permission::PrivTemporary },
		{ Attributes::TriggerPriv, Permission::PrivTrigger },
		{ Attributes::TruncatePriv, Permission::PrivTruncate },
		{ Attributes::UpdatePriv, Permission::PrivUpdate },
		{ Attributes::UsagePriv, Permission::PrivUsage }
	};

	try
	{
		xmlparser.getElementAttributes(priv_attribs);
		revoke=priv_attribs[Attributes::Revoke]==Attributes::True;
		cascade=priv_attribs[Attributes::Cascade]==Attributes::True;

		xmlparser.savePosition();
		xmlparser.accessElement(XmlParser::ChildElement);
		xmlparser.getElementAttributes(attribs);

		obj_type=BaseObject::getObjectType(attribs[Attributes::Type]);
		obj_name=attribs[Attributes::Name];
		parent_name=attribs[Attributes::Parent];

		//If the object is a column its needed to get the parent table
		if(obj_type==ObjectType::Column)
		{
			parent_table=dynamic_cast<PhysicalTable *>(getObject(parent_name, {ObjectType::Table, ObjectType::ForeignTable}));

			if(parent_table)
				object=parent_table->getColumn(obj_name);
		}
		else if(obj_type==ObjectType::Database)
		{
			object=this;
		}
		else
			object=getObject(obj_name, obj_type);

		//Raises an error if the permission references an object that does not exists
		if(!object)
			throw Exception(Exception::getErrorMessage(ErrorCode::PermissionRefInexistObject)
							.arg(obj_name)
							.arg(BaseObject::getTypeName(obj_type)),
							ErrorCode::PermissionRefInexistObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		perm=new Permission(object);
		perm->setRevoke(revoke);
		perm->setCascade(cascade);

		do
		{
			if(xmlparser.getElementName()==Attributes::Roles)
			{
				xmlparser.getElementAttributes(attribs);
				list = attribs[Attributes::Names].split(',');

				for(auto &name : list)
				{
					role=dynamic_cast<Role *>(getObject(name.trimmed(), ObjectType::Role));

					//Raises an error if the referenced role doesn't exists
					if(!role)
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::PermissionRefInexistObject)
										.arg(name)
										.arg(BaseObject::getTypeName(ObjectType::Role)),
										ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);

					}

					perm->addRole(role);
				}
			}
			else if(xmlparser.getElementName()==Attributes::Privileges)
			{
				xmlparser.getElementAttributes(priv_attribs);

				itr=priv_attribs.begin();
				itr_end=priv_attribs.end();

				while(itr!=itr_end)
				{
					if(itr->first!=Attributes::GrantOp)
					{
						priv_value=(itr->second==Attributes::True);
						grant_op=(itr->second==Attributes::GrantOp);

						/*if(itr->first==Attributes::ConnectPriv)
							priv_type=Permission::PrivConnect;
						else if(itr->first==Attributes::CreatePriv)
							priv_type=Permission::PrivCreate;
						else if(itr->first==Attributes::DeletePriv)
							priv_type=Permission::PrivDelete;
						else if(itr->first==Attributes::ExecutPriv)
							priv_type=Permission::PrivExecute;
						else if(itr->first==Attributes::InsertPriv)
							priv_type=Permission::PrivInsert;
						else if(itr->first==Attributes::ReferencesPriv)
							priv_type=Permission::PrivReferences;
						else if(itr->first==Attributes::SelectPriv)
							priv_type=Permission::PrivSelect;
						else if(itr->first==Attributes::TemporaryPriv)
							priv_type=Permission::PrivTemporary;
						else if(itr->first==Attributes::TriggerPriv)
							priv_type=Permission::PrivTrigger;
						else if(itr->first==Attributes::TruncatePriv)
							priv_type=Permission::PrivTruncate;
						else if(itr->first==Attributes::UpdatePriv)
							priv_type=Permission::PrivUpdate;
						else if(itr->first==Attributes::UsagePriv)
							priv_type=Permission::PrivUsage;

						perm->setPrivilege(priv_type, (priv_value || grant_op), grant_op); */

						perm->setPrivilege(priv_ids[itr->first], (priv_value || grant_op), grant_op);
					}
					itr++;
				}
			}
		}
		while(xmlparser.accessElement(XmlParser::NextElement));

		xmlparser.restorePosition();
	}
	catch(Exception &e)
	{
		delete perm;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, getErrorExtraInfo());
	}

	return perm;
}

void DatabaseModel::validateRelationships(TableObject *object, Table *parent_tab)
{
	try
	{
		bool revalidate_rels=false, ref_tab_inheritance=false;
		Relationship *rel=nullptr;
		std::vector<BaseObject *>::iterator itr, itr_end;
		ObjectType obj_type;

		if(object && parent_tab)
		{
			obj_type=object->getObjectType();

			/* Relationship validation condition:
			> Case the object is a column and its reference by the parent table primary key
			> Case the parent table is a partition and a column is being removed
			> Case the object is a constraint and its a table primary key */
			revalidate_rels=((obj_type==ObjectType::Column &&
												(parent_tab->isConstraintRefColumn(dynamic_cast<Column *>(object), ConstraintType::PrimaryKey) ||
												 parent_tab->isPartition() || parent_tab->isPartitioned())) ||
											 (obj_type==ObjectType::Constraint &&
												dynamic_cast<Constraint *>(object)->getConstraintType()==ConstraintType::PrimaryKey));

			/* Additional validation for columns: checks if the parent table participates on a
			generalization/copy as destination table */
			if(obj_type==ObjectType::Column)
			{
				itr=relationships.begin();
				itr_end=relationships.end();

				while(itr!=itr_end && !ref_tab_inheritance)
				{
					rel=dynamic_cast<Relationship *>(*itr);
					itr++;
					ref_tab_inheritance=(rel->getRelationshipType()==Relationship::RelationshipGen &&
															 rel->getReferenceTable()==parent_tab);
				}
			}

			if(revalidate_rels || ref_tab_inheritance)
			{
				storeSpecialObjectsXML();
				disconnectRelationships();
				validateRelationships();
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString DatabaseModel::__getSourceCode(SchemaParser::CodeType def_type)
{
	QString def, bkp_appended_sql, bkp_prepended_sql;

	//Forcing the name/signature cleanup due to the validation temp. names feature
	attributes[Attributes::Name]="";
	attributes[Attributes::Signature]="";

	if(conn_limit >= 0)
		attributes[Attributes::ConnLimit]=QString("%1").arg(conn_limit);

	if(def_type==SchemaParser::SqlCode)
	{
		QString loc_attribs[]={ Attributes::LcCtype,  Attributes::LcCollate };

		if(encoding!=EncodingType::Null)
			attributes[Attributes::Encoding]=QString("'%1'").arg(~encoding);

		for(unsigned i=0; i < 2; i++)
		{
			if(!localizations[i].isEmpty())
			{
				attributes[loc_attribs[i]]=QString("'%1'").arg(localizations[i]);
			}
		}
	}
	else
	{
		attributes[Attributes::Encoding]=(~encoding);
		attributes[Attributes::LcCollate]=localizations[1];
		attributes[Attributes::LcCtype]=localizations[0];
		attributes[Attributes::AppendAtEod]=(append_at_eod ? Attributes::True : "");
		attributes[Attributes::PrependAtBod]=(prepend_at_bod ? Attributes::True : "");
	}

	attributes[Attributes::IsTemplate]=(is_template ? Attributes::True : Attributes::False);
	attributes[Attributes::AllowConns]=(allow_conns ? Attributes::True : Attributes::False);
	attributes[Attributes::TemplateDb]=template_db;

	if(def_type==SchemaParser::SqlCode && append_at_eod)
	{
		bkp_appended_sql=this->appended_sql;
		this->appended_sql.clear();
	}

	if(def_type==SchemaParser::SqlCode && prepend_at_bod)
	{
		bkp_prepended_sql=this->prepended_sql;
		this->prepended_sql.clear();
	}

	try
	{
		def=this->BaseObject::__getSourceCode(def_type);

		if(def_type==SchemaParser::SqlCode && append_at_eod)
			this->appended_sql=bkp_appended_sql;

		if(def_type==SchemaParser::SqlCode && prepend_at_bod)
			this->prepended_sql=bkp_prepended_sql;

		return def;
	}
	catch(Exception &e)
	{
		this->appended_sql=bkp_appended_sql;
		this->prepended_sql=bkp_prepended_sql;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString DatabaseModel::getSQLDefinition(const std::vector<BaseObject *> objects, CodeGenMode code_gen_mode)
{
	if(objects.empty())
		return "";

	try
	{
		std::map<unsigned, BaseObject *> objs_map;

		// Putting the object in the proper cration order
		std::for_each(objects.begin(), objects.end(), [&objs_map](auto obj){
			objs_map[obj->getObjectId()] = obj;
		});

		/* If we are getting dependencies or children SQL we also need to
		 * put them object in the proper cration order */
		if(code_gen_mode == DependenciesSql || code_gen_mode == ChildrenSql)
		{
			std::for_each(objects.begin(), objects.end(), [&objs_map, this, code_gen_mode](auto obj){
				 for(auto &dep_obj : getCreationOrder(obj, code_gen_mode == ChildrenSql))
					 objs_map[dep_obj->getObjectId()] = dep_obj;
			});
		}

		QString code;
		ObjectType obj_type;

		for(auto &[_, obj] : objs_map)
		{
			obj_type = obj->getObjectType();

			if((obj->isSQLDisabled() && !gen_dis_objs_code) ||
				 obj_type == ObjectType::Textbox || obj_type == ObjectType::Tag ||
				 (obj_type == ObjectType::BaseRelationship &&
					dynamic_cast<BaseRelationship *>(obj)->getRelationshipType() !=
					BaseRelationship::RelationshipFk))
				continue;

			if(obj->getObjectType() == ObjectType::Database)
				code += dynamic_cast<DatabaseModel *>(obj)->__getSourceCode(SchemaParser::SqlCode);
			else
				code += obj->getSourceCode(SchemaParser::SqlCode);
		}

		if(!code.isEmpty() &&
			 (code_gen_mode == DependenciesSql || code_gen_mode == ChildrenSql))
		{
			code.prepend(tr("-- NOTE: the code below contains the SQL for the object itself\n\
-- as well as for its dependencies or children (if applicable).\n\
-- \n\
-- This feature is only a convenience in order to allow you to test\n\
-- the whole object's SQL definition at once.\n\
-- \n\
-- When exporting or generating the SQL for the whole database model\n\
-- all objects will be placed at their original positions.\n\n\n"));
		}

		return code;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString DatabaseModel::configureShellTypes(bool reset_config)
{
	QString shell_types_def;
	Type *usr_type = nullptr;

	for(auto &type : types)
	{
		usr_type=dynamic_cast<Type *>(type);

		if(usr_type->getConfiguration()==Type::BaseType)
		{
			usr_type->convertFunctionParameters(!reset_config);

			//Generating the shell type declaration (only for base types)
			if(!reset_config)
				shell_types_def += usr_type->getSourceCode(SchemaParser::SqlCode, true);

			/* Forcing the code invalidation for the type so the complete definition can be
			 * generated in the below iteration */
			usr_type->setCodeInvalidated(true);
		}
	}

	return shell_types_def;
}

QString DatabaseModel::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, true);
}

QString DatabaseModel::getSourceCode(SchemaParser::CodeType def_type, bool export_file)
{
	bool is_sql_def = (def_type == SchemaParser::SqlCode);
	attribs_map attribs_aux;
	unsigned general_obj_cnt, gen_defs_count;
	BaseObject *object=nullptr;
	QString def, search_path="pg_catalog,public",
			msg=tr("Generating %1 code: `%2' (%3)"),
			attrib=Attributes::Objects, attrib_aux,
			def_type_str =(is_sql_def ? "SQL" : "XML");
	Type *usr_type=nullptr;
	std::map<unsigned, BaseObject *> objects_map;
	ObjectType obj_type;

	try
	{
		cancel_saving = false;
		objects_map = getCreationOrder(def_type);
		general_obj_cnt = objects_map.size();
		gen_defs_count=0;

		attribs_aux[Attributes::ShellTypes]="";
		attribs_aux[Attributes::Permission]="";
		attribs_aux[Attributes::Schema]="";
		attribs_aux[Attributes::Tablespace]="";
		attribs_aux[Attributes::Role]="";
		attribs_aux[Attributes::Objects]="";
		attribs_aux[getSchemaName()] = "";

		if(is_sql_def)
		{
			attribs_aux[Attributes::Function] = (!functions.empty() ? Attributes::True : "");
			attribs_aux[Attributes::ShellTypes] = configureShellTypes(false);
		}

		setDatabaseModelAttributes(attribs_aux, def_type);

		for(auto &obj_itr : objects_map)
		{
			if(cancel_saving)
				return "";

			object = obj_itr.second;
			obj_type = object->getObjectType();

			/* Ignoring disabled SQL code if the flag to include this kind
			 * of code is off. This will diminish the size of the resulting
			 * script */
			if(is_sql_def && object->isSQLDisabled() && !gen_dis_objs_code)
				continue;

			if(obj_type == ObjectType::Type && is_sql_def)
			{
				usr_type = dynamic_cast<Type *>(object);
				attribs_aux[attrib] += usr_type->getSourceCode(def_type);
			}
			else if(obj_type == ObjectType::Database)
			{
				if(is_sql_def)
					attribs_aux[this->getSchemaName()] += this->__getSourceCode(def_type);
				else
					attribs_aux[attrib] += this->__getSourceCode(def_type);
			}
			else if(obj_type == ObjectType::Permission)
			{
				attribs_aux[Attributes::Permission] += dynamic_cast<Permission *>(object)->getSourceCode(def_type);
			}
			else if(obj_type == ObjectType::Constraint)
			{
				attribs_aux[attrib] += dynamic_cast<Constraint *>(object)->getSourceCode(def_type, true);
			}
			else if(obj_type == ObjectType::Role || obj_type == ObjectType::Tablespace || obj_type == ObjectType::Schema)
			{
				//The "public" schema does not have the SQL code definition generated
				if(is_sql_def)
					attrib_aux = BaseObject::getSchemaName(obj_type);
				else
					attrib_aux = attrib;

				/* The Tablespace has the SQL code definition disabled when generating the
				 * code of the entire model because this object cannot be created from a multiline sql command */
				if(obj_type == ObjectType::Tablespace && !object->isSystemObject() && is_sql_def)
					attribs_aux[attrib_aux] += object->getSourceCode(def_type);

				//System object doesn't has the XML generated (the only exception is for public schema)
				else if((obj_type != ObjectType::Schema && !object->isSystemObject()) ||
								(obj_type == ObjectType::Schema &&
								 ((object->getName() == "public" && !is_sql_def) ||
									 (object->getName() != "public" && object->getName() != "pg_catalog"))))
				{
					if(object->getObjectType() == ObjectType::Schema)
						search_path += "," + object->getName(true);

					// Avoiding writting the code definition for system objects when generating SQL code
					if((is_sql_def && !object->isSystemObject()) ||
						 (!is_sql_def && (!object->isSystemObject() || object->getName() == "public")))
					{
						//Generates the code definition and concatenates to the others
						attribs_aux[attrib_aux] += object->getSourceCode(def_type);
					}
				}
			}
			else
			{
				if(object->isSystemObject())
					attribs_aux[attrib] += "";
				else
					attribs_aux[attrib] += object->getSourceCode(def_type);
			}

			gen_defs_count++;

			if((is_sql_def && !object->isSQLDisabled()) ||
					(!is_sql_def && !object->isSystemObject()))
			{
				emit s_objectLoaded((gen_defs_count/static_cast<double>(general_obj_cnt)) * 100,
									msg.arg(def_type_str)
									.arg(object->getName())
									.arg(object->getTypeName()),
									enum_t(object->getObjectType()));
			}
		}

		attribs_aux[Attributes::SearchPath]=search_path;

		if(is_sql_def)
			configureShellTypes(true);

		attribs_aux[Attributes::ExportToFile] = (export_file ? Attributes::True : "");
		def = schparser.getSourceCode(Attributes::DbModel, attribs_aux, def_type);

		if(prepend_at_bod && is_sql_def)
			def="-- Prepended SQL commands --\n" + this->prepended_sql + Attributes::DdlEndToken + def;

		if(append_at_eod && is_sql_def)
			def+="-- Appended SQL commands --\n" + this->appended_sql + QChar('\n') + Attributes::DdlEndToken;
	}
	catch(Exception &e)
	{
		if(is_sql_def)
			configureShellTypes(true);

		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}


	return def;
}

void DatabaseModel::setDatabaseModelAttributes(attribs_map &attribs, SchemaParser::CodeType code_type)
{
	attribs[Attributes::Author] = author;
	attribs[Attributes::PgModelerVersion] = GlobalAttributes::PgModelerVersion;

	if(code_type == SchemaParser::XmlCode)
	{
		try
		{
			//Configuring the changelog attributes when generating XML code
			attribs[Attributes::UseChangelog] = persist_changelog ? Attributes::True : Attributes::False;
			attribs[Attributes::Changelog] = persist_changelog ? getChangelogDefinition() : "";
			attribs[Attributes::GenDisabledObjsCode]= gen_dis_objs_code ? Attributes::True : Attributes::False;
			attribs[Attributes::ShowSysSchemasRects]= show_sys_sch_rects ? Attributes::True : Attributes::False;
		}
		catch (Exception &e)
		{
			throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		QStringList act_layers;

		for(auto &layer_id : active_layers)
			act_layers.push_back(QString::number(layer_id));

		attribs[Attributes::Layers] = layers.join(',');
		attribs[Attributes::ActiveLayers] = act_layers.join(',');
		attribs[Attributes::LayerNameColors] = layer_name_colors.join(',');
		attribs[Attributes::LayerRectColors] = layer_rect_colors.join(',');
		attribs[Attributes::ShowLayerNames] = (is_layer_names_visible ? Attributes::True : Attributes::False);
		attribs[Attributes::ShowLayerRects] = (is_layer_rects_visible ? Attributes::True : Attributes::False);
		attribs[Attributes::MaxObjCount] = QString::number(static_cast<unsigned>(getMaxObjectCount() * 1.20));
		attribs[Attributes::Protected] = (this->is_protected ? Attributes::True : "");
		attribs[Attributes::LastPosition] = QString("%1,%2").arg(last_pos.x()).arg(last_pos.y());
		attribs[Attributes::LastZoom] = QString::number(last_zoom);
		attribs[Attributes::DefaultSchema] = (default_objs[ObjectType::Schema] ? default_objs[ObjectType::Schema]->getName(true) : "");
		attribs[Attributes::DefaultOwner] = (default_objs[ObjectType::Role] ? default_objs[ObjectType::Role]->getName(true) : "");
		attribs[Attributes::DefaultTablespace] = (default_objs[ObjectType::Tablespace] ? default_objs[ObjectType::Tablespace]->getName(true) : "");
		attribs[Attributes::DefaultCollation] = (default_objs[ObjectType::Collation] ? default_objs[ObjectType::Collation]->getName(true) : "");

		if(!scene_rect.isValid())
			attribs[Attributes::SceneRect] = "";
		else
		{
			attribs[Attributes::SceneRect] = QString("%1,%2,%3,%4")
																					 .arg(scene_rect.left())
																					 .arg(scene_rect.top())
																					 .arg(scene_rect.width())
																					 .arg(scene_rect.height());
		}
	}
}

std::map<unsigned, BaseObject *> DatabaseModel::getCreationOrder(SchemaParser::CodeType def_type, bool incl_relnn_objs, bool incl_rel1n_constrs, bool realloc_fk_perms)
{
	std::vector<BaseObject *> fkeys, fk_rels, aux_tables;
	std::vector<BaseObject *> *obj_list = nullptr;
	std::map<unsigned, BaseObject *> objects_map;
	PhysicalTable *table = nullptr;
	Constraint *constr = nullptr;
	View *view = nullptr;
	Relationship *rel = nullptr;
	ObjectType aux_obj_types[]={ ObjectType::Role, ObjectType::Tablespace, ObjectType::Schema, ObjectType::Tag };

	std::vector<ObjectType> obj_types_vect = getObjectTypes(false, { ObjectType::Role, ObjectType::Tablespace, ObjectType::Schema,
																																	 ObjectType::Tag, ObjectType::Database, ObjectType::Permission });

	//The first objects on the map will be roles, tablespaces, schemas and tags
	for(auto &obj_tp : aux_obj_types)
	{
		if(obj_tp != ObjectType::Tag || def_type == SchemaParser::XmlCode)
		{
			obj_list = getObjectList(obj_tp);

			for(auto &object : (*obj_list))
				objects_map[object->getObjectId()] = object;
		}
	}

	//Includes the database model on the objects map permitting to create the code in a correct order
	objects_map[this->getObjectId()] = this;

	for(auto &obj_type : obj_types_vect)
	{
		//For SQL definition, only the textbox and base relationship does not enters to the code generation list
		if(def_type == SchemaParser::SqlCode &&
			 (obj_type == ObjectType::Textbox || obj_type==ObjectType::BaseRelationship))
			obj_list = nullptr;
		else
			obj_list = getObjectList(obj_type);

		if(obj_list)
		{
			for(auto &object : (*obj_list))
			{
				/* If the object is a FK relationship it's stored in a separeted list in order to have the
				 * code generated at end of whole definition (after foreign keys definition) */
				if(object->getObjectType() == ObjectType::BaseRelationship &&
						dynamic_cast<BaseRelationship *>(object)->getRelationshipType() == BaseRelationship::RelationshipFk)
				{
					fk_rels.push_back(object);
				}
				else
				{
					if(def_type == SchemaParser::XmlCode || !incl_relnn_objs)
						objects_map[object->getObjectId()] = object;
					else
					{
						rel = dynamic_cast<Relationship *>(object);

						/* Avoiding many-to-many relationships to be included in the map.
						 * They are treated in a separated way below, because on the diff process (ModelsDiffHelper) the generated table
						 * need to be compared to other tables not the relationship itself */
						if(!incl_relnn_objs || !rel || (rel && rel->getRelationshipType() != BaseRelationship::RelationshipNn))
							objects_map[object->getObjectId()] = object;
					}
				}
			}
		}
	}

	/* Getting and storing the special objects (which reference columns of tables added for relationships)
	  on the map of objects. */
	aux_tables = tables;
	aux_tables.insert(aux_tables.end(), foreign_tables.begin(), foreign_tables.end());

	for(auto &obj : aux_tables)
	{
		table = dynamic_cast<PhysicalTable *>(obj);

		for(auto &obj : *table->getObjectList(ObjectType::Constraint))
		{
			constr = dynamic_cast<Constraint *>(obj);

			/* Case the constraint is a special object stores it on the objects map. Independently to the
			 * configuration, foreign keys are discarded in this iteration because on the end of the method
			 * they have the definition generated */
			if(constr->getConstraintType() != ConstraintType::ForeignKey && !constr->isAddedByLinking() &&
					((constr->getConstraintType() != ConstraintType::PrimaryKey && constr->isReferRelationshipAddedColumns())))
				objects_map[constr->getObjectId()] = constr;
			else if(constr->getConstraintType() == ConstraintType::ForeignKey && !constr->isAddedByLinking())
				fkeys.push_back(constr);
		}

		for(auto &obj : table->getObjects({ ObjectType::Column, ObjectType::Constraint }))
		{
			// Ignoring generated FK column indexes created by a relationship
			if(obj->getObjectType() == ObjectType::Index &&
				 dynamic_cast<TableObject *>(obj)->isAddedByRelationship())
				continue;

			objects_map[obj->getObjectId()] = obj;
		}
	}

	/* Getting and storing the special objects (which reference columns of tables added for relationships)
			on the map of objects. */
	for(auto &obj : views)
	{
		view=dynamic_cast<View *>(obj);

		for(auto obj : view->getObjects())
			objects_map[obj->getObjectId()] = obj;
	}

	/* SPECIAL CASE: Generating the correct order for tables, views, relationships and sequences

	 This generation is made in the following way:
	 1) Based on the relationship list, participant tables comes before the relationship itself.
	 2) Other tables came after the objects on the step 1.
	 3) The sequences must have their code generated after the tables
	 4) View are the last objects in the list avoiding table/column reference breaking */
	if(def_type == SchemaParser::SqlCode)
	{
		BaseObject *objs[3] = { nullptr, nullptr, nullptr };
		std::vector<BaseObject *> vet_aux, rel_constrs;

		vet_aux = relationships;
		vet_aux.insert(vet_aux.end(), tables.begin(), tables.end());
		vet_aux.insert(vet_aux.end(), foreign_tables.begin(), foreign_tables.end());
		vet_aux.insert(vet_aux.end(), sequences.begin(), sequences.end());
		vet_aux.insert(vet_aux.end(), views.begin(), views.end());

		for(auto &object : vet_aux)
		{
			if(object->getObjectType() == ObjectType::Relationship)
			{
				rel = dynamic_cast<Relationship *>(object);
				objs[0] = rel->getTable(Relationship::SrcTable);
				objs[1] = rel->getTable(Relationship::DstTable);

				/* For many-to-many relationship, the generated table and the foreign keys that represents
				 * the link are included on the creation order map instead of the relationship itself. This is
				 * done to permit the table to be accessed and compared on the diff process */
				if(incl_relnn_objs &&
						rel->getRelationshipType() == BaseRelationship::RelationshipNn &&
						rel->getGeneratedTable())
				{
					table = rel->getGeneratedTable();
					objs[2] = table;

					for(auto &tab_obj : *table->getObjectList(ObjectType::Constraint))
					{
						constr = dynamic_cast<Constraint *>(tab_obj);

						if(constr->getConstraintType() == ConstraintType::ForeignKey)
							fkeys.push_back(constr);
					}
				}
				else if(incl_rel1n_constrs)
				{
					for(auto &constr : rel->getGeneratedConstraints())
					{
						if(constr->getConstraintType()!=ConstraintType::PrimaryKey)
							rel_constrs.push_back(constr);
					}					
				}
				else
					objs[2] = rel;

				for(auto &obj : objs)
				{
					if(obj && objects_map.count(obj->getObjectId()) == 0)
						objects_map[obj->getObjectId()] = obj;
				}

				if(rel->getGeneratedIndex() && (incl_rel1n_constrs || incl_relnn_objs))
					objects_map[rel->getGeneratedIndex()->getObjectId()] = rel->getGeneratedIndex();
			}
			else
			{
				if(objects_map.count(object->getObjectId()) == 0)
					objects_map[object->getObjectId()] = object;
			}
		}

		fkeys.insert(fkeys.end(), rel_constrs.begin(), rel_constrs.end());
	}

	fkeys.insert(fkeys.end(), fk_rels.begin(), fk_rels.end());

	if(realloc_fk_perms)
	{
		//Adding fk relationships and foreign keys at end of objects map
		unsigned i = BaseObject::getGlobalId() + 1;

		for(auto &obj : fkeys)
		{
			objects_map[i] = obj;
			i++;
		}

		//Adding permissions at the very end of object map
		i = BaseObject::getGlobalId() + fkeys.size() + 1;

		for(auto &obj : permissions)
		{
			objects_map[i] = obj;
			i++;
		}
	}
	else
	{
		for(auto &obj : fkeys)
			objects_map[obj->getObjectId()] = obj;

		for(auto &obj : permissions)
			objects_map[obj->getObjectId()] = obj;
	}

	return objects_map;
}

std::vector<BaseObject *> DatabaseModel::getCreationOrder(BaseObject *object, bool only_children)
{
	if(!object)
		return {};

	std::map<unsigned, BaseObject *> objs_map;
	std::vector<BaseObject *> created_objs, children;
	std::vector<Permission *> perms_aux, perms;
	std::vector<Role *> roles;
	PhysicalTable *table=nullptr;
	Relationship *rel=nullptr;
	ObjectType obj_type=object->getObjectType();
	std::vector<BaseObject *> objs_aux;

	if(!only_children)
		created_objs = object->getDependencies();

	created_objs.push_back(object);

	/* Include tables generated by many-to-many relationships if their schemas are the same
	 as the 'object' when this one is a schema too */
	if(obj_type==ObjectType::Schema)
	{
		if(only_children)
		{
			children=getObjects(object);
			created_objs.insert(created_objs.end(), children.begin(), children.end());
		}

		for(auto &obj : relationships)
		{
			rel=dynamic_cast<Relationship *>(obj);

			if(rel->getRelationshipType()==Relationship::RelationshipNn &&
					rel->getGeneratedTable() &&
					rel->getGeneratedTable()->getSchema()==object)
			{
				if(only_children)
					created_objs.push_back(rel->getGeneratedTable());
				else
				{
					objs_aux = rel->getGeneratedTable()->getDependencies();
					created_objs.insert(created_objs.end(), objs_aux.begin(), objs_aux.end());
				}
			}
		}
	}

	if(only_children)
	{
		BaseTable *table=nullptr;
		Constraint *constr=nullptr;

		for(auto &obj : created_objs)
		{
			table=dynamic_cast<BaseTable *>(obj);

			if(table)
			{
				children=table->getObjects();

				for(auto &child : children)
				{
					constr=dynamic_cast<Constraint *>(child);

					if((!constr && child->getObjectType()!=ObjectType::Column) ||
							(constr &&
							 ((constr->getConstraintType()==ConstraintType::ForeignKey) ||
								(constr->getConstraintType()!=ConstraintType::ForeignKey &&
								 constr->getConstraintType()!=ConstraintType::PrimaryKey &&
							   constr->isReferRelationshipAddedColumns()))))
					{
						objs_aux.push_back(child);
					}
				}
			}
		}

		created_objs.insert(created_objs.end(), objs_aux.begin(), objs_aux.end());
	}
	else
	{
		//Retrieving all permission related to the gathered objects
		for(auto &obj : created_objs)
		{
			getPermissions(obj, perms_aux);
			perms.insert(perms.end(), perms_aux.begin(), perms_aux.end());
			perms_aux.clear();

			table=dynamic_cast<PhysicalTable *>(obj);

			if(table)
			{
				for(auto &col : *table->getObjectList(ObjectType::Column))
				{
					getPermissions(col, perms_aux);
					perms.insert(perms.end(), perms_aux.begin(), perms_aux.end());
				}
			}
		}

		/* Retrieving all additional roles (reference by permissions) that are not in the
		 * main object list (used as creation order) */
		for(auto &perm : perms)
		{
			roles=perm->getRoles();

			for(auto &role : roles)
			{
				if(std::find(created_objs.begin(), created_objs.end(), role)==created_objs.end())
				{
					objs_aux = role->getDependencies();
					created_objs.insert(created_objs.end(), objs_aux.begin(), objs_aux.end());
				}
			}
		}
	}

	if(created_objs.size() > 1 || !perms.empty())
	{
		//Putting all objects in a map ordering them by ID
		for(auto &obj : created_objs)
			objs_map[obj->getObjectId()]=obj;

		//Recreationg the object list now with objects ordered properly
		created_objs.clear();
		for(auto &itr : objs_map)
			created_objs.push_back(itr.second);

		//Appending permissions at the end of the creation order list
		created_objs.insert(created_objs.end(), perms.begin(), perms.end());
	}

	return created_objs;
}

void DatabaseModel::saveModel(const QString &filename, SchemaParser::CodeType def_type)
{
	try
	{
		if(!cancel_saving)
			UtilsNs::saveFile(filename, this->getSourceCode(def_type).toUtf8());
	}
	catch(Exception &e)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::FileNotWrittenInvalidDefinition).arg(filename),
										ErrorCode::FileNotWrittenInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

bool DatabaseModel::saveSplitCustomSQL(bool save_appended, const QString &path, const QString &file_prefix)
{
	QString filename, msg;
	QByteArray buffer;

	if(!save_appended && prepend_at_bod && !prepended_sql.isEmpty())
	{
		filename = file_prefix + "_prepended_code.sql";
		msg = tr("Saving prepended SQL code to file `%1'.").arg(filename);
		buffer.append((prepended_sql + QChar('\n') + Attributes::DdlEndToken).toUtf8());
	}
	else if(save_appended && append_at_eod && !appended_sql.isEmpty())
	{
		filename = file_prefix + "_appended_code.sql";
		msg = tr("Saving appended SQL code to file `%1'.").arg(filename);
		buffer.append((appended_sql + QChar('\n') + Attributes::DdlEndToken).toUtf8());
	}

	if(!buffer.isEmpty())
	{
		emit s_objectLoaded(!save_appended ? 0 : 100, msg, enum_t(ObjectType::Database));
		UtilsNs::saveFile(path + GlobalAttributes::DirSeparator + filename, buffer);
		return true;
	}

	return false;
}

void DatabaseModel::saveSplitSQLDefinition(const QString &path, CodeGenMode code_gen_mode, bool gen_drop_file)
{
	QFileInfo fi(path);
	QDir dir;

	if(fi.exists() && !fi.isDir())
		throw Exception(Exception::getErrorMessage(ErrorCode::InvOutputDirectory).arg(path),
										ErrorCode::InvOutputDirectory,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(code_gen_mode > GroupByType)
		throw Exception(Exception::getErrorMessage(ErrorCode::InvCodeGenerationMode).arg(code_gen_mode),
										ErrorCode::InvCodeGenerationMode,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!fi.exists())
		dir.mkdir(path);

	bool group_by_type = code_gen_mode == GroupByType;
	QByteArray buffer;
	std::map<unsigned, BaseObject *>  aux_objs,
			objects = getCreationOrder(SchemaParser::SqlCode, true, true, !group_by_type);
	int pad_size = QString::number(objects.size()).size(), idx = 1;
	QString filename, name, shell_types;
	BaseObject *obj = nullptr;
	QStringList sch_names;
	QRegularExpression name_fmt_regexp("(?!\\-)(\\W)");
	unsigned 	gen_defs_idx = 0, general_obj_cnt = 0;
	attribs_map attribs;
	std::map<QString, QByteArray> grouped_defs,
			grouped_drops;
	ObjectType obj_type;
	Table *tab = nullptr;
	ForeignTable *ftab = nullptr;
	Constraint *constr = nullptr;
	std::vector<Constraint *> contraints;
	QString obj_type_name;
	std::map<unsigned, QString> constr_filename = {
		{ ConstraintType::PrimaryKey, "constraint_pk" },
		{ ConstraintType::ForeignKey, "constraint_fk" },
		{ ConstraintType::Unique, "constraint_uq" },
		{ ConstraintType::Exclude, "constraint_ex" },
		{ ConstraintType::Check, "constraint_ck" },
	};

	/* When generating a split model in GroupByType mode
	 * we need to get primary key and unique keys that aren't
	 * included by getCreationOrder() so we can separate their
	 * SQL code from the parent tables and save it in the
	 * file reserved for constraints definition */
	if(code_gen_mode == GroupByType)
	{
		PhysicalTable *tab = nullptr;

		for(auto &itr : objects)
		{
			tab = dynamic_cast<PhysicalTable *>(itr.second);

			if(!tab)
				continue;

			for(auto &tab_obj : *tab->getObjectList(ObjectType::Constraint))
			{
				aux_objs[tab_obj->getObjectId()] = tab_obj;
				constr = dynamic_cast<Constraint *>(tab_obj);

				/* Forcing the flag decl_in_table of the contraint to false
				 * so the SQL code is generated in full form */
				if(constr->isDeclaredInTable())
				{
					constr->setDeclaredInTable(false);

					/* Store the constraint in a temporary list so the
					 * flag can be restored at the end of the export */
					contraints.push_back(constr);
				}
			}
		}

		objects.insert(aux_objs.begin(), aux_objs.end());
	}

	try
	{
		cancel_saving = false;
		general_obj_cnt = objects.size();
		shell_types = configureShellTypes(false);

		/* We try to save prepended code as the first script. In case of success increment the script index
		 * to keep generating the other scripts in the right order */
		if(saveSplitCustomSQL(false, path, QString::number(idx).rightJustified(pad_size, '0')))
			idx++;

		for(auto &itr : objects)
		{
			if(cancel_saving)
				break;

			obj = itr.second;
			obj_type = obj->getObjectType();

			if(obj_type == ObjectType::Schema)
				sch_names.append(obj->getName(true));

			gen_defs_idx++;

			if(obj->isSystemObject() ||
				 obj_type == ObjectType::BaseRelationship ||
				 obj_type == ObjectType::Relationship ||

				 /* Ignoring disabled SQL code if the flag to include this kind
					* of code is off. This will diminish the size of the resulting
					* scripts */
				 (obj->isSQLDisabled() && !gen_dis_objs_code))
				continue;

			// Saving the shell types before we start generating the files of other objects
			if(!shell_types.isEmpty() &&
				 obj_type != ObjectType::Role && obj_type != ObjectType::Tablespace &&
				 obj_type != ObjectType::Database && obj_type != ObjectType::Schema)
			{
				filename = QString("%1_%2.sql")
									 .arg(QString::number(idx++).rightJustified(pad_size, '0'),
												Attributes::ShellTypes.toLower().replace('-', '_'));

				emit s_objectLoaded((gen_defs_idx/static_cast<double>(general_obj_cnt)) * 100,
									tr("Saving SQL of shell types to file `%1'.").arg(filename),
									enum_t(ObjectType::Type));

				buffer.append(shell_types.toUtf8());
				UtilsNs::saveFile(path + GlobalAttributes::DirSeparator + filename, buffer);
				buffer.clear();
				shell_types.clear();
			}

			/* In GroupByType split mode we force the generation of table/foreign table
			 * DDL command without the constraint codes since they will be saved in
			 * a separated file */
			if(group_by_type && PhysicalTable::isPhysicalTable(obj_type))
			{
				tab = dynamic_cast<Table *>(obj);
				ftab = dynamic_cast<ForeignTable *>(obj);

				if(tab)
					buffer.append(tab->__getSourceCode(SchemaParser::SqlCode, true, false).toUtf8());
				else
					buffer.append(ftab->__getSourceCode(SchemaParser::SqlCode, true, false).toUtf8());
			}
			else
				buffer.append(getSQLDefinition({ obj }, code_gen_mode).toUtf8());

			if(buffer.isEmpty())
				continue;

			// Grouping the SQL definitions before saving to file
			if(group_by_type)
			{
				emit s_objectLoaded((gen_defs_idx/static_cast<double>(general_obj_cnt)) * 100,
														tr("Generating SQL of `%1' (%2).")
														.arg(obj->getSignature(), obj->getTypeName()),
														enum_t(obj_type));

				obj_type_name = obj->getSchemaName();

				if(obj_type == ObjectType::Constraint)
				{
					constr = dynamic_cast<Constraint *>(obj);
					obj_type_name = constr_filename[constr->getConstraintType().getTypeId()];
				}

				grouped_defs[obj_type_name] += buffer;
			}
			else
			{
				/* The name of the generated file will be:
				 * [creation order id]_[name]_[type]_[internal id].sql
				 * Note: the name portion of the file is treated to remove special char (non word chars) that may break
				 * the filename in some filesystems. The internal id is used for desambiguation purposes. */

				// If the object is a table child object we use its signature instead of name
				if(TableObject::isTableObject(obj_type))
					name = dynamic_cast<TableObject *>(obj)->TableObject::getSignature(true);
				else
					name = obj->getName(true);

				name.replace('"', "").replace(name_fmt_regexp, "_");

				filename = QString("%1_%2_%3_%4.sql")
									 .arg(QString::number(idx++).rightJustified(pad_size, '0'))
									 .arg(name)
									 .arg(obj->getSchemaName())
									 .arg(obj->getObjectId());

				emit s_objectLoaded((gen_defs_idx/static_cast<double>(general_obj_cnt)) * 100,
									tr("Saving SQL of `%1' (%2) to file `%3'.")
									.arg(obj->getSignature())
									.arg(obj->getTypeName())
									.arg(filename),
									enum_t(obj_type));

				UtilsNs::saveFile(path + GlobalAttributes::DirSeparator + filename, buffer);
			}

			buffer.clear();

			/* If the current object is the database itself, we need to save the sessionopts
			 * right before the saving of the database creation script */
			if(obj == this)
			{
				// Saving the sessionopts.sql containg session options like search_path and check_function_bodies
				attribs[Attributes::SearchPath] = sch_names.join(',');
				attribs[Attributes::Function] = !functions.empty() ? Attributes::True : "";

				filename = QString("%1_%2.sql")
									 .arg(QString::number(idx++).rightJustified(pad_size, '0'))
									 .arg(Attributes::SessionOpts);

				emit s_objectLoaded((gen_defs_idx/static_cast<double>(general_obj_cnt)) * 100, tr("Saving session options file `%1'.").arg(filename),
														enum_t(ObjectType::Database));

				buffer.append(schparser.getSourceCode(Attributes::SessionOpts, attribs, SchemaParser::SqlCode).toUtf8());
				UtilsNs::saveFile( path + GlobalAttributes::DirSeparator + filename, buffer);
				buffer.clear();
			}
		}

		/* Saving the grouped SQL definition files if the map of grouped
		 * defintions is filled. */
		for(auto &itr : grouped_defs)
		{
			filename = QString("%1_%2.sql")
								.arg(itr.first,
										 itr.first == BaseObject::getSchemaName(ObjectType::Permission) ?
										 Attributes::Grant : Attributes::Create);

			emit s_objectLoaded(100, tr("Saving SQL file `%1' .").arg(filename), enum_t(ObjectType::Database));

			UtilsNs::saveFile(path + GlobalAttributes::DirSeparator + filename, itr.second);
		}

		// Generating the file containing all DROP commands
		if(gen_drop_file)
		{
			std::map<unsigned, BaseObject *>::reverse_iterator ritr = objects.rbegin();
			QString drop_cmd;

			/* We iterate over the object in reverse order because they need to be destroyed
			 * from the last to the first */
			while(ritr != objects.rend())
			{
				obj = ritr->second;
				ritr++;

				if(obj->isSystemObject())
					continue;

				drop_cmd = obj->getDropCode(true);

				// Disabling the drop command if the object is also with SQL disabled
				if(obj->isSQLDisabled())
					drop_cmd.prepend("-- ");

				if(group_by_type)
				{
					obj_type_name = obj->getSchemaName();

					if(obj->getObjectType() == ObjectType::Constraint)
					{
						constr = dynamic_cast<Constraint *>(obj);
						obj_type_name = constr_filename[constr->getConstraintType().getTypeId()];
					}

					grouped_drops[obj_type_name] += drop_cmd.toUtf8();
				}
				else
					buffer.append(drop_cmd.toUtf8());
			}

			// Restoring the decl_in_table flag in constraints
			for(auto &constr : contraints)
				constr->setDeclaredInTable(true);

			/* If we are not generating grouped definitions
			 * we reuse the groped_drops having a key = Attributes::DropCmds
			 * and value = buffer with the whole drop commands just to
			 * make a single iteration in the for below to
			 * simplify the logic */
			if(!group_by_type)
				grouped_drops[Attributes::DropCmds] = buffer;

			// Generating the file(s) containing the drop commands
			for(auto &itr : grouped_drops)
			{
				if(itr.first == BaseObject::getSchemaName(ObjectType::BaseRelationship) ||
					 itr.first == BaseObject::getSchemaName(ObjectType::Relationship))
					continue;

				// If we are generating a single file with all DROP commands
				if(itr.first == Attributes::DropCmds)
				{
					filename = QString("%1_%2.sql")
										 .arg(QString::number(0).rightJustified(pad_size, '0'), Attributes::Drop);
				}
				// If we are generating a DROP file per object type
				else
				{
					filename = QString("%1_%2.sql")
										 .arg(itr.first,
													itr.first == BaseObject::getSchemaName(ObjectType::Permission) ?
													Attributes::Revoke : Attributes::Drop);
				}

				emit s_objectLoaded(100, tr("Saving drop commands file `%1'.").arg(filename),	enum_t(ObjectType::Database));

				UtilsNs::saveFile(path + GlobalAttributes::DirSeparator + filename, itr.second);
			}
		}

		// Saving the prepended sql file
		saveSplitCustomSQL(true, path, QString::number(idx).rightJustified(pad_size, '0'));
		configureShellTypes(true);
	}
	catch (Exception &e)
	{
		configureShellTypes(true);
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void DatabaseModel::setObjectsModified(std::vector<BaseObject *> &objects)
{
	for(auto &obj : objects)
	{
		if(BaseGraphicObject::isGraphicObject(obj->getObjectType()))
			dynamic_cast<BaseGraphicObject *>(obj)->setModified(true);
	}
}

void DatabaseModel::setObjectsModified(std::vector<ObjectType> types)
{
	ObjectType obj_types[]={
							ObjectType::Table, ObjectType::View, ObjectType::ForeignTable,
							ObjectType::Relationship, ObjectType::BaseRelationship,
							ObjectType::Textbox, ObjectType::Schema };
	std::vector<BaseObject *>::iterator itr, itr_end;
	std::vector<BaseObject *> *obj_list = nullptr;
	Textbox *label = nullptr;
	BaseRelationship *rel = nullptr;
	BaseGraphicObject *graph_obj = nullptr;
	unsigned i1 = 0;

	for(auto &obj_type : obj_types)
	{
		if(types.empty() || std::find(types.begin(), types.end(), obj_type) != types.end())
		{
			obj_list = getObjectList(obj_type);
			itr = obj_list->begin();
			itr_end = obj_list->end();

			while(itr != itr_end)
			{
				graph_obj = dynamic_cast<BaseGraphicObject *>(*itr);

				if(BaseTable::isBaseTable(obj_type))
				{
					/* We call the BaseTable::resetHashCode version to force
					 * to force table to be redrawn by overriding the
					 * hash validation in TableView::configureObject */
					dynamic_cast<BaseTable *>(graph_obj)->resetHashCode();
				}

				graph_obj->setModified(true);

				//For relationships is needed to set the labels as modified too
				if(obj_type == ObjectType::Relationship || obj_type == ObjectType::BaseRelationship)
				{
					rel=dynamic_cast<BaseRelationship *>(*itr);
					for(i1 = BaseRelationship::SrcCardLabel; i1 <= BaseRelationship::RelNameLabel; i1++)
					{
						label = rel->getLabel(static_cast<BaseRelationship::LabelId>(i1));
						if(label) label->setModified(true);
					}
				}

				itr++;
			}
		}
	}
}

void DatabaseModel::setCodesInvalidated(std::vector<ObjectType> types)
{
	std::vector<ObjectType> sel_types;
	std::vector<BaseObject *> *list = nullptr;

	if(types.empty())
		sel_types = BaseObject::getObjectTypes(false);
	else
	{
		ObjectType tab_obj_types[] { ObjectType::Column, ObjectType::Constraint,
																 ObjectType::Trigger, ObjectType::Rule, ObjectType::Index,
																 ObjectType::Policy };

		for(auto tab_obj_type : tab_obj_types)
			sel_types.erase(std::find(sel_types.begin(), sel_types.end(), tab_obj_type));

		sel_types = types;
	}

	while(!sel_types.empty())
	{
		list=getObjectList(sel_types.back());
		sel_types.pop_back();

		if(list)
		{
			for(auto &obj : *list)
				obj->setCodeInvalidated(true);
		}
	}
}

void DatabaseModel::validateSchemaRenaming(Schema *schema, const QString &prev_sch_name)
{
	std::vector<ObjectType> types = { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View,
																		ObjectType::Domain, ObjectType::Type, ObjectType::Sequence };
	std::vector<BaseObject *> list, sch_objs, refs;
	QString prev_name,
			fmt_prev_sch_name = BaseObject::formatName(prev_sch_name, false),
			obj_sig;

	//Raise an error if the schema is not allocated
	if(!schema)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Get all the objects on the informed schema
	for(auto &type : types)
	{
		list = getObjects(type, schema);
		sch_objs.insert(sch_objs.end(), list.begin(), list.end());
	}

	for(auto &obj : sch_objs)
	{
		//Configures the previous type name
		prev_name = fmt_prev_sch_name + "." +	obj->getName();
		obj_sig = obj->getSignature();

		PgSqlType::renameUserType(prev_name, obj, obj_sig);
		refs = obj->getReferences();

		//For graphical objects set them as modified to redraw them
		if(BaseTable::isBaseTable(obj->getObjectType()))
			dynamic_cast<BaseTable *>(obj)->setModified(true);

		for(auto &ref_obj : refs)
		{
			if(BaseTable::isBaseTable(ref_obj->getObjectType()))
				dynamic_cast<BaseTable *>(ref_obj)->setModified(true);
			else if(TableObject::isTableObject(ref_obj->getObjectType()))
			{
				BaseTable *tab = dynamic_cast<TableObject *>(ref_obj)->getParentTable();
				tab->setModified(true);
				tab->setCodeInvalidated(true);
			}

			ref_obj->setCodeInvalidated(true);
		}

		refs.clear();
	}
}

void DatabaseModel::createSystemObjects(bool create_public)
{
	Schema *public_sch=nullptr, *pg_catalog=nullptr;
	Language *lang=nullptr;
	Tablespace *tbspace=nullptr;
	QStringList langs={ DefaultLanguages::C, DefaultLanguages::Sql,
											DefaultLanguages::PlPgsql, DefaultLanguages::Internal };
	Role *postgres=nullptr;
	Collation *collation=nullptr;
	QString collnames[]={ "default", "C", "POSIX" };

	/* The particular case is for public schema that is created only when the flag
	is set. This because the public schema is written on model file even being
	a system object. This strategy permits the user controls the schema rectangle behavior */
	if(create_public && getObjectIndex("public", ObjectType::Schema) < 0)
	{
		public_sch=new Schema;
		public_sch->setName("public");
		public_sch->setSystemObject(true);
		addSchema(public_sch);
	}

	//Create the pg_catalog schema in order to insert default collations in
	pg_catalog=new Schema;
	pg_catalog->BaseObject::setName("pg_catalog");
	pg_catalog->setSystemObject(true);
	addSchema(pg_catalog);

	//Creating default collations
	for(const auto & collname : collnames)
	{
		collation = new Collation;
		collation->setName(collname);
		collation->setSchema(pg_catalog);
		collation->setEncoding(EncodingType("UTF8"));
		collation->setLocale("C");
		collation->setSystemObject(true);
		addCollation(collation);
	}

	for(auto &lang_name : langs)
	{
		if(getObjectIndex(lang_name, ObjectType::Language) < 0)
		{
			lang = new Language;
			lang->BaseObject::setName(lang_name);
			lang->setSystemObject(true);
			addLanguage(lang);
		}
	}

	tbspace=new Tablespace;
	tbspace->BaseObject::setName("pg_global");
	tbspace->setDirectory("_pg_global_dir_");
	tbspace->setSystemObject(true);
	addTablespace(tbspace);

	tbspace=new Tablespace;
	tbspace->BaseObject::setName("pg_default");
	tbspace->setDirectory("_pg_default_dir_");
	tbspace->setSystemObject(true);
	addTablespace(tbspace);

	postgres=new Role;
	postgres->setName(QString("postgres"));
	postgres->setOption(Role::OpSuperuser, true);
	postgres->setSystemObject(true);
	addRole(postgres);

	setDefaultObject(postgres);
	setDefaultObject(getObject("public", ObjectType::Schema), ObjectType::Schema);
}

std::vector<BaseObject *> DatabaseModel::findObjects(const QStringList &filters, const QString &search_attr, bool any_incl_cols)
{
	std::vector<BaseObject *> objects, aux_objs;
	QString pattern, mode;
	QStringList values, modes = { UtilsNs::FilterWildcard, UtilsNs::FilterRegExp };
	ObjectType obj_type;
	bool exact_match = false;
	std::vector<ObjectType> types;

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
		exact_match = (mode == UtilsNs::FilterWildcard && !pattern.contains(UtilsNs::WildcardChar));

		// Raises an error if the filter has an invalid object type, pattern or mode
		if((values[0] != Attributes::Any && obj_type == ObjectType::BaseObject) ||
				pattern.isEmpty() || !modes.contains(mode))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::InvalidObjectFilter).arg(filter).arg(modes.join('|')),
											ErrorCode::InvalidObjectFilter,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		// If we use the "any" filter key word all object types will be retrieved
		types.clear();

		if(obj_type == ObjectType::BaseObject)
		{
			std::vector<ObjectType> excl_types = {
				ObjectType::BaseRelationship, ObjectType::Textbox,
				ObjectType::Tag, ObjectType::GenericSql, ObjectType::Database
			};

			// Including Columns to the excluded type if the "any" filter doen't include columns
			if(!any_incl_cols)
				excl_types.push_back(ObjectType::Column);

			types = BaseObject::getObjectTypes(true, excl_types);
		}
		else
			types.push_back(obj_type);

		aux_objs = findObjects(pattern, types, false,
													 mode == UtilsNs::FilterRegExp, exact_match, search_attr);
		objects.insert(objects.end(), aux_objs.begin(), aux_objs.end());
	}

	std::sort(objects.begin(), objects.end());
	std::vector<BaseObject *>::iterator end = std::unique(objects.begin(), objects.end());
	objects.erase(end, objects.end());

	return objects;
}

std::vector<BaseObject *> DatabaseModel::findObjects(const QString &pattern, std::vector<ObjectType> types, bool case_sensitive, bool is_regexp, bool exact_match, const QString &search_attr)
{
	std::vector<BaseObject *> list, objs;
	std::vector<BaseObject *>::iterator end;
	std::vector<ObjectType>::iterator itr_tp=types.begin();
	std::vector<BaseObject *> tables;
	bool inc_tabs=false, inc_views=false, inc_rels = false;
	QRegularExpression regexp;
	attribs_map srch_attribs;

	if(!case_sensitive)
		regexp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

	if(is_regexp)
	{
		if(!exact_match)
			regexp.setPattern(pattern);
		else
			regexp.setPattern(QRegularExpression::anchoredPattern(pattern));
	}
	else
		regexp.setPattern(QRegularExpression::wildcardToRegularExpression(pattern));

	//If there is some table object types on the type list, gather tables and views
	while(itr_tp!=types.end() && (!inc_views || !inc_tabs))
	{
		if(!inc_tabs && TableObject::isTableObject(*itr_tp))
		{
			tables.insert(tables.end(), getObjectList(ObjectType::Table)->begin(), getObjectList(ObjectType::Table)->end());
			tables.insert(tables.end(), getObjectList(ObjectType::ForeignTable)->begin(), getObjectList(ObjectType::ForeignTable)->end());
			inc_tabs=true;
		}

		if(!inc_views && ((*itr_tp)==ObjectType::Rule || (*itr_tp)==ObjectType::Trigger))
		{
			tables.insert(tables.end(), getObjectList(ObjectType::View)->begin(), getObjectList(ObjectType::View)->end());
			inc_views=true;
		}

		itr_tp++;
	}

	//Gathering all other objects
	for(auto &obj_type : types)
	{
		if(obj_type == ObjectType::Database)
			objs.push_back(this);
		// Base relationships (fk rels and table-view rels) are treated as table to table relationship in the search
		else if(!inc_rels && (obj_type == ObjectType::BaseRelationship || obj_type == ObjectType::Relationship))
		{
			inc_rels = true;
			objs.insert(objs.end(), getObjectList(ObjectType::BaseRelationship)->begin(), getObjectList(ObjectType::BaseRelationship)->end());
			objs.insert(objs.end(), getObjectList(ObjectType::Relationship)->begin(), getObjectList(ObjectType::Relationship)->end());
		}
		else if(!TableObject::isTableObject(obj_type))
			objs.insert(objs.end(), getObjectList(obj_type)->begin(), getObjectList(obj_type)->end());
		else
		{
			//Including table object on the object list
			std::vector<TableObject *> *tab_objs=nullptr;

			for(auto &tab : tables)
			{
				if(PhysicalTable::isPhysicalTable(tab->getObjectType()))
					tab_objs=dynamic_cast<PhysicalTable *>(tab)->getObjectList(obj_type);
				else if(tab->getObjectType()==ObjectType::View &&	(obj_type==ObjectType::Trigger || obj_type==ObjectType::Rule))
					tab_objs=dynamic_cast<View *>(tab)->getObjectList(obj_type);

				if(tab_objs)
					objs.insert(objs.end(), tab_objs->begin(), tab_objs->end());
			}
		}
	}

	//Try to find  the objects on the configured list
	for(auto &obj : objs)
	{
		obj->configureSearchAttributes();
		srch_attribs = obj->getSearchAttributes();

		/* If we are searching in the constraints' source and referenced columns
		 * we have to split the list of names so the regexp can be correctly matched */
		if(search_attr == Attributes::SrcColumns || search_attr == Attributes::RefColumns)
		{
			QStringList aux_names, ref_names = srch_attribs[search_attr].split(UtilsNs::DataSeparator, Qt::SkipEmptyParts);

			for(auto &ref_name : ref_names)
			{
				// Splitting the column names since they are schema qualified
				aux_names = ref_name.split('.');

				/* We always use the last name in the generated list above in the matching
				 * since it probably contains the name of the column */
				if(regexp.match(aux_names.constLast()).hasMatch())
					list.push_back(obj);
			}
		}
		else if(regexp.match(srch_attribs[search_attr]).hasMatch())
			list.push_back(obj);
	}

	//Removing the duplicate items on the list
	std::sort(list.begin(), list.end());
	end=std::unique(list.begin(), list.end());
	list.erase(end, list.end());

	return list;
}

void DatabaseModel::setInvalidated(bool value)
{
	this->invalidated=value;
}

bool DatabaseModel::isInvalidated()
{
	return invalidated;
}

void  DatabaseModel::setAppendAtEOD(bool value)
{
	append_at_eod=value;
}

void DatabaseModel::setPrependAtBOD(bool value)
{
	prepend_at_bod=value;
}

void DatabaseModel::setDefaultObject(BaseObject *object, ObjectType obj_type)
{
	if((!object && default_objs.count(obj_type)==0) ||
			(object && default_objs.count(object->getObjectType())==0))
		throw Exception(ErrorCode::RefObjectInvalidType, PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!object)
		default_objs[obj_type]=nullptr;
	else
		default_objs[object->getObjectType()]=object;
}

void DatabaseModel::setIsTemplate(bool value)
{
	is_template = value;
}

void DatabaseModel::setAllowConnections(bool value)
{
	allow_conns = value;
}

bool  DatabaseModel::isAppendAtEOD()
{
	return append_at_eod;
}

bool DatabaseModel::isPrependedAtBOD()
{
	return prepend_at_bod;
}

bool DatabaseModel::isTemplate()
{
	return is_template;
}

bool DatabaseModel::isAllowConnections()
{
	return allow_conns;
}

void DatabaseModel::saveObjectsMetadata(const QString &filename, MetaAttrOptions options)
{
	QFile output(filename);
	QByteArray buf;
	QString objs_def;
	QStringList layer_ids;
	std::vector<BaseObject *> objects, tab_objs;
	attribs_map attribs;
	BaseGraphicObject *graph_obj=nullptr;
	Relationship *rel=nullptr;
	PhysicalTable *tab_nn=nullptr;
	BaseTable *src_tab=nullptr, *dst_tab=nullptr, *base_tab=nullptr;
	Schema *schema=nullptr;
	QPointF pnt;
	ObjectType obj_type;
	int idx=0;
	bool save_db_attribs=false, save_objs_pos=false, save_objs_prot=false,
			save_objs_sqldis=false, save_textboxes=false, save_tags=false,
			save_custom_sql=false, save_custom_colors=false, save_fadeout=false,
			save_collapsemode=false, save_genericsqls=false, save_objs_aliases=false,
			save_objs_z_value=false, save_objs_layers_cfg=false;
	QStringList labels_attrs={ Attributes::SrcLabel,
														 Attributes::DstLabel,
														 Attributes::NameLabel };

	save_db_attribs=(MetaDbAttributes & options) == MetaDbAttributes;
	save_objs_pos=(MetaObjsPositioning & options) == MetaObjsPositioning;
	save_objs_prot=(MetaObjsProtection & options) == MetaObjsProtection;
	save_objs_sqldis=(MetaObjsSqlDisabled & options) == MetaObjsSqlDisabled;
	save_textboxes=(MetaTextboxObjs & options) == MetaTextboxObjs;
	save_tags=(MetaTagObjs & options) == MetaTagObjs;
	save_custom_sql=(MetaObjsCustomSql & options) == MetaObjsCustomSql;
	save_custom_colors=(MetaObjsCustomColors & options) == MetaObjsCustomColors;
	save_fadeout=(MetaObjsFadeOut & options) == MetaObjsFadeOut;
	save_collapsemode=(MetaObjsCollapseMode & options) == MetaObjsCollapseMode;
	save_genericsqls=(MetaGenericSqlObjs & options) == MetaGenericSqlObjs;
	save_objs_aliases=(MetaObjsAliases & options) == MetaObjsAliases;
	save_objs_z_value=(MetaObjsZStackValue & options) == MetaObjsZStackValue;
	save_objs_layers_cfg=(MetaObjsLayersConfig & options) == MetaObjsLayersConfig;

	output.open(QFile::WriteOnly);

	if(!output.isOpen())
		throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(filename),
										ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		if(save_textboxes || save_tags || save_genericsqls)
		{
			if(save_textboxes)
				objects.insert(objects.end(), textboxes.begin(), textboxes.end());

			if(save_tags)
				objects.insert(objects.end(), tags.begin(), tags.end());

			if(save_genericsqls)
				objects.insert(objects.end(), genericsqls.begin(), genericsqls.end());
		}

		if(save_db_attribs)
			objects.push_back(this);

		if(save_objs_pos || save_objs_aliases)
		{
			objects.insert(objects.end(), schemas.begin(), schemas.end());
			objects.insert(objects.end(), tables.begin(), tables.end());
			objects.insert(objects.end(), foreign_tables.begin(), foreign_tables.end());
			objects.insert(objects.end(), views.begin(), views.end());
			objects.insert(objects.end(), relationships.begin(), relationships.end());
			objects.insert(objects.end(), base_relationships.begin(), base_relationships.end());

			//Adding rel. n-n generated tables to the list of tables too
			for(BaseObject *object : relationships)
			{
				rel=dynamic_cast<Relationship *>(object);

				if(save_objs_pos && rel->getRelationshipType()==BaseRelationship::RelationshipNn && rel->getReceiverTable())
				{
					tab_nn=rel->getReceiverTable();
					src_tab=rel->getTable(BaseRelationship::SrcTable);
					dst_tab=rel->getTable(BaseRelationship::DstTable);

					//Since the generated table does not have a position we create one based upon the source tables  positions
					pnt.setX((src_tab->getPosition().x() + dst_tab->getPosition().x())/2.0);
					pnt.setY((src_tab->getPosition().y() + dst_tab->getPosition().y())/2.0);
					tab_nn->setPosition(pnt);

					objects.push_back(tab_nn);
				}
			}

			//Saving aliases the children of tables and views
			if(save_objs_aliases)
			{
				for(auto &tab : tables)
				{
					tab_objs = dynamic_cast<Table *>(tab)->getObjects();
					objects.insert(objects.end(), tab_objs.begin(), tab_objs.end());
				}

				for(auto &tab : foreign_tables)
				{
					tab_objs = dynamic_cast<ForeignTable *>(tab)->getObjects();
					objects.insert(objects.end(), tab_objs.begin(), tab_objs.end());
				}

				for(auto &vw : views)
				{
					tab_objs = dynamic_cast<View *>(vw)->getObjects();
					objects.insert(objects.end(), tab_objs.begin(), tab_objs.end());
				}
			}
		}

		if(save_objs_prot || save_objs_sqldis)
		{
			std::vector<ObjectType> types=getChildObjectTypes(ObjectType::Database), sch_types=getChildObjectTypes(ObjectType::Schema);
			types.insert(types.end(), sch_types.begin(), sch_types.end());

			//Removing the types for schema, table and view to avoid retrieving the objects twice
			if(save_objs_pos)
			{
				types.erase(std::find(types.begin(), types.end(), ObjectType::Schema));
				types.erase(std::find(types.begin(), types.end(), ObjectType::Table));
				types.erase(std::find(types.begin(), types.end(), ObjectType::View));
			}

			//Append the other objects to the list of selected entities
			for(ObjectType type : types)
				objects.insert(objects.end(), getObjectList(type)->begin(), getObjectList(type)->end());
		}

		for(auto &object : objects)
		{
			obj_type = object->getObjectType();

			//When handling a tag , textbox or generic sql we just extract their XML code
			if(obj_type==ObjectType::Textbox || obj_type==ObjectType::Tag || obj_type == ObjectType::GenericSql)
			{
				emit s_objectLoaded(((idx++)/static_cast<double>(objects.size()))*100,
														tr("Saving object `%1' (%2)")
														.arg(object->getName()).arg(object->getTypeName()), enum_t(obj_type));

				objs_def+=object->getSourceCode(SchemaParser::XmlCode);
				continue;
			}
			
			//Discarding the relationship added table objects (when extracting aliases)
			if(TableObject::isTableObject(obj_type) && 
				 dynamic_cast<TableObject *>(object)->isAddedByRelationship())
				continue;

			graph_obj=dynamic_cast<BaseGraphicObject *>(object);
			base_tab=dynamic_cast<BaseTable *>(object);

			attribs[Attributes::Table]="";
			attribs[Attributes::Name]=(TableObject::isTableObject(obj_type) ? object->getName() : object->getSignature());
			attribs[Attributes::Alias]=(save_objs_aliases ? object->getAlias() : "");
			attribs[Attributes::Type]=object->getSchemaName();
			attribs[Attributes::Protected]=(save_objs_prot && object->isProtected() && !object->isSystemObject() ? Attributes::True : "");
			attribs[Attributes::SqlDisabled]=(save_objs_sqldis && object->isSQLDisabled() && !object->isSystemObject()  ? Attributes::True : "");
			attribs[Attributes::Tag]=(save_tags && base_tab && base_tab->getTag() ? base_tab->getTag()->getName() : "");
			attribs[Attributes::AppendedSql]=object->getAppendedSQL();
			attribs[Attributes::PrependedSql]=object->getPrependedSQL();			
			attribs[Attributes::FadedOut]=(save_fadeout && graph_obj && graph_obj->isFadedOut() ? Attributes::True : "");
			attribs[Attributes::CollapseMode]=(save_collapsemode && base_tab ? QString::number(enum_t(base_tab->getCollapseMode())) : "");

			//Saving layers information
			if(graph_obj && save_objs_layers_cfg)
			{
				for(auto &layer_id : graph_obj->getLayers())
					layer_ids.append(QString::number(layer_id));

				attribs[Attributes::Layers]=layer_ids.join(',');
				layer_ids.clear();
			}

			if(TableObject::isTableObject(obj_type))
			{
				base_tab = dynamic_cast<TableObject *>(object)->getParentTable();
				attribs[Attributes::Table]=base_tab->getSignature();
			}

			if(save_custom_sql && obj_type==ObjectType::Database)
			{
				attribs[Attributes::AppendAtEod]=(this->isAppendAtEOD() ? Attributes::True : Attributes::False);
				attribs[Attributes::PrependAtBod]=(this->isPrependedAtBOD() ? Attributes::True : Attributes::False);
			}

			//Configuring database model attributes
			if(save_db_attribs && object==this)
			{			
				attribs[Attributes::Author]=this->getAuthor();
				attribs[Attributes::LastPosition]=QString("%1,%2").arg(last_pos.x()).arg(last_pos.y());
				attribs[Attributes::LastZoom]=QString::number(last_zoom);
				attribs[Attributes::DefaultCollation]=(default_objs[ObjectType::Collation] ? default_objs[ObjectType::Collation]->getSignature() : "");
				attribs[Attributes::DefaultSchema]=(default_objs[ObjectType::Schema] ? default_objs[ObjectType::Schema]->getSignature() : "");
				attribs[Attributes::DefaultTablespace]=(default_objs[ObjectType::Tablespace] ? default_objs[ObjectType::Tablespace]->getSignature() : "");
				attribs[Attributes::DefaultOwner]=(default_objs[ObjectType::Role] ? default_objs[ObjectType::Role]->getSignature() : "");
			}

			//Saving database model layers information
			if(save_objs_layers_cfg && object==this)
			{
				for(auto &layer_id : active_layers)
					layer_ids.append(QString::number(layer_id));

				attribs[Attributes::ActiveLayers]= layer_ids.join(',');
				attribs[Attributes::Layers] = layers.join(',');
				attribs[Attributes::ShowLayerNames] = is_layer_names_visible ? Attributes::True : Attributes::False;
				attribs[Attributes::ShowLayerRects] = is_layer_rects_visible ? Attributes::True : Attributes::False;
				attribs[Attributes::LayerRectColors] = layer_rect_colors.join(',');
				attribs[Attributes::LayerNameColors] = layer_name_colors.join(',');
				layer_ids.clear();
			}

			/* If the object is a graphic one and has Z stack value (currently only for tables/view/foreign tables,
			 * textboxes are already save completely in the metadata, so there's no need to separate their z values) */
			if(save_objs_z_value && graph_obj && BaseTable::isBaseTable(graph_obj->getObjectType()))
			{
				attribs[Attributes::ZValue]=QString::number(graph_obj->getZValue());
			}

			//If the object is a graphic one and we need to save positions and colors
			if((save_objs_pos || save_custom_colors) && graph_obj)
			{
				if(obj_type!=ObjectType::BaseRelationship && obj_type!=ObjectType::Relationship)
				{
					pnt=graph_obj->getPosition();

					if(obj_type==ObjectType::Schema)
					{
						schema=dynamic_cast<Schema *>(object);

						attribs[Attributes::CustomColor]=(save_custom_colors ? schema->getFillColor().name() : "");
						attribs[Attributes::RectVisible]=(schema->isRectVisible() ? Attributes::True : Attributes::False);

						if(schema->isRectVisible())
						{
							attribs[Attributes::XPos]=QString::number(pnt.x());
							attribs[Attributes::YPos]=QString::number(pnt.y());
						}
					}
					else
					{
						attribs[Attributes::XPos]=QString::number(pnt.x());
						attribs[Attributes::YPos]=QString::number(pnt.y());
					}

					if(obj_type!=ObjectType::Schema || !attribs[Attributes::XPos].isEmpty())
					{
						schparser.ignoreUnkownAttributes(true);
						attribs[Attributes::Position]=
								schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, Attributes::Position),
																						attribs);
					}
				}
				else
				{
					BaseRelationship *rel=dynamic_cast<BaseRelationship *>(object);
					std::vector<QPointF> points=rel->getPoints();

					attribs_map aux_attribs;

					attribs[Attributes::CustomColor]=(save_custom_colors && rel->getCustomColor()!=Qt::transparent ? rel->getCustomColor().name() : Attributes::None);

					attribs[Attributes::SrcTable]=rel->getTable(BaseRelationship::SrcTable)->getSignature();
					attribs[Attributes::SrcType]=rel->getTable(BaseRelationship::SrcTable)->getSchemaName();

					attribs[Attributes::DstTable]=rel->getTable(BaseRelationship::DstTable)->getSignature();
					attribs[Attributes::DstType]=rel->getTable(BaseRelationship::DstTable)->getSchemaName();

					for(QPointF pnt : points)
					{
						attribs[Attributes::XPos]=QString::number(pnt.x());
						attribs[Attributes::YPos]=QString::number(pnt.y());

						schparser.ignoreUnkownAttributes(true);
						attribs[Attributes::Position]+=
								schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, Attributes::Position),
																						attribs);
					}

					//Saving the labels' custom positions
					for(unsigned id=BaseRelationship::SrcCardLabel; id <= BaseRelationship::RelNameLabel; id++)
					{
						pnt=rel->getLabelDistance(static_cast<BaseRelationship::LabelId>(id));
						if(!std::isnan(pnt.x()) && !std::isnan(pnt.y()))
						{
							aux_attribs[Attributes::XPos]=QString::number(pnt.x());
							aux_attribs[Attributes::YPos]=QString::number(pnt.y());
							aux_attribs[Attributes::RefType]=labels_attrs[id];

							aux_attribs[Attributes::Position]=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, Attributes::Position),
																																						aux_attribs);

							attribs[Attributes::Position]+=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath( GlobalAttributes::XMLSchemaDir, Attributes::Label),
																																				 aux_attribs);

						}
					}
				}
			}

			//Storing the custom SQLs if the object has them configured
			if(save_custom_sql)
			{
				if(!object->getAppendedSQL().isEmpty())
					attribs[Attributes::AppendedSql]=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, QString(Attributes::AppendedSql).remove(QChar('-'))),
																																			 attribs);


				if(!object->getPrependedSQL().isEmpty())
					attribs[Attributes::PrependedSql]=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir,	QString(Attributes::PrependedSql).remove(QChar('-'))),
																																				attribs);

			}

			/* The object's metadata code will be generated only if one of the key attributes
			 for each option were filled previously. */
			if((save_db_attribs && obj_type==ObjectType::Database) ||
				 (save_custom_colors &&
					((obj_type==ObjectType::Relationship || obj_type==ObjectType::BaseRelationship) ||
					 (!attribs[Attributes::CustomColor].isEmpty()))) ||
				 (save_objs_pos &&
					(!attribs[Attributes::Position].isEmpty() ||
					 !attribs[Attributes::RectVisible].isEmpty())) ||
				 (save_tags && !attribs[Attributes::Tag].isEmpty()) ||
				 (save_objs_prot && !attribs[Attributes::Protected].isEmpty()) ||
				 (save_objs_sqldis && !attribs[Attributes::SqlDisabled].isEmpty()) ||
				 (save_custom_sql && (!attribs[Attributes::AppendedSql].isEmpty() ||
															!attribs[Attributes::PrependedSql].isEmpty())) ||
				 (save_fadeout && !attribs[Attributes::FadedOut].isEmpty()) ||
				 (save_collapsemode && !attribs[Attributes::CollapseMode].isEmpty()) ||
				 (save_objs_aliases && !attribs[Attributes::Alias].isEmpty()) ||
				 (save_objs_z_value && !attribs[Attributes::ZValue].isEmpty()) ||
				 (save_objs_layers_cfg && !attribs[Attributes::Layers].isEmpty()))
			{
				emit s_objectLoaded(((idx++)/static_cast<double>(objects.size()))*100,
														tr("Saving metadata of the object `%1' (%2)")
														.arg(object->getSignature()).arg(object->getTypeName()), enum_t(obj_type));

				schparser.ignoreUnkownAttributes(true);

				objs_def +=	schparser.getSourceCode(
											GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir,
																													Attributes::Info), attribs);
			}
			else
				idx++;

			attribs.clear();
		}

		if(!objs_def.isEmpty())
		{
			//Generates the metadata XML buffer
			attribs[Attributes::Info]=objs_def;
			buf.append(schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::XMLSchemaDir, Attributes::Metadata),
																						 attribs).toUtf8());

			output.write(buf.data(),buf.size());

			emit s_objectLoaded(100, tr("Metadata file successfully saved!"), enum_t(ObjectType::BaseObject));
		}
		else
			emit s_objectLoaded(100, tr("Process successfully ended but no metadata was saved!"), enum_t(ObjectType::BaseObject));

		output.close();
	}
	catch(Exception &e)
	{
		if(output.isOpen()) output.close();
		throw Exception(Exception::getErrorMessage(ErrorCode::FileNotWrittenInvalidDefinition).arg(filename),
										ErrorCode::FileNotWrittenInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseModel::loadObjectsMetadata(const QString &filename, MetaAttrOptions options)
{
	QString elem_name, aux_elem, obj_name, ref_type,
			dtd_file=GlobalAttributes::getSchemasRootPath() +
							 GlobalAttributes::DirSeparator +
							 GlobalAttributes::XMLSchemaDir +
							 GlobalAttributes::DirSeparator +
							 GlobalAttributes::ObjectDTDDir +
							 GlobalAttributes::DirSeparator;
	attribs_map attribs, aux_attrib;
	ObjectType obj_type;
	BaseObject *object=nullptr, *new_object=nullptr, *aux_obj = nullptr;
	BaseTable *src_tab=nullptr, *dst_tab=nullptr, *base_tab=nullptr;
	std::vector<QPointF> points;
	std::map<QString, unsigned> labels_attrs;
	std::vector<QPointF> labels_pos={ QPointF(DNaN,DNaN), QPointF(DNaN,DNaN), QPointF(DNaN,DNaN) };
	BaseRelationship *rel=nullptr;
	Schema *schema=nullptr;
	Tag *tag=nullptr;
	int progress=0;
	bool load_db_attribs=false, load_objs_pos=false, load_objs_prot=false,
			load_objs_sqldis=false, load_textboxes=false, load_tags=false,
			load_custom_sql=false, load_custom_colors=false, load_fadeout=false,
			load_collapse_mode=false, load_genericsqls=false, load_objs_aliases=false,
			load_objs_z_value=false, load_objs_layers_cfg=false, merge_dup_objs=false;

	load_db_attribs=(MetaDbAttributes & options) == MetaDbAttributes;
	load_objs_pos=(MetaObjsPositioning & options) == MetaObjsPositioning;
	load_objs_prot=(MetaObjsProtection & options) == MetaObjsProtection;
	load_objs_sqldis=(MetaObjsSqlDisabled & options) == MetaObjsSqlDisabled;
	load_textboxes=(MetaTextboxObjs & options) == MetaTextboxObjs;
	load_tags=(MetaTagObjs & options) == MetaTagObjs;
	load_custom_sql=(MetaObjsCustomSql & options) == MetaObjsCustomSql;
	load_custom_colors=(MetaObjsCustomColors & options) == MetaObjsCustomColors;
	load_fadeout=(MetaObjsFadeOut & options) == MetaObjsFadeOut;
	load_collapse_mode=(MetaObjsCollapseMode & options) == MetaObjsCollapseMode;
	load_genericsqls=(MetaGenericSqlObjs & options) == MetaGenericSqlObjs;
	load_objs_aliases=(MetaObjsAliases & options) == MetaObjsAliases;
	load_objs_z_value=(MetaObjsZStackValue & options) == MetaObjsZStackValue;
	load_objs_layers_cfg=(MetaObjsLayersConfig & options) == MetaObjsLayersConfig;
	merge_dup_objs=(MetaMergeDuplicatedObjs & options) == MetaMergeDuplicatedObjs;

	try
	{
		labels_attrs[Attributes::SrcLabel]=BaseRelationship::SrcCardLabel;
		labels_attrs[Attributes::DstLabel]=BaseRelationship::DstCardLabel;
		labels_attrs[Attributes::NameLabel]=BaseRelationship::RelNameLabel;

		xmlparser.restartParser();

		xmlparser.setDTDFile(dtd_file + GlobalAttributes::MetadataDTD +
												 GlobalAttributes::ObjectDTDExt,
												 GlobalAttributes::MetadataDTD);

		xmlparser.loadXMLFile(filename);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem_name = xmlparser.getElementName();
					obj_type = BaseObject::getObjectType(elem_name);

					xmlparser.getElementAttributes(attribs);

					//Trying to create tag/textbox/generic sql object
					if((obj_type == ObjectType::Tag && load_tags) ||
						 (obj_type == ObjectType::Textbox && load_textboxes) ||
						 (obj_type == ObjectType::GenericSql && load_genericsqls))
					{
						xmlparser.savePosition();
						aux_obj = getObject(attribs[Attributes::Name], obj_type);
						new_object = createObject(obj_type);

						if(!aux_obj)
						{
							emit s_objectLoaded(progress, tr("Creating object `%1' (%2)")
																	.arg(attribs[Attributes::Name])
																	.arg(BaseObject::getTypeName(obj_type)), enum_t(obj_type));

							addObject(new_object);
						}
						else
						{
							emit s_objectLoaded(progress,
																	tr("Object `%1' (%2) already exists. %3.")
																	.arg(attribs[Attributes::Name])
																	.arg(BaseObject::getTypeName(obj_type))
																	.arg(merge_dup_objs ? tr("Merging") : tr("Ignoring")), enum_t(ObjectType::BaseObject));

							if(merge_dup_objs)
								CoreUtilsNs::copyObject(&aux_obj, new_object, obj_type);

							delete new_object;
							new_object = nullptr;
						}

						xmlparser.restorePosition();
					}
					else if(elem_name==Attributes::Info)
					{
						xmlparser.getElementAttributes(attribs);
						obj_name=attribs[Attributes::Object];
						xmlparser.savePosition();

						obj_type=BaseObject::getObjectType(attribs[Attributes::Type]);
						progress=xmlparser.getCurrentBufferLine()/static_cast<double>(xmlparser.getBufferLineCount()) * 100;

						if(obj_type==ObjectType::Database)
						{
							if(load_db_attribs)
							{
								QStringList pos=attribs[Attributes::LastPosition].split(',');

								default_objs[ObjectType::Schema]=getSchema(attribs[Attributes::DefaultSchema]);
								default_objs[ObjectType::Role]=getRole(attribs[Attributes::DefaultOwner]);
								default_objs[ObjectType::Collation]=getCollation(attribs[Attributes::DefaultCollation]);
								default_objs[ObjectType::Tablespace]=getTablespace(attribs[Attributes::DefaultTablespace]);
								author=attribs[Attributes::Author];
								last_zoom=attribs[Attributes::LastZoom].toDouble();

								if(pos.size()>=2)
									last_pos=QPoint(pos[0].toInt(), pos[1].toInt());
							}

							if(load_objs_layers_cfg && !attribs[Attributes::Layers].isEmpty() &&
								 !attribs[Attributes::LayerNameColors].isEmpty() && !attribs[Attributes::LayerRectColors].isEmpty() &&
								 !attribs[Attributes::ShowLayerNames].isEmpty() && !attribs[Attributes::ShowLayerRects].isEmpty())
							{
								active_layers.clear();

								for(auto &layer_id : attribs[Attributes::ActiveLayers].split(',', Qt::SkipEmptyParts))
									active_layers.append(layer_id.toInt());

								layers = attribs[Attributes::Layers].split(',', Qt::SkipEmptyParts);
								layer_name_colors = attribs[Attributes::LayerNameColors].split(',', Qt::SkipEmptyParts);
								layer_rect_colors = attribs[Attributes::LayerRectColors].split(',', Qt::SkipEmptyParts);
								is_layer_names_visible = attribs[Attributes::ShowLayerNames] == Attributes::True;
								is_layer_rects_visible = attribs[Attributes::ShowLayerRects] == Attributes::True;
							}

							object=this;
						}
						else if(TableObject::isTableObject(obj_type))
						{
							base_tab = getTable(attribs[Attributes::Table]);

							if(!base_tab && (obj_type == ObjectType::Rule || obj_type == ObjectType::Index || obj_type == ObjectType::Trigger))
								base_tab = getView(attribs[Attributes::Table]);

							if(base_tab)
								object = base_tab->getObject(attribs[Attributes::Object], obj_type);

							//Discarding the object if it was added by relationship
							if(object && dynamic_cast<TableObject *>(object)->isAddedByRelationship())
								object = nullptr;
						}
						else
							object=getObject(obj_name, obj_type);

						/* If the object does not exists but it is a relationship, we try to get the relationship
						 involving the tables in paramenters src-table and dst-table */
						if(!object && obj_type==ObjectType::Relationship)
						{
							src_tab=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::SrcTable],
																								BaseObject::getObjectType(attribs[Attributes::SrcType])));
							dst_tab=dynamic_cast<BaseTable *>(getObject(attribs[Attributes::DstTable],
																								BaseObject::getObjectType(attribs[Attributes::DstType])));
							object=getRelationship(src_tab, dst_tab);
						}

						if(object)
						{
							emit s_objectLoaded(progress, tr("Loading metadata for object `%1' (%2)")
																	.arg(object->getName()).arg(object->getTypeName()), enum_t(obj_type));

							if(!object->isSystemObject() &&
								 ((!attribs[Attributes::Protected].isEmpty() && load_objs_prot) ||
									(!attribs[Attributes::SqlDisabled].isEmpty() && load_objs_sqldis)))
							{
								if(!attribs[Attributes::Protected].isEmpty())
									object->setProtected(attribs[Attributes::Protected]==Attributes::True);

								if(!attribs[Attributes::SqlDisabled].isEmpty())
									object->setSQLDisabled(attribs[Attributes::SqlDisabled]==Attributes::True);
							}
							else if(BaseTable::isBaseTable(obj_type) && load_tags && !attribs[Attributes::Tag].isEmpty())
							{
								tag=getTag(attribs[Attributes::Tag]);

								if(tag)
									dynamic_cast<BaseTable *>(object)->setTag(tag);
							}
							else if((BaseTable::isBaseTable(obj_type) || obj_type == ObjectType::Textbox) &&
											load_objs_z_value && !attribs[Attributes::ZValue].isEmpty())
							{
								dynamic_cast<BaseTable *>(object)->setZValue(attribs[Attributes::ZValue].toInt());
							}
							else if(obj_type==ObjectType::Database && load_custom_sql)
							{
								if(!attribs[Attributes::AppendAtEod].isEmpty())
									this->setAppendAtEOD(attribs[Attributes::AppendAtEod]==Attributes::True);

								if(!attribs[Attributes::PrependAtBod].isEmpty())
									this->setPrependAtBOD(attribs[Attributes::PrependAtBod]==Attributes::True);
							}

							if(load_objs_aliases && !attribs[Attributes::Alias].isEmpty())
								object->setAlias(attribs[Attributes::Alias]);

							if(load_objs_layers_cfg && BaseGraphicObject::isGraphicObject(obj_type) && !attribs[Attributes::Layers].isEmpty())
								dynamic_cast<BaseGraphicObject *>(object)->setLayers(attribs[Attributes::Layers].split(',', Qt::SkipEmptyParts));

							if(xmlparser.accessElement(XmlParser::ChildElement))
							{
								do
								{
									aux_elem=xmlparser.getElementName();
									xmlparser.getElementAttributes(aux_attrib);

									//Retrieving and storing the points
									if(aux_elem==Attributes::Position)
									{
										points.push_back(QPointF(aux_attrib[Attributes::XPos].toDouble(),
																						 aux_attrib[Attributes::YPos].toDouble()));
									}
									//Retrieving and storing the labels' custom positions
									else if(aux_elem==Attributes::Label)
									{
										ref_type=aux_attrib[Attributes::RefType];
										xmlparser.savePosition();

										if(xmlparser.accessElement(XmlParser::ChildElement))
										{
											xmlparser.getElementAttributes(aux_attrib);
											labels_pos[labels_attrs[ref_type]]=QPointF(aux_attrib[Attributes::XPos].toDouble(),
																																 aux_attrib[Attributes::YPos].toDouble());
										}

										xmlparser.restorePosition();
									}
									else if(load_custom_sql && aux_elem==Attributes::AppendedSql &&
													attribs[Attributes::AppendedSql].isEmpty())
									{
										xmlparser.savePosition();
										xmlparser.accessElement(XmlParser::ChildElement);
										object->setAppendedSQL(xmlparser.getElementContent());
										xmlparser.restorePosition();
									}
									else if(load_custom_sql && aux_elem==Attributes::PrependedSql &&
													attribs[Attributes::PrependedSql].isEmpty())
									{
										xmlparser.savePosition();
										xmlparser.accessElement(XmlParser::ChildElement);
										object->setPrependedSQL(xmlparser.getElementContent());
										xmlparser.restorePosition();
									}
								}
								while(xmlparser.accessElement(XmlParser::NextElement));
							}

							if(BaseGraphicObject::isGraphicObject(obj_type))
							{
								base_tab=dynamic_cast<BaseTable *>(object);
								rel=dynamic_cast<BaseRelationship *>(object);
								schema=dynamic_cast<Schema *>(object);

								if(!rel && !schema && !points.empty() && load_objs_pos)
									dynamic_cast<BaseGraphicObject *>(object)->setPosition(points[0]);
								else if(rel)
								{
									if(load_custom_colors)
									{
										if(attribs[Attributes::CustomColor]==Attributes::None)
											rel->setCustomColor(Qt::transparent);
										else
											rel->setCustomColor(QColor(attribs[Attributes::CustomColor]));
									}

									if(load_objs_pos)
									{
										rel->setPoints(points);

										for(unsigned id=BaseRelationship::SrcCardLabel; id <= BaseRelationship::RelNameLabel; id++)
										{
											rel->setLabelDistance(static_cast<BaseRelationship::LabelId>(id), labels_pos[id]);
											labels_pos[id]=QPointF(DNaN, DNaN);
										}
									}
								}
								else if(schema)
								{
									if(load_custom_colors)
										schema->setFillColor(QColor(attribs[Attributes::CustomColor]));

									schema->setRectVisible(attribs[Attributes::RectVisible]==Attributes::True);
								}

								if(load_fadeout)
									dynamic_cast<BaseGraphicObject *>(object)->setFadedOut(attribs[Attributes::FadedOut]==Attributes::True);

								if(load_collapse_mode && base_tab)
									base_tab->setCollapseMode(static_cast<BaseTable::CollapseMode>(attribs[Attributes::CollapseMode].toUInt()));
							}

							points.clear();

						}
						else if(!object)
						{
							emit s_objectLoaded(progress, tr("Object `%1' (%2) not found. Ignoring metadata.")
																	.arg(obj_name).arg(BaseObject::getTypeName(obj_type)), enum_t(ObjectType::BaseObject));
						}

						xmlparser.restorePosition();

					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		emit s_objectLoaded(100, tr("Metadata file successfully loaded!"), enum_t(ObjectType::BaseObject));
		setObjectsModified();
	}
	catch(Exception &e)
	{
		QString extra_info;

		if(xmlparser.getCurrentElement())
			extra_info=QString(QObject::tr("%1 (line: %2)")).arg(xmlparser.getLoadedFilename()).arg(xmlparser.getCurrentElement()->line);

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
	}
}

void DatabaseModel::setLayers(const QStringList &layers)
{
	this->layers = layers;
}

QStringList DatabaseModel::getLayers()
{
	return layers;
}

void DatabaseModel::setActiveLayers(const QList<unsigned> &layers)
{
	active_layers = layers;
}

QList<unsigned> DatabaseModel::getActiveLayers()
{
	return active_layers;
}

void DatabaseModel::setLayerNameColors(const QStringList &color_names)
{
	layer_name_colors = color_names;
}

QStringList DatabaseModel::getLayerNameColors()
{
	return layer_name_colors;
}

void DatabaseModel::setLayerRectColors(const QStringList &color_names)
{
	layer_rect_colors = color_names;
}

void DatabaseModel::setLayerNamesVisible(bool value)
{
	is_layer_names_visible = value;
}

void DatabaseModel::setLayerRectsVisible(bool value)
{
	is_layer_rects_visible = value;
}

QStringList DatabaseModel::getLayerRectColors()
{
	return layer_rect_colors;
}

bool DatabaseModel::isLayerNamesVisible()
{
	return is_layer_names_visible;
}

bool DatabaseModel::isLayerRectsVisible()
{
	return is_layer_rects_visible;
}

void DatabaseModel::addChangelogEntry(BaseObject *object, Operation::OperType op_type, BaseObject *parent_obj)
{
	if(op_type == Operation::NoOperation || op_type == Operation::ObjMoved)
		return;

	QDateTime date_time = QDateTime::currentDateTime();
	std::map<Operation::OperType, QString> actions = {
		{ Operation::ObjCreated, Attributes::Created },
		{ Operation::ObjRemoved, Attributes::Deleted },
		{ Operation::ObjModified, Attributes::Updated }	};

	QString action = actions[op_type], obj_signature;

	if(!object || (object && TableObject::isTableObject(object->getObjectType()) && !parent_obj))
	{
		QString obj_name = object ? object->getSignature() : "",
				obj_type =  object ? object->getTypeName() : "";

		throw Exception(Exception::getErrorMessage(ErrorCode::InvChangelogEntryValues).arg(obj_name, obj_type, action, date_time.toString(Qt::ISODate)),
										ErrorCode::InvChangelogEntryValues, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(TableObject::isTableObject(object->getObjectType()))
	{
		obj_signature = parent_obj->getSignature() + "." + object->getName();
		changelog.push_back(std::make_tuple(date_time, parent_obj->getSignature(), parent_obj->getObjectType(), Attributes::Updated));
	}
	else
		obj_signature = object->getSignature();

	changelog.push_back(std::make_tuple(date_time, obj_signature, object->getObjectType(), action));
}

void DatabaseModel::addChangelogEntry(const QString &signature, const QString &type, const QString &action, const QString &date)
{
	QDateTime date_time = QDateTime::fromString(date, Qt::ISODate);
	ObjectType obj_type = BaseObject::getObjectType(type);
	QStringList actions = { Attributes::Created, Attributes::Deleted, Attributes::Updated };

	if(signature.isEmpty() || obj_type == ObjectType::BaseObject ||
		 !date_time.isValid() || !actions.contains(action))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::InvChangelogEntryValues).arg(signature, type, action, date),
										ErrorCode::InvChangelogEntryValues, PGM_FUNC, PGM_FILE, PGM_LINE,
										nullptr, QString("%1, %2, %3, %4").arg(signature, type, action, date));
	}

	changelog.push_back(std::make_tuple(date_time, signature, obj_type, action));
}

QStringList DatabaseModel::getFiltersFromChangelog(QDateTime start, QDateTime end)
{
	QStringList filters;
	QString signature, action;
	ObjectType type;
	QDateTime date;

	// Inverting the date range if the start is greater than the end
	if(start.isValid() && end.isValid() && end < start)
	{
		QDateTime aux = start;
		start = end;
		end = aux;
	}

	for(auto &entry : changelog)
	{
		date = std::get<LogDate>(entry);
		signature = std::get<LogSinature>(entry);
		type = std::get<LogObjectType>(entry);
		action = std::get<LogAction>(entry);

		if(((start.isValid() && end.isValid() && date >= start && date <= end) ||
				(start.isValid() && !end.isValid() && date >= start) ||
				(!start.isValid() && end.isValid() && date <= end)))
		{
			filters.append(BaseObject::getSchemaName(type) +
										 UtilsNs::FilterSeparator +
										 signature +
										 UtilsNs::FilterSeparator +
										 UtilsNs::FilterWildcard);
		}
	}

	filters.removeDuplicates();
	return filters;
}

void DatabaseModel::setPersistedChangelog(bool persist)
{
	persist_changelog = persist;
}

bool DatabaseModel::isPersistedChangelog()
{
	return persist_changelog;
}

void DatabaseModel::clearChangelog()
{
	changelog.clear();
}

QDateTime DatabaseModel::getLastChangelogDate()
{
	return changelog.empty() ?
				 QDateTime() : std::get<LogDate>(changelog.back());
}

QDateTime DatabaseModel::getFirstChangelogDate()
{
	return changelog.empty() ?
				 QDateTime() : std::get<LogDate>(changelog.front());
}

unsigned DatabaseModel::getChangelogLength(Operation::OperType op_type)
{
	if(op_type == Operation::NoOperation || op_type == Operation::ObjMoved)
		return changelog.size();

	unsigned cnt = 0;
	std::map<Operation::OperType, QString> actions = {
		{ Operation::ObjCreated, Attributes::Created },
		{ Operation::ObjRemoved, Attributes::Deleted },
		{ Operation::ObjModified, Attributes::Updated }	};

	QString act = actions[op_type];

	for(auto &entry : changelog)
	{
		if(std::get<LogAction>(entry) == act)
			cnt++;
	}

	return cnt;
}

template<class TableClass>
TableClass *DatabaseModel::createPhysicalTable()
{
	attribs_map attribs, aux_attribs;
	QString elem;
	TableClass *table=nullptr;
	TableObject *object=nullptr;
	BaseObject *tag=nullptr;
	ObjectType obj_type;
	std::vector<unsigned> idxs;
	std::vector<QString> names;
	PartitionKey part_key;
	std::vector<PartitionKey> partition_keys;

	try
	{
		table = new TableClass;
		setBasicAttributes(table);
		xmlparser.getElementAttributes(attribs);

		table->setObjectListsCapacity(attribs[Attributes::MaxObjCount].toUInt());
		table->setGenerateAlterCmds(attribs[Attributes::GenAlterCmds]==Attributes::True);
		table->setCollapseMode(attribs[Attributes::CollapseMode].isEmpty() ? BaseTable::NotCollapsed : static_cast<BaseTable::CollapseMode>(attribs[Attributes::CollapseMode].toUInt()));
		table->setPaginationEnabled(attribs[Attributes::Pagination]==Attributes::True);
		table->setCurrentPage(BaseTable::AttribsSection, attribs[Attributes::AttribsPage].toUInt());
		table->setCurrentPage(BaseTable::ExtAttribsSection, attribs[Attributes::ExtAttribsPage].toUInt());
		table->setFadedOut(attribs[Attributes::FadedOut]==Attributes::True);
		table->setLayers(attribs[Attributes::Layers].split(','));

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					elem=xmlparser.getElementName();
					xmlparser.savePosition();
					object=nullptr;

					if(elem==BaseObject::objs_schemas[enum_t(ObjectType::Column)])
						object=createColumn();
					else if(elem==BaseObject::objs_schemas[enum_t(ObjectType::Constraint)])
						object=createConstraint(table);
					else if(elem==BaseObject::objs_schemas[enum_t(ObjectType::Tag)])
					{
						xmlparser.getElementAttributes(aux_attribs);
						tag=getObject(aux_attribs[Attributes::Name], ObjectType::Tag);

						if(!tag)
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
															.arg(attribs[Attributes::Name])
									.arg(BaseObject::getTypeName(ObjectType::Table))
									.arg(aux_attribs[Attributes::Table])
									.arg(BaseObject::getTypeName(ObjectType::Tag))
									, ErrorCode::RefObjectInexistsModel,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						table->setTag(dynamic_cast<Tag *>(tag));
					}
					//Retrieving custom columns / constraint indexes
					else if(elem==Attributes::CustomIdxs)
					{
						xmlparser.getElementAttributes(aux_attribs);
						obj_type=BaseObject::getObjectType(aux_attribs[Attributes::ObjectType]);

						xmlparser.savePosition();

						if(xmlparser.accessElement(XmlParser::ChildElement))
						{
							do
							{
								if(xmlparser.getElementType()==XML_ELEMENT_NODE)
								{
									elem=xmlparser.getElementName();

									//The element <object> stores the index for each object in the current group
									if(elem==Attributes::Object)
									{
										xmlparser.getElementAttributes(aux_attribs);
										names.push_back(aux_attribs[Attributes::Name]);
										idxs.push_back(aux_attribs[Attributes::Index].toUInt());
									}
								}
							}
							while(xmlparser.accessElement(XmlParser::NextElement));

							table->setRelObjectsIndexes(names, idxs, obj_type);
							names.clear();
							idxs.clear();
						}

						xmlparser.restorePosition();
					}
					else if(elem==Attributes::Partitioning)
					{
						xmlparser.getElementAttributes(aux_attribs);
						table->setPartitioningType(aux_attribs[Attributes::Type]);
						xmlparser.savePosition();

						if(xmlparser.accessElement(XmlParser::ChildElement))
						{
							do
							{
								if(xmlparser.getElementType()==XML_ELEMENT_NODE &&
									 xmlparser.getElementName()==Attributes::PartitionKey)
								{
									createElement(part_key, nullptr, table);
									partition_keys.push_back(part_key);
								}
							}
							while(xmlparser.accessElement(XmlParser::NextElement));

							table->addPartitionKeys(partition_keys);
						}

						xmlparser.restorePosition();
					}
					//Retrieving initial data
					else if(elem==Attributes::InitialData)
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);
						table->setInitialData(xmlparser.getElementContent());
						xmlparser.restorePosition();
					}

					if(object)
						table->addObject(object);

					xmlparser.restorePosition();
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		table->setProtected(table->isProtected());
	}
	catch(Exception &e)
	{
		QString extra_info=getErrorExtraInfo();
		xmlparser.restorePosition();

		delete table;
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, extra_info);
	}

	return table;
}

void DatabaseModel::getDataDictionary(attribs_map &datadict, bool browsable, bool split, bool md_format)
{
	int idx = 0;
	BaseTable *base_tab = nullptr;
	std::vector<BaseObject *> objects;
	std::map<QString, BaseObject *> objs_map;
	QString styles, id, dict_index;
	attribs_map attribs, aux_attribs;
	QStringList dict_index_list;
	QString dict_ext = md_format ? ".md" : ".html",
			dict_sch_file = GlobalAttributes::getDictSchemaFilePath(md_format, GlobalAttributes::DataDictSchemaDir),
			item_sch_file = GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Item),
			dict_idx_sch_file = GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::DataDictIndex);

	objects.assign(tables.begin(), tables.end());
	objects.insert(objects.end(), foreign_tables.begin(), foreign_tables.end());
	objects.insert(objects.end(), views.begin(), views.end());
	objects.insert(objects.end(), relationships.begin(), relationships.end());

	// Placing the objects in alphabectical order
	for(auto &obj : objects)
	{
		// Retrieving the generated table if the current object is a relationship (n-n)
		if(obj->getObjectType() == ObjectType::Relationship)
		{
			Relationship *rel = dynamic_cast<Relationship *>(obj);

			if(!rel->getGeneratedTable())
				continue;

			obj = rel->getGeneratedTable();
		}

		id = obj->getSignature().remove(QChar('"'));
		objs_map[id] = obj;
		dict_index_list.push_back(id);
	}

	dict_index_list.sort();
	datadict.clear();

	attribs[Attributes::Styles] = "";
	attribs[Attributes::DataDictIndex] = "";
	attribs[Attributes::Split] = split ? Attributes::True : "";
	attribs[Attributes::Year] = QString::number(QDate::currentDate().year());
	attribs[Attributes::Date] = QDateTime::currentDateTime().toString(Qt::ISODate);
	attribs[Attributes::Version] = GlobalAttributes::PgModelerVersion;

	// Generates the the stylesheet (only for HTML data dicts)
	if(!md_format)
	{
		styles = schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Styles), attribs);

		// If the generation is a standalone HTML the css is embedded
		if(!split)
			attribs[Attributes::Styles] = styles;
		else
			// Otherwise we create a separated stylesheet file
			datadict[Attributes::Styles + ".css"] = styles;
	}

	// Generating individual data dictionaries
	for(auto &itr : objs_map)
	{
		base_tab = dynamic_cast<BaseTable *>( itr.second);

		// Generate the individual data dictionaries
		aux_attribs[Attributes::DataDictIndex] = browsable ? Attributes::True : "";
		aux_attribs[Attributes::Previous] = idx - 1 >= 0 ? dict_index_list.at(idx - 1) : "";
		aux_attribs[Attributes::Next] = (++idx <= dict_index_list.size() - 1) ? dict_index_list.at(idx) : "";	

		if(base_tab->getObjectType() != ObjectType::View)
		{
			Column *col = nullptr;
			std::vector<TableObject *> *cols = dynamic_cast<PhysicalTable *>(base_tab)->getObjectList(ObjectType::Column);
			std::map<Sequence *, QStringList> col_seqs;

			aux_attribs[Attributes::Sequences] = "";

			for(auto & itr : *cols)
			{
				col = dynamic_cast<Column *>(itr);

				if(col->getSequence())
					col_seqs[dynamic_cast<Sequence *>(col->getSequence())].append(col->getName());
			}

			for(auto &itr : col_seqs)
			{
				aux_attribs[Attributes::Sequences] +=
						itr.first->getDataDictionary(md_format, {{ Attributes::Columns, itr.second.join(", ") }});
			}

			col_seqs.clear();
		}
		else
			aux_attribs[Attributes::Sequences] = "";

		attribs[Attributes::Objects] += base_tab->getDataDictionary(split, md_format, aux_attribs);

		// If the generation is configured to be splitted we generate a complete HTML file for the current table
		if(split && !attribs[Attributes::Objects].isEmpty())
		{
			id = itr.first + dict_ext;
			schparser.ignoreEmptyAttributes(true);			
			datadict[id] = schparser.getSourceCode(dict_sch_file, attribs);
			attribs[Attributes::Objects].clear();
		}
	}

	// If the data dictionary is browsable we proceed with the index generation
	if(browsable)
	{
		attribs_map idx_attribs;

		idx_attribs[BaseObject::getSchemaName(ObjectType::Table)] = "";
		idx_attribs[BaseObject::getSchemaName(ObjectType::View)] = "";
		idx_attribs[BaseObject::getSchemaName(ObjectType::ForeignTable)] = "";
		idx_attribs[Attributes::Year] = attribs[Attributes::Year];
		idx_attribs[Attributes::Date] = attribs[Attributes::Date];
		idx_attribs[Attributes::Styles] = "";
		idx_attribs[Attributes::Version] = GlobalAttributes::PgModelerVersion;

		// Generating the index items
		for(auto &item : dict_index_list)
		{
			aux_attribs[Attributes::Split] = attribs[Attributes::Split];
			aux_attribs[Attributes::Item] = item;
			idx_attribs[objs_map[item]->getSchemaName()] += schparser.getSourceCode(item_sch_file, aux_attribs);
		}

		idx_attribs[Attributes::Name] = this->obj_name;
		idx_attribs[Attributes::Split] = attribs[Attributes::Split];

		schparser.ignoreEmptyAttributes(true);
		dict_index = schparser.getSourceCode(dict_idx_sch_file, idx_attribs);
	}

	// If the data dictionary is browsable and splitted the index goes into a separated file
	if(split && browsable)
		datadict[Attributes::Index + dict_ext] = dict_index;
	else if(!split)
	{
		attribs[Attributes::DataDictIndex] = dict_index;
		schparser.ignoreEmptyAttributes(true);
		datadict[Attributes::Database] = schparser.getSourceCode(dict_sch_file, attribs);
	}
}

void DatabaseModel::saveDataDictionary(const QString &path, bool browsable, bool split, bool md_format)
{
	try
	{
		attribs_map datadict;
		QByteArray buffer;
		QFileInfo finfo(path);
		QDir dir;
		QString filename;

		if(split)
		{
			if(finfo.exists() && !finfo.isDir())
				throw Exception(Exception::getErrorMessage(ErrorCode::InvOutputDirectory).arg(path),
												ErrorCode::InvOutputDirectory,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(!finfo.exists())
				dir.mkpath(path);
		}

		getDataDictionary(datadict, browsable, split, md_format);
		filename = path;

		for(auto &itr : datadict)
		{
			if(split)
				filename = path + GlobalAttributes::DirSeparator + itr.first;

			buffer.append(itr.second.toUtf8());
			UtilsNs::saveFile(filename, buffer);
			buffer.clear();
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString DatabaseModel::getChangelogDefinition(bool csv_format)
{
	try
	{
		QString date, type, signature, action,
				buffer, tmpl_line("\"%1\";\"%2\";\"%3\";\"%4\"\n");
		attribs_map attribs;

		for(auto &entry : changelog)
		{
			date = std::get<LogDate>(entry).toString(Qt::ISODate);
			signature = std::get<LogSinature>(entry);
			type = BaseObject::getSchemaName(std::get<LogObjectType>(entry));
			action = std::get<LogAction>(entry);

			if(csv_format)
			{
				buffer += tmpl_line.arg(date, signature, type, action);
			}
			else
			{
				attribs[Attributes::Date] = date;
				attribs[Attributes::Signature] = signature;
				attribs[Attributes::Type] = type;
				attribs[Attributes::Action] = action;
				buffer += schparser.getSourceCode(Attributes::Entry, attribs, SchemaParser::XmlCode);
			}
		}

		if(csv_format)
			return buffer;

		attribs.clear();
		attribs[Attributes::Entry] = buffer;
		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(Attributes::Changelog, attribs, SchemaParser::XmlCode);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void DatabaseModel::setSceneRect(const QRectF &rect)
{
	if(!rect.isValid())
		return;

	setCodeInvalidated(rect != scene_rect);
	scene_rect = rect;
}

QRectF DatabaseModel::getSceneRect()
{
	return scene_rect;
}

void DatabaseModel::setGenDisabledObjsCode(bool value)
{
	setCodeInvalidated(gen_dis_objs_code != value);
	gen_dis_objs_code = value;
}

bool DatabaseModel::isGenDisabledObjsCode()
{
	return gen_dis_objs_code;
}

void DatabaseModel::setShowSysSchemasRects(bool value)
{
	setCodeInvalidated(show_sys_sch_rects != value);
	show_sys_sch_rects = value;
}
