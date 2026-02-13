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

		void configureObject(void){}

	public:
		TableTitleView();
		virtual ~TableTitleView();

		void configureObject(BaseGraphicObject *object);
		void resizeTitle(double width, double height);

		virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
};

#endif
