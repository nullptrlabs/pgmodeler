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

#include "modelvalidationhelper.h"
#include "messagebox.h"
#include <QThread>

const QString ModelValidationHelper::SignalMsg { "`%1' (%2)" };

ModelValidationHelper::ModelValidationHelper()
{
	warn_count = error_count = 0;
	handled_objs = total_objs = 0;
	db_model = nullptr;
	conn = nullptr;
	valid_canceled = fix_mode = use_tmp_names = false;

	export_thread=new QThread;
	export_helper.moveToThread(export_thread);

	connect(export_thread, &QThread::started, &export_helper, qOverload<>(&ModelExportHelper::exportToDBMS));
	connect(&export_helper, &ModelExportHelper::s_progressUpdated, this, &ModelValidationHelper::redirectExportProgress);
	connect(&export_helper, &ModelExportHelper::s_exportFinished, this, &ModelValidationHelper::emitValidationFinished);
	connect(&export_helper, &ModelExportHelper::s_exportAborted, this, &ModelValidationHelper::captureThreadError);
}

ModelValidationHelper::~ModelValidationHelper()
{
	export_thread->quit();
	export_thread->wait();
	delete export_thread;
}

void ModelValidationHelper::generateValidationInfo(ValidationInfo::ValType val_type, BaseObject *object, std::vector<BaseObject *> refs)
{
	if(!refs.empty() ||
		 val_type==ValidationInfo::MissingExtension ||
		 (val_type==ValidationInfo::BrokenRelConfig &&
			 std::find(inv_rels.begin(), inv_rels.end(), object)==inv_rels.end()))
	{
		//Configures a validation info
		ValidationInfo info = ValidationInfo(val_type, object, refs);

		if(val_type == ValidationInfo::UniqueSameAsPk)
			warn_count++;
		else
			error_count++;

		val_infos.push_back(info);

		if(val_type==ValidationInfo::BrokenRelConfig)
			inv_rels.push_back(object);

		//Emit the signal containing the info
		emit s_validationInfoGenerated(info);
	}
}

void ModelValidationHelper::resolveConflict(ValidationInfo &info)
{
	try
	{
		std::vector<BaseObject *> refs=info.getReferences();
		BaseObject *obj=nullptr;

		//Resolving broken references by swaping the object ids
		if(info.getValidationType()==ValidationInfo::BrokenReference ||
			 info.getValidationType()==ValidationInfo::SpObjBrokenReference)
		{
			BaseObject *info_obj=info.getObject(), *aux_obj=nullptr;
			unsigned obj_id=info_obj->getObjectId();

			if(info.getValidationType()==ValidationInfo::BrokenReference)
			{
				//Search for the object with the minor id
				while(!refs.empty() && !valid_canceled)
				{
					//For commom broken reference, check if the object id is greater than the reference id
					if(obj_id > refs.back()->getObjectId())
					{
						obj=refs.back();
					}

					//Swap the id of the validation object and the found object (minor id)
					if(obj)
					{
						TableObject *tab_obj=dynamic_cast<TableObject *>(obj);

						if(!tab_obj)
						{
							BaseObject::swapObjectsIds(info_obj, obj, true);
							aux_obj=info_obj;
							emit s_objectIdChanged(obj);
						}
						else if(tab_obj && tab_obj->getParentTable()==info_obj)
						{
							BaseObject::updateObjectId(tab_obj);
							emit s_objectIdChanged(tab_obj);
						}
					}

					if(aux_obj && BaseTable::isBaseTable(aux_obj->getObjectType()))
					{
						std::vector<BaseRelationship *> base_rels=db_model->getRelationships(dynamic_cast<BaseTable *>(aux_obj));
						for(auto &rel : base_rels)
						{
							if(rel->getObjectId() < aux_obj->getObjectId())
							{
								BaseObject::updateObjectId(rel);
								emit s_objectIdChanged(rel);
							}
						}
					}

					refs.pop_back();
					obj=nullptr;
					obj_id=info_obj->getObjectId();
				}
			}
			else
			{
				BaseObject::updateObjectId(info_obj);
			}

			emit s_objectIdChanged(info_obj);
		}
		//Resolving no unique name by renaming the constraints/indexes
		else if(info.getValidationType()==ValidationInfo::NoUniqueName)
		{
			unsigned suffix=1;
			QString new_name;
			BaseTable *table=nullptr;
			ObjectType obj_type;
			BaseObject *obj=info.getObject();
			TableObject *tab_obj=nullptr;

			/* If the last element of the referrer objects is a table or view the
			info object itself need to be renamed since tables and views will not be renamed */
			bool rename_obj=BaseTable::isBaseTable(refs.back()->getObjectType());

			if(rename_obj)
			{
				table = dynamic_cast<TableObject *>(obj)->getParentTable();
				obj_type = obj->getObjectType();

				do
				{
					//Configures a new name for the object [name]_[suffix]
					new_name=QString("%1_%2").arg(obj->getName()).arg(suffix);
					suffix++;
				}
				//Generates a new name until no object is found on parent table
				while(table->getObjectIndex(new_name, obj_type) >= 0);

				//Renames the object
				obj->setName(new_name);
				table->setModified(true);
			}

			//Renaming the referrer objects
			while(!refs.empty() && !valid_canceled)
			{
				obj_type=refs.back()->getObjectType();
				tab_obj=dynamic_cast<TableObject *>(refs.back());

				//Tables and view aren't renamed only table child objects (constraints, indexes)
				if(tab_obj && !tab_obj->isAddedByRelationship())
				{
					table = tab_obj->getParentTable();

					do
					{
						//Configures a new name for the object [name]_[suffix]
						new_name = QString("%1_%2").arg(tab_obj->getName()).arg(suffix);
						suffix++;
					}
					//Generates a new name until no object is found on parent table
					while(table->getObjectIndex(new_name, obj_type) >= 0);

					//Renames the referrer object
					tab_obj->setName(new_name);
					table->setModified(true);
				}

				refs.pop_back();
			}
		}
		//Resolving the absence of postgis extension
		else if(info.getValidationType()==ValidationInfo::MissingExtension && !db_model->getExtension("postgis"))
		{
			Extension *extension = new Extension();			
			extension->setName("postgis");
			extension->setSchema(db_model->getSchema("public"));
			extension->setComment("PostGIS geometry, geography, and raster spatial types and functions");
			db_model->addExtension(extension);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

bool ModelValidationHelper::isValidationCanceled()
{
	return valid_canceled;
}

unsigned ModelValidationHelper::getWarningCount()
{
	return warn_count;
}

unsigned ModelValidationHelper::getErrorCount()
{
	return error_count;
}

void ModelValidationHelper::setValidationParams(DatabaseModel *model, Connection *conn, const QString &pgsql_ver, bool use_tmp_names)
{
	if(!model)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	fix_mode=false;
	valid_canceled=false;
	val_infos.clear();
	inv_rels.clear();
	this->db_model=model;
	this->conn=conn;
	this->pgsql_ver=pgsql_ver;
	this->use_tmp_names=use_tmp_names;
	export_helper.setExportToDBMSParams(this->db_model, conn, pgsql_ver, false, false, false, true, use_tmp_names);
}

void ModelValidationHelper::switchToFixMode(bool value)
{
	fix_mode=value;
}

bool ModelValidationHelper::isInFixMode()
{
	return fix_mode;
}

void ModelValidationHelper::validateModel()
{
	if(!db_model)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		static const std::vector<ObjectType> types {
			ObjectType::Role, ObjectType::Tablespace, ObjectType::Schema, ObjectType::Language, ObjectType::Function,
			ObjectType::Type, ObjectType::Domain, ObjectType::Sequence, ObjectType::Operator, ObjectType::OpFamily,
			ObjectType::OpClass, ObjectType::Collation, ObjectType::Table, ObjectType::Extension, ObjectType::View,
			ObjectType::Relationship, ObjectType::ForeignDataWrapper, ObjectType::ForeignServer, ObjectType::GenericSql,
			ObjectType::ForeignTable, ObjectType::Procedure, ObjectType::Transform
		};

		std::vector<BaseObject *> refs, refs_aux, *obj_list = nullptr, aux_tables;
		std::vector<BaseObject *>::iterator itr;
		std::map<QString, std::vector<BaseObject *> > dup_objects;
		std::map<QString, std::vector<BaseObject *> >::iterator mitr;

		warn_count = error_count = 0;
		handled_objs = total_objs = 0;
		val_infos.clear();
		valid_canceled=false;

		total_objs = db_model->getObjectsCount(types);

		/* Step 1: Validating broken references. This situation happens when a object references another
		 which id is smaller than the id of the first one. */

		for(auto &tp : types)
		{
			if(valid_canceled)
				break;

			obj_list = db_model->getObjectList(tp);
			itr = obj_list->begin();

			for(auto &object : *obj_list)
			{
				if(valid_canceled)
					break;

				//prog++;
				handled_objs++;

				//Excluding the validation of system objects (created automatically)
				if(object->isSystemObject())
					continue;

				emit s_objectProcessed(SignalMsg.arg(object->getName(), object->getTypeName()), object->getObjectType());

				/* Special validation case: For generalization and copy relationships validates the ids of participant tables.
					 Reference table cannot own an id greater thant receiver table */
				checkRelationshipTablesIds(object);

				// Validating broken references to the object
				checkObjectBrokenRefs(object);

				/* Validating a special object. The validation made here is to check if the special object
				 * (constraint/index/trigger/view) references a column added by a relationship and
				 *  that relationship is being created after the creation of the special object */
				checkSpObjectBrokenRefs(object);
			}

			//Emit a signal containing the validation progress
			emit s_progressUpdated((handled_objs / static_cast<double>(total_objs)) * (!conn ? 40 : 20), "");
		}

		/* Step 2: Validating name conflitcs between primary keys, unique keys, exclude constraints
		and indexs of all tables/foreign talbes/views. The tables/views names are checked too. */
		checkConstrNameConflicts();

		// Step 3: Checking if columns of any table is using GiS data types and the postgis extension is not created.
		checkMissingPostgisExt();

		/* Step 4: Checking if there are some invalidated relationship. In some cases, specially with identifier
		 * and generalization relationships,
		 * the columns aren't correctly propagated due to creation order and special behavior of those objects. Thus, in order to
		 * keep all columns synchonized it is need to make this step and change the relationship creation order if needed.
		 * This step is executed only when there is no validation infos generated because for each broken relationship there
		 * is the need to do a revalidation of all relationships */
		checkInvalidatedRels();

		/*Step 5: Checking if unique constraints on tables reference the same columns as the primary keys.
		 * In this, pgModeler emits an alert instead of a validation error this because PostgreSQL will
		 * ignore the unique constraint having the mentioned configuration but without warn the user.
		 * So this step is just here to help the user to understand why PostgreSQL doesn't create the
		 * constraints */
		checkUselessUqConstrs();

		if(!valid_canceled && !fix_mode)
		{
			//Step 6 (optional): Validating the SQL code onto a local DBMS.
			//Case the connection isn't specified indicates that the SQL validation will not be executed
			if(!conn)
			{
				//Emit a signal indicating the final progress
				emitValidationFinished();
			}
			//SQL validation only occurs when the model is completely validated.
			else
			{
				//If there is no errors start the dbms export thread
				if(error_count==0)
				{
					export_thread->start();
					emit s_sqlValidationStarted();
				}
				else
				{
					warn_count++;
					emitValidationFinished();
					emit s_validationInfoGenerated(ValidationInfo(tr("There are pending errors! SQL validation will not be performed.")));
				}
			}
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelValidationHelper::checkRelationshipTablesIds(BaseObject *object)
{
	Relationship *rel = dynamic_cast<Relationship *>(object);

	if(rel && (rel->getRelationshipType()==Relationship::RelationshipGen ||
						 rel->getRelationshipType()==Relationship::RelationshipDep ||
						 rel->getRelationshipType()==Relationship::RelationshipPart))
	{
		PhysicalTable * recv_tab = rel->getReceiverTable(),
				*ref_tab = rel->getReferenceTable();

		if(ref_tab->getObjectId() > recv_tab->getObjectId())
			generateValidationInfo(ValidationInfo::BrokenReference, ref_tab, { recv_tab });
	}
}

void ModelValidationHelper::checkObjectBrokenRefs(BaseObject *object)
{
	std::vector<BaseObject *> refs_aux;
	BaseObject *refer_obj=nullptr;
	TableObject *tab_obj = nullptr;
	Constraint *constr = nullptr;
	Column *col = nullptr;

	for(auto &obj : object->getReferences())
	{
		if(valid_canceled)
			return;

		/* Ignoring system objects that somehow references the object
		 * An example is a schema created by an extension, in this case,
		 * since the schema is created by pgModeler there is no need
		 * to check for broken reference */
		if(obj->isSystemObject())
			continue;

		//Checking if the referrer object is a table object. In this case its parent table is considered
		tab_obj = dynamic_cast<TableObject *>(obj);
		constr = dynamic_cast<Constraint *>(tab_obj);
		col = dynamic_cast<Column *>(tab_obj);

		/*
		 * If the current referrer object has an id less than reference object's id
		 * then it will be pushed into the list of invalid references.
		 * There's an exception which is that foreign keys are completely discarded from any validation
		 * since they are always created at end of code definition being free of any reference breaking.
		 */
		if(object != obj &&
			 (
				 ((col || (constr && constr->getConstraintType() != ConstraintType::ForeignKey)) &&
					(tab_obj->getParentTable()->getObjectId() <= object->getObjectId())) ||
				 (!constr && !col && obj->getObjectId() <= object->getObjectId()))
			 )
		{
			if(col || constr)
				refer_obj = tab_obj->getParentTable();
			else
				refer_obj = obj;

			refs_aux.push_back(refer_obj);
		}
	}

	generateValidationInfo(ValidationInfo::BrokenReference, object, refs_aux);
}

void ModelValidationHelper::checkSpObjectBrokenRefs(BaseObject *object)
{
	if(!BaseTable::isBaseTable(object->getObjectType()) &&
		 object->getObjectType() != ObjectType::GenericSql)
		return;

	std::vector<ObjectType> tab_aux_types { ObjectType::Constraint, ObjectType::Trigger, ObjectType::Index };
	std::vector<TableObject *> *tab_objs;
	std::vector<Column *> ref_cols;
	std::vector<BaseObject *> rels;
	BaseObject *rel = nullptr;
	View *view = nullptr;
	GenericSQL *gen_sql = nullptr;
	Constraint *constr = nullptr;
	PhysicalTable *table = nullptr;

	table = dynamic_cast<PhysicalTable *>(object);
	view = dynamic_cast<View *>(object);
	gen_sql = dynamic_cast<GenericSQL *>(object);

	if(table)
	{
		/* Checking the table children objects if they references some columns added by relationship.
		 * If so, the id of the relationships are swapped with the child object if the first is created
		 * after the latter. */
		for(auto &obj_tp : tab_aux_types)
		{
			tab_objs = table->getObjectList(obj_tp);

			if(!tab_objs)
				continue;

			for(auto &tab_obj : (*tab_objs))
			{
				ref_cols.clear();
				rels.clear();

				if(!tab_obj->isAddedByRelationship())
				{
					if(obj_tp == ObjectType::Constraint)
					{
						constr = dynamic_cast<Constraint *>(tab_obj);

						if(constr->getConstraintType() != ConstraintType::PrimaryKey)
							ref_cols = constr->getRelationshipAddedColumns();
					}
					else if(obj_tp == ObjectType::Trigger)
						ref_cols = dynamic_cast<Trigger *>(tab_obj)->getRelationshipAddedColumns();
					else
						ref_cols = dynamic_cast<Index *>(tab_obj)->getRelationshipAddedColumns();
				}

				//Getting the relationships that owns the columns
				for(auto &ref_col : ref_cols)
				{
					rel = ref_col->getParentRelationship();

					if(rel->getObjectId() > tab_obj->getObjectId() &&
						 std::find(rels.begin(), rels.end(), rel) == rels.end())
						rels.push_back(rel);
				}

				generateValidationInfo(ValidationInfo::SpObjBrokenReference, tab_obj, rels);
			}
		}
	}
	else if(view)
	{
		ref_cols = view->getRelationshipAddedColumns();

		//Getting the relationships that owns the columns
		for(auto &ref_col : ref_cols)
		{
			rel = ref_col->getParentRelationship();

			if(rel->getObjectId() > object->getObjectId() &&
				 std::find(rels.begin(), rels.end(), rel)==rels.end())
				rels.push_back(rel);
		}

		generateValidationInfo(ValidationInfo::SpObjBrokenReference, object, rels);
	}
	else
	{
		Column *col = nullptr;

		for(auto &ref_obj : gen_sql->getDependencies())
		{
			col = dynamic_cast<Column *>(ref_obj);
			if(!col || !col->isAddedByRelationship()) continue;

			rel = col->getParentRelationship();

			if(rel->getObjectId() > object->getObjectId() &&
				 std::find(rels.begin(), rels.end(), rel) == rels.end())
				rels.push_back(rel);
		}

		generateValidationInfo(ValidationInfo::SpObjBrokenReference, object, rels);
	}
}

void ModelValidationHelper::checkConstrNameConflicts()
{
	static const std::vector<ObjectType>
			tab_types { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View },
			tab_obj_types { ObjectType::Constraint, ObjectType::Index };

	unsigned cnt = 0;
	std::vector<BaseObject *> refs, refs_aux, *obj_list = nullptr, aux_tables;
	std::vector<BaseObject *>::iterator itr;
	TableObject *tab_obj=nullptr;
	BaseTable *base_tab = nullptr;
	Constraint *constr=nullptr;
	std::map<QString, std::vector<BaseObject *> > dup_objects;
	std::map<QString, std::vector<BaseObject *> >::iterator mitr;
	QString name;

	/* Step 2: Validating name conflitcs between primary keys, unique keys, exclude constraints
	and indexs of all tables/foreign talbes/views. The tables/views names are checked too. */
	aux_tables = *db_model->getObjectList(ObjectType::Table);
	aux_tables.insert(aux_tables.end(),
										db_model->getObjectList(ObjectType::View)->begin(),
										db_model->getObjectList(ObjectType::View)->end());


	//Searching the model's tables and gathering all the constraints and index
	for(auto &tab : aux_tables)
	{
		if(valid_canceled)
			return;

		base_tab = dynamic_cast<BaseTable *>(tab);
		emit s_objectProcessed(SignalMsg.arg(base_tab->getName(), base_tab->getTypeName()), base_tab->getObjectType());

		for(auto &tb_obj_typ : tab_obj_types)
		{
			if(valid_canceled)
				break;

			cnt = base_tab->getObjectCount(tb_obj_typ);

			for(unsigned idx = 0; idx < cnt; idx++)
			{
				if(valid_canceled)
					break;

				//Get the table object (constraint or index)
				tab_obj = dynamic_cast<TableObject *>(base_tab->getObject(idx, tb_obj_typ));

				//Configures the full name of the object including the parent name
				name = tab_obj->getParentTable()->getSchema()->getName(true) + "." + tab_obj->getName(true);
				name.remove('"');

				//Trying to convert the object to constraint
				constr = dynamic_cast<Constraint *>(tab_obj);

				/* If the object is an index or	a primary key, unique or exclude constraint,
				 * insert the object on duplicated	objects map */
				if((!constr ||
					(constr && (constr->getConstraintType()==ConstraintType::PrimaryKey ||
											constr->getConstraintType()==ConstraintType::Unique ||
											constr->getConstraintType()==ConstraintType::Exclude))))
					dup_objects[name].push_back(tab_obj);
			}
		}
	}

	/* Inserting the tables and views to the map in order to check if there are
	 * other table objects that conflicts with them */
	for(auto &tb_type : tab_types)
	{
		obj_list = db_model->getObjectList(tb_type);
		itr = obj_list->begin();

		while(itr != obj_list->end() && !valid_canceled)
		{
			dup_objects[(*itr)->getSignature(true).remove('"')].push_back(*itr);
			itr++;
		}
	}

	//Checking the map of duplicated objects
	mitr = dup_objects.begin();
	total_objs += dup_objects.size();

	while(mitr != dup_objects.end() && !valid_canceled)
	{
		/* If the vector of the current map element has more the one object
		 * indicates the duplicity thus generates a validation info */
		if(mitr->second.size() > 1)
		{
			refs.assign(mitr->second.begin() + 1, mitr->second.end());
			generateValidationInfo(ValidationInfo::NoUniqueName, mitr->second.front(), refs);
			refs.clear();
		}

		emit s_progressUpdated(21 + ((handled_objs / static_cast<double>(total_objs)) * (!conn ? 40 : 10)), "");
		handled_objs++;
		mitr++;
	}
}

void ModelValidationHelper::checkMissingPostgisExt()
{
	if(db_model->getObjectIndex("postgis", ObjectType::Extension) >= 0)
		return;

	std::vector<BaseObject *> tabs;

	tabs.assign(db_model->getObjectList(ObjectType::Table)->begin(),
							db_model->getObjectList(ObjectType::Table)->end());
	tabs.insert(tabs.end(),
							db_model->getObjectList(ObjectType::ForeignTable)->begin(),
							db_model->getObjectList(ObjectType::ForeignTable)->end());

	auto itr = tabs.begin();
	PhysicalTable *table = nullptr;
	Column *col = nullptr;
	std::vector<TableObject *> *obj_list = nullptr;

	total_objs += tabs.size();

	while(itr != tabs.end() && !valid_canceled)
	{
		table = dynamic_cast<PhysicalTable *>(*itr);
		obj_list = table->getObjectList(ObjectType::Column);
		itr++;

		for(auto &obj : *obj_list)
		{
			col = dynamic_cast<Column *>(obj);

			if(col->getType().isPostGiSType())
				generateValidationInfo(ValidationInfo::MissingExtension, col, {});
		}

		emit s_progressUpdated(31 + ((handled_objs / static_cast<double>(total_objs)) * (!conn ? 40 : 10)), "");
		handled_objs++;
	}
}

void ModelValidationHelper::checkInvalidatedRels()
{
	if(!val_infos.empty())
		return;

	auto obj_list = db_model->getObjectList(ObjectType::Relationship);
	auto itr = db_model->getObjectList(ObjectType::Relationship)->begin();

	total_objs += obj_list->size();

	while(itr != obj_list->end() && !valid_canceled)
	{
		if(dynamic_cast<Relationship *>(*itr)->isInvalidated())
			generateValidationInfo(ValidationInfo::BrokenRelConfig, *itr, {});

		itr++;
		emit s_progressUpdated(41 + ((handled_objs / static_cast<double>(total_objs)) * (!conn ? 40 : 10)), "");
		handled_objs++;
	}
}

void ModelValidationHelper::checkUselessUqConstrs()
{
	PhysicalTable *table = nullptr;
	Constraint *pk = nullptr;
	Constraint *uq = nullptr;
	std::vector<BaseObject *> tabs;

	tabs.assign(db_model->getObjectList(ObjectType::Table)->begin(),
							db_model->getObjectList(ObjectType::Table)->end());

	tabs.insert(tabs.end(),
							db_model->getObjectList(ObjectType::ForeignTable)->begin(),
							db_model->getObjectList(ObjectType::ForeignTable)->end());

	for(auto &tab : tabs)
	{
		if(valid_canceled)
			break;

		table = dynamic_cast<PhysicalTable *>(tab);
		pk = table->getPrimaryKey();

		if(!pk)
			continue;

		for(auto &tab_obj : *table->getObjectList(ObjectType::Constraint))
		{
			uq = dynamic_cast<Constraint *>(tab_obj);

			if(uq->getConstraintType() == ConstraintType::Unique &&
				 uq->isColumnsExist(pk->getColumns(Constraint::SourceCols), Constraint::SourceCols, true))
			{
				generateValidationInfo(ValidationInfo::UniqueSameAsPk, uq, { pk });
			}
		}
	}
}

void ModelValidationHelper::redirectExportProgress(int prog, QString msg, ObjectType obj_type, QString cmd, bool is_code_gen)
{
	if(!export_thread->isRunning())
		return;

	if(prog > 100)
		prog = 100;

	prog = 51 + (prog * 0.40);
	emit s_progressUpdated(prog, msg, obj_type, cmd, is_code_gen);
}

void ModelValidationHelper::applyFixes()
{
	if(fix_mode)
	{
		bool validate_rels=false, found_broken_rels=false;

		while(!val_infos.empty() && !valid_canceled && !found_broken_rels)
		{
			for(unsigned i=0; i < val_infos.size() && !valid_canceled; i++)
			{
				if(!validate_rels)
					validate_rels=(val_infos[i].getValidationType()==ValidationInfo::BrokenReference ||
												 val_infos[i].getValidationType()==ValidationInfo::SpObjBrokenReference ||
												 val_infos[i].getValidationType()==ValidationInfo::NoUniqueName ||
												 val_infos[i].getValidationType()==ValidationInfo::MissingExtension);

				/* Checking if a broken relatinship is found, when this is the case all the pending validation info
		   will not be analyzed until no broken relationship is found */
				if(!found_broken_rels)
					found_broken_rels=(val_infos[i].getValidationType()==ValidationInfo::BrokenRelConfig);

				try
				{
					if(!valid_canceled)
						resolveConflict(val_infos[i]);
				}
				catch(Exception &e)
				{
					emit s_fixFailed(e);
					cancelValidation();
					return;
				}
			}

			emit s_fixApplied();

			if(!valid_canceled && !found_broken_rels)
				validateModel();
		}

		if(!valid_canceled && (found_broken_rels || val_infos.empty()))
		{
			//Emits a signal indicating that the relationships must revalidated
			if(validate_rels || found_broken_rels)
				emit s_relsValidationRequested();

			fix_mode=false;
		}
	}
}

void ModelValidationHelper::cancelValidation()
{
	valid_canceled=true;
	fix_mode=false;
	val_infos.clear();
	export_helper.cancelExport();
	emitValidationCanceled();
}

void ModelValidationHelper::captureThreadError(Exception e)
{
	ValidationInfo val_info(e);
	export_thread->quit();
	export_thread->wait();
	warn_count++;

	/* Indicates the model invalidation only when there are validation warnings (broken refs. or no unique name)
	sql errors are ignored since validator cannot fix SQL related problems */
	db_model->setInvalidated(error_count > 0);

	emit s_validationInfoGenerated(val_info);

	if(val_info.getValidationType()==ValidationInfo::SqlValidationError)
		emit s_validationFinished();
}

void ModelValidationHelper::emitValidationCanceled()
{
	db_model->setInvalidated(!export_thread->isRunning());
	export_thread->quit();
	export_thread->wait();
	emit s_validationInfoGenerated(ValidationInfo(tr("Operation canceled by the user.")));
	emit s_validationCanceled();
}

void ModelValidationHelper::emitValidationFinished()
{
	export_thread->quit();

	/* Indicates the model invalidation only when there are validation warnings (broken refs. or no unique name)
	sql errors are ignored since validator cannot fix SQL related problems */
	db_model->setInvalidated(error_count > 0);
	emit s_validationFinished();

	//progress=100;
	//emit s_progressUpdated(progress,"");
	emit s_progressUpdated(100, "");
}
