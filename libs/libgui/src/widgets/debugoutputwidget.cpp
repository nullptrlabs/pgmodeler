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

#include "debugoutputwidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "application.h"
#include <QScrollBar>

DebugOutputWidget::DebugOutputWidget(QWidget *parent) : QWidget(parent)
{
	dbg_output_txt = GuiUtilsNs::createNumberedTextEditor(this, true);
	dbg_output_txt->setReadOnly(true);
	dbg_output_txt->showLineNumbers(false);
	dbg_output_txt->showActionButtons(false);
	dbg_output_txt->setFilenameFilters({ tr("Text files (*.txt)"), tr("All files (*)") }, "txt");

	/* This connection forces the update of vertical scrollbar maximum value
	 * after the document changes (lines are added) so the document can
	 * automatically be scrolled to the last line always */
	connect(dbg_output_txt->verticalScrollBar(), &QScrollBar::rangeChanged, this, [this](int, int max) {
		dbg_output_txt->verticalScrollBar()->setValue(max);
	});
}

void DebugOutputWidget::setLogAppMessages(bool value)
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
	/* Creates a cursor directly in the document
	 * so we can directly change it without a virtual posicion via textCursor() */
	QTextCursor tc(dbg_output_txt->document());
	tc.movePosition(QTextCursor::End);

	/* Inserts a paragraph separator so a new line can be added
	 * if the document is not empty */
	if(!dbg_output_txt->document()->isEmpty())
		tc.insertBlock();

	QTextCharFormat fmt;

	// Defining the coloring of the current line
	if(fg_color != Qt::transparent)
	{
		if(!CustomUiStyle::isDarkPalette())
			fmt.setForeground(fg_color.darker(130));
		else
			fmt.setForeground(fg_color);
	}

	/* Insert the message/line with the defined formatting
	 * and move the cursor to the start of the new line */
	tc.insertText(msg, fmt);
	tc.movePosition(QTextCursor::StartOfBlock);

	/* Updates the document cursor so the position change
	 * and new line addition can take effect */
	dbg_output_txt->setTextCursor(tc);

	/* Forcing the text to be always at left avoiding
	 * the scrolling to the end of the line */
	dbg_output_txt->horizontalScrollBar()->setValue(0);
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
