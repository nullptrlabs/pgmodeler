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
\class RelationshipWidget
\brief Implements the operations to create/edit relationships via form.
*/

#ifndef RELATIONSHIP_WIDGET_H
#define RELATIONSHIP_WIDGET_H

#include "baseobjectwidget.h"
#include "ui_relationshipwidget.h"
#include "widgets/customtablewidget.h"
#include "widgets/colorpickerwidget.h"
#include "widgets/numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"

class __libgui RelationshipWidget: public BaseObjectWidget, public Ui::RelationshipWidget {
	Q_OBJECT

	private:
		enum TabId: unsigned {
			GeneralTab,
			SettingsTab,
			AttributesTab,
			ConstraintsTab,
			SpecialPkTab,
			AdvancedTab
		};

		ColorPickerWidget *color_picker;

		NumberedTextEditor *part_bound_expr_txt;

		//! \brief Indicates if the current relationship can use the name pattern fields
		bool use_name_patterns;

		//! \brief Stores the tab objects to change the configuration of the form depending on the type of the relationship
		QWidgetList tabs;

		//! \brief Stores the original labels of the tabs
		QStringList tab_labels;

		SyntaxHighlighter *table1_hl,
		*table2_hl,
		*patterns_hl[8],
		*part_bound_expr_hl;

		//! \brief Table widgets that stores the attributes, constraint and advanced objects of relationship
		CustomTableWidget *attributes_tab,
		*constraints_tab,
		*advanced_objs_tab;

		//! \brief Lists the objects of relationship (attributes/constraints) on the respective table widget
		void listObjects(ObjectType obj_type);

		//! \brief Lists the advanced objects in the repective table widget
		void listAdvancedObjects();

		/*! \brief Shows the object data in the specified table row. The table widget is idenfied by
		 the current object type */
		void showObjectData(TableObject *object, int row);

		/*! \brief Template method that opens the editing form for the specified object.
				Class and ClassWidget should be compatible, e.g., "Column" can only be edited using ColumnWidget */
		template<class Class, class ClassWidget>
		int openEditingForm(TableObject *object, BaseObject *parent = nullptr);

	protected:
		void setAttributes(DatabaseModel *model, OperationList *op_list, PhysicalTable *src_tab,
											 PhysicalTable *dst_tab, BaseRelationship::RelType rel_type);

	public:
		RelationshipWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, BaseRelationship *base_rel);

		QSize getIdealSize();

	private slots:
		void addObject();
		void editObject(int row);
		void removeObject(int row);
		void removeObjects();
		void showAdvancedObject(int row);
		void selectCopyOptions();
		void listSpecialPkColumns();
		void duplicateObject(int curr_row, int new_row);
		void useFKGlobalSettings(bool value);
		void usePatternGlobalSettings(bool value);
		void generateBoundingExpr();

	public slots:
		void applyConfiguration() override;
		void cancelConfiguration() override;

		friend class ModelWidget;
};

#endif
