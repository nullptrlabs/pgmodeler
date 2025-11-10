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

#include "datagridwidget.h"
#include "customuistyle.h"
#include "plaintextitemdelegate.h"
#include "messagebox.h"
#include "guiutilsns.h"
#include "sqlexecutionwidget.h"
#include "customtablewidget.h"
#include "utilsns.h"
#include "databaseexplorerwidget.h"
#include <QClipboard>

DataGridWidget::DataGridWidget(const QString &sch_name, const QString &tab_name, ObjectType obj_type, const attribs_map &conn_params, QWidget * parent, Qt::WindowFlags f): QWidget(parent, f)
{
	setupUi(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::InfoFrmHint, result_info_frm);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, warning_frm);
	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, { tab_info_frm, separator_ln });

	schema_lbl->setText(sch_name);

	rows_cnt_lbl->setToolTip(rows_cnt_ico_lbl->toolTip());
	rows_added_lbl->setToolTip(rows_added_ico_lbl->toolTip());
	rows_changed_lbl->setToolTip(rows_changed_ico_lbl->toolTip());
	rows_deleted_lbl->setToolTip(rows_deleted_ico_lbl->toolTip());

	QAction *act = nullptr;

	save_enabled = undo_enabled = false;
	selection_enabled = browse_enabled = false;
	edit_enabled = export_enabled = filter_enabled = false;

	table_oid = 0;

	this->sch_name = sch_name;
	this->tab_name = tab_name;
	this->conn_params = conn_params;
	this->obj_type = obj_type;

	filter_hl = new SyntaxHighlighter(filter_txt, false, false, font().pointSizeF());
	filter_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	code_compl_wgt = new CodeCompletionWidget(filter_txt);
	code_compl_wgt->configureCompletion(nullptr, filter_hl, "");

	results_tbw->setItemDelegate(new PlainTextItemDelegate(this, false));
	resetDataGrid();

	act = copy_menu.addAction(tr("Copy as text"));
	act->setShortcut(QKeySequence("Ctrl+C"));
	act->setIcon(GuiUtilsNs::getIcon("txtfile"));

	connect(act, &QAction::triggered,	this, [this](){
		SQLExecutionWidget::copySelection(results_tbw, false, false);
		emit s_pasteEnabled(isPasteEnabled());
	});

	act = copy_menu.addAction(tr("Copy as CSV"));
	act->setShortcut(QKeySequence("Ctrl+Shift+C"));
	act->setIcon(GuiUtilsNs::getIcon("csvfile"));

	connect(act, &QAction::triggered, this, [this](){
		SQLExecutionWidget::copySelection(results_tbw, false, true);
		emit s_pasteEnabled(isPasteEnabled());
	});

	act = save_menu.menuAction();
	act->setText(tr("Save as..."));
	act->setIcon(GuiUtilsNs::getIcon("saveas"));
	copy_menu.addAction(act);

	act = save_menu.addAction(tr("Text file"));
	act->setIcon(GuiUtilsNs::getIcon("txtfile"));
	act->setShortcut(QKeySequence("Ctrl+Shift+T"));

	connect(act, &QAction::triggered, this, [this](){
		saveSelectedItems(false);
	});

	act = save_menu.addAction(tr("CSV file"));
	act->setIcon(GuiUtilsNs::getIcon("csvfile"));
	act->setShortcut(QKeySequence("Ctrl+Shift+S"));

	connect(act, &QAction::triggered, this, [this](){
		saveSelectedItems(true);
	});

	act = paste_menu.addAction(tr("Paste as text"));
	act->setShortcut(QKeySequence("Ctrl+V"));
	act->setIcon(GuiUtilsNs::getIcon("txtfile"));

	connect(act, &QAction::triggered,	this, [this](){
		loadDataFromCsv(true, false);
		emit s_pasteEnabled(false);
	});

	act = paste_menu.addAction(tr("Paste as CSV"));
	act->setShortcut(QKeySequence("Ctrl+Shift+V"));
	act->setIcon(GuiUtilsNs::getIcon("csvfile"));

	connect(act, &QAction::triggered,	this, [this](){
		loadDataFromCsv(true, true);
		emit s_pasteEnabled(false);
	});

	action_add = edit_menu.addAction(GuiUtilsNs::getIcon("addrow"),
																	 tr("Add row(s)"), QKeySequence("Ins"));
	action_add->setToolTip(tr("Add empty rows"));

	connect(action_add, &QAction::triggered, this, [this](){
		addRow();
		updateRowOperationsInfo();
	});

	action_delete = edit_menu.addAction(GuiUtilsNs::getIcon("delrow"),
																			tr("Delete row(s)"), QKeySequence("Del"),
																			this, &DataGridWidget::markDeleteOnRows);
	action_delete->setToolTip(tr("Mark the selected rows to be deleted"));

	action_bulk_edit = edit_menu.addAction(GuiUtilsNs::getIcon("bulkedit"), tr("Edit cells"));
	action_bulk_edit->setShortcut(QKeySequence("Ctrl+E"));
	action_bulk_edit->setToolTip(tr("Change the values of all selected cells at once"));

	connect(action_bulk_edit, &QAction::triggered, this, [this](){
		GuiUtilsNs::openColumnDataForm(results_tbw);
	});

	action_duplicate = edit_menu.addAction(GuiUtilsNs::getIcon("duprow"), tr("Duplicate row(s)"), QKeySequence("Ctrl+D"),
																				 this, &DataGridWidget::duplicateRows);
	action_duplicate->setToolTip(tr("Duplicate the selected rows"));

	action_clear = edit_menu.addAction(GuiUtilsNs::getIcon("cleartext"), tr("Clear cell(s)"), QKeySequence("Ctrl+R"),
																		 this, &DataGridWidget::clearItemsText);
	action_clear->setToolTip(tr("Clears the items selected on the grid"));

	truncate_menu.addAction(GuiUtilsNs::getIcon("truncate"),
													tr("Truncate"), QKeySequence("Ctrl+Del"),
													this, &DataGridWidget::truncateTable)->setData(QVariant::fromValue<bool>(false));

	truncate_menu.addAction(GuiUtilsNs::getIcon("trunccascade"),
													tr("Truncate cascade"), QKeySequence("Ctrl+Shift+Del"),
													this, &DataGridWidget::truncateTable)->setData(QVariant::fromValue<bool>(true));

	//Forcing the splitter that handles the bottom widgets to resize its children to their minimum size
	h_splitter->setSizes({500, 250, 500});

	filter_tbw->setVisible(false);
	csv_load_parent->setVisible(false);

	csv_load_wgt = new CsvLoadWidget(this, false);
	QVBoxLayout *layout = new QVBoxLayout;

	layout->addWidget(csv_load_wgt);
	layout->setContentsMargins(0,0,0,0);
	csv_load_parent->setLayout(layout);
	csv_load_parent->setMinimumSize(csv_load_wgt->minimumSize());

	columns_lst->installEventFilter(this);

	act = export_menu.addAction(tr("Text file"));
	act->setIcon(GuiUtilsNs::getIcon("txtfile"));

	connect(act, &QAction::triggered, this, [this](){
		SQLExecutionWidget::exportResults(results_tbw, false);
	});

	act = export_menu.addAction(tr("CSV file"));
	act->setIcon(GuiUtilsNs::getIcon("csvfile"));

	connect(act, &QAction::triggered, this, [this](){
		SQLExecutionWidget::exportResults(results_tbw, true);
	});

	connect(columns_lst, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item){
		if(item->checkState() == Qt::Checked)
			item->setCheckState(Qt::Unchecked);
		else
			item->setCheckState(Qt::Checked);

		toggleColumnDisplay(item);
	});

	connect(select_all_tb, &QToolButton::clicked, this, [this](){
		setColumnsCheckState(Qt::Checked);
	});

	connect(clear_all_tb, &QToolButton::clicked, this, [this](){
		setColumnsCheckState(Qt::Unchecked);
	});

	connect(columns_lst, &QListWidget::itemClicked, this, &DataGridWidget::toggleColumnDisplay);

	connect(add_ord_col_tb, &QToolButton::clicked, this, &DataGridWidget::addSortColumnToList);
	connect(ord_columns_lst, &QListWidget::itemDoubleClicked, this, &DataGridWidget::removeSortColumnFromList);
	connect(ord_columns_lst, &QListWidget::itemPressed, this, &DataGridWidget::changeOrderMode);
	connect(rem_ord_col_tb, &QToolButton::clicked, this, &DataGridWidget::removeSortColumnFromList);
	connect(clear_ord_cols_tb, &QToolButton::clicked, this, &DataGridWidget::clearSortColumnList);
	connect(results_tbw, &QTableWidget::itemChanged, this, &DataGridWidget::markUpdateOnRow);
	connect(ord_columns_lst, &QListWidget::currentRowChanged, this, &DataGridWidget::enableColumnControlButtons);
	connect(move_down_tb,  &QToolButton::clicked, this, &DataGridWidget::swapColumns);
	connect(move_up_tb,  &QToolButton::clicked, this, &DataGridWidget::swapColumns);

	//Using the QueuedConnection here to avoid the "edit: editing failed" when editing and navigating through items using tab key
	connect(results_tbw, &QTableWidget::currentCellChanged, this, &DataGridWidget::insertRowOnTabPress, Qt::QueuedConnection);
	connect(results_tbw, &QTableWidget::customContextMenuRequested, this, &DataGridWidget::showPopupMenu);

	connect(results_tbw, &QTableWidget::itemDoubleClicked, this, [this](QTableWidgetItem *item){
		if(PlainTextItemDelegate::getMaxDisplayLength() > 0 &&
			 !PlainTextItemDelegate::isTextEditorEnabled() &&
			 item->data(Qt::UserRole).toString().length() > PlainTextItemDelegate::getMaxDisplayLength())
		{
			GuiUtilsNs::openColumnDataForm(results_tbw);
		}
	});

	connect(results_tbw, &QTableWidget::itemSelectionChanged, this, &DataGridWidget::enableRowControlButtons);

	connect(csv_load_wgt, &CsvLoadWidget::s_csvFileLoaded, this, [this](){
		loadDataFromCsv();
	});

	connect(results_tbw->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [this](int section, Qt::SortOrder sort_order){
		// Applying the sorting on the clicked column when the Control key is pressed
		if(qApp->keyboardModifiers() == Qt::ControlModifier)
			sortResults(section, sort_order);
		else
			selectColumn(section, sort_order);
	});

	connect(this, &DataGridWidget::s_undoEnabled, this, [this](bool value) {
		undo_enabled = value;
	});

	connect(this, &DataGridWidget::s_saveEnabled, this, [this](bool value) {
		save_enabled = value;
	});

	connect(this, &DataGridWidget::s_selectionEnabled, this, [this](bool value) {
		selection_enabled = value;
	});

	connect(this, &DataGridWidget::s_browseEnabled, this, [this](bool value) {
		browse_enabled = value;
	});

	connect(this, &DataGridWidget::s_editEnabled, this, [this](bool value) {
		edit_enabled = value;
	});

	connect(this, &DataGridWidget::s_exportEnabled, this, [this](bool value) {
		export_enabled = value;
	});

	connect(this, &DataGridWidget::s_filterEnabled, this, [this](bool value) {
		filter_enabled = value;
	});

	connect(this, &DataGridWidget::s_truncateEnabled, this, [this](bool value) {
		truncate_enabled = value;
	});

	connect(duplicate_tb, &QToolButton::clicked, this, [this](){
		emit s_gridDuplicationRequested(this);
	});

	/* Installing event filters in the menus to override their
	 * default position */
	fks_menu.installEventFilter(this);
	copy_menu.installEventFilter(this);
	truncate_menu.installEventFilter(this);
	paste_menu.installEventFilter(this);
	edit_menu.installEventFilter(this);
	export_menu.installEventFilter(this);
}

void DataGridWidget::clearItemsText()
{
	for(auto &sel : results_tbw->selectedRanges())
	{
		for(int row = sel.topRow(); row <= sel.bottomRow(); row++)
		{
			for(int col = sel.leftColumn(); col <= sel.rightColumn(); col++)
				results_tbw->item(row, col)->setText("");
		}
	}
}

void DataGridWidget::sortResults(int column, Qt::SortOrder order)
{
	try
	{
		clearSortColumnList();
		ord_column_cmb->setCurrentIndex(column);
		asc_rb->setChecked(order == Qt::SortOrder::AscendingOrder);
		desc_rb->setChecked(order == Qt::SortOrder::DescendingOrder);
		addSortColumnToList();
		retrieveData();
		results_tbw->horizontalHeader()->setSortIndicator(column, order);
		results_tbw->horizontalHeader()->setSortIndicatorShown(true);
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DataGridWidget::selectColumn(int column, Qt::SortOrder order)
{
	/* If the column was clicked but the Control key was not pressed
	 * then whe reset to sorting settings and then apply a column selection */
	clearSortColumnList();

	/* Since the sorting order was changed by clicking the column we revert the sorting
	 * order to the previous value (before the click) so the next time the user uses
	 * Ctrl+Click the sort is applied correctly based on the sort indicatior */
	results_tbw->horizontalHeader()->blockSignals(true);
	results_tbw->horizontalHeader()->setSortIndicator(column, order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder);
	results_tbw->horizontalHeader()->setSortIndicatorShown(false);
	results_tbw->horizontalHeader()->blockSignals(false);

	results_tbw->selectColumn(column);
}

void DataGridWidget::listColumns(const std::vector<attribs_map> &cols)
{
	try
	{
		col_names.clear();
		code_compl_wgt->clearCustomItems();
		ord_column_cmb->clear();

		QStringList sel_ord_cols;

		for(int row = 0; row < ord_columns_lst->count(); row++)
			sel_ord_cols.append(ord_columns_lst->item(row)->text().split(' ').at(0));

		for(auto &col : cols)
		{
			col_names.push_back(col.at(Attributes::Name));
			code_compl_wgt->insertCustomItem(col.at(Attributes::Name), {},
																			 GuiUtilsNs::getPixmap("column"));

			if(!sel_ord_cols.contains(col.at(Attributes::Name)))
				ord_column_cmb->addItem(col.at(Attributes::Name));
		}

		add_ord_col_tb->setEnabled(!col_names.isEmpty());
		emit s_filterEnabled(!col_names.isEmpty());

		filter_tbw->setEnabled(filter_enabled);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DataGridWidget::retrieveData()
{
	Connection conn_sql { conn_params };
	Catalog catalog;

	try
	{
		if(!changed_rows.empty())
		{
			Messagebox msg_box;

			msg_box.show(tr("<strong>WARNING: </strong> There are some changed rows waiting the commit! Do you really want to discard them and retrieve the data now?"),
						 Messagebox::Alert, Messagebox::YesNoButtons);

			if(msg_box.isRejected())
				return;
		}

		static const QString tmpl_query = QString("SELECT %1 FROM \"%2\".\"%3\"");
		QString	query = tmpl_query.arg("*", sch_name, tab_name),
				cnt_query = tmpl_query.arg("count(*)", sch_name, tab_name);
		ResultSet res, cnt_res;
		unsigned limit = limit_spb->value();
		std::vector<int> curr_hidden_cols;
		int col_cnt = results_tbw->horizontalHeader()->count(), row_cnt = -1;
		QDateTime start_dt = QDateTime::currentDateTime(), end_dt;

		// Storing the current hidden columns to make them hidden again after retrive data
		for(int idx = 0; idx < col_cnt; idx++)
		{
			if(results_tbw->horizontalHeader()->isSectionHidden(idx))
				curr_hidden_cols.push_back(idx);
		}

		//Building the where clause
		if(!filter_txt->toPlainText().trimmed().isEmpty())
			query += " WHERE " + filter_txt->toPlainText();

		//Building the order by clause
		if(ord_columns_lst->count() > 0)
		{
			QStringList ord_cols, col;

			query += "\n ORDER BY ";

			for(int idx = 0; idx < ord_columns_lst->count(); idx++)
			{
				col = ord_columns_lst->item(idx)->text().split(" ");
				ord_cols.push_back("\"" + col[0] + "\" " + col[1]);
			}

			query += ord_cols.join(", ");
		}

		//Building the limit clause
		if(limit > 0)
			query += QString(" LIMIT %1").arg(limit);

		qApp->setOverrideCursor(Qt::WaitCursor);

		catalog.setConnection(conn_sql);
		conn_sql.connect();
		conn_sql.executeDMLCommand(query, res);
		conn_sql.executeDMLCommand(cnt_query, cnt_res);

		retrievePKColumns(catalog);
		retrieveFKColumns(catalog);
		listColumns(catalog.getObjectsAttributes(ObjectType::Column, sch_name, tab_name));

		cnt_res.accessTuple(ResultSet::FirstTuple);
		QString val = cnt_res.getColumnValue("count");
		row_cnt = val.toInt();

		rows_cnt_lbl->setText(QString::number(row_cnt) + " " + tr("row(s)"));
		SQLExecutionWidget::fillResultsTable(catalog, res, results_tbw, true);

		end_dt = QDateTime::currentDateTime();
		qint64 total_exec = end_dt.toMSecsSinceEpoch() - start_dt.toMSecsSinceEpoch();
		QString exec_time_str = total_exec >= 1000 ? QString("%1 s").arg(total_exec/1000.0) : QString("%1 ms").arg(total_exec);

		emit s_editEnabled(!col_names.isEmpty());
		emit s_exportEnabled(results_tbw->rowCount() > 0);

		result_info_frm->setVisible(results_tbw->rowCount() > 0);
		result_info_lbl->setText(QString("<em>[%1]</em> ").arg(end_dt.toString("hh:mm:ss.zzz")) +
								 tr("Row(s) returned: <strong>%1</strong> in <em><strong>%2</strong></em> ").arg(results_tbw->rowCount()).arg(exec_time_str) +
								 tr("<em>(Limit: <strong>%1</strong> rows)</em>").arg(limit_spb->value()==0 ? tr("none") : QString::number(limit_spb->value())));

		//Reset the changed rows state
		enableRowControlButtons();
		clearChangedRows();

		//If the table is empty automatically creates a new row
		if(results_tbw->rowCount() == 0 && PhysicalTable::isPhysicalTable(obj_type))
		{
			addRow();
			updateRowOperationsInfo();
		}
		else
			results_tbw->setFocus();

		qApp->restoreOverrideCursor();

		emit s_pasteEnabled(isPasteEnabled());

		emit s_truncateEnabled(obj_type == ObjectType::Table &&
													 res.getTupleCount() > 0 &&
													 !col_names.isEmpty());

		code_compl_wgt->clearCustomItems();
		code_compl_wgt->insertCustomItems(col_names, tr("Column"), ObjectType::Column);

		columns_lst->clear();
		QListWidgetItem *item = nullptr;

		for(auto &col : col_names)
		{
			columns_lst->addItem(col);
			item = columns_lst->item(columns_lst->count() - 1);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Checked);
			item->setData(Qt::UserRole, item->checkState());
		}

		//Restoring the visibily of the columns
		results_tbw->horizontalHeader()->blockSignals(true);
		col_cnt = results_tbw->horizontalHeader()->count();

		for(int idx = 0; idx < col_cnt; idx++)
			results_tbw->horizontalHeader()->setSectionHidden(idx, false);

		for(auto &idx : curr_hidden_cols)
		{
			item = columns_lst->item(idx);
			item->setCheckState(Qt::Unchecked);
			item->setData(Qt::UserRole, item->checkState());
			results_tbw->horizontalHeader()->setSectionHidden(idx, true);
		}

		results_tbw->horizontalHeader()->blockSignals(false);

		conn_sql.close();
		catalog.closeConnection();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		conn_sql.close();
		catalog.closeConnection();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DataGridWidget::resetDataGrid()
{
	result_info_frm->setVisible(false);
	results_tbw->setRowCount(0);
	results_tbw->setColumnCount(0);
	warning_frm->setVisible(false);
	csv_load_parent->setVisible(false);
	clearChangedRows();
}

void DataGridWidget::enableRowControlButtons()
{
	QList<QTableWidgetSelectionRange> sel_ranges=results_tbw->selectedRanges();
	bool cols_selected, rows_selected;

	cols_selected = rows_selected = !sel_ranges.isEmpty();

	for(auto &sel_rng : sel_ranges)
	{
		cols_selected &= (sel_rng.columnCount() == results_tbw->columnCount());
		rows_selected &= (sel_rng.rowCount() == results_tbw->rowCount());
	}

	action_delete->setEnabled(cols_selected);
	action_duplicate->setEnabled(cols_selected);
	action_bulk_edit->setEnabled(sel_ranges.count() != 0);
	action_clear->setEnabled(sel_ranges.count() != 0);

	emit s_selectionEnabled(sel_ranges.count() != 0);

	emit s_pasteEnabled(!qApp->clipboard()->text().isEmpty() &&
											!col_names.isEmpty());

	emit s_browseEnabled((!fk_infos.empty() || !ref_fk_infos.empty()) &&
													 sel_ranges.count() == 1 && sel_ranges.at(0).rowCount() == 1);
}

void DataGridWidget::addSortColumnToList()
{
	if(ord_column_cmb->count() > 0)
	{
		ord_columns_lst->addItem(ord_column_cmb->currentText() +
														 (asc_rb->isChecked() ? " ASC" : " DESC"));

		ord_column_cmb->removeItem(ord_column_cmb->currentIndex());

		enableColumnControlButtons();
	}
}

void DataGridWidget::enableColumnControlButtons()
{
	clear_ord_cols_tb->setEnabled(ord_columns_lst->count() > 0);
	add_ord_col_tb->setEnabled(ord_column_cmb->count() > 0);
	rem_ord_col_tb->setEnabled(ord_columns_lst->currentRow() >= 0);
	move_up_tb->setEnabled(ord_columns_lst->count() > 1 && ord_columns_lst->currentRow() > 0);
	move_down_tb->setEnabled(ord_columns_lst->count() > 1 &&
													 ord_columns_lst->currentRow() >= 0 &&
													 ord_columns_lst->currentRow() <= ord_columns_lst->count() - 2);
}

void DataGridWidget::swapColumns()
{
	int curr_idx = 0, new_idx = 0;
	QStringList items;

	curr_idx = new_idx = ord_columns_lst->currentRow();

	if(sender() == move_up_tb)
		new_idx--;
	else
		new_idx++;

	for(int i = 0; i < ord_columns_lst->count(); i++)
		items.push_back(ord_columns_lst->item(i)->text());

	items.move(curr_idx, new_idx);
	ord_columns_lst->blockSignals(true);
	ord_columns_lst->clear();
	ord_columns_lst->addItems(items);
	ord_columns_lst->blockSignals(false);
	ord_columns_lst->setCurrentRow(new_idx);
}

void DataGridWidget::loadDataFromCsv(bool load_from_clipboard, bool force_csv_parsing)
{
	try
	{
		QStringList csv_cols;
		int row_id = 0, col_id = 0;
		CsvDocument csv_doc;

		qApp->setOverrideCursor(Qt::WaitCursor);
		results_tbw->setUpdatesEnabled(false);

		if(load_from_clipboard)
		{
			if(qApp->clipboard()->text().isEmpty())
				return;

			QString csv_pattern = "(%1)(.)*(%1)(%2)";
			QChar separator = QChar::Tabulation, delimiter;
			QString text = qApp->clipboard()->text();

			if(force_csv_parsing)
			{
				if(text.contains(QRegularExpression(csv_pattern.arg("\"").arg(CsvDocument::Separator))))
					delimiter = '\"';
				else if(text.contains(QRegularExpression(csv_pattern.arg("'").arg(CsvDocument::Separator))))
					delimiter='\'';

							 // If one of the patterns matched the buffer we configure the right delimiter for csv buffer
				if(!delimiter.isNull())
					separator = CsvDocument::Separator;
			}

			csv_doc = CsvLoadWidget::loadCsvFromBuffer(text, separator, delimiter, false);
		}
		else
		{
			csv_doc = csv_load_wgt->getCsvDocument();
			csv_cols = csv_doc.getColumnNames();
		}

		// If there is only one empty row in the grid, this one will
		// be removed prior the csv loading
		if(results_tbw->rowCount()==1)
		{
			bool is_empty=true;

			for(int col=0; col < results_tbw->columnCount(); col++)
			{
				if(!results_tbw->item(0, col)->text().isEmpty())
				{
					is_empty=false;
					break;
				}
			}

			if(is_empty)
				removeNewRows({0});
		}

		for(int csv_row = 0; csv_row < csv_doc.getRowCount(); csv_row++)
		{
			addRow();
			row_id = results_tbw->rowCount() - 1;

			for(int csv_col = 0; csv_col < csv_doc.getColumnCount(); csv_col++)
			{
				if(csv_col > csv_doc.getColumnCount())
					break;

				if((!load_from_clipboard && csv_load_wgt->isColumnsInFirstRow()) ||
						(load_from_clipboard && !csv_cols.isEmpty()))
				{
					//First we need to get the index of the column by its name
					col_id = col_names.indexOf(csv_cols[csv_col]);

					//If a matching column is not found we add the value at the current position
					if(col_id < 0)
						col_id = csv_col;

					if(col_id >= 0 && col_id < results_tbw->columnCount())
						results_tbw->item(row_id, col_id)->setText(csv_doc.getValue(csv_row, csv_col));
				}
				else if(csv_col < results_tbw->columnCount())
				{
					//Insert the value to the cell in order of appearance
					results_tbw->item(row_id, csv_col)->setText(csv_doc.getValue(csv_row, csv_col));
				}
			}
		}

		results_tbw->setUpdatesEnabled(true);
		updateRowOperationsInfo();
		qApp->restoreOverrideCursor();		
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DataGridWidget::removeSortColumnFromList()
{
	if(qApp->mouseButtons() == Qt::NoButton || qApp->mouseButtons() == Qt::LeftButton)
	{
		QStringList items = col_names;
		int idx = 0;

		ord_columns_lst->takeItem(ord_columns_lst->currentRow());

		while(idx < ord_columns_lst->count())
			items.removeOne(ord_columns_lst->item(idx++)->text());

		ord_column_cmb->clear();
		ord_column_cmb->addItems(items);
		enableColumnControlButtons();
	}
}

void DataGridWidget::clearSortColumnList()
{
	ord_column_cmb->clear();
	ord_column_cmb->addItems(col_names);
	ord_columns_lst->clear();

	clear_ord_cols_tb->setEnabled(false);
	move_up_tb->setEnabled(false);
	move_down_tb->setEnabled(false);
	add_ord_col_tb->setEnabled(true);
}

void DataGridWidget::changeOrderMode(QListWidgetItem *item)
{
	if(qApp->mouseButtons() == Qt::RightButton)
	{
		QStringList texts = item->text().split(" ");

		if(texts.size() > 1)
			texts[1] = (texts[1] == "ASC" ? "DESC" : "ASC");

		item->setText(texts[0] + " " + texts[1]);
	}
}

void DataGridWidget::retrievePKColumns(Catalog &catalog)
{
	try
	{
		std::vector<attribs_map> pks, columns;
		table_oid = 0;

		//Retrieving the constraints from catalog using a custom filter to select only primary keys (contype=p)
		pks = catalog.getObjectsAttributes(ObjectType::Constraint, sch_name, tab_name, {}, {{Attributes::CustomFilter, "contype='p'"}});

		warning_frm->setVisible(pks.empty());

		if(pks.empty())
			warning_lbl->setText(tr("The table does not have a primary key! Updates and deletes will be performed by considering all columns as primary key. <strong>WARNING:</strong> these operations can affect more than one row."));
		else
			table_oid = pks[0][Attributes::Table].toUInt();

		pk_col_names.clear();

		if(!pks.empty())
		{
			std::vector<unsigned> col_ids;

			for(auto &id : Catalog::parseArrayValues(pks[0][Attributes::Columns]))
				col_ids.push_back(id.toUInt());

			for(auto &col : catalog.getObjectsAttributes(ObjectType::Column, sch_name, tab_name, col_ids))
				pk_col_names.push_back(col[Attributes::Name]);
		}

		//For tables, even if there is no pk the user can manipulate data
		if(PhysicalTable::isPhysicalTable(obj_type))
			results_tbw->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
		else
			results_tbw->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DataGridWidget::retrieveFKColumns(Catalog &catalog)
{
	try
	{
		QAction *action = nullptr;
		std::vector<attribs_map> fks, ref_fks;
		QString fk_name;

		fks_menu.clear();
		ref_fk_infos.clear();
		fk_infos.clear();

		if(obj_type==ObjectType::View)
			return;

		//Retrieving the constraints from catalog using a custom filter to select only foreign keys (contype=f)
		fks = catalog.getObjectsAttributes(ObjectType::Constraint, sch_name, tab_name, {}, {{Attributes::CustomFilter, "contype='f'"}});
		ref_fks = catalog.getObjectsAttributes(ObjectType::Constraint, "", "", {}, {{Attributes::CustomFilter, QString("contype='f' AND cs.confrelid=%1").arg(table_oid)}});

		if(!fks.empty() || !ref_fks.empty())
		{
			std::vector<unsigned> col_ids;
			QMenu *submenu = nullptr;

			attribs_map aux_table, aux_schema;
			QStringList name_list;

			submenu = new QMenu(this);

			QAction *act = submenu->menuAction();
			act->setIcon(GuiUtilsNs::getIcon("referenced"));
			act->setText(tr("Referenced tables"));
			fks_menu.addAction(act);

			if(fks.empty())
				submenu->addAction(tr("(none)"))->setEnabled(false);

			for(auto &fk : fks)
			{				
				aux_table = catalog.getObjectAttributes(ObjectType::Table, fk[Attributes::RefTable].toUInt());
				aux_schema = catalog.getObjectAttributes(ObjectType::Schema, aux_table[Attributes::Schema].toUInt());
				fk_name = QString("%1.%2.%3")
									.arg(aux_schema[Attributes::Name])
									.arg(aux_table[Attributes::Name])
									.arg(fk[Attributes::Name]);

				//Store the referenced schema and table names
				fk_infos[fk_name][Attributes::RefTable] = aux_table[Attributes::Name];
				fk_infos[fk_name][Attributes::Schema] = aux_schema[Attributes::Name];
				action = submenu->addAction(GuiUtilsNs::getIcon("table"),
																		QString("%1.%2 (%3)").arg(aux_schema[Attributes::Name])
																													.arg(aux_table[Attributes::Name])
																													.arg(fk[Attributes::Name]), this, &DataGridWidget::browseReferencedTable);
				action->setData(fk_name);

				col_ids.clear();
				name_list.clear();

				//Storing the source columns in a string
				for(QString id : Catalog::parseArrayValues(fk[Attributes::SrcColumns]))
					col_ids.push_back(id.toUInt());

				for(auto &col : catalog.getObjectsAttributes(ObjectType::Column, sch_name, tab_name, col_ids))
					name_list.push_back(BaseObject::formatName(col[Attributes::Name]));

				fk_infos[fk_name][Attributes::SrcColumns] = name_list.join(UtilsNs::DataSeparator);

				col_ids.clear();
				name_list.clear();

				//Storing the referenced columns in a string
				for(auto &id : Catalog::parseArrayValues(fk[Attributes::DstColumns]))
					col_ids.push_back(id.toUInt());

				for(auto &col : catalog.getObjectsAttributes(ObjectType::Column, aux_schema[Attributes::Name], aux_table[Attributes::Name], col_ids))
					name_list.push_back(BaseObject::formatName(col[Attributes::Name]));

				fk_infos[fk_name][Attributes::DstColumns] = name_list.join(UtilsNs::DataSeparator);
			}

			submenu = new QMenu(this);

			act = submenu->menuAction();
			act->setIcon(GuiUtilsNs::getIcon("referrer"));
			act->setText(tr("Referrer tables"));
			fks_menu.addAction(act);

			if(ref_fks.empty())
				submenu->addAction(tr("(none)"))->setEnabled(false);

			for(auto &fk : ref_fks)
			{
				col_ids.clear();
				name_list.clear();

				aux_table = catalog.getObjectAttributes(ObjectType::Table, fk[Attributes::Table].toUInt());
				aux_schema = catalog.getObjectAttributes(ObjectType::Schema, aux_table[Attributes::Schema].toUInt());
				fk_name = QString("%1.%2.%3")
									.arg(aux_schema[Attributes::Name])
									.arg(aux_table[Attributes::Name])
									.arg(fk[Attributes::Name]);

				//Storing the source columns in a string
				for(auto &id : Catalog::parseArrayValues(fk[Attributes::SrcColumns]))
					col_ids.push_back(id.toUInt());

				for(auto &col : catalog.getObjectsAttributes(ObjectType::Column, aux_schema[Attributes::Name], aux_table[Attributes::Name], col_ids))
					name_list.push_back(BaseObject::formatName(col[Attributes::Name]));

				action = submenu->addAction(GuiUtilsNs::getIcon("table"),
																		QString("%1.%2 (%3)").arg(aux_schema[Attributes::Name])
																													.arg(aux_table[Attributes::Name])
																													.arg(fk[Attributes::Name]), this, &DataGridWidget::browseReferrerTable);
				action->setData(fk_name);

				ref_fk_infos[fk_name][Attributes::SrcColumns] = name_list.join(UtilsNs::DataSeparator);
				ref_fk_infos[fk_name][Attributes::Table] = aux_table[Attributes::Name];
				ref_fk_infos[fk_name][Attributes::Schema] = aux_schema[Attributes::Name];
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DataGridWidget::markOperationOnRow(OperationId operation, int row)
{
	if(row < results_tbw->rowCount() &&
			(operation == NoOperation ||
			 results_tbw->verticalHeaderItem(row)->data(Qt::UserRole) != OpInsert))
	{
		QTableWidgetItem *item = nullptr, *header_item = results_tbw->verticalHeaderItem(row);
		QString tooltip = tr("This row is marked to be %1");
		QFont fnt = results_tbw->font();
		int marked_cols = 0;
		QColor item_fg_colors[3] = {
			CustomTableWidget::getTableItemColor(CustomTableWidget::AddedItemFgColor),
			CustomTableWidget::getTableItemColor(CustomTableWidget::UpdatedItemFgColor),
			CustomTableWidget::getTableItemColor(CustomTableWidget::RemovedItemFgColor) },

			item_bg_colors[3] = {
						CustomTableWidget::getTableItemColor(CustomTableWidget::AddedItemBgColor),
						CustomTableWidget::getTableItemColor(CustomTableWidget::UpdatedItemBgColor),
						CustomTableWidget::getTableItemColor(CustomTableWidget::RemovedItemBgColor) };

		if(operation == OpDelete)
			tooltip = tooltip.arg(tr("deleted"));
		else if(operation == OpUpdate)
			tooltip = tooltip.arg(tr("updated"));
		else if(operation == OpInsert)
			tooltip = tooltip.arg(tr("inserted"));
		else
			tooltip.clear();

		results_tbw->blockSignals(true);

		for(int col = 0; col < results_tbw->columnCount(); col++)
		{
			item = results_tbw->item(row, col);

			if(results_tbw->horizontalHeaderItem(col)->data(Qt::UserRole) != "bytea")
			{
				item->setToolTip(tooltip);

				//Restore the item's font and text when the operation is delete or none
				if(operation == NoOperation || operation == OpDelete)
				{
					item->setFont(fnt);
					item->setText(item->data(Qt::UserRole).toString());
				}

				if(operation == NoOperation)
				{
					//Restore the item's colors
					item->setBackground(prev_bg_colors[row]);
					item->setForeground(prev_fg_colors[row]);
				}
				else
				{
					//Saves the item's colors if it isn't already marked
					if(header_item->data(Qt::UserRole) != OpDelete &&
							header_item->data(Qt::UserRole) != OpUpdate)
					{
						prev_bg_colors[row] = item->background();
						prev_fg_colors[row] = item->foreground();
					}

					//Changes the item's background and foreground colors according to the operation
					item->setBackground(item_bg_colors[operation - 1]);
					item->setForeground(item_fg_colors[operation - 1]);
				}

				marked_cols++;
			}
		}

		if(marked_cols > 0)
		{
			auto itr = std::find(changed_rows.begin(), changed_rows.end(), row);

			if(operation == NoOperation && itr != changed_rows.end())
			{
				changed_rows.erase(std::find(changed_rows.begin(), changed_rows.end(), row));
				prev_bg_colors.erase(row);
				prev_fg_colors.erase(row);
			}
			else if(operation!=NoOperation && itr==changed_rows.end())
				changed_rows.push_back(row);

			header_item->setData(Qt::UserRole, operation);
			std::sort(changed_rows.begin(), changed_rows.end());

			emit s_saveEnabled(!changed_rows.empty());
			emit s_undoEnabled(!changed_rows.empty());
			emit s_dataModified(!changed_rows.empty());
		}

		results_tbw->blockSignals(false);
	}
}

void DataGridWidget::markUpdateOnRow(QTableWidgetItem *item)
{
	if(results_tbw->verticalHeaderItem(item->row())->data(Qt::UserRole) != OpInsert)
	{
		bool items_changed = false;
		QTableWidgetItem *aux_item = nullptr;
		QFont fnt = item->font();

		//Before mark the row to update it's needed to check if some item was changed
		for(int col = 0; col < results_tbw->columnCount(); col++)
		{
			aux_item = results_tbw->item(item->row(), col);

			if(!items_changed && aux_item->text() != aux_item->data(Qt::UserRole))
			{
				items_changed = true;
				break;
			}
		}

		fnt.setBold(items_changed);
		fnt.setUnderline(items_changed);
		item->setFont(fnt);
		markOperationOnRow(items_changed ? OpUpdate : NoOperation, item->row());
		updateRowOperationsInfo();
	}
}

void DataGridWidget::markDeleteOnRows()
{
	QTableWidgetItem *item = nullptr;
	std::vector<int> ins_rows;

	for(auto &sel_rng : results_tbw->selectedRanges())
	{
		for(int row = sel_rng.topRow(); row <= sel_rng.bottomRow(); row++)
		{
			item = results_tbw->verticalHeaderItem(row);

			if(item->data(Qt::UserRole) == OpInsert)
				ins_rows.push_back(row);
			else
				markOperationOnRow(OpDelete, row);
		}
	}

	removeNewRows(ins_rows);
	results_tbw->clearSelection();
	updateRowOperationsInfo();
}

void DataGridWidget::addRow(bool focus_new_row)
{
	int row = results_tbw->rowCount();
	QTableWidgetItem *item = nullptr;

	results_tbw->blockSignals(true);
	results_tbw->insertRow(row);

	for(int col = 0; col < results_tbw->columnCount(); col++)
	{
		item = new QTableWidgetItem;
		item->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		results_tbw->setItem(row, col, item);
	}

	results_tbw->setVerticalHeaderItem(row, new QTableWidgetItem(QString::number(row + 1)));
	results_tbw->blockSignals(false);

	markOperationOnRow(OpInsert, row);

	item = results_tbw->item(row, 0);

	if(focus_new_row)
	{
		results_tbw->setFocus();
		results_tbw->setCurrentCell(row, 0, QItemSelectionModel::ClearAndSelect);
		results_tbw->editItem(item);
	}
}

void DataGridWidget::duplicateRows()
{
	QList<QTableWidgetSelectionRange> sel_ranges = results_tbw->selectedRanges();

	if(!sel_ranges.isEmpty())
	{
		for(auto &sel_rng : sel_ranges)
		{
			for(int row = sel_rng.topRow(); row <= sel_rng.bottomRow(); row++)
			{
				addRow(false);

				for(int col = 0; col < results_tbw->columnCount(); col++)
				{
					results_tbw->item(results_tbw->rowCount() - 1, col)
							->setText(results_tbw->item(row, col)->text());
				}
			}
		}

		results_tbw->setCurrentItem(results_tbw->item(results_tbw->rowCount() - 1, 0),
																QItemSelectionModel::ClearAndSelect);

		updateRowOperationsInfo();
	}
}

void DataGridWidget::removeNewRows(std::vector<int> ins_rows)
{
	if(ins_rows.empty())
		return;

	unsigned cnt = ins_rows.size();
	int row_idx = 0;
	std::vector<int>::reverse_iterator itr, itr_end;

	//Mark the rows as no-op to remove their indexes from changed rows set
	for(unsigned idx = 0; idx < cnt; idx++)
		markOperationOnRow(NoOperation, ins_rows[idx]);

	//Remove the rows
	std::sort(ins_rows.begin(), ins_rows.end());
	while(!ins_rows.empty())
	{
		results_tbw->removeRow(ins_rows.back());
		ins_rows.pop_back();
	}

	//Reorganizing the changed rows vector to avoid row index out-of-bound errors
	row_idx = results_tbw->rowCount() - 1;
	itr = changed_rows.rbegin();
	itr_end = changed_rows.rend();

	while(itr != itr_end)
	{
		if((*itr) > row_idx)
		{
			(*itr) = row_idx;
			results_tbw->verticalHeaderItem(row_idx)->setText(QString::number(row_idx + 1));
			row_idx--;
		}
		else break;

		itr++;
	}
}

void DataGridWidget::clearChangedRows()
{
	changed_rows.clear();
	prev_bg_colors.clear();

	undo_enabled = false;
	save_enabled = false;

	emit s_saveEnabled(false);
	emit s_undoEnabled(false);
	emit s_dataModified(false);
}

void DataGridWidget::browseTable(const QString &fk_name, bool browse_ref_tab)
{
	QString value, schema, table;
	QStringList filter, src_cols, ref_cols;

	if(browse_ref_tab)
	{
		src_cols =  pk_col_names;
		ref_cols = ref_fk_infos[fk_name][Attributes::SrcColumns].split(UtilsNs::DataSeparator);
		schema = ref_fk_infos[fk_name][Attributes::Schema];
		table = ref_fk_infos[fk_name][Attributes::Table];
	}
	else
	{
		src_cols =  fk_infos[fk_name][Attributes::SrcColumns].split(UtilsNs::DataSeparator);
		ref_cols = fk_infos[fk_name][Attributes::DstColumns].split(UtilsNs::DataSeparator);
		schema = fk_infos[fk_name][Attributes::Schema];
		table = fk_infos[fk_name][Attributes::RefTable];
	}

	for(auto &col_name : src_cols)
	{
		value = results_tbw->item(results_tbw->currentRow(), col_names.indexOf(col_name))->text();

		if(value.isEmpty())
			filter.push_back(QString("%1 IS NULL").arg(ref_cols.front()));
		else
			filter.push_back(QString("%1 = '%2'").arg(ref_cols.front(), value));

		ref_cols.pop_front();
	}

	emit s_browseTableRequested(schema, table, filter.join("AND"), ObjectType::Table);
}

void DataGridWidget::browseReferrerTable()
{
	browseTable(qobject_cast<QAction *>(sender())->data().toString(), true);
}

void DataGridWidget::browseReferencedTable()
{
	browseTable(qobject_cast<QAction *>(sender())->data().toString(), false);
}

void DataGridWidget::undoOperations()
{
	QTableWidgetItem *item = nullptr;
	std::vector<int> rows, ins_rows;
	QList<QTableWidgetSelectionRange> sel_range = results_tbw->selectedRanges();

	if(!sel_range.isEmpty())
	{
		for(int row = sel_range[0].topRow(); row <= sel_range[0].bottomRow(); row++)
		{
			if(results_tbw->verticalHeaderItem(row)->data(Qt::UserRole).toUInt() == OpInsert)
				ins_rows.push_back(row);
			else
				rows.push_back(row);
		}
	}
	else
	{
		sel_range.clear();
		rows = changed_rows;
	}

	//Marking rows to be deleted/updated as no-op
	for(auto &row : rows)
	{
		item = results_tbw->verticalHeaderItem(row);

		if(item->data(Qt::UserRole).toUInt() != OpInsert)
			markOperationOnRow(NoOperation, row);
	}

	//If there is no selection, remove all new rows
	if(sel_range.isEmpty())
	{
		if(results_tbw->rowCount() > 0 &&
				results_tbw->verticalHeaderItem(results_tbw->rowCount() -1)->data(Qt::UserRole) == OpInsert)
		{
			do
			{
				results_tbw->removeRow(results_tbw->rowCount() - 1);
				item = results_tbw->verticalHeaderItem(results_tbw->rowCount()-1);
			}
			while(item && item->data(Qt::UserRole) == OpInsert);
		}

		clearChangedRows();
	}
	else
		//Removing just the selected new rows
		removeNewRows(ins_rows);

	results_tbw->clearSelection();
	updateRowOperationsInfo();
}

void DataGridWidget::insertRowOnTabPress(int curr_row, int curr_col, int prev_row, int prev_col)
{
	if(qApp->mouseButtons() == Qt::NoButton &&
			curr_row == 0 && curr_col == 0 &&
			prev_row == results_tbw->rowCount() - 1 && prev_col == results_tbw->columnCount() - 1)
	{
		addRow();
		updateRowOperationsInfo();
	}
}

void DataGridWidget::saveChanges()
{
#ifdef DEMO_VERSION
#warning "DEMO VERSION: data manipulation save feature disabled warning."
	Messagebox msg_box;
	msg_box.show(tr("Warning"),
				 tr("You're running a demonstration version! The save feature of the data manipulation form is available only in the full version!"),
				 Messagebox::Alert, Messagebox::OkButton);
#else
	int row = 0;
	Connection conn_sql { conn_params };

	try
	{
		QString cmd;
		Messagebox msg_box;

		msg_box.show(tr("<strong>WARNING:</strong> Once commited its not possible to undo the changes! Proceed with saving?"),
								 Messagebox::Alert,
								 Messagebox::YesNoButtons);

		if(msg_box.isAccepted())
		{
			//Forcing the cell editor to be closed by selecting an unexistent cell and clearing the selection
			results_tbw->setCurrentCell(-1,-1, QItemSelectionModel::Clear);

			conn_sql.connect();
			conn_sql.executeDDLCommand("START TRANSACTION");

			for(const auto &changed_row : changed_rows)
			{
				/* We make a copy of the row id so in case of exception (see in the catch block)
				 * we can highlight the problematic row data */
				row = changed_row;
				cmd = getDMLCommand(row);
				conn_sql.executeDDLCommand(cmd);
			}

			conn_sql.executeDDLCommand("COMMIT");
			conn_sql.close();

			changed_rows.clear();
			retrieveData();

			emit s_undoEnabled(false);
			emit s_saveEnabled(false);
			emit s_dataModified(false);
		}
	}
	catch(Exception &e)
	{
		std::map<unsigned, QString> op_names={{ OpDelete, tr("delete") },
																					{ OpUpdate, tr("update") },
																					{ OpInsert, tr("insert") }};

		QString fmt_tb_name = QString("%1.%2").arg(sch_name, tab_name);
		unsigned op_type = results_tbw->verticalHeaderItem(row)->data(Qt::UserRole).toUInt();

		conn_sql.executeDDLCommand("ROLLBACK");
		conn_sql.close();

		results_tbw->selectRow(row);
		results_tbw->scrollToItem(results_tbw->item(row, 0));

		Messagebox::error(Exception::getErrorMessage(ErrorCode::RowDataNotManipulated)
											.arg(op_names[op_type]).arg(fmt_tb_name).arg(row + 1).arg(e.getErrorMessage()),
											ErrorCode::RowDataNotManipulated, PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
#endif
}

QString DataGridWidget::getDMLCommand(int row)
{
	if(row < 0 || row >= results_tbw->rowCount())
		return "";

	QString fmt_tb_name = QString("\"%1\".\"%2\"").arg(sch_name, tab_name),
			upd_cmd = QString("UPDATE %1 SET %2 WHERE %3"),
			del_cmd = QString("DELETE FROM %1 WHERE %2"),
			ins_cmd = QString("INSERT INTO %1(%2) VALUES (%3)"),
			fmt_cmd;
	QTableWidgetItem *item = nullptr;
	unsigned op_type = results_tbw->verticalHeaderItem(row)->data(Qt::UserRole).toUInt();
	QStringList val_list, col_list, flt_list;
	QString col_name, value;
	QVariant data;

	if(op_type == OpDelete || op_type == OpUpdate)
	{
		if(pk_col_names.isEmpty())
		{
			//Considering all columns as pk when the tables doesn't has one (except bytea columns)
			for(int col = 0; col < results_tbw->columnCount(); col++)
			{
				if(results_tbw->horizontalHeaderItem(col)->data(Qt::ToolTipRole) != "bytea")
					pk_col_names.push_back(results_tbw->horizontalHeaderItem(col)->data(Qt::UserRole).toString());
			}
		}

		//Creating the where clause with original column's values
		for(auto &pk_col : pk_col_names)
		{
			data = results_tbw->item(row,  col_names.indexOf(pk_col))->data(Qt::UserRole);

			if(data.toString() == SQLExecutionWidget::ColumnNullValue)
				flt_list.push_back(QString("\"%1\" IS NULL").arg(pk_col));
			else
				flt_list.push_back(QString("\"%1\"='%2'").arg(pk_col).arg(data.toString().replace("\'","''")));
		}
	}

	if(op_type == OpDelete)
	{
		fmt_cmd=QString(del_cmd).arg(fmt_tb_name).arg(flt_list.join(" AND "));
	}
	else if(op_type == OpUpdate || op_type == OpInsert)
	{
		fmt_cmd=(op_type == OpUpdate ? upd_cmd : ins_cmd);

		for(int col=0; col < results_tbw->columnCount(); col++)
		{
			item = results_tbw->item(row, col);
			value = item->text();
			col_name = results_tbw->horizontalHeaderItem(col)->data(Qt::UserRole).toString();

			if(op_type==OpInsert || (op_type==OpUpdate && value != item->data(Qt::UserRole)))
			{
				//Checking if the value is a malformed unescaped value, e.g., {value, value}, {value\}
				if((value.startsWith(UtilsNs::UnescValueStart) && value.endsWith(QString("\\") + UtilsNs::UnescValueEnd)) ||
						(value.startsWith(UtilsNs::UnescValueStart) && !value.endsWith(UtilsNs::UnescValueEnd)) ||
						(!value.startsWith(UtilsNs::UnescValueStart) && !value.endsWith(QString("\\") + UtilsNs::UnescValueEnd) && value.endsWith(UtilsNs::UnescValueEnd)))
				{
					throw Exception(Exception::getErrorMessage(ErrorCode::MalformedUnescapedValue)
													.arg(row + 1).arg(col_name),
													ErrorCode::MalformedUnescapedValue, PGM_FUNC, PGM_FILE, PGM_LINE);
				}

				col_list.push_back(QString("\"%1\"").arg(col_name));

				//Empty values as considered as DEFAULT
				if(value.isEmpty())
				{
					value = "DEFAULT";
				}
				//Unescaped values will not be enclosed in quotes
				else if(value.startsWith(UtilsNs::UnescValueStart) && value.endsWith(UtilsNs::UnescValueEnd))
				{
					value.remove(0, 1);
					value.remove(value.length()-1, 1);
				}
				//Quoting value
				else
				{
					value.replace(QString("\\") + UtilsNs::UnescValueStart, UtilsNs::UnescValueStart);
					value.replace(QString("\\") + UtilsNs::UnescValueEnd, UtilsNs::UnescValueEnd);
					value.replace("\'","''");
					value = "E'" + value + "'";
				}

				if(op_type == OpInsert)
					val_list.push_back(value);
				else
					val_list.push_back(QString("\"%1\"=%2").arg(col_name).arg(value));
			}
		}

		if(col_list.isEmpty())
			return "";

		if(op_type == OpUpdate)
			fmt_cmd = fmt_cmd.arg(fmt_tb_name).arg(val_list.join(", ")).arg(flt_list.join(" AND "));
		else
			fmt_cmd = fmt_cmd.arg(fmt_tb_name).arg(col_list.join(", ")).arg(val_list.join(", "));
	}

	return fmt_cmd;
}

void DataGridWidget::setColumnsCheckState(Qt::CheckState state)
{
	QListWidgetItem *item = nullptr;

	for(int idx = 0; idx < columns_lst->count(); idx++)
	{
		item = columns_lst->item(idx);
		item->setCheckState(state);
		toggleColumnDisplay(item);
	}
}

bool DataGridWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == columns_lst)
	{
		if((event->type() == QEvent::KeyRelease &&
				dynamic_cast<QKeyEvent *>(event)->key() == Qt::Key_Space))
		{
			toggleColumnDisplay(columns_lst->currentItem());
		}
	}

	return QWidget::eventFilter(object, event);
}

void DataGridWidget::truncateTable()
{
	try
	{
		QAction *act = dynamic_cast<QAction *>(sender());

		if(DatabaseExplorerWidget::truncateTable(sch_name, tab_name,
																						 act->data().toBool(),
																						 conn_params))
		{
			retrieveData();
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
  }
}

void DataGridWidget::toggleColumnDisplay(QListWidgetItem *item)
{
	if(!item)
		return;

	if(item->checkState() != static_cast<Qt::CheckState>(item->data(Qt::UserRole).toInt()))
	{
		int idx = 0;
		bool hide = false;

		idx = col_names.indexOf(item->text());
		hide = item->checkState() == Qt::Unchecked;
		results_tbw->horizontalHeader()->setSectionHidden(idx, hide);
		item->setCheckState(hide ? Qt::Unchecked : Qt::Checked);
		item->setData(Qt::UserRole, item->checkState());
	}
}

void DataGridWidget::showPopupMenu(const QPoint &pnt)
{
	QAction *act = nullptr;
	items_menu.clear();

	act = copy_menu.menuAction();
	act->setIcon(GuiUtilsNs::getIcon("selection"));
	act->setText(tr("Selected items"));
	items_menu.addAction(act);

	act = paste_menu.menuAction();
	act->setIcon(GuiUtilsNs::getIcon("paste"));
	act->setText(tr("Paste items"));
	act->setEnabled(isPasteEnabled());
	items_menu.addAction(act);

	act = items_menu.addAction(GuiUtilsNs::getIcon("cleartext"), tr("Clear items"),
														 this, &DataGridWidget::clearItemsText);

	act->setEnabled(!results_tbw->selectedRanges().isEmpty());

	if(obj_type == ObjectType::Table)
	{
		items_menu.addSeparator();
		act = fks_menu.menuAction();
		act->setIcon(GuiUtilsNs::getIcon("browsetable"));
		act->setText(tr("Browse tables"));
		act->setEnabled(browse_enabled);
		items_menu.addAction(act);

		items_menu.addSeparator();
		items_menu.addAction(action_duplicate);
		items_menu.addAction(action_delete);
		items_menu.addAction(action_bulk_edit);
	}

	items_menu.exec(results_tbw->viewport()->mapToGlobal(pnt));
}

void DataGridWidget::saveSelectedItems(bool csv_format)
{
	try
	{
		QByteArray buffer = csv_format ?
														SQLExecutionWidget::generateCSVBuffer(results_tbw) :
														SQLExecutionWidget::generateTextBuffer(results_tbw);

		GuiUtilsNs::selectAndSaveFile(buffer,
																	 tr("Save file"),
																	 QFileDialog::AnyFile,
																	 { csv_format ? tr("CSV file (*.csv)") :tr("Text file (*.txt)"),	tr("All files (*.*)") },
																	 {}, csv_format ? "csv" : "txt");
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DataGridWidget::toggleFilter(bool toggle)
{
	filter_tbw->setVisible(toggle);

	if(toggle)
		filter_txt->setFocus();
}

bool DataGridWidget::isFilterToggled()
{
	return filter_tbw->isVisible();
}

void DataGridWidget::toggleCsvLoader(bool toggle)
{
	csv_load_parent->setVisible(toggle);
}

bool DataGridWidget::isCsvLoaderToggled()
{
	return csv_load_parent->isVisible();
}

bool DataGridWidget::isSaveEnabled()
{
	return save_enabled;
}

bool DataGridWidget::isUndoEnabled()
{
	return undo_enabled;
}

bool DataGridWidget::isBrowseEnabled()
{
	return browse_enabled;
}

bool DataGridWidget::isSelectionEnabled()
{
	return selection_enabled;
}

bool DataGridWidget::isExportEnabled()
{
	return export_enabled;
}

bool DataGridWidget::isTruncateEnabled()
{
	return truncate_enabled;
}

bool DataGridWidget::isFilterEnabled()
{
	return filter_enabled;
}

bool DataGridWidget::isEditEnabled()
{
	return edit_enabled;
}

bool DataGridWidget::isPasteEnabled()
{
	return !qApp->clipboard()->text().isEmpty() &&
				 !col_names.isEmpty();
}

bool DataGridWidget::hasChangedRows()
{
	return !changed_rows.empty();
}

void DataGridWidget::updateRowOperationsInfo()
{
	QHeaderView *vert_header = results_tbw->verticalHeader();
	std::map<OperationId, unsigned> oper_cnt {};
	OperationId op_id;

	for(int row = 0; row < vert_header->count(); row++)
	{
		op_id = static_cast<OperationId>(results_tbw->verticalHeaderItem(row)->data(Qt::UserRole).toUInt());
		oper_cnt[op_id]++;
	}

	rows_added_lbl->setText(QString::number(oper_cnt[OpInsert]));
	rows_added_lbl->setEnabled(oper_cnt[OpInsert] > 0);
	rows_added_ico_lbl->setEnabled(rows_added_lbl->isEnabled());

	rows_changed_lbl->setText(QString::number(oper_cnt[OpUpdate]));
	rows_changed_lbl->setEnabled(oper_cnt[OpUpdate] > 0);
	rows_changed_ico_lbl->setEnabled(rows_changed_lbl->isEnabled());

	rows_deleted_lbl->setText(QString::number(oper_cnt[OpDelete]));
	rows_deleted_lbl->setEnabled(oper_cnt[OpDelete] > 0);
	rows_deleted_ico_lbl->setEnabled(rows_deleted_lbl->isEnabled());
}
