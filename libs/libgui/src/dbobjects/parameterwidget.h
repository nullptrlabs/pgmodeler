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
\class ParameterWidget
\brief Implements the operations to create/edit function parameters via form.
*/

#ifndef PARAMETER_WIDGET_H
#define PARAMETER_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_parameterwidget.h"
#include "pgsqltypewidget.h"

class __libgui ParameterWidget: public BaseObjectWidget, public Ui::ParameterWidget {
	Q_OBJECT

	private:
		PgSQLTypeWidget *data_type;
		Parameter parameter;

	public:
		ParameterWidget(QWidget * parent = nullptr);
		void setAttributes(Parameter parameter, DatabaseModel *model);
		Parameter getParameter();

	public slots:
		void applyConfiguration() override;

	private slots:
		void enableVariadic();
};

#endif
