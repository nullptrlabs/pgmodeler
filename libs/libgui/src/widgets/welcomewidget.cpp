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

#include "welcomewidget.h"
#include "guiutilsns.h"
#include <QDesktopServices>

WelcomeWidget::WelcomeWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);
	nullptrlabs_lbl->installEventFilter(this);

	QList<QToolButton *> btns= { new_tb, load_tb, recent_tb, last_session_tb, sample_tb, support_tb };
	QFont fnt;

	for(auto &btn : btns)
	{
		fnt = btn->font();
		fnt.setWeight(QFont::Normal);
		btn->setFont(fnt);

#ifdef Q_OS_LINUX
		GuiUtilsNs::__configureWidgetFont(btn, 1.20);
#else
		GuiUtilsNs::__configureWidgetFont(btn, 1.40);
#endif
	}
}

bool WelcomeWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == nullptrlabs_lbl && event->type() == QEvent::MouseButtonPress)
		QDesktopServices::openUrl(GlobalAttributes::NullptrLabsSite);

	return QWidget::eventFilter(object, event);
}
