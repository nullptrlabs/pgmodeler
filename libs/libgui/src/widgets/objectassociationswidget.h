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

/**
\ingroup libgui
\class ObjectAssociationsWidget
\brief Implements the form to show the object's dependeces and references
*/

#ifndef OBJECT_ASSOCIATIONS_WIDGET_H
#define OBJECT_ASSOCIATIONS_WIDGET_H

#include <QWidget>
#include "ui_objectassociationswidget.h"
#include "guiglobal.h"
#include "modelwidget.h"

class __libgui ObjectAssociationsWidget: public QWidget, public Ui::ObjectAssociationsWidget {
	Q_OBJECT

	private:
		BaseObject *object;

		ModelWidget *model_wgt;

	public:
		explicit ObjectAssociationsWidget(QWidget * parent = nullptr);

		void setAttributes(ModelWidget *model_wgt, BaseObject *object, bool show_obj_name);

		void setAttributes(BaseObject *object, bool show_obj_name);

	private slots:
		void updateObjectTables();
		void handleItemSelection(const QModelIndex &index);
};

#endif
