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
\class BaseFunctionWidget
\brief Implements the operations to create/edit functions/procedure via form.
This widget is specialized in FunctionWidget and ProcedureWidget in order to
treat other attribute of those classes.
*/

#ifndef BASE_FUNCTION_WIDGET_H
#define BASE_FUNCTION_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_basefunctionwidget.h"
#include "widgets/customtablewidget.h"
#include "codecompletionwidget.h"
#include "widgets/numberedtexteditor.h"
#include "pgsqltypewidget.h"

class __libgui BaseFunctionWidget: public BaseObjectWidget, public Ui::BaseFunctionWidget  {
	Q_OBJECT

	protected:
		NumberedTextEditor *source_code_txt;

		//! \brief Function's source code highlighter
		SyntaxHighlighter *source_code_hl;

		//! \brief Function's source code completion
		CodeCompletionWidget *source_code_cp;

		//! \brief Table used to store the function's parameters
		CustomTableWidget *parameters_tab,

		//! \brief Table used to store the function's transform types
		*transform_types_tab,

		//! \brief Table used to store the function's configuration parameters
		*config_params_tab;

		//! \brief Widget used to select transform types of the function's
		PgSQLTypeWidget *transform_type_wgt;

		//! \brief Returns a parameter configured based upon the specified table and line
		Parameter getParameter(CustomTableWidget *params_tab, unsigned row, bool set_param_modes);

		//! \brief Shows the parameter data on the specified table at the specified line
		void showParameterData(CustomTableWidget *params_tab, Parameter param, unsigned row, bool show_param_modes);

		//! \brief Shows the parameter configuration form
		void showParameterForm(CustomTableWidget *params_tab, bool enable_param_modes);

		//! \brief Shows the configured parameter on the table that called the form
		void handleParameter(CustomTableWidget *params_tab, Parameter param, int result, bool handle_param_modes);

		//! \brief Duplicates the parameter in the curr_row placing it in new_row
		void duplicateParameter(CustomTableWidget *params_tab, int curr_row, int new_row, bool dup_param_modes);

		//! \brief Applies the values in the widget to the base function's instance
		void applyBasicConfiguration(BaseFunction *func);

	public:
		BaseFunctionWidget(QWidget * parent, ObjectType obj_type);

		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, BaseFunction *func);

	private slots:
		/*! \brief Selects the language used by the function and if available loads the
		syntax highlight configuration for the selected language. */
		void selectLanguage();
};

#endif
