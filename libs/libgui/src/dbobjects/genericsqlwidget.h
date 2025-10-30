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
\class GenericSQLWidget
\brief Implements the operations to create/edit generic sql objects via form.
*/

#ifndef GENERIC_SQL_WIDGET_H
#define GENERIC_SQL_WIDGET_H

#include "ui_genericsqlwidget.h"
#include "baseobjectwidget.h"
#include "codecompletionwidget.h"
#include "dbobjects/referenceswidget.h"
#include "widgets/customtablewidget.h"
#include "widgets/objectselectorwidget.h"
#include "widgets/numberedtexteditor.h"

class __libgui GenericSQLWidget: public BaseObjectWidget, public Ui::GenericSQLWidget {
	Q_OBJECT

	private:
		ReferencesWidget *obj_refs_wgt;

		NumberedTextEditor *definition_txt, *preview_txt;

		SyntaxHighlighter *definition_hl, *preview_hl;

		CodeCompletionWidget *definition_cp;

		/*! \brief This dummy object is used to generated the code preview while the user changes the fields
		 * in form. Once the dummy is configure it is copied to the real object being handled by the form (this->object) */
		GenericSQL dummy_gsql;

	public:
		GenericSQLWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, OperationList *op_list, GenericSQL *genericsql=nullptr);

	public slots:
		void applyConfiguration() override;

	private slots:
		void updateCodePreview();
};

#endif
