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
\class ConstraintWidget
\brief Implements the operations to create/edit constraints via form.
*/

#ifndef CONSTRAINT_WIDGET_H
#define CONSTRAINT_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_constraintwidget.h"
#include "elementstablewidget.h"
#include "columnpickerwidget.h"

class __libgui ConstraintWidget: public BaseObjectWidget, public Ui::ConstraintWidget {
	Q_OBJECT

	private:	
		ColumnPickerWidget *col_picker_wgt, *ref_col_picker_wgt;

		QFrame *info_frm;
		
		SyntaxHighlighter *expression_hl;
		
		ElementsTableWidget *excl_elems_tab;
		
		//! \brief Referenced table selector
		ObjectSelectorWidget *ref_table_sel;
				
	public:
		ConstraintWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, BaseObject *parent_obj, Constraint *constr);
		
	private slots:
		//! \brief Shows only the fields related to the selected constraint type
		void selectConstraintType();
		
		//! \brief Selects the referenced table as well updates the combo containing the referenced table columns
		void selectReferencedTable();
		
	public slots:
		void applyConfiguration() override;
};

#endif
