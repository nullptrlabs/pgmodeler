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
\class SchemaView
\brief Represents the schema in a graphical way on the object scene
*/

#ifndef SCHEMA_VIEW_H
#define SCHEMA_VIEW_H

#include "schema.h"
#include "baseobjectview.h"
#include "roundedrectitem.h"

class __libcanvas SchemaView: public BaseObjectView {
	Q_OBJECT

	private:
		QGraphicsSimpleTextItem *sch_name;

		RoundedRectItem *box;

		QPointF last_pos;

		//! \brief Indicates if all children objects are selected
		bool all_selected;

		//! \brief Stores the views and tables that belongs to this schema
		QList<BaseObjectView *> children;

		void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

		//! \brief Fills the children vector
		void fetchChildren();

		//! \brief Returns if all children are selected
		bool isChildrenSelected();

		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	public:
		SchemaView(Schema *schema);
		~SchemaView() override;

		//! \brief Visually selects all the schema children
		void selectChildren();

		unsigned getChildrenCount();

		QList<BaseObjectView *> getChildren();

		void togglePlaceholder(bool visible) override;

		void moveTo(QPointF new_pos);

	public slots:
		void configureObject() override;
};

#endif
