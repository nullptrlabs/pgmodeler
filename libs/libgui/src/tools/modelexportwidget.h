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
\class ModelExportWidget
\brief Implements the operations to export a model to a file, image or directly to DBMS.
*/

#ifndef MODEL_EXPORT_WIDGET_H
#define MODEL_EXPORT_WIDGET_H

#include "ui_modelexportwidget.h"
#include "schemaparser.h"
#include "widgets/modelwidget.h"
#include "modelexporthelper.h"
#include "utils/htmlitemdelegate.h"
#include "widgets/fileselectorwidget.h"
#include "modeldbselectorwidget.h"

class __libgui ModelExportWidget: public QWidget, public Ui::ModelExportWidget {
	Q_OBJECT

	private:
		/*! \brief Indicates if the full output generated during the process should be displayed
		 * When this attribute is true, only errors and some key info messages are displayed. */
		static bool low_verbosity;

		//! \brief Custom delegate used to paint html texts in output tree
		HtmlItemDelegate *htmlitem_del;

		//! \brief Stores the model_wgt widget which will be exported
		ModelWidget *model_wgt;

		//! \brief Export helper
		ModelExportHelper export_hlp;

		//! \brief Thread used to manage the export helper when dealing with dbms export
		QThread *export_thread;

		//! \brief Auxiliary viewport passed to export helper when dealing with PNG export
		QGraphicsView *viewp;

		ModelDbSelectorWidget *model_sel_wgt;

		FileSelectorWidget *sql_file_sel,
		*img_file_sel,
		*dict_file_sel;

		void finishExport(const QString &msg);
		void enableExportModes(bool value);
		void closeEvent(QCloseEvent *event) override;
		void showEvent(QShowEvent *) override;

	public:
		ModelExportWidget(QWidget * parent = nullptr);

		//! \brief Defines if all the output generated during the import process should be displayed
		static void setLowVerbosity(bool value);

		//! \brief Updates the connections combo with the latest loaded connection settings
		void updateConnections();

		void updateModels(const QList<ModelWidget *> &models);

		bool isThreadRunning();

	private slots:
		void selectExportMode();
		void exportModel();
		void updateProgress(int progress, QString msg, ObjectType obj_type, QString cmd, bool is_code_gen);
		void captureThreadError(Exception e);
		void cancelExport();
		void handleExportFinished();
		void handleExportCanceled();
		void handleErrorIgnored(QString err_code, QString err_msg, QString cmd);
		void editConnections();
		void enableExport();
		void selectImageFormat();
		void selectDataDictMode();
		void selectSQLExportMode();

	signals:
		/*! \brief This signal is emitted whenever the user changes the connections settings
		within this widget without use the main configurations dialog */
		void s_connectionsUpdateRequested();

		void s_exportStarted();
		void s_exportFinished();
};

#endif
