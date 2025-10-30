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

#include "donatewidget.h"
#include "globalattributes.h"
#include <QDesktopServices>
#include <QUrl>
#include "guiutilsns.h"

DonateWidget::DonateWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);

	GuiUtilsNs::createDropShadow(this, 5, 5, 30);

	connect(hide_tb, &QToolButton::clicked, this, [this](){
		hide();
		emit s_hideRequested();
	});

	connect(donate_btn, &QPushButton::clicked, this, [this](){
		QDesktopServices::openUrl(QUrl(GlobalAttributes::PgModelerDonateURL));
		hide();
		emit s_hideRequested();
	});

	GuiUtilsNs::configureWidgetFont(title_lbl, GuiUtilsNs::BigFontFactor);
	this->adjustSize();
}
