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
\class SchemaWidget
\brief Implements the operations to create/edit schemas via form.
*/

#ifndef SCHEMA_WIDGET_H
#define SCHEMA_WIDGET_H

#include "baseobjectwidget.h"
#include "widgets/colorpickerwidget.h"
#include "ui_schemawidget.h"

class __libgui SchemaWidget: public BaseObjectWidget, public Ui::SchemaWidget {
	Q_OBJECT

	private:
		ColorPickerWidget *fill_color_picker,

		*name_color_picker;

	public:
		SchemaWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema);

	public slots:
		void applyConfiguration() override;
};

#endif
