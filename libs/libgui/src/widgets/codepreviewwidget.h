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
\class CodePreviewWidget
\brief Implements the operation to visualize object's source code.
*/

#ifndef CODE_PREVIEW_WIDGET_H
#define CODE_PREVIEW_WIDGET_H

#include "ui_codepreviewwidget.h"
#include "numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"
#include "databasemodel.h"

class __libgui CodePreviewWidget: public QWidget, public Ui::CodePreviewWidget {
	Q_OBJECT

	private:
		std::vector<BaseObject *> objects;

		DatabaseModel *dbmodel;

		BaseObject *object;

		NumberedTextEditor *sqlcode_txt, *xmlcode_txt;

		SyntaxHighlighter *hl_sqlcode, *hl_xmlcode;

		int prev_pg_ver, prev_code_opt;

		void generateSQLCode();

		void generateXMLCode();

	public:
		CodePreviewWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, const std::vector<BaseObject *> &objs);

	private slots:
		void generateSourceCode(int def_type);
};

#endif
