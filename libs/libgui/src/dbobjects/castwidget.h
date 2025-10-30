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
\class CastWidget
\brief Implements the operations to create/edit type casts via form.
*/

#ifndef CAST_WIDGET_H
#define CAST_WIDGET_H

#include "baseobjectwidget.h"
#include "pgsqltypewidget.h"
#include "ui_castwidget.h"

class __libgui CastWidget: public BaseObjectWidget, public Ui::CastWidget {
	Q_OBJECT

	private:	
		//! \brief Datatype configurations
		PgSQLTypeWidget *src_datatype, *trg_datatype;
		
		//! \brief Conversion function selector
		ObjectSelectorWidget *conv_func_sel;
		
	public:
		CastWidget(QWidget * parent = nullptr);
		void setAttributes(DatabaseModel *model, OperationList *op_list, Cast *cast);
		
	public slots:
		void applyConfiguration() override;
};

#endif
