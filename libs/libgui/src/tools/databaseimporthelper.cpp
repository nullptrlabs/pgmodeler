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

#include "databaseimporthelper.h"
#include "defaultlanguages.h"
#include "utilsns.h"
#include "coreutilsns.h"

const QString DatabaseImportHelper::UnkownObjectOidXml {"\t<!--[ unknown object OID=%1 ]-->\n"};

DatabaseImportHelper::DatabaseImportHelper(QObject *parent) : QObject(parent)
{
	std::random_device rand_seed;
	rand_num_engine.seed(rand_seed());

	import_canceled=ignore_errors=import_sys_objs=import_ext_objs=false;
	comments_as_aliases=rand_rel_colors=update_fk_rels=is_working_model=false;
	auto_resolve_deps=true;
	import_filter=Catalog::ListAllObjects | Catalog::ExclExtensionObjs | Catalog::ExclSystemObjs;
	xmlparser=nullptr;
	dbmodel=nullptr;

	//Binding create methods
	create_methods = {
		//{ ObjectType::Database, std::bind(&DatabaseImportHelper::configureDatabase, this, std::placeholders::_1) },
		{ ObjectType::Tablespace, std::bind(&DatabaseImportHelper::createTablespace, this, std::placeholders::_1) },
		{ ObjectType::Schema, std::bind(&DatabaseImportHelper::createSchema, this, std::placeholders::_1) },
		{ ObjectType::Role, std::bind(&DatabaseImportHelper::createRole, this, std::placeholders::_1) },
		{ ObjectType::Domain, std::bind(&DatabaseImportHelper::createDomain, this, std::placeholders::_1) },
		{ ObjectType::Extension, std::bind(&DatabaseImportHelper::createExtension, this, std::placeholders::_1) },
		{ ObjectType::Function, std::bind(&DatabaseImportHelper::createFunction, this, std::placeholders::_1) },
		{ ObjectType::Language, std::bind(&DatabaseImportHelper::createLanguage, this, std::placeholders::_1) },
		{ ObjectType::OpFamily, std::bind(&DatabaseImportHelper::createOperatorFamily, this, std::placeholders::_1) },
		{ ObjectType::OpClass, std::bind(&DatabaseImportHelper::createOperatorClass, this, std::placeholders::_1) },
		{ ObjectType::Operator, std::bind(&DatabaseImportHelper::createOperator, this, std::placeholders::_1) },
		{ ObjectType::Collation, std::bind(&DatabaseImportHelper::createCollation, this, std::placeholders::_1) },
		{ ObjectType::Cast, std::bind(&DatabaseImportHelper::createCast, this, std::placeholders::_1) },
		{ ObjectType::Conversion, std::bind(&DatabaseImportHelper::createConversion, this, std::placeholders::_1) },
		{ ObjectType::Sequence, std::bind(&DatabaseImportHelper::createSequence, this, std::placeholders::_1) },
		{ ObjectType::Aggregate, std::bind(&DatabaseImportHelper::createAggregate, this, std::placeholders::_1) },
		{ ObjectType::Type, std::bind(&DatabaseImportHelper::createType, this, std::placeholders::_1) },
		{ ObjectType::Table, std::bind(&DatabaseImportHelper::createTable, this, std::placeholders::_1) },
		{ ObjectType::View, std::bind(&DatabaseImportHelper::createView, this, std::placeholders::_1) },
		{ ObjectType::Rule, std::bind(&DatabaseImportHelper::createRule, this, std::placeholders::_1) },
		{ ObjectType::Trigger, std::bind(&DatabaseImportHelper::createTrigger, this, std::placeholders::_1) },
		{ ObjectType::Index, std::bind(&DatabaseImportHelper::createIndex, this, std::placeholders::_1) },
		{ ObjectType::Constraint, std::bind(&DatabaseImportHelper::createConstraint, this, std::placeholders::_1) },
		{ ObjectType::Policy, std::bind(&DatabaseImportHelper::createPolicy, this, std::placeholders::_1) },
		{ ObjectType::EventTrigger, std::bind(&DatabaseImportHelper::createEventTrigger, this, std::placeholders::_1) },
		{ ObjectType::ForeignDataWrapper, std::bind(&DatabaseImportHelper::createForeignDataWrapper, this, std::placeholders::_1) },
		{ ObjectType::ForeignServer, std::bind(&DatabaseImportHelper::createForeignServer, this, std::placeholders::_1) },
		{ ObjectType::UserMapping, std::bind(&DatabaseImportHelper::createUserMapping, this, std::placeholders::_1) },
		{ ObjectType::ForeignTable, std::bind(&DatabaseImportHelper::createForeignTable, this, std::placeholders::_1) },
		{ ObjectType::Transform, std::bind(&DatabaseImportHelper::createTransform, this, std::placeholders::_1) },
		{ ObjectType::Procedure, std::bind(&DatabaseImportHelper::createProcedure, this, std::placeholders::_1) }
	};
}

void DatabaseImportHelper::setConnection(Connection &conn)
{
	try
	{
		connection.setConnectionParams(conn.getConnectionParams());
		catalog.setConnection(connection);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::closeConnection()
{
	connection.close();
	catalog.closeConnection();
}

void DatabaseImportHelper::setCurrentDatabase(const QString &dbname)
{
	try
	{
		connection.switchToDatabase(dbname);
		catalog.setConnection(connection);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::setSelectedOIDs(DatabaseModel *db_model, const std::map<ObjectType, std::vector<unsigned> > &obj_oids, const std::map<unsigned, std::vector<unsigned> > &col_oids)
{
	if(!db_model)
		throw Exception(ErrorCode::AsgNotAllocattedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	dbmodel=db_model;
	xmlparser=dbmodel->getXMLParser();
	object_oids.insert(obj_oids.begin(), obj_oids.end());
	column_oids.insert(col_oids.begin(), col_oids.end());

	//Fills the creation order vector with the selected OIDs
	creation_order.clear();

	for(auto &itr : object_oids)
		creation_order.insert(creation_order.end(), itr.second.begin(), itr.second.end());

	//Sort the creation order vector to create the object in the correct sequence
	std::sort(creation_order.begin(), creation_order.end());

	user_objs.clear();
	system_objs.clear();
}

void DatabaseImportHelper::setImportOptions(bool import_sys_objs, bool import_ext_objs, bool auto_resolve_deps,
																						bool ignore_errors,	bool debug_mode, bool rand_rel_colors,
																						bool update_rels, bool comments_as_aliases, bool is_working_model)
{
	this->import_sys_objs = import_sys_objs;
	this->import_ext_objs = import_ext_objs;
	this->auto_resolve_deps = auto_resolve_deps;
	this->debug_mode = debug_mode;
	this->rand_rel_colors = rand_rel_colors;
	this->update_fk_rels = update_rels;
	this->comments_as_aliases = comments_as_aliases;
	this->is_working_model = is_working_model;
	this->ignore_errors = is_working_model ? true : ignore_errors;

	Connection::setPrintSQL(debug_mode);

	if(!import_sys_objs && import_ext_objs)
		import_filter = Catalog::ListAllObjects | Catalog::ExclBuiltinArrayTypes | Catalog::ExclSystemObjs;
	else if(import_sys_objs && !import_ext_objs)
		import_filter = Catalog::ListAllObjects | Catalog::ExclBuiltinArrayTypes | Catalog::ExclExtensionObjs;
	else if(import_sys_objs && import_ext_objs)
		import_filter = Catalog::ListAllObjects | Catalog::ExclBuiltinArrayTypes;
	else
		import_filter = Catalog::ListAllObjects | Catalog::ExclBuiltinArrayTypes | Catalog::ExclExtensionObjs | Catalog::ExclSystemObjs;
}

unsigned DatabaseImportHelper::getLastSystemOID()
{
	return catalog.getLastSysObjectOID();
}

QString DatabaseImportHelper::getCurrentDatabase()
{
	return connection.getConnectionParam(Connection::ParamDbName);
}

Catalog DatabaseImportHelper::getCatalog()
{
	return catalog;
}

attribs_map DatabaseImportHelper::getObjects(ObjectType obj_type, const QString &schema, const QString &table, attribs_map extra_attribs)
{
	try
	{
		catalog.setQueryFilter(import_filter);
		return catalog.getObjectsNames(obj_type, schema, table, extra_attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

std::vector<attribs_map> DatabaseImportHelper::getObjects(std::vector<ObjectType> obj_types, const QString &schema, const QString &table, attribs_map extra_attribs)
{
	try
	{
		catalog.setQueryFilter(import_filter);
		return catalog.getObjectsNames(obj_types, schema, table, extra_attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::swapSequencesTablesIds()
{
	BaseObject *table=nullptr, *sequence=nullptr;
	std::map<QString, QString>::iterator itr;

	//Swapping the id's between sequences and tables to avoid reference breaking on SQL code
	itr=seq_tab_swap.begin();
	while(itr!=seq_tab_swap.end())
	{
		sequence=dbmodel->getObject(getObjectName(itr->first), ObjectType::Sequence);
		table=dbmodel->getObject(getObjectName(itr->second), ObjectType::Table);
		if(sequence && table)
			BaseObject::swapObjectsIds(sequence, table, false);
		itr++;
	}
}

void DatabaseImportHelper::retrieveSystemObjects()
{
	int progress=0;
	std::vector<attribs_map>::iterator itr;
	std::map<unsigned, attribs_map> *obj_map=nullptr;
	std::vector<attribs_map> objects;
	std::vector<ObjectType> sys_objs={ ObjectType::Schema, ObjectType::Role, ObjectType::Tablespace,
																ObjectType::Language, ObjectType::Type };
	unsigned i = 0, oid = 0, cnt = sys_objs.size();

	for(i=0; i < cnt && !import_canceled; i++)
	{
		emit s_progressUpdated(progress,
								 tr("Retrieving system objects... `%1'").arg(BaseObject::getTypeName(sys_objs[i])),
							   sys_objs[i]);

		if(sys_objs[i] != ObjectType::Type)
		{
			obj_map=&system_objs;

			if(sys_objs[i] != ObjectType::Language)
				catalog.setQueryFilter(Catalog::ListOnlySystemObjs);
			else
				catalog.setQueryFilter(Catalog::ListAllObjects);
		}
		else
		{
			obj_map = &types;

			/* Only system built in types are loaded initially.
			 * User defined types attributes are retrived only on demand (see getType()) */
			catalog.setQueryFilter(Catalog::ListOnlySystemObjs);
		}

		//Query the objects on the catalog and put them on the map
		objects=catalog.getObjectsAttributes(sys_objs[i]);
		itr=objects.begin();

		while(itr!=objects.end() && !import_canceled)
		{
			oid=itr->at(Attributes::Oid).toUInt();
			(*obj_map)[oid]=(*itr);
			itr++;
		}

		progress=(i/static_cast<double>(cnt))*10;
	}
}

void DatabaseImportHelper::retrieveUserObjects()
{
	int progress = 0;
	std::vector<attribs_map> obj_attribs;
	unsigned i = 0, oid = 0;
	std::map<unsigned, std::vector<unsigned>>::iterator col_itr;
	QStringList names;

	catalog.setQueryFilter(import_filter);

	//Retrieving selected database level objects and table children objects (except columns)
	for(auto &[obj_type, obj_oids] : object_oids)
	{
		if(import_canceled)
			break;

		emit s_progressUpdated(progress,
													 tr("Retrieving objects... `%1'").arg(BaseObject::getTypeName(obj_type)),
													 obj_type);

		obj_attribs = catalog.getObjectsAttributes(obj_type, "", "", obj_oids);

		for(auto &attrs : obj_attribs)
		{
			if(import_canceled)
				break;

			oid = attrs[Attributes::Oid].toUInt();
			user_objs[oid] = attrs;
		}

		obj_attribs.clear();
		progress = (i/static_cast<double>(object_oids.size()))*100;
		i++;
	}

	//Retrieving all selected table columns
	i = 0;
	for(auto &[tab_oid, col_oids] : column_oids)
	{
		if(import_canceled)
			break;

		names = getObjectName(QString::number(tab_oid)).split(".");

		if(names.size() < 2)
			continue;

		emit s_progressUpdated(progress,
													 tr("Retrieving columns of table `%1.%2', oid `%3'...").arg(names[0], names[1]).arg(tab_oid),
													 ObjectType::Column);

		retrieveTableColumns(names[0], names[1], col_oids);

		progress=(i/static_cast<double>(column_oids.size()))*100;
		i++;
	}
}

void DatabaseImportHelper::retrieveTableColumns(const QString &sch_name, const QString &tab_name, std::vector<unsigned> col_ids)
{
	try
	{
		std::vector<attribs_map> cols;
		unsigned tab_oid=0, col_oid;

		cols=catalog.getObjectsAttributes(ObjectType::Column, sch_name, tab_name, col_ids);

		for(auto &itr : cols)
		{
			col_oid=itr.at(Attributes::Oid).toUInt();
			tab_oid=itr.at(Attributes::Table).toUInt();
			columns[tab_oid][col_oid]=itr;
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::createObjects()
{
	int progress=0;
	attribs_map attribs;
	ObjectType obj_type;
	unsigned i=0, oid=0, prev_size=0;
	std::vector<unsigned> not_created_objs, oids;
	std::vector<unsigned>::iterator itr, itr_end;
	std::vector<Exception> aux_errors;
	std::map<unsigned, attribs_map>::iterator itr_objs, itr_objs_end;

	created_objs.reserve(creation_order.size());
	itr_objs = user_objs.begin();
	itr_objs_end = user_objs.end();

	while(itr_objs != itr_objs_end && !import_canceled)
	{
		oid = itr_objs->first;
		attribs = itr_objs->second;
		obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
		i++;
		itr_objs++;

		try
		{
			/* Constraints are ignored in these phase being pushed into an auxiliary list
				 in order to be created later */
			if(obj_type != ObjectType::Constraint)
			{
				emit s_progressUpdated(progress, tr("Creating `%1' (%2), oid `%3'...")
															.arg(/* attribs[Attributes::Name] */ getObjectName(attribs[Attributes::Oid], true))
															.arg(BaseObject::getTypeName(obj_type))
															.arg(attribs[Attributes::Oid]),
															obj_type);

				createObject(attribs);
			}
			else
				constraints.push_back(attribs);
		}
		catch(Exception &)
		{
			//Storing the object id when some error occurs in order to try to create it again below
			not_created_objs.push_back(oid);
		}

		progress=(i/static_cast<double>(creation_order.size())) * 100;
	}

	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: disabling object recreation in reverse engineering."
	#else
		//Trying to recreate objects that failed to be created previously
		if(!not_created_objs.empty())
		{
			unsigned max_tries=10, tries=1;

			do
			{
				/* Store the current size of the objects list. If this size is the same after
			 scan the list recreating the objects means that any object was not created
			 which determines an unrecoverable errors, e.g., objects that references
			 system objects and this ones was not imported */
				prev_size=not_created_objs.size();

				progress=0;
				oids=not_created_objs;
				not_created_objs.clear();
				itr=oids.begin();
				itr_end=oids.end();

				//Scan the oid list recreating the objects
				while(itr!=itr_end && !import_canceled)
				{
					attribs=user_objs[*itr];
					obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
					itr++;

					emit s_progressUpdated(progress,
											 tr("Trying to recreate `%1' (%2), oid `%3'...")
											.arg(/* attribs[Attributes::Name] */ getObjectName(attribs[Attributes::Oid], true))
											.arg(BaseObject::getTypeName(obj_type))
											.arg(attribs[Attributes::Oid]),
							obj_type);

					try
					{
						createObject(attribs);
					}
					catch(Exception &e)
					{
						//In case of some error store the oid and the error in separated lists
						not_created_objs.push_back(*itr);
						aux_errors.push_back(Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC,PGM_FILE,PGM_LINE, &e, dumpObjectAttributes(attribs)));
					}

					progress=(i/static_cast<double>(not_created_objs.size())) * 100;
				}

				tries++;

				if(tries >= max_tries)
					emit s_progressUpdated(progress,
											 tr("Import failed to recreate some objects in `%1' tries.").arg(max_tries),
											 ObjectType::BaseObject);

				if(!import_canceled)
				{
					/* If the previous list size is the same as the not_created_object list means
					 * that no object was created in this interaction which means error */
					if(prev_size==not_created_objs.size() && !ignore_errors)
					{
						throw Exception(aux_errors.back().getErrorMessage(), aux_errors.back().getErrorCode(),
														PGM_FUNC,PGM_FILE,PGM_LINE, aux_errors);
					}

					if(ignore_errors)
						errors.insert(errors.end(), aux_errors.begin(), aux_errors.end());

					aux_errors.clear();
				}
			}
			while(!not_created_objs.empty() && !import_canceled && tries < max_tries);
		}
	#endif
}

void DatabaseImportHelper::createConstraints()
{
	int progress=0;
	attribs_map attribs;
	unsigned i=0;
	std::vector<attribs_map>::iterator itr, itr_end;

	itr = constraints.begin();
	itr_end = constraints.end();

	while(itr != itr_end && !import_canceled)
	{
		attribs = *itr;
		itr++;
		i++;

		try
		{
			//Check constraints are created only if they are not inherited, other types are created normally
			if(attribs[Attributes::Type]!=Attributes::CkConstr ||
					(attribs[Attributes::Type]==Attributes::CkConstr &&
					 attribs[Attributes::Inherited]!=Attributes::True))
			{
				emit s_progressUpdated(progress,
															 tr("Creating `%1' (%2)...")
															 .arg(getObjectName(attribs[Attributes::Oid], true),
																		BaseObject::getTypeName(ObjectType::Constraint)),
															 ObjectType::Constraint);

				createObject(attribs);
			}
		}
		catch(Exception &e)
		{
			if(ignore_errors)
				errors.push_back(Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC,PGM_FILE,PGM_LINE, &e, dumpObjectAttributes(attribs)));
			else
				throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		progress=(i/static_cast<double>(constraints.size())) * 100;
	}
}

void DatabaseImportHelper::createPermissions()
{
	attribs_map attribs;

	try
	{
		unsigned i=0, progress=0;
		std::vector<unsigned>::iterator itr, itr_obj=obj_perms.begin();
		std::map<unsigned, std::vector<unsigned>>::iterator itr_cols=col_perms.begin();
		QString msg=tr("Creating permissions of `%1' (%2)...");
		ObjectType obj_type;

		//Create the object level permission
		while(itr_obj != obj_perms.end() && !import_canceled)
		{
			attribs = getObjectAttributes(*itr_obj);
			obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
			emit s_progressUpdated(progress,
														 msg.arg(getObjectName(attribs[Attributes::Oid]))
														.arg(BaseObject::getTypeName(obj_type)), ObjectType::Permission);

			createPermission(attribs);
			itr_obj++;

			progress=((i++)/static_cast<double>(obj_perms.size())) * 100;
		}

		if(!import_canceled)
			emit s_progressUpdated(progress, tr("Creating columns permissions..."), ObjectType::Permission);

		//Create the column level permission
		i=0;
		while(itr_cols!=col_perms.end() && !import_canceled)
		{
			itr=col_perms[itr_cols->first].begin();

			while(itr!=itr_cols->second.end())
			{
				attribs=columns[itr_cols->first][*itr];
				obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
				emit s_progressUpdated(progress,
									   msg.arg(getObjectName(attribs[Attributes::Oid]))
						.arg(BaseObject::getTypeName(obj_type)), ObjectType::Permission);

				createPermission(attribs);
				itr++;
			}

			itr_cols++;
			progress=((i++)/static_cast<double>(col_perms.size())) * 100;
		}
	}
	catch(Exception &e)
	{
		if(ignore_errors)
			errors.push_back(Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC,PGM_FILE,PGM_LINE, &e, dumpObjectAttributes(attribs)));
		else
			throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::updateFKRelationships()
{
	int progress=0;
	std::vector<BaseObject *>::iterator itr_tab, itr_tab_end;
	unsigned i=0, count=0;
	Table *tab=nullptr;

	itr_tab=dbmodel->getObjectList(ObjectType::Table)->begin();
	itr_tab_end=dbmodel->getObjectList(ObjectType::Table)->end();
	count=dbmodel->getObjectList(ObjectType::Table)->size();
	i=0;

	try
	{
		while(itr_tab!=itr_tab_end && !import_canceled)
		{
			tab=dynamic_cast<Table *>(*itr_tab);

			emit s_progressUpdated(progress,
									 tr("Updating relationships of `%1' (%2)...")
									 .arg(tab->getSignature())
								   .arg(BaseObject::getTypeName(ObjectType::Table)),
								   ObjectType::Table);

			dbmodel->updateTableFKRelationships(tab);

			progress=(i/static_cast<double>(count)) * 90;
			itr_tab++; i++;
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::importDatabase()
{
	try
	{
		if(!dbmodel)
			throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

		BaseGraphicObject::setUpdatesEnabled(false);
		dbmodel->setObjectListsCapacity(creation_order.size());

		cached_names.clear();
		cached_signatures.clear();

		retrieveSystemObjects();
		retrieveUserObjects();
		createObjects();
		createTableInheritances();
		createTablePartitionings();

		/* Forcing the creation of the columns
		 * when importing to the working model. */
		if(is_working_model)
			createColumns();

		createConstraints();
		destroyDetachedColumns();
		createPermissions();

		if(update_fk_rels)
			updateFKRelationships();

		if(!inherited_cols.empty())
		{
			emit s_progressUpdated(100, tr("Validating relationships..."), ObjectType::Relationship);
			dbmodel->validateRelationships();
		}

		BaseGraphicObject::setUpdatesEnabled(true);
		dbmodel->setObjectsModified();

		if(!import_canceled)
		{
			swapSequencesTablesIds();
			assignSequencesToColumns();

			if(!errors.empty())
			{
				QString log_name;

				//Writing the erros to log file
				log_name=GlobalAttributes::getTemporaryFilePath(QString("%1_%2_%3.log").arg(dbmodel->getName())
																												.arg("import")
																												.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss")));

				import_log.setFileName(log_name);
				import_log.open(QFile::WriteOnly);

				for(unsigned i=0; i < errors.size() && import_log.isOpen(); i++)
					import_log.write(errors[i].getExceptionsText().toStdString().c_str());

				import_log.close();

				emit s_importFinished(Exception(tr("The database import ended but some errors were generated and saved into the log file `%1'. This file will last until pgModeler quit.").arg(log_name),
															PGM_FUNC,PGM_FILE,PGM_LINE));
			}
		}
		else
			emit s_importCanceled();

		if(!import_canceled)
		{
			//Generating random colors for relationships
			if(rand_rel_colors)
			{
				std::uniform_int_distribution<unsigned> dist(0,255);
				ObjectType rel_types[] { ObjectType::Relationship, ObjectType::BaseRelationship };
				BaseRelationship *rel=nullptr;

				for(auto & rel_tp : rel_types)
				{
					for(auto &obj : *dbmodel->getObjectList(rel_tp))
					{
						rel = dynamic_cast<BaseRelationship *>(obj);
						rel->setCustomColor(QColor(dist(rand_num_engine),
																			 dist(rand_num_engine),
																			 dist(rand_num_engine)));
					}
				}
			}

			emit s_importFinished();
		}

		resetImportParameters();
	}
	catch(Exception &e)
	{
		BaseGraphicObject::setUpdatesEnabled(true);
		dbmodel->setObjectsModified();
		resetImportParameters();

		/* When running in a separated thread (other than the main application thread)
		 * redirects the error in form of signal */
		if(this->thread() && this->thread() != qApp->thread())
			emit s_importAborted(Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo()));
		else
			//Redirects any error to the user
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void DatabaseImportHelper::setObjectFilters(QStringList filter, bool only_matching, bool match_signature, QStringList force_tab_obj_types)
{
	catalog.setObjectFilters(filter, only_matching, match_signature, force_tab_obj_types);
}

std::map<ObjectType, QString> DatabaseImportHelper::getObjectFilters()
{
	return catalog.getObjectFilters();
}

void DatabaseImportHelper::cancelImport()
{
	import_canceled=true;
}

void DatabaseImportHelper::createObject(attribs_map &attribs)
{
	unsigned oid = attribs[Attributes::Oid].toUInt();
	ObjectType obj_type = static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
	QString obj_name = getObjectName(attribs[Attributes::Oid], (obj_type==ObjectType::Function || obj_type==ObjectType::Operator));

	// Avoiding the creation of pgModeler's temp objects created in database during the catalog reading
	if(obj_name.contains(Catalog::PgModelerTempDbObj))
		return;

	/* If the current object is a system one and it is already exists in the database model we just
	 * register the OID in the list of created objects to avoid creating it further.
	 *
	 * By system objects in this case we mean any object which OID is less than the last system oid
	 * of the database:
	 *
	 * Schema: public, pg_catalog, information_schema
	 * Role: postgres
	 * Collation: C, POSIX, default
	 * Languages: C, SQL, PlPgsql, Internal
	 * Tablespaces: pg_global, pg_default */

	if(catalog.isSystemObject(oid) &&
			(obj_type == ObjectType::Schema || obj_type == ObjectType::Role ||
			 obj_type == ObjectType::Collation || obj_type == ObjectType::Tablespace ||
			 obj_type == ObjectType::Language) &&
			dbmodel->getObjectIndex(obj_name, obj_type) >= 0)
	{
		created_objs.push_back(oid);
		return;
	}

	/* If we are importing to the working model and the object already exists
	 * we just mark it as created */
	if(is_working_model && obj_type != ObjectType::Database)
	{
		bool obj_exists = false;

		/* If the object is a table-child one, we retrieve the parent table from the model
		 * and then check if the object exists in it */
		if(TableObject::isTableObject(obj_type))
		{
			BaseTable *tab = dynamic_cast<BaseTable *>(dbmodel->getObject(getObjectName(attribs[Attributes::Table]),
																																		{ ObjectType::Table,
																																			ObjectType::ForeignTable,
																																			ObjectType::View }));
			obj_exists = (tab && tab->getObjectIndex(attribs[Attributes::Name], obj_type) >= 0);
		}
		else
		{
			// Checking if the object exists in the database model
			obj_exists = dbmodel->getObjectIndex(getObjectName(attribs[Attributes::Oid], true), obj_type) >= 0;

			/* If the object is a table/view/foreign table, we need to retrieve the column attributes
			 * from the database so constraints and other objects to be created can find them */
			if(BaseTable::isBaseTable(obj_type) && !columns.count(oid))
				retrieveTableColumns(getObjectName(attribs[Attributes::Schema]), attribs[Attributes::Name]);
		}

		if(obj_exists)
		{
			created_objs.push_back(oid);
			return;
		}
	}

	try
	{
		if(!import_canceled && std::find(created_objs.begin(), created_objs.end(), oid)==created_objs.end())
		{
			if(TableObject::isTableObject(obj_type))
				attribs[Attributes::DeclInTable]="";

			//System objects will have the sql disabled by default
			attribs[Attributes::SqlDisabled]=(catalog.isSystemObject(oid) || catalog.isExtensionObject(oid) ? Attributes::True : "");

			if(comments_as_aliases &&
					(BaseGraphicObject::isGraphicObject(obj_type) || TableObject::isTableObject(obj_type)))
				attribs[Attributes::Alias] = attribs[Attributes::Comment].mid(0, BaseObject::ObjectNameMaxLength - 1);

			attribs[Attributes::Comment]=getComment(attribs);

			if(attribs.count(Attributes::Owner))
				attribs[Attributes::Owner]=getDependencyObject(attribs[Attributes::Owner], ObjectType::Role, false, auto_resolve_deps);

			if(attribs.count(Attributes::Tablespace))
				attribs[Attributes::Tablespace]=getDependencyObject(attribs[Attributes::Tablespace], ObjectType::Tablespace, false, auto_resolve_deps);

			if(attribs.count(Attributes::Schema))
			{
				//Here we preserve the schema oid for latter usage in certain methods
				attribs[Attributes::SchemaOid]=attribs[Attributes::Schema];
				attribs[Attributes::Schema]=getDependencyObject(attribs[Attributes::Schema], ObjectType::Schema, false, auto_resolve_deps);
			}

			/* Due to the object recreation mechanism there are some situations when pgModeler fails to recreate
			them due to the duplication of permissions. So, to avoid this problem we need to check if the OID of the
			object was previously registered in the vector of permissions to be created */
			if(!attribs[Attributes::Permission].isEmpty() &&
				 std::find(obj_perms.begin(), obj_perms.end(), oid)==obj_perms.end())
				obj_perms.push_back(oid);

			if(debug_mode)
			{
				qDebug().noquote() << dumpObjectAttributes(attribs);
			}

			if(obj_type == ObjectType::Database)
			{
				configureDatabase(attribs);
				dbmodel->setPgOid(oid);
			}
			else if(create_methods.count(obj_type))
			{
				BaseObject *obj = create_methods[obj_type](attribs);

				if(obj)
					obj->setPgOid(oid);

				/* Register that the object was successfully created in order to avoid
				 * creating it again on the recursive object creation. (see getDependencyObject()) */
				created_objs.push_back(oid);
			}
			else if (debug_mode)
				qDebug().noquote() << QString("** create() method for %s isn't implemented!").arg(BaseObject::getSchemaName(obj_type));
		}
	}
	catch(Exception &e)
	{
		/* We just ignore the object duplication error and just mark the
		 * related object's attribs so it'll not be processed again */
		if(e.getErrorCode() == ErrorCode::AsgDuplicatedObject)
			created_objs.push_back(oid);
		else
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::ObjectNotImported)
											.arg(obj_name).arg(BaseObject::getTypeName(obj_type)).arg(attribs[Attributes::Oid]),
											ErrorCode::ObjectNotImported,PGM_FUNC,PGM_FILE,PGM_LINE, &e, dumpObjectAttributes(attribs));
		}
	}
}

QString DatabaseImportHelper::getComment(attribs_map &attribs)
{
	try
	{
		QString xml_def;

		if(!attribs[Attributes::Comment].isEmpty())
			xml_def=schparser.getSourceCode(Attributes::Comment, attribs, SchemaParser::XmlCode);

		return xml_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString DatabaseImportHelper::getDependencyObject(const QString &oid, ObjectType obj_type, bool use_signature,
																									bool recursive_dep_res, bool generate_xml, attribs_map extra_attribs)
{
	try
	{
		unsigned obj_oid = oid.toUInt();

		if(obj_oid == 0)
			return "";

		QString xml_def;
		attribs_map obj_attr = getObjectAttributes(obj_oid);

		/* If the attributes for the dependency does not exists and the automatic dependency
		resolution is enable, the object's attributes will be retrieved from catalog */
		if(auto_resolve_deps && obj_attr.empty() &&
				((import_ext_objs && catalog.isExtensionObject(obj_oid)) ||
				 (import_sys_objs  && obj_oid <= catalog.getLastSysObjectOID()) ||
				 (obj_oid > catalog.getLastSysObjectOID() && !catalog.isExtensionObject(obj_oid))))
		{
			catalog.setQueryFilter(Catalog::ListAllObjects);
			std::vector<attribs_map> attribs_vect=catalog.getObjectsAttributes(obj_type,"","", { obj_oid });

			if(!attribs_vect.empty())
			{
				if(obj_oid <= catalog.getLastSysObjectOID())
					system_objs[obj_oid]=attribs_vect[0];
				else
					user_objs[obj_oid]=attribs_vect[0];

				obj_attr=attribs_vect[0];
			}
		}

		if(!obj_attr.empty())
		{
			for(auto &itr : extra_attribs)
				obj_attr[itr.first] = itr.second;

			/* If the attributes of the dependency exists but it was not created yet,
				 pgModeler will create it and it's dependencies recursively */
			if(recursive_dep_res &&
					obj_type != ObjectType::Database &&	!TableObject::isTableObject(obj_type) &&
					std::find(created_objs.begin(), created_objs.end(), oid.toUInt()) == created_objs.end())
			{
				createObject(obj_attr);
			}

			QString obj_name;

			if(use_signature)
				obj_name = obj_attr[Attributes::Signature] = getObjectName(oid, true);
			else
				obj_name = getObjectName(oid);

			if(generate_xml)
			{
				obj_attr[Attributes::ReducedForm] = Attributes::True;
				schparser.ignoreUnkownAttributes(true);
				xml_def = schparser.getSourceCode(BaseObject::getSchemaName(obj_type), obj_attr, SchemaParser::XmlCode);
				schparser.ignoreUnkownAttributes(false);
			}
			else
				xml_def = obj_name;
		}
		else
			/* If the object oid is valid but there is no attribute set to it creates a xml definition
				 containing an alert indicating that the object is unknown */
			xml_def = UnkownObjectOidXml.arg(oid);

		return xml_def;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::loadObjectXML(ObjectType obj_type, attribs_map &attribs)
{
	QString xml_buf;

	try
	{
		schparser.ignoreUnkownAttributes(true);
		xml_buf=schparser.getSourceCode(BaseObject::getSchemaName(obj_type), attribs, SchemaParser::XmlCode);

		schparser.ignoreUnkownAttributes(false);
		xmlparser->restartParser();

		if(debug_mode)
		{
			qDebug().noquote() << QString("<!-- XML code: %1 (OID: %2) -->\n")
														.arg(attribs[Attributes::Name], attribs[Attributes::Oid]) << xml_buf;
		}

		xmlparser->loadXMLBuffer(xml_buf);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, xml_buf);
	}
}

void DatabaseImportHelper::resetImportParameters()
{
	Connection::setPrintSQL(false);
	import_canceled=false;
	dbmodel=nullptr;
	column_oids.clear();
	object_oids.clear();
	types.clear();
	user_objs.clear();
	creation_order.clear();
	seq_tab_swap.clear();
	columns.clear();
	system_objs.clear();
	errors.clear();
	constraints.clear();
	obj_perms.clear();
	col_perms.clear();
	connection.close();
	catalog.closeConnection();
	inherited_cols.clear();
	imported_tables.clear();
	created_objs.clear();
}

QString DatabaseImportHelper::dumpObjectAttributes(attribs_map &attribs)
{
	QString dump_str;

	dump_str+=QString("-- Raw attributes: %1 (OID: %2) --\n")
			  .arg(attribs[Attributes::Name])
			.arg(attribs[Attributes::Oid]);

	for(auto &attr : attribs)
		dump_str+=QString("%1: %2\n").arg(attr.first).arg(attr.second);

	dump_str+="---\n";

	return dump_str;
}

void DatabaseImportHelper::removeInheritedCols(PhysicalTable *tab)
{
	if(!tab || inherited_cols.empty())
		return;

	for(auto &col : *tab->getObjectList(ObjectType::Column))
	{
		auto inh_col_itr = std::find(inherited_cols.begin(), inherited_cols.end(), col);

		if(inh_col_itr != inherited_cols.end())
			inherited_cols.erase(inh_col_itr);
	}
}

Tablespace *DatabaseImportHelper::createTablespace(attribs_map &attribs)
{
	Tablespace *tabspc=nullptr;

	try
	{
		loadObjectXML(ObjectType::Tablespace, attribs);
		tabspc=dbmodel->createTablespace();
		dbmodel->addObject(tabspc);

		return tabspc;
	}
	catch(Exception &e)
	{
		delete tabspc;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

Schema *DatabaseImportHelper::createSchema(attribs_map &attribs)
{
	Schema *schema = nullptr;
	std::uniform_int_distribution<unsigned> dist(0,255);

	/* Avoiding the creation of the schemas pg_catalog and public when these objects
	 * already exist in the destination model. This prevents the import to be aborted
	 * due to duplicity error related to these schemas. */
	if((attribs[Attributes::Name] == "public" || attribs[Attributes::Name] == "pg_catalog") &&
		 dbmodel->getSchema(attribs[Attributes::Name]))
		return nullptr;

	try
	{
		attribs[Attributes::RectVisible]="";
		attribs[Attributes::FillColor]=QColor(dist(rand_num_engine),
																					dist(rand_num_engine),
																					dist(rand_num_engine)).name();
		loadObjectXML(ObjectType::Schema, attribs);

		schema=dbmodel->createSchema();
		dbmodel->addObject(schema);

		return schema;
	}
	catch(Exception &e)
	{
		delete schema;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Role *DatabaseImportHelper::createRole(attribs_map &attribs)
{
	Role *role=nullptr;

	try
	{
		QStringList rl_oids, rl_names, role_types={ Attributes::AdminRoles,
																								Attributes::MemberRoles };

		for(auto &rl_type : role_types)
		{
			rl_oids = Catalog::parseArrayValues(attribs[rl_type]);

			for(auto &rl_oid : rl_oids)
				rl_names.append(getDependencyObject(rl_oid, ObjectType::Role, false, auto_resolve_deps, false));

			attribs[rl_type] = rl_names.join(',');
			rl_names.clear();
		}

		loadObjectXML(ObjectType::Role, attribs);
		role=dbmodel->createRole();
		dbmodel->addObject(role);

		return role;
	}
	catch(Exception &e)
	{
		delete role;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Domain *DatabaseImportHelper::createDomain(attribs_map &attribs)
{
	Domain *dom=nullptr;
	QStringList constraints, constr_attrs;
	attribs_map aux_attribs;
	QString expr;

	try
	{
		constraints = attribs[Attributes::Constraints].split(UtilsNs::DataSeparator, Qt::SkipEmptyParts);
		attribs[Attributes::Constraints].clear();

		for(auto &constr : constraints)
		{
			constr_attrs = constr.split(" CHECK ");
			aux_attribs[Attributes::Name] = constr_attrs.at(0).trimmed();
			expr = constr_attrs.at(1).trimmed();
			expr.remove(expr.indexOf('('), 1);
			expr.remove(expr.lastIndexOf(')'), 1);
			aux_attribs[Attributes::Expression] = expr;
			attribs[Attributes::Constraints]+= schparser.getSourceCode(Attributes::DomConstraint, aux_attribs, SchemaParser::XmlCode);
		}

		attribs[Attributes::Type]=getType(attribs[Attributes::Type], true, attribs);
		attribs[Attributes::Collation]=getDependencyObject(attribs[Attributes::Collation], ObjectType::Collation);
		loadObjectXML(ObjectType::Domain, attribs);
		dom=dbmodel->createDomain();
		dbmodel->addDomain(dom);

		return dom;
	}
	catch(Exception &e)
	{
		delete dom;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Extension *DatabaseImportHelper::createExtension(attribs_map &attribs)
{
	Extension *ext = nullptr;

	try
	{
		QStringList ext_types = Catalog::parseArrayValues(attribs[Attributes::Types]),
								ext_schemas = Catalog::parseArrayValues(attribs[Attributes::Schemas]);
		attribs_map aux_attrs;

		attribs[Attributes::Objects] = "";

		for(auto &sch_name : ext_schemas)
		{
			aux_attrs[Attributes::Name] = sch_name;
			aux_attrs[Attributes::Type] = BaseObject::getSchemaName(ObjectType::Schema);

			schparser.ignoreEmptyAttributes(true);
			schparser.ignoreUnkownAttributes(true);
			attribs[Attributes::Objects] += schparser.getSourceCode(Attributes::Object, aux_attrs, SchemaParser::XmlCode);
		}

		QString typ_name, sch_name;
		QStringList name_list;

		for(auto &tp_name : ext_types)
		{
			name_list = tp_name.split('.');

			if(name_list.isEmpty())
				continue;

			if(name_list.size() == 1)
			{
				typ_name = name_list.at(0);
				sch_name = "";
			}
			else
			{
				typ_name = name_list.at(1);
				sch_name = name_list.at(0);
			}

			aux_attrs[Attributes::Name] = typ_name;
			aux_attrs[Attributes::Parent] = sch_name;
			aux_attrs[Attributes::Type] = BaseObject::getSchemaName(ObjectType::Type);

			schparser.ignoreEmptyAttributes(true);
			schparser.ignoreUnkownAttributes(true);
			attribs[Attributes::Objects] += schparser.getSourceCode(Attributes::Object, aux_attrs, SchemaParser::XmlCode);
		}

		loadObjectXML(ObjectType::Extension, attribs);
		ext = dbmodel->createExtension();
		dbmodel->addExtension(ext);

		return ext;
	}
	catch(Exception &e)
	{
		delete ext;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

void DatabaseImportHelper::configureBaseFunctionAttribs(attribs_map &attribs)
{
	Parameter param;
	PgSqlType type;
	QStringList param_types, param_names, param_modes,
			param_def_vals, param_xmls, used_names, transform_types,
			config_params, list;
	QString param_tmpl_name = QString("_param%1"), pname;
	std::vector<Parameter> parameters;
	attribs_map cfg_attrs;

	try
	{
		param_types = getTypes(attribs[Attributes::ArgTypes], false);
		param_names = Catalog::parseArrayValues(attribs[Attributes::ArgNames]);
		param_modes = Catalog::parseArrayValues(attribs[Attributes::ArgModes]);
		param_def_vals = Catalog::parseDefaultValues(attribs[Attributes::ArgDefaults]);

		transform_types = getTypes(attribs[Attributes::TransformTypes], false);
		attribs[Attributes::TransformTypes] = transform_types.join(',');

		config_params = attribs[Attributes::ConfigParams].split(UtilsNs::DataSeparator, Qt::SkipEmptyParts);
		attribs[Attributes::ConfigParams] = "";

		for(auto &cfg : config_params)
		{
			list = cfg.split('=');

			if(list.size() < 2)
				continue;

			cfg_attrs[Attributes::Name] = list[0];
			cfg_attrs[Attributes::Value] = list[1];
			attribs[Attributes::ConfigParams] +=	schparser.getSourceCode(Attributes::ConfigParam, cfg_attrs, SchemaParser::XmlCode);
		}

		for(int i=0; i < param_types.size(); i++)
		{
			/* If the function is to be used as a user-defined data type support functions
				 the parameter type will be renamed to "any" (see rules on Type::setFunction()) */
			if(i==0 &&
					(attribs[Attributes::RefType] == Attributes::SendFunc ||
					 attribs[Attributes::RefType] == Attributes::OutputFunc ||
					 attribs[Attributes::RefType] == Attributes::CanonicalFunc))
				type = PgSqlType("\"any\"");
			else
				type = PgSqlType::parseString(param_types[i]);

			//Alocates a new parameter
			param = Parameter();
			param.setType(type);

			if(!param_names.isEmpty())
			{
				pname = param_names[i].remove('"');

				if(pname.isEmpty())
					param.setName(param_tmpl_name.arg(i+1));
				/* In some cases fucntions came with duplicated names
				 * so we solve this duplication in order the function to
				 * be created correctly */
				else if(used_names.indexOf(pname) >= 0)
					param.setName(QString("%1%2").arg(pname).arg(i + 1));
				else
					param.setName(pname);
			}
			else
				param.setName(param_tmpl_name.arg(i + 1));

			used_names.append(param.getName());

			//Parameter modes: i = IN, o = OUT, b = INOUT, v = VARIADIC
			if(!param_modes.isEmpty())
			{
				param.setIn(param_modes[i] == "i" || param_modes[i] == "b");
				param.setOut(param_modes[i] == "o" || param_modes[i] == "b");
				param.setVariadic(param_modes[i] == "v");
			}

			//If the mode is 't' indicates that the current parameter will be used as a return table colum
			if(!param_modes.isEmpty() && param_modes[i]=="t")
				attribs[Attributes::ReturnTable]+=param.getSourceCode(SchemaParser::XmlCode);
			else
				parameters.push_back(param);
		}

		if(!parameters.empty())
		{
			std::vector<Parameter>::reverse_iterator ritr, ritr_end;

			ritr = parameters.rbegin();
			ritr_end = parameters.rend();

			while(ritr != ritr_end)
			{
				param = *ritr;
				ritr++;

				//Setting the default value for the current paramenter. OUT parameter doesn't receive default values.
				if(!param_def_vals.isEmpty() && (!param.isOut() || (param.isIn() && param.isOut())))
				{
					param.setDefaultValue(param_def_vals.back());
					param_def_vals.pop_back();
				}

				param_xmls.push_front(param.getSourceCode(SchemaParser::XmlCode));
			}

			attribs[Attributes::Parameters] += param_xmls.join(QChar('\n'));
		}

		//Case the function's language is C the symbol is the 'definition' attribute
		if(getObjectName(attribs[Attributes::Language]).toLower() == DefaultLanguages::C)
		{
			attribs[Attributes::Symbol] = attribs[Attributes::Definition];
			attribs[Attributes::Definition]="";
		}
		else
		{
			/* Removing extra \n from the beggining and from the end of the fucntion's definition.
			 * Some functions come from the catalogs with a extra \n appended/prepended in the field prosrc in which we get the functions source.
			 * This way if repeatedely export and import a model the amount of \n at the beginning and end of the source
			 * will increase leading to an unecessary addition of new lines to the functions source. */
			if(attribs[Attributes::Definition].startsWith(QChar::LineFeed))
				attribs[Attributes::Definition].remove(0, 1);

			if(attribs[Attributes::Definition].endsWith(QChar::LineFeed))
				attribs[Attributes::Definition].remove(attribs[Attributes::Definition].length() - 1, 1);
		}

		//Get the language reference code
		attribs[Attributes::Language] = getDependencyObject(attribs[Attributes::Language], ObjectType::Language);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

Function *DatabaseImportHelper::createFunction(attribs_map &attribs)
{
	Function *func=nullptr;

	try
	{
		configureBaseFunctionAttribs(attribs);

		//Get the return type if there is no return table configured
		if(attribs[Attributes::ReturnTable].isEmpty())
		{
			/* If the function is to be used as a user-defined data type support functions
				 the return type will be renamed to "any" (see rules on Type::setFunction()) */
			if(attribs[Attributes::RefType] == Attributes::InputFunc ||
					attribs[Attributes::RefType] == Attributes::RecvFunc ||
					attribs[Attributes::RefType] == Attributes::CanonicalFunc)
				attribs[Attributes::ReturnType] = PgSqlType("\"any\"").getSourceCode(SchemaParser::XmlCode);
			else
				attribs[Attributes::ReturnType] = getType(attribs[Attributes::ReturnType], true);
		}

		loadObjectXML(ObjectType::Function, attribs);
		func = dbmodel->createFunction();
		dbmodel->addFunction(func);

		return func;
	}
	catch(Exception &e)
	{
		delete func;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Procedure *DatabaseImportHelper::createProcedure(attribs_map &attribs)
{
	Procedure *proc=nullptr;

	try
	{
		configureBaseFunctionAttribs(attribs);
		loadObjectXML(ObjectType::Procedure, attribs);
		proc = dbmodel->createProcedure();
		dbmodel->addProcedure(proc);

		return proc;
	}
	catch(Exception &e)
	{
		delete proc;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Language *DatabaseImportHelper::createLanguage(attribs_map &attribs)
{
	Language *lang=nullptr;

	try
	{
		unsigned lang_oid, func_oid;
		QString func_types[] { Attributes::ValidatorFunc,
													 Attributes::HandlerFunc,
													 Attributes::InlineFunc };

		lang_oid = attribs[Attributes::Oid].toUInt();

		for(auto &func_type : func_types)
		{
			func_oid = attribs[func_type].toUInt();

			/* Workaround: in case of importing system languages like "internal" where the validator/handler
				 function is defined after the language pgModeler will raise errors so in order to continue
				 the import these fuctions are simply ignored */
			if(func_oid < lang_oid)
				attribs[func_type]=getDependencyObject(attribs[func_type], ObjectType::Function, true , true, true, {{Attributes::RefType, func_type}});
			else
				attribs[func_type]="";
		}

		loadObjectXML(ObjectType::Language, attribs);
		lang=dbmodel->createLanguage();
		dbmodel->addLanguage(lang);

		return lang;
	}
	catch(Exception &e)
	{
		delete lang;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

OperatorFamily *DatabaseImportHelper::createOperatorFamily(attribs_map &attribs)
{
	OperatorFamily *opfam=nullptr;

	try
	{
		loadObjectXML(ObjectType::OpFamily, attribs);
		opfam=dbmodel->createOperatorFamily();
		dbmodel->addOperatorFamily(opfam);

		return opfam;
	}
	catch(Exception &e)
	{
		delete opfam;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

OperatorClass *DatabaseImportHelper::createOperatorClass(attribs_map &attribs)
{
	OperatorClass *opclass=nullptr;

	try
	{
		attribs_map elem_attr;
		std::vector<attribs_map> elems;
		QStringList array_vals, list;

		attribs[Attributes::Family]=getObjectName(attribs[Attributes::Family], true);
		attribs[Attributes::Type]=getType(attribs[Attributes::Type], true /*, attribs */);

		//Generating attributes for STORAGE elements
		if(attribs[Attributes::Storage]!="0")
		{
			elem_attr[Attributes::Storage]=Attributes::True;
			elem_attr[Attributes::Definition]=getType(attribs[Attributes::Storage], true);
			elems.push_back(elem_attr);
		}
		else if(attribs[Attributes::Function].isEmpty() &&
				attribs[Attributes::Operator].isEmpty())
		{
			elem_attr[Attributes::Storage]=Attributes::True;
			elem_attr[Attributes::Definition]=attribs[Attributes::Type];
			elems.push_back(elem_attr);
		}

		//Generating attributes for FUNCTION elements
		if(!attribs[Attributes::Function].isEmpty())
		{
			elem_attr.clear();
			elem_attr[Attributes::Function]=Attributes::True;
			array_vals=Catalog::parseArrayValues(attribs[Attributes::Function]);

			for(const auto & array_val : array_vals)
			{
				list = array_val.split(':');
				elem_attr[Attributes::StrategyNum] = list[0];
				elem_attr[Attributes::Definition] = getDependencyObject(list[1], ObjectType::Function, true);
				elems.push_back(elem_attr);
			}
		}

		//Generating attributes for OPERATOR elements
		if(!attribs[Attributes::Operator].isEmpty())
		{
			elem_attr.clear();
			elem_attr[Attributes::Operator] = Attributes::True;
			array_vals = Catalog::parseArrayValues(attribs[Attributes::Operator]);

			for(auto &array_val : array_vals)
			{
				list = array_val.split(':');
				elem_attr[Attributes::Definition] = "";
				elem_attr[Attributes::StrategyNum] = list[0];
				elem_attr[Attributes::Definition] += getDependencyObject(list[1], ObjectType::Operator, true);
				elem_attr[Attributes::Definition] += getDependencyObject(list[2], ObjectType::OpFamily, true);
				elems.push_back(elem_attr);
			}
		}

		//Generating the complete XML code for operator class elements
		for(auto &elem : elems)
		{
			schparser.ignoreUnkownAttributes(true);
			attribs[Attributes::Elements] += schparser.getSourceCode(Attributes::Element, elem, SchemaParser::XmlCode);
			schparser.ignoreUnkownAttributes(false);
		}

		loadObjectXML(ObjectType::OpClass, attribs);
		opclass=dbmodel->createOperatorClass();
		dbmodel->addOperatorClass(opclass);

		return opclass;
	}
	catch(Exception &e)
	{
		delete opclass;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Operator *DatabaseImportHelper::createOperator(attribs_map &attribs)
{
	Operator *oper=nullptr;

	try
	{
		int pos;
		QRegularExpression regexp;
		QRegularExpressionMatch match;
		QString op_signature,

				func_types[]={ Attributes::OperatorFunc,
											 Attributes::RestrictionFunc,
											 Attributes::JoinFunc },

				arg_types[]= { Attributes::LeftType,
											 Attributes::RightType },

				op_types[]= { Attributes::CommutatorOp,
											Attributes::NegatorOp };

		for(auto &func_type : func_types)
			attribs[func_type]= getDependencyObject(attribs[func_type], ObjectType::Function, true, true, true, {{Attributes::RefType, func_type}});

		for(auto &arg_type : arg_types)
			attribs[arg_type] = getType(attribs[arg_type], true, {{Attributes::RefType, arg_type}});

		regexp.setPattern(Attributes::Signature + "(=)(\")");
		for(auto & op_type : op_types)
		{
			attribs[op_type] = getDependencyObject(attribs[op_type], ObjectType::Operator, true, false, true, {{Attributes::RefType, op_type}});

			if(!attribs[op_type].isEmpty())
			{
				/* Extracting the operator's signature to check if it was previouly created:
					Defining a operator as ++(A,B) and it's commutator as *++(B,A) PostgreSQL will automatically
					create on the second operator a commutator reference to ++(A,B). But to pgModeler only the first
					reference is valid, so the extracted signature is used to check if the commutator was previously
					created in order to avoid reference errors */
				match = regexp.match(attribs[op_type]);
				pos = match.capturedStart() + match.capturedLength();
				op_signature = attribs[op_type].mid(pos, (attribs[op_type].indexOf('"',pos) - pos));

				//If the operator is not defined clear up the reference to it
				if(dbmodel->getObjectIndex(op_signature, ObjectType::Operator) < 0)
					attribs[op_type].clear();
			}
		}

		loadObjectXML(ObjectType::Operator, attribs);
		oper=dbmodel->createOperator();
		dbmodel->addOperator(oper);

		return oper;
	}
	catch(Exception &e)
	{
		delete oper;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Collation *DatabaseImportHelper::createCollation(attribs_map &attribs)
{
	Collation *coll=nullptr;

	try
	{
		loadObjectXML(ObjectType::Collation, attribs);
		coll=dbmodel->createCollation();
		dbmodel->addCollation(coll);

		return coll;
	}
	catch(Exception &e)
	{
		delete coll;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Cast *DatabaseImportHelper::createCast(attribs_map &attribs)
{
	Cast *cast=nullptr;

	try
	{
		attribs[Attributes::Function]=getDependencyObject(attribs[Attributes::Function], ObjectType::Function, true);
		attribs[Attributes::SourceType]=getType(attribs[Attributes::SourceType], true);
		attribs[Attributes::DestType]=getType(attribs[Attributes::DestType], true);
		loadObjectXML(ObjectType::Cast, attribs);
		cast=dbmodel->createCast();
		dbmodel->addCast(cast);

		return cast;
	}
	catch(Exception &e)
	{
		delete cast;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Conversion *DatabaseImportHelper::createConversion(attribs_map &attribs)
{
	Conversion *conv=nullptr;

	try
	{
		attribs[Attributes::Function]=getDependencyObject(attribs[Attributes::Function], ObjectType::Function, true, auto_resolve_deps);
		loadObjectXML(ObjectType::Conversion, attribs);
		conv=dbmodel->createConversion();
		dbmodel->addConversion(conv);

		return conv;
	}
	catch(Exception &e)
	{
		delete conv;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Sequence *DatabaseImportHelper::createSequence(attribs_map &attribs)
{
	Sequence *seq=nullptr;
	Column *col = nullptr;

	try
	{
		QStringList owner_col=attribs[Attributes::OwnerColumn].split(':'),
				seq_attribs=Catalog::parseArrayValues(attribs[Attributes::Attribute]);
		QString attr[]={ Attributes::Start, Attributes::MinValue,
						 Attributes::MaxValue, Attributes::Increment,
						 Attributes::Cache, Attributes::Cycle };

		attribs[Attributes::OwnerColumn]="";

		/* If there are owner columns and the oid of sequence is greater than the owner column's table oid
		stores the oid of both (sequence and table) in order to swap it's ids at the end of import to
		avoid reference breaking when generation SQL code */
		if(owner_col.size()==2)
		{
			PhysicalTable *tab = nullptr;
			QString col_name, tab_name;
			attribs_map extra_attrs,
					pos_attrib={
				{ Attributes::XPos, "0" },
				{ Attributes::YPos, "0" }};

			if(attribs[Attributes::Oid].toUInt() > owner_col[0].toUInt())
				seq_tab_swap[attribs[Attributes::Oid]]=owner_col[0];

			/* Get the table and the owner column instances so the sequence code can be disabled if the
				column is an identity one */
			extra_attrs[Attributes::Position] = schparser.getSourceCode(Attributes::Position, pos_attrib, SchemaParser::XmlCode);
			tab_name = getDependencyObject(owner_col[0], ObjectType::Table, true, auto_resolve_deps, false, extra_attrs);
			tab = dbmodel->getTable(tab_name);

			if(!tab)
			{
				tab_name = getDependencyObject(owner_col[0], ObjectType::ForeignTable, true, auto_resolve_deps, false, extra_attrs);
				tab = dbmodel->getTable(tab_name);
			}

			col_name=getColumnName(owner_col[0], owner_col[1]);

			if(tab)
				col = tab->getColumn(col_name);
		}

		for(int i=0; i < seq_attribs.size(); i++)
			attribs[attr[i]]=seq_attribs[i];

		loadObjectXML(ObjectType::Sequence, attribs);
		seq=dbmodel->createSequence();
		dbmodel->addSequence(seq);

		//Disable the sequence's SQL when the owner column is identity
		if(col && col->isIdentity())
		{
			col->setIdSeqAttributes(seq->getMinValue(), seq->getMaxValue(), seq->getIncrement(),
															seq->getStart(), seq->getCache(), seq->isCycle());
			seq->setSQLDisabled(true);
		}

		return seq;
	}
	catch(Exception &e)
	{
		delete seq;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Aggregate *DatabaseImportHelper::createAggregate(attribs_map &attribs)
{
	Aggregate *agg=nullptr;

	try
	{
		QStringList types;
		QString func_types[]={ Attributes::TransitionFunc,
													 Attributes::FinalFunc },
				sch_name;

		for(auto & func_type : func_types)
			attribs[func_type] = getDependencyObject(attribs[func_type], ObjectType::Function, true, auto_resolve_deps, true, {{Attributes::RefType, func_type}});

		types=getTypes(attribs[Attributes::Types], true);
		attribs[Attributes::Types]="";

		for(const auto & type : types)
			attribs[Attributes::Types] += type;

		attribs[Attributes::StateType]=getType(attribs[Attributes::StateType], true,
		{{Attributes::RefType, Attributes::StateType}});
		attribs[Attributes::SortOp]=getDependencyObject(attribs[Attributes::SortOp], ObjectType::Operator, true);

		loadObjectXML(ObjectType::Aggregate, attribs);
		agg=dbmodel->createAggregate();
		dbmodel->addAggregate(agg);

		/* Removing the schema name from the aggregate name.
				The catalog query for certain aggregates (under pg_catalog for instance)
				will return names in the form "pg_catalog.agg_name" which cause objects
				to be imported with wrong names so the fix below is needed */
		sch_name = agg->getSchema()->getName() + QChar('.');

		if(agg->getName().startsWith(sch_name))
			agg->setName(agg->getName().remove(sch_name));

		return agg;
	}
	catch(Exception &e)
	{
		delete agg;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Type *DatabaseImportHelper::createType(attribs_map &attribs)
{
	/* Types that are children of any extension are discarded
	 * and not created in the database model */
	if(attribs[Attributes::IsExtType] == Attributes::True)
		return nullptr;

	Type *type = nullptr;
	attribs_map aux_attribs;

	try
	{
		attribs[attribs[Attributes::Configuration]]=Attributes::True;

		if(!attribs[Attributes::EnumType].isEmpty())
		{
			for(auto &label : attribs[Attributes::Labels].split(UtilsNs::DataSeparator, Qt::SkipEmptyParts))
			{
				aux_attribs[Attributes::Label] = label;
				attribs[Attributes::Labels] += schparser.getSourceCode(Attributes::EnumType, aux_attribs, SchemaParser::XmlCode);
			}
		}
		else if(!attribs[Attributes::CompositeType].isEmpty())
		{
			QStringList comp_attribs, values;
			TypeAttribute type_attrib;

			comp_attribs = Catalog::parseArrayValues(attribs[Attributes::TypeAttribute]);
			attribs[Attributes::TypeAttribute]="";

			for(auto &cmp_attr : comp_attribs)
			{
				values = cmp_attr.split(':');

				if(values.size() >= 2)
				{
					type_attrib.setName(values[0].remove('"'));
					type_attrib.setType(PgSqlType::parseString(getType(values[1], false)));
					type_attrib.setCollation(dbmodel->getCollation(getDependencyObject(values[2], ObjectType::Collation, true, false)));
					attribs[Attributes::TypeAttribute]+=type_attrib.getSourceCode(SchemaParser::XmlCode);
				}
			}
		}
		else if(!attribs[Attributes::RangeType].isEmpty())
		{
			QStringList range_attr=Catalog::parseArrayValues(attribs[Attributes::RangeAttribs]);

			attribs[Attributes::Subtype]=getType(range_attr[0], true);
			attribs[Attributes::Collation]=getDependencyObject(range_attr[1], ObjectType::Collation, true);
			attribs[Attributes::OpClass]=getDependencyObject(range_attr[2], ObjectType::OpClass, true);
			attribs[Attributes::CanonicalFunc]=getDependencyObject(range_attr[3], ObjectType::Function, true);
			attribs[Attributes::SubtypeDiffFunc]=getDependencyObject(range_attr[4], ObjectType::Function, true);
		}
		else
		{
			QString type_name=getObjectName(attribs[Attributes::Oid]),
					func_types[]={ Attributes::InputFunc,
								   Attributes::OutputFunc,
								   Attributes::RecvFunc,
								   Attributes::SendFunc,
								   Attributes::TpmodInFunc,
								   Attributes::TpmodOutFunc,
								   Attributes::AnalyzeFunc };
			unsigned i, count=sizeof(func_types)/sizeof(QString);

			attribs[Attributes::Element]=getType(attribs[Attributes::Element], false);

			/* Workaround: if importing a datatype that is part of an extension we avoid the importing of
			 * its supporting functions (since they will not be necessary here because the type will be sql-disabled)*/
			if(!catalog.isExtensionObject(attribs[Attributes::Oid].toUInt()))
			{
				for(i=0; i < count; i++)
				{
					attribs[func_types[i]]=getDependencyObject(attribs[func_types[i]], ObjectType::Function, true, true, true, {{Attributes::RefType, func_types[i]}});

					/* Since pgModeler requires that type functions refers to the constructing type as "any"
						 it's necessary to replace the function parameter types names */
					attribs[func_types[i]].replace("IN " + type_name, "IN any");
				}
			}
		}

		loadObjectXML(ObjectType::Type, attribs);
		type=dbmodel->createType();
		dbmodel->addType(type);

		return type;
	}
	catch(Exception &e)
	{
		delete type;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Table *DatabaseImportHelper::createTable(attribs_map &attribs)
{
	Table *table=nullptr;

	try
	{
		std::vector<unsigned> inh_cols;
		attribs_map pos_attrib={
			{ Attributes::XPos, "0" },
			{ Attributes::YPos, "0" }};

		attribs[Attributes::Position] = schparser.getSourceCode(Attributes::Position, pos_attrib, SchemaParser::XmlCode);
		attribs[Attributes::Columns] = createColumns(attribs, inh_cols).join(QChar::LineFeed);
		loadObjectXML(ObjectType::Table, attribs);
		table=dbmodel->createTable();

		for(unsigned col_idx : inh_cols)
			inherited_cols.push_back(table->getColumn(col_idx));

		// Storing the partition bound expression temporarily in the table in order to configure the partition hierarchy later
		table->setPartitionBoundingExpr(attribs[Attributes::PartitionBoundExpr].remove(QRegularExpression("^(FOR)( )+(VALUES)( )*", QRegularExpression::CaseInsensitiveOption)));

		// Retrieving the partitioned table related to the partition table being created
		if(!attribs[Attributes::PartitionedTable].isEmpty())
		{
			Table *partitioned_tab = nullptr;

			attribs[Attributes::PartitionedTable] =
					getDependencyObject(attribs[Attributes::PartitionedTable], ObjectType::Table, true, auto_resolve_deps, false);

			partitioned_tab = dbmodel->getTable(attribs[Attributes::PartitionedTable]);
			table->setPartionedTable(partitioned_tab);

			if(!partitioned_tab)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(attribs[Attributes::Name]).arg(BaseObject::getTypeName(ObjectType::Table))
												.arg(attribs[Attributes::PartitionedTable]).arg(BaseObject::getTypeName(ObjectType::Table)),
												ErrorCode::RefObjectInexistsModel ,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		// Creating partition keys if present
		if(attribs[Attributes::IsPartitioned] == Attributes::True)
		{
			QStringList cols, collations, opclasses, exprs;
			PartitionKey part_key;
			PartitioningType part_type;
			QString coll_name, opc_name;
			Collation *coll = nullptr;
			OperatorClass *opclass = nullptr;
			std::vector<PartitionKey> part_keys;

			part_type = PartitioningType(attribs[Attributes::Partitioning]);
			table->setPartitioningType(part_type);

			cols=Catalog::parseArrayValues(attribs[Attributes::PartKeyCols]);
			collations=Catalog::parseArrayValues(attribs[Attributes::PartKeyColls]);
			opclasses=Catalog::parseArrayValues(attribs[Attributes::PartKeyOpCls]);
			exprs = Catalog::parseIndexExpressions(attribs[Attributes::PartKeyExprs]);

			for(int i = 0; i < cols.size(); i++)
			{
				part_key = PartitionKey();

				// Retrieving the column used by the partition key
				if(cols[i] != "0")
					part_key.setColumn(table->getColumn(getColumnName(attribs[Attributes::Oid], cols[i])));
				else if(!exprs.isEmpty())
				{
					part_key.setExpression(exprs.front());
					exprs.pop_front();
				}

				// Retriving the collation for the partion key
				if(i < collations.size() && collations[i] != "0")
				{
					coll_name = getDependencyObject(collations[i], ObjectType::Collation, false, true, false);
					coll = dynamic_cast<Collation *>(dbmodel->getObject(coll_name, ObjectType::Collation));

					//Even if the collation exists we'll ignore it when it is the "pg_catalog.default"
					if(coll && (!coll->isSystemObject() ||
											(coll->isSystemObject() && coll->getName() != "default")))
						part_key.setCollation(coll);
				}

				// Retriving the operator class for the partion key
				if(i < opclasses.size() && opclasses[i] != "0")
				{
					opc_name = getDependencyObject(opclasses[i], ObjectType::OpClass, true, true, false);
					opclass = dynamic_cast<OperatorClass *>(dbmodel->getObject(opc_name, ObjectType::OpClass));

					if(opclass)
						part_key.setOperatorClass(opclass);
				}

				part_keys.push_back(part_key);
			}

			table->addPartitionKeys(part_keys);
		}

		dbmodel->addTable(table);
		imported_tables[attribs[Attributes::Oid].toUInt()] = table;

		return table;
	}
	catch(Exception &e)
	{
		if(table)
		{
			removeInheritedCols(table);
			delete table;
		}

		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

View *DatabaseImportHelper::createView(attribs_map &attribs)
{
	View *view = nullptr;
	unsigned type_oid = 0;
	QString type_name, sch_name, col_name;
	bool is_type_registered = false;
	std::vector<Reference> references;
	std::vector<SimpleColumn> custom_cols;
	BaseTable *ref_tab = nullptr;
	QStringList options, key_val;

	try
	{
		attribs_map pos_attrib={{ Attributes::XPos, "0" },
														{ Attributes::YPos, "0" }};

		attribs[Attributes::Position]=schparser.getSourceCode(Attributes::Position, pos_attrib, SchemaParser::XmlCode);
		sch_name = getDependencyObject(attribs[Attributes::SchemaOid], ObjectType::Schema, true, auto_resolve_deps, false);
		retrieveTableColumns(sch_name, attribs[Attributes::Name]);

		options = Catalog::parseArrayValues(attribs[Attributes::Options]);

		for(auto &opt : options)
		{
			key_val = opt.split('=');

			if(key_val.isEmpty())
				continue;

			/* We replace the _ of the option name by - to match
			 * the related attribute in Attributes namespace */
			attribs[key_val[0].replace('_','-')] =
					/* If the key_val has two elements we use the index #1 as the value,
					 * in the abscence of the index #1 we consider the option to having a
					 * boolean (true) value */
					key_val.size() == 2 ? key_val[1] : Attributes::True;
		}

		//Creating columns
		for(auto &itr : columns[attribs[Attributes::Oid].toUInt()])
		{
			col_name = itr.second[Attributes::Name];
			type_oid = itr.second[Attributes::TypeOid].toUInt();

			/* If the type has an entry on the types map and its OID is greater than system object oids,
			 * means that it's a user defined type, thus, there is the need to check if the type
			 * is registered. */
			if(types.count(type_oid)!=0 && type_oid > catalog.getLastSysObjectOID())
			{
				/* Building the type name prepending the schema name in order to search it on
				 * the user defined types list at PgSQLType class */
				type_name=BaseObject::formatName(getObjectName(types[type_oid][Attributes::Schema], true), false);
				type_name+=".";

				if(types[type_oid][Attributes::Category] == ~CategoryType(CategoryType::Array))
				{
					int dim = types[type_oid][Attributes::Name].count("[]");
					QString aux_name = types[type_oid][Attributes::Name].remove("[]");
					type_name+=BaseObject::formatName(aux_name, false);
					type_name+=QString("[]").repeated(dim);
				}
				else
					type_name+=BaseObject::formatName(types[type_oid][Attributes::Name], false);

				is_type_registered=PgSqlType::isRegistered(type_name, dbmodel);
			}
			else
			{
				type_name = itr.second[Attributes::Type];
				is_type_registered=(types.count(type_oid)!=0 && PgSqlType::isRegistered(type_name, dbmodel));
			}

			/* Checking if the type used by the column exists (is registered),
			 * if not it'll be created when auto_resolve_deps is checked. The only exception here if for
			 * array types [] that will not be automatically created because they are derivated from
			 * the non-array type, this way, if the original type is created there is no need to create the array form */
			if(auto_resolve_deps && !is_type_registered && !type_name.contains("[]"))
				// Try to create the missing data type
				getType(itr.second[Attributes::TypeOid], false);

			custom_cols.push_back(SimpleColumn(col_name, *PgSqlType::parseString(type_name), ""));
		}

		// Configuring the reference tables
		for(auto &tab_oid : Catalog::parseArrayValues(attribs[Attributes::RefTables]))
		{
			ref_tab = dbmodel->getTable(getDependencyObject(tab_oid, ObjectType::Table, true, true, false));

			// If we couldn't get a table from tab_oid we try to get a foreign table or a view
			if(!ref_tab)
			{
				ref_tab = dbmodel->getForeignTable(getDependencyObject(tab_oid, ObjectType::ForeignTable, true, true, false));

				if(!ref_tab)
					ref_tab = dbmodel->getView(getDependencyObject(tab_oid, ObjectType::View, true, true, false));
			}

			if(ref_tab)
				references.push_back(Reference(ref_tab, ref_tab->getName(), "", true, false, false));
		}

		for(auto &ref : references)
			attribs[Attributes::References] += ref.getXmlCode();

		for(auto &col : custom_cols)
			attribs[Attributes::Columns] += col.getXmlCode();

		loadObjectXML(ObjectType::View, attribs);
		view = dbmodel->createView();
		dbmodel->addView(view);

		return view;
	}
	catch(Exception &e)
	{
		delete view;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Rule *DatabaseImportHelper::createRule(attribs_map &attribs)
{
	QString cmds=attribs[Attributes::Commands];
	int start=-1;
	QRegularExpression cond_regexp("(WHERE)(.)+(DO)");
	QRegularExpressionMatch match;
	ObjectType table_type=ObjectType::Table;

	try
	{
		match = cond_regexp.match(cmds);
		start = match.capturedStart();

		if(start >=0)
		{
			attribs[Attributes::Condition]=cmds.mid(start, match.capturedLength());
			attribs[Attributes::Condition].remove(QRegularExpression("(DO)|(WHERE)"));
		}

		attribs[Attributes::Commands]=Catalog::parseRuleCommands(attribs[Attributes::Commands]).join(';');

		if(attribs[Attributes::TableType]==BaseObject::getSchemaName(ObjectType::View))
			table_type=ObjectType::View;

		attribs[Attributes::Table]=getDependencyObject(attribs[Attributes::Table], table_type, true, auto_resolve_deps, false);

		loadObjectXML(ObjectType::Rule, attribs);
		Rule *rule = dbmodel->createRule();
		rule->setSQLDisabled(rule->getParentTable()->isSQLDisabled());

		return rule;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Trigger *DatabaseImportHelper::createTrigger(attribs_map &attribs)
{
	try
	{
		ObjectType table_type;
		QStringList args;

		table_type=BaseObject::getObjectType(attribs[Attributes::TableType]);
		attribs[Attributes::Table]=getDependencyObject(attribs[Attributes::Table], table_type, true, auto_resolve_deps, false);
		attribs[Attributes::TriggerFunc]=getDependencyObject(attribs[Attributes::TriggerFunc], ObjectType::Function, true, true);

		args = attribs[Attributes::Arguments].split(Catalog::EscapedNullChar, Qt::SkipEmptyParts);
		attribs[Attributes::Arguments] = args.join(UtilsNs::DataSeparator);

		loadObjectXML(ObjectType::Trigger, attribs);

		Trigger *trig = dbmodel->createTrigger();
		trig->setSQLDisabled(trig->getParentTable()->isSQLDisabled());

		return trig;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Index *DatabaseImportHelper::createIndex(attribs_map &attribs)
{
	try
	{
		QStringList cols, opclasses, collations, exprs, incl_cols, options;
		IndexElement elem;
		BaseTable *parent_tab=nullptr;
		Collation *coll=nullptr;
		OperatorClass *opclass=nullptr;
		QString tab_name, coll_name, opc_name;
		int i = 0, elem_cnt = 0;
		bool desc_order = false, nulls_first = false;

		attribs[Attributes::Factor]="90";
		tab_name=getDependencyObject(attribs[Attributes::Table], ObjectType::Table, true, auto_resolve_deps, false);
		parent_tab=dynamic_cast<BaseTable *>(dbmodel->getObject(tab_name, ObjectType::Table));

		if(!parent_tab)
		{
			tab_name=getDependencyObject(attribs[Attributes::Table], ObjectType::View, true, auto_resolve_deps, false);
			parent_tab=dynamic_cast<BaseTable *>(dbmodel->getObject(tab_name, ObjectType::View));

			if(!parent_tab)
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(attribs[Attributes::Name]).arg(BaseObject::getTypeName(ObjectType::Index))
												.arg(tab_name).arg(BaseObject::getTypeName(ObjectType::Table)),
												ErrorCode::RefObjectInexistsModel ,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		cols = Catalog::parseArrayValues(attribs[Attributes::Columns]);
		collations = Catalog::parseArrayValues(attribs[Attributes::Collations]);
		opclasses = Catalog::parseArrayValues(attribs[Attributes::OpClasses]);
		exprs = Catalog::parseIndexExpressions(attribs[Attributes::Expressions]);
		elem_cnt = attribs[Attributes::ElementsCount].toInt();
		options = Catalog::parseArrayValues(attribs[Attributes::Options]);

		for(i=0; i < elem_cnt; i++)
		{
			elem=IndexElement();

			if(cols[i] != "0")
			{
				if(parent_tab->getObjectType() == ObjectType::Table)
					elem.setColumn(dynamic_cast<Table *>(parent_tab)->getColumn(getColumnName(attribs[Attributes::Table], cols[i])));
				else if(parent_tab->getObjectType() == ObjectType::View)
					elem.setSimpleColumn(dynamic_cast<View *>(parent_tab)->getColumn(getColumnName(attribs[Attributes::Table], cols[i])));
			}
			else if(!exprs.isEmpty())
			{
				elem.setExpression(exprs.front());
				exprs.pop_front();
			}

			if(i < options.size())
			{
				/* The definition of the index option bits is in src/include/catalog/pg_index.h
				 * So, to determine if DESC ordering is set as well as NULLS FIRST we have
				 * to perform a bitwise operation. */

				/*
				 * Index AMs that support ordered scans must support these two indoption
				 * bits.  Otherwise, the content of the per-column indoption fields is
				 * open for future definition.
				 */
				// #define INDOPTION_DESC          0x0001  /* values are in reverse order */
				// #define INDOPTION_NULLS_FIRST   0x0002  /* NULLs are first instead of last */

				desc_order = (options[i].toUInt() & 1) == 1;
				nulls_first = (options[i].toUInt() & 2) == 2;
				elem.setSortingEnabled(desc_order || nulls_first);
				elem.setSortingAttribute(IndexElement::AscOrder, !desc_order);
				elem.setSortingAttribute(IndexElement::NullsFirst, nulls_first);
			}

			if(i < collations.size() && collations[i]!="0")
			{
				coll_name=getDependencyObject(collations[i], ObjectType::Collation, false, true, false);
				coll=dynamic_cast<Collation *>(dbmodel->getObject(coll_name, ObjectType::Collation));

				//Even if the collation exists we'll ignore it when it is the "pg_catalog.default"
				if(coll && (!coll->isSystemObject() ||
										(coll->isSystemObject() && coll->getName() != "default")))
					elem.setCollation(coll);
			}

			if(i < opclasses.size() && opclasses[i]!="0")
			{
				opc_name=getDependencyObject(opclasses[i], ObjectType::OpClass, true, true, false);
				opclass=dynamic_cast<OperatorClass *>(dbmodel->getObject(opc_name, ObjectType::OpClass));

				if(opclass)
					elem.setOperatorClass(opclass);
			}

			if(elem.getColumn() || elem.getSimpleColumn().isValid() || !elem.getExpression().isEmpty())
				attribs[Attributes::Elements]+=elem.getSourceCode(SchemaParser::XmlCode);
		}

		for(i = elem_cnt; i < cols.size(); i++)
			incl_cols.append(getColumnName(attribs[Attributes::Table], cols[i]));

		attribs[Attributes::IncludedCols] = incl_cols.join(',');
		attribs[Attributes::Table]=tab_name;

		loadObjectXML(ObjectType::Index, attribs);
		Index *index = dbmodel->createIndex();
		index->setSQLDisabled(index->getParentTable()->isSQLDisabled());

		return index;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Constraint *DatabaseImportHelper::createConstraint(attribs_map &attribs)
{
	Constraint *constr=nullptr;

	try
	{
		QString table_oid=attribs[Attributes::Table],
				ref_tab_oid=attribs[Attributes::RefTable],
				tab_name;
		PhysicalTable *table=nullptr;

		//If the table oid is 0 indicates that the constraint is part of a data type like domains
		if(!table_oid.isEmpty() && table_oid!="0")
		{
			ObjectType tab_type = BaseObject::getObjectType(attribs[Attributes::TableType]);
			QStringList factor=Catalog::parseArrayValues(attribs[Attributes::Factor]);

			//Retrieving the table is it was not imported yet and auto_resolve_deps is true
			tab_name=getDependencyObject(table_oid, tab_type, true, auto_resolve_deps, false);

			if(!factor.isEmpty() && factor[0].startsWith("fillfactor="))
				attribs[Attributes::Factor]=factor[0].remove("fillfactor=");

			attribs[attribs[Attributes::Type]]=Attributes::True;
			table=dynamic_cast<PhysicalTable *>(dbmodel->getObject(tab_name, tab_type));

			if(attribs[Attributes::Type]==Attributes::ExConstr)
			{
				QStringList cols, opclasses, opers, exprs, options;
				ExcludeElement elem;
				QString opc_name, op_name;
				OperatorClass *opclass=nullptr;
				Operator *oper=nullptr;
				bool desc_order = false, nulls_first = false;

				attribs[Attributes::SrcColumns]="";
				attribs[Attributes::Expression]=attribs[Attributes::Condition];

				cols=Catalog::parseArrayValues(attribs[Attributes::Columns]);
				opers=Catalog::parseArrayValues(attribs[Attributes::Operators]);
				opclasses=Catalog::parseArrayValues(attribs[Attributes::OpClasses]);
				options = Catalog::parseArrayValues(attribs[Attributes::Options]);

				/* Due to the way exclude constraints are constructed (similar to indexes),
				 * we get the constraint's definition in for of expressions. Internally we use pg_get_constraintdef.
				 * This way we will get EXCLUDE USING [index type](elements). The elements in this case is a set of expression
				 * which we work to separate column only references from complex expression. Only complex expression will be used
				 * and assigned to their exclude constraint elements. Column references are used in exclude elements but relying in
				 * the cols list above */
				exprs = attribs[Attributes::Expressions]
							.replace(QString("EXCLUDE USING %1 (").arg(attribs[Attributes::IndexType]), "")
							.split(QRegularExpression("(WITH )(\\+|\\-|\\*|\\/|\\<|\\>|\\=|\\~|\\!|\\@|\\#|\\%|\\^|\\&|\\||\\'|\\?)+((,)?|(\\))?)"),
										 Qt::SkipEmptyParts);

				for(int i=0; i < cols.size(); i++)
				{
					elem=ExcludeElement();

					if(cols[i] != "0")
						elem.setColumn(table->getColumn(getColumnName(table_oid, cols[i])));
					else
						elem.setExpression(exprs.front().trimmed());

					exprs.pop_front();

					if(i < opclasses.size() && opclasses[i]!="0")
					{
						opc_name=getDependencyObject(opclasses[i], ObjectType::OpClass, true, true, false);
						opclass=dynamic_cast<OperatorClass *>(dbmodel->getObject(opc_name, ObjectType::OpClass));

						if(opclass)
							elem.setOperatorClass(opclass);
					}

					if(i < opers.size() && opers[i]!="0")
					{
						op_name=getDependencyObject(opers[i], ObjectType::Operator, true, true, false);
						oper=dynamic_cast<Operator *>(dbmodel->getObject(op_name, ObjectType::Operator));

						if(oper)
							elem.setOperator(oper);
					}

					if(i < options.size())
					{
						desc_order = (options[i].toUInt() & 1) == 1;
						nulls_first = (options[i].toUInt() & 2) == 2;
						elem.setSortingEnabled(desc_order || nulls_first);
						elem.setSortingAttribute(ExcludeElement::AscOrder, !desc_order);
						elem.setSortingAttribute(ExcludeElement::NullsFirst, nulls_first);
					}

					attribs[Attributes::Elements]+=elem.getSourceCode(SchemaParser::XmlCode);
				}
			}
			else
			{
				//Clears the tablespace attribute when the constraint is fk avoiding errors
				if(attribs[Attributes::Type]==Attributes::FkConstr)
					attribs[Attributes::Tablespace]="";
				else if(attribs[Attributes::Type]==Attributes::CkConstr)
				{
					QString expr = attribs[Attributes::Expressions];
					expr.replace("CHECK (", "");
					expr.remove(expr.lastIndexOf(')'), 1);
					attribs[Attributes::Expression] = expr;
				}

				attribs[Attributes::SrcColumns] = getColumnNames(attribs[Attributes::Table], attribs[Attributes::SrcColumns]).join(',');
			}

			attribs[Attributes::RefTable] = getDependencyObject(ref_tab_oid, ObjectType::Table, false, true, false);
			attribs[Attributes::DstColumns] = getColumnNames(ref_tab_oid, attribs[Attributes::DstColumns]).join(',');
			attribs[Attributes::Table] = tab_name;

			loadObjectXML(ObjectType::Constraint, attribs);
			constr = dbmodel->createConstraint(nullptr);
			constr->setSQLDisabled(table->isSQLDisabled());

			if(table &&  constr->getConstraintType()==ConstraintType::PrimaryKey)
				table->addConstraint(constr);

			table->setModified(true);
		}

		return constr;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Policy *DatabaseImportHelper::createPolicy(attribs_map &attribs)
{
	try
	{
		attribs[Attributes::Table]=getDependencyObject(attribs[Attributes::Table], ObjectType::Table, true, auto_resolve_deps, false);
		attribs[Attributes::Roles]=getObjectNames(attribs[Attributes::Roles]).join(',');
		loadObjectXML(ObjectType::Policy, attribs);

		Policy *pol = dbmodel->createPolicy();
		pol->setSQLDisabled(pol->getParentTable()->isSQLDisabled());

		return pol;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

EventTrigger *DatabaseImportHelper::createEventTrigger(attribs_map &attribs)
{
	try
	{
		attribs[Attributes::Function]=getDependencyObject(attribs[Attributes::Function], ObjectType::Function, true, true);
		attribs[Attributes::Filter]=QString("\t<%1 %2=\"%3\" %4=\"%5\"/>\n")
										   .arg(Attributes::Filter)
										   .arg(Attributes::Variable).arg(Attributes::Tag.toUpper())
										   .arg(Attributes::Values)
										   .arg(Catalog::parseArrayValues(attribs[Attributes::Values].remove('"')).join(','));

		loadObjectXML(ObjectType::EventTrigger, attribs);
		EventTrigger *ev_trig = dbmodel->createEventTrigger();
		dbmodel->addEventTrigger(ev_trig);

		return ev_trig;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

ForeignDataWrapper *DatabaseImportHelper::createForeignDataWrapper(attribs_map &attribs)
{
	ForeignDataWrapper *fdw=nullptr;

	try
	{
		QStringList func_types={ Attributes::ValidatorFunc, Attributes::HandlerFunc };

		for(auto &func_tp : func_types)
			attribs[func_tp] = getDependencyObject(attribs[func_tp], ObjectType::Function, true , true, true, {{Attributes::RefType, func_tp}});

		attribs[Attributes::Options] = Catalog::parseArrayValues(attribs[Attributes::Options]).join(ForeignDataWrapper::OptionsSeparator);

		loadObjectXML(ObjectType::ForeignDataWrapper, attribs);
		fdw = dbmodel->createForeignDataWrapper();
		dbmodel->addForeignDataWrapper(fdw);

		return fdw;
	}
	catch(Exception &e)
	{
		delete fdw;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

ForeignServer *DatabaseImportHelper::createForeignServer(attribs_map &attribs)
{
	ForeignServer *server=nullptr;

	try
	{
		attribs[Attributes::Fdw] = getDependencyObject(attribs[Attributes::Fdw], ObjectType::ForeignDataWrapper, true , true, true);
		attribs[Attributes::Options] = Catalog::parseArrayValues(attribs[Attributes::Options]).join(ForeignDataWrapper::OptionsSeparator);

		loadObjectXML(ObjectType::ForeignServer, attribs);
		server = dbmodel->createForeignServer();
		dbmodel->addForeignServer(server);

		return server;
	}
	catch(Exception &e)
	{
		delete server;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

UserMapping *DatabaseImportHelper::createUserMapping(attribs_map &attribs)
{
	UserMapping *usr_map=nullptr;

	try
	{
		attribs[Attributes::Server] = getDependencyObject(attribs[Attributes::Server], ObjectType::ForeignServer, true , true, true);
		attribs[Attributes::Options] = Catalog::parseArrayValues(attribs[Attributes::Options]).join(ForeignDataWrapper::OptionsSeparator);

		loadObjectXML(ObjectType::UserMapping, attribs);
		usr_map = dbmodel->createUserMapping();
		dbmodel->addUserMapping(usr_map);

		return usr_map;
	}
	catch(Exception &e)
	{
		delete usr_map;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

ForeignTable *DatabaseImportHelper::createForeignTable(attribs_map &attribs)
{
	ForeignTable *ftable=nullptr;

	try
	{
		std::vector<unsigned> inh_cols;
		attribs_map pos_attrib={
			{ Attributes::XPos, "0" },
			{ Attributes::YPos, "0" }};

		attribs[Attributes::Server] = getDependencyObject(attribs[Attributes::Server], ObjectType::ForeignServer, true , true, true);
		attribs[Attributes::Options] = Catalog::parseArrayValues(attribs[Attributes::Options]).join(ForeignDataWrapper::OptionsSeparator);
		attribs[Attributes::Position]=schparser.getSourceCode(Attributes::Position, pos_attrib, SchemaParser::XmlCode);

		attribs[Attributes::Columns] = createColumns(attribs, inh_cols).join(QChar::LineFeed);
		loadObjectXML(ObjectType::ForeignTable, attribs);
		ftable=dbmodel->createForeignTable();

		for(unsigned col_idx : inh_cols)
			inherited_cols.push_back(ftable->getColumn(col_idx));

		// Storing the partition bound expression temporarily in the table in order to configure the partition hierarchy later
		ftable->setPartitionBoundingExpr(attribs[Attributes::PartitionBoundExpr].remove(QRegularExpression("^(FOR)( )+(VALUES)( )*", QRegularExpression::CaseInsensitiveOption)));

		// Retrieving the partitioned table related to the partition table being created
		if(!attribs[Attributes::PartitionedTable].isEmpty())
		{
			Table *partitioned_tab = nullptr;

			attribs[Attributes::PartitionedTable] =
					getDependencyObject(attribs[Attributes::PartitionedTable], ObjectType::Table, true, auto_resolve_deps, false);

			partitioned_tab = dbmodel->getTable(attribs[Attributes::PartitionedTable]);
			ftable->setPartionedTable(partitioned_tab);

			if(!partitioned_tab)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(attribs[Attributes::Name]).arg(BaseObject::getTypeName(ObjectType::Table))
												.arg(attribs[Attributes::PartitionedTable]).arg(BaseObject::getTypeName(ObjectType::Table)),
												ErrorCode::RefObjectInexistsModel ,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		dbmodel->addForeignTable(ftable);
		imported_tables[attribs[Attributes::Oid].toUInt()] = ftable;

		return ftable;
	}
	catch(Exception &e)
	{
		if(ftable)
		{
			removeInheritedCols(ftable);
			delete ftable;
		}
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

Transform *DatabaseImportHelper::createTransform(attribs_map &attribs)
{
	Transform *transf = nullptr;

	try
	{
		attribs[Attributes::Type] = getType(attribs[Attributes::Type], true);
		attribs[Attributes::Language] = getDependencyObject(attribs[Attributes::Language], ObjectType::Language, false);
		attribs[Attributes::FromSqlFunc] = getDependencyObject(attribs[Attributes::FromSqlFunc], ObjectType::Function, true, true, true, {{ Attributes::RefType, Attributes::FromSqlFunc }});
		attribs[Attributes::ToSqlFunc] = getDependencyObject(attribs[Attributes::ToSqlFunc], ObjectType::Function, true, true, true, {{ Attributes::RefType, Attributes::ToSqlFunc }});
		loadObjectXML(ObjectType::Transform, attribs);
		transf = dbmodel->createTransform();
		dbmodel->addTransform(transf);

		return transf;
	}
	catch(Exception &e)
	{
		delete transf;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
										PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

void DatabaseImportHelper::createPermission(attribs_map &attribs)
{
	ObjectType obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());

	if(!Permission::acceptsPermission(obj_type))
		return;

	Permission *permission=nullptr;
	QString sig;
	QStringList perm_list;
	std::vector<Permission::PrivilegeId> privs, gop_privs;
	QString role_name;
	Role *role=nullptr;
	BaseObject *object=nullptr;
	PhysicalTable *table=nullptr;

	//Parses the permissions vector string
	perm_list=Catalog::parseArrayValues(attribs[Attributes::Permission]);

	if(!perm_list.isEmpty())
	{
		if(obj_type!=ObjectType::Column)
		{
			if(obj_type==ObjectType::Database)
				object=dbmodel;
			else
			{
				sig = getObjectName(attribs[Attributes::Oid], true);
				object = dbmodel->getObject(getObjectName(attribs[Attributes::Oid], true), obj_type);
			}
		}
		else
		{
			//If the object is column it's necessary to retrive the parent table to get the valid reference to column
			table=dynamic_cast<PhysicalTable *>(dbmodel->getObject(getObjectName(attribs[Attributes::Table]), {ObjectType::Table, ObjectType::ForeignTable}));
			object=table->getObject(getColumnName(attribs[Attributes::Table], attribs[Attributes::Oid]), ObjectType::Column);
		}
	}

	//for(int i=0; i < perm_list.size(); i++)
	for(auto perm_str : perm_list)
	{
		//Parses the permission retrieving the role name as well the privileges over the object
		role_name=Permission::parsePermissionString(perm_str, privs, gop_privs);

		//Removing extra backslash from the role's names to avoid the role not to be found
		role_name.remove(QChar('\\'));

		if(!privs.empty() || gop_privs.empty())
		{
			role=dynamic_cast<Role *>(dbmodel->getObject(role_name, ObjectType::Role));

			if(auto_resolve_deps && !role_name.isEmpty() && !role)
			{
				try
				{
					QString oid = catalog.getObjectOID(role_name, ObjectType::Role);
					getDependencyObject(oid, ObjectType::Role);
					role = dynamic_cast<Role *>(dbmodel->getObject(role_name, ObjectType::Role));

					if(oid == "0" && !role_name.isEmpty() && !role && !import_sys_objs)
					{
						qInfo() << QString("** createPermission(): Failed to create the permission: %1").arg(attribs[Attributes::Permission]);
						qInfo() << QString("** Role `%1' was not imported and/or created in the database model.").arg(role_name);
						qInfo() << "** Consider enabling the importing of system objects to avoid this warning." << Qt::endl;
						return;
					}
				}
				catch(Exception &e)
				{
					throw Exception(Exception::getErrorMessage(ErrorCode::ObjectNotImported)
													.arg(perm_str)
													.arg(BaseObject::getTypeName(ObjectType::Permission))
													.arg("n/d"),
													ErrorCode::ObjectNotImported,PGM_FUNC,PGM_FILE,PGM_LINE, &e,
													dumpObjectAttributes(attribs));
				}
			}

			/* If the role doesn't exists and there is a name defined, throws an error because
			the roles wasn't found on the model */
			if(!role && !role_name.isEmpty())
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RefObjectInexistsModel)
												.arg(perm_str).arg(BaseObject::getTypeName(ObjectType::Permission))
												.arg(role_name).arg(BaseObject::getTypeName(ObjectType::Role)), PGM_FUNC, PGM_FILE, PGM_LINE);
			}

			try
			{
				//Configuring the permisison
				permission = new Permission(object);

				if(role)
					permission->addRole(role);

				//Setting the ordinary privileges
				while(!privs.empty())
				{
					permission->setPrivilege(privs.back(), true, false);
					privs.pop_back();
				}

				//Setting the grant option privileges
				while(!gop_privs.empty())
				{
					permission->setPrivilege(gop_privs.back(), true, true);
					gop_privs.pop_back();
				}

				dbmodel->addPermission(permission);
			}
			catch(Exception &e)
			{
				delete permission;

				if(!ignore_errors)
					throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);

				errors.push_back(Exception(e.getErrorMessage(), e.getErrorCode(),
												 PGM_FUNC,PGM_FILE,PGM_LINE, &e, dumpObjectAttributes(attribs)));
			}
		}
	}
}

void DatabaseImportHelper::createTableInheritances()
{
	//Creating table inheiritances
	if(dbmodel->getObjectCount(ObjectType::Table) > 0 && !import_canceled)
	{
		try
		{
			emit s_progressUpdated(90,
								   tr("Creating table inheritances..."),
								   ObjectType::Relationship);
			__createTableInheritances();
		}
		catch(Exception &e)
		{
			if(ignore_errors)
				errors.push_back(e);
			else
				throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}
}

void DatabaseImportHelper::createTablePartitionings()
{
	if(imported_tables.empty())
		return;

	try
	{
		PhysicalTable *table = nullptr, *part_table = nullptr;
		QString part_bound_expr;
		Relationship *rel_part = nullptr;

		emit s_progressUpdated(95,
								 tr("Creating table partitionings..."),
								 ObjectType::Relationship);

		// Creating the paritioning relationships
		for(auto &itr : imported_tables)
		{
			table = itr.second;

			if(table->isPartition())
			{
				part_bound_expr = table->getPartitionBoundingExpr();
				part_table = table->getPartitionedTable();

				/* Here, we force the detaching of the partition table so when
				 * creating the relationship below all the needed validations can be done correctly */
				table->setPartionedTable(nullptr);
				table->setPartitionBoundingExpr("");

				rel_part = new Relationship(BaseRelationship::RelationshipPart, table, part_table);
				rel_part->setPartitionBoundingExpr(part_bound_expr);
				dbmodel->addRelationship(rel_part);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportHelper::destroyDetachedColumns()
{
	if(inherited_cols.empty() || import_canceled)
		return;

	std::vector<BaseObject *> refs;
	PhysicalTable *parent_tab=nullptr;

	// Saving the special objects' xmls is needed here in case of importing objects to an already populated model
	dbmodel->storeSpecialObjectsXML();
	dbmodel->disconnectRelationships();

	emit s_progressUpdated(100,
												 tr("Destroying unused detached columns..."),
												 ObjectType::Column);

	//Destroying detached columns before create inheritances
	for(auto &col : inherited_cols)
	{
		if(col->isReferenced())
			continue;

		try
		{
			parent_tab = dynamic_cast<PhysicalTable *>(col->getParentTable());

			/* Removing the column from the parent table only when the column isn't referenced by a partition key.
				 After removing from table, we destroy it since they will be recreated by inheritances/partitioning */
			if(!parent_tab->isPartitionKeyRefColumn(col))
			{
				parent_tab->removeObject(col);
				delete col;
			}
		}
		catch(Exception &e)
		{
			if(ignore_errors)
				errors.push_back(e);
			else
				throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
		}
	}

	/* Force the validation and connection of inheritance relationships
	 leading to the creation of inherited columns */
	dbmodel->validateRelationships();
}

void DatabaseImportHelper::createColumns()
{
	try
	{
		std::vector<unsigned> inh_cols;
		QString tab_signature;
		attribs_map tab_attr, col_attr;
		PhysicalTable *tab = nullptr;
		Column *col = nullptr;
		QStringList col_xmls;
		ObjectType obj_type;

		for(auto &[tab_oid, _] : columns)
		{
			tab_attr = user_objs[tab_oid];
			obj_type = static_cast<ObjectType>(tab_attr[Attributes::ObjectType].toUInt());

			// View columns are ignored
			if(obj_type == ObjectType::View)
				continue;

			/* Configuring the XML of the columns to be created in the table,
			 * after calling this method, the code is stored in the attribute
			 * Attributes::Columns of the tab_attr */
			col_xmls = createColumns(tab_attr, inh_cols);

			/* Retrieving the instance of table in the database model
			 * that will receive the columns */
			tab_signature = getObjectName(QString::number(tab_oid), true);
			tab = dynamic_cast<PhysicalTable *>(dbmodel->getObject(tab_signature, obj_type));

			if(!tab)
			{
				throw Exception(tr("Trying to create column(s) in the table `%1' that does not exist in the model!").arg(tab_signature),
												ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE);
			}

			for(auto &col_xml : col_xmls)
			{
				if(debug_mode)
				{
					qDebug().noquote() << QString("<!-- XML code of column(s) in: %1 (OID: %2) -->\n")
																.arg(tab_signature).arg(tab_oid) << col_xml;
				}

				try
				{
					col = nullptr;
					xmlparser->restartParser();
					xmlparser->loadXMLBuffer(col_xml);
					xmlparser->getElementAttributes(col_attr);

					// If the column already exists we just ignore it
					if(tab->getColumn(col_attr[Attributes::Name]))
						continue;

					// Creating the columns on the table
					col = dbmodel->createColumn();
					tab->addColumn(col);
				}
				catch(Exception &e)
				{
					delete col;
					throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
				}
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QStringList DatabaseImportHelper::createColumns(attribs_map &attribs, std::vector<unsigned> &inh_cols)
{
	unsigned tab_oid=attribs[Attributes::Oid].toUInt(), type_oid=0, col_idx=0;
	bool is_type_registered=false;
	Column col;
	QString type_name, def_val;
	std::map<unsigned, attribs_map>::iterator itr, itr1, itr_end;
	QStringList col_xmls;

	if(tab_oid == 0)
		return {};

	//Retrieving columns if they were not retrieved yet
	if(!columns.count(attribs[Attributes::Oid].toUInt()) && auto_resolve_deps)
	{
		QString sch_name = getDependencyObject(attribs[Attributes::SchemaOid], ObjectType::Schema, true, auto_resolve_deps, false);
		retrieveTableColumns(sch_name, attribs[Attributes::Name]);
	}

	itr = itr1 = columns[attribs[Attributes::Oid].toUInt()].begin();
	itr_end = columns[attribs[Attributes::Oid].toUInt()].end();
	attribs[Attributes::MaxObjCount] = QString::number(columns[attribs[Attributes::Oid].toUInt()].size());

	//Creating columns
	while(itr!=itr_end)
	{
		if(itr->second.count(Attributes::Permission) &&
				!itr->second.at(Attributes::Permission).isEmpty())
			col_perms[tab_oid].push_back(itr->second[Attributes::Oid].toUInt());

		if(itr->second[Attributes::Inherited]==Attributes::True)
			inh_cols.push_back(col_idx);

		col.setName(itr->second[Attributes::Name]);
		type_oid=itr->second[Attributes::TypeOid].toUInt();

		/* If the type has an entry on the types map and its OID is greater than system object oids,
		 * means that it's a user defined type, thus, there is the need to check if the type
		 * is registered. */
		if(types.count(type_oid) !=0 && type_oid > catalog.getLastSysObjectOID())
		{
			/* Building the type name prepending the schema name in order to search it on
			 * the user defined types list at PgSQLType class */
			QString sch_name = BaseObject::formatName(getObjectName(types[type_oid][Attributes::Schema], true), false);
			sch_name += ".";
			type_name.clear();

			/* Special verification for PostGiS types: if the current type is a gis based one
			 * (geometry, geography, box3d or box2d) we override the usage of the current type
			 * and force the use of the pgModeler built-in one. */
			if((PgSqlType::isPostGisGeoType(types[type_oid][Attributes::Name]) ||
					PgSqlType::isPostGisBoxType(types[type_oid][Attributes::Name])) &&
				 types[type_oid][Attributes::Configuration] == Attributes::BaseType &&
				 types[type_oid][Attributes::Category] == ~CategoryType(CategoryType::UserDefined))
			{
				type_name = itr->second[Attributes::Type];
				type_name.remove(sch_name);
				is_type_registered = true;
			}
			else
			{
				if(types[type_oid][Attributes::Category] == ~CategoryType(CategoryType::Array))
				{
					int dim = types[type_oid][Attributes::Name].count("[]");
					QString aux_name = types[type_oid][Attributes::Name].remove("[]");
					type_name+=BaseObject::formatName(aux_name, false);
					type_name+=QString("[]").repeated(dim);
					type_name.prepend(sch_name);
				}
				else
					type_name = getType(QString::number(type_oid), false);

				is_type_registered=PgSqlType::isRegistered(type_name, dbmodel);
			}
		}
		else
		{
			type_name = itr->second[Attributes::Type];
			is_type_registered=(types.count(type_oid)!=0 && PgSqlType::isRegistered(type_name, dbmodel));
		}

		/* Checking if the type used by the column exists (is registered),
		 * if not it'll be created when auto_resolve_deps is checked. */
		if(auto_resolve_deps && !is_type_registered &&
			 type_oid > catalog.getLastSysObjectOID())
		{
			// Try to create the missing data type
			getType(itr->second[Attributes::TypeOid], false);
		}

		col.setIdentityType(IdentityType::Null);
		col.setGenerated(false);
		col.setType(PgSqlType::parseString(type_name));
		col.setNotNull(!itr->second[Attributes::NotNull].isEmpty());
		col.setComment(itr->second[Attributes::Comment]);

		if(comments_as_aliases)
			col.setAlias(col.getComment().mid(0, BaseObject::ObjectNameMaxLength - 1));

		//Overriding the default value if the column is identity
		if(!itr->second[Attributes::IdentityType].isEmpty())
			col.setIdentityType(itr->second[Attributes::IdentityType]);
		else if(itr->second[Attributes::Generated] == Attributes::True)
		{
			col.setGenerated(true);
			def_val = itr->second[Attributes::DefaultValue];

			if(def_val.startsWith('(') && def_val.endsWith(')'))
			{
				def_val.remove(0, 1);
				def_val.remove(def_val.length() - 1, 1);
			}

			col.setDefaultValue(def_val);
		}
		else
		{
			/* Removing extra/forced type casting in the retrieved default value.
			 This is done in order to avoid unnecessary entries in the diff results.

			 For instance: say in the model we have a column with the following configutation:
			 > varchar(3) default 'foo'

			 Now, when importing the same column the default value for it will be something like:
			 > varchar(3) default 'foo'::character varying

			 Since the extra chars in the default value of the imported column are redundant (casting
			 varchar to character varying) we remove the '::character varying'. The idea here is to eliminate
			 the cast if the casting is equivalent to the column type. */
			def_val = itr->second[Attributes::DefaultValue];

			if(!def_val.startsWith("nextval(") && def_val.contains("::"))
			{
				QStringList values = def_val.split("::");

				if(values.size() > 1 &&
					 ((~col.getType() == values[1]) ||
						(~col.getType() == "char" && values[1] == "bpchar") ||
						(col.getType().isUserType() && (~col.getType()).endsWith(values[1]))))
					def_val=values[0];
			}

			col.setDefaultValue(def_val);
		}

		//Checking if the collation used by the column exists, if not it'll be created when auto_resolve_deps is checked
		if(auto_resolve_deps && !itr->second[Attributes::Collation].isEmpty())
			getDependencyObject(itr->second[Attributes::Collation], ObjectType::Collation);

		col.setCollation(dbmodel->getObject(getObjectName(itr->second[Attributes::Collation]),ObjectType::Collation));
		//attribs[Attributes::Columns]+=col.getSourceCode(SchemaParser::XmlCode);
		col_xmls.append(col.getSourceCode(SchemaParser::XmlCode));
		itr++;
		col_idx++;
	}

	return col_xmls;
}

void DatabaseImportHelper::assignSequencesToColumns()
{
	PhysicalTable *table=nullptr;
	Column *col=nullptr;
	std::vector<BaseObject *> tables;
	int progress = 0, i = 0;

	tables = *dbmodel->getObjectList(ObjectType::Table);
	tables.insert(tables.end(),
								dbmodel->getObjectList(ObjectType::ForeignTable)->begin(),
								dbmodel->getObjectList(ObjectType::ForeignTable)->end());

	for(auto &object : tables)
	{
		table=dynamic_cast<PhysicalTable *>(object);
		progress=(i/static_cast<double>(tables.size())) * 100;
		i++;

		for(auto &tab_obj : *table->getObjectList(ObjectType::Column))
		{
			col=dynamic_cast<Column *>(tab_obj);

			//Translating the default value nextval('sequence'::regclass)
			if((col->getType().isIntegerType() || col->getType().isNumericType()) &&
					col->getDefaultValue().contains("nextval("))
			{
				QString seq_name=col->getDefaultValue();
				Sequence *seq=nullptr;
				QStringList names;

				//Extracting the name from the nextval(''::regclass) portion and formating it
				seq_name.remove(0, seq_name.indexOf(QChar('\'')) + 1);
				seq_name.remove(seq_name.indexOf(QChar('\'')), seq_name.length());
				names = seq_name.split('.');

				/* Checking if the sequence name contains the schema prepended.
				 * If not, it'll be prepended the default "public" */
				if(names.size() <= 1)
				{
					seq_name.prepend("public.");
					names = seq_name.split('.');
				}

				seq = dbmodel->getSequence(seq_name);

				try
				{
					// If the sequence doesn't exists we try to create it if the auto deps resolution is enabled
					if(!seq && auto_resolve_deps)
					{
						QString seq_oid;

						catalog.clearObjectFilter(ObjectType::Sequence);
						seq_oid = catalog.getObjectOID(names[1], ObjectType::Sequence, names[0]);
						seq_name = getDependencyObject(seq_oid, ObjectType::Sequence, true, true, false);
						seq = dbmodel->getSequence(seq_name);
					}
				}
				catch(Exception &e)
				{
					// Failing to create the sequence will not abort the entire process, instead, it'll dump a debug message
					qInfo() << QString("** assignSequencesToColumns(): Failed to create the sequence: %1").arg(seq_name) << Qt::endl;
					qInfo() << e.getExceptionsText() << Qt::endl;
				}

				if(seq)
				{
					emit s_progressUpdated(progress,
											 tr("Assigning sequence `%1' to column `%2'...").arg(seq->getSignature()).arg(col->getSignature()),
											 ObjectType::Sequence);

					col->setSequence(seq);

					if(col->getParentTable()->getObjectId() < seq->getObjectId())
						BaseObject::swapObjectsIds(col->getParentTable(), seq, false);
				}
			}
		}
	}
}

void DatabaseImportHelper::__createTableInheritances()
{
	std::vector<unsigned> table_oids;
	Relationship *rel=nullptr;
	PhysicalTable *parent_tab=nullptr, *child_tab=nullptr;
	QStringList inh_list;
	ObjectType tab_type;
	attribs_map tab_attribs;
	unsigned inh_oid;

	table_oids = object_oids[ObjectType::Table];
	table_oids.insert(table_oids.end(),
										object_oids[ObjectType::ForeignTable].begin(),
										object_oids[ObjectType::ForeignTable].end());

	for(auto &oid : table_oids)
	{
		tab_attribs = (user_objs.count(oid) ? user_objs[oid] : system_objs[oid]);
		tab_type = static_cast<ObjectType>(tab_attribs[Attributes::ObjectType].toUInt());

		//Get the list of parent table's oids
		inh_list = Catalog::parseArrayValues(user_objs[oid][Attributes::Parents]);

		if(!inh_list.isEmpty())
		{
			//Get the child table resolving it's name from the oid
			QString tab_name = getObjectName(user_objs[oid][Attributes::Oid]);
			child_tab = dynamic_cast<PhysicalTable *>(dbmodel->getObject(tab_name, tab_type));

			for(auto &oid : inh_list)
			{
				inh_oid = oid.toUInt();
				tab_attribs=(user_objs.count(inh_oid) ? user_objs[inh_oid] : system_objs[inh_oid]);

				// The parent tis is not created and auto_resolve_deps is enabled we try to create it
				if(auto_resolve_deps && tab_attribs.empty())
				{
					getDependencyObject(inh_list.front(), ObjectType::Table, true);
					tab_attribs=(user_objs.count(inh_oid) ? user_objs[inh_oid] : system_objs[inh_oid]);
				}

				tab_type = static_cast<ObjectType>(tab_attribs[Attributes::ObjectType].toUInt());
				parent_tab=dynamic_cast<PhysicalTable *>(dbmodel->getObject(getObjectName(inh_list.front()), tab_type));

				try
				{
					if(!parent_tab)
						throw Exception(Exception::getErrorMessage(ErrorCode::InvInheritParentTableNotFound)
														.arg(child_tab->getSignature()).arg(inh_oid),
														ErrorCode::InvInheritParentTableNotFound,PGM_FUNC,PGM_FILE,PGM_LINE);

					//Create the inheritance relationship
					rel = new Relationship(Relationship::RelationshipGen, child_tab, parent_tab);
					rel->setName(CoreUtilsNs::generateUniqueName(rel, (*dbmodel->getObjectList(ObjectType::Relationship))));

					dbmodel->addRelationship(rel);
					rel = nullptr;
				}
				catch(Exception &e)
				{
					dbmodel->removeRelationship(rel);

					delete rel;

					if(ignore_errors)
						errors.push_back(e);
					else
						throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
				}
			}
		}
	}

}

void DatabaseImportHelper::configureDatabase(attribs_map &attribs)
{
	try
	{
		attribs[Attributes::AppendAtEod]="";
		attribs[Attributes::Layers]="";
		attribs[Attributes::ActiveLayers]="";
		loadObjectXML(ObjectType::Database, attribs);
		dbmodel->configureDatabase(attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),
						PGM_FUNC,PGM_FILE,PGM_LINE, &e, xmlparser->getXMLBuffer());
	}
}

QString DatabaseImportHelper::getObjectName(const QString &oid, bool signature_form)
{
	unsigned obj_oid=oid.toUInt();
	attribs_map obj_attr = getObjectAttributes(obj_oid);

	if(obj_oid == 0 || obj_attr.empty())
		return "";

	if(!signature_form && cached_names.count(obj_oid))
		return cached_names[obj_oid];

	if(signature_form && cached_signatures.count(obj_oid))
		return cached_signatures[obj_oid];

	QString sch_name,
			obj_name = obj_attr[Attributes::Name];
	ObjectType obj_type = static_cast<ObjectType>(obj_attr[Attributes::ObjectType].toUInt());

	if(TableObject::isTableObject(obj_type))
		obj_name.prepend(getObjectName(obj_attr[Attributes::Table], signature_form) + ".");
	else
	{
		//If the object accepts an schema retrieve the schema name too
		if(BaseObject::acceptsSchema(obj_type))
			sch_name = getObjectName(obj_attr[Attributes::Schema]);

		if(!sch_name.isEmpty())
			obj_name.prepend(sch_name + ".");
	}

	//Formatting the name in form of signature (only for functions and operators)
	if(signature_form &&
		 (obj_type==ObjectType::Function || obj_type==ObjectType::Procedure ||
			obj_type==ObjectType::Operator || obj_type==ObjectType::Aggregate ||
			obj_type==ObjectType::OpFamily || obj_type==ObjectType::OpClass))
	{
		QStringList params;

		if(obj_type==ObjectType::Function || obj_type==ObjectType::Procedure)
		{
			QStringList arg_types=getTypes(obj_attr[Attributes::ArgTypes], false),
					arg_modes=Catalog::parseArrayValues(obj_attr[Attributes::ArgModes]);

			for(int i=0; i < arg_types.size(); i++)
			{
				if(arg_modes.isEmpty())
					params.push_back(arg_types[i]);
				else if(arg_modes[i]!="t" && arg_modes[i]!="o")
				{
					if(arg_modes[i]=="i" || arg_modes[i]=="b")
						params.push_back(arg_types[i]);
					else
						params.push_back("VARIADIC " + arg_types[i]);
				}
			}
		}
		else if(obj_type==ObjectType::Aggregate)
		{
			QStringList params=getTypes(obj_attr[Attributes::Types], false);

			if(params.isEmpty())
				params.push_back("*");
		}
		else if(obj_type==ObjectType::Operator)
		{
			if(obj_attr[Attributes::LeftType].toUInt() > 0)
				params.push_back(getType(obj_attr[Attributes::LeftType], false));
			else
				params.push_back("NONE");

			if(obj_attr[Attributes::RightType].toUInt() > 0)
				params.push_back(getType(obj_attr[Attributes::RightType], false));
			else
				params.push_back("NONE");
		}
		else
		{
			obj_name += QString(" USING %1").arg(obj_attr[Attributes::IndexType]);
		}

		if(obj_type != ObjectType::OpFamily && obj_type != ObjectType::OpClass)
			obj_name+="(" + params.join(',') + ")";
	}

	/* If signature must be returned and the schema name couldn't be determined
	 * above, we don't cache the object signature, instead, the entire above
	 * routine is repeated until the schema name of the object is known */
	if(signature_form &&
			(!BaseObject::acceptsSchema(obj_type) ||
			 (BaseObject::acceptsSchema(obj_type) && !sch_name.isEmpty())))
		cached_signatures[obj_oid] = obj_name;
	else if(!signature_form)
		cached_names[obj_oid] = obj_name;

	return obj_name;
}

attribs_map DatabaseImportHelper::getObjectAttributes(unsigned oid)
{
	if(user_objs.count(oid))
		return user_objs[oid];

	if(system_objs.count(oid))
		return system_objs[oid];

	return {};
}

QStringList DatabaseImportHelper::getObjectNames(const QString &oid_vect, bool signature_form)
{
	QStringList oids = Catalog::parseArrayValues(oid_vect),
			obj_names;

	for(auto &oid : oids)
		obj_names.append(getObjectName(oid, signature_form));

	return obj_names;
}

QString DatabaseImportHelper::getColumnName(const QString &tab_oid_str, const QString &col_id_str, bool prepend_tab_name)
{
	QString col_name;
	unsigned tab_oid=tab_oid_str.toUInt(), col_id=col_id_str.toUInt();

	if(columns.count(tab_oid) && columns[tab_oid].count(col_id))
	{
		if(prepend_tab_name)
			col_name=getObjectName(tab_oid_str) + ".";

		col_name+=columns[tab_oid][col_id].at(Attributes::Name);
	}

	return col_name;
}

QStringList DatabaseImportHelper::getColumnNames(const QString &tab_oid_str, const QString &col_id_vect, bool prepend_tab_name)
{
	QStringList col_names;
	QString tab_name;
	unsigned tab_oid = tab_oid_str.toUInt(), col_id = 0;

	if(columns.count(tab_oid))
	{
		if(prepend_tab_name)
			tab_name = getObjectName(tab_oid_str) + ".";

		for(auto &c_id : Catalog::parseArrayValues(col_id_vect))
		{
			col_id = c_id.toUInt();

			if(columns[tab_oid].count(col_id))
				col_names.push_back(tab_name + columns[tab_oid][col_id].at(Attributes::Name));
		}
	}


	return col_names;
}

QString DatabaseImportHelper::getType(const QString &oid_str, bool generate_xml, attribs_map extra_attribs)
{
	try
	{
		unsigned type_oid=oid_str.toUInt();

		if(type_oid == 0)
			return "";

		attribs_map type_attr;
		QString xml_def, sch_name, obj_name, aux_name;
		static const QString brackets = "[]";
		unsigned elem_tp_oid = 0, dimension=0, object_id=0;
		bool is_derivated_from_obj = false, is_postgis_type = false;
		int num_brkt = 0;

		if(types.count(type_oid))
			type_attr = types[type_oid];
		else
		{
			/* If the type was not found is more likely that is a user-defined type which was not listed
				 * while retrieving system types (see retrieveSystemObjects()).User defined types are created on demand,
				 * this way pgModeler will import its attributes so it can be created correctly below. */
			Catalog::QueryFilter curr_filter = catalog.getQueryFilter();

			catalog.setQueryFilter(Catalog::ListAllObjects);
			type_attr = catalog.getObjectAttributes(ObjectType::Type, type_oid);
			catalog.setQueryFilter(curr_filter);

			/* Formatting/Quoting the name of the type (if necessary) in order to avoid
				 * breaking the importing if there are user defined types in CamelCase for example.
				 * This way the type will be always referenced like schema."Type"
				 * instead of schema.Type (which is the same as schema.type). */
			if(type_attr[Attributes::Category] == "A")
			{
				/* Special treatment for array data types: Before quoting the name
					 * we have to extract the [], then format the name and then restore the
					 * brackets. This avoids the brackets to be considered as part of the name and
					 * thus causing the name to be quoted inconditionaly. */
				num_brkt = type_attr[Attributes::Name].count(brackets);
				QString aux_name = BaseObject::formatName(type_attr[Attributes::Name].remove(brackets));

				aux_name += brackets.repeated(num_brkt);
				type_attr[Attributes::Name] = aux_name;
				types[type_oid] = type_attr;

				/* For array types we also retrieve the original data type (element type)
				 * and store its attributes in the types map, so the original type can be also
				 * created if needed */
				elem_tp_oid = type_attr[Attributes::Element].toUInt();
				types[elem_tp_oid] = catalog.getObjectAttributes(ObjectType::Type, elem_tp_oid);
			}
			else
				type_attr[Attributes::Name] = BaseObject::formatName(type_attr[Attributes::Name]);

			types[type_oid] = type_attr;
		}

		object_id = type_attr[Attributes::ObjectId].toUInt();

		//Special treatment for array types. Removes the [] descriptor when generating XML code for the type
		if(!type_attr.empty() && type_attr[Attributes::Category]=="A" &&
			 type_attr[Attributes::Name].contains(brackets))
		{
			obj_name = type_attr[Attributes::Name];

			if(generate_xml)
			{
				dimension = type_attr[Attributes::Name].count(brackets);
				obj_name.remove(brackets);
			}
		}
		else
			obj_name = type_attr[Attributes::Name];

		/* If the type was generated from a table/sequence/view/domain and the source object is not
				 yet imported and the auto resolve deps is enabled, we need to import it */
		if(auto_resolve_deps &&
			 (!type_attr[Attributes::TypeClass].isEmpty() ||
				type_attr[Attributes::Configuration] == BaseObject::getSchemaName(ObjectType::Domain)))
		{
			ObjectType obj_type;

			if(type_attr[Attributes::TypeClass]==BaseObject::getSchemaName(ObjectType::Table))
				obj_type=ObjectType::Table;
			else if(type_attr[Attributes::TypeClass]==BaseObject::getSchemaName(ObjectType::ForeignTable))
				obj_type=ObjectType::ForeignTable;
			else if(type_attr[Attributes::TypeClass]==BaseObject::getSchemaName(ObjectType::View))
				obj_type=ObjectType::View;
			else if(type_attr[Attributes::TypeClass]==BaseObject::getSchemaName(ObjectType::Sequence))
				obj_type=ObjectType::Sequence;
			else
			{
				/* Domains are the only kind of object associated to data types that don't
					 * contains an object-id attribute (which is related to the object in pg_class that generates the type)
					 * this way we need to use type_oid instead to force the importing of the domain */
				obj_type=ObjectType::Domain;
				object_id = type_oid;
			}

			is_derivated_from_obj = true;

			if(!user_objs.count(object_id) && !system_objs.count(object_id))
				getDependencyObject(QString::number(object_id), obj_type, true, true, false);
		}
		/* Special case for the built-in pseudo-type any. We double-quote it in the first usage/reference
			 * in order to avoid reference breaking and malformed SQL code */
		else if(type_attr[Attributes::Configuration] == "pseudo" && obj_name == "any")
		{
			obj_name.prepend('"');
			obj_name.append('"');
			types[type_oid][Attributes::Name] = obj_name;
		}

		/* Removing the optional modifier "without time zone" from date/time types.
				 Since the class PgSQLTypes ommits the modifier it is necessary to reproduce
				 this behavior here to avoid futher errors */
		if(obj_name.startsWith("timestamp") || obj_name.startsWith("time"))
			obj_name.remove(" without time zone");

		/* Prepend the schema name only if the type it is not in a system schema ('pg_catalog' or 'information_schema'),
			 * if the schema's names is already present in the type's name (in case of table types) or if the type being
			 * retrieved is not a PostGiS one (because, despite the type being from extension PostGiS, it is considered
			 * a built-in type in pgModeler so there's no need to use schema qualified name) */
		is_postgis_type = catalog.isExtensionObject(type_oid, "postgis");
		sch_name = getObjectName(type_attr[Attributes::Schema]);

		if(!sch_name.isEmpty() && !is_postgis_type &&
			 (is_derivated_from_obj ||
				(sch_name != "pg_catalog" && sch_name != "information_schema") ||
				type_oid > catalog.getLastSysObjectOID()) &&
			 !obj_name.contains(QRegularExpression(QString("^(\\\")?(%1)(\\\")?(\\.)").arg(sch_name))))
		{
			/* To format an array data type name we need first remove the brackets
			 * the format the name properly, after that, we include the brackets again
			 * so the correct name can be returned */
			obj_name.remove(brackets);
			obj_name = BaseObject::formatName(obj_name, false);
			obj_name.append(brackets.repeated(num_brkt));
			obj_name.prepend(BaseObject::formatName(sch_name, false) + ".");
		}

		/* In case of auto resolve dependencies, if the type is a user defined one and is
		 * not derivated from table/view/sequence, is not from the postgis extension  and
		 * was not created in the database model but its attributes were retrieved, the object
		 * will be created to avoid reference errors */
		aux_name = obj_name;
		aux_name.remove(brackets);

		if(auto_resolve_deps && !type_attr.empty() && !is_derivated_from_obj && !is_postgis_type &&
			 type_oid > catalog.getLastSysObjectOID() && !dbmodel->getType(aux_name))
		{
			//If the type is not an array one we simply use the current type attributes map
			if(type_attr[Attributes::Category] != "A")
				createObject(type_attr);
			/* In case the type is an array one we should use the oid held by "element" attribute to
				 create the type related to current one */
			else if(elem_tp_oid > catalog.getLastSysObjectOID() &&	types.count(elem_tp_oid))
				createObject(types[elem_tp_oid]);
		}

		if(generate_xml)
		{
			extra_attribs[Attributes::Name]=obj_name;
			extra_attribs[Attributes::Dimension]=(dimension > 0 ? QString::number(dimension) : "");

			schparser.ignoreUnkownAttributes(true);
			xml_def = schparser.getSourceCode(Attributes::PgSqlBaseType, extra_attribs, SchemaParser::XmlCode);
			schparser.ignoreUnkownAttributes(false);

			return xml_def;
		}

		return obj_name;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QStringList DatabaseImportHelper::getTypes(const QString &oid_vect, bool generate_xml)
{
	QStringList oids = Catalog::parseArrayValues(oid_vect),
			types;

	for(auto &oid : oids)
		types.append(getType(oid, generate_xml));

	return types;
}
