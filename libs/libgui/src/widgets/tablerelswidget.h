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
\ingroup libgui
\class TableRelsWidget
\brief Lists the relationships of a table that are represented by its relationship marker, along with the table on the other end: those
of the table's own scene (first tab) and those whose other end lives in a different scene (second tab)
*/

#ifndef TABLE_RELS_WIDGET_H
#define TABLE_RELS_WIDGET_H

#include "guiglobal.h"
#include "ui_tablerelswidget.h"
#include "customtablewidget.h"
#include "objectselectorwidget.h"
#include "databasemodel.h"
#include "basetable.h"
#include "baserelationship.h"
#include <QHash>

class __libgui TableRelsWidget: public QWidget, public Ui::TableRelsWidget {
	Q_OBJECT

	private:
		//! \brief Read-only selector showing the table whose relationships are listed
		ObjectSelectorWidget *table_sel;

		//! \brief Grid listing one relationship of the table's own scene per row
		CustomTableWidget *hidden_rels_tab;

		//! \brief Grid listing one cross-scene relationship per row
		CustomTableWidget *cross_rels_tab;

		/*! \brief Relationship of each row of the grids, by the row's key (see getRowKey()). The grids are
		 * sortable, which moves the rows without moving the data stored in their vertical headers, so the
		 * relationship can't be kept as row data */
		QHash<QString, BaseRelationship *> rels_rows;

		//! \brief Key identifying the relationship of a grid's row: the contents of its name and type cells
		QString getRowKey(CustomTableWidget *grid, unsigned row);

		//! \brief Table whose relationships are listed
		BaseTable *table;

		//! \brief Model that owns the table and the relationships
		DatabaseModel *model;

		//! \brief Column of the grids showing the table on the other end of the relationship
		static constexpr unsigned RelatedTableCol = 2;

		//! \brief Returns the table on the other end of rel (the listed table itself in a self-relationship)
		BaseTable *getRelatedTable(BaseRelationship *rel);

		//! \brief Creates a grid with the relationship/type/related table columns, wired to s_objectActivated
		CustomTableWidget *createRelsGrid();

		//! \brief Fills a grid with one sortable row per relationship
		void fillRelsGrid(CustomTableWidget *grid, const QList<BaseRelationship *> &rels);

	public:
		explicit TableRelsWidget(QWidget *parent = nullptr);

		//! \brief Configures the widget to list the relationships of the specified table
		void setAttributes(DatabaseModel *model, BaseTable *table);

	public slots:
		//! \brief Reloads the grids from the table's current relationships
		void updateRelationships();

	signals:
		/*! \brief Emitted when the user double-clicks a cell of a relationship's row. The object is the
		 * relationship, or the related table when the cell is the one showing it */
		void s_objectActivated(BaseObject *object);
};

#endif
