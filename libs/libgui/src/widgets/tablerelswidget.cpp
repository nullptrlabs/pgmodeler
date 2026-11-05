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

#include "tablerelswidget.h"
#include "basetableview.h"
#include "guiutilsns.h"

TableRelsWidget::TableRelsWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);

	table = nullptr;
	model = nullptr;

	GuiUtilsNs::configureWidgetFont(tab_name_lbl, GuiUtilsNs::SmallFontFactor, true);

	table_sel = new ObjectSelectorWidget({ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable }, this);
	table_sel->setReadOnly(true);
	table_sel_lt->addWidget(table_sel);
	tab_name_lbl->setBuddy(table_sel);

	hidden_rels_tab = createRelsGrid();
	hidden_rels_tab->setObjectName("hidden_rels_tab");
	hidden_rels_lt->addWidget(hidden_rels_tab);

	cross_rels_tab = createRelsGrid();
	cross_rels_tab->setObjectName("cross_rels_tab");
	cross_rels_lt->addWidget(cross_rels_tab);

	setMinimumSize(640, 320);
}

CustomTableWidget *TableRelsWidget::createRelsGrid()
{
	CustomTableWidget *grid = new CustomTableWidget(CustomTableWidget::NoButtons, false, this);

	grid->setColumnCount(3);
	grid->setHeaderLabel(tr("Relationship"), 0);
	grid->setHeaderIcon(GuiUtilsNs::getIcon("relationship"), 0);
	grid->setHeaderLabel(tr("Type"), 1);
	grid->setHeaderLabel(tr("Related table"), RelatedTableCol);
	grid->setHeaderIcon(GuiUtilsNs::getIcon("table"), RelatedTableCol);
	grid->setHeaderVisible(Qt::Vertical, true);
	grid->setSortingEnabled(true);

	connect(grid, &CustomTableWidget::s_cellDoubleClicked, this, [this, grid](int row, int col){
		BaseRelationship *rel = rels_rows.value(getRowKey(grid, row));

		/* The row may be stale: an earlier edition can cascade into removing this relationship or
		 * one of its tables. In that case the grids are refreshed instead of activating the row */
		if(!rel ||
			 model->getObjectIndex(rel) < 0 ||
			 model->getObjectIndex(rel->getTable(BaseRelationship::SrcTable)) < 0 ||
			 model->getObjectIndex(rel->getTable(BaseRelationship::DstTable)) < 0)
		{
			updateRelationships();
			return;
		}

		// The related table cell edits the table, the other cells edit the relationship
		if(col == RelatedTableCol)
			emit s_objectActivated(getRelatedTable(rel));
		else
			emit s_objectActivated(rel);
	});

	return grid;
}

BaseTable *TableRelsWidget::getRelatedTable(BaseRelationship *rel)
{
	return rel->getTable(BaseRelationship::SrcTable) == table ?
				 rel->getTable(BaseRelationship::DstTable) :
				 rel->getTable(BaseRelationship::SrcTable);
}

QString TableRelsWidget::getRowKey(CustomTableWidget *grid, unsigned row)
{
	return grid->getCellText(row, 0) + "|" + grid->getCellText(row, 1);
}

void TableRelsWidget::setAttributes(DatabaseModel *model, BaseTable *table)
{
	this->model = model;
	this->table = table;
	table_sel->setSelectedObject(table);

	if(table)
		setWindowTitle(tr("Relationships of %1").arg(table->getSignature(true)));

	updateRelationships();
}

void TableRelsWidget::fillRelsGrid(CustomTableWidget *grid, const QList<BaseRelationship *> &rels)
{
	grid->blockSignals(true);
	grid->removeRows();

	// Sorting while the cells are being set would move the row before its other cells are filled
	grid->setSortingEnabled(false);

	for(auto &rel : rels)
	{
		BaseTable *other_tab = getRelatedTable(rel);
		int row = grid->addRow();

		grid->setCellText(rel->getName(), row, 0);
		grid->setCellIcon(GuiUtilsNs::getIcon(rel->getRelTypeAttribute().replace("rel", "relationship")), row, 0);
		grid->setCellText(rel->getRelationshipTypeName(), row, 1);
		grid->setCellText(other_tab->getSignature(true), row, RelatedTableCol);
		grid->setCellIcon(GuiUtilsNs::getIcon(other_tab->getSchemaName()), row, RelatedTableCol);
		rels_rows[getRowKey(grid, row)] = rel;
	}

	grid->setSortingEnabled(true);
	grid->clearSelection();
	grid->blockSignals(false);
	grid->resizeContents();
}

void TableRelsWidget::updateRelationships()
{
	/* The relationships are read from the table's current view every time instead of being
	 * cached, since that view (and its lists) can be replaced while the user edits a relationship */
	BaseTableView *tab_view = nullptr;
	QList<BaseRelationship *> hidden_rels, cross_rels;

	/* The table itself may be gone from the model (a cascade deletion triggered by a relationship
	 * edition), so it's only dereferenced after confirming it's still there */
	if(model->getObjectIndex(table) < 0)
	{
		table = nullptr;
		table_sel->clearSelector();
	}
	else
		tab_view = dynamic_cast<BaseTableView *>(table->getOverlyingObject());

	if(tab_view)
	{
		hidden_rels = tab_view->getHiddenRelationships();
		cross_rels = tab_view->getExternalRelationships();
	}

	rels_rows.clear();
	fillRelsGrid(hidden_rels_tab, hidden_rels);
	fillRelsGrid(cross_rels_tab, cross_rels);
}
