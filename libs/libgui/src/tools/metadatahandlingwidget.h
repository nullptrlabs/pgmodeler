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
\class ObjectsMetadataWidget
\brief Implements an interface to the feature to extract and restore objects metadata in a model.
*/

#ifndef METADATA_HANDLING_WIDGET_H
#define METADATA_HANDLING_WIDGET_H

#include "ui_metadatahandlingwidget.h"
#include "widgets/modelwidget.h"
#include "widgets/fileselectorwidget.h"
#include "tools/modeldbselectorwidget.h"

class __libgui MetadataHandlingWidget: public QWidget, public Ui::MetadataHandlingWidget {
	Q_OBJECT

	private:
		QTreeWidgetItem *root_item;

		FileSelectorWidget *backup_file_sel;

		ModelDbSelectorWidget *extract_model_sel, *apply_model_sel;

		enum MetaOpType {
			OpExtractRestore,
			OpExtractOnly,
			OpRestoreOnly
		};

	public:
		MetadataHandlingWidget(QWidget * parent = nullptr);

		void updateModels(const QList<ModelWidget *> &models);
		bool isMetadataHandlingEnabled();

	public slots:
		void handleObjectsMetada();

	private slots:
		void updateProgress(int progress, QString msg, unsigned type_id);
		void enableMetadataHandling();
		void selectAllOptions();
		void configureSelector();

	signals:
		void s_metadataHandlingEnabled(bool);
};

#endif
