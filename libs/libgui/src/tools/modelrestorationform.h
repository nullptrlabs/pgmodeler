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
\class ModelRestorationForm
\brief Implements the operation to retore models before a unexpected crash.
*/

#ifndef MODEL_RESTORATION_FORM_H
#define MODEL_RESTORATION_FORM_H

#include "guiglobal.h"
#include "ui_modelrestorationform.h"

class __libgui ModelRestorationForm: public QDialog, public Ui::ModelRestorationForm {
	Q_OBJECT

	private:
		QStringList ignored_files;

	public:
		ModelRestorationForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);

		//! \brief Returns the list of temporary files existant on tmp/ dir
		QStringList getTemporaryModels();

		//! \brief Returns the list of temporary files selected to be restored
		QStringList getSelectedModels();

	public slots:
		int exec() override;

		//! \brief List of files to be ignored by the restoration form
		void setIgnoredFiles(const QStringList &list);

		//! \brief Clears the tmp/ dir by removing all temporary files (*.dbm, *.dbk, *.omf, *.sql. *.log)
		void removeTemporaryFiles();

		//! \brief Clears the tmp/ dir removing all temporary models (*.dbm)
		void removeTemporaryModels();

		//! \brief Remove only the specified temp model
		void removeTemporaryModel(const QString &tmp_model);

		//! \brief Checks if there is at least one temporary file on tmp/ dir
		bool hasTemporaryModels();

	private slots:
		void enableRestoration();
};

#endif
