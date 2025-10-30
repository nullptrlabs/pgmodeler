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
\class IndexWidget
\brief Implements the operations to create/edit indexes via form.
*/

#ifndef INDEX_WIDGET_H
#define INDEX_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_indexwidget.h"
#include "elementstablewidget.h"
#include "columnpickerwidget.h"

class __libgui IndexWidget: public BaseObjectWidget, public Ui::IndexWidget {
	Q_OBJECT

	private:
		//! \brief Syntax highlighter for predicate
		SyntaxHighlighter *predicate_hl;

		ElementsTableWidget *elements_tab;

		ColumnPickerWidget *incl_cols_picker_wgt;

	public:
		IndexWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, BaseTable *parent_obj, Index *index);

	private slots:
		void selectIndexingType();

	public slots:
		void applyConfiguration() override;
};

#endif
