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

#include "objectsdiffinfo.h"

ObjectsDiffInfo::ObjectsDiffInfo()
{
	object=old_object=nullptr;
	diff_type=NoDifference;
}

ObjectsDiffInfo::ObjectsDiffInfo(DiffType diff_type, BaseObject *object, BaseObject *old_obj)
{
	this->diff_type=diff_type;
	this->object=object;
	this->old_object=old_obj;
}

ObjectsDiffInfo::DiffType ObjectsDiffInfo::getDiffType()
{
	return diff_type;
}

QString ObjectsDiffInfo::getInfoMessage()
{
	QString msg=QString("%1 `%2' (%3)"), obj_name;
	BaseObject *ref_obj=nullptr;
	ObjectType obj_type=ObjectType::BaseObject;

	if(diff_type==AlterObject && old_object)
		ref_obj=old_object;
	else
		ref_obj=object;

	obj_type=ref_obj->getObjectType();

	/* Forcing the usage of BaseObject::getSignature for the following object,
	 since the custom getSignature for those types return some undesired
	 SQL keywords for this context */
	if(obj_type==ObjectType::Constraint || obj_type==ObjectType::Trigger || obj_type==ObjectType::Rule)
		obj_name=dynamic_cast<TableObject *>(ref_obj)->TableObject::getSignature();
	else if(obj_type==ObjectType::OpClass || obj_type==ObjectType::OpFamily)
		obj_name=ref_obj->BaseObject::getSignature();
	else
		obj_name=ref_obj->getSignature();

	if(diff_type == NoDifference)
		return "";

	if(diff_type==DropObject)
	{
		msg=msg.arg("<font color=\"#e00000\"><strong>DROP</strong></font>")
				.arg(obj_name)
				.arg(ref_obj->getTypeName());
	}
	else if(diff_type==CreateObject)
	{
		msg=msg.arg("<font color=\"#008000\"><strong>CREATE</strong></font>")
				.arg(obj_name)
				.arg(ref_obj->getTypeName());
	}
	else if(diff_type==AlterObject)
	{
		msg=msg.arg("<font color=\"#ff8000\"><strong>ALTER</strong></font>")
				.arg(obj_name)
				.arg(ref_obj->getTypeName());
	}
	else if(diff_type==IgnoreObject)
	{
		msg=msg.arg("<font color=\"#606060\"><strong>IGNORE</strong></font>")
				.arg(obj_name)
				.arg(ref_obj->getTypeName());

	}

	return msg;
}

QString ObjectsDiffInfo::getDiffTypeString()
{
	static const std::map<DiffType, QString> type_map {
		{ NoDifference, ""}, { DropObject, "DROP" },
		{ CreateObject, "CREATE"}, { AlterObject, "ALTER" },
		{ IgnoreObject, "IGNORE" }
	};

	return type_map.at(diff_type);
}

BaseObject *ObjectsDiffInfo::getObject()
{
	return object;
}

BaseObject *ObjectsDiffInfo::getOldObject()
{
	return old_object;
}

bool ObjectsDiffInfo::operator == (ObjectsDiffInfo &info)
{
	return (this->diff_type==info.diff_type &&
					this->object==info.object &&
					this->old_object==info.old_object);
}
