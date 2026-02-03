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
\class ObjectTypesListWidget
\brief Implements a list widget specialized in show object types where the user can change check state of the items.
*/

#ifndef OBJECT_TYPES_LIST_WIDGET_H
#define OBJECT_TYPES_LIST_WIDGET_H

#include "guiglobal.h"
#include "baseobject.h"
#include "ui_objecttypeslistwidget.h"

class __libgui ObjectTypesListWidget : public QWidget, public Ui::ObjectTypesListWidget {
	Q_OBJECT

	public:
		explicit ObjectTypesListWidget(QWidget *parent = nullptr, const std::vector<ObjectType> &excl_types = {});

		void setTypeNamesCheckState(const QStringList &obj_types, Qt::CheckState state);
		void setTypesCheckState(const std::vector<ObjectType> &obj_types, Qt::CheckState state);
		void setTypesCheckState(Qt::CheckState state);

		std::vector<ObjectType> getTypesPerCheckState(Qt::CheckState state);
		QStringList getTypeNamesPerCheckState(Qt::CheckState state);
		int getTypesCountPerCheckState(Qt::CheckState state);

	signals:
		void s_typeCheckStateChanged(ObjectType, Qt::CheckState);
		void s_typesCheckStateChanged(Qt::CheckState);
};

#endif
