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

#include "metadatahandlingwidget.h"
#include "utils/htmlitemdelegate.h"
#include "guiutilsns.h"
#include "utilsns.h"
#include "settings/generalconfigwidget.h"
#include <QTemporaryFile>

MetadataHandlingWidget::MetadataHandlingWidget(QWidget *parent) : QWidget(parent)
{
	setupUi(this);

	progress_wgt->setVisible(false);
	root_item = nullptr;
	output_trw->setItemDelegateForColumn(0, new HtmlItemDelegate(this));

	backup_file_sel = new FileSelectorWidget(this);
	backup_file_sel->setNameFilters({tr("Objects metadata file (*%1)").arg(GlobalAttributes::ObjMetadataExt), tr("All files (*.*)")});
	backup_file_sel->setWindowTitle(tr("Select backup file"));
	settings_grid->addWidget(backup_file_sel, 3, 1);

	extract_model_sel = new ModelDbSelectorWidget(this);
	settings_grid->addWidget(extract_model_sel, 1, 1);

	apply_model_sel = new ModelDbSelectorWidget(this);
	settings_grid->addWidget(apply_model_sel, 2, 1);

	connect(extract_model_sel, &ModelDbSelectorWidget::s_selectionChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(apply_model_sel, &ModelDbSelectorWidget::s_selectionChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(backup_file_sel, &FileSelectorWidget::s_selectorChanged, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(operation_cmb, &QComboBox::activated, this, &MetadataHandlingWidget::enableMetadataHandling);
	connect(operation_cmb, &QComboBox::activated, this, &MetadataHandlingWidget::configureSelector);
	connect(select_all_btn, &QPushButton::clicked, this, &MetadataHandlingWidget::selectAllOptions);
	connect(clear_all_btn,  &QPushButton::clicked, this, &MetadataHandlingWidget::selectAllOptions);

	configureSelector();
	enableMetadataHandling();
}

void MetadataHandlingWidget::enableMetadataHandling()
{
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());

	merge_dup_objs_chk->setEnabled(op_type != OpExtractOnly);

	if(op_type == OpExtractOnly)
		merge_dup_objs_chk->setChecked(false);

	bool extract_req = (op_type == OpExtractRestore || op_type == OpExtractOnly),
			 apply_req = (op_type == OpExtractRestore || op_type == OpRestoreOnly),
			 bkp_file_req = (op_type == OpRestoreOnly || op_type == OpExtractOnly);

	extract_model_sel->setVisible(extract_req);
	extract_from_lbl->setVisible(extract_req);

	apply_model_sel->setVisible(apply_req);
	apply_to_lbl->setVisible(apply_req);

	backup_file_sel->setVisible(bkp_file_req);
	backup_file_lbl->setVisible(bkp_file_req);

	if(!extract_req)
	{
		extract_model_sel->blockSignals(true);
		extract_model_sel->clearSelection();
		extract_model_sel->blockSignals(false);
	}

	if(!apply_req)
	{
		apply_model_sel->blockSignals(true);
		apply_model_sel->clearSelection();
		apply_model_sel->blockSignals(false);
	}

	emit s_metadataHandlingEnabled(isMetadataHandlingEnabled());
}

bool MetadataHandlingWidget::isMetadataHandlingEnabled()
{
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());

	return (op_type == OpExtractRestore &&
					extract_model_sel->isModelSelected() &&
					apply_model_sel->isModelSelected()) ||

				 (op_type == OpExtractOnly &&
					extract_model_sel->isModelSelected() &&
					!backup_file_sel->getSelectedFile().isEmpty()) ||

				 (op_type == OpRestoreOnly &&
					apply_model_sel->isModelSelected() &&
					!backup_file_sel->getSelectedFile().isEmpty());
}

void MetadataHandlingWidget::selectAllOptions()
{
	bool check = sender() == select_all_btn;

	for(auto &chk : options_gb->findChildren<QCheckBox *>())
		chk->setChecked(check);
}

void MetadataHandlingWidget::updateModels(const QList<ModelWidget *> &models)
{
	extract_model_sel->updateModels(models);
	apply_model_sel->updateModels(models);

	progress_wgt->setVisible(false);
	output_trw->clear();

	operation_cmb->setCurrentIndex(0);
	extract_model_sel->clearSelection();
	apply_model_sel->clearSelection();
	backup_file_sel->clearSelector();
}

void MetadataHandlingWidget::handleObjectsMetada()
{
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());

	if(op_type != OpExtractOnly &&
		 GeneralConfigWidget::getConfigurationParam(Attributes::Configuration,
																								Attributes::AlertApplyMetadata) != Attributes::False)
	{
		Messagebox msgbox;
		msgbox.setCustomOptionText("Always proceed without alerting me next time.");
		msgbox.show(tr("Applying the contents of a metadata file to a database model is an irreversible operation! Do you want to proceed?"),
								Messagebox::Alert, Messagebox::YesNoButtons);

		GeneralConfigWidget::appendConfigurationSection(Attributes::Configuration,
																										{{ Attributes::AlertApplyMetadata,
																											 msgbox.isCustomOptionChecked() ? Attributes::False : Attributes::True }});

		if(msgbox.isRejected())
			return;
	}

	QString bkp_filename = backup_file_sel->getSelectedFile();

	if(!bkp_filename.isEmpty())
	{
		if((extract_model_sel->isModelSelected() &&
				bkp_filename == extract_model_sel->getSelectedModel()->getFilename()) ||

			 (apply_model_sel->isModelSelected() &&
				bkp_filename == apply_model_sel->getSelectedModel()->getFilename()))
		{
			throw Exception(tr("The backup file cannot be the same as the one of the involved database model files!"),
											ErrorCode::Custom,	PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}

	QTemporaryFile tmp_file;
	QString metadata_file;
	DatabaseModel::MetaAttrOptions options = DatabaseModel::MetaNoOpts;
	DatabaseModel *extract_model = nullptr, *apply_model = nullptr;

	try
	{
		root_item = nullptr;
		output_trw->clear();
		progress_wgt->setVisible(true);

		options |= (db_metadata_chk->isChecked() ? DatabaseModel::MetaDbAttributes : DatabaseModel::MetaNoOpts);
		options |= (custom_colors_chk->isChecked() ? DatabaseModel::MetaObjsCustomColors : DatabaseModel::MetaNoOpts);
		options |= (custom_sql_chk->isChecked() ? DatabaseModel::MetaObjsCustomSql : DatabaseModel::MetaNoOpts);
		options |= (objs_positioning_chk->isChecked() ? DatabaseModel::MetaObjsPositioning : DatabaseModel::MetaNoOpts);
		options |= (objs_protection_chk->isChecked() ? DatabaseModel::MetaObjsProtection : DatabaseModel::MetaNoOpts);
		options |= (objs_sql_disabled_chk->isChecked() ? DatabaseModel::MetaObjsSqlDisabled : DatabaseModel::MetaNoOpts);
		options |= (tag_objs_chk->isChecked() ? DatabaseModel::MetaTagObjs : DatabaseModel::MetaNoOpts);
		options |= (textbox_objs_chk->isChecked() ? DatabaseModel::MetaTextboxObjs : DatabaseModel::MetaNoOpts);
		options |= (objs_fadedout_chk->isChecked() ? DatabaseModel::MetaObjsFadeOut : DatabaseModel::MetaNoOpts);
		options |= (objs_collapse_mode_chk->isChecked() ? DatabaseModel::MetaObjsCollapseMode : DatabaseModel::MetaNoOpts);
		options |= (generic_sql_objs_chk->isChecked() ? DatabaseModel::MetaGenericSqlObjs : DatabaseModel::MetaNoOpts);
		options |= (objs_aliases_chk->isChecked() ? DatabaseModel::MetaObjsAliases : DatabaseModel::MetaNoOpts);
		options |= (objs_z_stack_value_chk->isChecked() ? DatabaseModel::MetaObjsZStackValue : DatabaseModel::MetaNoOpts);
		options |= (objs_layers_config_chk->isChecked() ? DatabaseModel::MetaObjsLayersConfig : DatabaseModel::MetaNoOpts);
		options |= (merge_dup_objs_chk->isChecked() ? DatabaseModel::MetaMergeDuplicatedObjs : DatabaseModel::MetaNoOpts);

		extract_model = extract_model_sel->isModelSelected() ?
										extract_model_sel->getSelectedModel()->getDatabaseModel() :	nullptr;

		apply_model = apply_model_sel->isModelSelected() ?
									apply_model_sel->getSelectedModel()->getDatabaseModel() : nullptr;

		if(extract_model)
			connect(extract_model, &DatabaseModel::s_objectLoaded, this, &MetadataHandlingWidget::updateProgress, Qt::UniqueConnection);

		if(apply_model)
			connect(apply_model, &DatabaseModel::s_objectLoaded, this, &MetadataHandlingWidget::updateProgress, Qt::UniqueConnection);

		if(op_type == OpExtractRestore || op_type == OpExtractOnly)
		{
			if(op_type == OpExtractOnly)
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

			root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																									 UtilsNs::formatMessage(tr("Extracting metadata to file `%1'").arg(metadata_file)),
																									 QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

			extract_model->saveObjectsMetadata(metadata_file, options);

			if(op_type == OpRestoreOnly && !bkp_filename.isEmpty())
			{
				root_item->setExpanded(false);
				root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																										 UtilsNs::formatMessage(tr("Saving backup metadata to file `%1'").arg(backup_file_sel->getSelectedFile())),
																										 QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);

				apply_model->saveObjectsMetadata(bkp_filename);
			}
		}
		else
		{
			metadata_file = bkp_filename;
		}

		if(root_item)
			root_item->setExpanded(false);

		if(op_type != OpExtractOnly)
		{
			ModelWidget *apply_model_wgt = apply_model_sel->getSelectedModel();

			root_item = GuiUtilsNs::createOutputTreeItem(output_trw,
																										UtilsNs::formatMessage(tr("Applying metadata from file `%1'").arg(metadata_file)),
																										QPixmap(GuiUtilsNs::getIconPath("info")), nullptr);


			apply_model->loadObjectsMetadata(metadata_file, options);
			apply_model_wgt->adjustSceneRect(false);
			apply_model_wgt->updateSceneLayers();
			apply_model_wgt->restoreLastCanvasPosition();
			apply_model_wgt->setModified(true);
			apply_model_wgt->updateObjectsOpacity();
		}
	}
	catch(Exception &e)
	{
		QPixmap icon = QPixmap(GuiUtilsNs::getIconPath("error"));

		GuiUtilsNs::createOutputTreeItem(output_trw,
																		 UtilsNs::formatMessage(e.getErrorMessage()),
																		 icon, nullptr);

		ico_lbl->setPixmap(icon);
		progress_lbl->setText(tr("Metadata processing aborted!"));

		Messagebox::error(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

	if(extract_model)
		disconnect(extract_model, nullptr, this, nullptr);

	if(apply_model)
		disconnect(apply_model, nullptr, this, nullptr);
}

void MetadataHandlingWidget::configureSelector()
{
	MetaOpType op_type = static_cast<MetaOpType>(operation_cmb->currentIndex());

	if(op_type == OpExtractRestore || op_type == OpExtractOnly)
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

void MetadataHandlingWidget::updateProgress(int progress, QString msg, unsigned int type_id)
{
	ObjectType obj_type = static_cast<ObjectType>(type_id);
	QString fmt_msg = UtilsNs::formatMessage(msg);
	QPixmap icon;

	if(obj_type == ObjectType::BaseObject)
	{
		if(progress == 100)
			icon = QPixmap(GuiUtilsNs::getIconPath("info"));
		else
			icon = QPixmap(GuiUtilsNs::getIconPath("alert"));
	}
	else
		icon = QPixmap(GuiUtilsNs::getIconPath(obj_type));

	GuiUtilsNs::createOutputTreeItem(output_trw, fmt_msg, icon, root_item);
	progress_lbl->setText(fmt_msg);
	ico_lbl->setPixmap(icon);
	progress_pb->setValue(progress);
}
