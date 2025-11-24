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

#include "simplecolumnswidget.h"
#include "baseobjectwidget.h"
#include "guiutilsns.h"

SimpleColumnsWidget::SimpleColumnsWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	model = nullptr;

	pgsqltype_wgt = new PgSQLTypeWidget(this);

	columns_tab = new CustomTableWidget(CustomTableWidget::AllButtons, true, this);
	columns_tab->setColumnCount(3);
	columns_tab->setHeaderLabel(tr("Name"), 0);
	columns_tab->setHeaderIcon(GuiUtilsNs::getIcon("uid"),0);
	columns_tab->setHeaderLabel(tr("Type"), 1);
	columns_tab->setHeaderIcon(GuiUtilsNs::getIcon("usertype"),1);
	columns_tab->setHeaderLabel(tr("Alias"), 2);

	QFrame *info_frm = BaseObjectWidget::generateInformationFrame(tr("This tab can be used to inform additional columns that the view owns and were not generated from the object references. This is just a convenience to make the visualization of this kind of object more intuitive."));

	layout()->addWidget(pgsqltype_wgt);
	layout()->addWidget(columns_tab);
	info_frm->setParent(this);
	layout()->addWidget(info_frm);

	setMinimumSize(640, 480);

	connect(columns_tab, &CustomTableWidget::s_rowAdded, this, &SimpleColumnsWidget::addColumn);
	connect(columns_tab, &CustomTableWidget::s_rowUpdated, this, &SimpleColumnsWidget::updateColumn);
	connect(columns_tab, &CustomTableWidget::s_rowEdited, this, &SimpleColumnsWidget::editColumn);
	connect(columns_tab, &CustomTableWidget::s_rowDuplicated, this, &SimpleColumnsWidget::duplicateColumn);
}

void SimpleColumnsWidget::setAttributes(DatabaseModel *model, const std::vector<SimpleColumn> &cols)
{
	this->model = model;
	pgsqltype_wgt->setAttributes(PgSqlType(), model, true,
															 UserTypeConfig::AllUserTypes ^ UserTypeConfig::SequenceType, true, false);

	unsigned row = 0;
	columns_tab->blockSignals(true);

	for(auto &col : cols)
	{
		columns_tab->addRow();
		showColumnData(row, col.getName(), col.getType(), col.getAlias());
		row++;
	}

	columns_tab->clearSelection();
	columns_tab->blockSignals(false);
}

std::vector<SimpleColumn> SimpleColumnsWidget::getColumns()
{
	std::vector<SimpleColumn> cols;

	for(unsigned row = 0; row < columns_tab->getRowCount(); row++)
		cols.push_back(columns_tab->getRowData(row).value<SimpleColumn>());

	return cols;
}

void SimpleColumnsWidget::showColumnData(int row, const QString &name, const QString &type, const QString &alias)
{
	columns_tab->setCellText(name, row, 0);
	columns_tab->setCellText(type, row, 1);
	columns_tab->setCellText(alias, row, 2);
	columns_tab->setRowData(QVariant::fromValue<SimpleColumn>(SimpleColumn(name, type, alias)), row);
}

void SimpleColumnsWidget::handleColumn(int row)
{
	PgSqlType type = pgsqltype_wgt->getPgSQLType();
	showColumnData(row, name_edt->text(), *type, alias_edt->text());

	name_edt->clear();
	alias_edt->clear();
	name_edt->setFocus();
}

void SimpleColumnsWidget::addColumn(int row)
{
	if(!name_edt->text().isEmpty())
		handleColumn(row);
	else
		columns_tab->removeRow(row);
}

void SimpleColumnsWidget::updateColumn(int row)
{
	if(!name_edt->text().isEmpty())
		handleColumn(row);
}

void SimpleColumnsWidget::editColumn(int row)
{
	SimpleColumn col = columns_tab->getRowData(row).value<SimpleColumn>();

	name_edt->setText(col.getName());
	alias_edt->setText(col.getAlias());
	pgsqltype_wgt->setAttributes(PgSqlType::parseString(col.getType()), model, true,
															 UserTypeConfig::AllUserTypes ^ UserTypeConfig::SequenceType, true, false);
}

void SimpleColumnsWidget::duplicateColumn(int src_row, int new_row)
{
	columns_tab->setRowData(columns_tab->getRowData(src_row), new_row);
}
