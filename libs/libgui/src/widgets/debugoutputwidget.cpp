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

#include "debugoutputwidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "application.h"
#include "settings/appearanceconfigwidget.h"

DebugOutputWidget::DebugOutputWidget(QWidget *parent) : QWidget(parent)
{
	dbg_output_txt = GuiUtilsNs::createNumberedTextEditor(this, true);
	dbg_output_txt->setReadOnly(true);
	dbg_output_txt->showLineNumbers(false);
	dbg_output_txt->showActionButtons(false);
	dbg_output_txt->setFilenameFilters({ tr("Text files (*.txt)"), tr("All files (*)") }, "txt");
}

void DebugOutputWidget::setLogMessages(bool value)
{
	if(value)
	{
		connect(pgApp, &Application::s_messageLogged, this,
						qOverload<QtMsgType, const QMessageLogContext &, const QString &>(&DebugOutputWidget::logMessage),
						Qt::QueuedConnection);
	}
	else
		disconnect(pgApp, &Application::s_messageLogged, this, nullptr);
}

void DebugOutputWidget::clear()
{
	dbg_output_txt->clear();
	dbg_output_txt->showActionButtons(false);
}

void DebugOutputWidget::showActionButtons(bool show)
{
	dbg_output_txt->showActionButtons(show);
}

void DebugOutputWidget::logMessage(const QString &msg, const QColor &fg_color)
{
	QTextCursor tc = dbg_output_txt->textCursor();
	int ini_pos = tc.position();

	dbg_output_txt->appendPlainText(msg);

	if(fg_color != Qt::transparent)
	{
		QTextCharFormat fmt = tc.charFormat();
		int curr_pos = tc.position();

		tc.setPosition(ini_pos, QTextCursor::MoveAnchor);
		tc.setPosition(curr_pos, QTextCursor::KeepAnchor);

		if(!CustomUiStyle::isDarkPalette())
			fmt.setForeground(fg_color.darker(130));
		else
			fmt.setForeground(fg_color);

		tc.mergeCharFormat(fmt);
	}
}

void DebugOutputWidget::logMessage(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
	static std::map<QtMsgType, QColor> msg_colors {
		{ QtDebugMsg, Qt::transparent },
		{ QtInfoMsg, Qt::cyan },
		{ QtWarningMsg, Qt::yellow },
		{ QtCriticalMsg, Qt::red },
		{ QtFatalMsg, Qt::red }
	};

	logMessage(msg, msg_colors.count(type) ?
							 msg_colors[type] : Qt::transparent);
}
