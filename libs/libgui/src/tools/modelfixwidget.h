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
\class ModelFixWidget
\brief Implements an interface to pgmodeler-cli --fix-model command.
*/

#ifndef MODEL_FIX_WIDGET_H
#define MODEL_FIX_WIDGET_H

#include <QProcess>
#include "ui_modelfixwidget.h"
#include "widgets/fileselectorwidget.h"
#include "widgets/debugoutputwidget.h"

class __libgui ModelFixWidget: public QWidget, public Ui::ModelFixWidget {
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

		void showEvent(QShowEvent *) override;
		void enableFixOptions(bool enable);

	public:
		ModelFixWidget(QWidget * parent = nullptr);

		void setExtraCliArgs(const QStringList &extra_args);

		/*! \brief Defines the input filename to be fixed.
		 *  If gen_out_filename is true, the the output file selector will
		 *  receive a filename derived from filename in the format:
		 *  [filename (w/o extension)][_fixed][_yyyyMMdd_hhmmss][.dbm] */
		void setInputModel(const QString &filename, bool gen_out_filename);

		bool isProcessRunning();

		bool isFixEnabled();

	public slots:
		void fixModel();
		void cancelFix();
		void resetFixForm();

	private slots:
		void enableFix();
		void updateOutput();
		void handleProcessFinish(int res);
		void generateOutputFilename();

	signals:
		void s_modelLoadRequested(QString);
		void s_modelFixEnabled(bool);
		void s_modelFixStarted();
		void s_modelFixFinished();
};

#endif
