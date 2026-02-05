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
\class NewObjectOverlayWidget
\brief Implements the a basic overlay to quicly trigger action to create new objects.
*/

#ifndef NEW_OBJECT_OVERLAY_WIDGET_H
#define NEW_OBJECT_OVERLAY_WIDGET_H

#include "guiglobal.h"
#include <vector>
#include "baseobject.h"
#include "ui_newobjectoverlaywidget.h"

//Adding the declaration of ModelWidget class in order to be visible to NewObjectOverlayWidget
class ModelWidget;

class __libgui NewObjectOverlayWidget: public QWidget, public Ui::NewObjectOverlayWidget {
	Q_OBJECT

	private:
		//! \brief Store the actions in the ModelWidget corresponding to each button the in overlay
		std::map<QToolButton *, QAction *> btn_actions;

	public:
		explicit NewObjectOverlayWidget(ModelWidget * parent);
		void setSelectedObjects(std::vector<BaseObject *> &sel_objs);
		void configureOverlayButtons();

	private slots:
		/*! \brief Execute the action related to the triggered button.
		 * This method executes only one action at a time. If the user presses two or more
		 * buttons via shortcut then only the first one will be considered, the rest is discard */
		void executeAction();
};

#endif
