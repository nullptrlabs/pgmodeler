/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class ElementWidget
\brief Implements the operations to create/edit constraints (exclude) and indexes elements, and partition keys via form.
*/

#ifndef ELEMENT_WIDGET_H
#define ELEMENT_WIDGET_H

#include "ui_elementwidget.h"
#include "widgets/objectselectorwidget.h"
#include "partitionkey.h"
#include "syntaxhighlighter.h"

class __libgui ElementWidget: public QWidget, public Ui::ElementWidget {
	Q_OBJECT

	private:
		Element *element;

		//! \brief Parent object (table or relationship) from which the columns will be referenced on the elements
		BaseObject *parent_obj;
		
		//! \brief Syntax highlighter for element expression
		SyntaxHighlighter *elem_expr_hl;
		
		//! \brief Operator class selector
		ObjectSelectorWidget *op_class_sel,
		
		//! \brief Collation selector (only for index elements)
		*collation_sel,
		
		//! \brief Operator selector (only for exclude elements)
		*operator_sel;
		
		//! \brief Updates the column combobox with the existent columns on parent table
		void updateColumnsCombo();
		
		void setAttributes(DatabaseModel *model, BaseObject *parent_obj);

		//! \brief Enables the widget to handle index elements
		void setIndexElement(IndexElement *elem);

		//! \brief Enables the widget to handle exclude constraint elements
		void setExcludeElement(ExcludeElement *elem);

		//! \brief Enables the widget to handle partition key elements
		void setPartitionKey(PartitionKey *elem);

		//! \brief Allocates the handled element based upon the provided Class (should be child of Element)
		template<class Class>
		void createElement(Class *elem);
		
	public:
		ElementWidget(QWidget *parent = nullptr);
		
		//! \brief Configures the widget to handle the element considering its type (IndexElement, ExcludeElement, PartitionKey)
		void setAttributes(DatabaseModel *model, BaseObject *parent_obj, Element *elem);

		//! \brief Returns the configured element
		Element *getElement();

	public slots:
		void applyConfiguration();
				
	private slots:
		void selectElementObject();
};

#endif
