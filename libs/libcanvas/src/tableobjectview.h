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
\ingroup libcanvas
\class TableObjectView
\brief implements the graphical representation for table objects (columns, constraints, triggers, rules, indexes) as well the view references
*/

#ifndef TABLE_OBJECT_VIEW_H
#define TABLE_OBJECT_VIEW_H

#include "baseobjectview.h"
#include "column.h"
#include "simplecolumn.h"
#include "pgsqltypes/constrainttype.h"
#include "pgsqltypes/pgsqltype.h"

class __libcanvas TableObjectView: public BaseObjectView {
	Q_OBJECT

	private:
		static bool hide_sch_name_usr_type;

		//! \brief Table object descriptor
		QGraphicsItem *descriptor;

		bool fake_selection;

		//! \brief Labels used to show objects informatoni (name, type, constraints/aliases)
		QGraphicsSimpleTextItem *lables[3];

		/*! \brief Configures the descriptor object according to the source object.
		 The constraint type parameter is only used when the source object is a
		 column  and is used to format the descriptor indication that the column
		 has a constraint */
		void configureDescriptor(ConstraintType constr_type = ConstraintType::Null);

		QVariant itemChange(GraphicsItemChange, const QVariant &value) override
		{
			return value;
		}

		void calculateBoundingRect();

		/*! \brief Returns the type name either schema-qualified or not depending on the flag hide_sch_name_usr_type
		 * and if the provider type is an user-defined one */
		QString formatUserTypeName(PgSqlType type);

	public:
		static const QString	TypeSeparator,
		ConstrSeparator,
		TextUnique,
		TextExclude,
		TextCheck,
		TextPrimaryKey,
		TextForeignKey,
		TextNotNull,
		ConstrDelimStart,
		ConstrDelimEnd;

		enum ChildObjectId: unsigned {
			ObjDescriptor,
			NameLabel,
			TypeLabel,
			ConstrAliasLabel
		};

		TableObjectView(TableObject *object=nullptr);

		~TableObjectView() override;

		//! \brief Configures a item from a SimpleColumn instance
		void configureObject(const SimpleColumn &col);

		//! \brief Configures the object as a table object
		void configureObject() override;

		//! \brief Sets the horizontal position of the specified child object (index)
		void setChildObjectXPos(ChildObjectId obj_id, double px);

		//! \brief Returns the child object at the specified index
		QGraphicsItem *getChildObject(ChildObjectId obj_id);

		/*! \brief Returns a formatted string containing the keywords indicating the constraints
		 that is applyed to the passed column */
		static QString getConstraintString(Column *column);

		void setFakeSelection(bool value);
		bool hasFakeSelection();

		static void setSchemaNameUserType(bool value);
		static bool isHideSchemaNameUserType();

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

		QRectF boundingRect(void) const override;
		virtual void configureObjectSelection();
		void configureObjectShadow(void) = delete;
};

#endif
