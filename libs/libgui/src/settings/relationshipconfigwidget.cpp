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

#include "relationshipconfigwidget.h"
#include "attributes.h"
#include "utils/syntaxhighlighter.h"
#include "relationshipview.h"
#include <QButtonGroup>

std::map<QString, attribs_map> RelationshipConfigWidget::config_params;

RelationshipConfigWidget::RelationshipConfigWidget(QWidget * parent) : BaseConfigWidget(parent)
{
	QStringList list, rel_types={ Attributes::Relationship11, Attributes::Relationship1n,
																Attributes::RelationshipNn, Attributes::RelationshipGen,
																Attributes::RelationshipDep, Attributes::RelationshipPart };
	BaseRelationship::RelType rel_types_id[]={
								BaseRelationship::Relationship11, BaseRelationship::Relationship1n,
								BaseRelationship::RelationshipNn, BaseRelationship::RelationshipGen,
								BaseRelationship::RelationshipDep, BaseRelationship::RelationshipPart };

	Ui_RelationshipConfigWidget::setupUi(this);

	SyntaxHighlighter *pattern_hl=nullptr;
	QList<QPlainTextEdit *> pattern_fields={ src_col_pattern_txt, dst_col_pattern_txt,
																					 src_fk_pattern_txt, dst_fk_pattern_txt,
																					 pk_pattern_txt, uq_pattern_txt, pk_col_pattern_txt,
																					 fk_idx_pattern_txt };

	for(auto &patt_field : pattern_fields)
	{
		pattern_hl = new SyntaxHighlighter(patt_field, true, false, font().pointSizeF());
		pattern_hl->loadConfiguration(GlobalAttributes::getPatternHighlightConfPath());

		connect(patt_field, &QPlainTextEdit::textChanged, this, &RelationshipConfigWidget::updatePattern);
	}

	deferral_cmb->addItems(DeferralType::getTypes());

	list = ActionType::getTypes();
	list.push_front(tr("Default"));
	del_action_cmb->addItems(list);
	upd_action_cmb->addItems(list);

	unsigned i = 0;

	for(auto &rel_type : rel_types)
		rel_type_cmb->addItem(BaseRelationship::getRelationshipTypeName(rel_types_id[i++]), rel_type);

	index_type_cmb->addItem(tr("None"));

	for(auto &type : IndexingType::getTypes())
		index_type_cmb->addItem(type, type);

	conn_mode_gb->setFocusProxy(crows_foot_tb);
	foreign_key_gb->setFocusProxy(deferrable_chk);

	QButtonGroup *conn_mode_btns_grp = new QButtonGroup(this);
	conn_mode_btns_grp->setExclusive(true);

	for(auto &btn : conn_mode_gb->findChildren<QToolButton *>())
		conn_mode_btns_grp->addButton(btn);

	connect(conn_mode_btns_grp, &QButtonGroup::buttonToggled, this, [this](){
			setConfigurationChanged(true);
	});

	connect(deferrable_chk, &QCheckBox::toggled, deferral_lbl, &QLabel::setEnabled);
	connect(deferrable_chk, &QCheckBox::toggled, deferral_cmb, &QComboBox::setEnabled);

	connect(deferrable_chk, &QCheckBox::toggled, this, [this]() {
		setConfigurationChanged(true);
	});

	connect(rel_type_cmb,  &QComboBox::currentIndexChanged, this,&RelationshipConfigWidget::fillNamePatterns);

	connect(del_action_cmb, &QComboBox::currentTextChanged, this, [this](){
		setConfigurationChanged(true);
	});

	connect(upd_action_cmb, &QComboBox::currentTextChanged, this, [this](){
		setConfigurationChanged(true);
	});

	connect(deferral_cmb, &QComboBox::currentTextChanged, this, [this](){
		setConfigurationChanged(true);
	});

	connect(index_type_cmb, &QComboBox::currentTextChanged, this, [this](){
		setConfigurationChanged(true);
	});
}

std::map<QString, attribs_map> RelationshipConfigWidget::getConfigurationParams()
{
	return config_params;
}

void RelationshipConfigWidget::loadConfiguration()
{
	try
	{
		int idx = -1;
		BaseConfigWidget::loadConfiguration(GlobalAttributes::RelationshipsConf, config_params, { Attributes::Type });

		fk_to_pk_tb->setChecked(config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode]==Attributes::ConnectFkToPk);
		center_pnts_tb->setChecked(config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode]==Attributes::ConnectCenterPnts);
		tab_edges_tb->setChecked(config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode]==Attributes::ConnectTableEdges);
		crows_foot_tb->setChecked(config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode]==Attributes::CrowsFoot);

		deferrable_chk->setChecked(config_params[Attributes::ForeignKeys][Attributes::Deferrable]==Attributes::True);
		deferral_cmb->setCurrentText(config_params[Attributes::ForeignKeys][Attributes::DeferType]);

		idx = upd_action_cmb->findText(config_params[Attributes::ForeignKeys][Attributes::UpdAction]);
		upd_action_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

		idx = del_action_cmb->findText(config_params[Attributes::ForeignKeys][Attributes::DelAction]);
		del_action_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

		idx = index_type_cmb->findText(config_params[Attributes::ForeignKeys][Attributes::FkIdxType]);
		index_type_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

		patterns[Attributes::Relationship11]=config_params[Attributes::Relationship11];
		patterns[Attributes::Relationship1n]=config_params[Attributes::Relationship1n];
		patterns[Attributes::RelationshipNn]=config_params[Attributes::RelationshipNn];
		patterns[Attributes::RelationshipGen]=config_params[Attributes::RelationshipGen];
		patterns[Attributes::RelationshipDep]=config_params[Attributes::RelationshipDep];
		patterns[Attributes::RelationshipPart]=config_params[Attributes::RelationshipPart];

		fillNamePatterns();
		applyConfiguration();
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void RelationshipConfigWidget::saveConfiguration()
{  
	try
	{
		QString patterns_sch;

		patterns_sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																	Attributes::Patterns +
																																	GlobalAttributes::SchemaExt);
		if(crows_foot_tb->isChecked())
			config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode] = Attributes::CrowsFoot;
		else if(fk_to_pk_tb->isChecked())
			config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode] = Attributes::ConnectFkToPk;
		else if(tab_edges_tb->isChecked())
			config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode] = Attributes::ConnectTableEdges;
		else
			config_params[GlobalAttributes::RelationshipsConf][Attributes::LinkMode] = Attributes::ConnectCenterPnts;

		config_params[Attributes::ForeignKeys][Attributes::Deferrable] = (deferrable_chk->isChecked() ? Attributes::True : Attributes::False);
		config_params[Attributes::ForeignKeys][Attributes::DeferType] = deferral_cmb->currentText();
		config_params[Attributes::ForeignKeys][Attributes::UpdAction] = (upd_action_cmb->currentIndex() > 0 ? upd_action_cmb->currentText() : "");
		config_params[Attributes::ForeignKeys][Attributes::DelAction] = (del_action_cmb->currentIndex() > 0 ? del_action_cmb->currentText() : "");
		config_params[Attributes::ForeignKeys][Attributes::FkIdxType] = (index_type_cmb->currentIndex() > 0 ? index_type_cmb->currentText() : "");

		config_params[Attributes::NamePatterns][Attributes::Patterns] = "";

		for(auto &itr : patterns)
		{
			schparser.ignoreUnkownAttributes(true);
			schparser.ignoreEmptyAttributes(true);
			config_params[itr.first] = itr.second;
			config_params[Attributes::NamePatterns][Attributes::Patterns] += schparser.getSourceCode(patterns_sch, itr.second);
		}

		BaseConfigWidget::saveConfiguration(GlobalAttributes::RelationshipsConf, config_params);
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipConfigWidget::applyConfiguration()
{
	RelationshipView::setCrowsFoot(crows_foot_tb->isChecked());

	if(!crows_foot_tb->isChecked())
	{
		if(fk_to_pk_tb->isChecked())
			RelationshipView::setLineConnectionMode(RelationshipView::ConnectFkToPk);
		else if(tab_edges_tb->isChecked())
			RelationshipView::setLineConnectionMode(RelationshipView::ConnectTableEdges);
		else
			RelationshipView::setLineConnectionMode(RelationshipView::ConnectCenterPoints);
	}

	setConfigurationChanged(false);
}

void RelationshipConfigWidget::restoreDefaults()
{
	try
	{
		BaseConfigWidget::restoreDefaults(GlobalAttributes::RelationshipsConf, false);
		loadConfiguration();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipConfigWidget::fillNamePatterns()
{
	QString rel_type=rel_type_cmb->currentData().toString();
	bool relnn=false, reldep=false, relgen=false;
	QList<QPlainTextEdit *> inputs={ pk_pattern_txt, uq_pattern_txt, src_col_pattern_txt,
																	 dst_col_pattern_txt, src_fk_pattern_txt, dst_fk_pattern_txt,
																	 pk_col_pattern_txt, fk_idx_pattern_txt };

	QList<QString> pattern_ids={ Attributes::PkPattern,  Attributes::UqPattern,
								 Attributes::SrcColPattern, Attributes::DstColPattern,
								 Attributes::SrcFkPattern, Attributes::DstFkPattern,
								 Attributes::PkColPattern, Attributes::FkIdxPattern };

	relnn=(rel_type==Attributes::RelationshipNn);
	reldep=(rel_type==Attributes::RelationshipDep || rel_type==Attributes::RelationshipPart);
	relgen=(rel_type==Attributes::RelationshipGen);

	dst_col_pattern_txt->setEnabled(relnn);
	dst_fk_pattern_txt->setEnabled(relnn);
	src_col_pattern_txt->setEnabled(!relgen && !reldep);
	src_fk_pattern_txt->setEnabled(!relgen && !reldep);
	uq_pattern_txt->setEnabled(!relgen && !reldep);
	pk_col_pattern_txt->setEnabled(relnn);
	fk_idx_pattern_txt->setEnabled(!relgen && !reldep);

	dst_col_pattern_lbl->setEnabled(relnn);
	dst_fk_pattern_lbl->setEnabled(relnn);
	src_col_pattern_lbl->setEnabled(!relgen && !reldep);
	src_fk_pattern_lbl->setEnabled(!relgen && !reldep);
	uq_pattern_lbl->setEnabled(!relgen && !reldep);
	pk_col_pattern_lbl->setEnabled(relnn);
	fk_idx_pattern_lbl->setEnabled(!relgen && !reldep);

	int i = 0;

	for(auto &input : inputs)
	{
		input->blockSignals(true);
		input->clear();

		if(input->isEnabled() && patterns[rel_type].count(pattern_ids[i]))
			input->setPlainText(patterns[rel_type][pattern_ids[i]]);

		input->blockSignals(false);
		i++;
	}
}

void RelationshipConfigWidget::updatePattern()
{
	QPlainTextEdit *input=qobject_cast<QPlainTextEdit *>(sender());
	QString rel_type=rel_type_cmb->currentData().toString();
	std::map<QPlainTextEdit *, QString> inputs_map={ { pk_pattern_txt, Attributes::PkPattern },
												{ uq_pattern_txt, Attributes::UqPattern },
												{ src_col_pattern_txt, Attributes::SrcColPattern },
												{ dst_col_pattern_txt, Attributes::DstColPattern },
												{ src_fk_pattern_txt, Attributes::SrcFkPattern   },
												{ dst_fk_pattern_txt, Attributes::DstFkPattern   },
												{ pk_col_pattern_txt, Attributes::PkColPattern   },
												{ fk_idx_pattern_txt, Attributes::FkIdxPattern   } };

	setConfigurationChanged(true);
	patterns[rel_type][inputs_map[input]] = input->toPlainText();
}
