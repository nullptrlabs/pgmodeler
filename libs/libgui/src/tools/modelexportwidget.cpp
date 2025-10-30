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

#include "modelexportwidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include "connectionsconfigwidget.h"
#include "pgsqlversions.h"
#include <QThread>
#include <QButtonGroup>

bool ModelExportWidget::low_verbosity {false};

ModelExportWidget::ModelExportWidget(QWidget *parent) : QWidget(parent)
{
	model_wgt = nullptr;
	viewp = nullptr;
	setupUi(this);

	alert_frm->setVisible(false);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);
	GuiUtilsNs::configureWidgetsFont({ export_btn, cancel_btn }, GuiUtilsNs::BigFontFactor);

	model_sel_wgt = new ModelDbSelectorWidget(this);
	input_model_gb->layout()->addWidget(model_sel_wgt);

	sql_file_sel = new FileSelectorWidget(this);
	sql_file_sel->setFileDialogTitle(tr("Export model to SQL file"));
	sql_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	sql_file_sel->setAllowFilenameInput(true);
	sql_file_sel->setFileIsMandatory(false);
	sql_file_sel->setAppendSuffix(true);
	export_to_file_grid->addWidget(sql_file_sel, 1, 1);

	img_file_sel = new FileSelectorWidget(this);
	img_file_sel->setFileDialogTitle(tr("Export model to graphics file"));
	img_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	img_file_sel->setAllowFilenameInput(true);
	img_file_sel->setFileIsMandatory(false);
	img_file_sel->setAppendSuffix(true);
	export_to_img_grid->addWidget(img_file_sel, 2, 1, 1, 3);

	dict_file_sel = new FileSelectorWidget(this);
	dict_file_sel->setFileDialogTitle(tr("Export model to data dictionary"));
	dict_file_sel->setAllowFilenameInput(true);
	dict_file_sel->setFileIsMandatory(false);
	dict_file_sel->setAppendSuffix(true);
	export_to_dict_grid->addWidget(dict_file_sel, 1, 1, 1, 5);

	htmlitem_del=new HtmlItemDelegate(this);
	output_trw->setItemDelegateForColumn(0, htmlitem_del);

	export_thread=new QThread(this);
	export_hlp.moveToThread(export_thread);

	export_to_dbms_gb->setFocusProxy(export_to_dbms_tb);
	export_to_file_gb->setFocusProxy(export_to_file_tb);
	export_to_dict_gb->setFocusProxy(export_to_dict_tb);
	export_to_img_gb->setFocusProxy(export_to_img_tb);

	QButtonGroup *radios_grp = new QButtonGroup(this);

	radios_grp->addButton(export_to_dbms_tb);
	radios_grp->addButton(export_to_file_tb);
	radios_grp->addButton(export_to_dict_tb);
	radios_grp->addButton(export_to_img_tb);

	h_splitter->setSizes({800, 700});

	connect(model_sel_wgt, &ModelDbSelectorWidget::s_selectionChanged, this, [this](){
		model_wgt = model_sel_wgt->getSelectedModel();
		enableExport();
	});

	connect(export_to_dbms_tb, &QToolButton::toggled, export_to_dbms_wgt, &QWidget::setEnabled);
	connect(export_to_img_tb, &QToolButton::toggled, export_to_img_wgt, &QWidget::setEnabled);
	connect(export_to_file_tb, &QToolButton::toggled, export_to_file_wgt, &QWidget::setEnabled);
	connect(export_to_dict_tb, &QToolButton::toggled, export_to_dict_wgt, &QWidget::setEnabled);

	connect(sql_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelExportWidget::enableExport);
	connect(sql_file_sel, &FileSelectorWidget::s_fileSelected, this, &ModelExportWidget::enableExport);
	connect(sql_file_sel, &FileSelectorWidget::s_selectorCleared, this, &ModelExportWidget::enableExport);

	connect(img_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelExportWidget::enableExport);
	connect(img_file_sel, &FileSelectorWidget::s_fileSelected, this, &ModelExportWidget::enableExport);
	connect(img_file_sel, &FileSelectorWidget::s_selectorCleared, this, &ModelExportWidget::enableExport);

	connect(dict_file_sel, &FileSelectorWidget::s_selectorChanged, this, &ModelExportWidget::enableExport);
	connect(dict_file_sel, &FileSelectorWidget::s_fileSelected, this, &ModelExportWidget::enableExport);
	connect(dict_file_sel, &FileSelectorWidget::s_selectorCleared, this, &ModelExportWidget::enableExport);

	connect(export_to_file_tb, &QToolButton::clicked, this, &ModelExportWidget::selectExportMode);
	connect(export_to_dbms_tb, &QToolButton::clicked, this, &ModelExportWidget::selectExportMode);
	connect(export_to_img_tb, &QToolButton::clicked, this, &ModelExportWidget::selectExportMode);
	connect(export_to_dict_tb, &QToolButton::clicked, this, &ModelExportWidget::selectExportMode);

	connect(pgsqlvers_chk, &QCheckBox::toggled, pgsqlvers1_cmb, &QComboBox::setEnabled);
	connect(export_btn, &QPushButton::clicked, this, &ModelExportWidget::exportModel);
	connect(drop_chk, &QCheckBox::toggled, drop_mode_cmb, &QComboBox::setEnabled);

	connect(drop_mode_cmb, &QComboBox::activated, this, [this](int idx){
		force_db_drop_chk->setEnabled(idx == 1);

		if(idx == 0)
			force_db_drop_chk->setChecked(false);
	});

	connect(export_thread, &QThread::started, &export_hlp, [this](){
		output_trw->setUniformRowHeights(true);

		if(export_to_dbms_tb->isChecked())
			export_hlp.exportToDBMS();
		else if(export_to_img_tb->isChecked())
		{
			if(img_fmt_cmb->currentIndex() == 0)
				export_hlp.exportToPNG();
			else
				export_hlp.exportToSVG();
		}
		else if(export_to_dict_tb->isChecked())
			export_hlp.exportToDataDict();
		else
			export_hlp.exportToSQL();
	});

	connect(export_thread, &QThread::finished, &export_hlp, [this](){
		output_trw->setUniformRowHeights(false);
	});

	connect(&export_hlp, &ModelExportHelper::s_progressUpdated, this, &ModelExportWidget::updateProgress, Qt::BlockingQueuedConnection);
	connect(&export_hlp, &ModelExportHelper::s_exportFinished, this, &ModelExportWidget::handleExportFinished);
	connect(&export_hlp, &ModelExportHelper::s_exportCanceled, this, &ModelExportWidget::handleExportCanceled);
	connect(&export_hlp, &ModelExportHelper::s_errorIgnored, this, &ModelExportWidget::handleErrorIgnored);
	connect(&export_hlp, &ModelExportHelper::s_exportAborted, this, &ModelExportWidget::captureThreadError);

	connect(cancel_btn, &QPushButton::clicked, this, &ModelExportWidget::cancelExport);
	connect(connections_cmb, &QComboBox::currentIndexChanged, this, &ModelExportWidget::editConnections);

	connect(img_fmt_cmb, &QComboBox::currentIndexChanged, this, &ModelExportWidget::selectImageFormat);

	connect(ignore_error_codes_chk, &QCheckBox::toggled, error_codes_edt, &QLineEdit::setEnabled);
	connect(dict_mode_cmb, &QComboBox::currentIndexChanged, this, &ModelExportWidget::selectDataDictMode);
	connect(dict_format_cmb, &QComboBox::currentIndexChanged, this, &ModelExportWidget::selectDataDictMode);
	connect(sql_standalone_rb, &QRadioButton::toggled, this, &ModelExportWidget::selectSQLExportMode);
	connect(sql_split_rb, &QRadioButton::toggled, this, &ModelExportWidget::selectSQLExportMode);
	connect(sql_split_rb, &QRadioButton::toggled, code_options_cmb, &QComboBox::setEnabled);

	pgsqlvers_cmb->addItems(PgSqlVersions::AllVersions);
	pgsqlvers1_cmb->addItems(PgSqlVersions::AllVersions);

	double values[]={ ModelWidget::MinimumZoom, 0.20, 0.30, 0.5, 0.75, 1, 1.25, 1.50, 1.75, 2,
										2.25, 2.50, 2.75, 3, 3.25, 3.50, 3.75, ModelWidget::MaximumZoom };
	unsigned cnt=sizeof(values)/sizeof(double);

	for(unsigned i=0; i < cnt; i++)
		zoom_cmb->addItem(QString("%1%").arg(values[i] * 100), QVariant(values[i]));

	zoom_cmb->setCurrentText(QString("100%"));
	settings_tbw->setTabEnabled(1, false);

	selectImageFormat();
	selectDataDictMode();
	selectSQLExportMode();
}

void ModelExportWidget::setLowVerbosity(bool value)
{
	low_verbosity = value;
}

void ModelExportWidget::updateModels(const QList<ModelWidget *> &models)
{
	model_sel_wgt->updateModels(models);
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, Connection::OpExport);
	selectExportMode();

#ifdef DEMO_VERSION
	#warning "DEMO VERSION: export to DBMS is disabled in demo version."
	export_to_dbms_tb->blockSignals(true);
	export_to_dbms_tb->setEnabled(false);
	export_to_dbms_gb->setEnabled(false);
	export_to_dbms_tb->setChecked(false);
	export_to_dbms_tb->blockSignals(false);

	#warning "DEMO VERSION: export to data dictionary is disabled in demo version."
	export_to_dict_tb->blockSignals(true);
	export_to_dict_tb->setEnabled(false);
	export_to_dict_gb->setEnabled(false);
	export_to_dict_tb->blockSignals(false);

	#warning "DEMO VERSION: export to data PNG limited to zoom factor of 50%."
	zoom_cmb->setCurrentText("30%");
	zoom_cmb->setEnabled(false);

	#warning "DEMO VERSION: export to data SVG is disabled in demo version."
	img_fmt_cmb->setEnabled(false);

	export_to_file_tb->setChecked(true);
#endif
}

bool ModelExportWidget::isThreadRunning()
{
	return export_thread->isRunning();
}

void ModelExportWidget::handleErrorIgnored(QString err_code, QString err_msg, QString cmd)
{
	QTreeWidgetItem *item=nullptr;

	item=GuiUtilsNs::createOutputTreeItem(output_trw, tr("Error code <strong>%1</strong> found and ignored. Proceeding with export.").arg(err_code),
																					 QPixmap(GuiUtilsNs::getIconPath("alert")), nullptr, false);

	GuiUtilsNs::createOutputTreeItem(output_trw, UtilsNs::formatMessage(err_msg),
																			QPixmap(GuiUtilsNs::getIconPath("alert")),	item, false, true);

	GuiUtilsNs::createOutputTreeItem(output_trw, cmd, QPixmap(), item, false, true);
}

void ModelExportWidget::updateProgress(int progress, QString msg, ObjectType obj_type, QString cmd, bool is_code_gen)
{
	QTreeWidgetItem *item=nullptr;
	QString text=UtilsNs::formatMessage(msg);
	QPixmap ico;

	progress_lbl->setText(text);
	progress_pb->setValue(progress);

	if(obj_type != ObjectType::BaseObject)
		ico = QPixmap(GuiUtilsNs::getIconPath(obj_type));
	else if(!cmd.isEmpty())
		ico = QPixmap(GuiUtilsNs::getIconPath("sqlcode"));
	else
		ico = QPixmap(GuiUtilsNs::getIconPath("info"));

	ico_lbl->setPixmap(ico);

	// If low_verbosity is set only messages hinted by obj_type == BaseObject are show because they hold key info messages
	if(!is_code_gen && (!low_verbosity || (low_verbosity && obj_type == ObjectType::BaseObject && cmd.isEmpty())))
	{
		item=GuiUtilsNs::createOutputTreeItem(output_trw, text, ico, nullptr, false);

		if(!cmd.isEmpty())
			GuiUtilsNs::createOutputTreeItem(output_trw, cmd, QPixmap(), item, false);
	}
}

void ModelExportWidget::exportModel()
{
	try
	{
		// Alerting the user about dropping objects/database
		if(export_to_dbms_tb->isChecked() && drop_chk->isChecked())
		{
			Messagebox msg_box;
			QString msg;

			if(drop_mode_cmb->currentIndex() == 0)
				msg = tr("<strong>CAUTION:</strong> You are about to drop an entire database from the chosen server! All data will be completely wiped out. Do you really want to proceed?");
			else
				msg = tr("<strong>CAUTION:</strong> You are about to drop objects in a database of the chosen server! Data can be lost in the process. Do you really want to proceed?");

			msg_box.show(tr("Warning"), msg, Messagebox::Alert, Messagebox::YesNoButtons);

			if(msg_box.isRejected())
				return;
		}

		model_wgt->setInteractive(false);
		output_trw->clear();
		settings_tbw->setTabEnabled(1, true);
		settings_tbw->setCurrentIndex(1);
		enableExportModes(false);
		cancel_btn->setEnabled(true);

		//Export to png
		if(export_to_img_tb->isChecked())
		{
			viewp=new QGraphicsView(model_wgt->scene);

			if(img_fmt_cmb->currentIndex() == 0)
				export_hlp.setExportToPNGParams(model_wgt->scene, viewp, img_file_sel->getSelectedFile(),
																				zoom_cmb->itemData(zoom_cmb->currentIndex()).toDouble(),
																				show_grid_chk->isChecked(), show_delim_chk->isChecked(),
																				 page_by_page_chk->isChecked(), override_bg_color_chk->isChecked());
			else
				export_hlp.setExportToSVGParams(model_wgt->scene, img_file_sel->getSelectedFile(),
																				show_grid_chk->isChecked(),
																				show_delim_chk->isChecked());
		}
		else
		{
			progress_lbl->setText(tr("Initializing model export..."));

			if(low_verbosity)
				GuiUtilsNs::createOutputTreeItem(output_trw, tr("<strong>Low verbosity is set:</strong> only key informations and errors will be displayed."),
																						QPixmap(GuiUtilsNs::getIconPath("alert")), nullptr, false);

			//Exporting to sql file
			if(export_to_file_tb->isChecked())
			{
				progress_lbl->setText(tr("Saving file '%1'").arg(sql_file_sel->getSelectedFile()));
				export_hlp.setExportToSQLParams(model_wgt->db_model, sql_file_sel->getSelectedFile(),
																				pgsqlvers_cmb->currentText(), sql_split_rb->isChecked(),
																				static_cast<DatabaseModel::CodeGenMode>(code_options_cmb->currentIndex()),
																				gen_drop_file_chk->isChecked());
			}
			else if(export_to_dict_tb->isChecked())
			{
				export_hlp.setExportToDataDictParams(model_wgt->db_model, dict_file_sel->getSelectedFile(),
																						 incl_index_chk->isChecked(),
																						 dict_mode_cmb->currentIndex() == 1,
																						 dict_format_cmb->currentIndex() == 1);
			}
			//Exporting directly to DBMS
			else
			{
				Connection *conn=reinterpret_cast<Connection *>(connections_cmb->itemData(connections_cmb->currentIndex()).value<void *>());
				QString version;

				//If the user chose a specific version
				if(pgsqlvers1_cmb->isEnabled())
					version=pgsqlvers1_cmb->currentText();

				export_hlp.setExportToDBMSParams(model_wgt->db_model, conn, version,
																				 ignore_dup_chk->isChecked(),
																				 drop_chk->isChecked() && drop_mode_cmb->currentIndex() == 1,
																				 drop_chk->isChecked() && drop_mode_cmb->currentIndex() == 0,
																				 false, false,
																				 drop_chk->isChecked() && force_db_drop_chk->isChecked(),
																				 run_in_transaction_chk->isChecked());

				if(ignore_error_codes_chk->isChecked())
					export_hlp.setIgnoredErrors(error_codes_edt->text().simplified().split(' '));
			}
		}

		export_thread->start();
		emit s_exportStarted();
	}
	catch(Exception &e)
	{
		finishExport(tr("Exporting process aborted!"));
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelExportWidget::selectExportMode()
{
	sql_file_sel->setFileIsMandatory(export_to_file_tb->isChecked());
	img_file_sel->setFileIsMandatory(export_to_img_tb->isChecked());
	dict_file_sel->setFileIsMandatory(export_to_dict_tb->isChecked());

	pgsqlvers1_cmb->setEnabled(export_to_dbms_tb->isChecked() && pgsqlvers_chk->isChecked());
	enableExport();
}

void ModelExportWidget::captureThreadError(Exception e)
{
	QTreeWidgetItem *item=GuiUtilsNs::createOutputTreeItem(output_trw, UtilsNs::formatMessage(e.getErrorMessage()),
																														QPixmap(GuiUtilsNs::getIconPath("error")), nullptr, false, true);

	GuiUtilsNs::createExceptionsTree(output_trw, e, item);

	ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("error")));
	finishExport(tr("Exporting process aborted!"));

	Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
}

void ModelExportWidget::cancelExport()
{
	export_hlp.cancelExport();
	cancel_btn->setEnabled(false);
}

void ModelExportWidget::handleExportCanceled()
{
	QPixmap ico=QPixmap(GuiUtilsNs::getIconPath("alert"));
	QString msg=tr("Exporting process canceled by user!");

	finishExport(msg);
	ico_lbl->setPixmap(ico);
	GuiUtilsNs::createOutputTreeItem(output_trw, msg, ico);
}

void ModelExportWidget::handleExportFinished()
{
	QPixmap ico=QPixmap(GuiUtilsNs::getIconPath("info"));
	QString msg=tr("Exporting process sucessfully ended!");

	finishExport(msg);
	ico_lbl->setPixmap(ico);
	GuiUtilsNs::createOutputTreeItem(output_trw, msg, ico);

	qApp->alert(this);
}

void ModelExportWidget::finishExport(const QString &msg)
{
	if(export_thread->isRunning())
		export_thread->quit();

	enableExportModes(true);

	model_wgt->setInteractive(true);
	cancel_btn->setEnabled(false);
	progress_pb->setValue(100);
	progress_lbl->setText(msg);
	progress_lbl->repaint();

	if(viewp)
	{
		export_thread->wait();
		delete viewp;
		viewp=nullptr;
	}

	emit s_exportFinished();
}

void ModelExportWidget::enableExportModes(bool value)
{
	export_to_dbms_tb->setEnabled(value);
	export_to_file_tb->setEnabled(value);
	export_to_img_tb->setEnabled(value);
	export_to_dict_tb->setEnabled(value);
	export_btn->setEnabled(value);
}

void ModelExportWidget::closeEvent(QCloseEvent *event)
{
	/* Ignore the close event when the thread is running this avoid
  close the form and make thread execute in background */
	if(export_thread->isRunning())
		event->ignore();
}

void ModelExportWidget::showEvent(QShowEvent *)
{
	alert_frm->setVisible(model_wgt && model_wgt->getDatabaseModel()->isInvalidated());
}

void ModelExportWidget::editConnections()
{
	if(connections_cmb->currentIndex() == connections_cmb->count()-1)
	{
		ConnectionsConfigWidget::openConnectionsConfiguration(connections_cmb, true);
		emit s_connectionsUpdateRequested();
	}

	enableExport();
}

void ModelExportWidget::enableExport()
{
	alert_frm->setVisible(model_wgt && model_wgt->getDatabaseModel()->isInvalidated());

	export_btn->setEnabled(model_wgt &&
												 (export_to_dbms_tb->isChecked() && connections_cmb->currentIndex() > 0 &&
													connections_cmb->currentIndex() != connections_cmb->count() - 1) ||
												 (export_to_file_tb->isChecked() && !sql_file_sel->hasWarning()) ||
												 (export_to_img_tb->isChecked() && !img_file_sel->hasWarning()) ||
												 (export_to_dict_tb->isChecked() && !dict_file_sel->hasWarning()));
}

void ModelExportWidget::selectImageFormat()
{
	bool is_png = img_fmt_cmb->currentIndex() == 0;

	if(is_png)
	{
		img_file_sel->setMimeTypeFilters({"image/png", "application/octet-stream"});
		img_file_sel->setDefaultSuffix("png");
		override_bg_color_chk->setEnabled(true);
	}
	else
	{
		img_file_sel->setMimeTypeFilters({"image/svg+xml", "application/octet-stream"});
		img_file_sel->setDefaultSuffix("svg");
		override_bg_color_chk->setEnabled(false);
	}

	zoom_cmb->setEnabled(is_png);
	zoom_lbl->setEnabled(is_png);
	page_by_page_chk->setEnabled(is_png);
}

void ModelExportWidget::selectDataDictMode()
{
	if(dict_mode_cmb->currentIndex() == 0)
	{
		if(dict_format_cmb->currentIndex() == 0)
		{
			dict_file_sel->setMimeTypeFilters({"text/html", "application/octet-stream"});
			dict_file_sel->setDefaultSuffix("html");
		}
		else
		{
			dict_file_sel->setMimeTypeFilters({"text/markdown", "application/octet-stream"});
			dict_file_sel->setDefaultSuffix("md");
		}

		dict_file_sel->setAcceptMode(QFileDialog::AcceptSave);
		dict_file_sel->setDirectoryMode(false);
		dict_file_sel->setFileMustExist(false);
	}
	else
	{
		dict_file_sel->setDefaultSuffix("");
		dict_file_sel->setMimeTypeFilters({});
		dict_file_sel->setDirectoryMode(true);
		dict_file_sel->setFileMustExist(false);
		dict_file_sel->setAcceptMode(QFileDialog::AcceptOpen);
	}
}

void ModelExportWidget::selectSQLExportMode()
{
	if(sql_standalone_rb->isChecked())
	{
		sql_file_sel->setMimeTypeFilters({"application/sql", "application/octet-stream"});
		sql_file_sel->setDefaultSuffix("sql");
		sql_file_sel->setFileMustExist(false);
		sql_file_sel->setDirectoryMode(false);
		sql_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	}
	else
	{
		sql_file_sel->setMimeTypeFilters({});
		sql_file_sel->setDefaultSuffix("");
		sql_file_sel->setAcceptMode(QFileDialog::AcceptOpen);
		sql_file_sel->setFileMustExist(false);
		sql_file_sel->setDirectoryMode(true);
	}
}

void ModelExportWidget::updateConnections()
{
	ConnectionsConfigWidget::fillConnectionsComboBox(connections_cmb, true, Connection::OpExport);
}
