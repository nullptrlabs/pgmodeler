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

#include "crossscenerelswidget.h"
#include "basetableview.h"
#include "guiutilsns.h"

CrossSceneRelsWidget::CrossSceneRelsWidget(QWidget *parent): QWidget(parent)
{
	table = nullptr;
	model = nullptr;

	setWindowTitle(tr("Cross-scene relationships"));

	table_sel = new ObjectSelectorWidget({ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable }, this);
	table_sel->setReadOnly(true);

	rels_tab = new CustomTableWidget(CustomTableWidget::NoButtons, false, this);
	rels_tab->setColumnCount(3);
	rels_tab->setHeaderLabel(tr("Relationship"), 0);
	rels_tab->setHeaderIcon(GuiUtilsNs::getIcon("relationship"), 0);
	rels_tab->setHeaderLabel(tr("Type"), 1);
	rels_tab->setHeaderLabel(tr("Related table"), 2);
	rels_tab->setHeaderIcon(GuiUtilsNs::getIcon("table"), 2);

	QVBoxLayout *layout = GuiUtilsNs::createVBoxLayout(GuiUtilsNs::LtMargin, GuiUtilsNs::LtSpacing, this);
	layout->addWidget(table_sel);
	layout->addWidget(rels_tab);

	setMinimumSize(640, 320);

	connect(rels_tab, &CustomTableWidget::s_cellDoubleClicked, this, [this](int row, int){
		BaseRelationship *rel = reinterpret_cast<BaseRelationship *>(rels_tab->getRowData(row).value<void *>());

		/* The row may be stale: an earlier edition can cascade into removing this relationship or
		 * one of its tables. In that case the grid is refreshed instead of activating the row */
		if(model->getObjectIndex(rel) >= 0 &&
			 model->getObjectIndex(rel->getTable(BaseRelationship::SrcTable)) >= 0 &&
			 model->getObjectIndex(rel->getTable(BaseRelationship::DstTable)) >= 0)
			emit s_relationshipActivated(rel);
		else
			updateRelationships();
	});
}

void CrossSceneRelsWidget::setAttributes(DatabaseModel *model, BaseTable *table)
{
	this->model = model;
	this->table = table;
	table_sel->setSelectedObject(table);
	updateRelationships();
}

void CrossSceneRelsWidget::updateRelationships()
{
	/* The relationships are read from the table's current view every time instead of being
	 * cached, since that view (and its list) can be replaced while the user edits a relationship */
	BaseTableView *tab_view = nullptr;
	QList<BaseRelationship *> rels;

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
		rels = tab_view->getExternalRelationships();

	rels_tab->blockSignals(true);
	rels_tab->removeRows();

	for(auto &rel : rels)
	{
		BaseTable *other_tab = rel->getTable(BaseRelationship::SrcTable) == table ?
														 rel->getTable(BaseRelationship::DstTable) :
														 rel->getTable(BaseRelationship::SrcTable);
		int row = rels_tab->addRow();

		rels_tab->setCellText(rel->getName(), row, 0);
		rels_tab->setCellIcon(GuiUtilsNs::getIcon(rel->getRelTypeAttribute().replace("rel", "relationship")), row, 0);
		rels_tab->setCellText(rel->getRelationshipTypeName(), row, 1);
		rels_tab->setCellText(other_tab->getSignature(true), row, 2);
		rels_tab->setCellIcon(GuiUtilsNs::getIcon(other_tab->getSchemaName()), row, 2);
		rels_tab->setRowData(QVariant::fromValue<void *>(rel), row);
	}

	rels_tab->clearSelection();
	rels_tab->blockSignals(false);
	rels_tab->resizeContents();
}
