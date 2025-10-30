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
\class Extension
\brief Implements the operations to manipulate extensions on the database.
**/

#ifndef EXTENSION_H
#define EXTENSION_H

#include "baseobject.h"

class __libcore Extension: public BaseObject {
	public:
		struct ExtObject {
			protected:
				QString name, parent;
				ObjectType obj_type;
				QString signature;

			public:
				ExtObject(const QString &_name, ObjectType _obj_type, const QString &_parent = "") :
					name(_name), parent(_parent),
					obj_type(_obj_type),
					signature((BaseObject::isChildObjectType(ObjectType::Schema, _obj_type) && _parent.isEmpty() ? "" :
											(!_parent.isEmpty() ?	BaseObject::formatName(_parent) + "." + BaseObject::formatName(_name) :
																						BaseObject::formatName(_name)))){}



				bool operator == (const ExtObject &obj) const
				{
					return obj.name == name &&
								 obj.obj_type == obj_type &&
								 obj.parent == parent;
				}

				bool isValid() const
				{
					return BaseObject::isValidName(name) &&
								 (parent.isEmpty() || BaseObject::isValidName(parent)) &&
								 (obj_type == ObjectType::Schema || obj_type == ObjectType::Type);
				}

				const QString &getName() const
				{
					return name;
				}

				const QString &getSignature() const
				{
					return signature;
				}

				const QString &getParent() const
				{
					return parent;
				}

				ObjectType getType() const
				{
					return obj_type;
				}
		};

	private:
		/*! \brief Versions of the installed extension. The first one is the
		current version and the last is the old version from which the extension
		is being updated */
		QString versions[2];

		/*! \brief Store the child object names distinct by the object type.
		 *  Currently only user-defined types and schemas are supported */
		std::map<ObjectType, std::vector<ExtObject>> ext_objects;

	public:
		enum VersionId: unsigned {
			CurVersion,
			OldVersion
		};

		Extension();

		void setSchema(BaseObject *schema) override;

		void addObject(const ExtObject &ext_obj);

		void removeObjects(ObjectType obj_type = ObjectType::BaseObject);

		const std::vector<ExtObject> &getObjects(ObjectType obj_type);

		//! \brief Set the versions of the extension
		void setVersion(VersionId ver, const QString &value);

		//! \brief Returns on of the versions of the extension
		QString getVersion(VersionId ver);

		bool containsObject(const ExtObject &ext_obj);

		//! \brief Returns the SQL / XML code definition for the extension
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getAlterCode(BaseObject *object) final;

		QString getDropCode(bool cascade) final;

		QString getSignature(bool format = true) final;

		/*! \brief Returns the extension name. This method was overriden from BaseObject::getName() to make the prepend_schema
		 * to be false by default (instead of true in BaseObject::getName) so the SQL code definition of the
		 * extension can be created correctly since the CREATE EXTENSION does not allow the schema name appended to the object's name */
		QString getName(bool format = false, bool prepend_schema = false) final;
};

#endif
