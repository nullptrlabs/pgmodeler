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

#include "resultsetmodel.h"
#include "guiutilsns.h"
#include <pgsqltypes/pgsqltype.h>

ResultSetModel::ResultSetModel(ResultSet &res, Catalog &catalog, QObject *parent) : QAbstractTableModel(parent)
{
	try
	{
		Catalog aux_cat = catalog;
		std::vector<unsigned> type_ids;
		std::vector<unsigned>::iterator end;
		std::vector<attribs_map> types;
		std::map<int, QString> type_names;
		//int col = 0;

		header_icons.clear();
		col_count = res.getColumnCount();
		row_count = res.getTupleCount();

		insertColumns(0, col_count);
		insertRows(0, row_count);

		for(int col = 0; col < col_count; col++)
		{
			header_data.push_back(" " + res.getColumnName(col));
			type_ids.push_back(res.getColumnTypeId(col));
		}

		if(res.accessTuple(ResultSet::FirstTuple))
		{
			do
			{
				//Fills the current row with the values of current tuple
				for(int col = 0; col < col_count; col++)
				{
					item_data.push_back(res.getColumnValue(col));
				}
			}
			while(res.accessTuple(ResultSet::NextTuple));
		}

		aux_cat.setQueryFilter(Catalog::ListAllObjects);
		std::sort(type_ids.begin(), type_ids.end());
		end=std::unique(type_ids.begin(), type_ids.end());
		type_ids.erase(end, type_ids.end());

		types = aux_cat.getObjectsAttributes(ObjectType::Type, "", "", type_ids);
		//col = 0;

		for(auto &tp : types)
			type_names[tp[Attributes::Oid].toInt()] = tp[Attributes::Name];

		int tp_id = 0;

		for(int col = 0; col < col_count; col++)
		{
			tp_id = res.getColumnTypeId(col);
			header_icons.append(QIcon(GuiUtilsNs::getIconPath(getPgTypeIconName(type_names[tp_id]))));
			tooltip_data.push_back(type_names[tp_id]);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

int ResultSetModel::rowCount(const QModelIndex &) const
{
	return row_count;
}

int ResultSetModel::columnCount(const QModelIndex &) const
{
	return col_count;
}

QModelIndex ResultSetModel::index(int row, int column, const QModelIndex &parent) const
{
	return QAbstractTableModel::index(row, column, parent);
}

QModelIndex ResultSetModel::parent(const QModelIndex &) const
{
	return {};
}

QVariant ResultSetModel::data(const QModelIndex &index, int role) const
{
	if(index.row() < row_count && index.column() < col_count)
	{
		if(role == Qt::DisplayRole)
			return (item_data.at((index.row() * col_count) + index.column()));

		if(role == Qt::TextAlignmentRole)
			return { Qt::AlignLeft | Qt::AlignVCenter };
	}

	return {};
}

QVariant ResultSetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal)
	{
		if(section >= col_count)
			return {};

		if(role == Qt::DisplayRole)
			return header_data.at(section);

		if(role == Qt::DecorationRole)
			return header_icons.at(section);

		if(role == Qt::ToolTipRole)
			return tooltip_data.at(section);

		if(role == Qt::TextAlignmentRole)
			return { Qt::AlignLeft | Qt::AlignVCenter };
	}

	return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ResultSetModel::flags(const QModelIndex &) const
{
	return (Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled );
}

void ResultSetModel::append(ResultSet &res)
{
	try
	{
		if(res.isValid() && !res.isEmpty())
		{
			if(res.accessTuple(ResultSet::FirstTuple))
			{
				do
				{
					for(int col = 0; col < col_count; col++)
					{
						item_data.push_back(col < res.getColumnCount() ?
																res.getColumnValue(col) : "");
					}
				}
				while(res.accessTuple(ResultSet::NextTuple));
			}

			row_count += res.getTupleCount();
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

bool ResultSetModel::isEmpty()
{
	return (row_count <= 0);
}

QString ResultSetModel::getPgTypeIconName(const QString &type)
{
	try
	{
		static QStringList category_icons = {
			/* OidType */ "typeoid",
			/* PolymorphicType */ "typepolymorphic",
			/* PseudoType */ "typepseudo",
			/* TimezoneType */ "typetimezone",
			/* DateTimeType */ "typedatetime",
			/* NumericType */ "typenumeric",
			/* IntegerType */ "typeinteger",
			/* FloatPointType */ "typefloatpoint",
			/* CharacterType */ "typecharacter",
			/* NetworkType */ "typenetwork",
			/* MonetaryType */ "typemonetary",
			/* BinaryType */ "typebinary",
			/* BooleanType */ "typeboolean",
			/* GeometricType */ "typegeometric",
			/* BitStringType */ "typebitstring",
			/* TextSearchType */ "typetextsearch",
			/* UuidType */ "typeuuid",
			/* XmlType */ "typexml",
			/* JsonType */ "typejson",
			/* RangeType */ "typerange",
			/* PostGiSType */ "typepostgis",
			/* OtherType */ "typeother"
			/* SerialType (not used) */
			/* UserType (not used) */
		};

		PgSqlType pgtype = PgSqlType::parseString(type);
		PgSqlType::TypeCategory cat = pgtype.getCategory();

		return cat < category_icons.size() ?
					 category_icons.at(cat) : "usertype";
	}
	catch(Exception &)
	{
		return "usertype";
	}
}
