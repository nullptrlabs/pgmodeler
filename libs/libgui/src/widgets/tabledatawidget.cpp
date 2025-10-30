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

#include "tabledatawidget.h"
#include "customuistyle.h"
#include "messagebox.h"
#include "tools/sqlexecutionwidget.h"
#include "guiutilsns.h"
#include "csvparser.h"
#include "utils/plaintextitemdelegate.h"
#include <QClipboard>

const QString TableDataWidget::PlaceholderColumn {"$placeholder$"};

TableDataWidget::TableDataWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::BaseObject)
{
	Ui_TableDataWidget::setupUi(this);
	configureFormLayout(tabledata_grid, ObjectType::BaseObject);
 	
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(ObjectType::Table)));

	comment_lbl->setVisible(false);
	comment_edt->setVisible(false);

	data_tbw->setItemDelegate(new PlainTextItemDelegate(this, false));

	QFont font=name_edt->font();
	font.setItalic(true);
	name_edt->setReadOnly(true);
	name_edt->setFont(font);

	add_row_tb->setToolTip(add_row_tb->toolTip() + QString(" (%1)").arg(add_row_tb->shortcut().toString()));
	del_rows_tb->setToolTip(del_rows_tb->toolTip() + QString(" (%1)").arg(del_rows_tb->shortcut().toString()));
	dup_rows_tb->setToolTip(dup_rows_tb->toolTip() + QString(" (%1)").arg(dup_rows_tb->shortcut().toString()));
	clear_rows_tb->setToolTip(clear_rows_tb->toolTip() + QString(" (%1)").arg(clear_rows_tb->shortcut().toString()));
	clear_cols_tb->setToolTip(clear_cols_tb->toolTip() + QString(" (%1)").arg(clear_cols_tb->shortcut().toString()));

	add_col_tb->setMenu(&col_names_menu);
	data_tbw->removeEventFilter(this);
	csv_load_parent->setVisible(false);

	csv_load_wgt = new CsvLoadWidget(this, true);
	QVBoxLayout *layout = new QVBoxLayout;

	layout->addWidget(csv_load_wgt);
	layout->setContentsMargins(0,0,0,0);
	csv_load_parent->setLayout(layout);
	csv_load_parent->setMinimumSize(csv_load_wgt->minimumSize());

	setMinimumSize(640, 480);

	connect(add_row_tb, &QToolButton::clicked, this, &TableDataWidget::addRow);
	connect(dup_rows_tb, &QToolButton::clicked, this, &TableDataWidget::duplicateRows);

	connect(del_rows_tb, &QToolButton::clicked, this, &TableDataWidget::deleteRows);
	connect(del_cols_tb, &QToolButton::clicked, this, &TableDataWidget::deleteColumns);

	connect(clear_cols_tb, &QToolButton::clicked, this, &TableDataWidget::clearColumns);
	connect(clear_rows_tb, &QToolButton::clicked, this, [this](){
		clearRows();
	});

	connect(data_tbw, &QTableWidget::currentCellChanged, this, &TableDataWidget::insertRowOnTabPress, Qt::QueuedConnection);
	connect(&col_names_menu, &QMenu::triggered, this, &TableDataWidget::addColumn);
	connect(data_tbw, &QTableWidget::itemSelectionChanged, this, &TableDataWidget::enableButtons);
	connect(data_tbw->horizontalHeader(), &QHeaderView::sectionDoubleClicked, this, &TableDataWidget::changeColumnName);

	connect(csv_load_tb, &QToolButton::toggled, csv_load_parent, &QWidget::setVisible);

	connect(csv_load_wgt, &CsvLoadWidget::s_csvFileLoaded, this, [this](){
		populateDataGrid(csv_load_wgt->getCsvDocument());
	});

	connect(paste_tb, &QToolButton::clicked, this, [this](){
		__trycatch(
			CsvDocument csv_doc = csv_load_wgt->loadCsvFromBuffer(qApp->clipboard()->text(),
																														CsvDocument::Separator,
																														CsvDocument::TextDelimiter,
																														true);
			populateDataGrid(csv_doc);
			qApp->clipboard()->clear();
			paste_tb->setEnabled(false);
		)
	});

	connect(bulkedit_tb, &QToolButton::clicked, this, [this](){
		GuiUtilsNs::openColumnDataForm(data_tbw);
	});

	connect(copy_tb, &QToolButton::clicked, this, [this](){
		SQLExecutionWidget::copySelection(data_tbw, false, true);
		paste_tb->setEnabled(qApp->clipboard()->ownsClipboard());
	});

	connect(data_tbw, &QTableWidget::customContextMenuRequested, this, &TableDataWidget::showItemContextMenu);
}

void TableDataWidget::showItemContextMenu(const QPoint &pos)
{
	QMenu item_menu;
	QAction *act = nullptr;
	QList<QToolButton *> btns = { add_row_tb, add_col_tb, dup_rows_tb, bulkedit_tb, nullptr,
																del_rows_tb, del_cols_tb, nullptr,
																clear_rows_tb, clear_cols_tb, nullptr,
																copy_tb, paste_tb };

	for(auto &btn : btns)
	{
		if(!btn)
		{
			item_menu.addSeparator();
			continue;
		}

		if(btn->menu())
		{
			act = btn->menu()->menuAction();
			act->setIcon(btn->icon());
			act->setText(btn->text());
			act->setShortcut(btn->shortcut());
			item_menu.addAction(act);
		}
		else
			act = item_menu.addAction(btn->icon(), btn->text(), btn->shortcut(), btn, &QToolButton::click);

		act->setEnabled(btn->isEnabled());
	}

	item_menu.exec(data_tbw->viewport()->mapToGlobal(pos));
}

void TableDataWidget::insertRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col)
{
	if(qApp->mouseButtons()==Qt::NoButton &&
			curr_row==0 && curr_col==0 &&
			prev_row==data_tbw->rowCount()-1 && prev_col==data_tbw->columnCount()-1)
	{
		addRow();
	}
}

void TableDataWidget::duplicateRows()
{
	QList<QTableWidgetSelectionRange> sel_ranges=data_tbw->selectedRanges();

	if(!sel_ranges.isEmpty())
	{
		for(auto &sel_rng : sel_ranges)
		{
			for(int row=sel_rng.topRow(); row <= sel_rng.bottomRow(); row++)
			{
				addRow();

				for(int col=0; col < data_tbw->columnCount(); col++)
				{
					data_tbw->item(data_tbw->rowCount() - 1, col)
							->setText(data_tbw->item(row, col)->text());
				}
			}
		}

		data_tbw->clearSelection();
	}
}

void TableDataWidget::deleteRows()
{
	QTableWidgetSelectionRange sel_range;

	while(!data_tbw->selectedRanges().isEmpty())
	{
		sel_range=data_tbw->selectedRanges().at(0);

		for(int i = 0; i < sel_range.rowCount(); i++)
			data_tbw->removeRow(sel_range.topRow());
	}
}

void TableDataWidget::deleteColumns()
{
	int res = Messagebox::confirm(tr("Deleting columns is an irreversible action! Are you sure you want to proceed?"));

	if(Messagebox::isAccepted(res))
	{
		QTableWidgetSelectionRange sel_range;

		while(!data_tbw->selectedRanges().isEmpty())
		{
			sel_range=data_tbw->selectedRanges().at(0);

			for(int i = 0; i < sel_range.columnCount(); i++)
				data_tbw->removeColumn(sel_range.leftColumn());
		}

		//Clears the entire table if no columns is left
		if(data_tbw->columnCount()==0)
		{
			clearRows(false);
			add_row_tb->setEnabled(false);
			clear_cols_tb->setEnabled(false);
		}

		del_cols_tb->setEnabled(false);

		toggleWarningFrame();
		configureColumnNamesMenu();
	}
}

void TableDataWidget::clearRows(bool confirm)
{
	int res = Messagebox::Rejected;

	if(confirm)
		res = Messagebox::confirm(tr("Removing all rows is an irreversible action! Are you sure you want to proceed?"));

	if(!confirm || Messagebox::isAccepted(res))
	{
		data_tbw->clearContents();
		data_tbw->setRowCount(0);
		clear_rows_tb->setEnabled(false);
	}
}

void TableDataWidget::clearColumns()
{
	int res = Messagebox::confirm(tr("Removing all columns is an irreversible action! Are you sure you want to proceed?"));

	if(Messagebox::isAccepted(res))
	{
		clearRows(false);
		data_tbw->setColumnCount(0);
		clear_cols_tb->setEnabled(false);
		alert_frm->setVisible(false);
		add_row_tb->setEnabled(false);
		configureColumnNamesMenu();
	}
}

void TableDataWidget::changeColumnName(int col_idx)
{
	QTableWidgetItem *item=data_tbw->horizontalHeaderItem(col_idx);

	if(item)
	{
		QAction * act=nullptr;

		col_names_menu.blockSignals(true);
		act=col_names_menu.exec(QCursor::pos());
		col_names_menu.blockSignals(false);

		if(act && act->isEnabled())
		{
			QTableWidgetItem *item=data_tbw->horizontalHeaderItem(col_idx);
			QString col_name=act->text();

			item->setText(col_name);

			if(act->text()==PlaceholderColumn)
			{
				item->setFlags(Qt::NoItemFlags);
				item->setForeground(QColor(Qt::red));
				item->setToolTip(tr("Unknown column"));
			}
			else
			{
				PhysicalTable *table=dynamic_cast<PhysicalTable *>(this->object);
				Column *column = table->getColumn(col_name);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
				item->setForeground(data_tbw->horizontalHeader()->palette().color(QPalette::WindowText));
				item->setToolTip(QString("%1 [%2]").arg(col_name).arg(~column->getType()));
			}

			for(int row = 0; row < data_tbw->rowCount(); row++)
			{
				item=data_tbw->item(row, col_idx);

				if(col_name==PlaceholderColumn)
					setItemInvalid(item);
				else
				{
					item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
					item->setBackground(item->data(Qt::UserRole).value<QBrush>());
				}
			}

			toggleWarningFrame();
			configureColumnNamesMenu();
			data_tbw->horizontalHeader()->update();
			data_tbw->update();
		}
	}
}

void TableDataWidget::enableButtons()
{
	QList<QTableWidgetSelectionRange> sel_ranges=data_tbw->selectedRanges();
	bool cols_selected, rows_selected;

	cols_selected = rows_selected = !sel_ranges.isEmpty();

	for(auto &sel_rng : sel_ranges)
	{
		cols_selected &= (sel_rng.columnCount() == data_tbw->columnCount());
		rows_selected &= (sel_rng.rowCount() == data_tbw->rowCount());
	}

	del_rows_tb->setEnabled(cols_selected);
	add_row_tb->setEnabled(data_tbw->columnCount() > 0);
	del_cols_tb->setEnabled(rows_selected);
	dup_rows_tb->setEnabled(cols_selected);
	bulkedit_tb->setEnabled(!sel_ranges.isEmpty());
	copy_tb->setEnabled(!sel_ranges.isEmpty());
}

void TableDataWidget::setAttributes(DatabaseModel *model, PhysicalTable *table)
{
	BaseObjectWidget::setAttributes(model, table, nullptr);
	bool enable=(object != nullptr);

	protected_obj_frm->setVisible(false);
	obj_id_lbl->setVisible(false);
	data_tbw->setEnabled(enable);
	add_row_tb->setEnabled(enable);

	if(object)
		populateDataGrid();
}

void TableDataWidget::populateDataGrid(const CsvDocument &csv_doc)
{
	PhysicalTable *table = dynamic_cast<PhysicalTable *>(this->object);
	QTableWidgetItem *item = nullptr;
	QStringList columns, aux_cols;
	QVector<int> invalid_cols;
	Column *column = nullptr;
	CsvDocument ini_data_csv;

	qApp->setOverrideCursor(Qt::WaitCursor);
	clearRows(false);

	if(!csv_doc.isEmpty())
		ini_data_csv = csv_doc;
	else
	{
		try
		{
			CsvParser csv_parser;
			csv_parser.setColumnInFirstRow(true);
			ini_data_csv = csv_parser.parseBuffer(table->getInitialData());
		}
		catch(Exception &e)
		{
			//qApp->restoreOverrideCursor();

			Messagebox msgbox;
			msgbox.show(e,
									tr("Failed to parse the table's initial data, check the stack trace for more detail. Do you want to dump the data into an external file in order to fix and import them back into the table?"),
									Messagebox::Alert,
									Messagebox::YesNoButtons);

			if(msgbox.isAccepted())
			{
				try
				{
					GuiUtilsNs::selectAndSaveFile(table->getInitialData().toUtf8(),
															 tr("Save CSV to file..."), QFileDialog::AnyFile,
															 {}, {"text/csv", "application/octet-stream"}, "csv");
				}
				catch(Exception &e)
				{
					//msgbox.show(e);
					Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
				}
			}
		}
	}

	/* If the initial data buffer is preset the columns
		there have priority over the current table's columns */
	if(!ini_data_csv.isEmpty())
		columns = ini_data_csv.getColumnNames();
	else
	{
		for(auto object : *table->getObjectList(ObjectType::Column))
			columns.push_back(object->getName());
	}

	data_tbw->setColumnCount(columns.size());

	//Creating the header of the grid
	int col = 0;
	for(auto &col_name : columns)
	{
		column = table->getColumn(col_name);
		item = new QTableWidgetItem(col_name);

		/* Marking the invalid columns. The ones which aren't present in the table
			or were already created in a previous iteration */
		if(!column || aux_cols.contains(col_name))
		{
			invalid_cols.push_back(col);

			if(!column)
				item->setToolTip(tr("Unknown column"));
			else
				item->setToolTip(tr("Duplicated column"));
		}
		else
			item->setToolTip(QString("%1 [%2]").arg(col_name).arg(~column->getType()));

		aux_cols.append(col_name);
		data_tbw->setHorizontalHeaderItem(col++, item);
	}

	//Populating the grid with the data
	for(int row = 0; row < ini_data_csv.getRowCount(); row++)
	{
		addRow();

		for(col = 0; col < ini_data_csv.getColumnCount(); col++)
		{
			//if(col < columns.size())
			data_tbw->item(row, col)->setText(ini_data_csv.getValue(row, col));
		}
	}

	//Disabling invalid columns avoiding the user interaction
	if(!invalid_cols.isEmpty())
	{
		for(int dis_col : invalid_cols)
		{
			for(int row = 0; row < data_tbw->rowCount(); row++)
				setItemInvalid(data_tbw->item(row, dis_col));

			item=data_tbw->horizontalHeaderItem(dis_col);
			item->setFlags(Qt::NoItemFlags);
			item->setForeground(QColor(Qt::red));
		}
	}

	alert_frm->setVisible(!invalid_cols.isEmpty());
	data_tbw->resizeColumnsToContents();
	data_tbw->resizeRowsToContents();

	add_row_tb->setEnabled(!columns.isEmpty());
	clear_cols_tb->setEnabled(!columns.isEmpty());
	configureColumnNamesMenu();
	qApp->restoreOverrideCursor();

}

void TableDataWidget::configureColumnNamesMenu()
{
	PhysicalTable *table=dynamic_cast<PhysicalTable *>(this->object);
	QStringList col_names;

	col_names_menu.clear();

	for(auto object : *table->getObjectList(ObjectType::Column))
		col_names.push_back(object->getName());

	for(int col = 0; col < data_tbw->columnCount(); col++)
		col_names.removeOne(data_tbw->horizontalHeaderItem(col)->text());

	if(!col_names.isEmpty())
	{
		col_names.sort();

		for(QString col_name : col_names)
			col_names_menu.addAction(col_name);
	}

	col_names_menu.addSeparator();
	col_names_menu.addAction(PlaceholderColumn);
}

void TableDataWidget::toggleWarningFrame()
{
	bool has_inv_cols=false;

	for(int col = 0; col < data_tbw->columnCount() && !has_inv_cols; col++)
		has_inv_cols = data_tbw->horizontalHeaderItem(col)->flags()==Qt::NoItemFlags;

	alert_frm->setVisible(has_inv_cols);
}

void TableDataWidget::setItemInvalid(QTableWidgetItem *item)
{
	if(item)
	{
		item->setData(Qt::UserRole, item->background());
		item->setBackground(QColor("#FFC0C0"));
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}
}

QString TableDataWidget::generateDataBuffer()
{
	QStringList val_list, col_names, buffer;
	QString value, dbl_quotes = QString("%1%1").arg(CsvDocument::TextDelimiter);
	int col = 0, col_count = data_tbw->horizontalHeader()->count();

	for(int col=0; col < col_count; col++)
	{
		value = data_tbw->horizontalHeaderItem(col)->text();
		value.replace(CsvDocument::TextDelimiter, dbl_quotes);
		value.append(CsvDocument::TextDelimiter);
		value.prepend(CsvDocument::TextDelimiter);
		col_names.push_back(value);
	}

	//The first line of the buffer consists in the column names
	buffer.push_back(col_names.join(CsvDocument::Separator));

	for(int row = 0; row < data_tbw->rowCount(); row++)
	{
		for(col = 0; col < col_count; col++)
		{
			value = data_tbw->item(row, col)->text();
			value.replace(CsvDocument::TextDelimiter, dbl_quotes);
			value.append(CsvDocument::TextDelimiter);
			value.prepend(CsvDocument::TextDelimiter);
			val_list.push_back(value);
		}

		buffer.push_back(val_list.join(CsvDocument::Separator));
		val_list.clear();
	}

	if(buffer.size() <= 1)
		return "";

	// CSV documents need to end with a line break in order to be parsed correctly
	return buffer.join(CsvDocument::LineBreak) + CsvDocument::LineBreak;
}

void TableDataWidget::enterEvent(QEnterEvent *)
{
	paste_tb->setEnabled(!qApp->clipboard()->text().isEmpty());
}

void TableDataWidget::showEvent(QShowEvent *)
{
	paste_tb->setEnabled(!qApp->clipboard()->text().isEmpty());
}

void TableDataWidget::addRow()
{
	int row=data_tbw->rowCount();
	QTableWidgetItem *item = nullptr;

	data_tbw->blockSignals(true);
	data_tbw->insertRow(row);

	for(int col=0; col < data_tbw->columnCount(); col++)
	{
		item=new QTableWidgetItem;

		if(data_tbw->horizontalHeaderItem(col)->flags()==Qt::NoItemFlags)
			setItemInvalid(item);
		else
			item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		data_tbw->setItem(row, col, item);
	}

	data_tbw->clearSelection();
	data_tbw->setCurrentCell(row, 0, QItemSelectionModel::ClearAndSelect);

	if(item && item->flags()!=Qt::NoItemFlags)
		data_tbw->editItem(data_tbw->item(row, 0));

	data_tbw->blockSignals(false);

	clear_rows_tb->setEnabled(true);
}

void TableDataWidget::addColumn(QAction *action)
{
	if(action)
	{
		QTableWidgetItem *item=nullptr;
		int col = data_tbw->columnCount();

		data_tbw->insertColumn(col);

		item=new QTableWidgetItem;
		item->setText(action->text());
		data_tbw->setHorizontalHeaderItem(col, item);

		for(int row=0; row < data_tbw->rowCount(); row++)
		{
			item=new QTableWidgetItem;
			item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			data_tbw->setItem(row, col, item);
		}

		add_row_tb->setEnabled(true);
		clear_cols_tb->setEnabled(true);
		data_tbw->resizeColumnsToContents();
		configureColumnNamesMenu();
	}
}

void TableDataWidget::applyConfiguration()
{
	try
	{
		PhysicalTable *table = dynamic_cast<PhysicalTable *>(this->object);
		table->setInitialData(generateDataBuffer());
		emit s_closeRequested();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

