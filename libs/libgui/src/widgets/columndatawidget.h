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

#ifndef COLUMN_DATA_WIDGET_H
#define COLUMN_DATA_WIDGET_H

#include "guiglobal.h"
#include "numberedtexteditor.h"

class __libgui ColumnDataWidget: public QWidget {
	Q_OBJECT

	private:
		NumberedTextEditor *value_txt;

		void showEvent(QShowEvent *) override;

	public:
		ColumnDataWidget(bool use_syntax_hl = false, const QString &hl_conf = "", QWidget *parent = nullptr);

		void setData(const QString &data);

		QString getData();

		void setReadOnly(bool value);
};

#endif
