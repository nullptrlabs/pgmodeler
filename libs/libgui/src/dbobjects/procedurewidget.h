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
\class ProcedureWidget
\brief Implements the operations to create/edit procedures via form.
*/

#ifndef PROCEDURE_WIDGET_H
#define PROCEDURE_WIDGET_H

#include "basefunctionwidget.h"

class __libgui ProcedureWidget: public BaseFunctionWidget  {
	Q_OBJECT

	public:
		ProcedureWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Procedure *proc);

	private slots:
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
