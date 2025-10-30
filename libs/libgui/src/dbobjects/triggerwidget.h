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
\class TriggerWidget
\brief Implements the operations to create/edit triggers via form.
*/

#ifndef TRIGGER_WIDGET_H
#define TRIGGER_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_triggerwidget.h"
#include "widgets/customtablewidget.h"
#include "dbobjects/columnpickerwidget.h"
#include "utils/syntaxhighlighter.h"

class __libgui TriggerWidget: public BaseObjectWidget, public Ui::TriggerWidget {
	Q_OBJECT

	private:
		ColumnPickerWidget *col_picker_wgt;

		//! \brief Conditional expression syntax highlighter
		SyntaxHighlighter *cond_expr_hl;

		//! \brief Object tables used to control columns and agruments
		CustomTableWidget *arguments_tab;

		//! \brief Selectors for reference table and trigger function
		ObjectSelectorWidget *ref_table_sel,

		*function_sel;

	public:
		TriggerWidget(QWidget * parent  = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, BaseTable *parent_table, Trigger *trigger);

	private slots:
		void selectUpdateEvent();
		void handleArgument(int lin_idx);
		void editArgument(int lin_idx);
		void setConstraintTrigger(bool value);
		void enableTransitionTableNames();

	public slots:
		void applyConfiguration() override;
};

#endif
