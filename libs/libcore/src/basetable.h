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
\class BaseTable
\brief Base class used to define table like objects (views and tables)
\note <strong>Creation date:</strong> 09/04/2008
*/

#ifndef BASE_TABLE_H
#define BASE_TABLE_H

#include "basegraphicobject.h"
#include "tag.h"

class __libcore BaseTable: public BaseGraphicObject {
	public:
		//! \brief This enum is used to control the collapsing of the tables
		enum CollapseMode: unsigned {
			AllAttribsCollapsed, //Columns (attributes) and extended attributes are collapsed
			ExtAttribsCollapsed, //Extended attributes are collapsed
			NotCollapsed //Table is fully expanded (columns and extended attributes)
		};

	protected:
		Tag *tag;

		//! \brief Indicates if the pagination is enabled for the table
		bool pagination_enabled;

		//! \brief Stores the current collpase mode for the table
		CollapseMode collapse_mode;

		//! \brief Stores the current page visible on the table
		unsigned curr_page[2];

		QString hash_code;

	public:
		enum TableSection: unsigned {
			AttribsSection,
			ExtAttribsSection
		};

		BaseTable();
		~BaseTable() override = default;

		QString getHashCode();

		virtual void setTag(Tag *tag);

		virtual Tag *getTag();

		//! \brief Returns true if the provided table is considered a base table (Table, ForeignTable, View)
		static bool isBaseTable(ObjectType obj_tp);

		//! \brief Adds an object to the table. It can be inserted at a specified index 'obj_idx'.
		virtual void addObject(BaseObject *obj, int obj_idx=-1)=0;

		//! \brief Gets a object from table through its index and type
		virtual BaseObject *getObject(unsigned obj_idx, ObjectType obj_type)=0;

		//! \brief Gets a object from table through its name and type
		virtual BaseObject *getObject(const QString &name, ObjectType obj_type)=0;

		//! \brief Removes a object from table through its index and type
		virtual void removeObject(unsigned obj_idx, ObjectType obj_type)=0;

		//! \brief Removes a object from table through its name and type
		virtual void removeObject(const QString &name, ObjectType obj_type)=0;

		//! \brief Removes the specified object from table
		virtual void removeObject(BaseObject *obj)=0;

		/*! \brief Gets the the count for the specified object type. The boolean parameter indicates
		 that objects added by relationship must be counted */
		virtual unsigned getObjectCount(ObjectType obj_type, bool inc_added_by_rel=true)=0;

		//! \brief Gets the object index using its name and type
		virtual int getObjectIndex(const QString &name, ObjectType obj_type)=0;

		//! \brief Returns the index for the specified table object
		virtual int getObjectIndex(BaseObject *obj)=0;

		//! \brief Returns all children objects of the table but excluding the ones of the provided type
		virtual std::vector<BaseObject *> getObjects(const std::vector<ObjectType> &excl_types = {})=0;

		QString getSourceCode(SchemaParser::CodeType tipo_def) override = 0;

		QString getAlterCode(BaseObject *object) override;

		/*! \brief Set the initial capacity of the objects list for a optimized memory usage.
		 * This method should be called prior to adding the first object to the table because, depending o the capacity,
		 * there'll be memory reallocations which can degradate the performance */
		virtual void setObjectListsCapacity(unsigned capacity) = 0;

		//! \brief Returns the maximum item count from all of the objects lists
		virtual unsigned getMaxObjectCount(void) = 0;

		//! \brief Copy the attributes between two tables
		void operator = (BaseTable &tab);

		/*! \brief Defines the current collapse mode for the table. Calling this method direclty
		 * will not update the geometry of the graphical representation of this object. For that,
		 * the setModified(true) should be called */
		void setCollapseMode(CollapseMode coll_mode);

		CollapseMode getCollapseMode();

		/*! \brief Defines the pagination enabling for the table. Calling this method direclty
		 * will not update the geometry of the graphical representation of this object. For that,
		 * the setModified(true) should be called */
		void setPaginationEnabled(bool value);

		bool isPaginationEnabled();

		void setZValue(int z_value) override;

		/*! \brief Defines the current page visible on the table. Calling this method direclty
		 * will not update the geometry of the graphical representation of this object. For that,
		 * the setModified(true) should be called */
		void setCurrentPage(TableSection section_id, unsigned value);
		void resetCurrentPages();
		unsigned getCurrentPage(TableSection section_id);

		/*! \brief Returns the data dictionary definition of the table (in HTML format).
		 * The split parameter is used to inform the generation process that the dicts are being
		 * saved in separated files. This changes the way links are generated inside the data dictionaries */
		virtual QString getDataDictionary(bool split, bool md_format, const attribs_map &extra_attribs = {}) = 0;

		void setCodeInvalidated(bool value) override;

		void setModified(bool value) override;

		virtual void generateHashCode();

		void resetHashCode();

		void setPosition(const QPointF &pos) override;

		void updateDependencies() override = 0;

		std::vector<BaseObject *> getDependencies(bool inc_indirect_deps = false,
																							const std::vector<ObjectType> &excl_types = {},
																							bool rem_duplicates = false) override;

		void updateDependencies(const std::vector<BaseObject *> &deps, const std::vector<BaseObject *> &old_deps = {});

		friend class DatabaseModel;
};

#endif
