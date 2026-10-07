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
#include "customtablewidget.h"
#include "objectselectorwidget.h"
#include "databasemodel.h"
#include "basetable.h"
#include "baserelationship.h"
#include <QWidget>
#include <QTabWidget>

class __libgui TableRelsWidget: public QWidget {
	Q_OBJECT

	private:
		//! \brief Read-only selector showing the table whose relationships are listed
		ObjectSelectorWidget *table_sel;

		QTabWidget *rels_tbw;

		//! \brief Grid listing one relationship of the table's own scene per row (the relationship is the row's data)
		CustomTableWidget *hidden_rels_tab;

		//! \brief Grid listing one cross-scene relationship per row (the relationship is the row's data)
		CustomTableWidget *cross_rels_tab;

		//! \brief Table whose relationships are listed
		BaseTable *table;

		//! \brief Model that owns the table and the relationships
		DatabaseModel *model;

		//! \brief Creates a grid with the relationship/type/related table columns, wired to s_relationshipActivated
		CustomTableWidget *createRelsGrid();

		//! \brief Fills a grid with one row per relationship
		void fillRelsGrid(CustomTableWidget *grid, const QList<BaseRelationship *> &rels);

	public:
		explicit TableRelsWidget(QWidget *parent = nullptr);

		//! \brief Configures the widget to list the relationships of the specified table
		void setAttributes(DatabaseModel *model, BaseTable *table);

	public slots:
		//! \brief Reloads the grids from the table's current relationships
		void updateRelationships();

	signals:
		//! \brief Emitted when the user double-clicks a relationship's row
		void s_relationshipActivated(BaseRelationship *rel);
};

#endif
