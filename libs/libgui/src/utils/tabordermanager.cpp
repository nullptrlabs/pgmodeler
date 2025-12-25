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
#include <QDebug>
#include <QDateTime>
#include <QKeyEvent>
#include <QTabWidget>
#include <QToolTip>

TabOrderManager::TabOrderManager(QWidget *parent)	: QObject { parent }
{
	//connect(&cfg_timer, &QTimer::timeout, this, &TabOrderManager::configureTabOrder);
	connect(&cfg_timer, &QTimer::timeout, this, &TabOrderManager::__configureTabOrder);
}

bool TabOrderManager::eventFilter(QObject *object, QEvent *event)
{
	if(object == parent())
	{
		if(event->type() == QEvent::LayoutRequest)
		{
			/* Avoiding calling multiple times the configuration
			* method by setting to true the flag indicating that
			* we have a pending tab order configuration */
			qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "LayoutRequest | QUEUEING";
			cfg_timer.start(100);
		}
	}
	else if(object != parent() &&
					event->type() == QEvent::EnabledChange)
	{
		qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "EnabledChange | QUEUEING";
		cfg_timer.start(100);
	}
	/* else if(object != parent() &&
					event->type() == QEvent::KeyPress)
	{
		QKeyEvent *kevent = dynamic_cast<QKeyEvent *>(event);

		if(kevent->key() == Qt::Key_Tab)
		{
			qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "KeyPress";
			int idx = tab_order_list.indexOf(object);

			// Navigating onwards when no modifier key is held
			if(kevent->modifiers() == Qt::NoModifier)
			{
				if(idx < tab_order_list.size() - 1)
					idx++;
				else
					idx = 0;
			}
			// Navigating backwards when shift is held
			else if(kevent->modifiers() == Qt::ShiftModifier)
			{
				if(idx > 0)
					idx--;
				else
					idx = tab_order_list.size() - 1;
			}

			tab_order_list[idx]->setFocus(Qt::TabFocusReason);

			QToolTip::showText(tab_order_list[idx]->mapToGlobal(QPoint(0,0)), tab_order_list[idx]->objectName(),
												 tab_order_list[idx], tab_order_list[idx]->geometry(), 2000);

			return true;
		}
	} */

	return QObject::eventFilter(object, event);
}

/* QWidgetList TabOrderManager::getTabOrderList(const QWidgetList &wgt_list)
{
	if(wgt_list.isEmpty())
		return {};

	QHash<QWidget *, QWidgetList> ord_map;
	QMap<int, QMap<int, QWidgetList>> vh_ord_wgts;
	QHash<QWidget *, QWidgetList> parent_ord_wgts;
	QWidgetList tab_ord_list;
	QPoint pos;
	QWidget *root_wgt = qobject_cast<QWidget *>(parent());

	for(auto &wgt : wgt_list)
		ord_map[wgt->parentWidget()].append(wgt);

	for(const auto &[parent_wgt, child_wgts] : ord_map.asKeyValueRange())
	{
		for(auto &wgt : child_wgts)
		{
//		pos = root_wgt->mapFromGlobal(wgt->mapToGlobal(wgt->pos()));
			pos = root_wgt->mapFromGlobal(wgt->mapToGlobal(wgt->geometry().center()));
			vh_ord_wgts[pos.y()][pos.x()].append(wgt);
		}

		for(const auto &[v_pos, h_ord_wgts] : vh_ord_wgts.asKeyValueRange())
		{
			for(const auto &[h_pos, wgts] : h_ord_wgts.asKeyValueRange())
				parent_ord_wgts[parent_wgt].append(wgts);
		}

		vh_ord_wgts.clear();
	}

	// Final positional ordering: we need to order vertically and horizontally
	// all the retrieved parent widgets so we can get the sorted children lists
	// in the correct order
	vh_ord_wgts.clear();

	for(const auto &[parent_wgt, child_wgts] : parent_ord_wgts.asKeyValueRange())
	{
		pos = root_wgt->mapFromGlobal(parent_wgt->mapToGlobal(parent_wgt->pos()));
		vh_ord_wgts[pos.y()][pos.x()].append(parent_wgt);
	}

	for(const auto &[v_pos, h_ord_wgts] : vh_ord_wgts.asKeyValueRange())
	{
		for(const auto &[h_pos, wgts] : h_ord_wgts.asKeyValueRange())
			tab_ord_list.append(parent_ord_wgts[wgts[0]]);
	}

	return tab_ord_list;
} */

/* void TabOrderManager::configureTabOrder()
{
	qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "configureTabOrder";
	cfg_timer.stop();

	QWidget *parent_wgt = qobject_cast<QWidget *>(parent());
	QWidgetList child_wgts = parent_wgt->findChildren<QWidget *>();

	for(auto &wgt : tab_order_list)
		wgt->removeEventFilter(this);

	tab_order_list.clear();

	child_wgts.removeIf([](const QWidget *wgt){
		static const QStringList ignored_classes {
			"QFrame", "QGroupBox", "QTabWidget", "QLabel",
			"QLineEditIconButton", "QWidget"
		};

		return ignored_classes.contains(wgt->metaObject()->className()) ||
					 !wgt->isVisible() || !wgt->isEnabled() ||
					 wgt->objectName().startsWith("qt_") ||
					 wgt->objectName().isEmpty();
	});

	tab_order_list = getTabOrderList(child_wgts);

	if(tab_order_list.isEmpty())
		return;

	int count = tab_order_list.size();

	qDebug().noquote() << "** Tab order **";

	for(int idx = 0; idx < count; idx++)
	{
		qDebug().noquote() << tab_order_list[idx]->objectName() << " " << tab_order_list[idx]->metaObject()->className();

		tab_order_list[idx]->installEventFilter(this);
		tab_order_list[idx]->setFocusPolicy(Qt::StrongFocus);

		if(idx <= (count - 2))
			QWidget::setTabOrder(tab_order_list[idx], tab_order_list[idx + 1]);
	}

	tab_order_list[0]->setFocus(Qt::TabFocusReason);
} */

void TabOrderManager::__configureTabOrder()
{
	qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "configureTabOrder";
	cfg_timer.stop();

	QWidget *parent_wgt = qobject_cast<QWidget *>(parent());
	QWidgetList child_wgts = parent_wgt->findChildren<QWidget *>(Qt::FindDirectChildrenOnly);

	for(auto &wgt : tab_order_list)
		wgt->removeEventFilter(this);

	tab_order_list.clear();

	for(auto &wgt : child_wgts)
		__collectChildWidget(parent_wgt, tab_order_list);

	tab_order_list.removeIf([](const QWidget *wgt){
		static const QStringList ignored_classes {
			"QFrame", "QGroupBox", "QTabWidget", "QLabel",
			"QLineEditIconButton", "QWidget"
		};

		return ignored_classes.contains(wgt->metaObject()->className()) ||
					 /* !wgt->isVisible() || !wgt->isEnabled() || */
					 wgt->objectName().startsWith("qt_") ||
					 wgt->objectName().isEmpty() ||
					 wgt->objectName() == wgt->metaObject()->className();
	});

	if(tab_order_list.isEmpty())
		return;

	int count = tab_order_list.size();

	qDebug().noquote() << "** Tab order **";

	for(int idx = 0; idx < count; idx++)
	{
		qDebug().noquote() << tab_order_list[idx]->objectName() << " " << tab_order_list[idx]->metaObject()->className();

		tab_order_list[idx]->installEventFilter(this);
		tab_order_list[idx]->setFocusPolicy(Qt::StrongFocus);

		if(idx <= (count - 2))
			QWidget::setTabOrder(tab_order_list[idx], tab_order_list[idx + 1]);
	}
}

void TabOrderManager::__collectChildWidget(QWidget *root, QWidgetList &child_wgts)
{
	if(!root)
		return;

	QWidgetList wgts = root->findChildren<QWidget *>(Qt::FindDirectChildrenOnly);

	for(auto &wgt : wgts)
	{
		child_wgts.append(wgt);

		if(!wgt->childrenRect().isNull())
			__collectChildWidget(wgt, child_wgts);
	}
}
