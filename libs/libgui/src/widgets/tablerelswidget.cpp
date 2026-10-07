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
	table = nullptr;
	model = nullptr;

	setWindowTitle(tr("Table relationships"));

	table_sel = new ObjectSelectorWidget({ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable }, this);
	table_sel->setReadOnly(true);

	hidden_rels_tab = createRelsGrid();
	hidden_rels_tab->setObjectName("hidden_rels_tab");

	cross_rels_tab = createRelsGrid();
	cross_rels_tab->setObjectName("cross_rels_tab");

	rels_tbw = new QTabWidget(this);

	// Each grid sits in its own page so it gets a margin inside the tab
	for(auto &[grid, title] : QList<QPair<CustomTableWidget *, QString>> {
			{ hidden_rels_tab, tr("Relationships") }, { cross_rels_tab, tr("Cross rels.") } })
	{
		QWidget *page = new QWidget;
		QVBoxLayout *page_lt = GuiUtilsNs::createVBoxLayout(GuiUtilsNs::LtMargin, GuiUtilsNs::LtSpacing, page);

		page_lt->addWidget(grid);
		rels_tbw->addTab(page, GuiUtilsNs::getIcon("relationship"), title);
	}

	QVBoxLayout *layout = GuiUtilsNs::createVBoxLayout(GuiUtilsNs::LtMargin, GuiUtilsNs::LtSpacing, this);
	layout->addWidget(table_sel);
	layout->addWidget(rels_tbw);

	setMinimumSize(640, 320);
}

CustomTableWidget *TableRelsWidget::createRelsGrid()
{
	CustomTableWidget *grid = new CustomTableWidget(CustomTableWidget::NoButtons, false, this);

	grid->setColumnCount(3);
	grid->setHeaderLabel(tr("Relationship"), 0);
	grid->setHeaderIcon(GuiUtilsNs::getIcon("relationship"), 0);
	grid->setHeaderLabel(tr("Type"), 1);
	grid->setHeaderLabel(tr("Related table"), 2);
	grid->setHeaderIcon(GuiUtilsNs::getIcon("table"), 2);
	grid->setHeaderVisible(Qt::Vertical, true);

	connect(grid, &CustomTableWidget::s_cellDoubleClicked, this, [this, grid](int row, int){
		BaseRelationship *rel = reinterpret_cast<BaseRelationship *>(grid->getRowData(row).value<void *>());

		/* The row may be stale: an earlier edition can cascade into removing this relationship or
		 * one of its tables. In that case the grids are refreshed instead of activating the row */
		if(model->getObjectIndex(rel) >= 0 &&
			 model->getObjectIndex(rel->getTable(BaseRelationship::SrcTable)) >= 0 &&
			 model->getObjectIndex(rel->getTable(BaseRelationship::DstTable)) >= 0)
			emit s_relationshipActivated(rel);
		else
			updateRelationships();
	});

	return grid;
}

void TableRelsWidget::setAttributes(DatabaseModel *model, BaseTable *table)
{
	this->model = model;
	this->table = table;
	table_sel->setSelectedObject(table);
	updateRelationships();
}

void TableRelsWidget::fillRelsGrid(CustomTableWidget *grid, const QList<BaseRelationship *> &rels)
{
	grid->blockSignals(true);
	grid->removeRows();

	for(auto &rel : rels)
	{
		BaseTable *other_tab = rel->getTable(BaseRelationship::SrcTable) == table ?
														 rel->getTable(BaseRelationship::DstTable) :
														 rel->getTable(BaseRelationship::SrcTable);
		int row = grid->addRow();

		grid->setCellText(rel->getName(), row, 0);
		grid->setCellIcon(GuiUtilsNs::getIcon(rel->getRelTypeAttribute().replace("rel", "relationship")), row, 0);
		grid->setCellText(rel->getRelationshipTypeName(), row, 1);
		grid->setCellText(other_tab->getSignature(true), row, 2);
		grid->setCellIcon(GuiUtilsNs::getIcon(other_tab->getSchemaName()), row, 2);
		grid->setRowData(QVariant::fromValue<void *>(rel), row);
	}

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

	fillRelsGrid(hidden_rels_tab, hidden_rels);
	fillRelsGrid(cross_rels_tab, cross_rels);
}
