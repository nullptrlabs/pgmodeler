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

#include "columnpickerwidget.h"
#include "guiutilsns.h"
#include "table.h"
#include "view.h"
#include "relationship.h"
#include "messagebox.h"

ColumnPickerWidget::ColumnPickerWidget(QWidget *parent) :	QWidget(parent)
{
	setupUi(this);

	parent_obj = nullptr;

	columns_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
																		 (CustomTableWidget::EditButton |
																			CustomTableWidget::UpdateButton |
																			CustomTableWidget::DuplicateButton), true, this);

	columns_tab->setColumnCount(2);
	columns_tab->setHeaderLabel(tr("Column"), 0);
	columns_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("column")),0);
	columns_tab->setHeaderLabel(tr("Type"), 1);
	columns_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);

	col_picker_grid->addWidget(columns_tab, 1, 0, 1, 3);

	connect(columns_tab, &CustomTableWidget::s_rowAdded, this, [this](int idx){
		__trycatch( addColumn(idx); )
	});

	connect(columns_tab, &CustomTableWidget::s_rowRemoved, this, [this](int){
		__trycatch( updateColumnsCombo(); )
	});

	connect(columns_tab, &CustomTableWidget::s_rowsRemoved, this, [this](){
		__trycatch( updateColumnsCombo(); )
	});

	setParentObject(nullptr);
}

void ColumnPickerWidget::setParentObject(BaseObject *p_obj)
{
	try
	{
		/* Currently, column picker supports only tables and relatinoships.
	 * Since views can't handle columns yet they will be ignored */
		if(p_obj &&
				p_obj->getObjectType() != ObjectType::Table &&
				p_obj->getObjectType() != ObjectType::View &&
				p_obj->getObjectType() != ObjectType::Relationship)
			p_obj = nullptr;

		parent_obj = p_obj;
		setEnabled(p_obj != nullptr);

		columns_tab->blockSignals(true);
		columns_tab->removeRows();
		columns_tab->blockSignals(false);

		updateColumnsCombo();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ColumnPickerWidget::setColumns(const std::vector<Column *> &cols)
{
	int row = 0;

	columns_tab->blockSignals(true);
	columns_tab->removeRows();

	for(auto &col : cols)
	{
		if(parent_obj &&
				(col->getParentRelationship() == parent_obj ||
				 col->getParentTable() == parent_obj))
		{
			columns_tab->addRow();
			addColumn(col, row++);
			columns_cmb->removeItem(columns_cmb->findData(QVariant::fromValue<void *>(col)));
		}
	}

	columns_tab->clearSelection();
	columns_tab->blockSignals(false);
}

void ColumnPickerWidget::setColumns(const std::vector<SimpleColumn> &cols)
{
	int row = 0;

	columns_tab->blockSignals(true);
	columns_tab->removeRows();

	for(auto &col : cols)
	{
		if(parent_obj)
		{
			columns_tab->addRow();
			addColumn(col, row++);
			columns_cmb->removeItem(columns_cmb->findData(QVariant::fromValue<SimpleColumn>(col)));
		}
	}

	columns_tab->clearSelection();
	columns_tab->blockSignals(false);
}

void ColumnPickerWidget::updateColumnsCombo()
{
	try
	{
		Column *column=nullptr;
		std::vector<TableObject *> columns;
		std::vector<SimpleColumn> simple_cols;
		Table *table = dynamic_cast<Table *>(parent_obj);
		View *view = dynamic_cast<View *>(parent_obj);
		Relationship *relationship = dynamic_cast<Relationship *>(parent_obj);

		if(table)
			columns = *table->getObjectList(ObjectType::Column);
		else if(relationship)
			columns = relationship->getAttributes();
		else if(view)
			simple_cols = view->getColumns();

		columns_cmb->clear();

		if(!view)
		{
			for(auto &col : columns)
			{
				column = dynamic_cast<Column *>(col);

				//If the column does not exists on the column's table, adds it
				if(columns_tab->getRowIndex(QVariant::fromValue<void *>(column)) < 0)
					columns_cmb->addItem(QString("%1 (%2)").arg(column->getName(), ~column->getType()), QVariant::fromValue<void *>(column));
			}
		}
		else
		{
			for(auto &col : simple_cols)
			{
				//If the column does not exists on the column's table, adds it
				if(columns_tab->getRowIndex(QVariant::fromValue<SimpleColumn>(col)) < 0)
					columns_cmb->addItem(QString("%1 (%2)").arg(col.getName(), col.getType()), QVariant::fromValue<SimpleColumn>(col));
			}
		}

		columns_tab->setButtonsEnabled(CustomTableWidget::AddButton, columns_cmb->count() != 0);
		columns_cmb->adjustSize();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ColumnPickerWidget::addColumn(int row)
{
	try
	{
		if(parent_obj->getObjectType() == ObjectType::View)
		{
			SimpleColumn simple_col = columns_cmb->itemData(columns_cmb->currentIndex(), Qt::UserRole).value<SimpleColumn>();
			addColumn(simple_col, row);
		}
		else
		{
			Column *column = reinterpret_cast<Column *>(columns_cmb->itemData(columns_cmb->currentIndex(), Qt::UserRole).value<void *>());
			addColumn(column, row);
		}

		columns_cmb->removeItem(columns_cmb->currentIndex());
		columns_tab->setButtonsEnabled(CustomTableWidget::AddButton, (columns_cmb->count()!=0));
	}
	catch(Exception &e)
	{
		columns_tab->removeRow(row);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ColumnPickerWidget::addColumn(Column *column, int row)
{
	if(!column || row < 0)
		return;

	columns_tab->setCellText(column->getName(), row, 0);
	columns_tab->setCellText(~column->getType(), row, 1);
	columns_tab->setRowData(QVariant::fromValue<void *>(column), row);

	//Change the table row background color if the column is protected or added by relationship
	if(column->isAddedByRelationship() || column->isProtected())
	{
		QFont fnt;
		fnt=columns_tab->font();
		fnt.setItalic(true);

		if(column->isAddedByRelationship())
		{
			columns_tab->setRowFont(row, fnt);
			columns_tab->setRowColors(row,
																CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemFgColor),
																CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemBgColor));
		}
		else
		{
			columns_tab->setRowFont(row, fnt);
			columns_tab->setRowColors(row,
																CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemFgColor),
																CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemBgColor));
		}
	}
}

void ColumnPickerWidget::addColumn(const SimpleColumn &column, int row)
{
	if(column.getName().isEmpty() || row < 0)
		return;

	columns_tab->setCellText(column.getName(), row, 0);
	columns_tab->setCellText(column.getType(), row, 1);
	columns_tab->setRowData(QVariant::fromValue<SimpleColumn>(column), row);
}

std::vector<Column *> ColumnPickerWidget::getColumns()
{
	std::vector<Column *> cols;

	for(unsigned row = 0; row < columns_tab->getRowCount(); row++)
		cols.push_back(reinterpret_cast<Column *>(columns_tab->getRowData(row).value<void *>()));

	return cols;
}

std::vector<SimpleColumn> ColumnPickerWidget::getSimpleColumns()
{
	if(parent_obj && parent_obj->getObjectType() != ObjectType::View)
		return {};

	std::vector<SimpleColumn> cols;

	for(unsigned row = 0; row < columns_tab->getRowCount(); row++)
		cols.push_back(columns_tab->getRowData(row).value<SimpleColumn>());

	return cols;
}

void ColumnPickerWidget::clear()
{
	columns_tab->removeRows();
}
