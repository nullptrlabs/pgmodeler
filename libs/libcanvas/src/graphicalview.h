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
\ingroup libcanvas
\class GraphicalView
\brief Implements the graphical representation for Views
*/

#ifndef GRAPHICAL_VIEW_H
#define GRAPHICAL_VIEW_H

#include "view.h"
#include "basetableview.h"

class __libcanvas GraphicalView: public BaseTableView {
	Q_OBJECT

	public:
		GraphicalView(View *view);

	private slots:
		void configureObject() override;
};

#endif
