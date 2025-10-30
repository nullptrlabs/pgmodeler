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
\class TableTitleView
\brief Implements the graphical representation for table title
*/

#ifndef TABLE_TITLE_VIEW_H
#define TABLE_TITLE_VIEW_H

#include "baseobjectview.h"
#include "roundedrectitem.h"

class __libcanvas TableTitleView: public BaseObjectView {
	Q_OBJECT

	private:
		//! \brief Rounded rectangle object that defines the title border
		RoundedRectItem *box;

		//! \brief Graphical texts that is used to store the object name and schema name
		QGraphicsSimpleTextItem *obj_name,
		*schema_name;

		void configureObject(void) override {}

	public:
		TableTitleView();
		~TableTitleView() override;

		void configureObject(BaseGraphicObject *object);
		void resizeTitle(double width, double height);
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
};

#endif
