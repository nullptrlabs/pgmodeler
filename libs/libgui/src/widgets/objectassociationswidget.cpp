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

#include "objectassociationswidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"

ObjectAssociationsWidget::ObjectAssociationsWidget(QWidget *parent): QWidget(parent)
{
	Ui_ObjectAssociationsWidget::setupUi(this);

	object = nullptr;
	model_wgt = nullptr;

	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint,
															{ obj_name_frm, separator_ln, separator2_ln });

	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);
	alert_frm->setVisible(false);

	connect(inc_indirect_links_chk,	&QCheckBox::toggled, this, &ObjectAssociationsWidget::updateObjectTables);
	connect(unique_results_chk,	&QCheckBox::toggled, this, &ObjectAssociationsWidget::updateObjectTables);
	connect(dependencies_view, &QTableView::doubleClicked, this, &ObjectAssociationsWidget::handleItemSelection);
	connect(references_view, &QTableView::doubleClicked, this, &ObjectAssociationsWidget::handleItemSelection);

	setMinimumSize(580, 350);
}

void ObjectAssociationsWidget::setAttributes(ModelWidget *model_wgt, BaseObject *object, bool show_obj_name)
{
	this->model_wgt = model_wgt;
	setAttributes(object, show_obj_name);
}

void ObjectAssociationsWidget::setAttributes(BaseObject *object, bool show_obj_name)
{
	this->setEnabled(object != nullptr);
	this->object = object;
	updateObjectTables();

	obj_name_frm->setVisible(show_obj_name);

	if(object)
	{
		obj_name_lbl->setText(object->getSignature().remove('"'));
		obj_id_lbl->setText(QString::number(object->getObjectId()));
		type_name_lbl->setText(object->getTypeName());
		type_icon_lbl->setPixmap(GuiUtilsNs::getPixmap(object->getObjectType()));
	}
}

void ObjectAssociationsWidget::updateObjectTables()
{
	if(!object)
		return;

	GuiUtilsNs::populateObjectsTable(dependencies_view,
																	 object->getDependencies(inc_indirect_links_chk->isChecked(),
																													 {}, unique_results_chk->isChecked()));

	GuiUtilsNs::populateObjectsTable(references_view,
																	 object->getReferences(inc_indirect_links_chk->isChecked(),
																												 {}, unique_results_chk->isChecked()));
}

void ObjectAssociationsWidget::handleItemSelection(const QModelIndex &index)
{
	if(!model_wgt || !object)
		return;

	BaseObject *sel_obj = nullptr, *parent = nullptr;
	Table *parent_tab = nullptr;
	View *parent_view = nullptr;
	DatabaseModel *dbmodel = model_wgt->getDatabaseModel();

	sel_obj = reinterpret_cast<BaseObject*>(index.data(Qt::UserRole).value<void *>());

	if(!sel_obj)
		return;

	if(TableObject::isTableObject(sel_obj->getObjectType()))
		parent = dynamic_cast<TableObject *>(sel_obj)->getParentTable();

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
			(dbmodel->getObjectIndex(this->object) >= 0))
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
