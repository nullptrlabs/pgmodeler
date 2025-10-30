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
\class TextboxWidget
\brief Implements the operations to create/edit textboxes via form.
*/

#ifndef TEXTBOX_WIDGET_H
#define TEXTBOX_WIDGET_H

#include "ui_textboxwidget.h"
#include "baseobjectwidget.h"

class __libgui TextboxWidget: public BaseObjectWidget, public Ui::TextboxWidget {
	Q_OBJECT

	public:
		TextboxWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Textbox *txtbox=nullptr, double obj_px=DNaN, double obj_py=DNaN);

	private slots:
		void selectTextColor();

	public slots:
		void applyConfiguration() override;
};

#endif
