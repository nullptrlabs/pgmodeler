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
\class TabOrderManager
\brief Auxiliary class used by configure the tab order dynamicaly in a widget.
*/

#ifndef TAB_ORDER_MANAGER_H
#define TAB_ORDER_MANAGER_H

#include "guiglobal.h"
#include <QObject>
#include <QTimer>
#include <QWidget>

class __libgui  TabOrderManager: public QObject {
	Q_OBJECT

	private:
		QTimer cfg_timer;

		QWidgetList tab_order_list;

		void collectChildWidgets(QWidget *root, QWidgetList &child_wgts);

		/*! \brief This method stops the configuration timer and
		 *  clear the tab order list and uninstall the event filter
		 *  from childer widgets */
		void clearTabOrder();

	protected:
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		explicit TabOrderManager(QWidget *parent);

		~TabOrderManager() override;

	private slots:
		void configureTabOrder();
};

#endif
