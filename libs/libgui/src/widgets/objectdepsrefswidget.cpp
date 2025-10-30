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

#include "objectdepsrefswidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"

ObjectDepsRefsWidget::ObjectDepsRefsWidget(QWidget *parent): BaseObjectWidget(parent)
{
	Ui_ObjectDepsRefsWidget::setupUi(this);
	configureFormLayout(objectdepsrefs_grid, ObjectType::BaseObject);

	model_wgt=nullptr;
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);
	alert_frm->setVisible(false);

	connect(inc_indirect_links_chk,	&QCheckBox::toggled, this, &ObjectDepsRefsWidget::updateObjectTables);
	connect(unique_results_chk,	&QCheckBox::toggled, this, &ObjectDepsRefsWidget::updateObjectTables);
	connect(dependencies_view, &QTableView::doubleClicked, this, &ObjectDepsRefsWidget::handleItemSelection);
	connect(references_view, &QTableView::doubleClicked, this, &ObjectDepsRefsWidget::handleItemSelection);

	setMinimumSize(580, 350);
}

void ObjectDepsRefsWidget::setAttributes(DatabaseModel *model, BaseObject *object, BaseObject *parent_obj)
{
	BaseObjectWidget::setAttributes(model, object, parent_obj);

	if(object->getObjectType() == ObjectType::Constraint ||
		 object->getObjectType() == ObjectType::UserMapping)
		name_edt->setText(object->getName());

	this->name_edt->setReadOnly(true);
	this->protected_obj_frm->setVisible(false);
	this->comment_edt->setVisible(false);
	this->comment_lbl->setVisible(false);
	obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(object->getObjectType())));
	updateObjectTables();
}

void ObjectDepsRefsWidget::setAttributes(ModelWidget *model_wgt, BaseObject *object, BaseObject *parent_obj)
{
	if(!model_wgt)
		return;

	this->model_wgt=model_wgt;
	setAttributes(model_wgt->getDatabaseModel(), object, parent_obj);
}

void ObjectDepsRefsWidget::applyConfiguration()
{
	emit s_closeRequested();
}

void ObjectDepsRefsWidget::updateObjectTables()
{
	GuiUtilsNs::populateObjectsTable(dependencies_view,
																	this->object->getDependencies(inc_indirect_links_chk->isChecked(), {}, unique_results_chk->isChecked()));

	GuiUtilsNs::populateObjectsTable(references_view,
																	this->object->getReferences(inc_indirect_links_chk->isChecked(), {}, unique_results_chk->isChecked()));
}

void ObjectDepsRefsWidget::handleItemSelection(const QModelIndex& index)
{
	BaseObject *sel_obj = nullptr, *parent = nullptr;
	Table *parent_tab = nullptr;
	View *parent_view = nullptr;

	sel_obj = reinterpret_cast<BaseObject*>(index.data(Qt::UserRole).value<void *>());

	if(sel_obj)
	{
		if(TableObject::isTableObject(sel_obj->getObjectType()))
			parent=dynamic_cast<TableObject *>(sel_obj)->getParentTable();

		model_wgt->showObjectForm(sel_obj->getObjectType(), sel_obj, parent);

		if(TableObject::isTableObject(this->object->getObjectType()))
		{
			parent=dynamic_cast<TableObject *>(this->object)->getParentTable();

			if(parent->getObjectType()==ObjectType::Table)
				parent_tab=dynamic_cast<Table *>(parent);
			else
				parent_view=dynamic_cast<View *>(parent);
		}

		if((parent_tab && parent_tab->getObjectIndex(this->object) >= 0) ||
				(parent_view && parent_view->getObjectIndex(this->object) >= 0) ||
				(model->getObjectIndex(this->object) >= 0))
		{
			updateObjectTables();
		}
		else
		{
			references_view->setEnabled(false);
			dependencies_view->setEnabled(false);
			inc_indirect_links_chk->setEnabled(false);
			unique_results_chk->setEnabled(false);
			alert_frm->setVisible(true);
		}
	}
}
