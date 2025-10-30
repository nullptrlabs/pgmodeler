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

#include "generalconfigwidget.h"
#include "objectsscene.h"
#include "relationshipview.h"
#include "widgets/modelwidget.h"
#include "operationlist.h"
#include "mainwindow.h"
#include "widgets/numberedtexteditor.h"
#include "tools/sqlexecutionwidget.h"
#include "tools/difftoolwidget.h"
#include "tools/databaseimportwidget.h"
#include "tools/modelexportwidget.h"

std::map<QString, GeneralConfigWidget::WidgetState> GeneralConfigWidget::widgets_geom;
std::map<QString, attribs_map> GeneralConfigWidget::config_params;

GeneralConfigWidget::GeneralConfigWidget(QWidget * parent) : BaseConfigWidget(parent)
{
	std::vector<QPageSize::PageSizeId> page_ids={ QPageSize::A0, QPageSize::A1, QPageSize::A2, QPageSize::A3, QPageSize::A4, QPageSize::A5,
																			 QPageSize::A6, QPageSize::A7, QPageSize::A8, QPageSize::A9, QPageSize::B0, QPageSize::B1,
																			 QPageSize::B10, QPageSize::B2, QPageSize::B3, QPageSize::B4, QPageSize::B5, QPageSize::B6,
																			 QPageSize::B7, QPageSize::B8, QPageSize::B9, QPageSize::C5E, QPageSize::Comm10E, QPageSize::DLE,
																			 QPageSize::Executive, QPageSize::Folio, QPageSize::Ledger, QPageSize::Legal, QPageSize::Letter,
																			 QPageSize::Tabloid, QPageSize::Custom };
	Ui_GeneralConfigWidget::setupUi(this);

	confs_dir_sel = new FileSelectorWidget(this);
	confs_dir_sel->setToolTip(tr("pgModeler configurations directory for the current user"));
	confs_dir_sel->setReadOnly(true);
	confs_dir_sel->setDirectoryMode(true);
	confs_dir_sel->setSelectedFile(GlobalAttributes::getConfigurationsPath());
	general_grid->addWidget(confs_dir_sel, 1, 1, 1, 1);

	source_editor_sel = new FileSelectorWidget(this);
	source_editor_sel->setAllowFilenameInput(true);
	source_editor_sel->setFileMustExist(true);
	source_editor_sel->setAcceptMode(QFileDialog::AcceptOpen);
	source_editor_sel->setWindowTitle(tr("Select application"));
	source_editor_sel->setToolTip(tr("External source code editor application"));
	source_editor_sel->setCheckExecutionFlag(true);
	general_grid->addWidget(source_editor_sel, 2, 1, 1, 1);

	int i = 0;
	for(auto &pg_id : page_ids)
		paper_cmb->setItemData(i++, QVariant(pg_id));

	check_versions_cmb->setItemData(0, Attributes::AllVersions);
	check_versions_cmb->setItemData(1, Attributes::StableBeta);
	check_versions_cmb->setItemData(2, Attributes::StableOnly);

	connect(check_update_chk, &QCheckBox::toggled, check_versions_cmb, &QComboBox::setEnabled);
	connect(unity_cmb, &QComboBox::currentIndexChanged, this, &GeneralConfigWidget::convertMarginUnity);
	connect(autosave_interv_chk, &QCheckBox::toggled, autosave_interv_spb, &QSpinBox::setEnabled);
	connect(paper_cmb, &QComboBox::currentIndexChanged, this,  &GeneralConfigWidget::selectPaperSize);
	connect(save_restore_geometry_chk, &QCheckBox::toggled, reset_sizes_btn, &QPushButton::setEnabled);
	connect(reset_sizes_btn, &QPushButton::clicked, this, &GeneralConfigWidget::resetDialogsSizes);

	connect(trunc_columns_data_chk, &QCheckBox::toggled, trunc_columns_data_spb, &QComboBox::setEnabled);
	connect(trunc_columns_data_chk, &QCheckBox::toggled, disable_inline_editor_chk, &QComboBox::setEnabled);

	config_params[Attributes::Configuration][Attributes::GridSize]="";
	config_params[Attributes::Configuration][Attributes::OpListSize]="";
	config_params[Attributes::Configuration][Attributes::AutoSaveInterval]="";
	config_params[Attributes::Configuration][Attributes::PaperType]="";
	config_params[Attributes::Configuration][Attributes::PaperOrientation]="";
	config_params[Attributes::Configuration][Attributes::PaperMargin]="";
	config_params[Attributes::Configuration][Attributes::PaperCustomSize]="";
	config_params[Attributes::Configuration][Attributes::File]="";
	config_params[Attributes::Configuration][Attributes::RecentModels]="";
	config_params[Attributes::Configuration][Attributes::PrintPgNum]="";
	config_params[Attributes::Configuration][Attributes::PrintGrid]="";
	config_params[Attributes::Configuration][Attributes::HideRelName]="";
	config_params[Attributes::Configuration][Attributes::HideExtAttribs]="";
	config_params[Attributes::Configuration][Attributes::HideTableTags]="";
	config_params[Attributes::Configuration][Attributes::FileAssociated]="";
	config_params[Attributes::Configuration][Attributes::CanvasCornerMove]="";
	config_params[Attributes::Configuration][Attributes::InvertRangeSelTrigger]="";
	config_params[Attributes::Configuration][Attributes::CheckUpdate]="";
	config_params[Attributes::Configuration][Attributes::CheckVersions]="";
	config_params[Attributes::Configuration][Attributes::SaveLastPosition]="";
	config_params[Attributes::Configuration][Attributes::ShowMainMenu]="";
	config_params[Attributes::Configuration][Attributes::DisableSmoothness]="";
	config_params[Attributes::Configuration][Attributes::SimplifiedObjCreation]="";
	config_params[Attributes::Configuration][Attributes::ConfirmValidation]="";
	config_params[Attributes::Configuration][Attributes::ShowMainMenu]="";
	config_params[Attributes::Configuration][Attributes::CodeCompletion]="";
	config_params[Attributes::Configuration][Attributes::UsePlaceholders]="";
	config_params[Attributes::Configuration][Attributes::HistoryMaxLength]="";
	config_params[Attributes::Configuration][Attributes::SourceEditorApp]="";
	config_params[Attributes::Configuration][Attributes::UiLanguage]="";
	config_params[Attributes::Configuration][Attributes::UseCurvedLines]="";
	config_params[Attributes::Configuration][Attributes::SaveRestoreGeometry]="";
	config_params[Attributes::Configuration][Attributes::LowVerbosity]="";
	config_params[Attributes::Configuration][Attributes::DefaultSchema]="";
	config_params[Attributes::Configuration][Attributes::HideEmptyObjGroups]="";

	selectPaperSize();

#ifdef NO_UPDATE_CHECK
	check_update_chk->setChecked(false);
	check_update_chk->setVisible(false);
	check_versions_cmb->setVisible(false);
#endif

	//Retrieving the available UI dictionaries
	QStringList langs = QDir(GlobalAttributes::getLanguagesPath() +
													 GlobalAttributes::DirSeparator,
													 "*.qm", QDir::Name, QDir::AllEntries | QDir::NoDotAndDotDot).entryList();

	langs.replaceInStrings(".qm", "");
	ui_language_cmb->addItem(tr("System default"));
	QString native_lang;

	for(auto &lang : langs)
	{
		native_lang = QLocale(lang).nativeLanguageName();
		native_lang[0] = native_lang[0].toUpper();
		ui_language_cmb->addItem(QString("%1 (%2 : %3)")
														 .arg(native_lang)
														 .arg(QLocale::languageToString(QLocale(lang).language()))
														 .arg(lang), lang);
	}

	QList<QCheckBox *> chk_boxes=this->findChildren<QCheckBox *>();
	QList<QSpinBox *> spin_boxes=this->findChildren<QSpinBox *>();
	QList<QDoubleSpinBox *> dspin_boxes=this->findChildren<QDoubleSpinBox *>();
	QList<QComboBox *> combos=this->findChildren<QComboBox *>();
	QList<QRadioButton *> radios=this->findChildren<QRadioButton *>();

	for(auto &chk : chk_boxes)
	{
		child_wgts.push_back(chk);
		connect(chk, &QCheckBox::toggled, this, &GeneralConfigWidget::setConfigurationChanged);
	}

	for(auto &spin : spin_boxes)
	{
		child_wgts.push_back(spin);
		connect(spin, &QSpinBox::valueChanged, this, &GeneralConfigWidget::setConfigurationChanged);
	}

	for(auto &dspin : dspin_boxes)
	{
		child_wgts.push_back(dspin);
		connect(dspin,  &QDoubleSpinBox::valueChanged, this, &GeneralConfigWidget::setConfigurationChanged);
	}

	for(auto &cmb : combos)
	{
		child_wgts.push_back(cmb);
		connect(cmb, &QComboBox::currentIndexChanged, this, &GeneralConfigWidget::setConfigurationChanged);
	}

	for(auto &radio : radios)
	{
		child_wgts.push_back(radio);
		connect(radio, &QRadioButton::toggled, this, &GeneralConfigWidget::setConfigurationChanged);
	}

	connect(source_editor_sel, &FileSelectorWidget::s_selectorChanged, this, &GeneralConfigWidget::setConfigurationChanged);

	connect(clear_sql_history_btn, &QPushButton::clicked, this, [](){
		SQLExecutionWidget::destroySQLHistory();
	});

	connect(reset_alerts_choices_btn, &QPushButton::clicked, this, &GeneralConfigWidget::resetAlertChoices);
}

void GeneralConfigWidget::showEvent(QShowEvent *)
{
	reset_alerts_choices_btn->setEnabled(config_params[Attributes::Configuration][Attributes::AlertUnsavedModels] != Attributes::True ||
																			 config_params[Attributes::Configuration][Attributes::AlertOpenSqlTabs] != Attributes::True ||
																			 config_params[Attributes::Configuration][Attributes::UseDefDisambiguation] == Attributes::True ||
																			 config_params[Attributes::Configuration][Attributes::AlertApplyMetadata] == Attributes::False);

}

void GeneralConfigWidget::resetAlertChoices()
{
	config_params[Attributes::Configuration][Attributes::AlertUnsavedModels] = Attributes::True;
	config_params[Attributes::Configuration][Attributes::AlertOpenSqlTabs] = Attributes::True;
	config_params[Attributes::Configuration][Attributes::UseDefDisambiguation] = Attributes::False;
	config_params[Attributes::Configuration][Attributes::AlertApplyMetadata] = Attributes::True;
	reset_alerts_choices_btn->setEnabled(false);
}

void GeneralConfigWidget::loadConfiguration()
{
	try
	{
		QStringList margin, custom_size;
		unsigned interv=0;
		int x=0, y=0, w=0, h=0, idx = -1;

		for(auto &wgt : child_wgts)
			wgt->blockSignals(true);

		BaseConfigWidget::loadConfiguration(GlobalAttributes::GeneralConf, config_params, { Attributes::Id });

		if(!config_params[Attributes::Configuration].count(Attributes::AlertUnsavedModels) ||
			 !config_params[Attributes::Configuration].count(Attributes::AlertOpenSqlTabs) ||
			 !config_params[Attributes::Configuration].count(Attributes::UseDefDisambiguation) ||
			 !config_params[Attributes::Configuration].count(Attributes::AlertApplyMetadata))
			resetAlertChoices();

		oplist_size_spb->setValue((config_params[Attributes::Configuration][Attributes::OpListSize]).toUInt());
		history_max_length_spb->setValue(config_params[Attributes::Configuration][Attributes::HistoryMaxLength].toUInt());

		interv=(config_params[Attributes::Configuration][Attributes::AutoSaveInterval]).toUInt();

		autosave_interv_chk->setChecked(interv > 0);
		autosave_interv_spb->setValue(interv);
		autosave_interv_spb->setEnabled(autosave_interv_chk->isChecked());

		corner_move_chk->setChecked(config_params[Attributes::Configuration][Attributes::CanvasCornerMove]==Attributes::True);
		invert_rangesel_chk->setChecked(config_params[Attributes::Configuration][Attributes::InvertRangeSelTrigger]==Attributes::True);

		check_update_chk->setChecked(config_params[Attributes::Configuration][Attributes::CheckUpdate]==Attributes::True);
		idx = check_versions_cmb->findData(config_params[Attributes::Configuration][Attributes::CheckVersions]);
		check_versions_cmb->setCurrentIndex(idx < 0 ? 0 : idx);
		check_versions_cmb->setEnabled(check_update_chk->isChecked());	
		old_pgsql_versions_chk->setChecked(config_params[Attributes::Configuration][Attributes::OldPgSqlVersions]==Attributes::True);

		save_last_pos_chk->setChecked(config_params[Attributes::Configuration][Attributes::SaveLastPosition]==Attributes::True);
		disable_smooth_chk->setChecked(config_params[Attributes::Configuration][Attributes::DisableSmoothness]==Attributes::True);
		simple_obj_creation_chk->setChecked(config_params[Attributes::Configuration][Attributes::SimplifiedObjCreation]==Attributes::True);
		confirm_validation_chk->setChecked(config_params[Attributes::Configuration][Attributes::ConfirmValidation]==Attributes::True);
		code_completion_chk->setChecked(config_params[Attributes::Configuration][Attributes::CodeCompletion]==Attributes::True);
		use_placeholders_chk->setChecked(config_params[Attributes::Configuration][Attributes::UsePlaceholders]==Attributes::True);
		use_curved_lines_chk->setChecked(config_params[Attributes::Configuration][Attributes::UseCurvedLines]==Attributes::True);

		print_grid_chk->setChecked(config_params[Attributes::Configuration][Attributes::PrintGrid]==Attributes::True);
		print_pg_num_chk->setChecked(config_params[Attributes::Configuration][Attributes::PrintPgNum]==Attributes::True);

		paper_cmb->setCurrentIndex((config_params[Attributes::Configuration][Attributes::PaperType]).toUInt());
		orientation_cmb->setCurrentIndex(config_params[Attributes::Configuration][Attributes::PaperOrientation] == Attributes::Landscape ? 0 : 1);

		margin=config_params[Attributes::Configuration][Attributes::PaperMargin].split(',');
		custom_size=config_params[Attributes::Configuration][Attributes::PaperCustomSize].split(',');

		left_marg->setValue((margin.count() >= 4 ? margin[0].toDouble() : 2));
		top_marg->setValue((margin.count()>= 4 ? margin[1].toDouble() : 2));
		right_marg->setValue((margin.count() >= 4 ? margin[2].toDouble() : 2));
		bottom_marg->setValue((margin.count() >= 4 ? margin[3].toDouble() : 2));

		width_spb->setValue((custom_size.count() >= 2 ? custom_size[0].toDouble() : 500));
		height_spb->setValue((custom_size.count() >= 2 ? custom_size[1].toDouble() : 500));

		hide_ext_attribs_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideExtAttribs]==Attributes::True);
		hide_rel_name_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideRelName]==Attributes::True);
		hide_table_tags_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideTableTags]==Attributes::True);
		hide_sch_name_usr_types_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideSchNameUserTypes]==Attributes::True);
		hide_obj_shadows_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideObjShadows]==Attributes::True);

		source_editor_sel->setSelectedFile(config_params[Attributes::Configuration][Attributes::SourceEditorApp]);
		source_editor_args_edt->setText(config_params[Attributes::Configuration][Attributes::SourceEditorArgs]);

		save_restore_geometry_chk->setChecked(config_params[Attributes::Configuration][Attributes::SaveRestoreGeometry]==Attributes::True);
		reset_sizes_btn->setEnabled(save_restore_geometry_chk->isChecked());
		low_verbosity_chk->setChecked(config_params[Attributes::Configuration][Attributes::LowVerbosity]==Attributes::True);
		escape_comments_chk->setChecked(config_params[Attributes::Configuration][Attributes::EscapeComment]==Attributes::True);

		trunc_columns_data_chk->setChecked(config_params[Attributes::Configuration][Attributes::TruncateColumnData]==Attributes::True);
		trunc_columns_data_spb->setValue(config_params[Attributes::Configuration][Attributes::ColumnTruncThreshold].toInt());

		trunc_columns_data_spb->setEnabled(trunc_columns_data_chk->isChecked());
		disable_inline_editor_chk->setChecked(config_params[Attributes::Configuration][Attributes::DisableInlineEditor]==Attributes::True);
		disable_inline_editor_chk->setEnabled(disable_inline_editor_chk->isChecked());

		hide_obj_sel_info_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideObjectsSelInfo]==Attributes::True);
		hide_cur_pos_zoom_info_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideCurPosZoomInfo]==Attributes::True);

		hide_empty_obj_grps_chk->setChecked(config_params[Attributes::Configuration][Attributes::HideEmptyObjGroups]==Attributes::True);

		int ui_idx = ui_language_cmb->findData(config_params[Attributes::Configuration][Attributes::UiLanguage]);
		ui_language_cmb->setCurrentIndex(ui_idx >= 0 ? ui_idx : 0);

		for(auto &wgt : child_wgts)
			wgt->blockSignals(false);

		widgets_geom.clear();
		for(auto &itr : config_params)
		{
			if(itr.second.count(Attributes::XPos))
			{
				x = itr.second[Attributes::XPos].toInt();
				y = itr.second[Attributes::YPos].toInt();
				w = itr.second[Attributes::Width].toInt();
				h = itr.second[Attributes::Height].toInt();
				widgets_geom[itr.first].geometry = QRect(QPoint(x,y), QSize(w, h));
				widgets_geom[itr.first].maximized = itr.second[Attributes::Maximized] == Attributes::True;
			}
		}

		applyConfiguration();
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void GeneralConfigWidget::setConfigurationSection(const QString &section_id, const attribs_map &params)
{
	BaseConfigWidget::setConfigurationSection(config_params, section_id, params);
}

void GeneralConfigWidget::removeConfigurationSection(const QRegularExpression &section_regex)
{
	std::map<QString, attribs_map>::iterator itr, itr_end;

	itr=config_params.begin();
	itr_end=config_params.end();

	while(itr!=itr_end)
	{
		if(section_regex.match(itr->first).hasMatch())
		{
			config_params.erase(itr);
			itr=config_params.begin();
			itr_end=config_params.end();
		}

		itr++;
	}
}

std::map<QString, attribs_map> GeneralConfigWidget::getConfigurationParams()
{
	return config_params;
}

void GeneralConfigWidget::appendConfigurationSection(const QString &section_id, const attribs_map &params)
{
	BaseConfigWidget::appendConfigurationSection(config_params, section_id, params);
}

QString GeneralConfigWidget::getConfigurationParam(const QString &section_id, const QString &param_name)
{
	if(config_params.count(section_id) &&	config_params[section_id].count(param_name))
		return config_params[section_id][param_name];

	return "";
}

void GeneralConfigWidget::saveWidgetGeometry(QWidget *widget, const QString &custom_wgt_name)
{
	if(!widget ||
		 config_params[Attributes::Configuration][Attributes::SaveRestoreGeometry] != Attributes::True)
		return;

	QString dlg_name = custom_wgt_name.isEmpty() ? widget->metaObject()->className() : custom_wgt_name;

	widgets_geom[dlg_name.toLower()].geometry = widget->geometry();
	widgets_geom[dlg_name.toLower()].maximized = widget->isMaximized();
}

bool GeneralConfigWidget::restoreWidgetGeometry(QWidget *widget, const QString &custom_wgt_name)
{
	if(!widget ||
		 config_params[Attributes::Configuration][Attributes::SaveRestoreGeometry] != Attributes::True)
		return false;

	QString dlg_name = custom_wgt_name.isEmpty() ? widget->metaObject()->className() : custom_wgt_name;
	dlg_name = dlg_name.toLower();

	if(widgets_geom.count(dlg_name) &&
		 (widgets_geom[dlg_name].maximized ||
			(widgets_geom[dlg_name].geometry.width() > 0 && widgets_geom[dlg_name].geometry.height() > 0)))
	{
		QList<QScreen *> screens = qApp->screens();
		WidgetState wgt_st = widgets_geom[dlg_name];
		bool scr_contains_geom = false;
		QRect scr_geom;

		// Validating the widget geometry against the available screens sizes
		for(auto &scr : screens)
		{
			scr_geom = scr->geometry();
			scr_contains_geom = ((wgt_st.maximized && scr_geom.contains(wgt_st.geometry.topLeft())) ||
													 (scr_geom.contains(wgt_st.geometry)));
			if(scr_contains_geom) break;
		}

		/* If the current window geometry doesn't fit the screen(s) geometry
		 * the default geometry of the window is used */
		if(!scr_contains_geom)
			return false;

		if(wgt_st.maximized)
		{
			widget->move(wgt_st.geometry.topLeft());
			widget->setWindowState(Qt::WindowMaximized);
		}
		else
			widget->setGeometry(wgt_st.geometry);

		return true;
	}

	return false;
}

void GeneralConfigWidget::saveConfiguration()
{
	try
	{
		attribs_map attribs;
		std::map<QString, attribs_map >::iterator itr, itr_end;
		QString file_sch, widget_sch;
		int recent_mdl_idx = 0;

		file_sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																														 Attributes::File +
																														 GlobalAttributes::SchemaExt);

		widget_sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																															Attributes::Widget +
																															GlobalAttributes::SchemaExt);

		config_params[Attributes::Configuration][Attributes::PgModelerVersion]=GlobalAttributes::PgModelerVersion;
		config_params[Attributes::Configuration][Attributes::FirstRun]=Attributes::False;

		config_params[Attributes::Configuration][Attributes::OpListSize]=QString::number(oplist_size_spb->value());
		config_params[Attributes::Configuration][Attributes::AutoSaveInterval]=QString::number(autosave_interv_chk->isChecked() ? autosave_interv_spb->value() : 0);
		config_params[Attributes::Configuration][Attributes::PaperType]=QString::number(paper_cmb->currentIndex());
		config_params[Attributes::Configuration][Attributes::PaperOrientation]=(orientation_cmb->currentIndex()  == 0 ? Attributes::Landscape : Attributes::Portrait);

		config_params[Attributes::Configuration][Attributes::CanvasCornerMove]=(corner_move_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::InvertRangeSelTrigger]=(invert_rangesel_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::CheckUpdate]=(check_update_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::CheckVersions]=check_versions_cmb->currentData().toString();
		config_params[Attributes::Configuration][Attributes::SaveLastPosition]=(save_last_pos_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::DisableSmoothness]=(disable_smooth_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::SimplifiedObjCreation]=(simple_obj_creation_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::ConfirmValidation]=(confirm_validation_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::CodeCompletion]=(code_completion_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::UsePlaceholders]=(use_placeholders_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HistoryMaxLength]=QString::number(history_max_length_spb->value());
		config_params[Attributes::Configuration][Attributes::UseCurvedLines]=(use_curved_lines_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::ShowCanvasGrid]=(ObjectsScene::isShowGrid() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::ShowPageDelimiters]=(ObjectsScene::isShowPageDelimiters() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::AlignObjsToGrid]=(ObjectsScene::isAlignObjectsToGrid() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::LockPageDelimResize]=(ObjectsScene::isDelimiterScaleLocked() ? Attributes::True : "");

		unity_cmb->setCurrentIndex(UnitMilimeters);
		config_params[Attributes::Configuration][Attributes::PaperMargin]=QString("%1,%2,%3,%4").arg(left_marg->value())
																						 .arg(top_marg->value())
																						 .arg(right_marg->value())
																						 .arg(bottom_marg->value());

		if(paper_cmb->currentIndex()!=paper_cmb->count()-1)
			config_params[Attributes::Configuration][Attributes::PaperCustomSize]="";
		else
			config_params[Attributes::Configuration][Attributes::PaperCustomSize]=QString("%1,%2").arg(width_spb->value()).arg(height_spb->value());

		config_params[Attributes::Configuration][Attributes::PrintPgNum]=(print_pg_num_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::PrintGrid]=(print_grid_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::HideExtAttribs]=(hide_ext_attribs_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HideRelName]=(hide_rel_name_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HideTableTags]=(hide_table_tags_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HideSchNameUserTypes]=(hide_sch_name_usr_types_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HideObjShadows]=(hide_obj_shadows_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::SourceEditorApp]=source_editor_sel->getSelectedFile();
		config_params[Attributes::Configuration][Attributes::SourceEditorArgs]=source_editor_args_edt->text();
		config_params[Attributes::Configuration][Attributes::UiLanguage]=ui_language_cmb->currentData().toString();

		config_params[Attributes::Configuration][Attributes::CompactView]=(BaseObjectView::isCompactViewEnabled() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::SaveRestoreGeometry]=(save_restore_geometry_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::LowVerbosity]=(low_verbosity_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::EscapeComment]=(escape_comments_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::OldPgSqlVersions]=(old_pgsql_versions_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::TruncateColumnData]=(trunc_columns_data_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::ColumnTruncThreshold]=QString::number(trunc_columns_data_spb->value());
		config_params[Attributes::Configuration][Attributes::DisableInlineEditor]=(trunc_columns_data_chk->isChecked() &&
																																							 disable_inline_editor_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::HideObjectsSelInfo]=(hide_obj_sel_info_chk->isChecked() ? Attributes::True : "");
		config_params[Attributes::Configuration][Attributes::HideCurPosZoomInfo]=(hide_cur_pos_zoom_info_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::HideEmptyObjGroups]=(hide_empty_obj_grps_chk->isChecked() ? Attributes::True : "");

		config_params[Attributes::Configuration][Attributes::File]="";
		config_params[Attributes::Configuration][Attributes::RecentModels]="";

		itr=config_params.begin();
		itr_end=config_params.end();

		config_params[Attributes::Configuration][Attributes::DockWidgets]="";
		config_params[Attributes::Configuration][Attributes::WidgetsGeometry]="";
		config_params[Attributes::Configuration][Attributes::RecentModels]="";
		config_params[Attributes::Configuration][Attributes::File]="";

		while(itr!=itr_end)
		{
			//Checking if the current attribute is a file to be stored in a <session> tag
			if((itr->first).contains(QRegularExpression("(" + Attributes::File + ")([0-9]+)")))
			{
				config_params[Attributes::Configuration][Attributes::File] +=	schparser.getSourceCode(file_sch, itr->second);
			}
			//Checking if the current attribute is a file to be stored in a <recent-models> tag
			else if(recent_mdl_idx < MaxRecentModels && (itr->first).contains(QRegularExpression("(" + Attributes::Recent + ")([0-9]+)")))
			{
				config_params[Attributes::Configuration][Attributes::RecentModels] +=	schparser.getSourceCode(file_sch, itr->second);
				recent_mdl_idx++;
			}
			else if(itr->first==Attributes::Validator ||
							itr->first==Attributes::ObjectFinder ||
							itr->first==Attributes::SqlTool ||
							itr->first==Attributes::LayersConfig)
			{
				schparser.ignoreUnkownAttributes(true);
				schparser.ignoreEmptyAttributes(true);
				config_params[Attributes::Configuration][Attributes::DockWidgets]+=
						schparser.getSourceCode(widget_sch, itr->second);
				schparser.ignoreUnkownAttributes(false);
				schparser.ignoreEmptyAttributes(false);
			}

			itr++;
		}

		if(save_restore_geometry_chk->isChecked())
		{
			for(auto &itr : widgets_geom)
			{
				/* Ignoring the validator, objectfinder and sqltool widget ids
				 * In order to avoid the saving of widget geometry of that objects */
				if(itr.first==Attributes::Validator ||
					 itr.first==Attributes::ObjectFinder ||
					 itr.first==Attributes::SqlTool)
					continue;

				attribs[Attributes::Id] = itr.first;
				attribs[Attributes::XPos] = QString::number(itr.second.geometry.left());
				attribs[Attributes::YPos] = QString::number(itr.second.geometry.top());
				attribs[Attributes::Width] = QString::number(itr.second.geometry.width());
				attribs[Attributes::Height] = QString::number(itr.second.geometry.height());
				attribs[Attributes::Maximized] = itr.second.maximized ? Attributes::True : "";

				schparser.ignoreUnkownAttributes(true);
				config_params[Attributes::Configuration][Attributes::WidgetsGeometry]+=
						schparser.getSourceCode(widget_sch, attribs);
				schparser.ignoreUnkownAttributes(false);
			}
		}

		BaseConfigWidget::saveConfiguration(GlobalAttributes::GeneralConf, config_params);
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void GeneralConfigWidget::applyConfiguration()
{
	int unit = unity_cmb->currentIndex();

	if(!save_restore_geometry_chk->isChecked())
	  widgets_geom.clear();

	BaseObject::setEscapeComments(escape_comments_chk->isChecked());

	QPageLayout page_lt;
	QPageSize::PageSizeId size_id = static_cast<QPageSize::PageSizeId>(paper_cmb->itemData(paper_cmb->currentIndex()).toInt());
	QPageSize page_sz(size_id);

	// Forcing the Point unity to configure the margins on the page layout
	unity_cmb->setCurrentIndex(UnitPoint);

	if(size_id == QPageSize::Custom)
		page_sz = QPageSize(QSizeF(width_spb->value(), height_spb->value()), QPageSize::Point);

	page_lt.setPageSize(page_sz);
	page_lt.setOrientation(orientation_cmb->currentIndex() == 0 ? QPageLayout::Landscape : QPageLayout::Portrait);
	page_lt.setMargins(QMarginsF(left_marg->value(), top_marg->value(), right_marg->value(), bottom_marg->value()));
	ObjectsScene::setPageLayout(page_lt);

	unity_cmb->setCurrentIndex(unit);

	ObjectsScene::setEnableCornerMove(corner_move_chk->isChecked());
	ObjectsScene::setInvertRangeSelectionTrigger(invert_rangesel_chk->isChecked());
	ObjectsScene::setShowGrid(config_params[Attributes::Configuration][Attributes::ShowCanvasGrid]==Attributes::True);
	ObjectsScene::setAlignObjectsToGrid(config_params[Attributes::Configuration][Attributes::AlignObjsToGrid]==Attributes::True);
	ObjectsScene::setShowPageDelimiters(config_params[Attributes::Configuration][Attributes::ShowPageDelimiters]==Attributes::True);
	ObjectsScene::setLockDelimiterScale(config_params[Attributes::Configuration][Attributes::LockPageDelimResize]==Attributes::True, 1);

	OperationList::setMaximumSize(oplist_size_spb->value());
	BaseTableView::setHideExtAttributes(hide_ext_attribs_chk->isChecked());
	BaseTableView::setHideTags(hide_table_tags_chk->isChecked());

	NumberedTextEditor::setSourceEditorApp(source_editor_sel->getSelectedFile());
	NumberedTextEditor::setSourceEditorAppArgs(source_editor_args_edt->text());

	RelationshipView::setHideNameLabel(hide_rel_name_chk->isChecked());
	RelationshipView::setCurvedLines(use_curved_lines_chk->isChecked());
	TableObjectView::setSchemaNameUserType(hide_sch_name_usr_types_chk->isChecked());
	ModelWidget::setSaveLastCanvasPosition(save_last_pos_chk->isChecked());
	ModelWidget::setRenderSmoothnessDisabled(disable_smooth_chk->isChecked());
	ModelWidget::setSimplifiedObjectCreation(simple_obj_creation_chk->isChecked());
	MainWindow::setConfirmValidation(confirm_validation_chk->isChecked());

	BaseObjectView::setCompactViewEnabled(config_params[Attributes::Configuration][Attributes::CompactView]==Attributes::True);
	BaseObjectView::setPlaceholderEnabled(use_placeholders_chk->isChecked());
	BaseObjectView::setShadowHidden(hide_obj_shadows_chk->isChecked());

	SQLExecutionWidget::setSQLHistoryMaxLength(history_max_length_spb->value());
	DiffToolWidget::setLowVerbosity(low_verbosity_chk->isChecked());
	DatabaseImportWidget::setLowVerbosity(low_verbosity_chk->isChecked());
	ModelExportWidget::setLowVerbosity(low_verbosity_chk->isChecked());
	Connection::setIgnoreDbVersion(old_pgsql_versions_chk->isChecked());

	PlainTextItemDelegate::setMaxDisplayLength(trunc_columns_data_chk->isChecked() ? trunc_columns_data_spb->value() : 0);
	PlainTextItemDelegate::setTextEditorEnabled(trunc_columns_data_chk->isChecked() ? !disable_inline_editor_chk->isChecked() : false);

	setConfigurationChanged(false);
}

void GeneralConfigWidget::restoreDefaults()
{
	try
	{
		BaseConfigWidget::restoreDefaults(GlobalAttributes::GeneralConf, false);
		BaseConfigWidget::restoreDefaults(GlobalAttributes::XMLHighlightConf, true);
		BaseConfigWidget::restoreDefaults(GlobalAttributes::SQLHighlightConf, true);
		BaseConfigWidget::restoreDefaults(GlobalAttributes::SchHighlightConf, true);
		loadConfiguration();
		applyConfiguration();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void GeneralConfigWidget::convertMarginUnity()
{
	static int prev_unit=UnitMilimeters;
	double conv_factor[]={1.0, 2.83, 0.04, 0.1},
			left, right, top, bottom, width, height;

	left=left_marg->value() / conv_factor[prev_unit];
	right=right_marg->value() / conv_factor[prev_unit];
	bottom=bottom_marg->value() / conv_factor[prev_unit];
	top=top_marg->value() / conv_factor[prev_unit];
	width=width_spb->value() / conv_factor[prev_unit];
	height=height_spb->value() / conv_factor[prev_unit];

	left_marg->setValue(left * conv_factor[unity_cmb->currentIndex()]);
	right_marg->setValue(right * conv_factor[unity_cmb->currentIndex()]);
	bottom_marg->setValue(bottom * conv_factor[unity_cmb->currentIndex()]);
	top_marg->setValue(top * conv_factor[unity_cmb->currentIndex()]);
	width_spb->setValue(width * conv_factor[unity_cmb->currentIndex()]);
	height_spb->setValue(height * conv_factor[unity_cmb->currentIndex()]);

	prev_unit=unity_cmb->currentIndex();
}

void GeneralConfigWidget::selectPaperSize()
{
	bool visible=paper_cmb->currentIndex()==paper_cmb->count()-1;

	width_lbl->setVisible(visible);
	height_lbl->setVisible(visible);
	width_spb->setVisible(visible);
	height_spb->setVisible(visible);
}

void GeneralConfigWidget::setConfigurationChanged(bool changed)
{
	if(child_wgts.contains(sender()))
		BaseConfigWidget::setConfigurationChanged(true);
	else
		BaseConfigWidget::setConfigurationChanged(changed);
}

void GeneralConfigWidget::resetDialogsSizes()
{
	int res = Messagebox::confirm(tr("This action will reset all dialogs to their default size and positions on the screen! Do you really want to proceed?"));

	if(Messagebox::isAccepted(res))
		widgets_geom.clear();
}
