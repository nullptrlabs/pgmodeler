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
\class FunctionWidget
\brief Implements the operations to create/edit functions via form.
*/

#ifndef FUNCTION_WIDGET_H
#define FUNCTION_WIDGET_H

#include "basefunctionwidget.h"
#include "ui_functionwidget.h"

class __libgui FunctionWidget: public BaseFunctionWidget, public Ui::FunctionWidget  {
	Q_OBJECT

	private:
		QWidget *ret_type_parent;

		//! \brief Widget used to configure the function's return type
		PgSQLTypeWidget *ret_type;

		//! \brief Table that represents the table returned by the function
		CustomTableWidget *return_tab;

		//! \brief Validates the new function configuration in relation to the other objects that references it.
		void validateConfiguredFunction();

	public:
		FunctionWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Function *func);

	private slots:
		void alternateReturnTypes();

		//! \brief Shows the parameter configuration form
		void showParameterForm();

		//! \brief Shows the configured parameter on the table that called the form
		void handleParameter(Parameter param, int result);

		//! brief Duplicates the parameter in the curr_row placing it in new_row
		void duplicateParameter(int curr_row, int new_row);

	public slots:
		void applyConfiguration() override;
};

#endif
