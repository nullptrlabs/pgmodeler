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

#include "objecttypeslistwidget.h"
#include "guiutilsns.h"

ObjectTypesListWidget::ObjectTypesListWidget(QWidget *parent, const std::vector<ObjectType> &excl_types)	: QWidget(parent)
{
	setupUi(this);

	QListWidgetItem *item = nullptr;
	std::vector<ObjectType> excluded_types { ObjectType::BaseRelationship };
	excluded_types.insert(excluded_types.end(), excl_types.begin(), excl_types.end());

	for(auto &type : BaseObject::getObjectTypes(true, excluded_types))
	{
		item = new QListWidgetItem(BaseObject::getTypeName(type));
		item->setIcon(QIcon(GuiUtilsNs::getIconPath(type)));
		item->setData(Qt::UserRole, QVariant::fromValue<ObjectType>(type));
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		obj_types_lst->addItem(item);
	}

	obj_types_lst->adjustSize();

	connect(check_all_btn, &QPushButton::clicked, this, [this](){
		setTypesCheckState(Qt::Checked);
	});

	connect(uncheck_all_btn, &QPushButton::clicked, this, [this](){
		setTypesCheckState(Qt::Unchecked);
	});

	connect(obj_types_lst, &QListWidget::itemChanged, this, [this](QListWidgetItem *item){
		emit s_typeCheckStateChanged(item->data(Qt::UserRole).value<ObjectType>(), item->checkState());
	});
}

void ObjectTypesListWidget::setTypeNamesCheckState(const QStringList &obj_types, Qt::CheckState state)
{
	std::vector<ObjectType> types;

	for(auto &obj_type : obj_types)
		types.push_back(BaseObject::getObjectType(obj_type));

	setTypesCheckState(types, state);
}

void ObjectTypesListWidget::setTypesCheckState(const std::vector<ObjectType> &obj_types, Qt::CheckState state)
{
	ObjectType obj_type;

	for(auto &item : obj_types_lst->findItems("*", Qt::MatchWildcard))
	{
		obj_type = item->data(Qt::UserRole).value<ObjectType>();

		if(std::find(obj_types.cbegin(), obj_types.cend(), obj_type) != obj_types.cend())
			item->setCheckState(state);
	}

	emit s_typesCheckStateChanged(state);
}

void ObjectTypesListWidget::setTypesCheckState(Qt::CheckState state)
{
	for(auto &item : obj_types_lst->findItems("*", Qt::MatchWildcard))
		item->setCheckState(state);

	emit s_typesCheckStateChanged(state);
}

std::vector<ObjectType> ObjectTypesListWidget::getTypesPerCheckState(Qt::CheckState state)
{
	std::vector<ObjectType> types;

	for(auto &item : obj_types_lst->findItems("*", Qt::MatchWildcard))
	{
		if(item->checkState() == state)
			types.push_back(item->data(Qt::UserRole).value<ObjectType>());
	}

	return types;
}

QStringList ObjectTypesListWidget::getTypeNamesPerCheckState(Qt::CheckState state)
{
	QStringList types;

	for(auto &item : obj_types_lst->findItems("*", Qt::MatchWildcard))
	{
		if(item->checkState() == state)
			types.append(BaseObject::getSchemaName(item->data(Qt::UserRole).value<ObjectType>()));
	}

	return types;
}

int ObjectTypesListWidget::getTypesCountPerCheckState(Qt::CheckState state)
{
	int count = 0;

	for(auto &item : obj_types_lst->findItems("*", Qt::MatchWildcard))
	{
		if(item->checkState() == state)
			count++;
	}

	return count;
}
