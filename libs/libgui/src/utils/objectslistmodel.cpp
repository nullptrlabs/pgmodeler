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

#include "objectslistmodel.h"
#include "guiutilsns.h"
#include <pgsqltypes/pgsqltype.h>
#include "customtablewidget.h"
#include "tableobject.h"
#include "permission.h"
#include "baserelationship.h"
#include "utilsns.h"

const QStringList ObjectsListModel::HeaderTexts {
	QT_TR_NOOP("Object"), QT_TR_NOOP("Type"), QT_TR_NOOP("ID"),
	QT_TR_NOOP("Parent"), QT_TR_NOOP("Parent type")
};

const QStringList ObjectsListModel::HeaderIcons {
	"objects", "usertype", "typeoid",
	"schema", "usertype", "attribute"
};

ObjectsListModel::ObjectsListModel(const std::vector<BaseObject *> &obj_list, const QString &search_attr, QObject *parent) : QAbstractTableModel(parent)
{
	col_count = search_attr.isEmpty() ? 5 : 6;
	row_count = obj_list.size();
	insertColumns(0, col_count);
	insertRows(0, row_count);	
	fillModel(obj_list, search_attr);
}

ObjectsListModel::ObjectsListModel(const std::vector<attribs_map> &attr_list, QObject* parent) : QAbstractTableModel(parent)
{
	col_count = 6;
	row_count = attr_list.size();
	insertColumns(0, col_count);
	insertRows(0, row_count);
	fillModel(attr_list);
}

int ObjectsListModel::rowCount(const QModelIndex &) const
{
	return row_count;
}

int ObjectsListModel::columnCount(const QModelIndex &) const
{
	return col_count;
}

QModelIndex ObjectsListModel::index(int row, int column, const QModelIndex &parent) const
{
	return QAbstractTableModel::index(row, column, parent);
}

QModelIndex ObjectsListModel::parent(const QModelIndex &) const
{
	return {};
}

std::tuple<int,int,int> ObjectsListModel::getIndexMargins()
{
	static int horiz_margin_no_icon = (qApp->style()->pixelMetric(QStyle::PM_HeaderMargin) * 2) +
																		qApp->style()->pixelMetric(QStyle::PM_HeaderGripMargin) +
																		qApp->style()->pixelMetric(QStyle::PM_HeaderMarkSize),

			horiz_margin = horiz_margin_no_icon + qApp->style()->pixelMetric(QStyle::PM_SmallIconSize),

			vert_margin = qApp->style()->pixelMetric(QStyle::PM_HeaderMargin) * 2;

	return { horiz_margin, horiz_margin_no_icon, vert_margin };
}

void ObjectsListModel::configureHeader(const QString &search_attr)
{
	QFontMetrics fm(qApp->font());
	ItemData header_dt;
	QStringList header_texts = HeaderTexts;
	auto [ h_margin, h_margin_no_ico, v_margin ] = getIndexMargins();

	// Configuring the header items
	header_texts.append(
			BaseObject::getSearchAttributeI18N(
					(search_attr != Attributes::Name &&
					 search_attr != Attributes::Schema &&
					 search_attr != Attributes::Comment) ? search_attr : Attributes::Comment));

	for(int col = 0; col < col_count; col++)
	{
		header_dt.text = header_texts[col];
		header_dt.icon = GuiUtilsNs::getIconPath(HeaderIcons[col]);
		header_dt.sz_hint = fm.boundingRect(header_dt.text).size() + QSize(h_margin, v_margin);
		header_data.append(header_dt);
		header_dt.clear();
	}
}

QVariant ObjectsListModel::getItemData(const ItemData &item_dt, int role) const
{
	if(role == Qt::DisplayRole)
	{
		if(item_dt.id > 0)
			return item_dt.id;

		return item_dt.text;
	}

	if(role == Qt::ForegroundRole && !item_dt.fg_color.isEmpty())
		return QColor(item_dt.fg_color);

	if(role == Qt::BackgroundRole && !item_dt.bg_color.isEmpty())
		return QColor(item_dt.bg_color);

	if(role == Qt::UserRole)
	{
		if(item_dt.object)
			return QVariant::fromValue<void *>(reinterpret_cast<void *>(item_dt.object));

		if(item_dt.obj_type != ObjectType::BaseObject)
			return QVariant::fromValue<ObjectType>(item_dt.obj_type);

		return item_dt.text;
	}

	if(role == Qt::FontRole)
	{
		QFont font;
		font.setItalic(item_dt.italic);
		font.setStrikeOut(item_dt.strikeout);
		return font;
	}

	if(role == Qt::DecorationRole)
		return QIcon(item_dt.icon);

	if(role == Qt::SizeHintRole)
		return item_dt.sz_hint;

	if(role == Qt::TextAlignmentRole)
		return { Qt::AlignLeft | Qt::AlignVCenter };

	return {};
}

void ObjectsListModel::fillModel(const std::vector<BaseObject*>& obj_list, const QString& search_attr)
{
	QFontMetrics fm(qApp->font());
	int	lf_cnt = 0;
	QSize sz;
	ItemData item_dt;
	BaseObject *parent_obj = nullptr;
	attribs_map search_attribs;
	ObjectType obj_type;
	int sub_type = -1;
	auto [ h_margin, h_margin_no_ico, v_margin ] = getIndexMargins();

	configureHeader(search_attr);

	for(auto &obj : obj_list)
	{
		sub_type = -1;
		obj_type = obj->getObjectType();

		if(obj_type == ObjectType::BaseRelationship ||
				obj_type == ObjectType::Relationship)
			sub_type = dynamic_cast<BaseRelationship *>(obj)->getRelationshipType();
		else if(obj_type == ObjectType::Constraint)
			sub_type = dynamic_cast<Constraint *>(obj)->getConstraintType().getTypeId();

		//First column: Object name
		item_dt.clear();
		item_dt.text = obj->getName();
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_dt.icon = GuiUtilsNs::getIconPath(obj_type, sub_type);
		item_dt.object = obj;

		if(obj->isProtected() || obj->isSystemObject())
		{
			item_dt.italic = true;
			item_dt.fg_color = CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemAltFgColor).name();
		}
		else if(dynamic_cast<TableObject *>(obj) &&
						 dynamic_cast<TableObject *>(obj)->isAddedByRelationship())
		{
			item_dt.italic = true;
			item_dt.fg_color = CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemAltFgColor).name();
		}

		item_dt.strikeout = obj->isSQLDisabled() && !obj->isSystemObject();
		item_data.append(item_dt);

		//Second column: Object type
		item_dt.clear();
		item_dt.text = obj->getTypeName();
		item_dt.obj_type = obj ? obj->getObjectType() : ObjectType::BaseObject;
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin_no_ico, v_margin);
		item_dt.italic = true;
		item_data.append(item_dt);

		//Third column: Object id
		item_dt.clear();
		item_dt.id = obj->getObjectId();
		item_dt.text = QString::number(obj->getObjectId());
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin_no_ico, v_margin);
		item_data.append(item_dt);

		//Fourth column: Parent object name
		item_dt.clear();

		if(dynamic_cast<TableObject *>(obj))
			parent_obj = dynamic_cast<TableObject *>(obj)->getParentTable();
		else if(obj->getSchema())
			parent_obj = obj->getSchema();
		else if(dynamic_cast<Permission *>(obj))
			parent_obj = dynamic_cast<Permission *>(obj)->getObject();
		else
			parent_obj = obj->getDatabase();

		item_dt.text = parent_obj ? parent_obj->getName() : "-";
		item_dt.obj_type = parent_obj ? parent_obj->getObjectType() : ObjectType::BaseObject;
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_dt.object = parent_obj;

		if(parent_obj)
		{
			item_dt.icon = GuiUtilsNs::getIconPath(parent_obj->getObjectType());

			if(parent_obj->isProtected() || parent_obj->isSystemObject())
			{
				item_dt.italic = true;
				item_dt.fg_color = CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemAltFgColor).name();
			}
		}

		item_data.append(item_dt);

		//Fifth column: Parent object type
		item_dt.clear();
		item_dt.text = parent_obj ? parent_obj->getTypeName() : "-";
		item_dt.obj_type = parent_obj ? parent_obj->getObjectType() : ObjectType::BaseObject;
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_dt.italic = true;
		item_data.append(item_dt);

		//Sixth column: object comment or the specified search attribute
		if(!search_attr.isEmpty())
		{
			item_dt.clear();
			search_attribs = obj->getSearchAttributes();

			if(search_attr != Attributes::Name &&
					search_attr != Attributes::Schema &&
					search_attr != Attributes::Comment)
			{
				item_dt.text = search_attribs[search_attr];

				if(search_attr == Attributes::SrcColumns ||
						search_attr == Attributes::RefColumns)
				{
					item_dt.text.replace(UtilsNs::DataSeparator, ", ");
				}
			}
			else
				item_dt.text = obj->getComment();

			sz = fm.boundingRect(item_dt.text).size();
			lf_cnt = item_dt.text.count(QChar(QChar::LineFeed)) + 1;
			sz.setHeight((sz.height() * lf_cnt) + v_margin);
			item_dt.sz_hint = sz;
			item_data.append(item_dt);
		}
	}
}

void ObjectsListModel::fillModel(const std::vector<attribs_map> &attr_list)
{
	QFontMetrics fm(qApp->font());
	ItemData item_dt;
	ObjectType obj_type, parent_type;
	auto [ h_margin, h_margin_no_ico, v_margin ] = getIndexMargins();

	/* When configuring a list of object attributes the extra column (sixth one)
	 * will always be the signature */
	configureHeader(Attributes::Signature);

	for(auto &attribs : attr_list)
	{
		obj_type = static_cast<ObjectType>(attribs.at(Attributes::ObjectType).toUInt());

		//First column: Object name
		item_dt.clear();
		item_dt.text = attribs.at(Attributes::Name);
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_dt.icon = GuiUtilsNs::getIconPath(obj_type);
		item_data.append(item_dt);

		//Second column: Object type
		item_dt.clear();
		item_dt.text = BaseObject::getTypeName(obj_type);
		item_dt.obj_type = obj_type;
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin_no_ico, v_margin);
		item_dt.italic = true;
		item_data.append(item_dt);

		//Third column: Object id
		item_dt.clear();
		item_dt.text = attribs.at(Attributes::Oid);
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin_no_ico, v_margin);
		item_data.append(item_dt);

		//Fourth column: Parent object name
		item_dt.clear();
		parent_type = BaseObject::getObjectType(attribs.at(Attributes::ParentType));
		item_dt.text = parent_type != ObjectType::BaseObject ? attribs.at(Attributes::Parent) : "-";
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);

		if(parent_type != ObjectType::BaseObject)
			item_dt.icon = GuiUtilsNs::getIconPath(parent_type);

		item_data.append(item_dt);

		//Fifth column: Parent object type
		item_dt.clear();
		item_dt.text = parent_type != ObjectType::BaseObject ? BaseObject::getTypeName(parent_type) : "-";
		item_dt.obj_type = parent_type;
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_dt.italic = true;
		item_data.append(item_dt);

		//Sixth column: Object signature
		item_dt.clear();
		item_dt.text = attribs.at(Attributes::Signature);
		item_dt.sz_hint = fm.boundingRect(item_dt.text).size() + QSize(h_margin, v_margin);
		item_data.append(item_dt);
	}
}

QVariant ObjectsListModel::data(const QModelIndex &index, int role) const
{
	if(index.row() < row_count && index.column() < col_count)
		return getItemData(item_data.at((index.row() * col_count) + index.column()), role);

	return {};
}

QVariant ObjectsListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal)
	{
		if(section >= col_count)
			return {};

		return getItemData(header_data.at(section), role);
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ObjectsListModel::flags(const QModelIndex &) const
{
	return (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
}

bool ObjectsListModel::isEmpty()
{
	return (row_count <= 0);
}
