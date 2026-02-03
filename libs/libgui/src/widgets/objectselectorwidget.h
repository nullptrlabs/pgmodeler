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
\class ObjectSelectorWidget
\brief Implements a simple object picker based upon the object view widget.
*/

#ifndef OBJECT_SELECTOR_WIDGET_H
#define OBJECT_SELECTOR_WIDGET_H

#include "databasemodel.h"
#include "modelobjectswidget.h"
#include "ui_objectselectorwidget.h"

class __libgui ObjectSelectorWidget: public QWidget, public Ui::ObjectSelectorWidget {
	Q_OBJECT

	private:
		//! \brief Indicates whether the object id label must be displayed
		bool show_id_label;

		//! \brief An object view widget instance used as object picker
		ModelObjectsWidget *obj_view_wgt;

		//! \brief Selected object reference
		BaseObject *selected_obj;

		//! \brief Object types that the selector is able to pick
		std::vector<ObjectType> sel_obj_types;

		//! \brief Stores the database model which the selector must search the object
		DatabaseModel *model;

		//! \brief Configures the selectors attributes at construction time
		void configureSelector();

		bool eventFilter(QObject *obj, QEvent *evnt) override;

	public:
		explicit ObjectSelectorWidget(ObjectType sel_obj_type, QWidget * parent, bool show_id_lbl = false);

		explicit ObjectSelectorWidget(std::vector<ObjectType> sel_obj_types, QWidget * parent, bool show_id_lbl = false);

		~ObjectSelectorWidget() override;

		//! \brief Enables the creation of new objects from withing the object selector dialog
		void enableObjectCreation(bool value);

		//! \brief Returns the reference to the selected object
		BaseObject *getSelectedObject();

		//! \brief Returns the selected object formated name
		QString getSelectedObjectName();

		//! \brief Defines the initial selected object to be show on the selector
		void setSelectedObject(BaseObject *object);

		//! \brief Defines the model which the selector will search the objects
		void setModel(DatabaseModel *model);

		/*! \brief Defines the selector as readonly, hiding the clear and object browse buttons
		 *  Also, disables the event filter over the object name input to avoid displaying the
		 *  object selection dialog. The method setSelectedObject still defines the object to
		 *  be displayed, only user interactions in the UI are disabled. */
		void setReadOnly(bool ro);

		void setSizePolicy(QSizePolicy::Policy horiz_pol, QSizePolicy::Policy vert_pol);

	private slots:
		void showSelectedObject(BaseObject *obj_sel, bool=false);

		//! \brief Shows the object view widget (picker)
		void showObjectView();

	public slots:
		void clearSelector();

	signals:
		//! \brief Signal emitted when the user selects an object
		void s_objectSelected();

		//! \brief Signal emitted when the user clears the selector
		void s_selectorCleared();

		/*! \brief Signal emitted when the user clears the selector or selects and object
		 * The boolean param indicates if there's an object selected or not */
		void s_selectorChanged(bool);
};

#endif
