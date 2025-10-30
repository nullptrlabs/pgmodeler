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
\class NumberedTextEditor
\brief Implements a simple text editor with a line number block attached to it. This class is
based upon the code editor example provided by Qt
*/

#ifndef NUMBERED_TEXT_EDITOR_H
#define NUMBERED_TEXT_EDITOR_H

#include <QPlainTextEdit>
#include <QToolButton>
#include <QProcess>
#include <QLabel>
#include "linenumberswidget.h"
#include "searchreplacewidget.h"

class __libgui NumberedTextEditor : public QPlainTextEdit {
	Q_OBJECT

	private:
		/*! \brief Controls globally line numbers visibility.
		 *  If this flag is set to false then toggleLineNumbers will
		 *  have no effect */
		static bool line_nums_visible,

		//! \brief Controls if current line must be highlighted
		highlight_lines;

		//! \brief Line highlight color
		static QColor line_hl_color;

		//! \brief Default font for texts
		static QFont default_font;

		//! \brief Default tab size for texts
		static double tab_width;

		static QString src_editor_app;

		static QString src_editor_app_args;

		//! \brief Widget used to expose document line numbers
		LineNumbersWidget *line_numbers_wgt;

		//! \brief Indicates if the text editor has the advanced text action buttons enabled.
		bool action_btns_enabled;

		SearchReplaceWidget *search_wgt;

		QWidget *top_widget, *editor_alert_wgt;

		QLabel *msg_lbl;

		QToolButton *load_file_btn, *edit_src_btn, *clear_btn,
		*save_file_btn,	*word_wrap_btn, *search_btn, *copy_btn;

		//! \brief The name of the temp file currently being used to edit the souce
		QString tmp_src_file;

		//! \brief The process object that holds the source code editor app
		QProcess src_editor_proc;

		/*! \brief Determines a custom font size used by the text editor widget as well as the line numbers widget
		 * The default is to use the font size of the default font (see setDefaultFont) */
		qreal custom_fnt_size;

		//! \brief The filename filters used by the save action
		QStringList file_filters;

		//! \brief The default extension for file saved by the save action
		QString default_ext;

		//! \brief Indicates the current visiblity of the line numbers in the current object
		bool show_line_nums,

		//! \brief Indicates the current visiblity of the action buttons in the current object
		show_act_btns;

		//! \brief Determines and returns the line numbers widget width
		int getLineNumbersWidth();

		void setWordWrapMode(QTextOption::WrapMode policy);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		void keyPressEvent(QKeyEvent *event)  override;

	public:
		NumberedTextEditor(QWidget * parent = nullptr, bool act_btns_enabled = false, qreal custom_fnt_size = 0);
		~NumberedTextEditor() override;

		static void setDefaultFont(const QFont &font);
		static void setLineNumbersVisible(bool value);
		static void setHighlightLines(bool value);
		static bool isHighlightLines();
		static void setLineHighlightColor(const QColor &color);
		static QColor getLineHighlightColor();
		static void setTabDistance(double value);
		static double getTabDistance();
		static void setSourceEditorApp(const QString &app);
		static void setSourceEditorAppArgs(const QString &args);

		/*! \brief Disable the custom context menu designed specifically for this class.
		This method is useful when the user needs to create another context menu that executes actions
		differents from the original ones */
		void setCustomContextMenuEnabled(bool enabled);

		//! \brief Configure the filename filters for the save action
		void setFilenameFilters(const QStringList &list, const QString &def_ext);

	private slots:
		void showContextMenu();
		void pasteCode();

		void changeSelectionToLower();
		void changeSelectionToUpper();
		void changeSelectionCase(bool lower);

		void identSelectionRight();
		void identSelectionLeft();
		void identSelection(bool ident_right);

		void loadFile();
		void saveFile();
		void editSource();
		void updateSource(int exit_code, QProcess::ExitStatus);
		void handleProcessStart();
		void handleProcessError();
		void enableEditor();

	public slots:
		void setReadOnly(bool ro);

		/*! \brief Toggles the line numbers widget visibility for this widget.
		 *  This method will have no effect if the global flag for line numbers
		 *  visiblity is set to false */
		void showLineNumbers(bool show);

		//! \brief Toggles the display of the action buttons top widget for this widget.
		void showActionButtons(bool show);

		//! \brief Grabs the keyboard input and also highlight the current line
		void setFocus();

		//! \brief Draw the line numbers according to the current visible lines
		void updateLineNumbers();

		/*! \brief Configures the size of the internal widgets (top widget, line numbers and search widget)
		 * depending on the current visible lines and the text editor height */
		void resizeWidgets();

		//! \brief Colors the background of the line where the cursor is
		void highlightCurrentLine();

		void setWordWrap(bool value);
};

#endif
