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
\class TaskProgressWidget
\brief Implements a widget that shows the progress of executed operations (e.g. loading a model file, generation source code)
*/

#ifndef TASK_PROGRESS_WIDGET_H
#define TASK_PROGRESS_WIDGET_H

#include "guiglobal.h"
#include "ui_taskprogresswidget.h"
#include <map>

class __libgui TaskProgressWidget: public QDialog, public Ui::TaskProgressWidget {
	Q_OBJECT

	private:
		//! \brief Stores the icons that are shown as the icons tokens are send via	updateProgress() slot
		std::map<unsigned, QIcon> icons;

	public:
		TaskProgressWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::Widget);
		void addIcon(unsigned id, const QIcon &ico);
		void setNoProgressState(bool value);

	public slots:
		void show();
		void close();
		void updateProgress(int progress, unsigned icon_id);
		void updateProgress(int progress, QString text, unsigned icon_id);
};

#endif
