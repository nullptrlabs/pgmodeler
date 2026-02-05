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
\class LayersWidget
\brief Implements the widget that allows the user to quickly setup object's layers
*/

#ifndef LAYERS_WIDGET_H
#define LAYERS_WIDGET_H

#include "guiglobal.h"
#include "ui_layerswidget.h"
#include "basegraphicobject.h"

class ModelWidget;

class __libgui LayersWidget : public QDialog, Ui::LayersWidget {
	Q_OBJECT

	private:
		//! \brief The current object selection in a model widget that the layer widget will operate on.
		std::vector<BaseGraphicObject *> selected_objs;

		//! \brief Indicates if the user have interacted with the layers checkboxes changing objects layers.
		bool layers_changed;

		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		explicit LayersWidget(QWidget *parent = nullptr);

		//! \brief Configures the widget with layer names and a set of object selected in a model widget instance
		void setAttributes(ModelWidget *model_wgt);

		//! \brief Returns true if the layers of the selected object have been changed by the user.
		bool isLayersChanged();

	private slots:
		void updateObjectsLayers();

	signals:
		void s_newLayerRequested(const QString &layer_name);

		void s_objectsLayerChanged();
};

#endif
