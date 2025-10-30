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

#include "fileselectorwidget.h"
#include "guiutilsns.h"
#include <QDesktopServices>

FileSelectorWidget::FileSelectorWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	allow_filename_input = read_only = false;
	file_is_mandatory = check_exec_flag = false;
	file_must_exist = false;
	file_mode = QFileDialog::AnyFile;

	filename_edt->setReadOnly(true);
	filename_edt->installEventFilter(this);

	warn_ico_lbl = new QLabel(this);
	warn_ico_lbl->setVisible(false);

	int sz = filename_edt->style()->pixelMetric(QStyle::PM_LineEditIconSize) +
					(filename_edt->style()->pixelMetric(QStyle::PM_LineEditIconMargin)/2);

	warn_ico_lbl->setMinimumSize(sz, sz);
	warn_ico_lbl->setMaximumSize(warn_ico_lbl->minimumSize());
	warn_ico_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	warn_ico_lbl->setScaledContents(true);
	warn_ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("alert")));
	warn_ico_lbl->setToolTip(tr("No such file or directory!"));

	connect(sel_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::openFileDialog);
	connect(rem_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::clearSelector);

	connect(filename_edt, &QLineEdit::textChanged, this, [this](const QString &text){
		validateSelectedFile();
		emit s_selectorChanged(!text.isEmpty());
	});
}

bool FileSelectorWidget::eventFilter(QObject *obj, QEvent *evnt)
{
	if(isEnabled() && evnt->type() == QEvent::MouseButtonPress &&
		 QApplication::mouseButtons() == Qt::LeftButton && obj == filename_edt)
	{
		if(!allow_filename_input && !read_only)
		{
			openFileDialog();
			return true;
		}
	}

	return QWidget::eventFilter(obj, evnt);
}

void FileSelectorWidget::resizeEvent(QResizeEvent *)
{
	warn_ico_lbl->move(filename_edt->width() - warn_ico_lbl->width() - 5,
										 (filename_edt->height() - warn_ico_lbl->height()) / 2);
}

void FileSelectorWidget::showEvent(QShowEvent *)
{
	showWarning();
}

void FileSelectorWidget::setAllowFilenameInput(bool allow_fl_input)
{
	allow_filename_input = allow_fl_input && !read_only;
	filename_edt->setReadOnly(!allow_filename_input);
}

void FileSelectorWidget::setDirectoryMode(bool dir_mode)
{
	if(dir_mode)
		file_mode = QFileDialog::Directory;
	else
	{
		if(file_must_exist)
			file_mode = QFileDialog::ExistingFile;
		else
			file_mode = QFileDialog::AnyFile;
	}

	validateSelectedFile();
}

void FileSelectorWidget::setAcceptMode(QFileDialog::AcceptMode accept_mode)
{
	this->accept_mode = accept_mode;
}

void FileSelectorWidget::setNameFilters(const QStringList &filters)
{
	name_filters = filters;
}

void FileSelectorWidget::setNamePattern(const QString &pattern)
{
	name_regexp.setPattern(pattern);
}

void FileSelectorWidget::setCheckExecutionFlag(bool value)
{
	check_exec_flag = value;
}

void FileSelectorWidget::setFileIsMandatory(bool value)
{
	file_is_mandatory = value;
	validateSelectedFile();
}

void FileSelectorWidget::setFileMustExist(bool value)
{
	file_must_exist = value;

	if(file_must_exist && file_mode == QFileDialog::AnyFile)
		file_mode = QFileDialog::ExistingFile;

	validateSelectedFile();
}

void FileSelectorWidget::setFileDialogTitle(const QString &title)
{
	file_dlg_title = title;
}

void FileSelectorWidget::setSelectedFile(const QString &file)
{
	filename_edt->setText(file);
}

void FileSelectorWidget::setMimeTypeFilters(const QStringList &filters)
{
	mime_filters = filters;
}

void FileSelectorWidget::setDefaultSuffix(const QString &suffix)
{
	def_suffix = suffix;
}

void FileSelectorWidget::setAppendSuffix(bool append)
{
	append_suffix = append;
}

bool FileSelectorWidget::hasWarning()
{
	return !warn_ico_lbl->toolTip().isEmpty();
}

QString FileSelectorWidget::getSelectedFile()
{
	if(append_suffix && allow_filename_input &&
		 file_mode != QFileDialog::Directory &&
		 !def_suffix.isEmpty())
	{
		QString filename = filename_edt->text();

		if(QFileInfo(filename).completeSuffix().isEmpty())
			filename.append("." + def_suffix);

		return filename;
	}

	return filename_edt->text();
}

void FileSelectorWidget::clearCustomWarning()
{
	warn_ico_lbl->setToolTip("");
	showWarning();
}

void FileSelectorWidget::setReadOnly(bool value)
{
	read_only = value;
	filename_edt->setReadOnly(value);
	allow_filename_input = false;

	sel_file_tb->setToolTip(value ? tr("Open in file manager") : tr("Select file"));
	rem_file_tb->setVisible(!value);

	if(value)
	{
		disconnect(sel_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::openFileDialog);
		connect(sel_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::openFileExternally);
	}
	else
	{
		connect(sel_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::openFileDialog);
		disconnect(sel_file_tb, &QToolButton::clicked, this, &FileSelectorWidget::openFileExternally);
	}

	filename_edt->adjustSize();
	adjustSize();
}

bool FileSelectorWidget::isReadOnly()
{
	return read_only;
}

void FileSelectorWidget::setToolTip(const QString &tooltip)
{
	filename_edt->setToolTip(tooltip);
}

void FileSelectorWidget::setCustomWarning(const QString &warn_msg)
{
	warn_ico_lbl->setToolTip(warn_msg);
	showWarning();
}

void FileSelectorWidget::openFileDialog()
{
	QFileDialog file_dlg;

	filename_edt->clearFocus();
	file_dlg.setWindowIcon(QIcon(GuiUtilsNs::getIconPath("pgmodeler_logo")));
	file_dlg.selectFile(filename_edt->text());
	file_dlg.setFileMode(file_mode);
	file_dlg.setAcceptMode(accept_mode);

	if(!mime_filters.isEmpty())
		file_dlg.setMimeTypeFilters(mime_filters);

	if(!name_filters.isEmpty())
		file_dlg.setNameFilters(name_filters);

	file_dlg.setWindowTitle(file_dlg_title);	
	file_dlg.setDefaultSuffix(def_suffix);

	GuiUtilsNs::restoreFileDialogState(&file_dlg);
	file_dlg.exec();
	GuiUtilsNs::saveFileDialogState(&file_dlg);

	if(file_dlg.result() == QDialog::Accepted && !file_dlg.selectedFiles().isEmpty())
	{
		filename_edt->setText(file_dlg.selectedFiles().at(0));
		emit s_fileSelected(file_dlg.selectedFiles().at(0));
	}
}

void FileSelectorWidget::openFileExternally()
{
	QDesktopServices::openUrl(QUrl("file:///" + filename_edt->text()));
}

void FileSelectorWidget::showWarning()
{
	QColor color = qApp->palette().color(QPalette::Text);
	int padding = 0;
	bool has_warn = !warn_ico_lbl->toolTip().isEmpty();

	warn_ico_lbl->setVisible(has_warn);

	if(has_warn)
	{
		color.setRgb(255, 0, 0);
		padding = warn_ico_lbl->width();
	}

	filename_edt->setStyleSheet(QString("padding: 2px %1px 2px 1px; color: %2")
															.arg(QString::number(padding), color.name()));
}

void FileSelectorWidget::validateSelectedFile()
{
	QFileInfo fi(filename_edt->text());

	warn_ico_lbl->setToolTip("");
	rem_file_tb->setEnabled(!filename_edt->text().isEmpty());

	if((file_is_mandatory && fi.absoluteFilePath().isEmpty()) ||
		 (!fi.absoluteFilePath().isEmpty() && !fi.isAbsolute()))
	{
		if(file_mode == QFileDialog::Directory)
			warn_ico_lbl->setToolTip(tr("An absolute path to a directory must be provided!"));
		else
			warn_ico_lbl->setToolTip(tr("An absolute path to a file must be provided!"));
	}
	else if(!fi.absoluteFilePath().isEmpty())
	{
		if(fi.exists() && fi.isDir() && file_mode != QFileDialog::Directory)
			warn_ico_lbl->setToolTip(tr("The provided path is not a file!"));
		else if(fi.exists() && fi.isFile() && file_mode == QFileDialog::Directory)
			warn_ico_lbl->setToolTip(tr("The provided path is not a directory!"));
		else if(file_must_exist && !fi.exists())
		{
			if(file_mode == QFileDialog::Directory)
				warn_ico_lbl->setToolTip(tr("No such directory!"));
			else
				warn_ico_lbl->setToolTip(tr("No such file!"));
		}
		else if(fi.exists())
		{
			// Validating the name pattern against the selected filename
			if(name_regexp.isValid() && !fi.absoluteFilePath().contains(name_regexp))
			{
				if(file_mode == QFileDialog::Directory)
					warn_ico_lbl->setToolTip(tr("The selected directory is not valid!"));
				else
					warn_ico_lbl->setToolTip(tr("The selected file is not valid!"));
			}
			else if(check_exec_flag && !fi.isDir() && !fi.isExecutable())
				warn_ico_lbl->setToolTip(tr("The selected file is not executable!"));
		}
	}

	showWarning();
}

void FileSelectorWidget::clearSelector()
{
	filename_edt->clear();
	filename_edt->clearFocus();
	rem_file_tb->setEnabled(false);

	emit s_selectorCleared();
	emit s_selectorChanged(false);
}
