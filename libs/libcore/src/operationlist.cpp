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

#include "operationlist.h"
#include "coreutilsns.h"

unsigned OperationList::max_size {500};

OperationList::OperationList(DatabaseModel *model)
{
	/* Raises an error if the user tries to allocate an operation list linked to
		to an unallocated model */
	if(!model)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->model=model;
	xmlparser=model->getXMLParser();
	current_index=0;
	next_op_chain=Operation::NoChain;
	ignore_chain=false;
	operations.reserve(max_size);
}

OperationList::~OperationList()
{
	removeOperations();
}

unsigned OperationList::getCurrentSize()
{
	return operations.size();
}

unsigned OperationList::getMaximumSize()
{
	return max_size;
}

int OperationList::getCurrentIndex()
{
	return current_index;
}

void OperationList::startOperationChain()
{
	/* If the chaining is started and the user try it initializes
		again, the earlier chaining is finished */
	if(next_op_chain!=Operation::NoChain)
		finishOperationChain();

	/* The next operation inserted on the list will be the start
		of the chaining */
	next_op_chain=Operation::ChainStart;
}

void OperationList::finishOperationChain()
{
	/* If the chain is not ignored indicates that the next
		element of the list no longer will be part of chaining */
	if(!ignore_chain)
		next_op_chain=Operation::NoChain;
	else if(ignore_chain)
		/* If the chain is canceled indicates that the next element
		 of the list still part of the chaining */
		next_op_chain=Operation::ChainMiddle;

	if(operations.size() > 0 && !ignore_chain)
	{
		unsigned idx=operations.size()-1;

		/* Marks the last operatin as being the end of chaining in case it is
		 on the middle of chain */
		if(operations[idx]->getChainType()==Operation::ChainMiddle)
			operations[idx]->setChainType(Operation::ChainEnd);

		/* If the last operation is marked as CHAIN_START indicates that
		 the chaining was open but only one operation is recorded
		 and thus the operation is marked as NO_CHAIN because as it is
		 only one operation there is no need to treat it as chaining */
		else if(operations[idx]->getChainType()==Operation::ChainStart)
			operations[idx]->setChainType(Operation::NoChain);
	}
}

void OperationList::ignoreOperationChain(bool value)
{
	ignore_chain=value;
}

bool OperationList::isOperationChainStarted()
{
	return (next_op_chain==Operation::ChainStart ||
					next_op_chain==Operation::ChainMiddle);
}

bool OperationList::isObjectRegistered(BaseObject *object, Operation::OperType op_type)
{
	bool registered=false;
	std::vector<Operation *>::iterator itr=operations.begin();

	while(itr!=operations.end() && !registered)
	{
		registered=((*itr)->getOriginalObject()==object && (*itr)->getOperationType()==op_type);
		itr++;
	}

	return registered;
}

bool OperationList::isRedoAvailable()
{
	/* The redo operation only can be performed
		if the current index from the list of operations is at most
		the penultimate element and the list can not be empty */
	return (!operations.empty() && current_index < static_cast<int>(operations.size()));
}

bool OperationList::isUndoAvailable()
{
	/* For the undo operation be performed is
		enough that the list of operations is not empty */
	return (!operations.empty() && current_index > 0);
}

void OperationList::setMaximumSize(unsigned max)
{
	//Raises an error if a zero max size is assigned to the list
	if(max==0)
		throw Exception(ErrorCode::AsgInvalidMaxSizeOpList,PGM_FUNC,PGM_FILE,PGM_LINE);

	max_size=max;
}

void OperationList::addToPool(BaseObject *object, Operation::OperType op_type)
{
	ObjectType obj_type;

	try
	{

		//Raises an error if the object to be added is not allocated
		if(!object)
			throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		obj_type=object->getObjectType();

		//Stores a copy of the object if its about to be moved or modified
		if(op_type==Operation::ObjModified ||
				op_type==Operation::ObjMoved)
		{
			BaseObject *copy_obj=nullptr;

			if(obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::Database)
				CoreUtilsNs::copyObject(&copy_obj, object, obj_type);
			else
				throw Exception(ErrorCode::AsgObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

			//Raises an error if the copy fails (returning a null object)
			if(!copy_obj)
				throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
			
			//Inserts the copy on the pool
			object_pool.push_back(copy_obj);
		}
		else
			//Inserts the original object on the pool (in case of adition or deletion operations)
			object_pool.push_back(object);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void OperationList::removeOperations()
{
	BaseObject *object=nullptr;
	TableObject *tab_obj=nullptr;
	BaseTable *tab=nullptr;
	Operation *oper=nullptr;
	std::vector<BaseObject *> invalid_objs;

	//Destroy the operations
	while(!operations.empty())
	{
		oper=operations.back();

		/* If the operation is not valid means that in some moment the pool object inside it
	   was destroyed (by a relationship invalidation for instance) and to avoid crashes
	   this object is stored in a invalid objects list */
		if(!oper->isOperationValid())
			invalid_objs.push_back(oper->getPoolObject());

		delete oper;
		operations.pop_back();
	}

	//Destroy the object pool
	while(!object_pool.empty())
		removeFromPool(0);

	//Case there is not removed object
	while(!not_removed_objs.empty())
	{
		object=not_removed_objs.back();

		//If the object is not an invalid one, proceed with its deallocation
		if(std::find(invalid_objs.begin(), invalid_objs.end(), object)==invalid_objs.end())
		{
			if(unallocated_objs.count(object)==0)
				tab_obj=dynamic_cast<TableObject *>(object);

			//Deletes the object if its not unallocated already or referenced on the model
			if(unallocated_objs.count(object)==0 &&
					(!tab_obj && model->getObjectIndex(object) < 0))
			{
				if(object->getObjectType()==ObjectType::Table)
				{
					std::vector<BaseObject *> list=dynamic_cast<Table *>(object)->getObjects();

					while(!list.empty())
					{
						unallocated_objs[list.back()]=true;
						list.pop_back();
					}
				}

				unallocated_objs[object]=true;
				delete object;
			}
			else if(tab_obj && unallocated_objs.count(tab_obj)==0)
			{
				tab = tab_obj->getParentTable();

				//Deletes the object if its not unallocated already or referenced by some table
				if(!tab ||
						(unallocated_objs.count(tab)==1) ||
						(tab && unallocated_objs.count(tab)==0 && tab->getObjectIndex(tab_obj) < 0))
				{
					unallocated_objs[tab_obj]=true;
					delete tab_obj;
				}
			}
		}

		not_removed_objs.pop_back();
		tab_obj=nullptr;
	}

	current_index=0;
	unallocated_objs.clear();
}

void OperationList::validateOperations()
{
	std::vector<Operation *>::iterator itr, itr_end;
	Operation *oper=nullptr;

	itr=operations.begin();
	itr_end=operations.end();

	while(itr!=itr_end)
	{
		oper=(*itr);

		//Case the object isn't on the pool
		if(!isObjectOnPool((*itr)->getPoolObject()) ||
				!oper->isOperationValid())
		{
			//Remove the operation
			operations.erase(itr);
			delete oper;
			itr=operations.begin();
			itr_end=operations.end();
		}
		else itr++;
	}
}

bool OperationList::isObjectOnPool(BaseObject *object)
{
	bool found=false;
	std::vector<BaseObject *>::iterator itr, itr_end;

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=object_pool.begin();
	itr_end=object_pool.end();
	while(!found && itr!=itr_end)
	{
		found=*(itr)==object;
		itr++;
	}

	return found;
}

void OperationList::removeFromPool(unsigned obj_idx)
{
	BaseObject *object=nullptr;
	std::vector<BaseObject *>::iterator itr;

	//Avoiding the removal of an object in invalid index (out of bound)
	if(obj_idx >= object_pool.size())
		return;

	//Gets the element to bo removed through its index
	itr=object_pool.begin() + obj_idx;
	object=(*itr);

	//Removes the object from pool
	object_pool.erase(itr);

	/* Stores the object that was in the pool on the 'not_removed_objs' vector.
		The object will be deleted in the destructor of the list. Note: The object is not
		deleted immediately because the model / table / list of operations may still
		referencing the object or it becomes referenced when an operation is performed on the list. */
	not_removed_objs.push_back(object);
}


int OperationList::registerObject(BaseObject *object, Operation::OperType op_type, int object_idx, BaseObject *parent_obj)
{
	ObjectType obj_type;
	Operation *operation=nullptr;
	BaseTable *parent_tab=nullptr;
	Relationship *parent_rel=nullptr;
	TableObject *tab_obj=nullptr;
	tab_obj=dynamic_cast<TableObject *>(object);
	int obj_idx=-1;

	try
	{
		//Raises an error if the user tries to register an operation with null object
		if(!object)
			throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		obj_type=object->getObjectType();

		if(tab_obj && !parent_obj)
			throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(parent_obj &&
				(((obj_type==ObjectType::Column || obj_type==ObjectType::Constraint) &&
					(parent_obj->getObjectType()!=ObjectType::Relationship && !PhysicalTable::isPhysicalTable(parent_obj->getObjectType()))) ||

				 ((obj_type==ObjectType::Trigger || obj_type==ObjectType::Rule || obj_type==ObjectType::Index) && !dynamic_cast<BaseTable *>(parent_obj))))
		{
			throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
		}		

		//If the operations list is full makes the automatic cleaning before inserting a new operation
		if(current_index == static_cast<int>(max_size-1))
			removeOperations();

		/* If adding an operation and the current index is not pointing
		 to the end of the list (available redo / user ran undo operations)
		 all elements from the current index to the end of the list will be deleted
		 as well as the objects in the pool that were linked to the excluded operations */
		if(current_index>=0 && static_cast<unsigned>(current_index)!=operations.size())
		{
			//Gets the last operation index
			int i=operations.size()-1;

			//Removes all the operation while the current index isn't reached
			while(i >= current_index)
			{
				removeFromPool(i);
				i--;
			}

			//Validates the remaining operatoins after the deletion
			validateOperations();
		}

		//Creates the new operation
		operation=new Operation;
		operation->setOperationType(op_type);
		operation->setChainType(next_op_chain);
		operation->setOriginalObject(object);

		//Adds the object on te pool
		addToPool(object, op_type);

		//Assigns the pool object to the operation
		operation->setPoolObject(object_pool.back());

		//Stores the object's permission befor its removal
		if(op_type==Operation::ObjRemoved)
		{
			std::vector<Permission *> perms;
			model->getPermissions(object, perms);
			operation->setPermissions(perms);
		}

		if(next_op_chain==Operation::ChainStart)
			next_op_chain=Operation::ChainMiddle;

		/*  Performing specific operations according to the type of object.
		 If the object has a parent object, it must be discovered
		 and moreover it is necessary to find and store the index of the object
		 in the list on the parent object */
		if(tab_obj)
		{
			if(parent_obj->getObjectType()==ObjectType::Relationship)
				parent_rel=dynamic_cast<Relationship *>(parent_obj);
			else
				parent_tab=dynamic_cast<BaseTable *>(parent_obj);

			if(((obj_type==ObjectType::Trigger && dynamic_cast<Trigger *>(tab_obj)->isReferRelationshipAddedColumn()) ||
				(obj_type==ObjectType::Index && dynamic_cast<Index *>(tab_obj)->isReferRelationshipAddedColumn()) ||
				(obj_type==ObjectType::Constraint && dynamic_cast<Constraint *>(tab_obj)->isReferRelationshipAddedColumns())))
			{
				if(op_type==Operation::ObjRemoved)
					tab_obj->setParentTable(parent_tab);

				if(tab_obj->getObjectType()==ObjectType::Constraint)
					operation->setXMLDefinition(dynamic_cast<Constraint *>(tab_obj)->getSourceCode(SchemaParser::XmlCode, true));
				else
					operation->setXMLDefinition(tab_obj->getSourceCode(SchemaParser::XmlCode));
			}

			operation->setParentObject(parent_obj);

			/* If there is a parent relationship will get the index of the object.
			Only columns and constraints are handled case the parent is a relationship */
			if(parent_rel && (obj_type==ObjectType::Column || obj_type==ObjectType::Constraint))
			{
				//Case a specific index wasn't specified
				if(object_idx < 0)
					//Stores on the operation the index on its parent
					obj_idx=parent_rel->getObjectIndex(tab_obj);
				else
					//Assigns the specific index to operation
					obj_idx=object_idx;
			}
			//Case there is a parent table will get the object's index
			else if(parent_tab)
			{
				if(object_idx < 0)
					//Stores on the operation the index of object on its parent
					obj_idx=parent_tab->getObjectIndex(object->getName(false), obj_type);
				else
					obj_idx=object_idx;
			}
			//Raises an error if both parent table / relationship isn't allocated
			else
				throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
		else
		{
			if((obj_type==ObjectType::Sequence && dynamic_cast<Sequence *>(object)->isReferRelationshipAddedColumn()) ||
					(obj_type==ObjectType::View && dynamic_cast<View *>(object)->isReferRelationshipAddedColumn()) ||
					(obj_type==ObjectType::GenericSql && dynamic_cast<GenericSQL *>(object)->isReferRelationshipAddedObject()))
				operation->setXMLDefinition(object->getSourceCode(SchemaParser::XmlCode));

			//Case a specific index wasn't specified
			if(object_idx < 0)
				//Stores on the operation the object index on the model
				obj_idx=model->getObjectIndex(object);
			else
				//Assigns the specific index to object
				obj_idx=object_idx;
		}

		if(obj_type==ObjectType::Column && dynamic_cast<Column *>(object)->getType().isUserType())
			operation->setXMLDefinition(object->getSourceCode(SchemaParser::XmlCode));

		operation->setObjectIndex(obj_idx);
		operations.push_back(operation);
		current_index=operations.size();

		//Registering a log entry for the object modification in database model's change log
		model->addChangelogEntry(object, op_type, parent_obj);

		//Returns the last operation position as operation's ID
		return operations.size() -1;
	}
	catch(Exception &e)
	{
		if(operation)
		{
			removeFromPool(object_pool.size()-1);
			delete operation;
		}
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

const Operation *OperationList::getOperation(unsigned oper_idx)
{
	if(oper_idx >= operations.size())
		throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return operations[oper_idx];
}

unsigned OperationList::getChainSize()
{
	int i=current_index-1;
	unsigned size=0;

	if(i < 0 && !operations.empty())
		i=0;

	//Checks if the current operations is chained
	if(!operations.empty() &&
			operations[i]->getChainType()!=Operation::NoChain)
	{
		Operation::ChainType chain_type=Operation::NoChain;
		int inc=0;

		//Case the operation is the end of a chain  runs the list in reverse order (from end to start)
		if(operations[i]->getChainType()==Operation::ChainEnd)
		{
			chain_type=Operation::ChainStart;
			inc=-1;
		}
		//Case the operation is the start of a chain  runs the list in normal order (from start to end)
		else if(operations[i]->getChainType()==Operation::ChainStart)
		{
			chain_type=Operation::ChainEnd;
			inc=1;
		}

		//Calculates the size of chain
		while(i>=0 && i < static_cast<int>(operations.size()) &&
			  size < operations.size() &&
			  operations[i]->getChainType()!=chain_type)
		{
			i+=inc;
			size++;
		}
	}

	return size;
}

void OperationList::undoOperation()
{
	if(isUndoAvailable())
	{
		Operation *operation=nullptr;
		bool chain_active=false;
		Exception error;

		do
		{
			/* In the undo operation, it is necessary to obtain the object
			at current index decremented by 1 since the current index always
			points to the element after the last element */
			operation=operations[current_index-1];

			/* If it is detected that the operation is chained with other
			and active chaining flag is cleared marks the flag to start
			the execution several operations at once */
			if(!ignore_chain && !chain_active &&
					operation->getChainType()!=Operation::NoChain)
				chain_active=true;

			/* If the chaining is active and the current operation is not part of
			chain, aborts execution of the operation */
			else if(chain_active &&
					(operation->getChainType()==Operation::ChainEnd ||
					 operation->getChainType()==Operation::NoChain))
				break;

			try
			{
				//if(chain_size > 0 && operation->isOperationValid())
				//	pos++;

				//Executes the undo operation
				executeOperation(operation, false);
			}
			catch(Exception &e)
			{
				this->removeOperations();
				error=e;
			}

			current_index--;
		}
		/* Performs the operations while the current operation is part of chain
		 or the undo option is available */
		while(!ignore_chain && isUndoAvailable() &&
			  operation->getChainType()!=Operation::NoChain);

		if(error.getErrorCode()!=ErrorCode::Custom)
			throw Exception(ErrorCode::UndoRedoOperationInvalidObject,PGM_FUNC,PGM_FILE,PGM_LINE, &error);
	}
}

void OperationList::redoOperation()
{
	if(isRedoAvailable())
	{
		Operation *operation=nullptr;
		bool chain_active=false;
		Exception error;

		do
		{
			//Gets the current operation
			operation=operations[current_index];

			/* If it is detected that the operation is chained with other
			and active chaining flag is cleared marks the flag to start
			the execution several operations at once */
			if(!ignore_chain && !chain_active &&
					operation->getChainType()!=Operation::NoChain)
				chain_active=true;

			/* If the chaining is active and the current operation is not part of
			chain or it is at the start of chain, aborts execution of the operation */
			else if(chain_active &&
					(operation->getChainType()==Operation::ChainStart ||
					 operation->getChainType()==Operation::NoChain))
				break;

			try
			{
				//Executes the redo operation (second argument as 'true')
				executeOperation(operation, true);
			}
			catch(Exception &e)
			{
				this->removeOperations();
				error=e;
			}
			current_index++;
		}
		/* Performs the operations while the current operation is part of chain
		 or the redo option is available */
		while(!ignore_chain && isRedoAvailable() &&
			  operation->getChainType()!=Operation::NoChain);

		if(error.getErrorCode()!=ErrorCode::Custom)
			throw Exception(ErrorCode::UndoRedoOperationInvalidObject,PGM_FUNC,PGM_FILE,PGM_LINE, &error);
	}
}

void OperationList::executeOperation(Operation *oper, bool redo)
{
	if(oper && oper->isOperationValid())
	{
		BaseObject *orig_obj=nullptr, *bkp_obj=nullptr,
				*object=nullptr, *aux_obj=nullptr, *parent_obj=nullptr;
		ObjectType obj_type;
		BaseTable *parent_tab=nullptr;
		Relationship *parent_rel=nullptr;
		QString xml_def;
		Operation::OperType op_type=Operation::NoOperation;
		int obj_idx=-1;

		object=oper->getPoolObject();
		obj_type=object->getObjectType();
		parent_obj=oper->getParentObject();
		xml_def=oper->getXMLDefinition();
		op_type=oper->getOperationType();
		obj_idx=oper->getObjectIndex();

		/* Converting the parent object, if any, to the correct class according
			to the type of the parent object. If ObjectType::Table|ObjectType::View, the pointer
			'parent_tab' get the reference to table/view and will be used as referential
			in the operations below. If the parent object is a relationship, the pointer
					'parent_rel' get the reference to the relationship */
		if(parent_obj)
		{
			if(parent_obj->getObjectType()==ObjectType::Relationship)
				parent_rel=dynamic_cast<Relationship *>(parent_obj);
			else
				parent_tab=dynamic_cast<BaseTable *>(parent_obj);
		}

		/* If the XML definition of object is set indicates that it is referencing a column
			included by relationship (special object) */
		if(!xml_def.isEmpty() &&
				((op_type==Operation::ObjRemoved && !redo) ||
				 (op_type==Operation::ObjCreated && redo) ||
				 (op_type==Operation::ObjModified ||
					op_type==Operation::ObjMoved)))
		{
			//Resets the XML parser and loads the buffer xml from the operation
			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(xml_def);

			if(obj_type==ObjectType::Trigger)
				aux_obj=model->createTrigger();
			else if(obj_type==ObjectType::Index)
				aux_obj=model->createIndex();
			else if(obj_type==ObjectType::Constraint)
				aux_obj=model->createConstraint(parent_obj);
			else if(obj_type==ObjectType::Sequence)
				aux_obj=model->createSequence();
			else if(obj_type==ObjectType::View)
				aux_obj=model->createView();
			else if(obj_type==ObjectType::Column)
				aux_obj=model->createColumn();
			else if(obj_type==ObjectType::GenericSql)
				aux_obj=model->createGenericSQL();
		}

		/* If the operation is a modified/moved object, the object copy
			stored in the pool will be restored */
		if(op_type==Operation::ObjModified || op_type==Operation::ObjMoved)
		{
			if(obj_type==ObjectType::Relationship)
			{
				/* Due to the complexity of the class Relationship and the strong link between all
				relationships of the model it is necessary to store XML for special objects and
				disconnect ALL relationships, perform the modification at it and the revalidating
				all relationships again. */
				model->storeSpecialObjectsXML();
				model->disconnectRelationships();
			}

			//Gets the object in the current state from the parent object
			if(parent_tab)
				orig_obj = dynamic_cast<TableObject *>(parent_tab->getObject(obj_idx, obj_type));
			else if(parent_rel)
				orig_obj = parent_rel->getObject(obj_idx, obj_type);
			else
				orig_obj = model->getObject(obj_idx, obj_type);

			if(aux_obj)
				oper->setXMLDefinition(orig_obj->getSourceCode(SchemaParser::XmlCode));

			//For pk constraint, before restore the previous configuration, uncheck the not-null flag of the source columns
			if(obj_type==ObjectType::Constraint)
				dynamic_cast<Constraint *>(orig_obj)->setColumnsNotNull(false);

			/* The original object (obtained from the table, relationship or model) will have its
			 * previous values restored with the existing copy on the pool. After restoring the object
			 * on the pool will have the same attributes as the object before being restored
			 * to enable redo operations */

			/* When executing a ObjModified operation, we nee first to clear the original
			 * object's dependencies to update them further */
			if(op_type == Operation::ObjModified)
				orig_obj->clearDependencies();

			CoreUtilsNs::copyObject((&bkp_obj), orig_obj, obj_type);
			CoreUtilsNs::copyObject((&orig_obj), object, obj_type);
			CoreUtilsNs::copyObject((&object), bkp_obj, obj_type);
			object = orig_obj;

			// Updating the original object dependencies after restoring the previous state
			if(op_type != Operation::ObjMoved)
				orig_obj->updateDependencies();

			if(aux_obj)
				CoreUtilsNs::copyObject(&object, aux_obj, obj_type);

			//For pk constraint, after restore the previous configuration, check the not-null flag of the new source columns
			if(obj_type==ObjectType::Constraint)
				dynamic_cast<Constraint *>(orig_obj)->setColumnsNotNull(true);
		}

		/* If the operation is of object removed and is not a redo, or
			if the object was previously created and wants to redo the operation
			the existing pool object will be inserted into table or in your relationship
			on its original index */
		else if((op_type==Operation::ObjRemoved && !redo) ||
						(op_type==Operation::ObjCreated && redo))
		{
			if(aux_obj)
				CoreUtilsNs::copyObject(&object, aux_obj, obj_type);

			if(parent_tab)
			{
				parent_tab->addObject(dynamic_cast<TableObject *>(object), obj_idx);

				if(object->getObjectType()==ObjectType::Constraint &&
						dynamic_cast<Constraint *>(object)->getConstraintType()==ConstraintType::ForeignKey)
					model->updateTableFKRelationships(dynamic_cast<Table *>(parent_tab));
			}
			else if(parent_rel)
				parent_rel->addObject(dynamic_cast<TableObject *>(object), obj_idx);
			else if(object->getObjectType()==ObjectType::Table)
				dynamic_cast<Table *>(object)->getSourceCode(SchemaParser::SqlCode);

			model->addObject(object, obj_idx);

			if(op_type==Operation::ObjRemoved)
				model->addPermissions(oper->getPermissions());
		}
		/* If the operation is a previously created object or if the object
			was removed and wants to redo the operation it'll be
			excluded from the table or relationship */
		else if((op_type==Operation::ObjCreated && !redo) ||
				(op_type==Operation::ObjRemoved && redo))
		{
			if(parent_tab)
				parent_tab->removeObject(object);
			else if(parent_rel)
				parent_rel->removeObject(dynamic_cast<TableObject *>(object));
			else
				model->removeObject(object, obj_idx);
		}

		/* If the parent table or parent relationship is set
			indicates that the operation was performed on a child object of these objects */
		if(parent_tab || parent_rel)
		{
			if(parent_tab)
			{
				parent_tab->setCodeInvalidated(true);
				parent_tab->setModified(true);

				if(parent_tab->getSchema())
					dynamic_cast<Schema *>(parent_tab->getSchema())->setModified(true);

				if(object->getObjectType()==ObjectType::Column ||
						object->getObjectType()==ObjectType::Constraint)
				{
					model->validateRelationships(dynamic_cast<TableObject *>(object), dynamic_cast<Table *>(parent_tab));

					if(object->getObjectType()==ObjectType::Constraint &&
						 dynamic_cast<Constraint *>(object)->getConstraintType()==ConstraintType::ForeignKey)
						model->updateTableFKRelationships(dynamic_cast<Table *>(parent_tab));
					else if(object->getObjectType() == ObjectType::Column)
					{
						Table *tab = dynamic_cast<Table *>(parent_tab);
						model->updateViewsReferencingTable(dynamic_cast<PhysicalTable *>(parent_tab));

						// Forcing the update of FK relationships that are based on the fks in which the column is in
						if(tab)
						{
							Constraint *constr = nullptr;
							Column *col = dynamic_cast<Column *>(object);
							BaseRelationship *rel = nullptr;

							for(auto &tab_obj : *tab->getObjectList(ObjectType::Constraint))
							{
								constr = dynamic_cast<Constraint *>(tab_obj);
								if(constr->getConstraintType() == ConstraintType::ForeignKey &&
									 constr->isColumnExists(col, Constraint::SourceCols))
								{
									rel = model->getRelationship(tab, constr->getReferencedTable(), constr);

									if(rel)
									{
										rel->setMandatoryTable(BaseRelationship::DstTable, col->isNotNull());
										rel->setModified(true);
									}
								}
							}
						}
					}
				}
			}
			else if(parent_rel)
			{
				parent_rel->setCodeInvalidated(true);
				parent_rel->forceInvalidate();
				model->validateRelationships();
			}
		}

		/* If the object in question is graphical it has the modified flag
			marked to force the redraw at the time of its restoration */
		else if(BaseGraphicObject::isGraphicObject(obj_type))
		{
			BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

			if(op_type==Operation::ObjModified ||
					op_type==Operation::ObjMoved)
				graph_obj->setModified(true);

			//Case the object is a view is necessary to update the table-view relationships on the model
			if(obj_type==ObjectType::View && op_type==Operation::ObjModified)
				model->updateViewRelationships(dynamic_cast<View *>(graph_obj));
			else if((obj_type==ObjectType::Relationship ||
							(PhysicalTable::isPhysicalTable(obj_type) && model->getRelationship(dynamic_cast<BaseTable *>(object), nullptr))) &&
							op_type==Operation::ObjModified)
				model->validateRelationships();

			//If a object had its schema restored is necessary to update the envolved schemas
			if(BaseTable::isBaseTable(obj_type) &&
				 ((bkp_obj && graph_obj->getSchema()!=bkp_obj->getSchema() && op_type==Operation::ObjModified) ||
					op_type==Operation::ObjMoved))
			{
				dynamic_cast<BaseGraphicObject *>(graph_obj->getSchema())->setModified(true);

				if(bkp_obj)
					dynamic_cast<BaseGraphicObject *>(bkp_obj->getSchema())->setModified(op_type==Operation::ObjModified);
			}
		}
		else if(op_type==Operation::ObjModified)
		{			
			if(obj_type==ObjectType::Schema)
			{
				model->validateSchemaRenaming(dynamic_cast<Schema *>(object), bkp_obj->getName());
				dynamic_cast<Schema *>(object)->setModified(true);
			}
			else if(obj_type==ObjectType::Tag)
			{
				for(auto &obj : object->getReferences())
					dynamic_cast<BaseTable *>(obj)->setModified(true);
			}
		}

		//Case the object is a type update the tables that are referencing it
		if(op_type==Operation::ObjModified &&
				(object->getObjectType()==ObjectType::Type || object->getObjectType()==ObjectType::Domain ||
				 object->getObjectType()==ObjectType::Table || object->getObjectType()==ObjectType::ForeignTable ||
				 object->getObjectType()==ObjectType::View || object->getObjectType()==ObjectType::Extension))
		{
			for(auto &obj : object->getReferences())
			{
				if(obj->getObjectType() == ObjectType::Column)
					dynamic_cast<Column *>(obj)->getParentTable()->setModified(true);
			}
		}
	}
}

void OperationList::removeLastOperation()
{
	if(!operations.empty())
	{
		Operation *oper=nullptr;
		bool end=false;
		std::vector<Operation *>::reverse_iterator itr;
		int oper_idx=operations.size()-1;

		//Gets the last operation on the list using reverse iterator
		itr=operations.rbegin();

		while(!end)
		{
			oper=(*itr);

			//Removes the object related to the operation from the pool
			removeFromPool(oper_idx);

			/* Stop condition for removing the operation:
			 1) The operation is not chained with others, or
			 2) If the last operation at the end of a chain
					all chained operations are removed including
					the first operation of the chain, when the latter
					is removed the iteration is stopped.*/
			end=(ignore_chain ||
				 (!ignore_chain &&
				  (oper->getChainType()==Operation::NoChain ||
				   oper->getChainType()==Operation::ChainStart)));

			itr++; oper_idx--;
		}

		/* If the head of chaining is removed (CHAIN_START)
		 marks that the next element in the list is the new
		 start of chain */
		if(oper && oper->getChainType()==Operation::ChainStart)
			next_op_chain=Operation::ChainStart;

		//Erasing the excluded operations
		for(int i=operations.size()-1; i > oper_idx ; i--)
			operations.erase(operations.begin() + i);

		//Validates the remaining operations
		validateOperations();

		/* Points the current index to the end of list if it is beyond
		 the last element */
		if(static_cast<unsigned>(current_index) > operations.size())
			current_index=operations.size();
	}
}

void OperationList::updateObjectIndex(BaseObject *object, unsigned new_idx)
{
	std::vector<Operation *>::iterator itr, itr_end;
	Operation *oper=nullptr;

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	itr=operations.begin();
	itr_end=operations.end();

	while(itr!=itr_end)
	{
		oper=(*itr);
		if(oper->getOriginalObject()==object)
			oper->setObjectIndex(new_idx);
		itr++;
	}
}

