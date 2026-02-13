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
\ingroup pgmodeler-se
\class SchemaEditorForm
\brief Implements the pgModeler's schema files editor and syntax checker.
*/

#ifndef SCHEMA_EDITOR_FORM_H
#define SCHEMA_EDITOR_FORM_H

#include <QMenu>
#include "ui_schemaeditorform.h"
#include "widgets/numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"
#include "widgets/fileselectorwidget.h"

class SchemaEditorForm: public QWidget, public Ui::SchemaEditorForm {
	Q_OBJECT

	private:
		static const QString UntitledFile;

		NumberedTextEditor *syntax_txt, *dtd_txt;

		SyntaxHighlighter *syntax_hl, *dtd_hl;

		FileSelectorWidget *syntax_conf_sel;

		QActionGroup *stx_action_grp;

		QMenu syntax_cfg_menu;

		void showEvent(QShowEvent *) override;

		void closeEvent(QCloseEvent *event) override;

		bool eventFilter(QObject *object, QEvent *event) override;

		QStringList showFileDialog(bool save_mode);

	public:
		explicit SchemaEditorForm(QWidget *parent = nullptr);
		void loadFiles(const QStringList &filenames);
		bool hasModifiedEditors();

	private slots:
		void loadSyntaxConfig();
		void applySyntaxConfig(bool from_temp_file = true);
		void saveSyntaxConfig();
		void addEditorTab(const QString &filename = "");
		void closeEditorTab(int idx, bool confirm_close = true);
		void loadFile();
		void saveFile(bool save_as = false);
		void setTabModified(bool modified);
		void indentAll();
		void saveAll();
		void closeAll();
		void loadSyntaxFromCurrentTab();
};

#endif
