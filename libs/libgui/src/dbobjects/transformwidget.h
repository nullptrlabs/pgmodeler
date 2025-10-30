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
\class TransformWidget
\brief Implements the operations to create/edit transforms via form.
*/

#ifndef TRANSFORM_WIDGET_H
#define TRANSFORM_WIDGET_H

#include "baseobjectwidget.h"
#include "pgsqltypewidget.h"
#include "widgets/objectselectorwidget.h"
#include "ui_transformwidget.h"

class __libgui TransformWidget: public BaseObjectWidget, public Ui::TransformWidget {
	Q_OBJECT

	private:
		PgSQLTypeWidget *type_wgt;

		ObjectSelectorWidget *from_sql_func_sel, *to_sql_func_sel, *language_sel;

	public:
		TransformWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Transform *transform);

	public slots:
		void applyConfiguration() override;
};

#endif
