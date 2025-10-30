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

#include "schemaeditorform.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "globalattributes.h"
#include "messagebox.h"
#include "settings/appearanceconfigwidget.h"
#include "settings/generalconfigwidget.h"
#include "sourceeditorwidget.h"
#include "aboutsewidget.h"
#include "baseform.h"
#include "utilsns.h"
#include "qtconnectmacros.h"
#include <QActionGroup>
#include <QTemporaryFile>

const QString SchemaEditorForm::UntitledFile { QT_TR_NOOP("(untitled)") };

SchemaEditorForm::SchemaEditorForm(QWidget *parent) : QWidget(parent)
{
	QToolButton *btn = nullptr;
	QFont fnt;

	setupUi(this);
	setWindowTitle(windowTitle() + " " + GlobalAttributes::PgModelerVersion);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	for(auto &obj : btns_parent_wgt->children())
	{
		btn = dynamic_cast<QToolButton *>(obj);
		if(!btn) continue;

		fnt = btn->font();
		fnt.setWeight(QFont::Normal);
		btn->setFont(fnt);
		GuiUtilsNs::createDropShadow(btn, 1, 1, 5);

		if(!btn->toolTip().isEmpty() && !btn->shortcut().toString().isEmpty())
			btn->setToolTip(btn->toolTip() + QString(" (%1)").arg(btn->shortcut().toString()));
	}

	AppearanceConfigWidget appearance_conf_wgt;
	appearance_conf_wgt.loadConfiguration();
	GuiUtilsNs::updateDropShadows(qApp->allWidgets());

	GeneralConfigWidget general_conf_wgt;
	general_conf_wgt.loadConfiguration();

	alert_frm->setVisible(false);

	syntax_txt = GuiUtilsNs::createNumberedTextEditor(syntax_wgt);
	syntax_hl = new SyntaxHighlighter(syntax_txt);
	syntax_hl->loadConfiguration(GlobalAttributes::getXMLHighlightConfPath());

	dtd_txt = GuiUtilsNs::createNumberedTextEditor(dtd_wgt);
	dtd_txt->setReadOnly(true);
	dtd_hl = new SyntaxHighlighter(dtd_txt);
	dtd_hl->loadConfiguration(GlobalAttributes::getXMLHighlightConfPath());

	SourceEditorWidget::setDefaultEditorPalette(syntax_txt->palette());

	syntax_conf_sel = new FileSelectorWidget(syntax_conf_wgt);
	syntax_conf_sel->setReadOnly(true);

	QVBoxLayout *vbox = new QVBoxLayout(syntax_conf_wgt);
	vbox->addWidget(syntax_conf_sel);
	vbox->setContentsMargins(0, 0, 0, 0);

	syntax_conf_sel->setNameFilters({ tr("Syntax highlight config file (*%1)").arg(GlobalAttributes::ConfigurationExt) });

	QAction *act = nullptr;
	stx_action_grp = new QActionGroup(&syntax_cfg_menu);

	act = syntax_cfg_menu.addAction("Schema file", this, &SchemaEditorForm::loadSyntaxConfig);
	stx_action_grp->addAction(act);
	act->setCheckable(true);
	act->setChecked(true);
	act->setData(GlobalAttributes::SchHighlightConf);

	act = syntax_cfg_menu.addAction("XML script", this, &SchemaEditorForm::loadSyntaxConfig);
	stx_action_grp->addAction(act);
	act->setCheckable(true);
	act->setChecked(false);
	act->setData(GlobalAttributes::XMLHighlightConf);

	act = syntax_cfg_menu.addAction("SQL script", this, &SchemaEditorForm::loadSyntaxConfig);
	stx_action_grp->addAction(act);
	act->setData(GlobalAttributes::SQLHighlightConf);
	act->setCheckable(true);
	act->setChecked(false);

	syntax_tb->setMenu(&syntax_cfg_menu);
	syntax_tb->setStyleSheet("QToolButton::menu-indicator { \
image: url(':/styles/styles/h_menu_indicator.png'); \
subcontrol-position: right center; }");

	syntax_cfg_menu.installEventFilter(this);

	connect(apply_conf_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::applySyntaxConfig));
	connect(save_conf_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::saveSyntaxConfig));
	connect(reload_conf_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::loadSyntaxConfig));

	connect(new_tb, &QToolButton::clicked, this, [this](){
		__trycatch( addEditorTab(); )
	});

	connect(load_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::loadFile));
	connect(exit_tb, &QToolButton::clicked, this, &SchemaEditorForm::close);

	connect(save_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::saveFile));
	connect(indent_all_tb, &QToolButton::clicked, this, &SchemaEditorForm::indentAll);

	connect(save_all_tb, &QToolButton::clicked, this, __slot(this, SchemaEditorForm::saveAll));
	connect(close_all_tb, &QToolButton::clicked, this, &SchemaEditorForm::closeAll);

	connect(editors_tbw, &QTabWidget::tabCloseRequested, this, [this](int idx){
		closeEditorTab(idx);
	});

	connect(editors_tbw, &QTabWidget::currentChanged, this, __slot(this, SchemaEditorForm::loadSyntaxFromCurrentTab));
	connect(use_tmpl_file_chk, &QCheckBox::toggled, this, __slot(this, SchemaEditorForm::loadSyntaxConfig));

	connect(syntax_txt, &NumberedTextEditor::textChanged, this, [this](){
		alert_frm->setVisible(true);
	});

	connect(save_as_tb, &QToolButton::clicked, this, [this](){
		__trycatch( saveFile(true); )
	});

	connect(about_tb, &QToolButton::clicked, this, [](){
		AboutSEWidget *info_wgt = new AboutSEWidget;
		BaseForm base_frm;	
		base_frm.setMainWidget(info_wgt);
		base_frm.exec();
	});
}

void SchemaEditorForm::showEvent(QShowEvent *)
{
	h_splitter->setSizes({ width(), width()/2});
}

bool SchemaEditorForm::hasModifiedEditors()
{
	bool editors_modified = false;
	SourceEditorWidget *editor = nullptr;

	for(int tab = 0; tab < editors_tbw->count(); tab++)
	{
		editor = dynamic_cast<SourceEditorWidget *>(editors_tbw->widget(tab));

		if(editor->isModified())
		{
			editors_modified = true;
			break;
		}
	}

	return editors_modified;
}

void SchemaEditorForm::closeEvent(QCloseEvent *event)
{
	if(alert_frm->isVisible() || hasModifiedEditors())
	{
		int res = Messagebox::confirm(tr("There are modified files! Do you want to exit without saving them?"));

		if(Messagebox::isRejected(res))
			event->ignore();
	}
}

bool SchemaEditorForm::eventFilter(QObject *object, QEvent *event)
{
	if(object == &syntax_cfg_menu && event->type() == QEvent::Show)
	{
		syntax_cfg_menu.move(mapToGlobal(syntax_tb->pos() + QPoint(syntax_tb->width(), 0)));
		syntax_cfg_menu.show();
		return true;
	}

	return QWidget::eventFilter(object, event);
}

void SchemaEditorForm::loadSyntaxFromCurrentTab()
{
	SourceEditorWidget *editor = dynamic_cast<SourceEditorWidget *>(editors_tbw->currentWidget());

	if(!editor)
		return;

	stx_action_grp->blockSignals(true);

	for(auto &act : stx_action_grp->actions())
	{
		if(act->data().toString() == editor->getCurrentSyntaxConfig())
		{
			act->setChecked(true);
			loadSyntaxConfig();
			break;
		}
	}

	stx_action_grp->blockSignals(true);
}

void SchemaEditorForm::loadSyntaxConfig()
{
	QAction *act = stx_action_grp->checkedAction();
	QFile input;
	QString filename;

	if(!act)
		filename = GlobalAttributes::getSchHighlightConfPath();
	else
	{
		if(!use_tmpl_file_chk->isChecked())
			filename = GlobalAttributes::getConfigurationFilePath(act->data().toString());
		else
			filename = GlobalAttributes::getTmplConfigurationFilePath("", act->data().toString() + GlobalAttributes::ConfigurationExt);
	}

	try
	{
		syntax_txt->setPlainText(UtilsNs::loadFile(filename));
		syntax_conf_sel->setSelectedFile(filename);

		if(dtd_txt->toPlainText().isEmpty())
		{
			filename = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir,
																																GlobalAttributes::CodeHighlightConf +
																																GlobalAttributes::ObjectDTDExt);

			dtd_txt->setPlainText(UtilsNs::loadFile(filename));
		}

		save_conf_tb->setEnabled(true);
		apply_conf_tb->setEnabled(true);
		reload_conf_tb->setEnabled(true);
		alert_frm->setVisible(false);
		applySyntaxConfig(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void SchemaEditorForm::applySyntaxConfig(bool from_temp_file)
{
	QTemporaryFile tmp_file;
	QString filename;

	/* When applying the syntax on-the-fly we save the syntax conf code to a temporary file
	 * and use it as configuration for in the open editors */
	if(from_temp_file)
	{
		tmp_file.setAutoRemove(false);
		tmp_file.setFileTemplate(GlobalAttributes::getTemporaryFilePath(QString("temp_XXXXXX%1").arg(GlobalAttributes::ConfigurationExt)));
		tmp_file.open();
		filename = tmp_file.fileName();

		if(!tmp_file.isOpen())
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(filename),
											ErrorCode::FileDirectoryNotAccessed, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		tmp_file.write(syntax_txt->toPlainText().toUtf8());
		tmp_file.close();
	}
	else if(stx_action_grp->checkedAction())
	{
		if(!use_tmpl_file_chk->isChecked())
			filename = GlobalAttributes::getConfigurationFilePath(stx_action_grp->checkedAction()->data().toString());
		else
			filename = GlobalAttributes::getTmplConfigurationFilePath("", stx_action_grp->checkedAction()->data().toString() + GlobalAttributes::ConfigurationExt);
	}

	try
	{
		/* Testing the temp file contents against a dummy syntax highlighter before
		 * applying to the open editors */
		QPlainTextEdit dummy_txt;
		SyntaxHighlighter stx_hl(&dummy_txt);
		stx_hl.loadConfiguration(filename);

		SourceEditorWidget *editor = dynamic_cast<SourceEditorWidget *>(editors_tbw->currentWidget());

		if(editor)
			editor->loadSyntaxConfig(filename);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	if(from_temp_file)
		tmp_file.remove();
}

void SchemaEditorForm::saveSyntaxConfig()
{
	UtilsNs::saveFile(syntax_conf_sel->getSelectedFile(), syntax_txt->toPlainText().toUtf8());
	alert_frm->setVisible(false);
	applySyntaxConfig(true);
}

void SchemaEditorForm::saveFile(bool save_as)
{
	SourceEditorWidget *editor = dynamic_cast<SourceEditorWidget *>(editors_tbw->widget(editors_tbw->currentIndex()));
	QString filename = editor->getFilename();

	if(save_as || filename.isEmpty())
	{
		QStringList files = showFileDialog(true);

		if(files.isEmpty())
			return;

		filename = files.at(0);
	}

	editor->saveFile(filename);
	editor->setModified(false);
	QFileInfo fi(filename);
	editors_tbw->setTabText(editors_tbw->currentIndex(), fi.fileName());
	editors_tbw->setTabToolTip(editors_tbw->currentIndex(), fi.absoluteFilePath());
}

void SchemaEditorForm::setTabModified(bool modified)
{
	SourceEditorWidget *editor = dynamic_cast<SourceEditorWidget *>(sender());
	int idx = editors_tbw->indexOf(editor);
	QString tab_text = editors_tbw->tabText(idx);	

	if(modified && !tab_text.endsWith('*'))
		tab_text += '*';
	else if(!modified)
		tab_text.remove('*');

	editors_tbw->setTabText(idx, tab_text);
}

void SchemaEditorForm::indentAll()
{
	SourceEditorWidget *editor = nullptr;

	qApp->setOverrideCursor(Qt::WaitCursor);

	for(int tab = 0; tab < editors_tbw->count(); tab++)
	{
		editor = dynamic_cast<SourceEditorWidget *>(editors_tbw->widget(tab));
		editor->indent_tb->click();
	}

	qApp->restoreOverrideCursor();
}

void SchemaEditorForm::saveAll()
{
	qApp->setOverrideCursor(Qt::WaitCursor);

	for(int tab = 0; tab < editors_tbw->count(); tab++)
	{
		editors_tbw->setCurrentIndex(tab);

		try
		{
			saveFile();
		}
		catch(Exception &e)
		{
			Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			break;
		}
	}

	qApp->restoreOverrideCursor();
}

void SchemaEditorForm::closeAll()
{
	if(hasModifiedEditors())
	{
		int res = Messagebox::confirm(tr("There are modified files! Do you want to close them without save?"));

		if(Messagebox::isRejected(res))
			return;
	}

	qApp->setOverrideCursor(Qt::WaitCursor);

	while(editors_tbw->count() > 0)
		closeEditorTab(0, false);

	qApp->restoreOverrideCursor();
}

QStringList SchemaEditorForm::showFileDialog(bool save_mode)
{
	QFileDialog file_dlg;
	QStringList files, filters= {
		tr("Schema file (*%1)").arg(GlobalAttributes::SchemaExt),
		tr("Database model file (*%1)").arg(GlobalAttributes::DbModelExt),
		tr("pgModeler config file (*%1)").arg(GlobalAttributes::ConfigurationExt),
		tr("Objects metadata file (*%1)").arg(GlobalAttributes::ObjMetadataExt),
		tr("SQL script file (*.sql)"),
		tr("XML file (*.xml)"),
		tr("DTD file (*.dtd)"),
		tr("All files (*.*)")
	};

	if(!save_mode)
		filters.prepend(tr("All supported files (*%1 *%2 *%3 *%4 *.sql *.xml *.dtd)")
										.arg(GlobalAttributes::SchemaExt, GlobalAttributes::DbModelExt,
												 GlobalAttributes::ConfigurationExt, GlobalAttributes::ObjMetadataExt));

	file_dlg.setNameFilters(filters);

	file_dlg.setWindowTitle(save_mode ? tr("Save file") : tr("Load file"));
	file_dlg.setFileMode(save_mode ? QFileDialog::AnyFile : QFileDialog::ExistingFiles);
	file_dlg.setAcceptMode(save_mode ? QFileDialog::AcceptSave : QFileDialog::AcceptOpen);

	if(save_mode)
	{
		file_dlg.setDefaultSuffix(".sch");
		connect(&file_dlg, &QFileDialog::filterSelected, this, [&file_dlg](QString filter){
			filter.remove(QRegularExpression("(.)+(\\*)"));
			filter.remove(")");
			file_dlg.setDefaultSuffix(filter);
		});
	}

	GuiUtilsNs::restoreFileDialogState(&file_dlg);

	if(file_dlg.exec() == QFileDialog::Accepted)
		files = file_dlg.selectedFiles();

	GuiUtilsNs::saveFileDialogState(&file_dlg);

	return files;
}

void SchemaEditorForm::loadFile()
{
	try
	{
		QStringList files = showFileDialog(false);

		if(!files.isEmpty())
			loadFiles(files);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void SchemaEditorForm::loadFiles(const QStringList &filenames)
{
	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		for(auto &file : filenames)
			addEditorTab(file);

		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void SchemaEditorForm::addEditorTab(const QString &filename)
{
	SourceEditorWidget *editor_wgt = nullptr;
	QFileInfo fi(filename);

	try
	{
		editor_wgt = new SourceEditorWidget;

		if(!filename.isEmpty())
			editor_wgt->loadFile(filename);

		connect(editor_wgt, &SourceEditorWidget::s_editorModified, this, &SchemaEditorForm::setTabModified);
	}
	catch(Exception &e)
	{
		delete editor_wgt;
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}

	editors_tbw->addTab(editor_wgt, filename.isEmpty() ? UntitledFile : fi.fileName());
	editors_tbw->setTabToolTip(editors_tbw->count() - 1, filename.isEmpty() ? "" : fi.absoluteFilePath());
	editors_tbw->setCurrentIndex(editors_tbw->count() - 1);
	save_as_tb->setEnabled(true);
	save_tb->setEnabled(true);
	indent_all_tb->setEnabled(true);
	save_all_tb->setEnabled(true);
	close_all_tb->setEnabled(true);
	syntax_cfg_edit_frm->setEnabled(true);
	syntax_tb->setEnabled(true);
}

void SchemaEditorForm::closeEditorTab(int idx, bool confirm_close)
{
	SourceEditorWidget *editor_wgt = dynamic_cast<SourceEditorWidget *>(editors_tbw->widget(idx));

	if(editor_wgt->isModified() && confirm_close)
	{
		int res = Messagebox::confirm(tr("The source code was modified! Do you really want to close it without save?"));

		if(Messagebox::isRejected(res))
			return;
	}

	editors_tbw->removeTab(idx);
	delete(editor_wgt);

	bool enable = editors_tbw->count() > 0;
	save_as_tb->setEnabled(enable);
	save_tb->setEnabled(enable);
	indent_all_tb->setEnabled(enable);
	save_all_tb->setEnabled(enable);
	close_all_tb->setEnabled(enable);
	syntax_cfg_edit_frm->setEnabled(enable);
	syntax_tb->setEnabled(enable);

	if(!enable)
	{
		syntax_txt->blockSignals(true);
		syntax_conf_sel->blockSignals(true);

		syntax_txt->clear();
		syntax_conf_sel->clearSelector();
		alert_frm->setVisible(false);

		syntax_txt->blockSignals(false);
		syntax_conf_sel->blockSignals(false);
	}
}
