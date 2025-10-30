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
\class DatabaseWidget
\brief Implements the operations to create/edit database attributes via form.
*/

#ifndef DATABASE_WIDGET_H
#define DATABASE_WIDGET_H

#include "ui_databasewidget.h"
#include "baseobjectwidget.h"

class __libgui DatabaseWidget: public BaseObjectWidget, public Ui::DatabaseWidget {
	Q_OBJECT

	private:
		ObjectSelectorWidget *def_collation_sel,
		*def_schema_sel,
		*def_owner_sel,
		*def_tablespace_sel;
		
	public:
		DatabaseWidget(QWidget * parent = nullptr);
		
		void setAttributes(DatabaseModel *model);
		
	public slots:
		void applyConfiguration() override;
};

#endif
