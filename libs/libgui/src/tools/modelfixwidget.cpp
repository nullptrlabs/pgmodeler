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

#include "modelfixwidget.h"
#include "customuistyle.h"
#include "globalattributes.h"

const QString ModelFixWidget::PgModelerCli {
#ifdef Q_OS_WIN
	"pgmodeler-cli.exe"
#else
	"pgmodeler-cli"
#endif
};

ModelFixWidget::ModelFixWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert2_frm);

	input_file_sel = new FileSelectorWidget(this);
	input_file_sel->setObjectName("input_file_sel");
	input_file_sel->setFileMustExist(true);
	input_file_sel->setNameFilters({tr("Database model (*%1)").arg(GlobalAttributes::DbModelExt), tr("All files (*.*)")});
	input_file_sel->setAcceptMode(QFileDialog::AcceptOpen);
	input_file_sel->setAllowFilenameInput(true);
	input_file_sel->setFileIsMandatory(true);
	input_file_sel->setWindowTitle(tr("Select input file"));
	model_fix_grid->addWidget(input_file_sel, 1, 2);

	output_file_sel = new FileSelectorWidget(this);
	output_file_sel->setObjectName("output_file_sel");
	output_file_sel->setNameFilters({tr("Database model (*%1)").arg(GlobalAttributes::DbModelExt), tr("All files (*.*)")});
	output_file_sel->setDefaultSuffix(GlobalAttributes::DbModelExt);
	output_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	output_file_sel->setAllowFilenameInput(true);
	output_file_sel->setFileIsMandatory(true);
	output_file_sel->setWindowTitle(tr("Select output file"));
	output_sel_lt->insertWidget(0, output_file_sel);

	pgmodeler_cli_sel = new FileSelectorWidget(this);
	pgmodeler_cli_sel->setObjectName("pgmodeler_cli_sel");
	pgmodeler_cli_sel->setFileMustExist(true);
	pgmodeler_cli_sel->setNameFilters({tr("pgModeler command line tool (%1)").arg(PgModelerCli)});
	pgmodeler_cli_sel->setAcceptMode(QFileDialog::AcceptOpen);
	pgmodeler_cli_sel->setAllowFilenameInput(true);
	pgmodeler_cli_sel->setWindowTitle(tr("Select pgmodeler-cli executable"));
	pgmodeler_cli_sel->setVisible(false);
	pgmodeler_cli_sel->setCheckExecutionFlag(true);
	pgmodeler_cli_sel->setFileIsMandatory(true);
	pgmodeler_cli_sel->setNamePattern(QString("(.)+(%1)$").arg(PgModelerCli));
	model_fix_grid->addWidget(pgmodeler_cli_sel, 0, 2);

	dbg_output_wgt = new DebugOutputWidget(this);
	output_lt->addWidget(dbg_output_wgt);

	connect(&pgmodeler_cli_proc, &QProcess::readyReadStandardOutput, this, &ModelFixWidget::updateOutput);
	connect(&pgmodeler_cli_proc, &QProcess::readyReadStandardError, this, &ModelFixWidget::updateOutput);
	connect(&pgmodeler_cli_proc, &QProcess::finished, this, &ModelFixWidget::handleProcessFinish);

	connect(input_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixWidget::enableFix);
	connect(output_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixWidget::enableFix);
	connect(pgmodeler_cli_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixWidget::enableFix);

	connect(input_file_sel, &FileSelectorWidget::s_selectorChanged, gen_filename_tb, &QToolButton::setEnabled);
	connect(gen_filename_tb, &QToolButton::clicked, this, &ModelFixWidget::generateOutputFilename);

	resetFixForm();
}

void ModelFixWidget::setExtraCliArgs(const QStringList &extra_args)
{
	extra_cli_args = extra_args;
}

void ModelFixWidget::setInputModel(const QString &filename, bool gen_out_filename)
{
	input_file_sel->setSelectedFile(filename);

	if(gen_out_filename)
		generateOutputFilename();
}

void ModelFixWidget::generateOutputFilename()
{
	QFileInfo fi(input_file_sel->getSelectedFile());

	output_file_sel->setSelectedFile(fi.absolutePath() +
																	 GlobalAttributes::DirSeparator +
																	 fi.completeBaseName() +
																	 "_fixed" +
																	 QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss.") +
																	 fi.suffix());
}

bool ModelFixWidget::isProcessRunning()
{
	return pgmodeler_cli_proc.state() == QProcess::Running;
}

void ModelFixWidget::resetFixForm()
{
	pgmodeler_cli_lbl->setVisible(false);
	pgmodeler_cli_sel->setVisible(false);
	input_file_sel->clearSelector();
	output_file_sel->clearSelector();

	dbg_output_wgt->clear();
	dbg_output_wgt->logMessage(tr("Waiting for the process to start..."));
	dbg_output_wgt->showActionButtons(false);

	load_model_chk->setChecked(true);
	enableFixOptions(true);

	progress_pb->setVisible(false);
}

void ModelFixWidget::enableFixOptions(bool enable)
{
	output_file_sel->setEnabled(enable);
	input_file_sel->setEnabled(enable);
	fix_tries_sb->setEnabled(enable);
	load_model_chk->setEnabled(enable);
}

void ModelFixWidget::showEvent(QShowEvent *)
{
	pgmodeler_cli_sel->setSelectedFile(GlobalAttributes::getPgModelerCLIPath());
}

void ModelFixWidget::enableFix()
{
	pgmodeler_cli_sel->setVisible(pgmodeler_cli_sel->hasWarning());
	pgmodeler_cli_lbl->setVisible(pgmodeler_cli_sel->hasWarning());

	emit s_modelFixEnabled(isFixEnabled());
}

bool ModelFixWidget::isFixEnabled()
{
	return !input_file_sel->hasWarning() && !input_file_sel->getSelectedFile().isEmpty() &&
				 !output_file_sel->hasWarning() && !output_file_sel->getSelectedFile().isEmpty() &&
				 !pgmodeler_cli_sel->hasWarning() && !pgmodeler_cli_sel->getSelectedFile().isEmpty();
}

void ModelFixWidget::fixModel()
{
	QStringList args;

	/* If extra CLI arguments list is not empty,
	 * we expect that it contains an special fix operation
	 * argument (which can be from a separate plugin), so,
	 * in that case, we override the normal --fix-model and
	 * let the external code to handle the fix operation */
	if(!extra_cli_args.isEmpty())
		args.append(extra_cli_args);
	else
		args.append("--fix-model");

	args.append("--fix-tries");
	args.append(QString::number(fix_tries_sb->value()));
	args.append("--input");
	args.append(input_file_sel->getSelectedFile());
	args.append("--output");
	args.append(output_file_sel->getSelectedFile());

	progress_pb->setValue(0);
	progress_pb->setVisible(true);
	dbg_output_wgt->clear();

	enableFixOptions(false);

	pgmodeler_cli_proc.blockSignals(false);
	pgmodeler_cli_proc.setArguments(args);
	pgmodeler_cli_proc.setProgram(pgmodeler_cli_sel->getSelectedFile());
	pgmodeler_cli_proc.start();

	emit s_modelFixStarted();
}

void ModelFixWidget::cancelFix()
{
	pgmodeler_cli_proc.terminate();
	pgmodeler_cli_proc.waitForFinished();
	dbg_output_wgt->logMessage(QString("\n%1\n").arg(tr("** Process cancelled by the user!")));
	enableFixOptions(true);

	emit s_modelFixFinished();
}

void ModelFixWidget::updateOutput()
{
	static QRegularExpression prog_rx { "^\\[\\d+\\%\\]" },
	perc_rx { "(?!(\\d))(\\W)" };

	QString txt;

	//Append both stdout and stderr
	txt.append(pgmodeler_cli_proc.readAllStandardOutput());
	txt.append(pgmodeler_cli_proc.readAllStandardError());

	if(txt.contains(prog_rx))
	{
		QStringList list = txt.split(QChar::LineFeed, Qt::SkipEmptyParts);
		int pos = -1;
		QString prog_str;

		for(auto &str : list)
		{
			// Extracting the progress value to display in the progress bar widget
			pos = str.indexOf("]");
			prog_str = str.mid(0, pos);
			prog_str.remove(perc_rx);

			if(prog_str.toInt() >= progress_pb->value())
			{
				progress_pb->setValue(prog_str.toInt());
				str.remove(0, pos + 2);
			}

			txt = str + "\n";
		}
	}

	dbg_output_wgt->logMessage(txt.trimmed());
}

void ModelFixWidget::handleProcessFinish(int res)
{
	enableFixOptions(true);
	pgmodeler_cli_proc.blockSignals(true);
	dbg_output_wgt->showActionButtons(true);

	emit s_modelFixFinished();

	if(res == 0)
	{
		progress_pb->setValue(100);

		// If the model was sucessfully fixed and user requested the loading
		if(load_model_chk->isChecked())
		{
			//Emitting a signal indicating the file to be loaded
			emit s_modelLoadRequested(output_file_sel->getSelectedFile());
			resetFixForm();
		}
	}
	else
	{
		dbg_output_wgt->logMessage("");
		dbg_output_wgt->logMessage(tr("** Executed command was:"));
		dbg_output_wgt->logMessage("> " + pgmodeler_cli_proc.program() + " " + pgmodeler_cli_proc.arguments().join(" "));
	}
}

