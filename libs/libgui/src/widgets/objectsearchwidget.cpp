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

#include "objectsearchwidget.h"
#include "guiutilsns.h"
#include "objectslistmodel.h"

ObjectSearchWidget::ObjectSearchWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	filter_wgt = new QWidget(this);
	obj_types_lst = new ObjectTypesListWidget(this);
	obj_types_lst->layout()->setContentsMargins(0,0,0,0);

	regexp_chk = new QCheckBox(this);
	regexp_chk->setText(tr("Regular expression"));

	exact_match_chk = new QCheckBox(this);
	exact_match_chk->setText(tr("Exact match"));
	exact_match_chk->setEnabled(false);

	case_sensitive_chk = new QCheckBox(this);
	case_sensitive_chk->setText(tr("Case sensitive"));

	_search_attrs_cmb = new QComboBox(this);
	_search_attrs_lbl = new QLabel(this);
	_search_attrs_lbl->setText(tr("Search field:"));
	_search_attrs_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	QHBoxLayout *hbox = new QHBoxLayout;
	hbox->addWidget(_search_attrs_lbl);
	hbox->addWidget(_search_attrs_cmb);

	QVBoxLayout *vbox = new QVBoxLayout(filter_wgt);
	vbox->addLayout(hbox);
	vbox->addWidget(obj_types_lst);
	vbox->addWidget(regexp_chk);
	vbox->addWidget(exact_match_chk);
	vbox->addWidget(case_sensitive_chk);

	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->setSpacing(GuiUtilsNs::LtSpacing);
	filter_wgt->setLayout(vbox);

	QWidgetAction *wgt_act_filter = new QWidgetAction(this);
	wgt_act_filter->setDefaultWidget(filter_wgt);
	filter_menu.addAction(wgt_act_filter);
	filter_btn->setMenu(&filter_menu);

	select_menu.addAction(tr("Listed"), this, &ObjectSearchWidget::selectObjects);
	select_menu.addAction(tr("Not listed"), this, &ObjectSearchWidget::selectObjects);
	select_btn->setMenu(&select_menu);

	fade_menu.addAction(tr("Listed"), this, &ObjectSearchWidget::fadeObjects);
	fade_menu.addAction(tr("Not listed"), this, &ObjectSearchWidget::fadeObjects);
	fade_btn->setMenu(&fade_menu);

	connect(pattern_edt, &QLineEdit::textChanged, this, [this](const QString &txt) {
		search_btn->setEnabled(!txt.isEmpty());
	});

	connect(search_btn, &QPushButton::clicked, this, &ObjectSearchWidget::findObjects);
	connect(hide_tb, &QPushButton::clicked, this, &ObjectSearchWidget::hide);
	connect(clear_res_btn, &QPushButton::clicked, this, &ObjectSearchWidget::clearResult);

	connect(result_view, &QTableView::doubleClicked, this, &ObjectSearchWidget::editObject);
	connect(result_view, &QTableView::pressed, this, &ObjectSearchWidget::showObjectMenu);

	connect(regexp_chk, &QCheckBox::toggled, this, [this](bool checked){
		exact_match_chk->setEnabled(checked);

		if(!checked)
			exact_match_chk->setChecked(false);
	});

	for(auto &attr : BaseObject::getSearchAttributesNames())
		_search_attrs_cmb->addItem(BaseObject::getSearchAttributeI18N(attr), attr);

	this->setModel(nullptr);
	pattern_edt->installEventFilter(this);
}

bool ObjectSearchWidget::eventFilter(QObject *object, QEvent *event)
{
	QKeyEvent *k_event=dynamic_cast<QKeyEvent *>(event);

	//Executes the search when user press and releases enter/return on the pattern field
	if(event->type() == QEvent::KeyRelease &&
			(k_event->key()==Qt::Key_Return || k_event->key()==Qt::Key_Enter))
	{
		search_btn->click();
		return true;
	}

	return QWidget::eventFilter(object, event);
}

void ObjectSearchWidget::hide()
{
	QWidget::hide();
	emit s_hideRequested();
}

void ObjectSearchWidget::showEvent(QShowEvent *)
{
	pattern_edt->setFocus();
}

void ObjectSearchWidget::resizeEvent(QResizeEvent *event)
{
	GuiUtilsNs::resizeChildToolButtons(this, event->size());
}

void ObjectSearchWidget::fadeObjects()
{
	if(!model_wgt)
		return;

	std::vector<BaseObject *> objects, other_objs;
	bool fade_listed = false;

	for(auto obj_type : {ObjectType::Table, ObjectType::ForeignTable, ObjectType::View, ObjectType::Textbox,
											 ObjectType::Relationship, ObjectType::BaseRelationship, ObjectType::Schema})
	{
		objects.insert(objects.end(),
									 model_wgt->getDatabaseModel()->getObjectList(obj_type)->begin(),
									 model_wgt->getDatabaseModel()->getObjectList(obj_type)->end());
	}

	model_wgt->fadeObjects(objects, true);

	if(!fade_menu.actions().contains(qobject_cast<QAction *>(sender())))
		return;

	fade_listed = qobject_cast<QAction *>(sender()) == fade_menu.actions().at(0);

	std::sort(objects.begin(), objects.end());
	std::sort(found_objs.begin(), found_objs.end());
	std::set_difference(objects.begin(), objects.end(), found_objs.begin(), found_objs.end(),
											std::inserter(other_objs, other_objs.begin()));

	model_wgt->fadeObjects(found_objs, !fade_listed);
	model_wgt->fadeObjects(other_objs, fade_listed);
}

void ObjectSearchWidget::selectObjects()
{
	if(!model_wgt)
		return;

	std::vector<BaseObject *> objects, other_objs;
	BaseObjectView *obj_view = nullptr;
	BaseGraphicObject *graph_obj = nullptr;
	bool sel_listed = false;

	for(auto obj_type : {ObjectType::Table, ObjectType::View, ObjectType::Textbox,
												ObjectType::Relationship, ObjectType::BaseRelationship, ObjectType::Schema})
	{
		objects.insert(objects.end(),
									 model_wgt->getDatabaseModel()->getObjectList(obj_type)->begin(),
									 model_wgt->getDatabaseModel()->getObjectList(obj_type)->end());
	}

	sel_listed = qobject_cast<QAction *>(sender()) == select_menu.actions().at(0);

	std::sort(objects.begin(), objects.end());
	std::sort(found_objs.begin(), found_objs.end());
	std::set_difference(objects.begin(), objects.end(), found_objs.begin(), found_objs.end(),
											std::inserter(other_objs, other_objs.begin()));

	objects.clear();

	if(sel_listed)
		objects.assign(found_objs.begin(), found_objs.end());
	else
		objects.assign(other_objs.begin(), other_objs.end());

	for(auto &obj : objects)
	{
		graph_obj = dynamic_cast<BaseGraphicObject *>(obj);

		if(graph_obj)
		{
			obj_view = dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

			if(obj_view)
			{
				obj_view->blockSignals(true);
				obj_view->setSelected(true);
				obj_view->blockSignals(false);
			}
		}
	}

	model_wgt->configureObjectSelection();
}

void ObjectSearchWidget::setModel(ModelWidget *model_wgt)
{
	bool enable=model_wgt!=nullptr;

	clearResult();
	this->model_wgt=model_wgt;
	filter_btn->setEnabled(enable);
	pattern_edt->setEnabled(enable);
	pattern_lbl->setEnabled(enable);
	search_btn->setEnabled(enable && !pattern_edt->text().isEmpty());
	result_view->setEnabled(enable);
}

void ObjectSearchWidget::clearResult()
{
	selected_obj=nullptr;
	found_objs.clear();
	selected_objs.clear();

	GuiUtilsNs::populateObjectsTable(result_view, std::vector<BaseObject *>());

	found_lbl->setVisible(false);
	clear_res_btn->setEnabled(false);

	select_btn->setEnabled(false);
	fade_btn->setEnabled(false);
}

void ObjectSearchWidget::findObjects()
{
	if(!model_wgt)
		return;

	std::vector<ObjectType> types;
	QString search_attr = _search_attrs_cmb->currentData().toString();

	qApp->setOverrideCursor(Qt::WaitCursor);
	clearResult();
	types = obj_types_lst->getTypesPerCheckState(Qt::Checked);

	//Search the objects on model
	found_objs = model_wgt->getDatabaseModel()->findObjects(pattern_edt->text(), types,
																													case_sensitive_chk->isChecked(),
																													regexp_chk->isChecked(),
																													exact_match_chk->isChecked(),
																													search_attr);
	found_lbl->setVisible(true);

	//Show a message indicating the number of found objects
	if(!found_objs.empty())
		found_lbl->setText(tr("Found <strong>%1</strong> object(s).").arg(found_objs.size()));
	else
		found_lbl->setText(tr("No objects found."));

	/* Everytime we make a new object search the selectionModel of the result view changes
	 * so we need to disconnect the signals/slot from the current selection model before
	 * the searching and then reconnect the new selection model to the related slots */
	if(result_view->selectionModel())
		disconnect(result_view->selectionModel(), nullptr, this, nullptr);

	GuiUtilsNs::populateObjectsTable(result_view, found_objs, search_attr);

	if(result_view->selectionModel())
		connect(result_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ObjectSearchWidget::selectObject);

	clear_res_btn->setEnabled(!found_objs.empty());
	select_btn->setEnabled(!found_objs.empty());
	fade_btn->setEnabled(!found_objs.empty());
	qApp->restoreOverrideCursor();
}

void ObjectSearchWidget::selectObject()
{
	selected_obj = nullptr;
	selected_objs.clear();
	model_wgt->scene->clearSelection();

	BaseGraphicObject *graph_obj = nullptr;
	BaseObjectView *obj_view = nullptr;
	BaseObject *object = nullptr;
	QModelIndexList sel_indexes = result_view->selectionModel()->selectedIndexes();

	if(sel_indexes.size() == 1)
	{
		QModelIndex index = sel_indexes.at(0);

		if(index.column() == ObjectsListModel::ObjName ||
			 index.column() == ObjectsListModel::ParentName)
		{
			object = reinterpret_cast<BaseObject *>(index.data(Qt::UserRole).value<void *>());

			if(object->getObjectType() == ObjectType::Permission)
				return;

			selected_obj = object;
		}
	}

	for(auto &index : sel_indexes)
	{
		object = reinterpret_cast<BaseObject *>(index.data(Qt::UserRole).value<void *>());

		if(!object ||
			 (object && object->getObjectType() == ObjectType::Permission))
			continue;

		selected_objs.push_back(object);
	}

	// Highlighting the selected graphical objects
	TableObject *tab_obj = nullptr;
	BaseTableView *tab_view = nullptr;

	for(auto &obj : selected_objs)
	{
		graph_obj = dynamic_cast<BaseGraphicObject *>(obj);
		tab_obj = dynamic_cast<TableObject *>(obj);

		if(graph_obj)
		{
			obj_view = dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

			if(obj_view)
			{
				model_wgt->viewport->centerOn(obj_view);
				obj_view->setSelected(true);
			}
		}
		else if(tab_obj)
		{
			tab_view = dynamic_cast<BaseTableView *>(tab_obj->getParentTable()->getOverlyingObject());
			tab_view->setChildSelected(tab_obj);
			model_wgt->viewport->centerOn(tab_view);
		}
	}

	model_wgt->configurePopupMenu(selected_objs);
	model_wgt->emitSceneInteracted();
}

void ObjectSearchWidget::showObjectMenu()
{
	if(!selected_objs.empty() && QApplication::mouseButtons()==Qt::RightButton)
		model_wgt->showObjectMenu();
}

void ObjectSearchWidget::editObject()
{
	if(selected_obj)
	{
		if(selected_obj->getObjectType() == ObjectType::Permission)
			model_wgt->showObjectForm(ObjectType::Permission, dynamic_cast<Permission *>(selected_obj)->getObject());
		else
		{
			std::vector<BaseObject *> vect;
			vect.push_back(selected_obj);
			model_wgt->scene->clearSelection();
			model_wgt->configurePopupMenu(vect);
			model_wgt->editObject();
		}

		selected_obj = nullptr;
	}
}
