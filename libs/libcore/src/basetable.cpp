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

#include "basetable.h"
#include "utilsns.h"
#include "coreutilsns.h"
#include "tableobject.h"

BaseTable::BaseTable()
{
	tag=nullptr;
	obj_type=ObjectType::BaseTable;
	attributes[Attributes::Tag]="";
	attributes[Attributes::MaxObjCount]="";
	attributes[Attributes::CollapseMode]="";
	attributes[Attributes::Pagination]="";
	attributes[Attributes::AttribsPage]="";
	attributes[Attributes::ExtAttribsPage]="";
	attributes[Attributes::ZValue]="";
	pagination_enabled = false;
	collapse_mode = CollapseMode::NotCollapsed;
	resetCurrentPages();
}

void BaseTable::setCodeInvalidated(bool value)
{
	BaseGraphicObject::setCodeInvalidated(value);
	resetHashCode();
}

void BaseTable::resetCurrentPages()
{
	curr_page[AttribsSection] = 0;
	curr_page[ExtAttribsSection] = 0;
}

void BaseTable::setTag(Tag *tag)
{
	setCodeInvalidated(this->tag != tag);
	this->tag=tag;
}

Tag *BaseTable::getTag()
{
	return tag;
}

bool BaseTable::isBaseTable(ObjectType obj_tp)
{
	return (obj_tp == ObjectType::Table ||
				 obj_tp == ObjectType::ForeignTable ||
				 obj_tp == ObjectType::View);
}

QString BaseTable::getAlterCode(BaseObject *object)
{
	try
	{
		return BaseObject::getAlterCode(object);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void BaseTable::operator = (BaseTable &tab)
{
	(*dynamic_cast<BaseGraphicObject *>(this))=dynamic_cast<BaseGraphicObject &>(tab);
	this->tag=tab.tag;
}

BaseTable::CollapseMode BaseTable::getCollapseMode()
{
	return collapse_mode;
}

void BaseTable::setPaginationEnabled(bool value)
{
	setCodeInvalidated(pagination_enabled != value);
	pagination_enabled = value;

	if(!pagination_enabled)
		resetCurrentPages();
}

bool BaseTable::isPaginationEnabled()
{
	return pagination_enabled;
}

void BaseTable::setCurrentPage(TableSection section_id, unsigned value)
{
	if(section_id > ExtAttribsSection)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	setCodeInvalidated(curr_page[section_id] != value);
	curr_page[section_id] = value;
}

unsigned BaseTable::getCurrentPage(TableSection section_id)
{
	if(section_id > ExtAttribsSection)
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return curr_page[section_id];
}

void BaseTable::generateHashCode()
{
	if(!isModified())
		return;

	QString buf;
	buf.append(obj_name);

	for(auto &obj : getObjects())
		buf.append(dynamic_cast<TableObject *>(obj)->getHashCode());

	buf.append(tag ? tag->getName() : "");
	buf.append(schema ? schema->getName() : "");
	buf.append(QString::number(sql_disabled));
	buf.append(QString::number(collapse_mode));
	buf.append(QString::number(static_cast<int>(pagination_enabled)));
	buf.append(QString::number(curr_page[0]));
	buf.append(QString::number(curr_page[1]));
	buf.append(QString::number(getPosition().x()));
	buf.append(QString::number(getPosition().y()));

	hash_code = UtilsNs::getStringHash(buf);
}

void BaseTable::resetHashCode()
{
	hash_code = "";
}

void BaseTable::setCollapseMode(CollapseMode coll_mode)
{
	setCodeInvalidated(collapse_mode != coll_mode);
	collapse_mode = coll_mode;
}

void BaseTable::setZValue(int z_value)
{
	setCodeInvalidated(this->z_value != z_value);
	BaseGraphicObject::setZValue(z_value);
}

QString BaseTable::getHashCode()
{
	return hash_code;
}

void BaseTable::setModified(bool value)
{
	BaseGraphicObject::setModified(value);
	generateHashCode();
}

void BaseTable::setPosition(const QPointF &pos)
{
	BaseGraphicObject::setPosition(pos);
	resetHashCode();
}

std::vector<BaseObject *> BaseTable::getDependencies(bool inc_indirect_deps, const std::vector<ObjectType> &excl_types, bool rem_duplicates)
{
	if(inc_indirect_deps)
	{
		std::vector<BaseObject *> ind_deps = BaseObject::getDependencies(true);

		for(auto &obj : getObjects())
			BaseObject::__getLinkedObjects(ObjDependencies, obj->getDependencies(), ind_deps);

		if(!excl_types.empty())
			ind_deps = CoreUtilsNs::filterObjectsByType(ind_deps, excl_types);

		if(rem_duplicates)
		{
			std::sort(ind_deps.begin(), ind_deps.end());
			auto end = std::unique(ind_deps.begin(), ind_deps.end());
			ind_deps.erase(end, ind_deps.end());
		}

		return ind_deps;
	}

	return BaseObject::getDependencies(false, excl_types, rem_duplicates);
}

void BaseTable::updateDependencies(const std::vector<BaseObject *> &deps, const std::vector<BaseObject *> &old_deps)
{
	std::vector<BaseObject *> aux_deps = { tag	};
	aux_deps.insert(aux_deps.end(), deps.begin(), deps.end());
	BaseObject::updateDependencies(aux_deps, old_deps);
}
