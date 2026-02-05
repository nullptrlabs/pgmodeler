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

#include "metadatahandlingform.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include <QTemporaryFile>

MetadataHandlingForm::MetadataHandlingForm(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	setupUi(this);

	root_item=nullptr;
	model_wgt=nullptr;
	settings_tbw->setTabEnabled(1, false);
	apply_btn->setEnabled(false);

	htmlitem_deleg=new HtmlItemDelegate(this);
	output_trw->setItemDelegateForColumn(0, htmlitem_deleg);

	backup_file_sel = new FileSelectorWidget(this);
	backup_file_sel->setNameFilters({tr("Objects metadata file (*%1)").arg(GlobalAttributes::ObjMetadataExt), tr("All files (*.*)")});
	backup_file_sel->setWindowTitle(tr("Select backup file"));
	settings_grid->addWidget(backup_file_sel, 6, 2);

	connect(close_btn, &QPushButton::clicked, this, &MetadataHandlingForm::reject);
	connect(apply_btn, &QPushButton::clicked, this, __slot(this, MetadataHandlingForm::handleObjectsMetada));

	connect(extract_from_cmb, &QComboBox::currentTextChanged, this, [this](){
		apply_btn->setDisabled(extract_from_cmb->count() == 0);
	});

	connect(extract_from_cmb, &QComboBox::currentIndexChanged, this, &MetadataHandlingForm::enableMetadataHandling);
	connect(backup_file_sel, &FileSelectorWidget::s_selectorChanged, this, &MetadataHandlingForm::enableMetadataHandling);
	connect(restore_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::configureSelector);
	connect(restore_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::enableMetadataHandling);
	connect(extract_restore_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::configureSelector);
	connect(extract_restore_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::enableMetadataHandling);
	connect(extract_only_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::configureSelector);
	connect(extract_only_rb, &QRadioButton::toggled, this, &MetadataHandlingForm::enableMetadataHandling);
	connect(select_all_btn, &QPushButton::clicked, this, &MetadataHandlingForm::selectAllOptions);
	connect(clear_all_btn,  &QPushButton::clicked, this, &MetadataHandlingForm::selectAllOptions);

	configureSelector();
}

void MetadataHandlingForm::enableMetadataHandling()
{
	merge_dup_objs_chk->setEnabled(!extract_only_rb->isChecked());

	if(extract_only_rb->isChecked())
		merge_dup_objs_chk->setChecked(false);

	extract_from_cmb->setVisible(!restore_rb->isChecked());
	extract_from_lbl->setVisible(!restore_rb->isChecked());
	apply_to_lbl->setVisible(!extract_only_rb->isChecked());
	apply_to_edt->setVisible(!extract_only_rb->isChecked());

	apply_btn->setEnabled(model_wgt &&
												(((extract_restore_rb->isChecked() && extract_from_cmb->count() > 0) ||
													(extract_only_rb->isChecked() && extract_from_cmb->count() > 0 && !backup_file_sel->getSelectedFile().isEmpty() && !backup_file_sel->hasWarning()) ||
													(restore_rb->isChecked() && !backup_file_sel->getSelectedFile().isEmpty() && !backup_file_sel->hasWarning()))));
}

void MetadataHandlingForm::selectAllOptions()
{
	bool check = sender() == select_all_btn;
	QCheckBox *checkbox = nullptr;

	for(auto &obj : options_grp->children())
	{
		checkbox = dynamic_cast<QCheckBox *>(obj);

		if(checkbox)
			checkbox->setChecked(check);
	}
}

void MetadataHandlingForm::setModelWidget(ModelWidget *model_wgt)
{
	this->model_wgt=model_wgt;

	apply_to_edt->clear();

	if(model_wgt)
	{
		apply_to_edt->setText(QString("%1 (%2)").arg(model_wgt->getDatabaseModel()->getName())
													.arg(model_wgt->getFilename().isEmpty() ? tr("model not saved yet") : model_wgt->getFilename()));
	}
}

void MetadataHandlingForm::setModelWidgets(QList<ModelWidget *> models)
{
	extract_from_cmb->clear();

	for(ModelWidget *model : models)
	{
		extract_from_cmb->addItem(QString("%1 (%2)").arg(model->getDatabaseModel()->getName())
															.arg(model->getFilename().isEmpty() ? tr("model not saved yet") : model->getFilename()),
															QVariant::fromValue<void *>(reinterpret_cast<void *>(model->getDatabaseModel())));
	}
}

void MetadataHandlingForm::handleObjectsMetada()
{
	if(!backup_file_sel->getSelectedFile().isEmpty() &&
		 backup_file_sel->getSelectedFile() == model_wgt->getFilename())
		throw Exception(tr("The backup file cannot be the same as the input model!"),
										ErrorCode::Custom,	__PRETTY_FUNCTION__,__FILE__,__LINE__);

	QTemporaryFile tmp_file;
	QString metadata_file;
	DatabaseModel::MetaAttrOptions options = DatabaseModel::MetaNoOpts;
	DatabaseModel *extract_model=nullptr;

	try
	{
		root_item=nullptr;
		output_trw->clear();
		settings_tbw->setTabEnabled(1, true);
		settings_tbw->setCurrentIndex(1);

		options|=(db_metadata_chk->isChecked() ? DatabaseModel::MetaDbAttributes : DatabaseModel::MetaNoOpts);
		options|=(custom_colors_chk->isChecked() ? DatabaseModel::MetaObjsCustomColors : DatabaseModel::MetaNoOpts);
		options|=(custom_sql_chk->isChecked() ? DatabaseModel::MetaObjsCustomSql : DatabaseModel::MetaNoOpts);
		options|=(objs_positioning_chk->isChecked() ? DatabaseModel::MetaObjsPositioning : DatabaseModel::MetaNoOpts);
		options|=(objs_protection_chk->isChecked() ? DatabaseModel::MetaObjsProtection : DatabaseModel::MetaNoOpts);
		options|=(objs_sql_disabled_chk->isChecked() ? DatabaseModel::MetaObjsSqlDisabled : DatabaseModel::MetaNoOpts);
		options|=(tag_objs_chk->isChecked() ? DatabaseModel::MetaTagObjs : DatabaseModel::MetaNoOpts);
		options|=(textbox_objs_chk->isChecked() ? DatabaseModel::MetaTextboxObjs : DatabaseModel::MetaNoOpts);
		options|=(objs_fadedout_chk->isChecked() ? DatabaseModel::MetaObjsFadeOut : DatabaseModel::MetaNoOpts);
		options|=(objs_collapse_mode_chk->isChecked() ? DatabaseModel::MetaObjsCollapseMode : DatabaseModel::MetaNoOpts);
		options|=(generic_sql_objs_chk->isChecked() ? DatabaseModel::MetaGenericSqlObjs : DatabaseModel::MetaNoOpts);
		options|=(objs_aliases_chk->isChecked() ? DatabaseModel::MetaObjsAliases : DatabaseModel::MetaNoOpts);
		options|=(objs_z_stack_value_chk->isChecked() ? DatabaseModel::MetaObjsZStackValue : DatabaseModel::MetaNoOpts);
		options|=(objs_layers_config_chk->isChecked() ? DatabaseModel::MetaObjsLayersConfig : DatabaseModel::MetaNoOpts);
		options|=(merge_dup_objs_chk->isChecked() ? DatabaseModel::MetaMergeDuplicatedObjs : DatabaseModel::MetaNoOpts);

		connect(model_wgt->getDatabaseModel(), &DatabaseModel::s_objectLoaded, this, &MetadataHandlingForm::updateProgress, Qt::UniqueConnection);

		if(extract_restore_rb->isChecked() || extract_only_rb->isChecked())
		{
			extract_model=reinterpret_cast<DatabaseModel *>(extract_from_cmb->currentData(Qt::UserRole).value<void *>());

			if(extract_only_rb->isChecked())
				metadata_file = backup_file_sel->getSelectedFile();
			else
			{
				//Configuring the temporary metadata file
				tmp_file.setFileTemplate(GlobalAttributes::getTemporaryFilePath(
																	 QString("%1_metadata_XXXXXX%2")
																	 .arg(extract_model->getName(), GlobalAttributes::ObjMetadataExt)));

				tmp_file.open();
				metadata_file=tmp_file.fileName();
				tmp_file.close();
			}

			connect(extract_model, &DatabaseModel::s_objectLoaded, this, &MetadataHandlingForm::updateProgress, Qt::UniqueConnection);

			root_item=GuiUtilsNs::createOutputTreeItem(output_trw,
																										UtilsNs::formatMessage(tr("Extracting metadata to file `%1'").arg(metadata_file)),
																										QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

			extract_model->saveObjectsMetadata(metadata_file, static_cast<DatabaseModel::MetaAttrOptions>(options));

			if(extract_restore_rb->isChecked() && !backup_file_sel->getSelectedFile().isEmpty())
			{
				root_item->setExpanded(false);
				root_item=GuiUtilsNs::createOutputTreeItem(output_trw,
																											UtilsNs::formatMessage(tr("Saving backup metadata to file `%1'").arg(backup_file_sel->getSelectedFile())),
																											QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

				model_wgt->getDatabaseModel()->saveObjectsMetadata(backup_file_sel->getSelectedFile());
			}
		}
		else
		{
			metadata_file = backup_file_sel->getSelectedFile();
		}

		if(root_item)
			root_item->setExpanded(false);

		if(!extract_only_rb->isChecked())
		{
			root_item=GuiUtilsNs::createOutputTreeItem(output_trw,
																										UtilsNs::formatMessage(tr("Applying metadata from file `%1'").arg(metadata_file)),
																										QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

			model_wgt->setUpdatesEnabled(false);
			model_wgt->getDatabaseModel()->loadObjectsMetadata(metadata_file, static_cast<DatabaseModel::MetaAttrOptions>(options));
			model_wgt->adjustSceneRect(false);
			model_wgt->updateSceneLayers();
			model_wgt->restoreLastCanvasPosition();
			model_wgt->setUpdatesEnabled(true);
			model_wgt->setModified(true);
			model_wgt->updateObjectsOpacity();
		}

		disconnect(model_wgt->getDatabaseModel(), nullptr, this, nullptr);

		if(extract_model)
			disconnect(extract_model, nullptr, this, nullptr);

		emit s_metadataHandled();
	}
	catch(Exception &e)
	{
		QPixmap icon=QPixmap(GuiUtilsNs::getIconPath("error"));

		disconnect(model_wgt->getDatabaseModel(), nullptr, this, nullptr);

		if(extract_model)
			disconnect(extract_model, nullptr, this, nullptr);

		GuiUtilsNs::createOutputTreeItem(output_trw,
																				UtilsNs::formatMessage(e.getErrorMessage()),
																				icon, nullptr);

		ico_lbl->setPixmap(icon);
		progress_lbl->setText(tr("Metadata processing aborted!"));

		throw Exception(e.getErrorMessage(),e.getErrorCode(),__PRETTY_FUNCTION__,__FILE__,__LINE__, &e);
	}
}

void MetadataHandlingForm::showEvent(QShowEvent *)
{
	if(!model_wgt)
	{
		apply_btn->setEnabled(false);
		settings_tbw->setEnabled(false);
	}
}

void MetadataHandlingForm::configureSelector()
{
	if(extract_restore_rb->isChecked() || extract_only_rb->isChecked())
	{
		backup_file_sel->setFileDialogTitle(tr("Save backup file"));
		backup_file_sel->setFileMustExist(false);
		backup_file_sel->setAcceptMode(QFileDialog::AcceptSave);
	}
	else
	{
		backup_file_sel->setFileDialogTitle(tr("Load backup file"));
		backup_file_sel->setFileMustExist(true);
		backup_file_sel->setAcceptMode(QFileDialog::AcceptOpen);
	}
}

void MetadataHandlingForm::updateProgress(int progress, QString msg, unsigned int type_id)
{
	ObjectType obj_type=static_cast<ObjectType>(type_id);
	QString fmt_msg=UtilsNs::formatMessage(msg);
	QPixmap icon;

	if(obj_type==ObjectType::BaseObject)
	{
		if(progress==100)
			icon=QPixmap(GuiUtilsNs::getIconPath("info"));
		else
			icon=QPixmap(GuiUtilsNs::getIconPath("alert"));
	}
	else
		icon=QPixmap(GuiUtilsNs::getIconPath(obj_type));

	GuiUtilsNs::createOutputTreeItem(output_trw, fmt_msg, icon, root_item);
	progress_lbl->setText(fmt_msg);
	ico_lbl->setPixmap(icon);
	progress_pb->setValue(progress);
}
