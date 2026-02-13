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
\ingroup pgmodeler-sc
\class SourceEditorWidget
\brief Implements the basic source code editor with minimal code completion for schema files.
*/

#ifndef SOURCE_EDITOR_WIDGET_H
#define SOURCE_EDITOR_WIDGET_H

#include <QMenu>
#include "ui_sourceeditorwidget.h"
#include "widgets/numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"
#include "widgets/searchreplacewidget.h"
#include "widgets/codecompletionwidget.h"
#include "widgets/fileselectorwidget.h"

class SourceEditorWidget: public QWidget, public Ui::SourceEditorWidget {
	Q_OBJECT

	private:
		static attribs_map snippets;

		static QPalette def_editor_pal;

		CodeCompletionWidget *code_compl_wgt;

		FileSelectorWidget *source_file_sel;

		NumberedTextEditor *editor_txt;

		SyntaxHighlighter *editor_hl;

		SearchReplaceWidget *search_wgt;

		QString filename, curr_sytax_cfg;

		QAction *act_break_inline_ifs;

		QMenu indent_opts_menu, metachar_conv_menu;

		bool is_modified;

	public:
		explicit SourceEditorWidget(QWidget *parent = nullptr);

		/*! \brief Defines the default pallete for text editor.
		 * This is used to restore the colors of the editor after an error is raised
		 * during syntax validation and the text selection color is changed to point the
		 * error location */
		static void setDefaultEditorPalette(const QPalette &pal);

		//! \brief Returns the file being handled by the editor
		QString getFilename();

		//! \brief Returns the current modification status of the editor
		bool isModified();

		QString getCurrentSyntaxConfig();

		void convertMetaChars(bool escaped_to_meta);

	private slots:
		//! \brief Validates the syntax of the editor's content (only for schema micro-language code)
		void validateSyntax();

		//! \brief Restores the editor default colors after highlighting an syntax error in a portion of the text
		void restoreEditorPalette();

		//! \brief Applies a custom identation on the editor's content (only for schema micro-language code)
		void applyIndentation();

		//! \brief Insert the selected snippet into the editors (only for schema micro-language code)
		void handleSelectedSnippet(const QString &snippet);

		void toggleComment();

	public slots:
		void setModified(bool value);
		void loadFile(const QString &filename);
		void saveFile(const QString &filename);
		void loadSyntaxConfig(const QString &filename);

	signals:
		void s_editorModified(bool value);
};

#endif
