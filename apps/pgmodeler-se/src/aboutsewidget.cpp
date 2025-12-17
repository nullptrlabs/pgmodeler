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

#include "aboutsewidget.h"
#include "guiutilsns.h"

AboutSEWidget::AboutSEWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	GuiUtilsNs::configureWidgetsFont({ title_lbl, pgmodeler_ver_lbl}, GuiUtilsNs::HugeFontFactor);
	GuiUtilsNs::configureWidgetFont(build_num_lbl, GuiUtilsNs::BigFontFactor);

	pgmodeler_ver_lbl->setText(QString("v%1 ").arg(GlobalAttributes::PgModelerVersion));
	build_num_lbl->setText(QString("%1 Qt %2").arg(GlobalAttributes::PgModelerBuildNumber).arg(QT_VERSION_STR));
}
