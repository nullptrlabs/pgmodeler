/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

/**
\ingroup libgui
\class OperationListWidget
\brief Implements the widget representation for the operation list, giving access to the undo/redo action over the model.
*/

#ifndef OPERATION_LIST_WIDGET_H
#define OPERATION_LIST_WIDGET_H

#include "ui_operationlistwidget.h"
#include "modelwidget.h"

class __libgui OperationListWidget: public QWidget, public Ui::OperationListWidget {
	Q_OBJECT

	private:
		ModelWidget *model_wgt;

		//! \brief Updates the operation list and emits the signal s_operationListUpdated to the connected objects
		void notifyUpdateOnModel();

	public:
		OperationListWidget(QWidget * parent = nullptr);

	public slots:
		void updateOperationList();
		void setModel(ModelWidget *model);
		void undoOperation();
		void redoOperation();
		void removeOperations();
		void hide();

	private slots:
		void selectItem(QTreeWidgetItem *item, int coluna);

	signals:
		void s_operationExecuted();
		void s_operationListUpdated();
		void s_hideRequested();
};

#endif
