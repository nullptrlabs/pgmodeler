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
\class HtmlItemDelegate
\brief Implements a custom item delegate used in treewidget to paint items that contains html as text
*/

#ifndef HTML_ITEM_DELEGATE_H
#define HTML_ITEM_DELEGATE_H

#include "guiglobal.h"
#include <QObject>
#include <QPainter>
#include "plaintextitemdelegate.h"

class __libgui HtmlItemDelegate : public PlainTextItemDelegate {
	Q_OBJECT

	private:
		/*! \brief Indicates whether the html tags should be ignored when
		 *  determining the size hint of the item. See sizeHint() */
		bool ignore_tags_sz_hint;

	public:
		static const QRegularExpression TagRegExp;

		HtmlItemDelegate(QObject * parent = nullptr, bool ignore_tags_sz_hint = false);
		~HtmlItemDelegate() override = default;

		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	protected:
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
