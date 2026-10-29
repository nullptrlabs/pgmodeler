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
\class ModelSelectorWidget
\brief Implements the widget that allows user to choose the model to be used in a operation
*/

#ifndef MODEL_SELECTOR_WIDGET_H
#define MODEL_SELECTOR_WIDGET_H

#include <QWidget>
#include "ui_modelselectorwidget.h"
#include "widgets/modelwidget.h"

class ModelSelectorWidget : public QWidget, public Ui::ModelSelectorWidget {
	Q_OBJECT

	public:
		explicit ModelSelectorWidget(QWidget *parent = nullptr);

		~ModelSelectorWidget() override = default;

		ModelWidget *getSelectedModel();

		bool isModelSelected();

		virtual void clearSelection();

		//! \brief Returns wheter the selector has a valid selection (database or model)
		virtual bool hasSelection();

		//! \brief Updates the combo of database models with the models in the list
		void updateModels(const QList<ModelWidget *> &models);

	public slots:
		void updateModelFilename();

	signals:
		void s_selectionChanged();
};

#endif
