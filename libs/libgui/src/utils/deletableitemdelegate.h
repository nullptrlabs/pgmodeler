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

/**
\ingroup libgui
\class DeletableItemDelegate
\brief Implements a custom item delegate exclusively for combo boxes which draws items with a tool button aside of them.
This button is used to trigger a delete action via signal.
*/

#ifndef DELETABLE_ITEM_DELEGATE_H
#define DELETABLE_ITEM_DELEGATE_H

#include "guiglobal.h"
#include <QStyledItemDelegate>
#include <QToolButton>
#include <QComboBox>

class __libgui DeletableItemDelegate: public QStyledItemDelegate {
	Q_OBJECT

	private:
		//! \brief The tool button which is displayed aside the selected item in the combo box
		QToolButton *del_btn;

		QComboBox *combo;

	public:
		//! \brief This class can be constructed only with combo boxes. One can specify custom tooltips and custom shortcut keys for the delete action
		DeletableItemDelegate(QComboBox *parent, const QString &tooltip = QString(), QKeySequence shortcut = QKeySequence("Ctrl+Shift+Delete"));

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	signals:
		//! \brief Signal emitted whenever the delete button is triggered either from click or shortcut
		void s_itemDeleteRequested(int index);
};
#endif
