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

#include "customsortproxymodel.h"

CustomSortProxyModel::CustomSortProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{

}

QVariant CustomSortProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	/* The default implementation of QSortFilterProxyModel is to display
	 * the original row number in place of the vertical header item, no matter the
	 * sorting order. This causes the vertical header items to draw row numbers
	 * in a random order which is a not desired behavior */
	if(orientation == Qt::Vertical && role == Qt::DisplayRole)
		return QString::number(section + 1);

	return QSortFilterProxyModel::headerData(section, orientation, role);
}

