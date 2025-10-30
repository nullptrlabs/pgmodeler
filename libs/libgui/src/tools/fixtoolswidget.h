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
\class FixToolsWidget
\brief Reunites in a single form all widgets related to model fix processes.
*/

#ifndef FIX_TOOLS_WIDGET_H
#define FIX_TOOLS_WIDGET_H

#include "ui_fixtoolswidget.h"
#include "modelfixwidget.h"
#include "metadatahandlingwidget.h"

class __libgui FixToolsWidget: public QWidget, public Ui::FixToolsWidget {
	Q_OBJECT

	private:
		ModelFixWidget *model_fix_wgt;

		MetadataHandlingWidget *metadata_wgt;

	private:

	public:
		FixToolsWidget(QWidget * parent = nullptr);

		~FixToolsWidget() override = default;

		bool isToolRunning();

		void updateModels(const QList<ModelWidget *> &models);

	public slots:

	private slots:
		void setCurrentTool();

	signals:
		void s_modelLoadRequested(QString);
};

#endif
