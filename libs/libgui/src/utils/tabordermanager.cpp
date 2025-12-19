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

TabOrderManager::TabOrderManager(QWidget *parent)	: QObject { parent }
{
	connect(&cfg_timer, &QTimer::timeout, this, &TabOrderManager::configureTabOrder);
}

TabOrderManager::~TabOrderManager()
{
	qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "TabOrderManager::~TabOrderManager";
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
					event->type() == QEvent::KeyPress)
	{
		QKeyEvent *kevent = dynamic_cast<QKeyEvent *>(event);

		if(kevent->key() == Qt::Key_Tab)
		{
			qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "KeyPress";
			int idx = tab_order_list.indexOf(object);

			if(idx < tab_order_list.size() - 1)
				idx++;
			else
				idx = 0;

			tab_order_list[idx]->setFocus(Qt::TabFocusReason);
			return true;
		}
	}

	return QObject::eventFilter(object, event);
}

void TabOrderManager::configureTabOrder()
{
	qDebug().noquote().nospace() << QDateTime::currentDateTime().toString() << " :: " << "configureTabOrder";
	cfg_timer.stop();

	QWidget *parent_wgt = qobject_cast<QWidget *>(parent());
	QWidgetList child_wgts = parent_wgt->findChildren<QWidget *>();
	QMap<int, QMap<int, QWidgetList>> ord_wgts;
	QPoint pos;

	static const QStringList accepted_classes {
		"QLineEdit", "QToolButton", "QPushButton",
		"QCheckBox", "QRadioButton", "QSpinBox",
		"QDoubleSpinBox", "QPlainTextEdit", "QComboBox"
	};

	for(auto &wgt : tab_order_list)
		wgt->removeEventFilter(this);

	for(auto &wgt : child_wgts)
	{
		wgt->setFocusPolicy(Qt::NoFocus);

		if(!wgt->isVisible() || !wgt->isEnabled())
			continue;

		if(wgt->objectName().startsWith("qt_") ||
			 !accepted_classes.contains(wgt->metaObject()->className()))
			continue;

		pos = parent_wgt->mapFromGlobal(wgt->mapToGlobal(wgt->pos()));
		ord_wgts[pos.y()][pos.x()].append(wgt);

		qDebug().noquote() << wgt->objectName() << " -> " << pos;
	}

	if(ord_wgts.isEmpty())
		return;

	tab_order_list.clear();

	for(const auto &[v_key, h_ord_map] : ord_wgts.asKeyValueRange())
	{
		for(const auto &[h_key, wgt_list] : h_ord_map.asKeyValueRange())
			tab_order_list.append(wgt_list);
	}

	int count = tab_order_list.size();

	for(int idx = 0; idx < count; idx++)
	{
		tab_order_list[idx]->installEventFilter(this);
		tab_order_list[idx]->setFocusPolicy(Qt::StrongFocus);

		if(idx <= (count - 2))
			QWidget::setTabOrder(tab_order_list[idx], tab_order_list[idx + 1]);

		//qDebug().noquote().nospace() << tab_order_list[idx]->objectName() << " :: " << tab_order_list[idx]->metaObject()->className();
	}

	tab_order_list[0]->setFocus();
}
