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

#include "operationlistwidget.h"
#include "guiutilsns.h"

OperationListWidget::OperationListWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setModel(nullptr);

	QFont fnt = title_lbl->font();
	fnt.setPointSizeF(fnt.pointSizeF() * 0.85);
	title_lbl->setFont(fnt);

	operations_tw->headerItem()->setHidden(true);
	connect(undo_tb, &QToolButton::clicked, this, &OperationListWidget::undoOperation);
	connect(redo_tb, &QToolButton::clicked, this, &OperationListWidget::redoOperation);
	connect(rem_operations_tb, &QToolButton::clicked, this, &OperationListWidget::removeOperations);
	connect(operations_tw, &QTreeWidget::itemClicked, this, &OperationListWidget::selectItem);
	connect(hide_tb, &QToolButton::clicked, this, &OperationListWidget::hide);
}

void OperationListWidget::hide()
{
	QWidget::hide();
	emit s_hideRequested();
}

void OperationListWidget::selectItem(QTreeWidgetItem *item, int)
{
	operations_tw->clearSelection();

	if(item)
	{
		if(item->parent())
			item=item->parent();

		item->setSelected(true);
		operations_tw->setCurrentItem(item);
	}
}

void OperationListWidget::updateOperationList()
{
	content_frm->setEnabled(this->model_wgt!=nullptr);

	if(!model_wgt)
	{
		operations_tw->clear();
		op_count_lbl->setText("-");
		current_pos_lbl->setText("-");
	}
	else
	{
		unsigned count = 0;
		QString str_aux, op_name, op_icon;
		QTreeWidgetItem *item = nullptr, *item1 = nullptr;
		QFont font = this->font();
		bool value = false;
		Operation::OperationInfo op_info;

		operations_tw->setUpdatesEnabled(false);
		op_count_lbl->setText(QString("%1").arg(model_wgt->op_list->getCurrentSize()));
		current_pos_lbl->setText(QString("%1").arg(model_wgt->op_list->getCurrentIndex()));
		redo_tb->setEnabled(model_wgt->op_list->isRedoAvailable());
		undo_tb->setEnabled(model_wgt->op_list->isUndoAvailable());

		count = model_wgt->op_list->getCurrentSize();
		operations_tw->clear();
		rem_operations_tb->setEnabled(count > 0);

		for(unsigned i = 0; i < count; i++)
		{
			op_info = model_wgt->op_list->getOperation(i)->getOperationInfo();

			value = (i == static_cast<unsigned>(model_wgt->op_list->getCurrentIndex() - 1));
			font.setBold(value);
			font.setItalic(value);

			item = new QTreeWidgetItem;
			str_aux = QString(BaseObject::getSchemaName(op_info.obj_type));
			item->setData(0, Qt::UserRole, QVariant(enum_t(op_info.obj_type)));

			if(op_info.obj_type == ObjectType::BaseRelationship)
				str_aux += "tv";

			item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath(str_aux)));

			operations_tw->insertTopLevelItem(i,item);
			item->setFont(0, font);
			item->setText(0, QString("%1 (%2)").arg(op_info.obj_name, BaseObject::getTypeName(op_info.obj_type)));

			if(op_info.oper_type == Operation::ObjCreated)
			{
				op_icon = "created";
				op_name = tr("created");
			}
			else if(op_info.oper_type == Operation::ObjRemoved)
			{
				op_icon = "removed";
				op_name = tr("removed");
			}
			else if(op_info.oper_type == Operation::ObjModified)
			{
				op_icon = "modified";
				op_name = tr("modified");
			}
			else if(op_info.oper_type == Operation::ObjMoved)
			{
				op_icon = "moved";
				op_name = tr("moved");
			}

			item1 = new QTreeWidgetItem(item);
			item1->setIcon(0, QPixmap(GuiUtilsNs::getIconPath(op_icon)));
			item1->setFont(0, font);
			item1->setText(0, op_name);

			operations_tw->expandItem(item);

			if(value)
				operations_tw->scrollToItem(item1);
		}

		operations_tw->setUpdatesEnabled(true);
	}

	emit s_operationListUpdated();
}

void OperationListWidget::setModel(ModelWidget *model)
{
	operations_tw->clear();
	this->model_wgt=model;
	updateOperationList();
}

void OperationListWidget::undoOperation()
{
	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);
		model_wgt->setBlinkAddedObjects(true);
		model_wgt->op_list->undoOperation();
		model_wgt->setBlinkAddedObjects(false);
		notifyUpdateOnModel();
		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		this->updateOperationList();

		if(e.getErrorCode()==ErrorCode::UndoRedoOperationInvalidObject)
		{
			Messagebox msg_box;
			msg_box.show(e, "", Messagebox::Alert);
		}
		else
			Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void OperationListWidget::redoOperation()
{
	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);
		model_wgt->setBlinkAddedObjects(true);
		model_wgt->op_list->redoOperation();
		model_wgt->setBlinkAddedObjects(false);
		notifyUpdateOnModel();
		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();

		if(e.getErrorCode()==ErrorCode::UndoRedoOperationInvalidObject)
		{
			Messagebox msg_box;
			msg_box.show(e, "", Messagebox::Alert);
		}
		else
			Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void OperationListWidget::removeOperations()
{
	int res = Messagebox::confirm(tr("Deleting the executed operations history is an irreversible action. Do you want to continue?"));

	if(Messagebox::isAccepted(res))
	{
		model_wgt->op_list->removeOperations();
		updateOperationList();
		rem_operations_tb->setEnabled(false);
	}
}

void OperationListWidget::notifyUpdateOnModel()
{
	updateOperationList();
	emit s_operationExecuted();
}
