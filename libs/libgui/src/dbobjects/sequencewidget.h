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
\class SequenceWidget
\brief Implements the operations to create/edit sequences via form.
*/

#ifndef SEQUENCE_WIDGET_H
#define SEQUENCE_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_sequencewidget.h"

class __libgui SequenceWidget: public BaseObjectWidget, public Ui::SequenceWidget {
	Q_OBJECT

	private:
		ObjectSelectorWidget *column_sel;

	protected:
		void setAttributes(DatabaseModel *model, Sequence *sequence);

		/*! \brief This auxiliary method is used in ColumnWidget to enable the user to edit the underlying sequence's attributes
		 * when handling an indentity column. This method disable almost all fields in the form letting only the ones related to
		 * sequence's values configuration enabled */
		void setAttributesReadonly(DatabaseModel *model, OperationList *op_list, Schema *schema, Sequence *sequence, Column *column);

	public:
		SequenceWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Sequence *sequence);

	public slots:
		void applyConfiguration() override;

	private slots:
		void setDefaultValues();

	friend class ColumnWidget;
};

#endif
