/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class Operation
\brief Implements the class that stores operations executed over an object
*/

#ifndef OPERATION_H
#define OPERATION_H

#include "baseobject.h"
#include "permission.h"
#include <QString>

class __libcore Operation {
	public:
		//! \brief Constants used to reference the type of operations
		enum OperType: unsigned {
			NoOperation,
			ObjModified,
			ObjCreated,
			ObjRemoved,

			/*! \brief This type of operation has the same effect of operation OBJECT_MODIFIED
								except that it not (re)validate relationships as happens with operations.
								This type of operation (OBJECT_MOVED) is useful to undo position changes of
								graphical objects without executing unnecessary revalidations of relationships */
			ObjMoved
		};

		//! \brief Operation chain types
		enum ChainType: unsigned {
			NoChain, //! \brief The operation is not part of a chain
			ChainStart, //! \brief The operation is the head of the chain
			ChainMiddle, //! \brief The operation is in the middle of the chain
			ChainEnd //! \brief The operation is the last on the chain
		};

	private:
		/*! \brief Uniquely identifies the object. This id is used to check if the operation object's somehow
		where delete (changing their addresses). This will avoid the operation list to try to execute
		an invalid operation */
		QString operation_id;

		/*! \brief Reference to the parent object of the original object that has suffered the operation,
		This attribute is used only in the case of table objects such as columns, indexes,
		constraints, rules. For other object types there is no need to use this attribute
		because the parent object for other objects is always the database model */
		BaseObject *parent_obj;

		//! \brief Reference (on the pool) to the copy of the original object
		BaseObject *pool_obj;

		//! \brief Reference to the original object that generates the operation.
		BaseObject *original_obj;

		/*! \brief Stores the XML definition of the special objects this means the objects
		 that reference columns added by relationship. This is the case of triggers,
		 indexes, sequences, constraints. */
		QString xml_definition;

		//! \brief Operation type (Constants OBJECT_[MODIFIED | CREATED | REMOVED | MOVED]
		OperType op_type;

		//! \brief Operation chain type. This attribute is used to redo/undo several operations at once
		ChainType chain_type;

		//! \brief Object index inside the list on its parent object
		int object_idx;

		//! \brief Stores the object's permission before it's removal
		std::vector<Permission *> permissions;

		//! \brief Generate an unique id for the operation based upon the memory addresses of objects held by it
		QString generateOperationId() const;

		void setObjectIndex(int idx);
		void setChainType(ChainType type);
		void setOperationType(OperType type);
		void setOriginalObject(BaseObject *object);
		void setPoolObject(BaseObject *object);
		void setParentObject(BaseObject *object);
		void setPermissions(const std::vector<Permission *> &perms);
		void setXMLDefinition(const QString &xml_def);

		int getObjectIndex();
		ChainType getChainType();
		OperType getOperationType();
		BaseObject *getOriginalObject();
		BaseObject *getPoolObject();
		BaseObject *getParentObject();
		std::vector<Permission *> getPermissions();
		QString getXMLDefinition();

		bool isOperationValid() const;

	public:
		struct OperationInfo {
			QString obj_name;
			ObjectType obj_type;
			OperType oper_type;
			int index;

			OperationInfo()
			{
				obj_type = ObjectType::BaseObject;
				oper_type = OperType::NoOperation;
			}

			OperationInfo(const QString &_obj_name, ObjectType _obj_type, OperType _oper_type)
			{
				obj_name = _obj_name;
				obj_type = _obj_type;
				oper_type = _oper_type;
			}
		};

		Operation();
		OperationInfo getOperationInfo() const;

		friend class OperationList;
};

#endif
