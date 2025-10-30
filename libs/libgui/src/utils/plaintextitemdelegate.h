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
\class PlainTextItemDelegate
\brief Implements a custom item delegate based upon a QPlainTextEdit to help edit long texts with line breaks
*/

#ifndef PLAIN_TEXT_ITEM_DELEGATE_H
#define PLAIN_TEXT_ITEM_DELEGATE_H

#include "guiglobal.h"
#include <QObject>
#include <QStyledItemDelegate>
#include <QPainter>

class __libgui PlainTextItemDelegate : public QStyledItemDelegate {
	Q_OBJECT

	private:
		static int max_display_len;

		static bool txt_editor_enabled;

		bool read_only;

		QString truncateText(const QString &text) const;

	public:
		explicit PlainTextItemDelegate(QObject * parent, bool read_only);
		~PlainTextItemDelegate() override = default;

		static void setMaxDisplayLength(int value);

		static int getMaxDisplayLength();

		static void setTextEditorEnabled(bool value);

		static bool isTextEditorEnabled();

	protected:
		//! \brief Overrides the default implementation and set the editor as read only/disable to prevent change the item data
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;

		QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
