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

#include "physicaltable.h"
#include "utilsns.h"
#include "coreutilsns.h"
#include "csvparser.h"

PhysicalTable::PhysicalTable()
{
	gen_alter_cmds=false;
	attributes[Attributes::Columns]="";
	attributes[Attributes::InhColumns]="";
	attributes[Attributes::Constraints]="";
	attributes[Attributes::ColsComment]="";
	attributes[Attributes::AncestorTable]="";
	attributes[Attributes::GenAlterCmds]="";
	attributes[Attributes::ConstrSqlDisabled]="";
	attributes[Attributes::ColIndexes]="";
	attributes[Attributes::ConstrIndexes]="";
	attributes[Attributes::InitialData]="";
	attributes[Attributes::Partitioning]="";
	attributes[Attributes::PartitionKey]="";
	attributes[Attributes::PartitionedTable]="";
	attributes[Attributes::PartitionBoundExpr]="";
	attributes[Attributes::CopyTable]="";

	copy_table=partitioned_table=nullptr;
	partitioning_type=PartitioningType::Null;
}

void PhysicalTable::destroyObjects()
{
	std::vector<BaseObject *> list=getObjects();

	while(!list.empty())
	{
		delete list.back();
		list.pop_back();
	}

	ancestor_tables.clear();
	partition_tables.clear();
}

void PhysicalTable::setName(const QString &name)
{
	QString prev_name=this->getName(true);
	BaseObject::setName(name);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void PhysicalTable::setSchema(BaseObject *schema)
{
	QString prev_name=this->getName(true);
	BaseObject::setSchema(schema);
	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

void PhysicalTable::setCopyTable(PhysicalTable *tab)
{
	setCodeInvalidated(copy_table != tab);
	copy_table=tab;

	if(!copy_table)
		copy_op = CopyOptions();
}

void PhysicalTable::setCopyTableOptions(CopyOptions cpy_op)
{
	if(copy_table)
	{
		setCodeInvalidated(copy_op != cpy_op);
		this->copy_op = cpy_op;
	}
}

PhysicalTable *PhysicalTable::getCopyTable()
{
	return copy_table;
}

CopyOptions PhysicalTable::getCopyTableOptions()
{
	return copy_op;
}

void PhysicalTable::setPartitioningType(PartitioningType part_type)
{
	setCodeInvalidated(partitioning_type != part_type);
	partitioning_type = part_type;

	if(part_type == PartitioningType::Null)
		partition_keys.clear();
	else
		// If changing the partitioning type of the table the ALTER commands for columns and constraints is disabled
		setGenerateAlterCmds(false);
}

PartitioningType PhysicalTable::getPartitioningType()
{
	return partitioning_type;
}

PhysicalTable *PhysicalTable::getPartitionedTable()
{
	return partitioned_table;
}

void PhysicalTable::setProtected(bool value)
{
	std::vector<ObjectType> obj_types = getChildObjectTypes(obj_type);
	std::vector<TableObject *> *list=nullptr;

	//Protected the table child objects
	for(auto &type : obj_types)
	{
		list=getObjectList(type);

		for(auto &tab_obj : *list)
		{
			/* Relationship included object are always protected, so
			the protection state of this objects is not altered */
			if(!tab_obj->isAddedByRelationship())
				tab_obj->setProtected(value);
		}
	}

	//Protectes the table itself
	BaseGraphicObject::setProtected(value);
}

void PhysicalTable::setCommentAttribute(TableObject *tab_obj)
{
	if(tab_obj && !tab_obj->getComment().isEmpty() && tab_obj->isDeclaredInTable())
	{
		attribs_map attribs;

		attribs[Attributes::Signature]=tab_obj->getSignature();
		attribs[Attributes::SqlObject]=tab_obj->getSQLName();
		attribs[Attributes::Column]=(tab_obj->getObjectType()==ObjectType::Column ? Attributes::True : "");
		attribs[Attributes::Constraint]=(tab_obj->getObjectType()==ObjectType::Constraint ? Attributes::True : "");
		attribs[Attributes::Table]=this->getName(true);
		attribs[Attributes::Name]=tab_obj->getName(true);

		QString comment = tab_obj->getEscapedComment(BaseObject::isEscapeComments());
		attribs[Attributes::EscapeComment]=BaseObject::isEscapeComments() ? Attributes::True : "";
		attribs[Attributes::Comment]=comment;

		schparser.ignoreUnkownAttributes(true);
		if(tab_obj->isSQLDisabled())
			attributes[Attributes::ColsComment]+="-- ";

		attributes[Attributes::ColsComment]+=schparser.getSourceCode(Attributes::Comment, attribs, SchemaParser::SqlCode);
		schparser.ignoreUnkownAttributes(false);
	}
}

void PhysicalTable::setAncestorTableAttribute()
{
	unsigned i, count=ancestor_tables.size();
	QStringList list;

	for(i=0; i < count; i++)
		list.push_back(ancestor_tables[i]->getName(true));

	attributes[Attributes::AncestorTable]=list.join(',');
}

void PhysicalTable::setRelObjectsIndexesAttribute()
{
	attribs_map aux_attribs;
	std::vector<std::map<QString, unsigned> *> obj_indexes={ &col_indexes, &constr_indexes };
	QString attribs[]={ Attributes::ColIndexes,  Attributes::ConstrIndexes };
	ObjectType obj_types[]={ ObjectType::Column, ObjectType::Constraint };
	unsigned idx=0, size=obj_indexes.size();

	for(idx=0; idx < size; idx++)
	{
		attributes[attribs[idx]]="";

		if(!obj_indexes[idx]->empty())
		{
			for(auto &obj_idx : (*obj_indexes[idx]))
			{
				aux_attribs[Attributes::Name]=obj_idx.first;
				aux_attribs[Attributes::Index]=QString::number(obj_idx.second);

				schparser.ignoreUnkownAttributes(true);
				aux_attribs[Attributes::Objects]+=schparser.getSourceCode(Attributes::Object, aux_attribs, SchemaParser::XmlCode);
			}

			aux_attribs[Attributes::ObjectType]=BaseObject::getSchemaName(obj_types[idx]);
			attributes[attribs[idx]]=schparser.getSourceCode(Attributes::CustomIdxs, aux_attribs, SchemaParser::XmlCode);
			aux_attribs.clear();
		}
	}
}

void PhysicalTable::setColumnsAttribute(SchemaParser::CodeType def_type, bool incl_rel_added_cols, bool incl_constraints)
{
	QStringList cols, inh_cols;

	for(auto &col : columns)
	{
		/* Do not generates the column code definition when it is not included by
		 relatoinship, in case of XML definition. */
		if((def_type==SchemaParser::SqlCode && !col->isAddedByCopy() && !col->isAddedByGeneralization()) ||
			 (def_type==SchemaParser::XmlCode && (!col->isAddedByRelationship() || (incl_rel_added_cols && col->isAddedByRelationship()))))
		{
			cols.append(col->getSourceCode(def_type));

			if(def_type==SchemaParser::SqlCode)
				setCommentAttribute(col);
		}
		else if(def_type==SchemaParser::SqlCode && col->isAddedByGeneralization() && !gen_alter_cmds)
		{
			inh_cols.append("-- " + col->getSourceCode(def_type));
		}
	}

	if(def_type == SchemaParser::SqlCode)
	{
		if(!cols.isEmpty())
		{
			/* Check if some column has its sql disabled. If so,
			 * its necessary to make some tweaks in order to not
			 * generate bad sql code */
			unsigned i = cols.size()-1;
			bool has_constr_enabled = false;
			Constraint *constr = nullptr;

			/* Checking if we have some primary key, check or exclude constraints enabled
			 * so we can determine if the last comma in the column definition must be removed */
			for(auto &obj : constraints)
			{
				constr = dynamic_cast<Constraint *>(obj);

				if(incl_constraints && !constr->isSQLDisabled() &&
					 constr->getConstraintType() != ConstraintType::ForeignKey)
				{
					has_constr_enabled = true;
					break;
				}
			}

			if(!has_constr_enabled)
			{
				//If the last line starts with -- indicates that sql code for the column is disabled
				if(cols[i].startsWith("--") && i > 0)
					//Removes the comma from the above line in order to avoid bad sql
					cols[i - 1].remove(cols[i-1].lastIndexOf(","), 1);
				else
					//Otherwise removes the comma from the last line
					cols[i].remove(cols[i].lastIndexOf(","), 1);
			}
		}

		for(auto &inh : inh_cols)
			attributes[Attributes::InhColumns] += inh;
	}

	for(auto &col : cols)
		attributes[Attributes::Columns] += col;
}

void PhysicalTable::setConstraintsAttribute(SchemaParser::CodeType def_type)
{
	QString str_constr;
	bool inc_added_by_rel;
	Constraint *constr=nullptr;
	std::vector<QString> lines;

	for(auto &tab_obj : constraints)
	{
		constr=dynamic_cast<Constraint *>(tab_obj);

		if(constr->getConstraintType()!=ConstraintType::ForeignKey &&

				((def_type==SchemaParser::SqlCode &&
					((!constr->isReferRelationshipAddedColumns() && constr->getConstraintType()!=ConstraintType::Check) ||
					 (constr->getConstraintType()==ConstraintType::Check && !constr->isAddedByGeneralization()) ||
					 constr->getConstraintType()==ConstraintType::PrimaryKey)) ||

				 (def_type==SchemaParser::XmlCode && !constr->isAddedByRelationship() &&
					((constr->getConstraintType()!=ConstraintType::PrimaryKey && !constr->isReferRelationshipAddedColumns()) ||
					 (constr->getConstraintType()==ConstraintType::PrimaryKey)))))
		{
			inc_added_by_rel=(def_type==SchemaParser::SqlCode);

			if(def_type==SchemaParser::XmlCode)
				str_constr+=constr->getSourceCode(def_type,inc_added_by_rel);
			else
				//For sql definition the generated constraints are stored in a vector to be treated below
				lines.push_back(constr->getSourceCode(def_type,inc_added_by_rel));

			if(def_type==SchemaParser::SqlCode)
				setCommentAttribute(constr);
		}
	}

	if(def_type==SchemaParser::SqlCode && !lines.empty())
	{
		unsigned i = 0;

		/* When the coistraints are being generated in form of ALTER commands
		simply concatenates all the lines */
		if(gen_alter_cmds)
		{
			for(i=0; i < lines.size(); i++)
				str_constr+=lines[i];
		}
		else
		{
			/* Check if some constraint has its sql disabled. If so,
				it necessary to make some tweaks in order to not generate bad sql code */
			i=lines.size()-1;
			unsigned dis_sql_cnt=0;

			//If the last line starts with -- indicates that sql code for the constraint is disable
			if(lines[i].startsWith("--") && i > 0)
				//Removes the comma from the above line in order to avoid bad sql
				lines[i-1].remove(lines[i-1].lastIndexOf(','), 1);
			else
				//Otherwise removes the comma from the last line
				lines[i].remove(lines[i].lastIndexOf(','), 1);

			for(i=0; i < lines.size(); i++)
			{
				if(lines[i].startsWith("--")) dis_sql_cnt++;
				str_constr+=lines[i];
			}

			attributes[Attributes::ConstrSqlDisabled]=(dis_sql_cnt==lines.size() ? Attributes::True : "");
		}
	}

	attributes[Attributes::Constraints]=str_constr;
}

std::vector<TableObject *> *PhysicalTable::getObjectList(ObjectType obj_type)
{
	if(obj_type==ObjectType::Column)
		return &columns;

	if(obj_type==ObjectType::Constraint)
		return &constraints;

	if(obj_type==ObjectType::Trigger)
		return &triggers;

	return nullptr;
}

void PhysicalTable::addObject(BaseObject *obj, int obj_idx)
{
	ObjectType obj_type;

	if(!obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	int idx;
	obj_type=obj->getObjectType();

	try
	{
		//Raises an error if already exists a object with the same name and type
		if(getObject(obj->getName(),obj_type,idx))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
											.arg(obj->getName(true))
											.arg(obj->getTypeName())
											.arg(this->getName(true))
											.arg(this->getTypeName()),
											ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		//Raises an error if the user try to set the table as ancestor/copy of itself
		if((isPhysicalTable(obj_type) || obj_type==ObjectType::BaseTable) && obj==this)
			throw Exception(ErrorCode::InvInheritCopyPartRelationship,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(!isPhysicalTable(obj_type))
		{
			TableObject *tab_obj;
			std::vector<TableObject *> *obj_list;
			Column *col;

			tab_obj=dynamic_cast<TableObject *>(obj);
			col=dynamic_cast<Column *>(tab_obj);

			//Sets the object parent table if there isn't one
			if(!tab_obj->getParentTable())
				tab_obj->setParentTable(this);

			//Raises an error if the parent table of the table object is different from table 'this'
			if(tab_obj->getParentTable()!=this)
				throw Exception(ErrorCode::AsgObjectBelongsAnotherTable,PGM_FUNC,PGM_FILE,PGM_LINE);

			//Validates the object SQL code befor insert on table
			obj->getSourceCode(SchemaParser::SqlCode);

			if(col && col->getType()==this)
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::InvColumnTableType)
												.arg(col->getName())
												.arg(this->getName()),
												ErrorCode::InvColumnTableType,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			if(obj_type==ObjectType::Constraint)
			{
				//Raises a error if the user try to add a second primary key on the table
				if(dynamic_cast<Constraint *>(tab_obj)->getConstraintType() == ConstraintType::PrimaryKey &&
					 this->getPrimaryKey())
					throw Exception(ErrorCode::AsgExistingPrimaryKeyTable,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			if(obj_type==ObjectType::Trigger)
				dynamic_cast<Trigger *>(tab_obj)->validateTrigger();

			obj_list=getObjectList(obj_type);

			//Adds the object to the table
			if(obj_idx < 0 || obj_idx >= static_cast<int>(obj_list->size()))
				obj_list->push_back(tab_obj);
			else
			{
				//If there is a object index specified inserts the object at the position
				if(obj_list->size() > 0)
					obj_list->insert((obj_list->begin() + obj_idx), tab_obj);
				else
					obj_list->push_back(tab_obj);
			}

			if(obj_type==ObjectType::Column || obj_type==ObjectType::Constraint)
			{
				updateAlterCmdsStatus();

				if(obj_type==ObjectType::Constraint)
					dynamic_cast<Constraint *>(tab_obj)->setColumnsNotNull(true);
			}

			tab_obj->updateDependencies();
		}
		else if(isPhysicalTable(obj_type))
		{
			PhysicalTable *tab = dynamic_cast<PhysicalTable *>(obj);

			if(obj_idx < 0 || obj_idx >= static_cast<int>(ancestor_tables.size()))
				ancestor_tables.push_back(tab);
			else
				ancestor_tables.insert((ancestor_tables.begin() + obj_idx), tab);
		}
		else
			throw Exception(ErrorCode::AsgObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		if(e.getErrorCode() == ErrorCode::UndefinedAttributeValue)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgObjectInvalidDefinition)
											.arg(obj->getName())
											.arg(obj->getTypeName()),
											ErrorCode::AsgObjectInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::addColumn(Column *col, int idx)
{
	try
	{
		addObject(col, idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::addTrigger(Trigger *trig, int idx)
{
	try
	{
		addObject(trig, idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::setPartionedTable(PhysicalTable *table)
{
	setCodeInvalidated(partitioned_table != table);

	if(table != partitioned_table && partitioned_table)
		partitioned_table->removePartitionTable(this);

	partitioned_table = table;

	if(partitioned_table)
	{
		partitioned_table->addPartitionTable(this);

		/* If the partitioned table is defined we need to disable the ALTER commands for columns and constraints
		 * in order to avoid SQL syntax errors */
		setGenerateAlterCmds(false);
	}
}

void PhysicalTable::setPartitionBoundingExpr(const QString part_bound_expr)
{
	setCodeInvalidated(part_bounding_expr != part_bound_expr);
	part_bounding_expr = part_bound_expr;
}

QString PhysicalTable::getPartitionBoundingExpr()
{
	return part_bounding_expr;
}

std::vector<PhysicalTable *> PhysicalTable::getPartionTables()
{
	return partition_tables;
}

bool PhysicalTable::isPartitionTableExists(PhysicalTable *table, bool compare_names)
{
	return (getPartitionTableIndex(table, compare_names) >= 0);
}

void PhysicalTable::addConstraint(Constraint *constr, int idx)
{
	try
	{
		addObject(constr, idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::addAncestorTable(PhysicalTable *tab, int idx)
{
	try
	{
		addObject(tab, idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::addPartitionTable(PhysicalTable *tab)
{
	if(tab && std::find(partition_tables.begin(), partition_tables.end(), tab) == partition_tables.end())
		partition_tables.push_back(tab);
}

void PhysicalTable::removePartitionTable(PhysicalTable *tab)
{
	int idx = getPartitionTableIndex(tab, false);

	if(idx >= 0)
		partition_tables.erase(partition_tables.begin() + idx);
}

int PhysicalTable::getPartitionTableIndex(PhysicalTable *tab, bool compare_names)
{
	if(!tab)
		return -1;

	std::vector<PhysicalTable *>::iterator itr = partition_tables.begin();

	while(itr != partition_tables.end())
	{
		if(*itr == tab || (compare_names && tab->getName(true) == (*itr)->getName(true)))
			break;

		itr++;
	}

	if(itr == partition_tables.end())
		return -1;

	return (itr - partition_tables.begin());
}

void PhysicalTable::addPartitionKeys(std::vector<PartitionKey> &part_keys)
{
	std::vector<PartitionKey> part_keys_bkp = partition_keys;

	if(partitioning_type == PartitioningType::Null)
		return;

	if(partitioning_type == PartitioningType::List && part_keys.size() > 1)
		throw Exception(Exception::getErrorMessage(ErrorCode::InvPartitionKeyCount).arg(this->getSignature()),
										ErrorCode::InvPartitionKeyCount,PGM_FUNC,PGM_FILE,PGM_LINE);

	partition_keys.clear();

	for(auto &part_key : part_keys)
	{
		if(std::find(partition_keys.begin(), partition_keys.end(), part_key) != partition_keys.end())
		{
			partition_keys = part_keys_bkp;
			throw Exception(ErrorCode::InsDuplicatedElement,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		if(part_key.getColumn() && part_key.getColumn()->isAddedByRelationship())
		{
			partition_keys = part_keys_bkp;
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgInvalidColumnPartitionKey)
											.arg(part_key.getColumn()->getSignature(true)),
											ErrorCode::AsgInvalidColumnPartitionKey,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		partition_keys.push_back(part_key);
	}

	setCodeInvalidated(true);
}

void PhysicalTable::removePartitionKeys()
{
  partition_keys.clear();
  setCodeInvalidated(true);
}

void PhysicalTable::removeObject(BaseObject *obj)
{
	try
	{
		if(obj)
		{
			TableObject *tab_obj=dynamic_cast<TableObject *>(obj);

			if(tab_obj)
			{
				removeObject(getObjectIndex(tab_obj), obj->getObjectType());
			}
			else
				removeObject(obj->getName(true), ObjectType::Table);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeObject(const QString &name, ObjectType obj_type)
{
	int idx;

	//Gets the object index
	getObject(name,obj_type,idx);

	//Removes the object If it was found (idx >= 0)
	if(idx>=0)
		removeObject(static_cast<unsigned>(idx),obj_type);
}

void PhysicalTable::removeObject(unsigned obj_idx, ObjectType obj_type)
{
	//Raises an error if the user try to remove a object with invalid type
	if(!TableObject::isTableObject(obj_type) && obj_type != ObjectType::Table)
		throw Exception(ErrorCode::RemObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);

	if(PhysicalTable::isPhysicalTable(obj_type) && obj_idx < ancestor_tables.size())
	{
		std::vector<PhysicalTable *>::iterator itr;
		itr=ancestor_tables.begin() + obj_idx;
		ancestor_tables.erase(itr);
	}
	else if(!PhysicalTable::isPhysicalTable(obj_type))
	{
		std::vector<TableObject *> *obj_list=getObjectList(obj_type);
		std::vector<TableObject *>::iterator itr;

		if(!obj_list)
			return;

		//Raises an error if the object index is out of bound
		if(obj_idx >= obj_list->size())
			throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(obj_type!=ObjectType::Column)
		{
			itr=obj_list->begin() + obj_idx;
			TableObject *tab_obj=(*itr);
			Constraint *constr=dynamic_cast<Constraint *>(tab_obj);

			tab_obj->setParentTable(nullptr);
			obj_list->erase(itr);

			if(constr && constr->getConstraintType()==ConstraintType::PrimaryKey)
				dynamic_cast<Constraint *>(tab_obj)->setColumnsNotNull(false);

			tab_obj->clearAllDepsRefs();
		}
		else
		{
			std::vector<TableObject *> refs;
			Column *column=nullptr;

			itr=obj_list->begin() + obj_idx;
			column=dynamic_cast<Column *>(*itr);

			//Gets the references to the column before the exclusion
			refs = getColumnReferences(column);

			//Case some trigger, constraint, index is referencing the column raises an error
			if(!refs.empty())
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RemInderectReference)
								.arg(column->getSignature())
								.arg(column->getTypeName())
								.arg(refs[0]->getSignature())
						.arg(refs[0]->getTypeName())
						.arg(this->getSignature())
						.arg(this->getTypeName()),
						ErrorCode::RemInderectReference,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			//Raises an error if the column is being referenced by any partition key
			if(isPartitionKeyRefColumn(column))
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::RemColumnRefByPartitionKey)
								.arg(column->getSignature()).arg(this->getSignature()),
								ErrorCode::RemColumnRefByPartitionKey,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			column->clearDependencies();
			column->setParentTable(nullptr);
			columns.erase(itr);
		}
	}

	setCodeInvalidated(true);
}

void PhysicalTable::removeColumn(const QString &name)
{
	try
	{
		removeObject(name,ObjectType::Column);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeColumn(unsigned idx)
{
	try
	{
		removeObject(idx,ObjectType::Column);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeTrigger(const QString &name)
{
	try
	{
		removeObject(name,ObjectType::Trigger);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeTrigger(unsigned idx)
{
	try
	{
		removeObject(idx,ObjectType::Trigger);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeConstraint(const QString &name)
{
	try
	{
		removeObject(name,ObjectType::Constraint);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeConstraint(unsigned idx)
{
	try
	{
		removeObject(idx,ObjectType::Constraint);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeAncestorTable(const QString &name)
{
	try
	{
		removeObject(name,ObjectType::Table);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PhysicalTable::removeAncestorTable(unsigned idx)
{
	try
	{
		removeObject(idx,ObjectType::Table);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

int PhysicalTable::getObjectIndex(const QString &name, ObjectType obj_type)
{
	int idx;
	getObject(name, obj_type, idx);
	return idx;
}

int PhysicalTable::getObjectIndex(BaseObject *obj)
{
	TableObject *tab_obj=dynamic_cast<TableObject *>(obj);
	std::vector<TableObject *> *obj_list = nullptr;
	std::vector<TableObject *>::iterator itr, itr_end;
	bool found=false;

	if(!obj) return -1;

	obj_list = getObjectList(obj->getObjectType());
	if(!obj_list) return -1;

	itr=obj_list->begin();
	itr_end=obj_list->end();

	while(itr!=itr_end && !found)
	{
		found=((tab_obj->getParentTable()==this && (*itr)==tab_obj) ||
					 (tab_obj->getName()==(*itr)->getName()));

		if(!found)
			itr++;
	}

	if(found)
		return (itr-obj_list->begin());

	return -1;
}

BaseObject *PhysicalTable::getObject(const QString &name, ObjectType obj_type)
{
	int idx;
	return (getObject(name, obj_type, idx));
}

BaseObject *PhysicalTable::getObject(const QString &name, ObjectType obj_type, int &obj_idx)
{
	BaseObject *object=nullptr;
	bool found=false, format=false;
	std::vector<TableObject *> *obj_list=getObjectList(obj_type);

	//Checks if the name contains ", if so, the search will consider formatted names
	format=name.contains('"');

	if(TableObject::isTableObject(obj_type) && obj_list)
	{
		std::vector<TableObject *>::iterator itr, itr_end;
		QString aux_name=name;

		itr=obj_list->begin();
		itr_end=obj_list->end();

		while(itr!=itr_end)
		{
			found=((*itr)->getName(format)==aux_name);
			if(!found) itr++;
			else break;
		}

		if(found)
		{
			obj_idx=(itr-obj_list->begin());
			object=(*itr);
		}
		else obj_idx=-1;
	}
	else if(isPhysicalTable(obj_type))
	{
		std::vector<PhysicalTable *>::iterator itr_tab, itr_end_tab;
		QString tab_name, aux_name=name;

		aux_name.remove('"');
		itr_tab=ancestor_tables.begin();
		itr_end_tab=ancestor_tables.end();

		while(itr_tab!=itr_end_tab)
		{
			/* Unlike other object types, tables are always compared with the FORMATTED NAME
			because they must be 'schema-qualified' preventing a table of the same name
			but different schemas are confused */
			tab_name=(*itr_tab)->getName(true).remove('"');
			found=(tab_name==aux_name);
			if(!found) itr_tab++;
			else break;
		}

		if(found)
		{
			obj_idx=(itr_tab-ancestor_tables.begin());
			object=(*itr_tab);
		}
		else obj_idx=-1;
	}
	else
		throw Exception(ErrorCode::ObtObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return object;
}

BaseObject *PhysicalTable::getObject(unsigned obj_idx, ObjectType obj_type)
{
	std::vector<TableObject *> *obj_list=nullptr;

	if(isPhysicalTable(obj_type))
	{
		//Raises an error if the object index is out of bound
		if(obj_idx >= ancestor_tables.size())
			throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

		return ancestor_tables[obj_idx];
	}

	obj_list = getObjectList(obj_type);

	if(!obj_list)
		return nullptr;

	//Raises an error if the object index is out of bound
	if(obj_idx >= obj_list->size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return obj_list->at(obj_idx);
}

PhysicalTable *PhysicalTable::getAncestorTable(const QString &name)
{
	int idx;
	return dynamic_cast<PhysicalTable *>(getObject(name, ObjectType::Table, idx));
}

PhysicalTable *PhysicalTable::getAncestorTable(unsigned idx)
{
	return dynamic_cast<PhysicalTable *>(getObject(idx, ObjectType::Table));
}

Constraint *PhysicalTable::getPrimaryKey()
{
	Constraint *constr = nullptr;

	for(auto &tab_obj : constraints)
	{
		constr = dynamic_cast<Constraint *>(tab_obj);

		if(constr->getConstraintType() == ConstraintType::PrimaryKey)
			return constr;
	}

	return nullptr;
}

Column *PhysicalTable::getColumn(const QString &name, bool ref_old_name)
{
	if(!ref_old_name)
	{
		int idx;
		return dynamic_cast<Column *>(getObject(name, ObjectType::Column, idx));
	}

	Column *column = nullptr;
	std::vector<TableObject *>::iterator itr, itr_end;
	bool found = false, format = false;

	format = name.contains('"');
	itr = columns.begin();
	itr_end=columns.end();

	//Search the column referencing the old name
	while(itr != itr_end && !found)
	{
		column = dynamic_cast<Column *>(*itr);
		itr++;
		found = (!name.isEmpty() &&
						 column->getOldName(format) == name);
	}

	if(!found)
		column = nullptr;

	return column;
}

Column *PhysicalTable::getColumn(unsigned idx)
{
	return dynamic_cast<Column *>(getObject(idx,ObjectType::Column));
}

Trigger *PhysicalTable::getTrigger(const QString &name)
{
	int idx;
	return dynamic_cast<Trigger *>(getObject(name,ObjectType::Trigger,idx));
}

Trigger *PhysicalTable::getTrigger(unsigned idx)
{
	return dynamic_cast<Trigger *>(getObject(idx,ObjectType::Trigger));
}

Constraint *PhysicalTable::getConstraint(const QString &name)
{
	int idx;
	return dynamic_cast<Constraint *>(getObject(name,ObjectType::Constraint,idx));
}

Constraint *PhysicalTable::getConstraint(unsigned idx)
{
	return dynamic_cast<Constraint *>(getObject(idx,ObjectType::Constraint));
}

unsigned PhysicalTable::getColumnCount()
{
	return columns.size();
}

unsigned PhysicalTable::getTriggerCount()
{
	return triggers.size();
}

unsigned PhysicalTable::getConstraintCount()
{
	return constraints.size();
}

unsigned PhysicalTable::getAncestorTableCount()
{
	return ancestor_tables.size();
}

unsigned PhysicalTable::getObjectCount(ObjectType obj_type, bool inc_added_by_rel)
{
	if(!TableObject::isTableObject(obj_type) && !isPhysicalTable(obj_type))
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(isPhysicalTable(obj_type))
		return ancestor_tables.size();

	std::vector<TableObject *> *list=nullptr;
	list = getObjectList(obj_type);

	if(!list)
		return 0;

	if(!inc_added_by_rel)
	{
		std::vector<TableObject *>::iterator itr, itr_end;
		unsigned count = 0;

		itr = list->begin();
		itr_end = list->end();

		while(itr!=itr_end)
		{
			if(!(*itr)->isAddedByRelationship())
				count++;

			itr++;
		}

		return count;
	}

	return list->size();
}

void PhysicalTable::setRelObjectsIndexes(const std::vector<QString> &obj_names, const std::vector<unsigned> &idxs, ObjectType obj_type)
{
	if(!obj_names.empty() && obj_names.size()==idxs.size())
	{
		std::map<QString, unsigned > *obj_idxs_map=nullptr;
		unsigned idx=0, size=obj_names.size();

		if(obj_type==ObjectType::Column)
			obj_idxs_map=&col_indexes;
		else if(obj_type==ObjectType::Constraint)
			obj_idxs_map=&constr_indexes;
		else
			throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

		for(idx=0; idx < size; idx++)
			(*obj_idxs_map)[obj_names[idx]]=idxs[idx];
	}
}

void PhysicalTable::saveRelObjectsIndexes(ObjectType obj_type)
{
	std::map<QString, unsigned > *obj_idxs_map=nullptr;
	std::vector<TableObject *> *list=nullptr;

	if(obj_type==ObjectType::Column)
	{
		obj_idxs_map=&col_indexes;
		list=&columns;
	}
	else if(obj_type==ObjectType::Constraint)
	{
		obj_idxs_map=&constr_indexes;
		list=&constraints;
	}

	obj_idxs_map->clear();
	setCodeInvalidated(true);

	if(isReferRelationshipAddedObject())
	{
		unsigned idx=0;

		for(auto &obj : (*list))
		{
			if(obj->isAddedByLinking())
				(*obj_idxs_map)[obj->getName()]=idx;

			idx++;
		}
	}
}

void PhysicalTable::resetRelObjectsIndexes()
{
	col_indexes.clear();
	constr_indexes.clear();
}

void PhysicalTable::saveRelObjectsIndexes()
{
	saveRelObjectsIndexes(ObjectType::Column);
	saveRelObjectsIndexes(ObjectType::Constraint);
}

void PhysicalTable::restoreRelObjectsIndexes()
{
	restoreRelObjectsIndexes(ObjectType::Column);
	restoreRelObjectsIndexes(ObjectType::Constraint);

	if(!col_indexes.empty() || !constr_indexes.empty())
	{
		setCodeInvalidated(true);
		this->setModified(true);
	}
}

void PhysicalTable::restoreRelObjectsIndexes(ObjectType obj_type)
{
	std::map<QString, unsigned> *obj_idxs=nullptr;

	if(obj_type==ObjectType::Column)
		obj_idxs=&col_indexes;
	else
		obj_idxs=&constr_indexes;

	if(!obj_idxs->empty())
	{
		std::vector<TableObject *> *list = getObjectList(obj_type);
		std::vector<TableObject *> new_list;
		QString name;
		TableObject *tab_obj = nullptr;
		unsigned i = 0, pos = 0, size = 0, obj_idx, names_used = 0, aux_size = 0;

		size = list->size();

		/* Indentify the maximum index on the existing rel objects. This is done
		 * 	to correctly resize the new list in order to avoid exceed the list bounds
		 * 	and consequently crashing the app */
		for(auto &itr : *obj_idxs)
		{
			if(aux_size < (itr.second + 1))
				aux_size = itr.second + 1;
		}

		/* If the auxiliary size is lesser than the current object list size
		 * the new list is resized with same capacity of the "list" vector */
		if(aux_size < size)
			aux_size = size;

		new_list.resize(aux_size);

		for(auto &obj : *list)
		{
			name = obj->getName();

			//Check if the current object is a relationship created one and its name is on the custom index map
			if(obj->isAddedByLinking() && obj_idxs->count(name))
			{
				//Allocate the object on its original position
				obj_idx = obj_idxs->at(name);
				new_list[obj_idx] = obj;
				names_used++;
			}
		}

		/* Allocating the other objects, the ones that aren't created by relationship or
		 * the one which were created by relationship but weren't positioned yet */
		pos = i= 0;
		while(pos < size && i < size)
		{
			tab_obj=list->at(pos);
			name=tab_obj->getName();

			if(!new_list[i] && obj_idxs->count(name) == 0)
			{
				new_list[i]=tab_obj;
				pos++;
				i++;
			}
			else if(obj_idxs->count(name) != 0)
				pos++;
			else if(new_list[i])
				i++;
		}

		list->clear();
		for(auto &obj : new_list)
		{
			if(!obj) continue;
			list->push_back(obj);
		}

		/* Checking if the object names used are equal to the map size. If not, indicates that
		 * one o more objects on the map doesn't exists anymore on the table thus there is
		 * the need to updated the object index map */
		if(names_used != obj_idxs->size())
			saveRelObjectsIndexes(obj_type);
	}
}

bool PhysicalTable::isConstraintRefColumn(Column *column, ConstraintType constr_type)
{
	bool found=false;
	std::vector<TableObject *>::iterator itr, itr_end;
	Constraint *constr=nullptr;

	if(column)
	{
		itr=constraints.begin();
		itr_end=constraints.end();
		while(itr!=itr_end && !found)
		{
			constr=dynamic_cast<Constraint *>(*itr);
			itr++;
			found=(constr->getConstraintType()==constr_type &&
				   constr->isColumnReferenced(column));
		}
	}

	return found;
}

bool PhysicalTable::isPartitionKeyRefColumn(Column *column)
{
	bool found=false;

	if(column)
	{
		for(auto &part_key : partition_keys)
		{
			if(part_key.getColumn() == column)
			{
				found = true;
				break;
			}
		}
	}

	return found;
}

void PhysicalTable::setGenerateAlterCmds(bool value)
{
	if(value && (isPartition() || isPartitioned()))
	{
		/* Forcing the disabling of ALTER commands for columns and constraints
		 * if the table is a partition or partitioned table in order to avoid
		 * SQL syntax errors */
		setCodeInvalidated(true);
		gen_alter_cmds = false;
		updateAlterCmdsStatus();
	}
	else
		__setGenerateAlterCmds(value);
}

void PhysicalTable::__setGenerateAlterCmds(bool value)
{
	setCodeInvalidated(gen_alter_cmds != value);
	gen_alter_cmds = value;
	updateAlterCmdsStatus();
}

bool PhysicalTable::isGenerateAlterCmds()
{
	return gen_alter_cmds;
}

void PhysicalTable::updateAlterCmdsStatus()
{
	unsigned i;

	for(i=0; i < columns.size(); i++)
		columns[i]->setDeclaredInTable(!gen_alter_cmds);

	//Foreign keys are aways created as ALTER form
	for(i=0; i < constraints.size(); i++)
		constraints[i]->setDeclaredInTable(!gen_alter_cmds &&
																			 dynamic_cast<Constraint *>(constraints[i])->getConstraintType()!=ConstraintType::ForeignKey);
}

void PhysicalTable::setTableAttributes(SchemaParser::CodeType def_type, bool incl_rel_added_objs, bool incl_contraints)
{
	QStringList part_keys_code;

	attributes[Attributes::GenAlterCmds]=(gen_alter_cmds ? Attributes::True : "");
	attributes[Attributes::AncestorTable]="";
	attributes[Attributes::PartitionedTable]="";
	attributes[Attributes::Tag]="";
	attributes[Attributes::Partitioning]=~partitioning_type;
	attributes[Attributes::PartitionKey]="";
	attributes[Attributes::PartitionBoundExpr]=part_bounding_expr;
	attributes[Attributes::Pagination]=(pagination_enabled ? Attributes::True : "");
	attributes[Attributes::CollapseMode]=QString::number(collapse_mode);
	attributes[Attributes::AttribsPage]=(pagination_enabled ? QString::number(curr_page[AttribsSection]) : "");
	attributes[Attributes::ExtAttribsPage]=(pagination_enabled ? QString::number(curr_page[ExtAttribsSection]) : "");

	for(auto part_key : partition_keys)
		part_keys_code+=part_key.getSourceCode(def_type);

	if(def_type == SchemaParser::SqlCode)
		attributes[Attributes::PartitionKey]=part_keys_code.join(',');
	else
		attributes[Attributes::PartitionKey]=part_keys_code.join(' ');

	if(def_type==SchemaParser::SqlCode && partitioned_table)
		attributes[Attributes::PartitionedTable]=partitioned_table->getName(true);

	if(tag && def_type==SchemaParser::XmlCode)
		attributes[Attributes::Tag]=tag->getSourceCode(def_type, true);

	setColumnsAttribute(def_type, incl_rel_added_objs, incl_contraints);

	if(incl_contraints)
		setConstraintsAttribute(def_type);

	setAncestorTableAttribute();

	if(def_type==SchemaParser::XmlCode)
	{
		setRelObjectsIndexesAttribute();
		setPositionAttribute();
		setFadedOutAttribute();
		setLayersAttribute();
		attributes[Attributes::InitialData]=initial_data;
		attributes[Attributes::MaxObjCount]=QString::number(static_cast<unsigned>(getMaxObjectCount() * 1.20));
		attributes[Attributes::ZValue]=QString::number(z_value);
	}
	else
		attributes[Attributes::InitialData]=getInitialDataCommands();
}

void PhysicalTable::operator = (PhysicalTable &table)
{
	QString prev_name = this->getName(true);

	(*dynamic_cast<BaseTable *>(this))=dynamic_cast<BaseTable &>(table);

	this->layers = table.layers;
	this->col_indexes=table.col_indexes;
	this->constr_indexes=table.constr_indexes;
	this->partitioning_type=table.partitioning_type;
	this->initial_data=table.initial_data;
	this->partition_keys=table.partition_keys;

	PgSqlType::renameUserType(prev_name, this, this->getName(true));
}

bool PhysicalTable::isReferRelationshipAddedObject()
{
	std::vector<TableObject *>::iterator itr, itr_end;
	ObjectType types[]={ ObjectType::Column, ObjectType::Constraint };
	bool found=false;

	for(unsigned i=0; i < 2 && !found; i++)
	{
		itr=getObjectList(types[i])->begin();
		itr_end=getObjectList(types[i])->end();

		while(itr!=itr_end && !found)
		{
			found=(*itr)->isAddedByRelationship();
			itr++;
		}
	}

	return found;
}

bool PhysicalTable::isPartition()
{
	return (partitioned_table != nullptr);
}

bool PhysicalTable::isPartitioned()
{
	return (partitioning_type != PartitioningType::Null);
}

bool PhysicalTable::isPhysicalTable(ObjectType obj_type)
{
	return (BaseTable::isBaseTable(obj_type) && obj_type != ObjectType::View);
}

void PhysicalTable::swapObjectsIndexes(ObjectType obj_type, unsigned idx1, unsigned idx2)
{
	std::vector<TableObject *> *obj_list=nullptr;
	std::vector<TableObject *>::iterator itr1, itr2;
	TableObject *aux_obj=nullptr, *aux_obj1=nullptr;

	try
	{
		if(idx1!=idx2)
		{
			obj_list=getObjectList(obj_type);

			//Raises an error if both index is out of list bounds
			if(idx1 >= obj_list->size() && idx2 >= obj_list->size())
				throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

			//If the idx1 is out of bound inserts the element idx2 at the list's begin
			if(idx1 >= obj_list->size())
			{
				aux_obj1=obj_list->front();
				itr2=obj_list->begin() + idx2;
				aux_obj=(*itr2);
				obj_list->erase(itr2);
				obj_list->insert(obj_list->begin(), aux_obj);
			}
			//If the idx2 is out of bound inserts the element idx1 on the list's end
			else if(idx2 >= obj_list->size())
			{
				itr1=obj_list->begin() + idx1;
				aux_obj=(*itr1);
				aux_obj1=obj_list->back();
				obj_list->erase(itr1);
				obj_list->push_back(aux_obj);
			}
			else
			{
				aux_obj=obj_list->at(idx1);
				itr1=obj_list->begin() + idx1;
				itr2=obj_list->begin() + idx2;

				(*itr1)=aux_obj1=(*itr2);
				(*itr2)=aux_obj;
			}

			if(obj_type!=ObjectType::Column && obj_type!=ObjectType::Constraint)
				BaseObject::swapObjectsIds(aux_obj, aux_obj1, false);

			setCodeInvalidated(true);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

std::vector<TableObject *> PhysicalTable::getColumnReferences(Column *column)
{
	if(!column || column->isAddedByRelationship())
		return {};

	std::vector<BaseObject *> refs = column->getReferences();
	std::vector<TableObject *> col_refs;

	std::for_each(refs.begin(), refs.end(),
								[&col_refs](auto &obj)
								{
									if(TableObject::isTableObject(obj->getObjectType()))
										col_refs.push_back(dynamic_cast<TableObject *>(obj));
								});

	return col_refs;
}

std::vector<BaseObject *> PhysicalTable::getObjects(const std::vector<ObjectType> &excl_types)
{
	std::vector<BaseObject *> list;
	std::vector<ObjectType> types = getChildObjectTypes(obj_type);

	for(auto type : types)
	{
		if(std::find(excl_types.begin(), excl_types.end(), type) != excl_types.end())
			continue;

		list.insert(list.end(), getObjectList(type)->begin(), getObjectList(type)->end()) ;
	}

	return list;
}

std::vector<PartitionKey> PhysicalTable::getPartitionKeys()
{
	return partition_keys;
}

void PhysicalTable::setCodeInvalidated(bool value)
{
	std::vector<ObjectType> types = getChildObjectTypes(obj_type);

	for(auto type : types)
	{
		for(auto &obj : *getObjectList(type))
			obj->setCodeInvalidated(value);
	}

	BaseTable::setCodeInvalidated(value);
}

void PhysicalTable::setInitialData(const QString &value)
{
	setCodeInvalidated(initial_data != value);
	initial_data = value;
}

QString PhysicalTable::getInitialData()
{
	return initial_data;
}

QString PhysicalTable::getInitialDataCommands()
{
	CsvDocument csv_doc;
	CsvParser csv_parser;

	try
	{
		csv_parser.setColumnInFirstRow(true);
		csv_doc = csv_parser.parseBuffer(initial_data);
	}
	catch(Exception &e)
	{
		return tr("/* Failed to create initial data commands!\n\n%1 */").arg(e.getErrorMessage());
	}

	if(csv_doc.isEmpty())
		return "";

	QStringList col_names, col_values, commands, selected_cols;
	int curr_col=0;
	QList<int> ignored_cols;

	col_names = csv_doc.getColumnNames();
	col_names.removeDuplicates();

	//Separating valid columns (selected) from the invalids (ignored)
	for(auto &col_name : col_names)
	{
		if(getObjectIndex(col_name, ObjectType::Column) >= 0)
			selected_cols.append(col_name);
		else
			ignored_cols.append(curr_col);

		curr_col++;
	}

	for(int row = 0; row < csv_doc.getRowCount(); row++)
	{
		//Filtering the invalid columns' values
		for(int col = 0; col < csv_doc.getColumnCount(); col++)
		{
			if(ignored_cols.contains(col))
				continue;

			col_values.append(csv_doc.getValue(row, col));
		}

		commands.append(createInsertCommand(selected_cols, col_values));
		col_values.clear();
	}

	return commands.join('\n');
}

QString PhysicalTable::createInsertCommand(const QStringList &col_names, const QStringList &values)
{
	QString fmt_cmd, insert_cmd = QString("INSERT INTO %1 (%2) VALUES (%3);\n%4");
	QStringList val_list, col_list;
	int curr_col=0;

	for(auto &col_name : col_names)
		col_list.push_back(BaseObject::formatName(col_name));

	for(auto value : values)
	{
		//Empty values as considered as DEFAULT
		if(value.isEmpty())
		{
			value = "DEFAULT";
		}
		//Unescaped values will not be enclosed in quotes
		else if(value.startsWith(UtilsNs::UnescValueStart) && value.endsWith(UtilsNs::UnescValueEnd))
		{
			value.remove(0,1);
			value.remove(value.length()-1, 1);
		}
		//Quoting value
		else
		{
			value.replace(QString("\\") + UtilsNs::UnescValueStart, UtilsNs::UnescValueStart);
			value.replace(QString("\\") + UtilsNs::UnescValueEnd, UtilsNs::UnescValueEnd);
			value.replace("\'", "''");
			value.replace(QChar(QChar::LineFeed), "\\n");
			value="E'" + value + "'";
		}

		val_list.push_back(value);
	}

	if(!col_list.isEmpty() && !val_list.isEmpty())
	{
		//If the set of values is greater than the set of columns it will be truncated
		if(val_list.size() > col_list.size())
			val_list.erase(val_list.begin() + col_list.size(), val_list.end());
		//If the set of columns is greater than the set of values than DEFAULT values will be provided
		else if(col_list.size() > val_list.size())
		{
			for(curr_col = val_list.size(); curr_col < col_list.size(); curr_col++)
				val_list.append("DEFAULT");
		}

		fmt_cmd=insert_cmd.arg(getSignature()).arg(col_list.join(", "))
									.arg(val_list.join(", ")).arg(Attributes::DdlEndToken);
	}

	return fmt_cmd;
}

void PhysicalTable::setObjectListsCapacity(unsigned capacity)
{
	if(capacity < DefMaxObjectCount || capacity > DefMaxObjectCount * 10)
		capacity = DefMaxObjectCount;

	for(auto &type : getChildObjectTypes(obj_type))
		getObjectList(type)->reserve(type != ObjectType::Column ? capacity/2 : capacity);
}

unsigned PhysicalTable::getMaxObjectCount()
{
	unsigned count = 0, max = 0;

	for(auto &type : getChildObjectTypes(obj_type))
	{
		count = getObjectList(type)->size();
		if(count > max) max = count;
	}

	return max;
}

QString PhysicalTable::getDataDictionary(bool split, bool md_format, const attribs_map &extra_attribs)
{
	Column *column = nullptr;
	attribs_map attribs, aux_attrs;
	QStringList tab_names, attr_names = { Attributes::Columns, Attributes::Constraints,
																									Attributes::Triggers, Attributes::Indexes };
	QString	link_dict_file = GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Link);

	attribs.insert(extra_attribs.begin(), extra_attribs.end());
	attribs[Attributes::Type] = getTypeName();
	attribs[Attributes::TypeClass] = getSchemaName();
	attribs[Attributes::Split] = split ? Attributes::True : "";
	attribs[Attributes::Name] = obj_name;
	attribs[Attributes::Schema] = schema ? schema->getName() : "";
	attribs[Attributes::Comment] = comment;

	/* Initialize the attributes for columns, trigger, indexes and contraints
	 * if there one or more missing in the attributes map */
	for(auto &attr : attr_names)
	{
	 if(!attribs.count(attr))
		 attribs[attr] = "";
	}

	aux_attrs[Attributes::Split] = attribs[Attributes::Split];

	try
	{
		// Gathering the acestor table names
		for(auto &tab : ancestor_tables)
		{
			aux_attrs[Attributes::Name] = tab->getSignature().remove(QChar('"'));
			tab_names.push_back(schparser.getSourceCode(link_dict_file, aux_attrs));
		}
		attribs[Attributes::Inherit] = tab_names.join(", ");
		tab_names.clear();

		attribs[Attributes::PartitionedTable] = "";
		if(partitioned_table)
		{
			aux_attrs[Attributes::Name] = partitioned_table->getSignature().remove(QChar('"'));
			attribs[Attributes::PartitionedTable] = schparser.getSourceCode(link_dict_file, aux_attrs);
		}

		// Gathering the patition table names
		for(auto &tab : partition_tables)
		{
			aux_attrs[Attributes::Name] = tab->getSignature().remove(QChar('"'));
			tab_names.push_back(schparser.getSourceCode(link_dict_file, aux_attrs));
		}
		attribs[Attributes::PartitionTables] = tab_names.join(", ");

		for(auto &obj : columns)
		{
			column = dynamic_cast<Column *>(obj);
			aux_attrs[Attributes::PkConstr] = isConstraintRefColumn(column, ConstraintType::PrimaryKey) ? CoreUtilsNs::DataDictCheckMark : "";
			aux_attrs[Attributes::UqConstr] = isConstraintRefColumn(column, ConstraintType::Unique) ? CoreUtilsNs::DataDictCheckMark : "";
			aux_attrs[Attributes::FkConstr] = isConstraintRefColumn(column, ConstraintType::ForeignKey) ? CoreUtilsNs::DataDictCheckMark : "";
			attribs[Attributes::Columns] += column->getDataDictionary(md_format, aux_attrs);
		}

		for(auto &obj : constraints)
		{
			attribs[Attributes::Constraints] +=
					dynamic_cast<Constraint *>(obj)->getDataDictionary(md_format, {{ Attributes::Split, attribs[Attributes::Split] }});
		}

		for(auto &obj : triggers)
		{
			attribs[Attributes::Triggers] +=
					dynamic_cast<Trigger *>(obj)->getDataDictionary(md_format, {{ Attributes::Split, attribs[Attributes::Split] }});
		}

		attribs[Attributes::Objects] += schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, Attributes::Objects), attribs);
		schparser.ignoreEmptyAttributes(true);
		return schparser.getSourceCode(GlobalAttributes::getDictSchemaFilePath(md_format, BaseObject::getSchemaName(ObjectType::Table)), attribs);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}
