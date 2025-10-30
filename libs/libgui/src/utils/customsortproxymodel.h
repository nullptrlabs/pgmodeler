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
\class CustomSortProxyModel
\brief Implements a custom sort/filter proxy model that preserves the vertical headers numbers order.
*/

#ifndef CUSTOM_SORT_PROXY_MODEL_H
#define CUSTOM_SORT_PROXY_MODEL_H

#include "guiglobal.h"
#include <QSortFilterProxyModel>

class __libgui CustomSortProxyModel: public QSortFilterProxyModel {
	Q_OBJECT

	public:
		CustomSortProxyModel(QObject *parent = nullptr);

		QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
};

#endif
