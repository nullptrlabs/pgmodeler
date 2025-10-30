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
\class ModelValidationWidget
\brief Implements the operations to display to the user the validation operation performed by ValidationHelper
*/

#ifndef MODEL_VALIDATION_WIDGET_H
#define MODEL_VALIDATION_WIDGET_H

#include "ui_modelvalidationwidget.h"
#include "widgets/modelwidget.h"
#include "modelvalidationhelper.h"
#include "utils/htmlitemdelegate.h"

class __libgui ModelValidationWidget: public QWidget, public Ui::ModelValidationWidget {
	Q_OBJECT

	private:
		QMenu output_menu;

		//! \brief Custom delegate used to paint html texts in output tree
		HtmlItemDelegate *htmlitem_del;

		//! \brief Reference model widget
		ModelWidget *model_wgt;

		//! \brief Object that handles the model validation steps
		ModelValidationHelper *validation_helper;

		//! \brief Current fix step
		int curr_step;

		//! \brief Thread used to control the validation helper
		QThread *validation_thread;

		/*! \brief Stores the graphical objects that have their ids changed so that in the end of
		the validation they can be updated to reflect the new id in the tooltips and forms */
		std::vector<BaseGraphicObject *> graph_objects;

		void emitValidationInProgress();

		//! \brief Creates a new validation thread
		void createThread();

		void configureValidation();

		void selectObject();

		//! \brief Creates a text representation of all entries in the output tree
		QString generateOutputText();

		//! \brief Creates a text representation of a single entry (and its children) in the output tree
		void generateOutputItemText(QTreeWidgetItem *item, QString &output, int level);

	protected:
		void resizeEvent(QResizeEvent *event) override;
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		ModelValidationWidget(QWidget * parent = nullptr);

		//! \brief Sets the database model to work on
		void setModel(ModelWidget *model_wgt);

		//! \brief Returns if there is a validation in progress
		bool isThreadRunning();

		//! \brief Updates the connections combo with the latest loaded connection settings
		void updateConnections();

	private slots:
		void applyFixes();
		void updateValidation(ValidationInfo val_info);
		void updateProgress(int prog, QString msg, ObjectType obj_type, QString cmd, bool is_code_gen);
		void updateObjectName(QString obj_name, ObjectType obj_type);
		void reenableValidation();
		void cancelValidation();
		void validateRelationships();
		void destroyThread(bool force=false);
		void updateGraphicalObjects();
		void editConnections();
		void handleSQLValidationStarted();
		void swapObjectsIds();
		void copyTextOutput();
		void handleFixFailed(Exception e);

	public slots:
		void hide();
		void clearOutput();
		void validateModel();

	signals:
		void s_hideRequested();
		void s_validationInProgress(bool);
		void s_validationFinished(bool);
		void s_validationCanceled();
		void s_fixApplied();
		void s_graphicalObjectsUpdated();

		/*! \brief This signal is emitted whenever the user changes the connections settings
		within this widget without use the main configurations dialog */
		void s_connectionsUpdateRequested();
};

#endif
