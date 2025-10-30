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
\class RuleWidget
\brief Implements the operations to create/edit rules via form.
*/

#ifndef RULE_WIDGET_H
#define RULE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_rulewidget.h"
#include "widgets/customtablewidget.h"
#include "codecompletionwidget.h"

class __libgui RuleWidget: public BaseObjectWidget, public Ui::RuleWidget {
	Q_OBJECT

	private:
		SyntaxHighlighter *cond_expr_hl,
		*command_hl;

		CodeCompletionWidget *command_cp;

		CustomTableWidget *commands_tab;

	public:
		RuleWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, BaseTable *parent_tab, Rule *rule);

	private slots:
		void handleCommand(int row);
		void editCommand(int row);

	public slots:
		void applyConfiguration() override;
};

#endif
