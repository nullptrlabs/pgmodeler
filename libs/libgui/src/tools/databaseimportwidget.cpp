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

#include "databaseimportwidget.h"
#include "customuistyle.h"
#include "widgets/taskprogresswidget.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include "defaultlanguages.h"
#include "settings/connectionsconfigwidget.h"
#include "settings/generalconfigwidget.h"
#include "objectslistmodel.h"

bool DatabaseImportWidget::low_verbosity {false};

DatabaseImportWidget::DatabaseImportWidget(QWidget *parent) : QWidget(parent)
{
	std::random_device rand_seed;
	rand_num_engine.seed(rand_seed());

	setupUi(this);

	model_wgt = nullptr;
	create_model = true;
	scene_size_incr = 0;
	import_helper = nullptr;
	import_thread = nullptr;

	GuiUtilsNs::configureWidgetsFont({ import_btn, cancel_btn }, GuiUtilsNs::BigFontFactor);

	model_sel_wgt = new ModelDbSelectorWidget(this);
	model_sel_wgt->setEnabled(false);
	import_to_model_lt->insertWidget(0, model_sel_wgt);

	pg_version_alert_frm->setVisible(false);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, pg_version_alert_frm);
	tree_filter_wgt->setVisible(false);

	objs_filter_wgt = new ObjectsFilterWidget(this);
	QVBoxLayout *vbox = new QVBoxLayout(filter_gb);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(objs_filter_wgt);

	htmlitem_del=new HtmlItemDelegate(this);
	output_trw->setItemDelegateForColumn(0, htmlitem_del);

	dbg_output_wgt = new DebugOutputWidget(this);
	vbox = new QVBoxLayout(settings_tbw->widget(2));
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(dbg_output_wgt);

	settings_tbw->setTabEnabled(1, false);
	settings_tbw->setTabVisible(2, false);

	objs_parent_wgt->setEnabled(false);
	buttons_wgt->setEnabled(false);

	splitter->setSizes({ 500, 500 });
	h_splitter->setSizes({800, 700});

	connect(by_oid_chk,  &QCheckBox::toggled, this, qOverload<>(&DatabaseImportWidget::filterObjects));

	connect(connections_cmb, &QComboBox::activated, this, [this](){
		__trycatch( listDatabases(); )
	});

	connect(database_cmb, &QComboBox::activated, this, [this](){
		__trycatch( listObjects(); )
	});

	connect(database_cmb, &QComboBox::currentIndexChanged, this, [this](int idx){
		enableImportControls(idx > 0);
	});

	connect(import_sys_objs_chk, &QCheckBox::clicked, this, [this](){
		__trycatch( listObjects(); )
	});

	connect(import_ext_objs_chk, &QCheckBox::clicked, this, [this](){
		__trycatch( listObjects(); )
	});

	connect(tree_filter_tb, &QToolButton::toggled, tree_filter_wgt, &QWidget::setVisible);
	connect(db_objects_tw, &QTreeWidget::itemChanged, this, qOverload<QTreeWidgetItem *, int>(&DatabaseImportWidget::setItemCheckState));
	connect(select_all_tb, &QToolButton::clicked, this, &DatabaseImportWidget::setItemsCheckState);
	connect(clear_all_tb, &QToolButton::clicked, this, &DatabaseImportWidget::setItemsCheckState);
	connect(filter_edt, &QLineEdit::textChanged, this, qOverload<>(&DatabaseImportWidget::filterObjects));
	connect(cancel_btn, &QPushButton::clicked, this, &DatabaseImportWidget::cancelImport);

	connect(import_btn, &QPushButton::clicked, this, __slot(this, DatabaseImportWidget::importDatabase));

	connect(objs_filter_wgt, &ObjectsFilterWidget::s_filterApplyingRequested, this, [this](){
		__trycatch( listObjects(); )
	});

	connect(expand_all_tb, &QToolButton::clicked,  this, [this](){
		db_objects_tw->blockSignals(true);
		db_objects_tw->expandAll();
		db_objects_tw->blockSignals(false);
		db_objects_tw->resizeColumnToContents(0);
	});

	connect(collapse_all_tb, &QToolButton::clicked,  this, [this](){
		db_objects_tw->blockSignals(true);
		db_objects_tw->collapseAll();
		db_objects_tw->blockSignals(false);
		db_objects_tw->resizeColumnToContents(0);
	});

	connect(db_objects_tw, &QTreeWidget::itemCollapsed, this, [this](){
		db_objects_tw->resizeColumnToContents(0);
	});

	connect(db_objects_tw, &QTreeWidget::itemExpanded, this, [this](){
		db_objects_tw->resizeColumnToContents(0);
	});

	connect(objs_filter_wgt, &ObjectsFilterWidget::s_filtersRemoved, this, [this](){
		__trycatch( listObjects(); )
	});

	connect(import_to_model_chk, &QCheckBox::toggled, this, [this](bool checked){
		create_model = !checked;
		rand_obj_pos_chk->setEnabled(checked);
		model_sel_wgt->setEnabled(checked);

		if(!checked)
		{
			model_sel_wgt->clearSelection();
			rand_obj_pos_chk->setChecked(false);
		}

		enableImport();
	});

	connect(rand_obj_pos_chk, &QCheckBox::toggled, this, [this](bool checked){
		scattering_lbl->setEnabled(checked);
		scattering_lvl_cmb->setEnabled(checked);
	});

	connect(debug_mode_chk, &QCheckBox::toggled, this, [this](bool checked){
		settings_tbw->setTabVisible(2, checked);
		dbg_output_wgt->setLogMessages(checked);
	});

	connect(import_to_model_chk, &QCheckBox::toggled, this, [this](bool checked){
		ignore_errors_chk->setChecked(checked);
		ignore_errors_chk->setDisabled(checked);
	});

	connect(model_sel_wgt, &ModelDbSelectorWidget::s_selectionChanged, this, [this]() {
		model_wgt = model_sel_wgt->getSelectedModel();
		enableImport();
	});

#ifdef DEMO_VERSION
	#warning "DEMO VERSION: forcing ignore errors in reverse engineering."
	ignore_errors_chk->setChecked(true);
	ignore_errors_chk->setEnabled(false);
#endif
}

DatabaseImportWidget::~DatabaseImportWidget()
{
	destroyThread();
}

void DatabaseImportWidget::updateModels(const QList<ModelWidget *> &models)
{
	model_sel_wgt->updateModels(models);
	import_to_model_chk->setDisabled(models.isEmpty());

	if(models.isEmpty())
		import_to_model_chk->setChecked(false);
}

void DatabaseImportWidget::enableImport()
{
	import_btn->setEnabled(hasObjectsToImport() &&
												 (model_wgt || !import_to_model_chk->isChecked()));
}

void DatabaseImportWidget::setLowVerbosity(bool value)
{
	low_verbosity = value;
}

void DatabaseImportWidget::createThread()
{
	import_thread = new QThread;

	import_helper = new DatabaseImportHelper;
	import_helper->moveToThread(import_thread);

	connect(import_thread, &QThread::started, this, [this](){
		output_trw->setUniformRowHeights(true);
	});

	connect(import_thread, &QThread::finished, this, [this](){
		output_trw->setUniformRowHeights(false);
	});

	connect(import_thread, &QThread::started, import_helper, [this](){
		__trycatch( import_helper->importDatabase(); )
	});

	connect(import_helper, &DatabaseImportHelper::s_importCanceled, this, &DatabaseImportWidget::handleImportCanceled);
	connect(import_helper, &DatabaseImportHelper::s_importFinished, this, &DatabaseImportWidget::handleImportFinished);
	connect(import_helper, &DatabaseImportHelper::s_importAborted, this, &DatabaseImportWidget::captureThreadError);
	connect(import_helper, &DatabaseImportHelper::s_progressUpdated, this, &DatabaseImportWidget::updateProgress, Qt::BlockingQueuedConnection);
}

void DatabaseImportWidget::destroyThread()
{
	if(import_thread)
	{
		import_thread->quit();
		import_thread->wait();
		delete import_thread;
		import_thread = nullptr;
		delete import_helper;
		import_helper = nullptr;
	}
}

void DatabaseImportWidget::listFilteredObjects()
{
	listFilteredObjects(*import_helper, filtered_objs_view);
}

void DatabaseImportWidget::listFilteredObjects(DatabaseImportHelper &import_hlp, QTableView *flt_objects_view)
{
	if(!flt_objects_view)
		return;

	std::vector<ObjectType> types = import_hlp.getCatalog().getFilteredObjectTypes();
	std::vector<attribs_map> obj_attrs;

	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);
		obj_attrs = import_hlp.getObjects(types);
		GuiUtilsNs::populateObjectsTable(flt_objects_view, obj_attrs);
		flt_objects_view->setEnabled(flt_objects_view->model() && flt_objects_view->model()->rowCount() > 0);
		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseImportWidget::updateProgress(int progress, QString msg, ObjectType obj_type)
{
	QPixmap ico;

	msg=UtilsNs::formatMessage(msg);
	progress_lbl->setText(msg);
	progress_pb->setValue(progress);

	if(obj_type!=ObjectType::BaseObject)
		ico=QPixmap(GuiUtilsNs::getIconPath(obj_type));
	else
		ico=QPixmap(GuiUtilsNs::getIconPath("info"));

	ico_lbl->setPixmap(ico);

	if(!low_verbosity)
		GuiUtilsNs::createOutputTreeItem(output_trw, msg, ico);
}

void DatabaseImportWidget::setItemCheckState(QTreeWidgetItem *item, int)
{
	db_objects_tw->blockSignals(true);
	setItemCheckState(item, item->checkState(0));
	setParentItemChecked(item->parent());
	enableImport();
	db_objects_tw->blockSignals(false);
}

void DatabaseImportWidget::setItemsCheckState()
{
	Qt::CheckState chk_state=(sender()==select_all_tb ? Qt::Checked : Qt::Unchecked);

	if(db_objects_stw->currentIndex() == 0)
	{
		QTreeWidgetItemIterator itr(db_objects_tw);

		db_objects_tw->blockSignals(true);
		while(*itr)
		{
			if(!(*itr)->isDisabled())
				(*itr)->setCheckState(0, chk_state);

			++itr;
		}
		db_objects_tw->blockSignals(false);
	}

	enableImport();
}

void DatabaseImportWidget::setObjectPosition(BaseGraphicObject *graph_obj)
{
	if(rand_obj_pos_chk->isChecked() && BaseTable::isBaseTable(graph_obj->getObjectType()))
	{
		QRectF rect;
		double factor = 1, scatter_lvl = 0.20;

		if(scattering_lvl_cmb->currentIndex() == 1)
			scatter_lvl = 0.50;
		else if(scattering_lvl_cmb->currentIndex() == 2)
			scatter_lvl = 0.80;

		rect = model_wgt->getObjectsScene()->sceneRect();
		rect.setHeight(rect.height() + 50);

		factor += scene_size_incr;
		scene_size_incr += (0.0125 * (scattering_lvl_cmb->currentIndex() + 1));

		BaseObjectView *obj_view = dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

		std::uniform_int_distribution<int>
				dist_x(rect.left(),
							rect.right() + ((rect.width() * scatter_lvl) * factor)),

				dist_y(rect.bottom(), rect.bottom() +
							 ((rect.height() * scatter_lvl) * factor));

		/* The new position's X coordinate will be between the rectangle's left and right with an additional value
		 * to allow the canvas to grow horizontally.
		 *
		 * The new position's Y coordinate will receive a value that start below the canvas rectangle's bottom
		 * to grow vertically without colliding with the existing objects. */
		obj_view->setPos(QPointF(dist_x(rand_num_engine), dist_y(rand_num_engine)));
	}
}

void DatabaseImportWidget::importDatabase()
{
	try
	{
		std::map<ObjectType, std::vector<unsigned>> obj_oids;
		std::map<unsigned, std::vector<unsigned>> col_oids;

		if(import_to_model_chk->isChecked())
		{
			Messagebox msg_box;

			msg_box.show(tr("<strong>ATTENTION:</strong> You are about to import objects to the current working model! This action will cause irreversible changes to it even in case of critical errors during the process. Do you want to proceed?"),
						 Messagebox::Alert, Messagebox::YesNoButtons);

			if(msg_box.isRejected())
				return;
		}

		output_trw->clear();
		settings_tbw->setTabEnabled(1, true);
		settings_tbw->setCurrentIndex(1);

		dbg_output_wgt->showActionButtons(false);
		dbg_output_wgt->clear();

		if(low_verbosity)
			GuiUtilsNs::createOutputTreeItem(output_trw, tr("<strong>Low verbosity is set:</strong> only key informations and errors will be displayed."),
																					QPixmap(GuiUtilsNs::getIconPath("alert")), nullptr, false);

		getObjectToImport(obj_oids, col_oids);
		obj_oids[ObjectType::Database].push_back(database_cmb->itemData(database_cmb->currentIndex()).value<unsigned>());

		if(create_model)
		{
			model_wgt = new ModelWidget;
			model_wgt->getDatabaseModel()->createSystemObjects(true);
			model_wgt->updateSceneLayers();
		}
		// If importing to working model
		else
		{
			// We temporarily disable interaction over it while import is running
			model_wgt->setInteractive(false);

			if(rand_obj_pos_chk->isChecked())
				connect(model_wgt, &ModelWidget::s_objectAdded, this, &DatabaseImportWidget::setObjectPosition);
		}

		import_helper->setImportOptions(import_sys_objs_chk->isChecked(), import_ext_objs_chk->isChecked(),
																		resolve_deps_chk->isChecked(), ignore_errors_chk->isChecked(),
																		debug_mode_chk->isChecked(), rand_rel_color_chk->isChecked(), true,
																		comments_as_aliases_chk->isChecked(), import_to_model_chk->isChecked());

		import_helper->setSelectedOIDs(model_wgt->getDatabaseModel(), obj_oids, col_oids);

		import_thread->start();
		cancel_btn->setEnabled(true);
		import_btn->setEnabled(false);
		database_gb->setEnabled(false);
		options_gb->setEnabled(false);
		filter_gb->setEnabled(false);

		emit s_importStarted();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::setItemCheckState(QTreeWidgetItem *item, Qt::CheckState chk_state)
{
	for(int i=0; i < item->childCount(); i++)
	{
		if(!item->child(i)->isDisabled() && !item->child(i)->isHidden())
			item->child(i)->setCheckState(0, chk_state);

		setItemCheckState(item->child(i), chk_state);
	}
}

void DatabaseImportWidget::setParentItemChecked(QTreeWidgetItem *item)
{
	if(item && !item->isDisabled())
	{
		if(item->checkState(0)!=Qt::Checked)
			item->setCheckState(0, Qt::Checked);

		setParentItemChecked(item->parent());
	}
}

bool DatabaseImportWidget::hasObjectsToImport()
{
	bool selected = false;

	if(db_objects_stw->currentIndex() == 0)
	{
		QTreeWidgetItemIterator itr(db_objects_tw);

		while(*itr && !selected)
		{
			//Only valid items (OID > 0) and with Checked state are considered as selected
			selected = ((*itr)->checkState(0) == Qt::Checked && (*itr)->data(ObjectId, Qt::UserRole).value<unsigned>() > 0);
			++itr;
		}
	}
	else if(filtered_objs_view->model())
	{
		selected = filtered_objs_view->model()->rowCount() > 0;
	}

	return selected;
}

void DatabaseImportWidget::getObjectToImport(std::map<ObjectType, std::vector<unsigned>> &obj_oids, std::map<unsigned, std::vector<unsigned>> &col_oids)
{
	ObjectType obj_type;
	unsigned tab_oid=0;

	obj_oids.clear();
	col_oids.clear();

	// Retriving selected items from objects tree
	if(db_objects_stw->currentIndex() == 0)
	{
		QTreeWidgetItemIterator itr(db_objects_tw);
		while(*itr)
		{
			//If the item is checked and its OID is valid
			if((*itr)->checkState(0)==Qt::Checked && (*itr)->data(ObjectId, Qt::UserRole).value<unsigned>() > 0)
			{
				obj_type=static_cast<ObjectType>((*itr)->data(ObjectTypeId, Qt::UserRole).value<unsigned>());

				//If the object is not a column store it on general object list
				if(obj_type!=ObjectType::Column)
					obj_oids[obj_type].push_back((*itr)->data(ObjectId, Qt::UserRole).value<unsigned>());
				//If its a column
				else
				{
					//Get the table's oid from the parent item
					tab_oid=(*itr)->parent()->parent()->data(ObjectId, Qt::UserRole).value<unsigned>();

					//Store the column oid on the selected colums map using the table oid as key
					col_oids[tab_oid].push_back((*itr)->data(ObjectId, Qt::UserRole).value<unsigned>());
				}
			}

			++itr;
		}
	}
	// Retriving selected items from filtered objects grid
	else
	{
		QAbstractItemModel *model = filtered_objs_view->model();
		QModelIndex oid_index, type_index;

		for(int row = 0; row < model->rowCount(); row++)
		{
			oid_index = model->index(row, ObjectsListModel::ObjId);
			type_index = model->index(row, ObjectsListModel::ObjType);
			obj_type = type_index.data(Qt::UserRole).value<ObjectType>();
			obj_oids[obj_type].push_back(oid_index.data(Qt::UserRole).toUInt());
		}
	}
}

void DatabaseImportWidget::listObjects()
{
	try
	{
		if(database_cmb->currentIndex() > 0)
		{
			Connection *conn = reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
			QStringList obj_filter = objs_filter_wgt->getObjectFilters();

			dbg_output_wgt->showActionButtons(false);
			dbg_output_wgt->clear();

			//Set the working database on import helper
			import_helper->closeConnection();
			import_helper->setConnection(*conn);
			import_helper->setCurrentDatabase(database_cmb->currentText());
			import_helper->setImportOptions(import_sys_objs_chk->isChecked(), import_ext_objs_chk->isChecked(),
																			resolve_deps_chk->isChecked(), ignore_errors_chk->isChecked(),
																			debug_mode_chk->isChecked(), rand_rel_color_chk->isChecked(), true,
																			comments_as_aliases_chk->isChecked());

			import_helper->setObjectFilters(obj_filter,
																			objs_filter_wgt->isOnlyMatching(),
																			objs_filter_wgt->isMatchBySignature(),
																			objs_filter_wgt->getForceObjectsFilter());
			if(obj_filter.isEmpty() &&
				 import_helper->getCatalog().getObjectsCount({ ObjectType::Table, ObjectType::ForeignTable,
																											 ObjectType::View, ObjectType::Index,
																											 ObjectType::Type, ObjectType::Function,
																											 ObjectType::Procedure }, false) > ObjectCountThreshould)
			{
				Messagebox msgbox;
				msgbox.show(tr("The selected database seems to have a huge amount of objects! \
Trying to import such database can take minutes or even hours and, in extreme cases, crash the application. \
Please, consider using the <strong>Filter</strong> tab in order to refine the set of objects to be imported. \
Do you really want to proceed?"),
										Messagebox::Alert, Messagebox::YesNoButtons);

				if(msgbox.isRejected())
				{
					database_cmb->setCurrentIndex(0);
					return;
				}
			}

			/* If the filter is set and the non matches need to be ignored
			 * switches to the strict view of listing filtered objects */
			if(!obj_filter.isEmpty() && objs_filter_wgt->isOnlyMatching())
			{
				db_objects_tw->clear();
				db_objects_stw->setCurrentIndex(1);
				listFilteredObjects();
			}
			else
			{
				//List the objects using the static helper method
				db_objects_tw->clear();
				GuiUtilsNs::populateObjectsTable(filtered_objs_view, std::vector<attribs_map>());
				db_objects_stw->setCurrentIndex(0);
				DatabaseImportWidget::listObjects(*import_helper, db_objects_tw, true,
																					GeneralConfigWidget::getConfigurationParam(Attributes::Configuration, Attributes::HideEmptyObjGroups) == Attributes::True ?
																					DatabaseImportWidget::HideDisableEmptyGrps : DatabaseImportWidget::DisableEmptyGrps,
																					false);
				objs_parent_wgt->setEnabled(db_objects_tw->topLevelItemCount() > 0);
			}
		}

		enableImportControls(true);
	}
	catch(Exception &e)
	{
		db_objects_tw->clear();
		enableImportControls(false);
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::enableImportControls(bool enable)
{
	enable = enable && db_objects_tw->topLevelItemCount() > 0;

	if(database_cmb->currentIndex() <= 0)
	{
		db_objects_tw->clear();
		GuiUtilsNs::populateObjectsTable(filtered_objs_view, std::vector<attribs_map>());
	}

	database_cmb->setEnabled(database_cmb->count() > 1);
	buttons_wgt->setEnabled(enable);
	objs_parent_wgt->setEnabled(enable);

	enableImport();
}

void DatabaseImportWidget::listDatabases()
{
	try
	{
		//Close a previous connection opened by the import helper
		import_helper->closeConnection();
		db_objects_tw->clear();

		if(connections_cmb->currentIndex() == connections_cmb->count()-1)
		{
			if(ConnectionsConfigWidget::openConnectionsConfiguration(connections_cmb, true))
				emit s_connectionsUpdateRequested();
		}

		Connection *conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());

		if(conn)
		{
			//List the available databases using the selected connection
			import_helper->setConnection(*conn);
			DatabaseImportWidget::listDatabases(*import_helper, database_cmb);

			pg_version_alert_frm->setVisible(
					Connection::isDbVersionIgnored() &&
					!import_helper->getCatalog().isServerSupported());

			if(conn->isAutoBrowseDB())
			{
				database_cmb->blockSignals(true);
				database_cmb->setCurrentText(conn->getConnectionParam(Connection::ParamDbName));
				listObjects();
				database_cmb->blockSignals(false);
			}
		}
		else
		{
			database_cmb->clear();
			pg_version_alert_frm->setVisible(false);
		}

		enableImportControls(true);
	}
	catch(Exception &e)
	{
		database_cmb->clear();
		enableImportControls(false);
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::closeEvent(QCloseEvent *event)
{
	/* Ignore the close event when the thread is running this avoid
	close the form and make thread execute in background */
	if(import_thread->isRunning())
		event->ignore();
	else
	{
		import_helper->closeConnection();
	}
}

void DatabaseImportWidget::captureThreadError(Exception e)
{
	QPixmap ico;
	QTreeWidgetItem *item=nullptr;

	destroyModel();
	finishImport(tr("Importing process aborted!"), true);

	ico = QPixmap(GuiUtilsNs::getIconPath("error"));
	ico_lbl->setPixmap(ico);

	item = GuiUtilsNs::createOutputTreeItem(output_trw, UtilsNs::formatMessage(e.getErrorMessage()), ico, nullptr, false, true);
	GuiUtilsNs::createExceptionsTree(output_trw, e, item);

	//Destroy the current import thread and helper to avoid reuse
	destroyThread();

	//Recreates a new import thread and helper to force user to reconfigure the import
	createThread();

	database_cmb->setCurrentIndex(0);
	Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
}

void DatabaseImportWidget::filterObjects()
{
	DatabaseImportWidget::filterObjects(db_objects_tw,
																		filter_edt->text(),
																		(by_oid_chk->isChecked() ? ObjectId : 0), false);
}

void DatabaseImportWidget::filterObjects(QTreeWidget *tree_wgt, const QString &pattern, int search_column, bool sel_single_leaf)
{
	if(!tree_wgt)
		throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	QList<QTreeWidgetItem*> items;
	QTreeWidgetItemIterator itr(tree_wgt);

	if(search_column == DatabaseImportWidget::ObjectId)
		items = tree_wgt->findItems(QString("^(0)*(%1)(.)*").arg(pattern), Qt::MatchRegularExpression | Qt::MatchRecursive, search_column);
	else
		items = tree_wgt->findItems(pattern, Qt::MatchStartsWith | Qt::MatchRecursive, search_column);

	tree_wgt->blockSignals(true);
	tree_wgt->collapseAll();
	tree_wgt->clearSelection();

	while(*itr)
	{
		(*itr)->setHidden(!pattern.isEmpty());
		++itr;
	}

	if(pattern.isEmpty())
	{
		tree_wgt->topLevelItem(0)->setExpanded(true);
	}
	else
	{
		QTreeWidgetItem *parent=nullptr, *item=nullptr, *leaf=nullptr;
		int leaf_count=0;

		while(!items.isEmpty())
		{
			item=items.front();
			item->setExpanded(true);
			item->setHidden(false);
			parent=item->parent();

			while(parent)
			{
				parent->setHidden(false);
				parent->setExpanded(true);
				parent=parent->parent();
			}

			items.pop_front();

			//Counting the leaf items found so far
			if(sel_single_leaf && item->childCount()==0 && item->parent())
			{
				leaf_count++;
				leaf=item;
			}
		}

		//Selecting the single leaf item
		if(sel_single_leaf && leaf_count == 1 && leaf)
		{
			leaf->setSelected(true);
			tree_wgt->setCurrentItem(leaf);
		}
	}

	tree_wgt->blockSignals(false);
}

void DatabaseImportWidget::cancelImport()
{
	import_helper->cancelImport();
	cancel_btn->setEnabled(false);
	database_cmb->setCurrentIndex(0);
}

void DatabaseImportWidget::destroyModel()
{
	if(create_model && model_wgt)
	{
		delete model_wgt;
		model_wgt=nullptr;
	}
}

void DatabaseImportWidget::handleImportCanceled()
{
	QPixmap ico=QPixmap(GuiUtilsNs::getIconPath("alert"));
	QString msg=tr("Importing process canceled by user!");

	destroyModel();
	finishImport(msg, false);
	ico_lbl->setPixmap(ico);

	GuiUtilsNs::createOutputTreeItem(output_trw, msg, ico);
	qApp->alert(this);
}

void DatabaseImportWidget::handleImportFinished(Exception e)
{	
	if(!e.getErrorMessage().isEmpty())
	{
		Messagebox msgbox;
		msgbox.show(e, e.getErrorMessage(), Messagebox::Alert);
	}

	model_wgt->getDatabaseModel()->setInvalidated(false);

	ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("info")));
	finishImport(tr("Importing process sucessfuly ended!"), false);

	import_helper->closeConnection();
	import_thread->quit();
	import_thread->wait();
	settings_tbw->setCurrentIndex(0);
}

void DatabaseImportWidget::finishImport(const QString &msg, bool aborted_by_error)
{
	if(import_thread->isRunning())
		import_thread->quit();

	cancel_btn->setEnabled(false);
	options_gb->setEnabled(true);
	filter_gb->setEnabled(true);
	database_gb->setEnabled(true);
	progress_pb->setValue(100);
	progress_lbl->setText(msg);
	progress_lbl->repaint();
	buttons_wgt->setEnabled(false);	
	dbg_output_wgt->showActionButtons(true);

	if(model_wgt)
	{
		model_wgt->setInteractive(true);

		if(!create_model)
		{
			model_wgt->getOperationList()->removeOperations();
			model_wgt->getObjectsScene()->adjustSceneRect(true);
		}
		else
			model_wgt->rearrangeSchemasInGrid();
	}

	if(!create_model && rand_obj_pos_chk->isChecked())
		disconnect(model_wgt, nullptr, this, nullptr);

	emit s_importFinished(aborted_by_error);
}

void DatabaseImportWidget::showEvent(QShowEvent *event)
{
	if(event->spontaneous() || isThreadRunning())
		return;

	/* Selecting the default connection for import
	 * if it there's no selected connection */
	if(connections_cmb->currentIndex() <= 0)
		ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, Connection::OpImport);

	if(!import_thread)
		createThread();

	/* In case the current connection is the default for import
	 * and the auto browse flag is set for the connected database */
	try
	{
		if(connections_cmb->currentIndex() > 0)
			listDatabases();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

ModelWidget *DatabaseImportWidget::getModel()
{
	if(create_model)
		return model_wgt;

	return nullptr;
}

bool DatabaseImportWidget::isThreadRunning()
{
	return import_thread && import_thread->isRunning();
}

void DatabaseImportWidget::listDatabases(Connection conn, QComboBox *dbcombo)
{
	try
	{
		DatabaseImportHelper import_hlp;

		import_hlp.setConnection(conn);
		listDatabases(import_hlp, dbcombo);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::listDatabases(DatabaseImportHelper &import_helper, QComboBox *dbcombo)
{
	if(!dbcombo)
		return;

	try
	{
		attribs_map db_attribs;
		attribs_map::iterator itr;
		QStringList list;
		std::map<QString, unsigned> oids;

		db_attribs=import_helper.getObjects(ObjectType::Database);
		dbcombo->blockSignals(true);
		dbcombo->clear();

		if(db_attribs.empty())
			dbcombo->addItem(tr("No databases found"));
		else
		{
			itr=db_attribs.begin();
			while(itr!=db_attribs.end())
			{
				list.push_back(itr->second);
				oids[itr->second]=itr->first.toUInt();
				itr++;
			}

			list.sort();
			dbcombo->addItems(list);

			for(int i=0; i < list.count(); i++)
			{
				dbcombo->setItemIcon(i, QPixmap(GuiUtilsNs::getIconPath(ObjectType::Database)));
				dbcombo->setItemData(i, oids[list[i]]);
			}

			dbcombo->insertItem(0, tr("Found %1 database(s)").arg(db_attribs.size()));
		}

		dbcombo->setCurrentIndex(0);
		dbcombo->blockSignals(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::listObjects(DatabaseImportHelper &import_helper, QTreeWidget *tree_wgt, bool checkable_items,
																			 ObjGroupsFlag group_flags, bool create_db_item, bool create_dummy_item, int sort_by)
{
	TaskProgressWidget task_prog_wgt;

	try
	{
		if(tree_wgt)
		{
			QTreeWidgetItem *db_item = nullptr, *item = nullptr;
			std::vector<QTreeWidgetItem *> sch_items, tab_items;
			double inc = 0, inc1 = 0, aux_prog = 0;

			if(!create_dummy_item)
			{
				task_prog_wgt.setWindowTitle(tr("Retrieving objects from database..."));
				task_prog_wgt.show();
				task_prog_wgt.updateProgress(1, tr("Retrieving cluster level objects..."), enum_t(ObjectType::Database));
			}

			tree_wgt->clear();
			tree_wgt->setColumnHidden(1, true);

			if(create_db_item)
			{
				Catalog catalog = import_helper.getCatalog();
				std::vector<attribs_map> attribs;

				//Creating database item
				db_item=new QTreeWidgetItem;
				db_item->setText(0, import_helper.getCurrentDatabase());
				db_item->setIcon(0, QIcon(GuiUtilsNs::getIconPath(ObjectType::Database)));
				attribs = catalog.getObjectsAttributes(ObjectType::Database, "", "", {}, {{Attributes::Name, import_helper.getCurrentDatabase()}});

				db_item->setData(ObjectId, Qt::UserRole, attribs[0].at(Attributes::Oid).toUInt());
				db_item->setData(ObjectTypeId, Qt::UserRole, enum_t(ObjectType::Database));

				db_item->setToolTip(0, QString("OID: %1").arg(attribs[0].at(Attributes::Oid)));
				tree_wgt->addTopLevelItem(db_item);
			}
			else
				/* If the database item is not created we use the invisible root item in the tree widget
				 * so we can correctly apply the disabled/hidden statuses in database level object items */
				db_item = tree_wgt->invisibleRootItem();

			//Retrieving and listing the cluster scoped objects
			sch_items = DatabaseImportWidget::updateObjectsTree(import_helper, tree_wgt,
																													BaseObject::getChildObjectTypes(ObjectType::Database),
																													checkable_items, group_flags, db_item);

			if(create_dummy_item)
			{
				for(auto &sch_item : sch_items)
				{
					item = new QTreeWidgetItem(sch_item);
					item->setText(0, "...");
					item->setData(ObjectOtherData, Qt::UserRole, QVariant::fromValue<int>(-1));
				}
			}
			else
			{
				ObjectType obj_type = ObjectType::BaseObject;
				aux_prog = task_prog_wgt.progress_pb->value();
				inc = 40/static_cast<double>(sch_items.size());

				for(auto &sch_item : sch_items)
				{
					task_prog_wgt.updateProgress(static_cast<int>(aux_prog),
																			 tr("Retrieving objects of schema `%1'...")
																			 .arg(sch_item->text(0)), enum_t(ObjectType::Schema));

					//Retrieving and listing the schema scoped objects
					tab_items = DatabaseImportWidget::updateObjectsTree(import_helper, tree_wgt,
																															BaseObject::getChildObjectTypes(ObjectType::Schema),
																															checkable_items, group_flags, sch_item, sch_item->text(0));

					inc1 = (60 /static_cast<double>(tab_items.size()))/static_cast<double>(sch_items.size());

					for(auto &tab_item : tab_items)
					{
						aux_prog += inc1;

						if(aux_prog > 99)
							aux_prog = 99;

						obj_type = static_cast<ObjectType>(tab_item->data(ObjectTypeId, Qt::UserRole).toUInt());

						task_prog_wgt.updateProgress(static_cast<int>(aux_prog),
																				 tr("Retrieving objects of `%1' (%2)...")
																				 .arg(tab_item->text(0), BaseObject::getTypeName(obj_type)), enum_t(obj_type));

						DatabaseImportWidget::updateObjectsTree(import_helper, tree_wgt,
																										BaseObject::getChildObjectTypes(obj_type), checkable_items, group_flags,
																										tab_item, sch_item->text(0), tab_item->text(0));
					}

					aux_prog += inc;

					if(aux_prog > 99)
						aux_prog = 99;

					task_prog_wgt.progress_pb->setValue(static_cast<int>(aux_prog));
					sch_items.pop_back();
				}
			}

			tree_wgt->sortItems(sort_by, Qt::AscendingOrder);

			if(db_item)
				db_item->setExpanded(true);

			if(!create_dummy_item)
			{
				task_prog_wgt.progress_pb->setValue(100);
				task_prog_wgt.close();
			}
		}

	}
	catch(Exception &e)
	{
		task_prog_wgt.close();
		tree_wgt->clear();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

std::vector<QTreeWidgetItem *> DatabaseImportWidget::updateObjectsTree(DatabaseImportHelper &import_helper, QTreeWidget *tree_wgt, std::vector<ObjectType> types, bool checkable_items,
																																			 ObjGroupsFlag group_flags, QTreeWidgetItem *root, const QString &schema, const QString &table)
{
	if(!tree_wgt)
		return {};

	std::vector<QTreeWidgetItem *> items_vect;
	QTreeWidgetItem *group = nullptr, *item = nullptr;
	QFont grp_fnt = tree_wgt->font();
	attribs_map extra_attribs = {{Attributes::FilterTableTypes, Attributes::True}};
	QString tooltip = QString("OID: %1"), name, label;
	bool child_checked = false,
			disable_empty_grps = (group_flags & DisableEmptyGrps) == DisableEmptyGrps,
			hide_empty_grps = (group_flags & HideEmptyGrps) == HideEmptyGrps;
	std::vector<attribs_map> objects_vect;
	std::map<ObjectType, QTreeWidgetItem *> gen_groups;
	ObjectType obj_type;
	QList<QTreeWidgetItem*> groups_list;
	unsigned oid=0;
	int start=-1, end=-1;
	std::map<QString, QString> constr_icons = { { Attributes::PkConstr, "constraint_pk" },
																							{ Attributes::FkConstr, "constraint_fk" },
																							{ Attributes::UqConstr, "constraint_uq" },
																							{ Attributes::CkConstr, "constraint_ck" },
																							{ Attributes::ExConstr, "constraint_ex" } };

	grp_fnt.setItalic(true);
	tree_wgt->blockSignals(true);
	tree_wgt->setUpdatesEnabled(false);
	tree_wgt->setSortingEnabled(false);

	try
	{
		for(ObjectType grp_type : types)
		{
			//Create a group item for the current type
			group = new QTreeWidgetItem(root);
			group->setIcon(0, QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(grp_type))));
			group->setFont(0, grp_fnt);
			group->setText(0, BaseObject::getTypeName(grp_type) + " (0)");

			//Group items does contains a zero valued id to indicate that is not a valide object
			group->setData(ObjectId, Qt::UserRole, 0);
			group->setData(ObjectTypeId, Qt::UserRole, enum_t(grp_type));
			group->setData(ObjectCount, Qt::UserRole, 0);
			group->setData(ObjectSchema, Qt::UserRole, schema);
			group->setData(ObjectTable, Qt::UserRole, table);
			group->setData(ObjectGroupId, Qt::UserRole, -(enum_t(grp_type) + (root ? root->data(ObjectId, 0).toUInt() : 0)));

			gen_groups[grp_type] = group;
			groups_list.push_back(group);
		}

		objects_vect = import_helper.getObjects(types, schema, table, extra_attribs);

		for(attribs_map &attribs : objects_vect)
		{
			obj_type = static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
			group = gen_groups[obj_type];
			group->setData(ObjectCount, Qt::UserRole,
										 group->data(ObjectCount, Qt::UserRole).toUInt() + 1);

			//Creates individual items for each object of the current type
			oid = attribs[Attributes::Oid].toUInt();

			attribs[Attributes::Name].remove(QRegularExpression("( )(without)( time zone)"));
			label = name = attribs[Attributes::Name];

			//Removing the trailing type string from op. families or op. classes names
			if(obj_type == ObjectType::OpFamily || obj_type == ObjectType::OpClass)
			{
				start = name.indexOf(QChar('['));
				end = name.lastIndexOf(QChar(']'));
				name.remove(start, (end-start)+1);
				name = name.trimmed();
			}

			item = new QTreeWidgetItem(group);

			if(obj_type == ObjectType::Constraint)
				item->setIcon(0, QIcon(GuiUtilsNs::getIconPath(constr_icons[attribs[Attributes::ExtraInfo]])));
			else
				item->setIcon(0, QIcon(GuiUtilsNs::getIconPath(obj_type)));

			item->setText(0, label);
			item->setText(ObjectId, attribs[Attributes::Oid].rightJustified(10, '0'));
			item->setData(ObjectId, Qt::UserRole, attribs[Attributes::Oid].toUInt());
			item->setData(ObjectName, Qt::UserRole, name);

			if(checkable_items)
			{
				/* If the current import helper has objects filter we will not mark the items in the tree as check
				 * since only the ones matching the object types are checked in the final step of the tree creation */
				if(/*!has_obj_filters &&*/
					 ((oid > import_helper.getLastSystemOID()) ||
						(obj_type == ObjectType::Schema && name == "public") ||
						(obj_type == ObjectType::Column && root && root->data(0, Qt::UserRole).toUInt() > import_helper.getLastSystemOID())))
				{
					item->setCheckState(0, Qt::Checked);
					child_checked=true;
				}
				else
					item->setCheckState(0, Qt::Unchecked);

				//Disabling items that refers to PostgreSQL's built-in data types
				if(obj_type == ObjectType::Type && oid <= import_helper.getLastSystemOID())
				{
					item->setDisabled(true);
					item->setToolTip(0, tr("This is a PostgreSQL built-in data type and cannot be imported."));
				}
				//Disabling items that refers to pgModeler's built-in system objects
				else if((obj_type == ObjectType::Tablespace && (name == "pg_default" || name == "pg_global")) ||
								(obj_type == ObjectType::Role && (name == "postgres")) ||
								(obj_type == ObjectType::Schema && (name == "pg_catalog" || name == "public")) ||
								(obj_type == ObjectType::Language && (name.toLower() == DefaultLanguages::C ||
																										name.toLower() == DefaultLanguages::Sql ||
																										name.toLower() == DefaultLanguages::PlPgsql)))
				{
					item->setFont(0, grp_fnt);
					item->setForeground(0, BaseObjectView::getFontStyle(Attributes::ProtColumn).foreground());
					item->setToolTip(0, tr("This is a pgModeler's built-in object. It will be ignored if checked by user."));
				}
			}

			//Stores the object's OID as the first data of the item
			item->setData(ObjectId, Qt::UserRole, oid);

			if(!item->toolTip(0).isEmpty())
				item->setToolTip(0,item->toolTip(0) + "\n" + tooltip.arg(oid));
			else
				item->setToolTip(0,tooltip.arg(oid));

			//Stores the object's type as the second data of the item
			item->setData(ObjectTypeId, Qt::UserRole, enum_t(obj_type));

			//Stores the schema and the table's name of the object
			item->setData(ObjectSchema, Qt::UserRole, schema);
			item->setData(ObjectTable, Qt::UserRole, table);

			if(obj_type==ObjectType::Schema || BaseTable::isBaseTable(obj_type))
				items_vect.push_back(item);
		}

		//Updating the object count in each group
		unsigned item_cnt = 0;

		for(auto &grp_type : types)
		{
			group = gen_groups[grp_type];

			item_cnt = group->data(ObjectCount, Qt::UserRole).toUInt();
			group->setDisabled(disable_empty_grps && item_cnt == 0);
			group->setHidden(hide_empty_grps && item_cnt == 0);

			group->setText(0, BaseObject::getTypeName(grp_type) +
												QString(" (%1)").arg(item_cnt));

			if(checkable_items)
			{
				if(!group->isDisabled() && child_checked)
					group->setCheckState(0, Qt::Checked);
				else
					group->setCheckState(0, Qt::Unchecked);
			}
		}

		tree_wgt->addTopLevelItems(groups_list);
		tree_wgt->setUpdatesEnabled(true);
		tree_wgt->blockSignals(false);

		return items_vect;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseImportWidget::updateConnections()
{
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, Connection::OpImport);

	db_objects_tw->clear();
	database_cmb->clear();
	enableImportControls(false);
}
