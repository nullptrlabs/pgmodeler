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

#include "htmlitemdelegate.h"
#include <QTextDocument>
#include <QLineEdit>

const QRegularExpression HtmlItemDelegate::TagRegExp {"<(\\/)?[a-z]+(>|\\/>)"};

HtmlItemDelegate::HtmlItemDelegate(QObject *parent, bool ignore_tags_sz_hint) : PlainTextItemDelegate(parent, true)
{
	this->ignore_tags_sz_hint = ignore_tags_sz_hint;
}

QSize HtmlItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize sz = PlainTextItemDelegate::sizeHint(option, index);

	if(ignore_tags_sz_hint)
	{
		QString text = index.data().toString();

		if(text.contains(TagRegExp))
		{
			/* When ignoring html tags in size hint calculation we erase all tags from
			 * the text and calculates the new size */
			text.remove(TagRegExp);
			sz.setWidth(option.fontMetrics.boundingRect(text).width() +
									(option.decorationSize.width() * 2));
		}
	}

	return sz;
}

void HtmlItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	static QRect rect;
	static QColor bg_color;
	static QIcon ico;
	static QString text;
	static QSize ico_sz;
	static int dy = 0;

	text = index.data().toString();
	ico = index.data(Qt::DecorationRole).value<QIcon>();
	rect = option.rect;
	ico_sz = option.decorationSize;

	//Painting the correct background color according to the item state
	if((option.state & QStyle::State_Selected) == QStyle::State_Selected)
		//Selected
		bg_color = option.palette.color(QPalette::Highlight);
	else if(option.features == QStyleOptionViewItem::Alternate)
		//Alternate color
		bg_color = option.palette.color(QPalette::AlternateBase);
	else
		//Base color
		bg_color = option.palette.color(QPalette::Base);

	painter->save();

	//Repaint the text area
	painter->fillRect(rect, bg_color);

	dy = abs(rect.height() - ico_sz.height()) / 2;
	ico.paint(painter, QRect(QPoint(rect.left() + 1, rect.top() + dy),  ico_sz));

	if(!text.contains(TagRegExp))
	{
		static QSize txt_sz;

		if((option.state & QStyle::State_Enabled) == QStyle::State_Enabled)
			painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
		else
			painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));

		txt_sz = option.fontMetrics.boundingRect(rect, Qt::Horizontal, text).size();
		dy = abs(rect.height() - txt_sz.height()) / 2;

		if(!ico.isNull())
			rect.translate(ico_sz.width() + 5, dy);
		else
			rect.translate(0, dy);

		painter->drawText(rect, text);
	}
	else
	{
		static QTextDocument doc;

		text.replace("\n", "<br/>");
		doc.setHtml(text);

		//Set the text to a html document instance and draw it to the painter
		dy = abs(rect.height() - doc.size().height())/2;
		painter->translate(rect.left() + ico_sz.width() + 1, rect.top() - dy);
		doc.drawContents(painter);
	}

	painter->restore();
}
