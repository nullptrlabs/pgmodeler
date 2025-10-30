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
\class DomainWidget
\brief Implements the operations to create/edit domains via form.
*/

#ifndef DOMAIN_WIDGET_H
#define DOMAIN_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_domainwidget.h"
#include "pgsqltypewidget.h"
#include "widgets/customtablewidget.h"

class __libgui DomainWidget: public BaseObjectWidget, public Ui::DomainWidget {
	Q_OBJECT

	private:	
		SyntaxHighlighter *check_expr_hl;
		PgSQLTypeWidget *data_type;
		CustomTableWidget *constr_tab;
		
	public:
		DomainWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Domain *domain);
		
	private slots:
		void handleConstraint(int row);
		void editConstraint(int row);

	public slots:
		void applyConfiguration() override;
};

#endif
