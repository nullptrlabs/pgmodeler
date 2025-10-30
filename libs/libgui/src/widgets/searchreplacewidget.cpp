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

#include "searchreplacewidget.h"
#include <QRegularExpression>
#include <QTimer>
#include "exception.h"

SearchReplaceWidget::SearchReplaceWidget(QPlainTextEdit *txt_edit, QWidget *parent): QWidget(parent)
{
	if(!txt_edit)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	setupUi(this);
	text_edt = txt_edit;

	search_edt->installEventFilter(this);

	search_info_lbl = new QLabel(txt_edit);
	search_info_lbl->setAutoFillBackground(true);
	search_info_lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	search_info_lbl->setVisible(false);
	search_info_lbl->setStyleSheet("padding: 10px;\
 font-size: 110%;\
 border-radius: 6px;\
 border: 1px solid palette(base);\
 background-color: palette(button)");

	search_info_timer.setInterval(2000);

	next_tb->setToolTip(next_tb->toolTip() + QString(" (%1)").arg(next_tb->shortcut().toString()));
	previous_tb->setToolTip(previous_tb->toolTip() + QString(" (%1)").arg(previous_tb->shortcut().toString()));

	connect(replace_tb, &QToolButton::clicked, this, &SearchReplaceWidget::replaceText);
	connect(replace_search_tb, &QToolButton::clicked, this, &SearchReplaceWidget::replaceFindText);
	connect(replace_all_tb, &QToolButton::clicked, this, &SearchReplaceWidget::replaceAll);

	connect(next_tb, &QToolButton::clicked, this, [this]() {
		searchText(false, true);
	});

	connect(previous_tb, &QToolButton::clicked, this, [this]() {
		searchText(true, true);
	});

	connect(search_edt, &QLineEdit::textChanged, this, [this]() {
		bool enable=!search_edt->text().isEmpty();
		next_tb->setEnabled(enable);
		previous_tb->setEnabled(enable);
		replace_tb->setEnabled(enable);
		replace_all_tb->setEnabled(enable);
		replace_search_tb->setEnabled(enable);
	});

	connect(hide_tb, &QToolButton::clicked, this, &SearchReplaceWidget::s_hideRequested);

	connect(&search_info_timer, &QTimer::timeout, search_info_lbl, &QLabel::hide);
}

bool SearchReplaceWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::KeyPress && object == search_edt)
	{
		QKeyEvent *kevent = dynamic_cast<QKeyEvent *>(event);

		if(kevent->key()==Qt::Key_Return || kevent->key()==Qt::Key_Enter)
		{
			next_tb->click();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

void SearchReplaceWidget::showEvent(QShowEvent *)
{
	search_edt->setFocus();
	replace_btns_parent->setVisible(!text_edt->isReadOnly());
	replace_lbl->setVisible(!text_edt->isReadOnly());
	replace_edt->setVisible(!text_edt->isReadOnly());
}

void SearchReplaceWidget::showSearchInfo(const QString &msg)
{
	search_info_lbl->setText(msg);
	search_info_lbl->setVisible(true);
	search_info_lbl->move((text_edt->viewport()->width() - search_info_lbl->width()) / 2,
												(text_edt->viewport()->height() - search_info_lbl->height()) / 2);
	search_info_timer.start();
}

void SearchReplaceWidget::replaceText()
{
	QTextCursor cursor=text_edt->textCursor();

	if(cursor.hasSelection())
	{
		cursor.removeSelectedText();
		cursor.insertText(replace_edt->text());
	}
}

void SearchReplaceWidget::replaceAll()
{
	QTextCursor orig_cursor, cursor=text_edt->textCursor();
	int replacements = 0;

	orig_cursor=cursor;
	cursor.setPosition(0);
	text_edt->setTextCursor(cursor);

	while(searchText(false, false))
	{
		text_edt->textCursor().insertText(replace_edt->text());
		replacements++;
	}

	text_edt->setTextCursor(orig_cursor);

	if(replacements > 0)
		showSearchInfo(tr("Replacements made: %1").arg(replacements));
	else
		showSearchInfo(tr("No replacements made!"));
}

void SearchReplaceWidget::replaceFindText()
{
	if(text_edt->textCursor().hasSelection())
	{
		replaceText();
		searchText(false, true);
	}
}

bool SearchReplaceWidget::searchText(const QString &text, bool regexp, QTextDocument::FindFlags flags)
{
	if(regexp)
	{
		QRegularExpression regexp(text);

		if(!flags.testFlag(QTextDocument::FindCaseSensitively))
			regexp.setPatternOptions(QRegularExpression::CaseInsensitiveOption);

		return text_edt->find(regexp, flags);
	}

	return text_edt->find(text, flags);
}

bool SearchReplaceWidget::searchText(bool backward, bool cyclic)
{
	QTextDocument::FindFlags flags;
	QTextCursor cursor;
	bool found=false;

	search_info_timer.stop();
	search_info_lbl->setVisible(false);

	if(backward)
		flags=QTextDocument::FindBackward;

	if(case_sensitive_tb->isChecked())
		flags=flags | QTextDocument::FindCaseSensitively;

	if(all_words_tb->isChecked())
		flags=flags | QTextDocument::FindWholeWords;

	found=searchText(search_edt->text(), regexp_tb->isChecked(), flags);

	if(!found && cyclic)
	{
		cursor=text_edt->textCursor();

		if(!backward)
			cursor.setPosition(0);
		else
			cursor.setPosition(text_edt->toPlainText().length());

		text_edt->setTextCursor(cursor);

		found = searchText(search_edt->text(), regexp_tb->isChecked(), flags);

		if(!found)
			showSearchInfo(tr("No occurrences found!"));
		else
			showSearchInfo(tr("The search returned to the starting point!"));
	}

	return found;
}
