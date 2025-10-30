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
\class AggregateWidget
\brief Implements the operations to create/edit aggregates via form.
*/

#ifndef AGGREGATE_WIDGET_H
#define AGGREGATE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_aggregatewidget.h"
#include "pgsqltypewidget.h"
#include "widgets/customtablewidget.h"

class __libgui AggregateWidget: public BaseObjectWidget, public Ui::AggregateWidget {
	Q_OBJECT

	private:	
		//! \brief Function input data type widget
		PgSQLTypeWidget *input_type,
		
		//! \brief Function state type widget
		*state_type;
		
		/*! \brief Table that stores the multiple input data types. The elements
		in this table are converted to class PgSQLType and assigned as input types
		to the aggregat */
		CustomTableWidget *input_types_tab;
		
		//! \brief Syntax highlighter used by the initial condition field
		SyntaxHighlighter *initial_cond_hl;
		
		//! \brief Aggregate functions selectors
		ObjectSelectorWidget *sort_op_sel,
		*final_func_sel,
		*transition_func_sel;
		
	public:
		AggregateWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Aggregate *aggregate);
		
	private slots:
		void handleDataType(int row);
		
	public slots:
		void applyConfiguration() override;
};

#endif
