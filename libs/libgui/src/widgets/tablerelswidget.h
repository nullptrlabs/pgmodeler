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
\brief Lists the relationships of a table whose other end lives in a different scene, along with the table on that end
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

class __libgui TableRelsWidget: public QWidget {
	Q_OBJECT

	private:
		//! \brief Read-only selector showing the table whose cross-scene relationships are listed
		ObjectSelectorWidget *table_sel;

		//! \brief Grid listing one cross-scene relationship per row (the relationship is the row's data)
		CustomTableWidget *rels_tab;

		//! \brief Table whose cross-scene relationships are listed
		BaseTable *table;

		//! \brief Model that owns the table and the relationships
		DatabaseModel *model;

	public:
		explicit TableRelsWidget(QWidget *parent = nullptr);

		//! \brief Configures the widget to list the cross-scene relationships of the specified table
		void setAttributes(DatabaseModel *model, BaseTable *table);

	public slots:
		//! \brief Reloads the grid from the table's current cross-scene relationships
		void updateRelationships();

	signals:
		//! \brief Emitted when the user double-clicks a relationship's row
		void s_relationshipActivated(BaseRelationship *rel);
};

#endif
