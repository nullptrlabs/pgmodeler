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
\ingroup libgui
\class ResultSetModel
\brief Implements a model representation of ResultSet class which can be used to show large amount of data in instances of QTableView.
*/

#ifndef RESULT_SET_MODEL_H
#define RESULT_SET_MODEL_H

#include "guiglobal.h"
#include <QAbstractTableModel>
#include "resultset.h"
#include "catalog.h"
#include <QIcon>

class __libgui ResultSetModel: public QAbstractTableModel {
	Q_OBJECT

	private:
		int col_count, row_count;

		QStringList item_data, header_data, tooltip_data;

		QList<QIcon> header_icons;

		void insertColumn(int, const QModelIndex &){}
		void insertRow(int, const QModelIndex &){}

	public:
		ResultSetModel(ResultSet &res, Catalog &catalog, QObject *parent = 0);
		int rowCount(const QModelIndex & = QModelIndex()) const override;
		int columnCount(const QModelIndex &) const override;
		QModelIndex index(int row, int column, const QModelIndex &parent) const override;
		QModelIndex parent(const QModelIndex &) const override;
		QVariant data(const QModelIndex &index, int role) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
		Qt::ItemFlags flags(const QModelIndex &) const override;
		void append(ResultSet &res);
		bool isEmpty();

		static QString getPgTypeIconName(const QString &type);
};

#endif
