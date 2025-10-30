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

#include "datahandlingform.h"
#include "guiutilsns.h"
#include "settings/generalconfigwidget.h"
#include "messagebox.h"
#include "datagridwidget.h"

DataHandlingForm::DataHandlingForm(QWidget * parent, Qt::WindowFlags f): QDialog(parent, f)
{
	setupUi(this);
	setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

	curr_grid_wgt = nullptr;

	QToolButton *btn = nullptr;
	QFont fnt;

	for(auto &obj : btns_parent_wgt->children())
	{
		btn = dynamic_cast<QToolButton *>(obj);
		if(!btn) continue;

		fnt = btn->font();
		fnt.setWeight(QFont::Medium);
		btn->setFont(fnt);
		GuiUtilsNs::configureWidgetFont(btn, GuiUtilsNs::MediumFontFactor);
	}

	refresh_tb->setToolTip(refresh_tb->toolTip() + QString(" (%1)").arg(refresh_tb->shortcut().toString()));
	save_tb->setToolTip(save_tb->toolTip() + QString(" (%1)").arg(save_tb->shortcut().toString()));
	export_tb->setToolTip(export_tb->toolTip() + QString(" (%1)").arg(export_tb->shortcut().toString()));
	undo_tb->setToolTip(undo_tb->toolTip() + QString(" (%1)").arg(undo_tb->shortcut().toString()));
	csv_load_tb->setToolTip(csv_load_tb->toolTip() + QString(" (%1)").arg(csv_load_tb->shortcut().toString()));
	filter_tb->setToolTip(filter_tb->toolTip() + QString(" (%1)").arg(filter_tb->shortcut().toString()));
	new_window_tb->setToolTip(new_window_tb->toolTip() + QString(" (%1)").arg(new_window_tb->shortcut().toString()));

	connect(data_grids_tbw, &QTabWidget::currentChanged, this, &DataHandlingForm::setCurrentDataGrid);

	connect(data_grids_tbw, &QTabWidget::tabCloseRequested, this, [this](int idx){
		closeDataGrid(idx, true);
	});

	connect(close_btn, &QPushButton::clicked, this, &DataHandlingForm::reject);
	connect(schema_cmb, &QComboBox::currentIndexChanged, this, &DataHandlingForm::listTables);
	connect(hide_views_chk, &QCheckBox::toggled, this, &DataHandlingForm::listTables);
	connect(schema_cmb, &QComboBox::currentIndexChanged, this, &DataHandlingForm::enableRefreshButton);
	connect(table_cmb, &QComboBox::currentIndexChanged, this, &DataHandlingForm::enableRefreshButton);

	auto add_data_grid_lmb = [this](){
		addDataGrid(schema_cmb->currentText(), table_cmb->currentText(), "",
								static_cast<ObjectType>(table_cmb->currentData().toUInt()));
	};

	connect(table_cmb, &QComboBox::activated, this, add_data_grid_lmb);
	connect(browse_data_tb, &QToolButton::clicked, this, add_data_grid_lmb);

	connect(new_window_tb, &QToolButton::clicked, this, [this](){
			openNewWindow(tmpl_conn_params);
	});
}

DataHandlingForm::~DataHandlingForm()
{
	while(data_grids_tbw->count() > 0)
	{
		data_grids_tbw->blockSignals(true);
		closeDataGrid(0, false);
	}
}

void DataHandlingForm::setAttributes(const attribs_map &conn_params, const QString curr_schema,
																		 const QString curr_table, ObjectType obj_type)
{
	if(!BaseTable::isBaseTable(obj_type))
	{
		Messagebox::error(ErrorCode::OprObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);
		return;
	}

	try
	{
		Connection conn { conn_params };

		connection_id = conn.getConnectionId(true, true, false);
		tmpl_conn_params = conn_params;
		db_name_lbl->setText(conn.getConnectionId(true, true, true));

		schema_cmb->clear();
		listObjects(schema_cmb, { ObjectType::Schema });

		enableRefreshButton();
		schema_cmb->setCurrentText(curr_schema);

		if(!curr_schema.isEmpty() && !curr_table.isEmpty())
		{
			table_cmb->blockSignals(true);
			table_cmb->setCurrentText(curr_table);
			table_cmb->blockSignals(false);
			addDataGrid(curr_schema, curr_table, "", obj_type);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DataHandlingForm::setCurrentDataGrid(int tab_idx)
{
	DataGridWidget *data_grid_wgt = qobject_cast<DataGridWidget *>(data_grids_tbw->currentWidget());
	bool enable_btns = tab_idx >= 0;

	if(curr_grid_wgt)
	{
		browse_tabs_tb->menu()->removeEventFilter(this);
		edit_tb->menu()->removeEventFilter(this);
		paste_tb->menu()->removeEventFilter(this);
		truncate_tb->menu()->removeEventFilter(this);
		export_tb->menu()->removeEventFilter(this);
		selection_tb->menu()->removeEventFilter(this);

		disconnect(filter_tb, nullptr, curr_grid_wgt, nullptr);
		disconnect(csv_load_tb, nullptr, curr_grid_wgt, nullptr);
		disconnect(refresh_tb, nullptr, curr_grid_wgt, nullptr);
		disconnect(save_tb, nullptr, curr_grid_wgt, nullptr);
		disconnect(undo_tb, nullptr, curr_grid_wgt, nullptr);

		disconnect(curr_grid_wgt, nullptr, save_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, undo_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, selection_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, browse_tabs_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, export_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, truncate_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, edit_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, csv_load_tb, nullptr);
		disconnect(curr_grid_wgt, nullptr, filter_tb, nullptr);

		disconnect(curr_grid_wgt, nullptr, this, nullptr);
	}

	refresh_tb->setEnabled(enable_btns);

	if(data_grid_wgt)
	{
		save_tb->setEnabled(data_grid_wgt->isSaveEnabled());
		undo_tb->setEnabled(data_grid_wgt->isUndoEnabled());

		filter_tb->setEnabled(data_grid_wgt->isFilterEnabled());
		filter_tb->setChecked(data_grid_wgt->isFilterToggled());

		csv_load_tb->setEnabled(data_grid_wgt->isEditEnabled());
		csv_load_tb->setChecked(data_grid_wgt->isCsvLoaderToggled());

		browse_tabs_tb->setEnabled(data_grid_wgt->isBrowseEnabled());
		browse_tabs_tb->setMenu(&data_grid_wgt->fks_menu);
		browse_tabs_tb->menu()->installEventFilter(this);

		edit_tb->setEnabled(data_grid_wgt->isEditEnabled());
		edit_tb->setMenu(&data_grid_wgt->edit_menu);
		edit_tb->menu()->installEventFilter(this);

		paste_tb->setEnabled(data_grid_wgt->isPasteEnabled());
		paste_tb->setMenu(&data_grid_wgt->paste_menu);
		paste_tb->menu()->installEventFilter(this);

		truncate_tb->setEnabled(data_grid_wgt->isTruncateEnabled());
		truncate_tb->setMenu(&data_grid_wgt->truncate_menu);
		truncate_tb->menu()->installEventFilter(this);

		export_tb->setEnabled(data_grid_wgt->isExportEnabled());
		export_tb->setMenu(&data_grid_wgt->export_menu);
		export_tb->menu()->installEventFilter(this);

		selection_tb->setEnabled(data_grid_wgt->isSelectionEnabled());
		selection_tb->setMenu(&data_grid_wgt->copy_menu);
		selection_tb->menu()->installEventFilter(this);

		connect(filter_tb, &QToolButton::toggled, data_grid_wgt, &DataGridWidget::toggleFilter);
		connect(csv_load_tb, &QToolButton::toggled, data_grid_wgt, &DataGridWidget::toggleCsvLoader);
		connect(refresh_tb, &QToolButton::clicked, data_grid_wgt, &DataGridWidget::retrieveData);
		connect(save_tb, &QToolButton::clicked, data_grid_wgt, &DataGridWidget::saveChanges);
		connect(undo_tb, &QToolButton::clicked, data_grid_wgt, &DataGridWidget::undoOperations);

		connect(data_grid_wgt, &DataGridWidget::s_saveEnabled, save_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_undoEnabled, undo_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_selectionEnabled, selection_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_browseEnabled, browse_tabs_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_exportEnabled, export_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_truncateEnabled, truncate_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_editEnabled, edit_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_editEnabled, csv_load_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_filterEnabled, filter_tb, &QToolButton::setEnabled);
		connect(data_grid_wgt, &DataGridWidget::s_pasteEnabled, paste_tb, &QToolButton::setEnabled);

		connect(data_grid_wgt, &DataGridWidget::s_dataModified, this, &DataHandlingForm::setDataGridModified);
		connect(data_grid_wgt, &DataGridWidget::s_gridDuplicationRequested, this, &DataHandlingForm::duplicateDataGrid);

		connect(data_grid_wgt, &DataGridWidget::s_browseTableRequested, this,
						qOverload<const QString &, const QString &, const QString &, ObjectType>(&DataHandlingForm::addDataGrid));

		curr_grid_wgt = data_grid_wgt;
	}

	if(curr_grid_wgt)
	{
		setWindowTitle(tr("Data handling") +
									 QString(" - %1 / %2").arg(curr_grid_wgt->objectName(), connection_id));
	}
	else
		setWindowTitle(tr("Data handling"));
}

void DataHandlingForm::setDataGridModified(bool modified)
{
	DataGridWidget *data_grid_wgt = qobject_cast<DataGridWidget *>(sender());
	int idx = data_grids_tbw->indexOf(data_grid_wgt);

	if(idx < 0)
		return;

	QString tab_txt = data_grids_tbw->tabText(idx);
	static const QString mod_str { " *" };

	if(modified && !tab_txt.endsWith(mod_str))
		tab_txt.append(mod_str);
	else if(!modified && tab_txt.endsWith(mod_str))
		tab_txt.remove(mod_str);

	data_grids_tbw->setTabText(idx, tab_txt);
}

void DataHandlingForm::closeDataGrid(int idx, bool confirm_close)
{
	if(confirm_close)
	{
		auto [ msg_displayed , msg_res ] = confirmDataGridClose(idx);

		if(msg_displayed && msg_res == QDialog::Rejected)
			return;
	}

	DataGridWidget *data_grid_wgt = qobject_cast<DataGridWidget *>(data_grids_tbw->widget(idx));

	curr_grid_wgt = nullptr;
	data_grids_tbw->removeTab(idx);

	delete(data_grid_wgt);

	if(data_grids_tbw->count() == 0)
	{
		filter_tb->setEnabled(false);
		filter_tb->setChecked(false);
		refresh_tb->setEnabled(false);
		edit_tb->setEnabled(false);
		save_tb->setEnabled(false);
		selection_tb->setEnabled(false);
		paste_tb->setEnabled(false);
		undo_tb->setEnabled(false);
		truncate_tb->setEnabled(false);
		browse_tabs_tb->setEnabled(false);
		export_tb->setEnabled(false);
		csv_load_tb->setEnabled(false);
		csv_load_tb->setChecked(false);
	}
}

DataGridWidget *DataHandlingForm::addDataGrid(const QString &schema, const QString &table, const QString &filter, ObjectType obj_type)
{
	return addDataGrid(schema, table, filter, obj_type, true);
}

DataGridWidget *DataHandlingForm::addDataGrid(const QString &schema, const QString &table, const QString &filter, ObjectType obj_type, bool retrieve_data)
{
	if(table_cmb->currentIndex() <= 0)
		return nullptr;

	DataGridWidget *data_grid_wgt = new DataGridWidget(schema, table, obj_type, tmpl_conn_params);
	QString data_grid_name = schema + "." + table;

	data_grid_wgt->setObjectName(data_grid_name);
	int tab_idx = data_grids_tbw->addTab(data_grid_wgt,
																			 QIcon(GuiUtilsNs::getIconPath(obj_type)),
																			 table);

	data_grids_tbw->setCurrentIndex(tab_idx);

	try
	{	
		data_grid_wgt->filter_txt->setPlainText(filter);

		if(retrieve_data)
			data_grid_wgt->retrieveData();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	return data_grid_wgt;
}

void DataHandlingForm::duplicateDataGrid(DataGridWidget *grid)
{
	bool toggle_filter = grid->isFilterToggled();
	DataGridWidget *data_grid_wgt =
			addDataGrid(grid->sch_name, grid->tab_name,
									grid->filter_txt->toPlainText(),
									grid->obj_type, false);

	if(data_grid_wgt)
	{
		// Setting the filter tab visibility
		filter_tb->setChecked(toggle_filter);
		data_grid_wgt->limit_spb->setValue(grid->limit_spb->value());

		// Setting the same order by columns
		for(int row = 0; row < grid->ord_columns_lst->count(); row++)
			data_grid_wgt->ord_columns_lst->addItem(grid->ord_columns_lst->item(row)->text());

		data_grid_wgt->asc_rb->setChecked(grid->asc_rb->isChecked());
		data_grid_wgt->desc_rb->setChecked(grid->desc_rb->isChecked());

		try
		{
			QListWidgetItem *orig_item = nullptr, *new_item = nullptr;
			data_grid_wgt->retrieveData();

			/* After retrieving the data of the duplicate data grid we
			 * need to replicate the selected/visible columns */
			for(int row = 0; row < grid->columns_lst->count(); row++)
			{
				orig_item = grid->columns_lst->item(row);
				new_item = data_grid_wgt->columns_lst->item(row);

				/* Ignoring null items or items that has different texts
				 * This may happen if the table structure is changed before
				 * the grid is duplicated */
				if((!orig_item || !new_item) ||
					 (orig_item->text() != new_item->text()))
					continue;

				new_item->setCheckState(orig_item->checkState());
				data_grid_wgt->toggleColumnDisplay(new_item);
			}
		}
		catch(Exception &e)
		{
			Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
		}
	}
}

void DataHandlingForm::listTables()
{
	try
	{
		table_cmb->clear();

		if(schema_cmb->currentIndex() > 0)
		{
			std::vector<ObjectType> types = { ObjectType::Table, ObjectType::ForeignTable };

			if(!hide_views_chk->isChecked())
				types.push_back(ObjectType::View);

			listObjects(table_cmb, types, schema_cmb->currentText());
		}

		table_cmb->setEnabled(table_cmb->count() > 0);
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DataHandlingForm::enableRefreshButton()
{
	bool enable = schema_cmb->currentIndex() > 0 &&
								table_cmb->currentIndex() > 0;

	refresh_tb->setEnabled(enable);
	browse_data_tb->setEnabled(enable);
}

void DataHandlingForm::listObjects(QComboBox *combo, std::vector<ObjectType> obj_types, const QString &schema)
{
	Connection conn { tmpl_conn_params };
	Catalog catalog;

	try
	{
		attribs_map objects;
		QStringList items;
		int idx = 0, count = 0;

		qApp->setOverrideCursor(Qt::WaitCursor);

		catalog.setConnection(conn);
		catalog.setQueryFilter(Catalog::ListAllObjects);

		combo->blockSignals(true);
		combo->clear();

		for(auto &obj_type : obj_types)
		{
			objects = catalog.getObjectsNames(obj_type, schema);

			for(auto &attr : objects)
				items.push_back(attr.second);

			items.sort();
			combo->addItems(items);
			count += items.size();
			items.clear();

			for(; idx < count; idx++)
			{
				combo->setItemIcon(idx, QPixmap(GuiUtilsNs::getIconPath(obj_type)));
				combo->setItemData(idx, enum_t(obj_type));
			}

			idx=count;
		}

		if(combo->count()==0)
			combo->insertItem(0, tr("No objects found"));
		else
			combo->insertItem(0, tr("Found %1 object(s)").arg(combo->count()));

		combo->setCurrentIndex(0);
		combo->blockSignals(false);

		qApp->restoreOverrideCursor();
		catalog.closeConnection();
	}
	catch(Exception &e)
	{
		catalog.closeConnection();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DataHandlingForm::resizeEvent(QResizeEvent *event)
{
	Qt::ToolButtonStyle style = Qt::ToolButtonIconOnly;
	QToolButton *btn = nullptr;
	QSize screen_sz = this->screen()->size();

	// If the new dialog height is greater than 60% of the screen height we hide the toolbuttons texts
	if(event->size().height() > screen_sz.height() * 0.70)
		style = Qt::ToolButtonTextUnderIcon;

	if(refresh_tb->toolButtonStyle() != style)
	{
		for(auto obj : btns_parent_wgt->children())
		{
			btn = qobject_cast<QToolButton *>(obj);

			if(btn)
				btn->setToolButtonStyle(style);
		}
	}
}

std::pair<bool, int> DataHandlingForm::confirmDataGridClose(int idx)
{
	DataGridWidget *data_grid_wgt = qobject_cast<DataGridWidget *>(data_grids_tbw->widget(idx));

	if(data_grid_wgt && data_grid_wgt->hasChangedRows())
	{
		Messagebox msgbox;

		data_grids_tbw->setCurrentIndex(idx);

		msgbox.show(tr("<strong>WARNING: </strong> The table <strong>%1</strong> is modified but the changes are not yet saved! Do you really want to close and discard the pending operations?").arg(data_grid_wgt->objectName()),
								Messagebox::Alert, Messagebox::YesNoButtons);

		return { true, msgbox.result() };
	}

	return { false, QDialog::Rejected };
}

int DataHandlingForm::confirmFormClose()
{
	if(data_grids_tbw->count() == 0)
		return QDialog::Accepted;

	for(int idx = 0; idx < data_grids_tbw->count(); idx++)
	{
		auto [ msg_displayed, msg_res ] = confirmDataGridClose(idx);

		if(msg_displayed && msg_res == QDialog::Accepted)
			return QDialog::Accepted;

		if(msg_displayed)
			return QDialog::Rejected;
	}

	return Messagebox::confirm(tr("Do you really want to close the data handling form?"));
}

void DataHandlingForm::closeEvent(QCloseEvent *event)
{
	if(confirmFormClose() == QDialog::Rejected)
	{
		event->ignore();
		return;
	}

	GeneralConfigWidget::saveWidgetGeometry(this);
}

void DataHandlingForm::reject()
{
	if(confirmFormClose() == QDialog::Rejected)
		return;

	GeneralConfigWidget::saveWidgetGeometry(this);

	QDialog::reject();
}

bool DataHandlingForm::eventFilter(QObject *object, QEvent *event)
{
	/* We override the menu position only if it's being triggered by their
	 * parent buttons. If the menu is being displayed from the items context menu
	 * the default position is preserved */
	if(event->type() == QEvent::Show &&
		 object->metaObject()->className() == QString("QMenu"))
	{
		QMenu *menu = dynamic_cast<QMenu *>(object);
		QWidget *btn = btns_parent_wgt->childAt(btns_parent_wgt->mapFromGlobal(QCursor::pos()));

		/* Sometime the button can be null indicating that the menu was called by right clicking
		 * in the data grid items. In that case, we just ignore skip the menu position adjustment */
		if(!btn)
			return false;

		/* Since the menus in this form have no parent we calculate their new position
		 * taking into account the position of the button associated to the menu as
		 * well as the height of the window title bar */
		menu->move(this->pos().x() + btn->pos().x() + btn->width(),
							 this->pos().y() + btn->pos().y() +
							 qApp->style()->pixelMetric(QStyle::PM_TitleBarHeight) +
							 GuiUtilsNs::LtSpacing);

		return false;
	}

	return QDialog::eventFilter(object, event);
}

void DataHandlingForm::openNewWindow(const attribs_map &conn_params, const QString &schema, const QString &table, ObjectType obj_type)
{
	if(!BaseTable::isBaseTable(obj_type))
	{
		Messagebox::error(ErrorCode::OprObjectInvalidType, PGM_FUNC, PGM_FILE, PGM_LINE);
		return;
	}

	try
	{
		DataHandlingForm *data_hand = new DataHandlingForm;

		data_hand->setWindowModality(Qt::NonModal);
		data_hand->setAttribute(Qt::WA_DeleteOnClose, true);
		data_hand->hide_views_chk->setChecked(obj_type != ObjectType::View);
		data_hand->setAttributes(conn_params, schema, table, obj_type);

		GuiUtilsNs::resizeWidget(data_hand);
		GeneralConfigWidget::restoreWidgetGeometry(data_hand);
		data_hand->show();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}
