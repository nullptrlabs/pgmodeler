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
\class ChangelogWidget
\brief Implements the widget that allows the user to handle model's changelog
*/

#ifndef CHANGELOG_WIDGET_H
#define CHANGELOG_WIDGET_H

#include "ui_changelogwidget.h"
#include "modelwidget.h"

class __libgui ChangelogWidget : public QWidget, Ui::ChangelogWidget {
	Q_OBJECT

	private:
		ModelWidget *model;

	public:
		explicit ChangelogWidget(QWidget *parent = nullptr);

		//! \brief Defines the model in which the widget will work
		void setModel(ModelWidget *model);

	public slots:
		void setVisible(bool value) override;
		void updateChangelogInfo();
		void clearChangelog();
		void inspectChangelog();

	signals:
		void s_visibilityChanged(bool value);
};

#endif
