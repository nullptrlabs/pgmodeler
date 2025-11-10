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

#include "deletableitemdelegate.h"
#include "guiutilsns.h"
#include <QPainter>

DeletableItemDelegate::DeletableItemDelegate(QComboBox *parent, const QString &tooltip, QKeySequence shortcut) : QStyledItemDelegate(parent)
{
	combo = parent;
	del_btn = new QToolButton(combo->view());
	del_btn->setVisible(false);
	del_btn->setIcon(GuiUtilsNs::getIcon("delete"));
	del_btn->setIconSize(QSize(parent->iconSize().width() * 0.80, parent->iconSize().height() * 0.80));
	del_btn->setAutoRaise(true);
	del_btn->setShortcut(shortcut);
	del_btn->setToolTip(tooltip + QString(" (%1)").arg(del_btn->shortcut().toString()));

	QObject::connect(del_btn, &QToolButton::clicked, this, [this]() {
		emit s_itemDeleteRequested(combo->view()->currentIndex().row());
	});
}

void DeletableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	painter->save();
	QStyledItemDelegate::paint(painter, option, index);
	painter->restore();

	QStyle::State sel_state = (QStyle::State_Active | QStyle::State_Enabled |
														 QStyle::State_HasFocus | QStyle::State_Selected);

	if(index.row() > 0 &&
		 (option.state == sel_state ||
			option.state == (sel_state | QStyle::State_MouseOver)))
	{
		del_btn->resize(option.rect.height(), option.rect.height());
		del_btn->move(option.rect.width() - del_btn->width(), option.rect.top());
		del_btn->setVisible(true);
	}
	else if(option.state == (QStyle::State_Active | QStyle::State_Enabled))
		del_btn->setVisible(false);
}
