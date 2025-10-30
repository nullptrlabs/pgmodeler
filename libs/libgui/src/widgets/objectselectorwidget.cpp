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

#include "objectselectorwidget.h"

ObjectSelectorWidget::ObjectSelectorWidget(ObjectType sel_obj_type, QWidget *parent) : QWidget(parent)
{
	this->sel_obj_types.push_back(sel_obj_type);
	configureSelector();
}

ObjectSelectorWidget::ObjectSelectorWidget(std::vector<ObjectType> sel_obj_types, QWidget * parent) : QWidget(parent)
{
	this->sel_obj_types=sel_obj_types;
	configureSelector();
}

void ObjectSelectorWidget::configureSelector()
{
	Ui_ObjectSelectorWidget::setupUi(this);
	obj_view_wgt = new ModelObjectsWidget(true);

	model=nullptr;
	selected_obj=nullptr;

	connect(sel_object_tb, &QToolButton::clicked, this, &ObjectSelectorWidget::showObjectView);
	connect(rem_object_tb, &QToolButton::clicked, this, &ObjectSelectorWidget::clearSelector);
	connect(obj_view_wgt, qOverload<BaseObject*, bool>(&ModelObjectsWidget::s_visibilityChanged),
						 this, qOverload<BaseObject*, bool>(&ObjectSelectorWidget::showSelectedObject));

	obj_name_edt->installEventFilter(this);
}

bool ObjectSelectorWidget::eventFilter(QObject *obj, QEvent *evnt)
{
	if(this->isEnabled() && evnt->type()==QEvent::MouseButtonPress
		 && QApplication::mouseButtons()==Qt::LeftButton && obj==obj_name_edt)
	{
		showObjectView();
		return true;
	}

	return QWidget::eventFilter(obj, evnt);
}

ObjectSelectorWidget::~ObjectSelectorWidget()
{
	delete obj_view_wgt;
}

void ObjectSelectorWidget::enableObjectCreation(bool value)
{
	obj_view_wgt->enableObjectCreation(value);
}

BaseObject *ObjectSelectorWidget::getSelectedObject()
{
	return selected_obj;
}

QString ObjectSelectorWidget::getSelectedObjectName()
{
	return selected_obj->getSignature();
}

void ObjectSelectorWidget::setSelectedObject(BaseObject *object)
{
	ObjectType obj_type = ObjectType::BaseObject;

	if(object)
		obj_type=object->getObjectType();

	if(object && std::find(sel_obj_types.begin(), sel_obj_types.end(),obj_type)!=sel_obj_types.end())
	{
		rem_object_tb->setEnabled(object);
		this->selected_obj=object;

		if(obj_type != ObjectType::Constraint)
		{
			if(obj_type != ObjectType::UserMapping)
				obj_name_edt->setText(selected_obj->getSignature());
			else
				obj_name_edt->setText(selected_obj->getName());
		}
		else
			obj_name_edt->setText(QString("%1.%2")
														.arg(dynamic_cast<TableObject *>(selected_obj)->getParentTable()->getSignature())
														.arg(selected_obj->getName(true)));

		emit s_objectSelected();
		emit s_selectorChanged(true);
	}
	else
		clearSelector();
}

void ObjectSelectorWidget::setModel(DatabaseModel *modelo)
{
	this->model=modelo;
}

void ObjectSelectorWidget::showSelectedObject(BaseObject *obj_sel, bool)
{
	if(obj_sel)
		setSelectedObject(obj_sel);
}

void ObjectSelectorWidget::clearSelector()
{
	this->selected_obj=nullptr;
	obj_name_edt->clear();
	rem_object_tb->setEnabled(false);
	emit s_selectorCleared();
	emit s_selectorChanged(false);
}

void ObjectSelectorWidget::showObjectView()
{
	obj_name_edt->clearFocus();

	for(auto &obj_tp : sel_obj_types)
		obj_view_wgt->setObjectVisible(obj_tp, true);

	if(sel_obj_types.size() == 1)
		obj_view_wgt->setWindowTitle(tr("Select %1").arg(BaseObject::getTypeName(sel_obj_types[0]).toLower()));

	obj_view_wgt->setModel(this->model);
	obj_view_wgt->show();
}
