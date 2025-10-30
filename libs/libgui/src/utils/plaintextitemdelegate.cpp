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

#include "plaintextitemdelegate.h"
#include <QPlainTextEdit>
#include <QLineEdit>

int PlainTextItemDelegate::max_display_len {500};
bool PlainTextItemDelegate::txt_editor_enabled {true};

PlainTextItemDelegate::PlainTextItemDelegate(QObject *parent, bool read_only) : QStyledItemDelegate(parent)
{
	this->read_only = read_only;
}

void PlainTextItemDelegate::setMaxDisplayLength(int value)
{
	max_display_len = value;
}

int PlainTextItemDelegate::getMaxDisplayLength()
{
	return max_display_len;
}

void PlainTextItemDelegate::setTextEditorEnabled(bool value)
{
	txt_editor_enabled = value;
}

bool PlainTextItemDelegate::isTextEditorEnabled()
{
	return txt_editor_enabled;
}

void PlainTextItemDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
	QPlainTextEdit *text_edt=qobject_cast<QPlainTextEdit *>(editor);
	QLineEdit *line_edt=qobject_cast<QLineEdit *>(editor);

	if(text_edt)
	{
		text_edt->setReadOnly(read_only);
		text_edt->setPlainText(index.data().toString());
		text_edt->selectAll();
	}
	else if(line_edt)
	{
		line_edt->setReadOnly(read_only);
		line_edt->setText(index.data().toString());
	}
	else
		QStyledItemDelegate::setEditorData(editor, index);
}

QWidget *PlainTextItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
	QString text = index.data().toString();

	if(!txt_editor_enabled && max_display_len > 0 && text.length() >= max_display_len)
		return nullptr;

	QWidget *editor = nullptr;

	if(text.contains(QChar::LineFeed))
	{
		editor = new QPlainTextEdit(parent);
		qobject_cast<QPlainTextEdit *>(editor)->setFrameShape(QFrame::NoFrame);
	}
	else
	{
		editor = new QLineEdit(parent);
		qobject_cast<QLineEdit *>(editor)->setFrame(false);
	}

	return editor;
}

void PlainTextItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(max_display_len <= 0 || index.data().toString().length() < max_display_len)
		QStyledItemDelegate::paint(painter, option, index);
	else
	{
		QStyleOptionViewItem idx_opt;
		initStyleOption(&idx_opt, index);

		painter->save();
		painter->setFont(idx_opt.font);

		// If the index is selected we need to preserve the selection foreground and background color
		if (option.state & QStyle::State_Selected &&
				option.state & QStyle::State_Active)
		{
			painter->setPen(option.palette.color(QPalette::HighlightedText));
			painter->fillRect(option.rect, option.backgroundBrush);
		}
		/* When not selected, we just use the colors configured in the idx_opt the item with the correct
		 * foreground and background colors */
		else
		{
			painter->fillRect(option.rect, idx_opt.backgroundBrush);
			painter->setPen(idx_opt.palette.color(QPalette::Text));
		}

		painter->drawText(option.rect,
											option.displayAlignment,
											truncateText(idx_opt.text));
		painter->restore();
	}
}

QSize PlainTextItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QString text = index.data().toString();

	if(max_display_len <= 0 || text.length() < max_display_len)
		return QStyledItemDelegate::sizeHint(option, index);

	return option.fontMetrics.boundingRect(option.rect,
																				 option.displayAlignment,
																				 truncateText(text)).size();
}

QString PlainTextItemDelegate::truncateText(const QString &text) const
{
	return text.mid(0, max_display_len) + " ...";
}
