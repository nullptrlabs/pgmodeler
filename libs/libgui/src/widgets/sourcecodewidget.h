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
\class SourceCodeWidget
\brief Implements the operation to visualize object's source code.
*/

#ifndef SOURCE_CODE_WIDGET_H
#define SOURCE_CODE_WIDGET_H

#include "ui_sourcecodewidget.h"
#include "dbobjects/baseobjectwidget.h"
#include "numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"

class __libgui SourceCodeWidget: public BaseObjectWidget, public Ui::SourceCodeWidget {
	Q_OBJECT

	private:
		std::vector<BaseObject *> objects;

		NumberedTextEditor *sqlcode_txt, *xmlcode_txt;

		SyntaxHighlighter *hl_sqlcode, *hl_xmlcode;

		int prev_pg_ver, prev_code_opt;

		void generateSQLCode();

		void generateXMLCode();

	public:
		SourceCodeWidget(QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, const std::vector<BaseObject *> &objects);

		/* Forcing the widget to indicate that the handled object is not protected
		 * even if it IS protected. This will avoid the ok button of the parent dialog
		 * to be disabled */
		virtual bool isHandledObjectProtected(){ return false; }

	public slots:
		void applyConfiguration();

	private slots:
		void generateSourceCode(int def_type);
		void saveSQLCode();
};

#endif

