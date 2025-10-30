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

#include "customtablewidget.h"
#include "exception.h"
#include "messagebox.h"

QColor CustomTableWidget::item_colors[12] {
	"#ffb4b4", "#303030", "#a4f9b0",
	"#303030", "#c0ffc0", "#000",
	"#ffffc0", "#000", "#ffc0c0",
	"#000", "#f00000", "#00f000"
};

CustomTableWidget::CustomTableWidget(QWidget *parent) : CustomTableWidget(AllButtons, false, parent)
{

}

CustomTableWidget::CustomTableWidget(ButtonConf button_conf, bool conf_exclusion, QWidget *parent): QWidget(parent)
{
	setupUi(this);

	table_tbw->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(move_down_tb, &QToolButton::clicked, this, &CustomTableWidget::moveRows);
	connect(move_up_tb, &QToolButton::clicked, this, &CustomTableWidget::moveRows);
	connect(move_first_tb, &QToolButton::clicked, this, &CustomTableWidget::moveRows);
	connect(move_last_tb, &QToolButton::clicked, this, &CustomTableWidget::moveRows);
	connect(add_tb, &QToolButton::clicked, this, qOverload<>(&CustomTableWidget::addRow));

	connect(remove_tb, &QToolButton::clicked, this, [this]() {
		removeRow();
	});

	connect(edit_tb, &QToolButton::clicked, this, &CustomTableWidget::editRow);
	connect(update_tb, &QToolButton::clicked, this, &CustomTableWidget::updateRow);
	connect(duplicate_tb, &QToolButton::clicked, this, &CustomTableWidget::duplicateRow);
	connect(remove_all_tb, &QToolButton::clicked, this, &CustomTableWidget::removeRows);
	connect(table_tbw, &QTableWidget::cellClicked, this, qOverload<>(&CustomTableWidget::setButtonsEnabled));
	connect(table_tbw, &QTableWidget::cellActivated, this, qOverload<>(&CustomTableWidget::setButtonsEnabled));
	connect(table_tbw, &QTableWidget::cellDoubleClicked, this, &CustomTableWidget::editRow);
	connect(table_tbw, &QTableWidget::itemSelectionChanged, this, qOverload<>(&CustomTableWidget::setButtonsEnabled));
	connect(table_tbw, &QTableWidget::itemSelectionChanged, this, &CustomTableWidget::emitRowSelected);

	connect(table_tbw, &QTableWidget::cellClicked, this, &CustomTableWidget::s_cellClicked);
	connect(table_tbw, &QTableWidget::cellChanged, this, &CustomTableWidget::s_cellChanged);

	connect(table_tbw, &QTableWidget::customContextMenuRequested, this, [this](const QPoint &pnt){
		emit s_contextMenuRequested(table_tbw->mapToGlobal(pnt),
																table_tbw->itemAt(pnt) != nullptr);
	});

	connect(table_tbw, &QTableWidget::itemSelectionChanged, this, [this](){
		if(table_tbw->selectedRanges().isEmpty())
			emit s_selectionCleared();
	});

	connect(resize_cols_tb, &QToolButton::clicked, this, &CustomTableWidget::resizeContents);

	this->conf_exclusion=conf_exclusion;
	tab_adds_row = false;

	setButtonConfiguration(button_conf);
	setColumnCount(1);

	add_tb->setToolTip(add_tb->toolTip() + QString(" (%1)").arg(add_tb->shortcut().toString()));
	remove_tb->setToolTip(remove_tb->toolTip() + QString(" (%1)").arg(remove_tb->shortcut().toString()));
	remove_all_tb->setToolTip(remove_all_tb->toolTip() + QString(" (%1)").arg(remove_all_tb->shortcut().toString()));
	update_tb->setToolTip(update_tb->toolTip() + QString(" (%1)").arg(update_tb->shortcut().toString()));
	edit_tb->setToolTip(edit_tb->toolTip() + QString(" (%1)").arg(edit_tb->shortcut().toString()));
	duplicate_tb->setToolTip(duplicate_tb->toolTip() + QString(" (%1)").arg(duplicate_tb->shortcut().toString()));
	move_last_tb->setToolTip(move_last_tb->toolTip() + QString(" (%1)").arg(move_last_tb->shortcut().toString()));
	move_first_tb->setToolTip(move_first_tb->toolTip() + QString(" (%1)").arg(move_first_tb->shortcut().toString()));
	move_up_tb->setToolTip(move_up_tb->toolTip() + QString(" (%1)").arg(move_up_tb->shortcut().toString()));
	move_down_tb->setToolTip(move_down_tb->toolTip() + QString(" (%1)").arg(move_down_tb->shortcut().toString()));
}

void CustomTableWidget::setTableItemColor(TableItemColor color_idx, const QColor color)
{
	if(color_idx > RelAddedItemAltFgColor)
		return;

	item_colors[color_idx] = color;
}

QColor CustomTableWidget::getTableItemColor(TableItemColor color_idx)
{
	if(color_idx > RelAddedItemAltFgColor)
		return {};

	return item_colors[color_idx];
}

void CustomTableWidget::setButtonConfiguration(ButtonConf button_conf)
{
	bool move_btn = false;

	//Checking via bitwise operation the buttons available on the 'button_conf'
	move_btn=(button_conf & MoveButtons) == MoveButtons;

	move_down_tb->setVisible(move_btn);
	move_up_tb->setVisible(move_btn);
	move_first_tb->setVisible(move_btn);
	move_last_tb->setVisible(move_btn);

	edit_tb->setVisible((button_conf & EditButton) == EditButton);
	remove_all_tb->setVisible((button_conf & RemoveAllButton) == RemoveAllButton);

	add_tb->setVisible((button_conf & AddButton) == AddButton);
	remove_tb->setVisible((button_conf & RemoveButton) == RemoveButton);
	update_tb->setVisible((button_conf & UpdateButton) == UpdateButton);
	duplicate_tb->setVisible((button_conf & DuplicateButton) == DuplicateButton);

	resize_cols_tb->setVisible((button_conf & ResizeColsButton) == ResizeColsButton);

	//Disabling the horizontal spacers when no buttons are visible
	if(button_conf==NoButtons)
	{
		left_spc->changeSize(0,0,QSizePolicy::Ignored,QSizePolicy::Ignored);
		right_spc->changeSize(0,0,QSizePolicy::Ignored,QSizePolicy::Ignored);
	}
	//Restoring the horizontal spacers when some buttons are visible
	else
	{
		left_spc->changeSize(10,10,QSizePolicy::Expanding,QSizePolicy::Preferred);
		right_spc->changeSize(10,10,QSizePolicy::Expanding,QSizePolicy::Preferred);
	}
}

void CustomTableWidget::setSelectionMode(QAbstractItemView::SelectionMode sel_mode)
{
	table_tbw->setSelectionMode(sel_mode);
}

QTableWidgetItem *CustomTableWidget::getItem(unsigned row_idx, unsigned col_idx)
{
	if(row_idx >= static_cast<unsigned>(table_tbw->rowCount()))
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return table_tbw->item(row_idx, col_idx);
}

void CustomTableWidget::updateVerticalHeader()
{
	QHeaderView *v_header = table_tbw->verticalHeader();

	if(!v_header->isVisible())
		return;

	for(int row = 0; row < table_tbw->rowCount(); row++)
		v_header->model()->setHeaderData(row, Qt::Vertical, QString::number(row + 1), Qt::DisplayRole);
}

void CustomTableWidget::adjustColumnToContents(int col)
{
	table_tbw->resizeColumnToContents(col);
	table_tbw->resizeRowsToContents();
}

void CustomTableWidget::setVerticalHeaderVisible(bool value)
{
	table_tbw->verticalHeader()->setVisible(value);
	table_tbw->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

void CustomTableWidget::setSortingEnabled(bool value)
{
	table_tbw->setSortingEnabled(value);
}

void CustomTableWidget::setAddRowOnTabPress(bool value)
{
	tab_adds_row = value;

	if(tab_adds_row)
		connect(table_tbw, &QTableWidget::currentCellChanged, this, &CustomTableWidget::addRowOnTabPress, Qt::UniqueConnection);
	else
		disconnect(table_tbw, &QTableWidget::currentCellChanged, this, &CustomTableWidget::addRowOnTabPress);
}

void CustomTableWidget::addCustomButton(QToolButton *btn)
{
	if(!btn)
		throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	buttons_lt->addWidget(btn);
	btn->setIconSize(add_tb->iconSize());
	btn->setToolButtonStyle(add_tb->toolButtonStyle());
	btn->setSizePolicy(add_tb->sizePolicy());
	btn->setMaximumSize(add_tb->maximumSize());
	btn->setParent(this);
}

void CustomTableWidget::setColumnCount(unsigned col_count)
{
	if(col_count > 0)
	{
		unsigned i = 0;
		QTableWidgetItem *item = nullptr;

		i = table_tbw->columnCount();
		table_tbw->setColumnCount(col_count);

		for(;i < col_count; i++)
		{
			item = new QTableWidgetItem;
			item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			table_tbw->setHorizontalHeaderItem(static_cast<int>(i),item);
		}
	}
}

void CustomTableWidget::setHeaderLabel(const QString &label, unsigned col_idx)
{
	QTableWidgetItem *item=nullptr;

	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	item=table_tbw->horizontalHeaderItem(col_idx);
	item->setText(label);
}

void CustomTableWidget::setHeaderVisible(unsigned col_idx, bool visible)
{
	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	table_tbw->horizontalHeader()->setSectionHidden(col_idx, !visible);
}

void CustomTableWidget::setHeaderIcon(const QIcon &icon, unsigned col_idx)
{
	QTableWidgetItem *item=nullptr;

	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	item=table_tbw->horizontalHeaderItem(col_idx);
	item->setIcon(icon);
}

void CustomTableWidget::setCellIcon(const QIcon &icon, unsigned row_idx, unsigned col_idx)
{
	getItem(row_idx, col_idx)->setIcon(icon);
}

void CustomTableWidget::setCellText(const QString &text, unsigned row_idx, unsigned col_idx)
{
	getItem(row_idx, col_idx)->setText(text);
}

void CustomTableWidget::setCellFlags(Qt::ItemFlags flags, unsigned int row_idx, unsigned int col_idx)
{
	getItem(row_idx, col_idx)->setFlags(flags);
}

void CustomTableWidget::clearCellText(unsigned row_idx, unsigned col_idx)
{
	try
	{
		setCellText("", row_idx, col_idx);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void CustomTableWidget::setRowFont(int row_idx, const QFont &font)
{
	if(row_idx >= table_tbw->rowCount())
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	int col_count = table_tbw->columnCount();

	for(int col = 0; col < col_count; col++)
		table_tbw->item(row_idx, col)->setFont(font);
}

void CustomTableWidget::setRowColors(int row_idx, const QColor &fg_color, const QColor &bg_color)
{
	if(row_idx >= table_tbw->rowCount())
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	QTableWidgetItem *item=nullptr;
	int col_count = table_tbw->columnCount();

	for(int col = 0; col < col_count; col++)
	{
		item = table_tbw->item(row_idx, col);

		if(!item)
			continue;

		item->setForeground(fg_color);
		item->setBackground(bg_color);
	}
}

void CustomTableWidget::setRowData(const QVariant &data, unsigned row_idx)
{
	QTableWidgetItem *item=nullptr;

	if(row_idx >= static_cast<unsigned>(table_tbw->rowCount()))
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Gets the vertical header of the row. This header stores the whole row data.
	item=table_tbw->verticalHeaderItem(row_idx);
	item->setData(Qt::UserRole, data);
}

unsigned CustomTableWidget::getColumnCount()
{
	return table_tbw->columnCount();
}

unsigned CustomTableWidget::getRowCount()
{
	return table_tbw->rowCount();
}

QString CustomTableWidget::getHeaderLabel(unsigned col_idx)
{
	QTableWidgetItem *item=nullptr;

	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	item=table_tbw->horizontalHeaderItem(col_idx);
	return item->text();
}

QString CustomTableWidget::getCellText(unsigned row_idx, unsigned col_idx)
{
	try
	{
		return getItem(row_idx, col_idx)->text();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

QStringList CustomTableWidget::getCellTexts(unsigned int section_idx, Qt::Orientation orientation)
{
	QStringList texts;
	bool use_cols = (orientation == Qt::Horizontal);
	unsigned end_idx = (use_cols ? getColumnCount() : getRowCount());

	try
	{
		for(unsigned idx = 0; idx < end_idx; idx++)
		{
			if(use_cols)
				texts.append(getCellText(section_idx, idx));
			else
				texts.append(getCellText(idx, section_idx));
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	return texts;
}

Qt::CheckState CustomTableWidget::getCellCheckState(unsigned row_idx, unsigned col_idx)
{
	return getItem(row_idx, col_idx)->checkState();
}

void CustomTableWidget::setCellCheckState(Qt::CheckState check_state, unsigned row_idx, unsigned col_idx)
{
	getItem(row_idx, col_idx)->setCheckState(check_state);
}

void CustomTableWidget::setCellDisabled(bool disabled, unsigned row_idx, unsigned col_idx)
{
	QTableWidgetItem *item = getItem(row_idx, col_idx);

	if(disabled)
		item->setFlags(Qt::NoItemFlags);
	else
		item->setFlags(Qt::ItemIsEnabled);
}

bool CustomTableWidget::isCellDisabled(unsigned row_idx, unsigned col_idx)
{
	return (getItem(row_idx, col_idx)->flags() == Qt::NoItemFlags);
}

QVariant CustomTableWidget::getRowData(unsigned row_idx)
{
	QTableWidgetItem *item=nullptr;

	if(row_idx >= static_cast<unsigned>(table_tbw->rowCount()))
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	item=table_tbw->verticalHeaderItem(row_idx);
	return item->data(Qt::UserRole);
}

int CustomTableWidget::getSelectedRow()
{
	return table_tbw->currentRow();
}

int CustomTableWidget::getRowIndex(const QVariant &data)
{
	QTableWidgetItem *item = nullptr;
	int idx = -1;

	for(auto row = 0; row < table_tbw->rowCount(); row++)
	{
		item = table_tbw->verticalHeaderItem(row);

		if(item && item->data(Qt::UserRole) == data)
		{
			idx = row;
			break;
		}
	}

	return idx;
}

void CustomTableWidget::addColumn(unsigned col_idx)
{
	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		col_idx=table_tbw->columnCount();

	table_tbw->insertColumn(col_idx);
	table_tbw->clearSelection();
	setButtonsEnabled();

	emit s_columnAdded(col_idx);
}

void CustomTableWidget::selectRow(int lin_idx)
{
	QTableWidgetItem *item=nullptr;

	item=table_tbw->item(0,lin_idx);

	if(item)
	{
		item=table_tbw->item(lin_idx,0);
		item->setSelected(true);
		table_tbw->setCurrentItem(item);
		setButtonsEnabled();
	}
}

void CustomTableWidget::addRow(unsigned lin_idx)
{
	QTableWidgetItem *item=nullptr;
	unsigned col_idx, col_cont=table_tbw->columnCount();

	table_tbw->insertRow(lin_idx);

	item=new QTableWidgetItem;
	item->setText(QString("%1").arg(lin_idx+1));
	table_tbw->setVerticalHeaderItem(lin_idx,item);

	for(col_idx=0; col_idx < col_cont; col_idx++)
	{
		item=new QTableWidgetItem;		
		table_tbw->setItem(lin_idx,col_idx,item);
	}

	item=table_tbw->item(lin_idx,0);
	item->setSelected(true);
	table_tbw->setCurrentItem(item);
}

int CustomTableWidget::addRow()
{
	this->addRow(table_tbw->rowCount());
	setButtonsEnabled();
	table_tbw->resizeRowsToContents();

	emit s_rowAdded(table_tbw->rowCount() - 1);
	emit s_rowCountChanged(table_tbw->rowCount());

	return table_tbw->rowCount() - 1;
}

void CustomTableWidget::removeRow(unsigned row_idx)
{
	int count = table_tbw->columnCount();
	bool conf = false;

	if(row_idx >= static_cast<unsigned>(table_tbw->rowCount()))
		throw Exception(ErrorCode::RefRowObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Before remove the row, clears the selection
	table_tbw->clearSelection();

	//Selects all the columns of the row to be removed
	for(int i = 0; i < count; i++)
		table_tbw->item(row_idx, i)->setSelected(true);

	table_tbw->setCurrentItem(table_tbw->item(row_idx, 0));

	//Disable temporarily the exclusion confirmation
	conf = conf_exclusion;
	conf_exclusion = false;
	removeRow();
	conf_exclusion = conf;
}

void CustomTableWidget::removeRow()
{
	QList<QTableWidgetItem *> sel_items = table_tbw->selectedItems();

	if(sel_items.isEmpty())
		return;

	std::vector<int> row_idxs;

	// Storing all items row ids so we can delete the correct rows
	std::for_each(sel_items.begin(), sel_items.end(), [&row_idxs](auto &item){
		row_idxs.push_back(item->row());
	});

	// Sorting the row ids and removing the duplicates
	std::sort(row_idxs.begin(), row_idxs.end());
	row_idxs.erase(std::unique(row_idxs.begin(), row_idxs.end()), row_idxs.end());

	int res = Messagebox::Rejected;
	unsigned row_idx = table_tbw->currentRow();

	if(conf_exclusion)
		res = Messagebox::confirm(tr("Are you sure you want to remove the selected item(s)?"));

	if(!conf_exclusion || (conf_exclusion && Messagebox::isAccepted(res)))
	{
		// Now we iterate reverselly over the row ids list and remove each row
		for(auto itr = row_idxs.rbegin(); itr != row_idxs.rend(); itr++)
		{
			emit s_rowAboutToRemove(*itr);

			setRowData(QVariant::fromValue<void *>(nullptr), *itr);
			table_tbw->removeRow(*itr);
			table_tbw->setCurrentItem(nullptr);
			setButtonsEnabled();

			emit s_rowRemoved(row_idx);
			emit s_rowCountChanged(table_tbw->rowCount());
		}

		updateVerticalHeader();
	}
}

void CustomTableWidget::duplicateRow()
{
	if(table_tbw->currentRow() >= 0)
	{
		int row = table_tbw->rowCount(),
				curr_row = table_tbw->currentRow();

		QTableWidgetItem *curr_item = nullptr,
				*dup_item=nullptr;

		addRow(row);

		for(int col = 0; col < table_tbw->columnCount(); col++)
		{
			curr_item = table_tbw->item(curr_row, col);
			dup_item = table_tbw->item(row, col);
			dup_item->setText(curr_item->text());
		}

		emit s_rowDuplicated(curr_row, row);
		emit s_rowCountChanged(table_tbw->rowCount());
	}
}

void CustomTableWidget::removeRows()
{
	if(table_tbw->rowCount() > 0)
	{
		QObject *sender_obj=sender();
		int res = Messagebox::Rejected;

		/* Only shows the confirmation message if the conf_exclusion is set and the user called the method
			 activating the 'remove_all_tb' button */
		if(conf_exclusion && sender_obj == remove_all_tb)
			res = Messagebox::confirm(tr("Are you sure you want to remove all the items?"));

		if(!conf_exclusion || (conf_exclusion && sender_obj != remove_all_tb) ||
				(conf_exclusion &&  sender_obj == remove_all_tb && Messagebox::isAccepted(res)))
		{
			table_tbw->clearContents();
			table_tbw->setRowCount(0);
			setButtonsEnabled();

			emit s_rowsRemoved();
			emit s_rowCountChanged(table_tbw->rowCount());
		}
	}
}

void CustomTableWidget::removeColumn(unsigned col_idx)
{
	if(col_idx >= static_cast<unsigned>(table_tbw->columnCount()))
		throw Exception(ErrorCode::RefColObjectTabInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	table_tbw->removeColumn(col_idx);
	table_tbw->clearSelection();
	setButtonsEnabled();

	emit s_columnRemoved(col_idx);
}

void CustomTableWidget::moveRows()
{
	QObject *sender_obj=sender();
	QTableWidgetItem *item=nullptr, *item1=nullptr;
	int row=-1, row1=-1;
	unsigned col, col_count=table_tbw->columnCount();
	QVariant aux_data;

	//Get the current selected row. Based upon this index that rows are swaped
	row=table_tbw->currentRow();

	//If the user click "move down" swap the selected row with the row imediately below
	if(sender_obj==move_down_tb)
		row1=row+1;
	//If the user click "move up" swap the selected row with the row imediately above
	else if(sender_obj==move_up_tb)
		row1=row-1;
	//If the user click "move first" swap the selected row with the first row
	else if(sender_obj==move_first_tb)
	{
		//Add a blank line to swap the data later
		this->addRow(0);
		row1=0;
		row++;
	}
	//If the user click "move last" swap the selected row with the last row
	else if(sender_obj==move_last_tb)
	{
		//Add a row at the end of the table
		this->addRow(table_tbw->rowCount());
		row1=table_tbw->rowCount()-1;
	}

	//Checking if the row indexes are valid
	if(row >= 0 && row < table_tbw->rowCount() &&
			row1 >= 0 && row1 < table_tbw->rowCount() &&
			row != row1)
	{
		//To swap the rows is necessary to swap all columns one by one
		for(col=0; col < col_count; col++)
		{
			item=table_tbw->item(row, col);
			table_tbw->takeItem(row, col);

			item1=table_tbw->item(row1, col);
			table_tbw->takeItem(row1, col);

			table_tbw->setItem(row, col, item1);
			table_tbw->setItem(row1, col, item);

			item1->setSelected(false);
			item->setSelected(true);
		}

		table_tbw->setCurrentItem(item);

		//Swapping the row data
		item=table_tbw->verticalHeaderItem(row);
		item1=table_tbw->verticalHeaderItem(row1);

		if(item && item1)
		{
			aux_data=item->data(Qt::UserRole);
			item->setData(Qt::UserRole, item1->data(Qt::UserRole));
			item1->setData(Qt::UserRole, aux_data);
		}

		//Special case for "move last" and "move first"
		if(sender_obj==move_last_tb || sender_obj==move_first_tb)
		{
			table_tbw->removeRow(row);

			if(sender_obj==move_first_tb)
			{
				row1=row-1;
				row=table_tbw->rowCount();
			}
		}

		setButtonsEnabled();
		table_tbw->resizeRowsToContents();

		emit s_rowsMoved(row, row1);

		updateVerticalHeader();
	}
}

void CustomTableWidget::editRow()
{
	emit s_rowEdited(table_tbw->currentRow());
}

void CustomTableWidget::updateRow()
{
	emit s_rowUpdated(table_tbw->currentRow());
}

void CustomTableWidget::clearSelection()
{
	table_tbw->clearSelection();
	table_tbw->setCurrentItem(nullptr);
	setButtonsEnabled();
}

void CustomTableWidget::setButtonsEnabled(ButtonConf button_conf, bool value)
{
	int row = -1, last_row = table_tbw->rowCount() - 1;
	bool multi_sel = false;
	QTableWidgetItem *item = table_tbw->currentItem();
	QList<QTableWidgetSelectionRange> sel_ranges = table_tbw->selectedRanges();

	if(item)
		row = item->row();

	if(!sel_ranges.isEmpty())
		multi_sel = sel_ranges.first().rowCount() > 1 || sel_ranges.size() > 1;

	if((button_conf & MoveButtons) == MoveButtons)
	{
		move_up_tb->setEnabled(value && row > 0 && !multi_sel);
		move_down_tb->setEnabled(value && row >= 0 && row < last_row && !multi_sel);
		move_first_tb->setEnabled(value && row > 0 && row <= last_row && !multi_sel);
		move_last_tb->setEnabled(value && row >=0 && row < last_row && !multi_sel);
	}

	if((button_conf & EditButton) == EditButton)
		edit_tb->setEnabled(value && row >= 0 && !multi_sel);

	if((button_conf & AddButton) == AddButton)
		add_tb->setEnabled(value);

	if((button_conf & RemoveButton) == RemoveButton)
		remove_tb->setEnabled(value && row >= 0);

	if((button_conf & RemoveAllButton) == RemoveAllButton)
		remove_all_tb->setEnabled(value && table_tbw->rowCount() > 0);

	if((button_conf & UpdateButton) == UpdateButton)
		update_tb->setEnabled(value && row >= 0 && !multi_sel);

	if((button_conf & DuplicateButton) == DuplicateButton)
		duplicate_tb->setEnabled(value && row >= 0 && !multi_sel);

	if((button_conf & ResizeColsButton) == ResizeColsButton)
		resize_cols_tb->setEnabled(value && table_tbw->rowCount() > 0);
}

void CustomTableWidget::setCellsEditable(bool value)
{
	table_tbw->setEditTriggers(value ? QAbstractItemView::AllEditTriggers : QAbstractItemView::NoEditTriggers);
}

void CustomTableWidget::resizeContents()
{
	table_tbw->resizeColumnsToContents();
	table_tbw->resizeRowsToContents();
	table_tbw->horizontalHeader()->setSectionResizeMode(table_tbw->horizontalHeader()->count() - 1, QHeaderView::Stretch);
}

void CustomTableWidget::setButtonsEnabled()
{
	setButtonsEnabled(AllButtons, true);
}

void CustomTableWidget::emitRowSelected()
{
	QTableWidgetItem *item=table_tbw->currentItem();

	if(item && item->row() >= 0)
		emit s_rowSelected(item->row());
}

void CustomTableWidget::resizeEvent(QResizeEvent *)
{
	table_tbw->resizeRowsToContents();
}

void CustomTableWidget::addRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col)
{
	if(qApp->mouseButtons() == Qt::NoButton && tab_adds_row &&
		 table_tbw->editTriggers() == QTableWidget::AllEditTriggers &&
		 curr_row == 0 && curr_col == 0 &&
		 prev_row == table_tbw->rowCount() - 1 && prev_col == table_tbw->columnCount() - 1)
	{
		addRow();
	}
}
