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

#include "modelsdiffhelper.h"
#include <QThread>
#include "utilsns.h"
#include <QDate>
#include "connection.h"
#include "pgsqlversions.h"

const QStringList	ModelsDiffHelper::TableObjsIgnoredAttribs { Attributes::Alias };

const QStringList	ModelsDiffHelper::RolesIgnoredTags {
		Attributes::AppendedSql,	Attributes::PrependedSql
};

const QStringList	ModelsDiffHelper::ObjectsIgnoredAttribs {
	Attributes::MaxObjCount, Attributes::Protected, Attributes::SqlDisabled,
	Attributes::RectVisible, Attributes::FillColor, Attributes::FadedOut,
	Attributes::CollapseMode,	Attributes::AttribsPage, Attributes::ExtAttribsPage,
	Attributes::Pagination,	Attributes::Alias,	Attributes::Layers
};

const QStringList	ModelsDiffHelper::ObjectsIgnoredTags {
	Attributes::Role, Attributes::Tablespace, Attributes::Collation,
	Attributes::Position,	Attributes::AppendedSql,	Attributes::PrependedSql
};

ModelsDiffHelper::ModelsDiffHelper()
{
	diff_canceled=false;
	pgsql_version=PgSqlVersions::DefaulVersion;
	source_model=imported_model=nullptr;
	resetDiffCounter();

	diff_opts[OptKeepClusterObjs]=true;
	diff_opts[OptCascadeMode]=true;
	diff_opts[OptRecreateUnmodifiable]=false;
	diff_opts[OptReplaceModified]=false;
	diff_opts[OptKeepObjectPerms]=true;
	diff_opts[OptReuseSequences]=true;
	diff_opts[OptPreserveDbName]=true;
	diff_opts[OptDontDropMissingObjs]=false;
	diff_opts[OptDropMissingColsConstr]=false;
}

ModelsDiffHelper::~ModelsDiffHelper()
{
	destroyTempObjects();
}

void ModelsDiffHelper::setDiffOption(DiffOptions opt_id, bool value)
{
	if(opt_id > OptDropMissingColsConstr)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(opt_id == OptDropMissingColsConstr)
		diff_opts[opt_id]=value & !diff_opts[OptDropMissingColsConstr];
	else
		diff_opts[opt_id]=value;
}

void ModelsDiffHelper::setForcedRecreateTypeNames(const QStringList &forced_rec_types)
{
	try
	{
		std::vector<ObjectType> obj_types;

		for(auto &tp_name : forced_rec_types)
			obj_types.push_back(BaseObject::getObjectType(tp_name));

		setForcedRecreateTypes(obj_types);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelsDiffHelper::setForcedRecreateTypes(const std::vector<ObjectType> &forced_rec_types)
{

	static const std::vector<ObjectType> inv_types {
		ObjectType::Relationship, ObjectType::Permission,
		ObjectType::Database, ObjectType::Tag,
		ObjectType::Textbox, ObjectType::GenericSql,
		ObjectType::BaseObject, ObjectType::BaseRelationship,
		ObjectType::BaseTable, ObjectType::TypeAttribute,
		ObjectType::Parameter
	};

	for(auto &type : inv_types)
	{
		if(std::find(forced_rec_types.begin(), forced_rec_types.end(), type) != forced_rec_types.end())
		{
			QString type_name = BaseObject::getSchemaName(type);

			if(type == ObjectType::BaseObject)
				type_name = QString::number(enum_t(type)) + " " + tr("(unrecognized id)");

			throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE, nullptr,
											tr("Object type id: %1").arg(type_name));
		}
	}

	forced_recreate_types = forced_rec_types;
}

bool ModelsDiffHelper::isForcedRecreateType(ObjectType obj_type)
{
	return std::find(forced_recreate_types.begin(),
									 forced_recreate_types.end(),
									 obj_type) != forced_recreate_types.end();
}

void ModelsDiffHelper::setPgSQLVersion(const QString pgsql_ver)
{
	this->pgsql_version=pgsql_ver;
}

void ModelsDiffHelper::resetDiffCounter()
{  
	diffs_counter[ObjectsDiffInfo::AlterObject]=0;
	diffs_counter[ObjectsDiffInfo::DropObject]=0;
	diffs_counter[ObjectsDiffInfo::CreateObject]=0;
	diffs_counter[ObjectsDiffInfo::IgnoreObject]=0;
}

QString ModelsDiffHelper::getDiffDefinition()
{
	return diff_def;
}

void ModelsDiffHelper::setModels(DatabaseModel *src_model, DatabaseModel *imp_model)
{
	source_model=src_model;
	imported_model=imp_model;
	filtered_objs.clear();
}

void ModelsDiffHelper::setFilteredObjects(const std::vector<BaseObject *> &objects)
{
	std::vector<Constraint *> constrs;
	ObjectType obj_type;

	filtered_objs.clear();

	for(auto &obj : objects)
	{		
		obj_type = obj->getObjectType();

		// Discarding objects that can't used in partial diff
		if(obj_type == ObjectType::BaseRelationship || obj_type == ObjectType::Textbox ||
			 obj_type == ObjectType::GenericSql || obj_type == ObjectType::Tag)
			continue;

		if(obj_type == ObjectType::Relationship)
		{
			Relationship *rel = dynamic_cast<Relationship *>(obj);
			BaseRelationship::RelType rel_type = rel->getRelationshipType();

			if(rel_type == BaseRelationship::RelationshipGen ||
				 rel_type == BaseRelationship::RelationshipPart)
				filtered_objs[rel->getObjectId()] = rel;

			// For many-to-many relationships we store their constraints (excpet pks) in a separated list
			else if(rel_type == BaseRelationship::RelationshipNn && rel->getGeneratedTable())
			{
				Table *gen_tab = rel->getGeneratedTable();
				Constraint *constr = nullptr;
				filtered_objs[gen_tab->getObjectId()] = gen_tab;

				for(auto &obj : *gen_tab->getObjectList(ObjectType::Constraint))
				{
					constr = dynamic_cast<Constraint *>(obj);

					if(constr->getConstraintType() != ConstraintType::PrimaryKey)
						constrs.push_back(constr);
				}
			}
		}
		else if(BaseTable::isBaseTable(obj_type))
		{
			Constraint *constr = nullptr;
			BaseTable *tab = dynamic_cast<BaseTable *>(obj);
			std::vector<BaseObject *> child_objs = tab->getObjects({ ObjectType::Column });

			filtered_objs[obj->getObjectId()] = obj;

			for(auto &child : child_objs)
			{
				if(child->getObjectType() == ObjectType::Constraint)
				{
					constr = dynamic_cast<Constraint *>(child);

					if(constr->getConstraintType() == ConstraintType::PrimaryKey)
						continue;

					constrs.push_back(constr);
				}
				else
					filtered_objs[child->getObjectId()] = child;
			}
		}
		else
			filtered_objs[obj->getObjectId()] = obj;
	}

	// Putting fks at the end of filtered objects
	for(auto &constr : constrs)
		filtered_objs[BaseObject::getGlobalId() + constr->getObjectId()] = constr;
}

QStringList ModelsDiffHelper::getRelationshipFilters(const std::vector<BaseObject *> &objects, bool use_signature)
{
	Relationship *rel = nullptr;
	QStringList filters;

	for(auto &obj : objects)
	{
		rel = dynamic_cast<Relationship *>(obj);

		if(rel)
		{
			// Creating a filter to force the retrieval of the generated table (relationship n:n)
			if(rel->getRelationshipType() == Relationship::RelationshipNn && rel->getGeneratedTable())
			{
				filters.append(BaseObject::getSchemaName(ObjectType::Table) +
													UtilsNs::FilterSeparator +
													(use_signature ?
														 rel->getGeneratedTable()->getSignature() :
														 rel->getGeneratedTable()->getName()) +
													UtilsNs::FilterSeparator +
													UtilsNs::FilterWildcard);
			}
			// Creating a filter to force the retrieval of the peer tables (inheritance and partitioning)
			else if(rel->getRelationshipType() == Relationship::RelationshipGen ||
							rel->getRelationshipType() == Relationship::RelationshipPart)
			{
				filters.append(BaseObject::getSchemaName(ObjectType::Table) +
													UtilsNs::FilterSeparator +
													(use_signature ?
														 rel->getReceiverTable()->getSignature() :
														 rel->getReceiverTable()->getName()) +
													UtilsNs::FilterSeparator +
													UtilsNs::FilterWildcard);

				filters.append(BaseObject::getSchemaName(ObjectType::Table) +
													UtilsNs::FilterSeparator +
													(use_signature ?
														 rel->getReferenceTable()->getSignature() :
														 rel->getReferenceTable()->getName()) +
													UtilsNs::FilterSeparator +
													UtilsNs::FilterWildcard);
			}
		}
	}

	return filters;
}

unsigned ModelsDiffHelper::getDiffTypeCount(ObjectsDiffInfo::DiffType diff_type)
{
	if(diff_type >= ObjectsDiffInfo::NoDifference)
		throw Exception(ErrorCode::RefElementInvalidIndex ,PGM_FUNC,PGM_FILE,PGM_LINE);

	return diffs_counter[diff_type];
}

void ModelsDiffHelper::diffModels()
{
	try
	{
		if(!source_model || !imported_model)
			throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

		//First, we need to detect the objects to be dropped
		diffModels(ObjectsDiffInfo::DropObject);
		//Second, we will check the objects to be created or modified
		diffModels(ObjectsDiffInfo::CreateObject);

		if(diff_canceled)
			emit s_diffCanceled();
		else
		{
			processDiffInfos();
			emit s_diffFinished();
		}
	}
	catch(Exception &e)
	{
		emit s_diffAborted(Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo()));
	}

	destroyTempObjects();
	resetDiffCounter();
}

void ModelsDiffHelper::cancelDiff()
{
	diff_canceled=true;
}

void ModelsDiffHelper::diffColsInheritance(PhysicalTable *parent_tab, PhysicalTable *child_tab)
{
	if(!parent_tab || !child_tab)
		return;

	Column *child_col = nullptr;

	for(auto &parent_col : *parent_tab->getObjectList(ObjectType::Column))
	{
		child_col = child_tab->getColumn(parent_col->getName());

		//Ignoring columns with sql disabled
		if(parent_col->isSQLDisabled())
			generateDiffInfo(ObjectsDiffInfo::IgnoreObject, parent_col);
		else if(!child_col)
		{
			/* Creating a column that has as parent table the child_tab
			 * This way the proper ALTER...ADD COLUM can be generated */
			child_col = new Column;
			*child_col = *dynamic_cast<Column *>(parent_col);
			child_col->setParentTable(child_tab);
			child_col->setDeclaredInTable(false);
			tmp_objects.push_back(child_col);
			generateDiffInfo(ObjectsDiffInfo::CreateObject, child_col);
		}

		if(diff_canceled)
			break;
	}
}

void ModelsDiffHelper::diffTables(PhysicalTable *src_table, PhysicalTable *imp_table, ObjectsDiffInfo::DiffType diff_type)
{
	ObjectType types[2]={ ObjectType::Column, ObjectType::Constraint };
	std::vector<TableObject *> *tab_objs=nullptr;
	Constraint *constr=nullptr;
	PhysicalTable *ref_tab=nullptr, *comp_tab=nullptr;
	BaseObject *aux_obj=nullptr;

	if(diff_type==ObjectsDiffInfo::DropObject)
	{
		/* In case of DROP, the reference table is the one from the database
	   and the compared table is the one from model */
		ref_tab=imp_table;
		comp_tab=src_table;
	}
	else if(diff_type==ObjectsDiffInfo::CreateObject ||
					diff_type==ObjectsDiffInfo::AlterObject)
	{
		ref_tab=src_table;
		comp_tab=imp_table;
	}

	for(unsigned i=0; i < 2 && !diff_canceled; i++)
	{
		tab_objs=ref_tab->getObjectList(types[i]);

		for(auto &tab_obj : *tab_objs)
		{
			//Get the object from the compared table
			aux_obj=comp_tab->getObject(tab_obj->getName(), tab_obj->getObjectType());
			constr=dynamic_cast<Constraint *>(tab_obj);

			//Ignoring object with sql disabled or check constraints added by generalizations
			if(tab_obj->isSQLDisabled() ||
				 (constr && constr->isAddedByGeneralization() &&
					constr->getConstraintType()==ConstraintType::Check))
			{
				generateDiffInfo(ObjectsDiffInfo::IgnoreObject, tab_obj);
			}
			else
			{
				/* If the current info is ALTER or CREATE, only objects created by generalization or
					 columns created by common relationships will be considered on the comparison. Also,
					foreign keys are discarded here, since they will be compared on the main comparison
					at diffModels() */
				if(aux_obj && diff_type != ObjectsDiffInfo::DropObject &&
						((tab_obj->isAddedByGeneralization() || !tab_obj->isAddedByLinking() ||
							(aux_obj->getObjectType()==ObjectType::Column && tab_obj->isAddedByLinking())) ||
						 (constr && constr->getConstraintType()!=ConstraintType::ForeignKey)))
				{
					//If there are some differences on the XML code of the objects
					if(tab_obj->isCodeDiffersFrom(aux_obj, TableObjsIgnoredAttribs))
						generateDiffInfo(ObjectsDiffInfo::AlterObject, tab_obj, aux_obj);

				}
				/*	If the object does not exists it will generate a drop info and the original
						one (tab_obj) was not included by generalization or partitioning (to avoid drop inherited/copied columns) */
				else if(!aux_obj && !tab_obj->isAddedByGeneralization() && !tab_obj->isAddedByCopy())
				{
					if(diff_type!=ObjectsDiffInfo::DropObject ||
						 (diff_type==ObjectsDiffInfo::DropObject && !diff_opts[OptDontDropMissingObjs]) ||
						 (diff_type==ObjectsDiffInfo::DropObject && diff_opts[OptDropMissingColsConstr]))
						generateDiffInfo(diff_type, tab_obj);
					else
						generateDiffInfo(ObjectsDiffInfo::IgnoreObject, tab_obj);
				}
			}

			if(diff_canceled)
				break;
		}
	}
}

void ModelsDiffHelper::diffModels(ObjectsDiffInfo::DiffType diff_type)
{
	if(diff_canceled)
		return;

	try
	{
		std::map<unsigned, BaseObject *> obj_order;
		BaseObject *object=nullptr, *aux_object=nullptr;
		ObjectType obj_type;
		QString obj_name;
		unsigned idx=0, factor=0, prog=0;
		DatabaseModel *aux_model=nullptr;
		bool objs_differs=false, xml_differs=false;

		if(diff_type == ObjectsDiffInfo::DropObject)
		{
			/* For DROP detection, we must gather the objects from the database in order to check
			 * if they exists on the model. The object drop order here is the inverse of the creation order
			 * on the database */
			obj_order=imported_model->getCreationOrder(SchemaParser::SqlCode, true);
			aux_model=source_model;
			factor=25;
		}
		else if(diff_type==ObjectsDiffInfo::CreateObject || diff_type==ObjectsDiffInfo::AlterObject)
		{
			/* For creation or modification of objects the order followed is the same
			 * as the creation order on the source model */
			if(filtered_objs.empty())
				obj_order = source_model->getCreationOrder(SchemaParser::SqlCode, true, true);
			else
				obj_order = filtered_objs;

			aux_model=imported_model;
			factor=50;
			prog=50;
		}

		for(auto &obj_itr : obj_order)
		{
			object = obj_itr.second;
			obj_type = object->getObjectType();
			idx++;

			/* If this checking the following objects are discarded:
			 * 1) ObjectType::ObjBaseRelationship objects
			 * 2) Objects which SQL code is disabled or system objects
			 * 3) Cluster objects such as roles and tablespaces (when the operatoin is DROP and keep_cluster_objs is true) */
			if(obj_type != ObjectType::BaseRelationship &&
					!object->isSystemObject() && !object->isSQLDisabled() &&

				 ((diff_type == ObjectsDiffInfo::DropObject &&
						(!diff_opts[OptKeepClusterObjs] ||
						 (diff_opts[OptKeepClusterObjs] && obj_type != ObjectType::Role && obj_type != ObjectType::Tablespace))) ||

					 (diff_type != ObjectsDiffInfo::DropObject)))
			{
				emit s_progressUpdated(prog + ((idx/static_cast<double>(obj_order.size())) * factor),
															 tr("Processing object `%1' (%2)...").arg(object->getSignature()).arg(object->getTypeName()),
															 object->getObjectType());

				//Processing objects that are not database, table child object (they are processed further)
				if(obj_type != ObjectType::Database && !TableObject::isTableObject(obj_type))
				{
					/* Processing permissions. If the operation is DROP and keep_obj_perms is true the
					 * the permission is ignored */
					if(obj_type == ObjectType::Permission &&

							((diff_type == ObjectsDiffInfo::DropObject &&
								!diff_opts[OptKeepObjectPerms]) ||

							 (diff_type == ObjectsDiffInfo::CreateObject &&
								(aux_model->getPermissionIndex(dynamic_cast<Permission *>(object), true) < 0 ||
								 !diff_opts[OptKeepObjectPerms]))))
					{
						generateDiffInfo(diff_type, object);
					}
					//Processing relationship (in this case only generalization and patitioning ones are considered)
					else if(obj_type==ObjectType::Relationship)
					{
						PhysicalTable *ref_tab=nullptr, *rec_tab=nullptr;
						Relationship *rel=dynamic_cast<Relationship *>(object);

						rec_tab=dynamic_cast<PhysicalTable *>(aux_model->getObject(rel->getReceiverTable()->getName(true),
																																				{ObjectType::Table, ObjectType::ForeignTable}));

						if(rel->getRelationshipType()==BaseRelationship::RelationshipGen ||
							 rel->getRelationshipType()==BaseRelationship::RelationshipPart)
						{
							Relationship *aux_rel = nullptr;

							ref_tab = dynamic_cast<PhysicalTable *>(aux_model->getObject(rel->getReferenceTable()->getName(true), {ObjectType::Table, ObjectType::ForeignTable}));
							aux_rel = dynamic_cast<Relationship *>(aux_model->getRelationship(ref_tab, rec_tab));

							/* If the receiver table exists on the model generates a info for the relationship,
									otherwise, the generalization will be created automatically when the table is
									created (see table's code defintion) */
							if(rec_tab && !aux_rel)
							{
								diffColsInheritance(ref_tab, rec_tab);
								generateDiffInfo(diff_type, rel);
							}
							/* Special case for partitioning: we detach (drop) and reattach (create) the partition
							 * if the partition bound expression differs from a model to another. This is done only
							 * if the receiver table (partition) exists in the imported model. */
							else if(rel->getRelationshipType()==BaseRelationship::RelationshipPart &&
											rec_tab &&
											aux_model == imported_model &&
											aux_rel && rel->getPartitionBoundingExpr().simplified() !=
											aux_rel->getPartitionBoundingExpr().simplified())
							{
								generateDiffInfo(ObjectsDiffInfo::DropObject, rel);
								generateDiffInfo(ObjectsDiffInfo::CreateObject, rel);
							}
						}
					}
					else if(obj_type != ObjectType::Permission)
					{
						//Get the object from the database
						obj_name = object->getSignature();
						aux_object = aux_model->getObject(obj_name, obj_type);

						//Special case for many-to-many relationships
						if(obj_type == ObjectType::Table && !aux_object)
							aux_object = getRelNNTable(obj_name, aux_model);

						if(diff_type != ObjectsDiffInfo::DropObject && aux_object)
						{
							/* Try to get a diff from the retrieve object and the current object,
							 * comparing only basic attributes like schema, tablespace and owner
							 * this is why the BaseObject::getAlterCode is called */
							objs_differs = !aux_object->BaseObject::getAlterCode(object).isEmpty();

							//If the objects does not differ, try to compare their XML definition
							if(!objs_differs)
							{
								xml_differs = object->isCodeDiffersFrom(aux_object,	ObjectsIgnoredAttribs,
																												obj_type != ObjectType::Role ? ObjectsIgnoredTags : RolesIgnoredTags);
							}

							//If a difference was detected between the objects
							if(objs_differs || xml_differs)
							{
								generateDiffInfo(ObjectsDiffInfo::AlterObject, object, aux_object);

								//If the object is a table, do additional comparision between their child objects
								if(!isForcedRecreateType(object->getObjectType()) &&
									 PhysicalTable::isPhysicalTable(object->getObjectType()))
								{
									PhysicalTable *tab=dynamic_cast<PhysicalTable *>(object),
											*aux_tab=dynamic_cast<PhysicalTable *>(aux_object);

									diffTables(tab, aux_tab, ObjectsDiffInfo::DropObject);
									diffTables(tab, aux_tab, ObjectsDiffInfo::CreateObject);
								}

								/* bjs_differs = */ xml_differs = false;
							}
						}
						else if(!aux_object)
						{
							if(diff_type != ObjectsDiffInfo::DropObject ||
								 (diff_type == ObjectsDiffInfo::DropObject && !diff_opts[OptDontDropMissingObjs]))
								generateDiffInfo(diff_type, object);
							else
								generateDiffInfo(ObjectsDiffInfo::IgnoreObject, object);
						}
					}
				}
				//Comparison for constraints (fks), triggers, rules, indexes
				else if(TableObject::isTableObject(obj_type))
					diffTableObject(dynamic_cast<TableObject *>(object), diff_type);
				//Comparison between model db and the imported db
				else if(diff_type == ObjectsDiffInfo::CreateObject)
				{
					if(!source_model->getAlterCode(imported_model).isEmpty())
						generateDiffInfo(ObjectsDiffInfo::AlterObject, source_model, imported_model);
				}

				if(diff_canceled)
					break;
			}
			else
			{
				generateDiffInfo(ObjectsDiffInfo::IgnoreObject, object);

				emit s_progressUpdated(prog + ((idx/static_cast<double>(obj_order.size())) * factor),
															 tr("Skipping object `%1' (%2)...").arg(object->getSignature(), object->getTypeName()),
															 object->getObjectType());

				if(diff_canceled)
					break;
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void ModelsDiffHelper::diffTableObject(TableObject *tab_obj, ObjectsDiffInfo::DiffType diff_type)
{
	BaseTable *base_tab=nullptr, *aux_base_tab=nullptr;
	ObjectType obj_type=tab_obj->getObjectType();
	QString tab_name, obj_name=tab_obj->getName(true);
	BaseObject *aux_tab_obj=nullptr;

	//Get the parent table of the object
	base_tab=tab_obj->getParentTable();
	tab_name=base_tab->getSignature();

	//If the operation is a DROP, try to get the table from the source mode
	if(diff_type==ObjectsDiffInfo::DropObject)
	{
		aux_base_tab=dynamic_cast<BaseTable *>(source_model->getObject(tab_name, base_tab->getObjectType()));

		//If the table was not found, try to find it between the many-to-many relationships
		if(!aux_base_tab)
			aux_base_tab=dynamic_cast<BaseTable *>(getRelNNTable(tab_name, source_model));
	}
	else if(diff_type==ObjectsDiffInfo::CreateObject || diff_type==ObjectsDiffInfo::AlterObject)
	{
		aux_base_tab=dynamic_cast<BaseTable *>(imported_model->getObject(tab_name, base_tab->getObjectType()));

		//If the table was not found, try to find it between the many-to-many relationships
		if(!aux_base_tab)
			aux_base_tab=dynamic_cast<BaseTable *>(getRelNNTable(obj_name, imported_model));
	}

	if(aux_base_tab)
	{
		if(obj_type==ObjectType::Constraint)
		{
			PhysicalTable *aux_table=dynamic_cast<PhysicalTable *>(aux_base_tab);
			aux_tab_obj=aux_table->getObject(obj_name, obj_type);
		}
		else
			aux_tab_obj=aux_base_tab->getObject(obj_name, obj_type);
	}

	if(!aux_tab_obj)
	{
		if(diff_type!=ObjectsDiffInfo::DropObject ||
			 (diff_type==ObjectsDiffInfo::DropObject && !diff_opts[OptDontDropMissingObjs]))
			generateDiffInfo(diff_type, tab_obj);
		else
			generateDiffInfo(ObjectsDiffInfo::IgnoreObject, tab_obj);
	}
	else if(diff_type!=ObjectsDiffInfo::DropObject && tab_obj->isCodeDiffersFrom(aux_tab_obj, TableObjsIgnoredAttribs))
		generateDiffInfo(ObjectsDiffInfo::AlterObject, tab_obj, aux_tab_obj);
}

BaseObject *ModelsDiffHelper::getRelNNTable(const QString &obj_name, DatabaseModel *model)
{
	std::vector<BaseObject *> *rels=model->getObjectList(ObjectType::Relationship);
	Relationship *rel=nullptr;
	BaseObject *tab=nullptr;

	for(auto &obj : *rels)
	{
		rel=dynamic_cast<Relationship *>(obj);
		if(rel->getRelationshipType()==BaseRelationship::RelationshipNn &&
				rel->getGeneratedTable() && rel->getGeneratedTable()->getSignature()==obj_name)
		{
			tab=rel->getGeneratedTable();
			break;
		}
	}

	return tab;
}

void ModelsDiffHelper::generateDiffInfo(ObjectsDiffInfo::DiffType diff_type, BaseObject *object, BaseObject *old_object)
{
	try
	{
		if(object)
		{
			ObjectsDiffInfo diff_info;

			/* If the info is for ALTER and there is a DROP info on the list,
			 * the object will be recreated instead of modified */
			if((!isForcedRecreateType(object->getObjectType()) ||
					diff_opts[OptRecreateUnmodifiable]) &&
				 diff_type == ObjectsDiffInfo::AlterObject &&
				 isDiffInfoExists(ObjectsDiffInfo::DropObject, old_object, nullptr) &&
				 !isDiffInfoExists(ObjectsDiffInfo::CreateObject, object, nullptr))
			{
				diff_info=ObjectsDiffInfo(ObjectsDiffInfo::CreateObject, object, nullptr);
				diff_infos.push_back(diff_info);
				diffs_counter[ObjectsDiffInfo::CreateObject]++;
				emit s_objectsDiffInfoGenerated(diff_info);
			}
			else if(!isDiffInfoExists(diff_type, object, old_object))
			{
				Column *col=dynamic_cast<Column *>(object),
						*old_col=dynamic_cast<Column *>(old_object);

				/* Special case for columns marked with ALTER.
				 * If the type of them is "serial" or similar then a sequence will be created and the
				 * type of the column changed to "integer" or similar, this because the ALTER command
				 * for columns don't accept the type "serial" */
				if(diff_type==ObjectsDiffInfo::AlterObject && col && old_col &&
						(col->getType()!=old_col->getType() && col->getType().isSerialType()))
				{
					Column *aux_col=new Column;
					Sequence *seq=new Sequence;
					BaseTable *tab=col->getParentTable();
					QString seq_name=QString("%1_%2_seq").arg(tab->getName()).arg(col->getName());

					if(seq_name.length() > BaseObject::ObjectNameMaxLength)
						seq_name.chop(seq_name.length() - BaseObject::ObjectNameMaxLength);

					seq->setName(seq_name);
					seq->setOwner(tab->getOwner());
					seq->setSchema(tab->getSchema());
					seq->setDefaultValues(col->getType().getAliasType());

					//Configure an auxiliary column with the same values of the original one
					(*aux_col)=(*col);
					aux_col->setDefaultValue("");
					//Setting the type as the alias of the serial type
					aux_col->setType(aux_col->getType().getAliasType());
					//Assigns the sequence to the column in order to configure the default value correctly
					aux_col->setSequence(seq);

					/* Creates a new ALTER info with the created column only if we don't need to reuse sequences or if one of the conditions is met:
					 *
					 * 1) if the sequence reusing is enabled but the type of the columns aren't equivalent
					 * 2) the types of columns are equivalent but the sequences used by each columns aren't the same (different name)
					 * 3) the original column (old_col) has not sequence assigned
					 *
					 * The ALTER info will change the columns type as well as define a the default value for the column
					 * as being nextval(seq_name) */
					if(!diff_opts[OptReuseSequences] ||
						 (diff_opts[OptReuseSequences] &&
							(!col->getType().getAliasType().isEquivalentTo(old_col->getType()) ||
							 !old_col->getSequence() ||
							 (old_col->getSequence() && old_col->getSequence()->getSignature() != seq->getSignature()))))
					{
						diff_info=ObjectsDiffInfo(ObjectsDiffInfo::AlterObject, aux_col, col);
						diff_infos.push_back(diff_info);
						diffs_counter[ObjectsDiffInfo::AlterObject]++;
						emit s_objectsDiffInfoGenerated(diff_info);
					}

					if(!diff_opts[OptReuseSequences] || imported_model->getObjectIndex(seq->getSignature(), ObjectType::Sequence) < 0)
					{
						//Creates a CREATE info with the sequence
						diff_info=ObjectsDiffInfo(ObjectsDiffInfo::CreateObject, seq, nullptr);
						diff_infos.push_back(diff_info);
						diffs_counter[ObjectsDiffInfo::CreateObject]++;
						emit s_objectsDiffInfoGenerated(diff_info);
					}
					else if(diff_opts[OptReuseSequences])
					{
						//Removing DROP infos related to the sequence that will be reused
						std::vector<ObjectsDiffInfo>::iterator itr=diff_infos.begin(),
								itr_end=diff_infos.end();

						while(itr!=itr_end)
						{
							if(itr->getDiffType()==ObjectsDiffInfo::DropObject &&
									itr->getObject()->getObjectType()==ObjectType::Sequence &&
									itr->getObject()->getSignature()==seq->getSignature())
							{
								diff_infos.erase(itr);
								break;
							}

							itr++;
						}
					}

					/* Stores the created objects in the temp list in order to be destroyed
					 * at the end of the process. */
					tmp_objects.push_back(aux_col);
					tmp_objects.push_back(seq);
				}
				else
				{
					diff_info=ObjectsDiffInfo(diff_type, object, old_object);
					diff_infos.push_back(diff_info);
					diffs_counter[diff_type]++;
					emit s_objectsDiffInfoGenerated(diff_info);
				}

				/* If the info is for DROP, generate the drop for referer objects of the
				 * one marked to be dropped */
				if((isForcedRecreateType(object->getObjectType()) ||
						diff_opts[OptRecreateUnmodifiable]
						/* diff_opts[OptReplaceModified] */) &&
						diff_type == ObjectsDiffInfo::DropObject)
				{
					std::vector<BaseObject *> ref_objs;
					ObjectType obj_type = object->getObjectType();

					ref_objs = object->getReferences();

					for(auto &obj : ref_objs)
					{
						/* Avoiding columns to be dropped when a sequence linked to them is dropped too. This because
						 * a column can be a reference to a sequence so to avoid drop and recreate that column this one
						 * will not be erased, unless the column does not exists in the model anymore */
						if((obj_type==ObjectType::Sequence && obj->getObjectType()!=ObjectType::Column) &&
								(obj_type!=ObjectType::Sequence && obj->getObjectType()!=ObjectType::BaseRelationship))
							generateDiffInfo(diff_type, obj);

						if(diff_canceled)
							break;
					}
				}
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

bool ModelsDiffHelper::isDiffInfoExists(ObjectsDiffInfo::DiffType  diff_type, BaseObject *object, BaseObject *old_object, bool exact_match)
{
	bool found_diff=false;
	ObjectsDiffInfo aux_diff(diff_type, object, old_object);

	for(ObjectsDiffInfo diff : diff_infos)
	{
		if((exact_match && diff==aux_diff) ||
				(!exact_match &&
				 ((object && diff.getObject()==object) ||
				  (old_object && diff.getOldObject()==old_object))))
		{
			found_diff=true;
			break;
		}
	}

	return found_diff;
}

void ModelsDiffHelper::processDiffInfos()
{
	BaseObject *object=nullptr;
	Relationship *rel=nullptr;
	std::map<unsigned, QString> drop_objs, create_objs, alter_objs, truncate_tabs, create_fks, create_constrs;
	std::vector<BaseObject *> drop_vect, create_vect, drop_cols;
	unsigned schema_id=0, idx=0;
	ObjectsDiffInfo::DiffType diff_type;
	ObjectType obj_type;
	std::map<unsigned, QString>::reverse_iterator ritr, ritr_end;
	attribs_map attribs;
	QString alter_def, no_inherit_def, inherit_def, set_perms,
			unset_perms, col_drop_def, curr_pgsql_ver = BaseObject::getPgSQLVersion();
	SchemaParser schparser;
	Type *type=nullptr;
	std::vector<Type *> types;
	Constraint *constr=nullptr;
	Column *col=nullptr, *aux_col=nullptr;
	PhysicalTable *parent_tab=nullptr;
	bool skip_obj=false;
	QStringList sch_names;

	try
	{
		//Overriding the global PostgreSQL version so the diff code can match the destination server version
		BaseObject::setIgnoreDbVersion(Connection::isDbVersionIgnored());
		BaseObject::setPgSQLVersion(pgsql_version);

		if(!diff_infos.empty())
			emit s_progressUpdated(0, tr("Processing diff infos..."));

		//Reuniting the schema names to inject a SET search_path command
		for(auto &schema : *imported_model->getObjectList(ObjectType::Schema))
			sch_names.push_back(schema->getName(true));

		//Separating the base types
		for(auto &diff : diff_infos)
		{
			type = dynamic_cast<Type *>(diff.getObject());

			if(type && type->getConfiguration()==Type::BaseType)
			{
				type->convertFunctionParameters();
				types.push_back(type);
			}
		}

		for(auto &diff : diff_infos)
		{
			diff_type = diff.getDiffType();
			object = diff.getObject();
			obj_type = object->getObjectType();
			rel = dynamic_cast<Relationship *>(object);
			constr = dynamic_cast<Constraint *>(object);
			col = dynamic_cast<Column *>(object);

			emit s_progressUpdated((idx++/static_cast<double>(diff_infos.size())) * 100,
														 tr("Processing `%1' info for object `%2' (%3)...")
														 .arg(diff.getDiffTypeString()).arg(object->getSignature()).arg(object->getTypeName()),
														 obj_type);

			/* Preliminary verification for check constraints: there is the need to
			 * check if the constraint is added by generalization or if this is not the case
			 * if it already exists in a ancestor table of its parent, this avoid the generation
			 * of commands to create or drop an inherited constraint raising errors when export the diff */
			if(constr && constr->getConstraintType() == ConstraintType::Check)
			{
				parent_tab = dynamic_cast<PhysicalTable *>(constr->getParentTable());
				skip_obj = constr->isAddedByGeneralization();

				for(unsigned i=0; i < parent_tab->getAncestorTableCount() && !skip_obj; i++)
					skip_obj=(parent_tab->getAncestorTable(i)->getConstraint(constr->getName())!=nullptr);

				if(skip_obj)
					continue;
			}
			//Igoring any operation done over inherited columns
			else if(col)
			{
				parent_tab = dynamic_cast<PhysicalTable *>(col->getParentTable());
				skip_obj = col->isAddedByGeneralization();

				for(unsigned i = 0; i < parent_tab->getAncestorTableCount() && !skip_obj; i++)
				{
					aux_col = parent_tab->getAncestorTable(i)->getColumn(col->getName());
					skip_obj = (aux_col && aux_col->getType().getAliasType() == col->getType());
				}

				if(skip_obj)
					continue;
			}

			//Generating the DROP commands
			if(diff_type == ObjectsDiffInfo::DropObject)
			{
				if(rel && (rel->getRelationshipType() == BaseRelationship::RelationshipGen ||
									 rel->getRelationshipType() == BaseRelationship::RelationshipPart))
				{
					//Undoing inheritances
					no_inherit_def+=rel->getAlterRelationshipDefinition(true);
				}
				else if(obj_type == ObjectType::Permission)
				{
					//Unsetting permissions
					unset_perms += object->getDropCode(diff_opts[OptCascadeMode]);
				}
				else
				{
					//Ordinary drop commands for any object except columns
					if(obj_type != ObjectType::Column)
						drop_objs[object->getObjectId()] = getSourceCode(object, true);
					else
					{
						/* Special case for columns: due to cases like inheritance there is the
						 * the need to drop the columns in the normal order of creation to avoid
						 * error like 'drop inherited column' or wrong propagation of drop on all
						 * child tables. */
						drop_cols.push_back(object);
					}
				}
			}
			//Generating the CREATE commands
			else if(diff_type == ObjectsDiffInfo::CreateObject)
			{
				if(rel && (rel->getRelationshipType() == BaseRelationship::RelationshipGen ||
									 rel->getRelationshipType() == BaseRelationship::RelationshipPart))
				{
					//Creating inheritances
					inherit_def += rel->getAlterRelationshipDefinition(false);
				}
				else if(obj_type == ObjectType::Permission)
				{
					//Setting permissions
					set_perms += object->getSourceCode(SchemaParser::SqlCode);
				}
				else
				{
					/* Special case for constaints: the creation commands for these objects are appended at the very end of create commands secion.
						Primary keys, unique keys, check constraints and exclude constraints are created after foreign keys */
					if(object->getObjectType() == ObjectType::Constraint)
					{
						Constraint *constr = dynamic_cast<Constraint *>(object);

						if(constr->getConstraintType() == ConstraintType::ForeignKey)
							create_fks[constr->getObjectId()] = getSourceCode(constr, false);
						// We only create a constraint if the parent is not being created
						else if(create_objs.count(constr->getParentTable()->getObjectId()) == 0 ||
									 /* Special case for unique constraints: in a very specific scenario, when diff against an empty database,
										* unique keys that reference columns added by relationship need to be forcibly created. This is needed because
										* these objects are not created in the parent table's definition, instead, the are created via separated
										* ALTER TABLE command. */
										(create_objs.count(constr->getParentTable()->getObjectId()) &&
										 constr->isReferRelationshipAddedColumns() && constr->getConstraintType() == ConstraintType::Unique))
							create_constrs[constr->getObjectId()] = getSourceCode(constr, false);
					}
					else
					{
						create_objs[object->getObjectId()] = getSourceCode(object, false);

						if(obj_type == ObjectType::Schema)
							sch_names.push_back(object->getName(true));
					}
				}
			}
			//Generating the ALTER commands
			else if(diff_type == ObjectsDiffInfo::AlterObject)
			{
				QString obj_sql, old_obj_sql;
				bool is_forced_create = isForcedRecreateType(object->getObjectType()),
						 is_recreate_unmod = diff_opts[OptRecreateUnmodifiable] && !object->acceptsAlterCommand(),
						 is_replace_mod = diff_opts[OptReplaceModified] && object->acceptsReplaceCommand();

				obj_sql = diff.getObject()->getSourceCode(SchemaParser::SqlCode).simplified();
				old_obj_sql = diff.getOldObject()->getSourceCode(SchemaParser::SqlCode).simplified();

				// If one or more options that controls the recreation of objects is set
				if(obj_type != ObjectType::Database && obj_sql != old_obj_sql &&
						(is_forced_create || is_recreate_unmod || is_replace_mod))
				{
					// Replacing objects that accepts CREATE OR REPLACE
					if(is_replace_mod)
						alter_objs[object->getObjectId()] = getSourceCode(object, false);
					// Recreating objects via DROP and CREATE
					else if(is_recreate_unmod || is_forced_create)
					{
						recreateObject(object, drop_vect, create_vect);

						//Generating the drop for the object's reference
						for(auto &obj : drop_vect)
							drop_objs[obj->getObjectId()] = getSourceCode(obj, true);

						//Generating the create for the object's reference
						for(auto &obj : create_vect)
						{
							//The there is no ALTER info registered for an object's reference
							if(!isDiffInfoExists(ObjectsDiffInfo::AlterObject, nullptr, obj, false))
							{
								/* Special case for constraints, their code will be appeded to a separated variable in order to
								 create them at the end of diff buffer */
								if(obj->getObjectType()==ObjectType::Constraint)
								{
									if(dynamic_cast<Constraint *>(obj)->getConstraintType()==ConstraintType::ForeignKey)
										create_fks[obj->getObjectId()] = getSourceCode(obj, false);
									else
										create_constrs[obj->getObjectId()] = getSourceCode(obj, false);
								}
								else
									create_objs[obj->getObjectId()] = getSourceCode(obj, false);
							}
						}

						drop_vect.clear();
						create_vect.clear();
					}
				}
				else
				{
					if(diff.getOldObject())
						alter_def = diff.getOldObject()->getAlterCode(object);

					if(obj_type == ObjectType::Database && diff_opts[OptPreserveDbName])
						alter_def.remove(QRegularExpression(QString("(ALTER)( )+(DATABASE)( )+(%1)( )+(RENAME)( )+(TO)(.)*(\\n)")
																								.arg(diff.getOldObject()->getSignature()), QRegularExpression::DotMatchesEverythingOption));

					if(!alter_def.isEmpty())
						alter_objs[object->getObjectId()] = alter_def;
				}
			}
		}

		//Creating the shell types declaration right below on the DDL that creates their schemas
		for(auto &type : types)
		{
			schema_id=type->getSchema()->getObjectId();

			if(create_objs.count(schema_id)!=0)
				create_objs[schema_id]+=type->getSourceCode(SchemaParser::SqlCode, true);
			else
				attribs[Attributes::CreateCmds]+=type->getSourceCode(SchemaParser::SqlCode, true);

			type->convertFunctionParameters(true);
		}

		//Generating the drop command for columns
		for(auto &col : drop_cols)
			col_drop_def+=getSourceCode(col, true);

		diff_def.clear();

		if(!drop_objs.empty() || !create_objs.empty() || !alter_objs.empty() ||
			 !create_fks.empty() || !create_constrs.empty() || !inherit_def.isEmpty() ||
			 !no_inherit_def.isEmpty() || !set_perms.isEmpty() || !col_drop_def.isEmpty())
		{
			unsigned create_objs_count=create_objs.size() + create_constrs.size() + create_fks.size();
			bool has_diffs=false;

			sch_names.removeDuplicates();
			has_diffs=(create_objs_count!=0 || alter_objs.size()!=0 || drop_objs.size()!=0);

			//Attributes used on the diff schema file
			attribs[Attributes::HasChanges]=Attributes::True;
			attribs[Attributes::PgModelerVersion]=GlobalAttributes::PgModelerVersion;
			attribs[Attributes::DbModel]=source_model->getName();
			attribs[Attributes::Database]=imported_model->getName();
			attribs[Attributes::Date]=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
			attribs[Attributes::Connection]=imported_model->getName();
			attribs[Attributes::Change]=QString::number(alter_objs.size());
			attribs[Attributes::Create]=QString::number(create_objs_count);
			attribs[Attributes::Drop]=QString::number(drop_objs.size());
			attribs[Attributes::AlterCmds]="";
			attribs[Attributes::DropCmds]="";
			attribs[Attributes::CreateCmds]="";
			attribs[Attributes::ConstrDefs]="";
			attribs[Attributes::FkDefs]="";
			attribs[Attributes::UnsetPerms]=unset_perms;
			attribs[Attributes::SetPerms]=set_perms;
			attribs[Attributes::Function]=(has_diffs && source_model->getObjectCount(ObjectType::Function)!=0 ? Attributes::True : "");
			attribs[Attributes::SearchPath]=(has_diffs ? sch_names.join(',') : "");

			ritr=drop_objs.rbegin();
			ritr_end=drop_objs.rend();

			attribs[Attributes::DropCmds]+=no_inherit_def;

			while(ritr!=ritr_end)
			{
				attribs[Attributes::DropCmds]+=ritr->second;
				ritr++;
			}

			attribs[Attributes::DropCmds]+=col_drop_def;


			for(auto &itr : create_objs)
				attribs[Attributes::CreateCmds]+=itr.second;

			attribs[Attributes::CreateCmds]+=inherit_def;

			for(auto &itr : create_constrs)
				attribs[Attributes::ConstrDefs]+=itr.second;

			for(auto &itr : create_fks)
				attribs[Attributes::FkDefs]+=itr.second;

			for(auto &itr : alter_objs)
				attribs[Attributes::AlterCmds]+=itr.second;

			//Generating the whole diff buffer
			schparser.setPgSQLVersion(pgsql_version, Connection::isDbVersionIgnored());
			diff_def=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, Attributes::Diff),
																					 attribs);
		}

		if(diff_def.isEmpty())
			emit s_progressUpdated(100, tr("No differences detected."));
		else
			emit s_progressUpdated(100, tr("Preparing diff code..."));

		//Restoring the global PostgreSQL version
		BaseObject::setPgSQLVersion(curr_pgsql_ver);
	}
	catch(Exception &e)
	{
		BaseObject::setPgSQLVersion(curr_pgsql_ver);

		for(Type *type : types)
			type->convertFunctionParameters(true);

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString ModelsDiffHelper::getSourceCode(BaseObject *object, bool drop_cmd)
{
	try
	{
		TableObject *tab_obj=dynamic_cast<TableObject *>(object);
		QString cmd;

		/* For columns and constraints it is needed to force the generation of
	   ALTER commands on the parent table */
		if(tab_obj && (tab_obj->getObjectType() == ObjectType::Column || tab_obj->getObjectType() == ObjectType::Constraint))
		{
			bool gen_alter=false;
			PhysicalTable *table=dynamic_cast<PhysicalTable *>(tab_obj->getParentTable());

			gen_alter = table->isGenerateAlterCmds();

			/* Using the method __setGenerateAlterCmds() instead of setGenerateAlterCmds()
			 * because the first it doesn't check if the table is a partition/partitioned
			 * table allowing the generation of ALTER...ADD commands for columns and constraints anyway.
			 * This is needed for the proper generation of diffs for partitioned tables. */
			table->__setGenerateAlterCmds(true);

			if(drop_cmd)
				cmd = tab_obj->getDropCode(diff_opts[OptCascadeMode]);
			else
				cmd = tab_obj->getSourceCode(SchemaParser::SqlCode);

			table->__setGenerateAlterCmds(gen_alter);
		}
		else
		{
			if(drop_cmd)
				cmd = object->getDropCode(diff_opts[OptCascadeMode]);
			else
				cmd = object->getSourceCode(SchemaParser::SqlCode);
		}

		return cmd;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void ModelsDiffHelper::destroyTempObjects()
{
	BaseObject *tmp_obj=nullptr;

	while(!tmp_objects.empty())
	{
		tmp_obj=tmp_objects.back();
		tmp_objects.pop_back();
		delete tmp_obj;
	}

	diff_infos.clear();
}

void ModelsDiffHelper::recreateObject(BaseObject *object, std::vector<BaseObject *> &drop_objs, std::vector<BaseObject *> &create_objs)
{
	if(object &&
			object->getObjectType()!=ObjectType::BaseRelationship &&
			object->getObjectType()!=ObjectType::Relationship &&
			object->getObjectType()!=ObjectType::Database)
	{
		std::vector<BaseObject *> ref_objs;
		BaseObject *aux_obj = nullptr;

		/* If the specified object is not a table's child object,
	   try to get an object from database which name is the same as 'object' */
		if(!TableObject::isTableObject(object->getObjectType()))
			aux_obj=imported_model->getObject(object->getSignature(), object->getObjectType());
		else
		{
			TableObject *tab_obj=dynamic_cast<TableObject *>(object);

			if(tab_obj->getParentTable())
			{
				/* If the object is a table's child object, first we get a table from the database which
				name is the same as the 'object' parent table's name. Sencond, we try to get a child
				object from that table using the same name as the 'object' */
				BaseTable *tab=dynamic_cast<BaseTable *>(imported_model->getObject(tab_obj->getParentTable()->getSignature(), tab_obj->getParentTable()->getObjectType()));
				aux_obj=tab->getObject(tab_obj->getName(true), tab_obj->getObjectType());
			}
		}

		//Get all references to the retrieved object on the database
		ref_objs = aux_obj->getReferences(false, { ObjectType::Permission });

		/* If the to-be recreate object is a constraint check if it's a pk,
		 if so, the fk's linked to it need to be recreated as well */
		if(aux_obj->getObjectType()==ObjectType::Constraint)
		{
			Constraint *constr=dynamic_cast<Constraint *>(aux_obj);

			if(constr->getConstraintType()==ConstraintType::PrimaryKey)
			{
				unsigned i=0, col_cnt=constr->getColumnCount(Constraint::SourceCols);
				std::vector<BaseObject *> ref_aux;
				Constraint *aux_constr=nullptr;

				for(i=0; i < col_cnt; i++)
				{
					//Get the objects referencing the source columns of the pk
					//imported_model->getObjectReferences(constr->getColumn(i, Constraint::SourceCols), ref_aux, false, true);
					ref_aux = constr->getColumn(i, Constraint::SourceCols)->getReferences(false, { ObjectType::Permission });

					//Selecting only fks from the references list
					for(auto &obj : ref_aux)
					{
						aux_constr = dynamic_cast<Constraint *>(obj);

						if(aux_constr && aux_constr->getConstraintType()==ConstraintType::ForeignKey)
							ref_objs.push_back(aux_constr);
					}
				}
			}
		}

		/* Register a drop info for the object only if there is no drop registered previously,
		 avoiding multiple drop statments for the same object */
		if(aux_obj && !isDiffInfoExists(ObjectsDiffInfo::DropObject, aux_obj, nullptr))
			drop_objs.push_back(aux_obj);

		/* Register a create info for the object only if there is no drop or create registered previously,
		 avoiding wrongly recreating the object */
		if(!isDiffInfoExists(ObjectsDiffInfo::DropObject, aux_obj, nullptr) &&
				!isDiffInfoExists(ObjectsDiffInfo::CreateObject, aux_obj, nullptr))
			create_objs.push_back(object);

		//Executing the recreation of the object's references
		for(auto &obj : ref_objs)
			recreateObject(obj, drop_objs, create_objs);
	}
}
