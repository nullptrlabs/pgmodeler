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
\class TabOrderManager
\brief Auxiliary class used by configure the tab order dynamicaly in a widget.
*/

#ifndef TAB_ORDER_MANAGER_H
#define TAB_ORDER_MANAGER_H

#include <QObject>
#include <QTimer>
#include <QWidget>

class TabOrderManager: public QObject {
	Q_OBJECT

	private:
		QTimer cfg_timer;

		QWidgetList tab_order_list;

		void retrieveChildWidgets(QWidget *root_wgt, QHash<QWidget *, QWidgetList> &ord_map);

		QWidgetList getTabOrderList(const QHash<QWidget *, QWidgetList> &ord_map);

	protected:
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		explicit TabOrderManager(QWidget *parent);

		~TabOrderManager() override;

	private slots:
		void configureTabOrder();
};

#endif
