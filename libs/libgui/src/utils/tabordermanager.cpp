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

#include "tabordermanager.h"
#include <QKeyEvent>

TabOrderManager::TabOrderManager(QWidget *parent)	: QObject { parent }
{
	connect(&cfg_timer, &QTimer::timeout, this, &TabOrderManager::configureTabOrder);
}

bool TabOrderManager::eventFilter(QObject *object, QEvent *event)
{
	/* We (re)start the configuration timer every time the LayoutRequest event
	 * from parent happens or a EnabledChange happens in any of its children.
	 * This will avoid repeatedly call configureTabOrder(), calling it only
	 * once when all events are processed */
	if((object == parent() && event->type() == QEvent::LayoutRequest) ||
		 (object != parent() && event->type() == QEvent::EnabledChange))
	{
		cfg_timer.start(100);
	}

	return QObject::eventFilter(object, event);
}

void TabOrderManager::configureTabOrder()
{
	QWidget *parent_wgt = qobject_cast<QWidget *>(parent());

	if(!parent_wgt->isVisible())
		return;

	cfg_timer.stop();

	// We need to work only on direct children only
	QWidgetList child_wgts = parent_wgt->findChildren<QWidget *>(Qt::FindDirectChildrenOnly);

	/* Uninstalling the event filter installed in the previous iteration
	 * to avoid the now hidden and disabled widgets to respond to events */
	for(auto &wgt : tab_order_list)
		wgt->removeEventFilter(this);

	tab_order_list.clear();

	// Recursively collect the children of the parent widget
	for(auto &wgt : child_wgts)
		collectChildWidgets(parent_wgt, tab_order_list);

	/* Removing container widgets and Qt-internal widgets
	 * that don't need to be in our tab ordering */
	tab_order_list.removeIf([](const QWidget *wgt){
		static const QStringList ignored_classes {
			"QFrame", "QGroupBox", "QTabWidget", "QLabel",
			"QLineEditIconButton", "QWidget", "QStackedWidget"
		};

		return ignored_classes.contains(wgt->metaObject()->className()) ||
					 wgt->objectName().startsWith("qt_") ||
					 wgt->objectName().isEmpty() ||
					 /* Special case for pgModeler's container widget
						* When not specified, the widget name is the same as the
						* class name. This is enough to exclude the widget since
						* their children were collected earlier */
					 wgt->objectName() == wgt->metaObject()->className();
	});

	if(tab_order_list.isEmpty())
		return;

	int count = tab_order_list.size();

	/* (Re)installing the event filter in the new
	 * tab order list, and using the Qt's tab order
	 * method configuration but with our list */
	for(int idx = 0; idx < count; idx++)
	{
		tab_order_list[idx]->installEventFilter(this);
		tab_order_list[idx]->setFocusPolicy(Qt::StrongFocus);

		if(idx <= (count - 2))
			QWidget::setTabOrder(tab_order_list[idx], tab_order_list[idx + 1]);
	}
}

void TabOrderManager::collectChildWidgets(QWidget *root_wgt, QWidgetList &child_wgts)
{
	if(!root_wgt)
		return;

	// Collecting the direct children of the root_wgt
	QWidgetList wgts = root_wgt->findChildren<QWidget *>(Qt::FindDirectChildrenOnly);
	QMap<int, QMap<int, QWidgetList>> vh_ord_wgts;
	QPoint pos;

	/* We need to sort the wigets by their position (y first, then x)
	 * so the tab order may be honored visually */
	for(auto &wgt : wgts)
	{
		pos = wgt->pos();
		vh_ord_wgts[pos.y()][pos.x()].append(wgt);
	}

	wgts.clear();

	// Creating a single list of the sorted widgets
	for(const auto &[_, wgts_map] : vh_ord_wgts.asKeyValueRange())
	{
		for(const auto &[_, ord_wgts] : wgts_map.asKeyValueRange())
			wgts.append(ord_wgts);
	}

	// Applying the same routine to the children widgets
	for(auto &wgt : wgts)
	{
		child_wgts.append(wgt);

		if(!wgt->childrenRect().isNull())
			collectChildWidgets(wgt, child_wgts);
	}
}
