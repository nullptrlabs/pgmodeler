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

#include "modelfixform.h"
#include "globalattributes.h"

const QString ModelFixForm::PgModelerCli {
#ifdef Q_OS_WIN
	"pgmodeler-cli.exe"
#else
	"pgmodeler-cli"
#endif
};

ModelFixForm::ModelFixForm(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);

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
	model_fix_grid->addWidget(output_file_sel, 2, 2);

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

	connect(&pgmodeler_cli_proc, &QProcess::readyReadStandardOutput, this, &ModelFixForm::updateOutput);
	connect(&pgmodeler_cli_proc, &QProcess::readyReadStandardError, this, &ModelFixForm::updateOutput);
	connect(&pgmodeler_cli_proc, &QProcess::finished, this, &ModelFixForm::handleProcessFinish);
	connect(fix_btn, &QPushButton::clicked, this, &ModelFixForm::fixModel);
	connect(input_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixForm::enableFix);
	connect(output_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixForm::enableFix);
	connect(pgmodeler_cli_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelFixForm::enableFix);
	connect(close_btn, &QPushButton::clicked, this, &ModelFixForm::close);

	connect(cancel_btn, &QPushButton::clicked, this, [this](){
		cancelFix();
	});

	resetFixForm();
}

void ModelFixForm::setExtraCliArgs(const QStringList &extra_args)
{
	extra_cli_args = extra_args;
}

void ModelFixForm::resetFixForm()
{
	pgmodeler_cli_lbl->setVisible(false);
	pgmodeler_cli_sel->setVisible(false);
	input_file_sel->clearSelector();
	output_file_sel->clearSelector();

	dbg_output_wgt->logMessage(tr("Waiting for the process to start..."));
	dbg_output_wgt->showActionButtons(false);

	load_model_chk->setChecked(true);
	enableFixOptions(true);

	progress_pb->setVisible(false);
	cancel_btn->setVisible(false);
}

void ModelFixForm::enableFixOptions(bool enable)
{
	fix_btn->setEnabled(enable);
	output_file_sel->setEnabled(enable);
	input_file_sel->setEnabled(enable);
	fix_tries_sb->setEnabled(enable);
	load_model_chk->setEnabled(enable);
}

void ModelFixForm::closeEvent(QCloseEvent *event)
{
	if(pgmodeler_cli_proc.state() == QProcess::Running)
		event->ignore();
	else
		resetFixForm();
}

int ModelFixForm::exec()
{
	pgmodeler_cli_sel->setSelectedFile(GlobalAttributes::getPgModelerCLIPath());
	return QDialog::exec();
}

void ModelFixForm::enableFix()
{
	pgmodeler_cli_sel->setVisible(pgmodeler_cli_sel->hasWarning());
	pgmodeler_cli_lbl->setVisible(pgmodeler_cli_sel->hasWarning());

	fix_btn->setEnabled(!input_file_sel->hasWarning() && !input_file_sel->getSelectedFile().isEmpty() &&
											!output_file_sel->hasWarning() && !output_file_sel->getSelectedFile().isEmpty() &&
											!pgmodeler_cli_sel->hasWarning() && !pgmodeler_cli_sel->getSelectedFile().isEmpty());
}

void ModelFixForm::fixModel()
{
	QString cmd = QString("\"%1\"");
	QStringList args;

#ifdef Q_OS_MAC
	cmd+=" pgmodeler-cli";
#endif

	args.append("--fix-model");
	args.append("--fix-tries");
	args.append(QString::number(fix_tries_sb->value()));
	args.append("--input");
	args.append(input_file_sel->getSelectedFile());
	args.append("--output");
	args.append(output_file_sel->getSelectedFile());
	args.append(extra_cli_args);

	progress_pb->setValue(0);
	progress_pb->setVisible(true);
	cancel_btn->setEnabled(true);
	cancel_btn->setVisible(true);

	dbg_output_wgt->clear();

	enableFixOptions(false);

	pgmodeler_cli_proc.blockSignals(false);
	pgmodeler_cli_proc.setArguments(args);
	pgmodeler_cli_proc.setProgram(pgmodeler_cli_sel->getSelectedFile());
	pgmodeler_cli_proc.start();
}

void ModelFixForm::cancelFix()
{
	cancel_btn->setEnabled(false);
	pgmodeler_cli_proc.terminate();
	pgmodeler_cli_proc.waitForFinished();
	dbg_output_wgt->logMessage(QString("\n%1\n").arg(tr("** Process cancelled by the user!")));
	enableFixOptions(true);
}

void ModelFixForm::updateOutput()
{
	QString txt;

	//Append both stdout and stderr
	txt.append(pgmodeler_cli_proc.readAllStandardOutput());
	txt.append(pgmodeler_cli_proc.readAllStandardError());

	if(txt.contains(QRegularExpression("^\\[\\d+\\%\\]")))
	{
		QStringList list = txt.split(QChar::LineFeed, Qt::SkipEmptyParts);
		int pos = -1;
		QString prog_str;

		for(auto &str : list)
		{
			// Extracting the progress value to display in the progress bar widget
			pos = str.indexOf("]");
			prog_str = str.mid(0, pos);
			prog_str.remove(QRegularExpression("(?!(\\d))(\\W)"));

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

void ModelFixForm::handleProcessFinish(int res)
{
	enableFixOptions(true);
	pgmodeler_cli_proc.blockSignals(true);
	cancel_btn->setEnabled(false);
	dbg_output_wgt->showActionButtons(true);

	if(res == 0)
	{
		progress_pb->setValue(100);

		/* If the model as sucessfully fixed and user
		 requested the loading */
		if(load_model_chk->isChecked())
		{
			//Emit a signal indicating the file to be loaded
			emit s_modelLoadRequested(output_file_sel->getSelectedFile());
			close();
		}
	}
}

