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
\class ObjectsMetadataForm
\brief Implements an interface to the feature to extract and restore objects metadata in a model.
*/

#ifndef METADATA_HANDLING_FORM_H
#define METADATA_HANDLING_FORM_H

#include "ui_metadatahandlingform.h"
#include "widgets/modelwidget.h"
#include "utils/htmlitemdelegate.h"
#include "widgets/fileselectorwidget.h"

class __libgui MetadataHandlingForm: public QDialog, public Ui::MetadataHandlingForm {
	Q_OBJECT

	private:
		ModelWidget *model_wgt;

		HtmlItemDelegate *htmlitem_deleg;

		QTreeWidgetItem *root_item;

		FileSelectorWidget *backup_file_sel;

		void showEvent(QShowEvent *);

	public:
		MetadataHandlingForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);
		void setModelWidget(ModelWidget *model_wgt);
		void setModelWidgets(QList<ModelWidget *> models);

	private slots:
		void updateProgress(int progress, QString msg, unsigned type_id);
		void handleObjectsMetada();
		void enableMetadataHandling();
		void selectAllOptions();
		void configureSelector();

	signals:
		void s_metadataHandled();
};

#endif
