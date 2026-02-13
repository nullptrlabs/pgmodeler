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
\class ModelFixForm
\brief Implements an interface to pgmodeler-cli --fix-model command.
*/

#ifndef MODEL_FIX_FORM_H
#define MODEL_FIX_FORM_H

#include <QProcess>
#include "ui_modelfixform.h"
#include "widgets/fileselectorwidget.h"
#include "widgets/debugoutputwidget.h"

class __libgui ModelFixForm: public QDialog, public Ui::ModelFixForm {
	Q_OBJECT

	private:
		static const QString PgModelerCli;

		//! \brief Process used to execute pgmodeler-cli
		QProcess pgmodeler_cli_proc;

		DebugOutputWidget *dbg_output_wgt;

		FileSelectorWidget *input_file_sel,
		*output_file_sel,
		*pgmodeler_cli_sel;

		QStringList extra_cli_args;

		void closeEvent(QCloseEvent *event);
		void resetFixForm();
		void enableFixOptions(bool enable);

	public:
		ModelFixForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);

		void setExtraCliArgs(const QStringList &extra_args);

	public slots:
		int exec();

	private slots:
		void enableFix();
		void fixModel();
		void cancelFix();
		void updateOutput();
		void handleProcessFinish(int res);

	signals:
		void s_modelLoadRequested(QString);

	friend class MainWindow;
};

#endif
