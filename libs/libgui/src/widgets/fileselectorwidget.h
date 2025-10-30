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
\class FileSelectorWidget
\brief Implements a simple file/directory picker with customizable behavior.
*/

#ifndef FILE_SELECTOR_WIDGET_H
#define FILE_SELECTOR_WIDGET_H

#include "guiglobal.h"
#include <QFileDialog>
#include <QLabel>
#include <QRegularExpression>
#include "ui_fileselectorwidget.h"

class __libgui FileSelectorWidget: public QWidget, public Ui::FileSelectorWidget {
	Q_OBJECT

	private:
		QStringList name_filters, mime_filters;
		QString file_dlg_title, def_suffix;
		QFileDialog::AcceptMode accept_mode;
		QFileDialog::FileMode file_mode;

		QLabel *warn_ico_lbl;

		bool allow_filename_input, read_only,
		check_exec_flag, file_is_mandatory,
		file_must_exist, append_suffix;

		QRegularExpression name_regexp;

		void showWarning();

	protected:
		bool eventFilter(QObject *obj, QEvent *evnt) override;
		void resizeEvent(QResizeEvent *event) override;
		void showEvent(QShowEvent *event) override;

	public:
		FileSelectorWidget(QWidget * parent = nullptr);

		//! \brief Indicate if the selector allows manual filename input
		void setAllowFilenameInput(bool allow_fl_input);

		//! \brief Toggles the directory selection mode. The default is to always select a single file.
		void setDirectoryMode(bool dir_mode);

		/*! \brief Indicate if the accept mode that the selector uses, this is used to configure
		 * the internal QFileDialog instance (see QFileDialog::setAcceptMode) */
		void setAcceptMode(QFileDialog::AcceptMode accept_mode);

		//! \brief Configures the name filters of the internal QFileDialog (see QFileDialog::setNameFilters)
		void setNameFilters(const QStringList &filters);

		//! \brief Configures a name pattern to validate the selected file/director name
		void setNamePattern(const QString &pattern);

		//! \brief If true, the selected file must be executable otherwise an warning is displayed
		void setCheckExecutionFlag(bool value);

		//! \brief If true, the selected file must be provided otherwise an warning is displayed
		void setFileIsMandatory(bool value);

		//! \brief If true, the selected file must exist in the file system
		void setFileMustExist(bool value);

		//! \brief Configures the window title of the file dialog instance
		void setFileDialogTitle(const QString &title);

		//! \brief Configures the currently selected file in the selector
		void setSelectedFile(const QString &file);

		//! \brief Configures the mime filters of the internal QFileDialog (see QFileDialog::setMimeTypeFilters)
		void setMimeTypeFilters(const QStringList &filters);

		//! \brief Defines the default suffix (extension) of the selected file if not provided
		void setDefaultSuffix(const QString &suffix);

		/*! \brief Defines if the default suffix (extension) should be appended to the selected file if not present
		 *  Applies only to file path typed by the user. Those selected via file dialog are not changed */
		void setAppendSuffix(bool append);

		//! \brief Returns if the selector is in warning state (the warning icon is visible due to any inconsistency)
		bool hasWarning();

		//! \brief Returns the selected file/directory
		QString getSelectedFile();

		//! \brief Sets a custom warning message to the selector
		void setCustomWarning(const QString &warn_msg);

		//! \brief Clears the custom warning message
		void clearCustomWarning();

		/*! \brief Configures the selector as readonly. This will cause
		 * the clear input button to be hidden, the input field to be set as readonly and
		 * the action of the file browse button will be to open the file/directory externally */
		void setReadOnly(bool value);

		bool isReadOnly();

		void setToolTip(const QString &tooltip);

	public slots:
		void clearSelector();
		void validateSelectedFile();
		void openFileDialog();
		void openFileExternally();

	signals:
		//! \brief Signal emitted when the user selects an file/directory
		void s_fileSelected(QString);

		//! \brief Signal emitted when the user clears the selector
		void s_selectorCleared();

		/*! \brief Signal emitted when the user clears the selector or selects a file/directory
		 * The boolean param indicates if there's an file/directory selected or not */
		void s_selectorChanged(bool);
};

#endif
