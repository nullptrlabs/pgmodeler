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

#include "relationshipwidget.h"
#include "constraintwidget.h"
#include "columnwidget.h"
#include "tablewidget.h"
#include "indexwidget.h"
#include "baseform.h"
#include "settings/relationshipconfigwidget.h"
#include "settings/generalconfigwidget.h"
#include "coreutilsns.h"
#include "guiutilsns.h"

RelationshipWidget::RelationshipWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Relationship)
{
	Ui_RelationshipWidget::setupUi(this);

	QStringList list;
	QGridLayout *grid=nullptr;
	QVBoxLayout *vlayout=nullptr;
	QFrame *frame=nullptr;
	QWidgetList pattern_fields={ src_col_pattern_txt, dst_col_pattern_txt,
															 src_fk_pattern_txt, dst_fk_pattern_txt,
															 pk_pattern_txt, uq_pattern_txt,
															 pk_col_pattern_txt, fk_idx_pattern_txt };

	table1_hl=nullptr;
	table1_hl=new SyntaxHighlighter(ref_table_txt, true, false, font().pointSizeF());
	table1_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	table2_hl=nullptr;
	table2_hl=new SyntaxHighlighter(recv_table_txt, true, false, font().pointSizeF());
	table2_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	for(int i=0; i < pattern_fields.size(); i++)
	{
		patterns_hl[i]=new SyntaxHighlighter(qobject_cast<QPlainTextEdit *>(pattern_fields[i]), true, false, font().pointSizeF());
		patterns_hl[i]->loadConfiguration(GlobalAttributes::getPatternHighlightConfPath());
	}

	attributes_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
										 (CustomTableWidget::UpdateButton |
											CustomTableWidget::MoveButtons), true, this);

	constraints_tab=new CustomTableWidget(CustomTableWidget::AllButtons  ^
											(CustomTableWidget::UpdateButton |
											 CustomTableWidget::MoveButtons), true, this);

	advanced_objs_tab=new CustomTableWidget(CustomTableWidget::EditButton, true, this);

	attributes_tab->setColumnCount(2);
	attributes_tab->setHeaderLabel(tr("Attribute"), 0);
	attributes_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("column")),0);
	attributes_tab->setHeaderLabel(tr("Type"), 1);
	attributes_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);

	constraints_tab->setColumnCount(2);
	constraints_tab->setHeaderLabel(tr("Constraint"), 0);
	constraints_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("constraint")),0);
	constraints_tab->setHeaderLabel(tr("Type"), 1);
	constraints_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);

	advanced_objs_tab->setColumnCount(2);
	advanced_objs_tab->setHeaderLabel(tr("Name"), 0);
	advanced_objs_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("column")),0);
	advanced_objs_tab->setHeaderLabel(tr("Type"), 1);
	advanced_objs_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);

	connect(advanced_objs_tab, &CustomTableWidget::s_rowEdited, this, &RelationshipWidget::showAdvancedObject);

	grid=new QGridLayout;
	grid->addWidget(attributes_tab, 0,0,1,1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	rel_attribs_tbw->widget(AttributesTab)->setLayout(grid);

	grid=new QGridLayout;
	grid->addWidget(constraints_tab, 0,0,1,1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	rel_attribs_tbw->widget(ConstraintsTab)->setLayout(grid);

	grid=dynamic_cast<QGridLayout *>(rel_attribs_tbw->widget(SpecialPkTab)->layout());
	frame=generateInformationFrame(tr("Use the special primary key if you want to include a primary key containing generated columns to the receiver table. <strong>Important:</strong> if this is a new relationship there is a need to finish its creation and reopen this dialog to create the special primary key."));

	grid->addWidget(frame, 1, 0, 1, 1);
	frame->setParent(rel_attribs_tbw->widget(SpecialPkTab));

	grid=new QGridLayout;
	grid->setContentsMargins(GuiUtilsNs::LtMargins);

	grid->addWidget(advanced_objs_tab, 0, 0, 1, 1);

	frame=generateInformationFrame(tr("This advanced tab shows the objects (columns or table) auto created by the relationship's connection as well the foreign keys that represents the link between the participant tables."));
	grid->addWidget(frame, 1, 0, 1, 1);

	rel_attribs_tbw->widget(AdvancedTab)->setLayout(grid);

	color_picker=new ColorPickerWidget(1,this);
	color_picker->setEnabled(false);
	grid=dynamic_cast<QGridLayout *>(rel_attribs_tbw->widget(GeneralTab)->layout());
	grid->addWidget(color_picker, 0, 1);

	configureFormLayout(relationship_grid, ObjectType::Relationship);

	deferral_cmb->addItems(DeferralType::getTypes());

	frame=generateInformationFrame(tr("Available tokens to define name patterns:<br/>\
				<strong>%1</strong> = Reference (source) primary key column name. <em>(Ignored on constraint patterns)</em><br/> \
				<strong>%2</strong> = Reference (source) table name.<br/> \
				<strong>%3</strong> = Receiver (destination) table name.<br/> \
				<strong>%4</strong> = Generated table name. <em>(Only for n:n relationships)</em>")
				.arg(Relationship::SrcColToken)
				.arg(Relationship::SrcTabToken)
				.arg(Relationship::DstTabToken)
				.arg(Relationship::GenTabToken));
	vlayout=dynamic_cast<QVBoxLayout *>(name_patterns_grp->layout());
	vlayout->addWidget(frame);

	list = ActionType::getTypes();
	list.prepend(tr("Default"));
	del_action_cmb->addItems(list);
	upd_action_cmb->addItems(list);

	index_type_cmb->addItem(tr("None"));

	for(auto &type : IndexingType::getTypes())
		index_type_cmb->addItem(type, type);

	tabs={ nullptr, rel_attribs_tbw->widget(SettingsTab),
				 rel_attribs_tbw->widget(AttributesTab), rel_attribs_tbw->widget(ConstraintsTab),
				 rel_attribs_tbw->widget(SpecialPkTab), rel_attribs_tbw->widget(AdvancedTab) };

	tab_labels=QStringList{ "", rel_attribs_tbw->tabText(SettingsTab),
						 rel_attribs_tbw->tabText(AttributesTab), rel_attribs_tbw->tabText(ConstraintsTab),
						 rel_attribs_tbw->tabText(SpecialPkTab), rel_attribs_tbw->tabText(AdvancedTab)};

	part_bound_expr_txt=new NumberedTextEditor(this, true);
	part_bound_expr_hl=new SyntaxHighlighter(part_bound_expr_txt);
	part_bound_expr_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());
	dynamic_cast<QGridLayout *>(part_bound_expr_gb->layout())->addWidget(part_bound_expr_txt, 1, 0);

	connect(deferrable_chk, &QCheckBox::toggled, deferral_cmb, &QComboBox::setEnabled);
	connect(deferrable_chk, &QCheckBox::toggled, deferral_lbl, &QLabel::setEnabled);

	connect(identifier_chk, &QCheckBox::toggled, this, [this](){
		table1_mand_chk->setDisabled(identifier_chk->isChecked());
		table2_mand_chk->setEnabled(!identifier_chk->isChecked() &&
																this->object &&
																dynamic_cast<BaseRelationship *>(this->object)->getRelationshipType() != BaseRelationship::Relationship1n);
	});

	connect(attributes_tab, &CustomTableWidget::s_rowsRemoved, this, __slot(this, RelationshipWidget::removeObjects));
	connect(attributes_tab, &CustomTableWidget::s_rowAdded, this, __slot(this, RelationshipWidget::addObject));
	connect(attributes_tab, &CustomTableWidget::s_rowEdited, this, __slot_n(this, RelationshipWidget::editObject));
	connect(attributes_tab, &CustomTableWidget::s_rowRemoved, this, __slot_n(this, RelationshipWidget::removeObject));
	connect(attributes_tab, &CustomTableWidget::s_rowDuplicated, this, __slot_n(this, RelationshipWidget::duplicateObject));

	connect(constraints_tab, &CustomTableWidget::s_rowsRemoved, this, __slot(this, RelationshipWidget::removeObjects));
	connect(constraints_tab, &CustomTableWidget::s_rowAdded, this, __slot(this, RelationshipWidget::addObject));
	connect(constraints_tab, &CustomTableWidget::s_rowEdited, this, __slot_n(this, RelationshipWidget::editObject));
	connect(constraints_tab, &CustomTableWidget::s_rowRemoved, this, __slot_n(this, RelationshipWidget::removeObject));
	connect(constraints_tab, &CustomTableWidget::s_rowDuplicated, this, __slot_n(this, RelationshipWidget::duplicateObject));

	connect(defaults_rb, &QRadioButton::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(including_rb, &QRadioButton::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(excluding_rb, &QRadioButton::toggled, this, &RelationshipWidget::selectCopyOptions);

	connect(defaults_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(constraints_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(comments_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(indexes_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(storage_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);
	connect(all_chk, &QCheckBox::toggled, this, &RelationshipWidget::selectCopyOptions);

	connect(custom_color_chk, &QCheckBox::toggled, color_picker, &ColorPickerWidget::setEnabled);

	connect(fk_gconf_chk, &QCheckBox::toggled, this, &RelationshipWidget::useFKGlobalSettings);
	connect(patterns_gconf_chk, &QCheckBox::toggled, this, &RelationshipWidget::usePatternGlobalSettings);
	connect(gen_bound_expr_btn, &QPushButton::clicked, this, &RelationshipWidget::generateBoundingExpr);
	connect(default_part_chk, &QCheckBox::toggled, part_bound_expr_txt, &NumberedTextEditor::setDisabled);

	setMinimumSize(600, 380);
}

void RelationshipWidget::setAttributes(DatabaseModel *model, OperationList *op_list, PhysicalTable *src_tab, PhysicalTable *dst_tab, BaseRelationship::RelType rel_type)
{
	Relationship *rel=nullptr;

	try
	{
		rel=new Relationship(rel_type, src_tab, dst_tab);

		color_picker->generateRandomColors();
		rel->setCustomColor(color_picker->getColor(0));

		this->new_object=true;
		this->setAttributes(model, op_list, rel);

		op_list->startOperationChain();
		operation_count=op_list->getCurrentSize();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseRelationship *base_rel)
{
	if(!base_rel)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseRelationship::RelType rel_type;
	Relationship *aux_rel=nullptr;
	bool rel1n=false, relnn=false, relgen_dep=false, has_foreign_tab=false;
	BaseObjectWidget::setAttributes(model, op_list, base_rel);

	if(!this->new_object)
	{
		op_list->startOperationChain();
		operation_count=op_list->getCurrentSize();
	}

	rel_type=base_rel->getRelationshipType();
	rel_type_name_lbl->setText(base_rel->getRelationshipTypeName());
	rel_icon_lbl->setPixmap(GuiUtilsNs::getIconPath(base_rel->getRelTypeAttribute().replace("rel", "relationship")));

	aux_rel=dynamic_cast<Relationship *>(base_rel);
	has_foreign_tab = (base_rel->getTable(BaseRelationship::SrcTable)->getObjectType() == ObjectType::ForeignTable ||
										 base_rel->getTable(BaseRelationship::DstTable)->getObjectType() == ObjectType::ForeignTable);

	if(base_rel->getObjectType()==ObjectType::BaseRelationship)
	{
		if(base_rel->getRelationshipType()!=BaseRelationship::RelationshipFk)
		{
			ref_table_lbl->setText(tr("Referrer view:"));
			ref_table_txt->setToolTip(tr("<p>Referrer view references one or more columns of a table to construct it's own columns.</p>"));
			recv_table_txt->setToolTip(tr("<p>Referenced table has its columns referenced by a view in order to construct the columns of this latter.</p>"));
		}
		else
		{
			ref_table_lbl->setText(tr("Referrer table:"));
			ref_table_txt->setToolTip(tr("<p>Referrer table references one or more columns of a table through foreign keys. This is the (n) side of relationship.</p>"));
			recv_table_txt->setToolTip(tr("<p>Referenced table has its columns referenced by a table's foreign key. This is the (1) side of relationship.</p>"));
		}

		recv_table_lbl->setText(tr("Referenced table:"));

		ref_table_txt->setPlainText(base_rel->getTable(BaseRelationship::SrcTable)->getName(true));
		recv_table_txt->setPlainText(base_rel->getTable(BaseRelationship::DstTable)->getName(true));
	}
	else if(aux_rel)
	{
		if(rel_type == BaseRelationship::RelationshipPart)
		{
			ref_table_lbl->setText(tr("Partitioned table:"));
			ref_table_txt->setToolTip(tr("<p>Partitioned table is the one which is splitted into smaller pieces (partitions). This table is where the partitioning strategy or type is defined.</p>"));

			recv_table_lbl->setText(tr("Partition table:"));
			recv_table_txt->setToolTip(tr("<p>Partition table is the one attached to a partitioned table in which operations over data will be routed (according to the paritionig rule) when trying to handle the partitioned table.</p>"));

			ref_table_txt->setPlainText(aux_rel->getReferenceTable()->getName(true));
			recv_table_txt->setPlainText(aux_rel->getReceiverTable()->getName(true));
		}
		else if(rel_type!=BaseRelationship::RelationshipNn)
		{
			ref_table_lbl->setText(tr("Reference table:"));
			ref_table_txt->setToolTip(tr("<p>Reference table has the columns from its primary key will copied to the receiver table in order to represent the linking between them. This is the (1) side of relationship.</p>"));

			recv_table_lbl->setText(tr("Receiver table:"));
			recv_table_txt->setToolTip(tr("<p>Receiver (or referer) table will receive the generated columns and the foreign key in order to represent the linking between them. This is the (n) side of relationship.</p>"));

			ref_table_txt->setPlainText(aux_rel->getReferenceTable()->getName(true));
			recv_table_txt->setPlainText(aux_rel->getReceiverTable()->getName(true));
		}
		else
		{
			ref_table_lbl->setText(tr("Reference table:"));
			ref_table_txt->setToolTip(tr("<p>In many-to-many relationships both tables are used as reference to generate the table that represents the linking. Columns from both tables are copied to the resultant table and two foreign keys are created as well in order to reference each participant table.</p>"));
			recv_table_lbl->setText(tr("Reference Table:"));
			recv_table_txt->setToolTip(ref_table_txt->toolTip());
			ref_table_txt->setPlainText(base_rel->getTable(BaseRelationship::SrcTable)->getName(true));
			recv_table_txt->setPlainText(base_rel->getTable(BaseRelationship::DstTable)->getName(true));
		}

		if(rel_type == BaseRelationship::RelationshipPart)
		{
			part_type_lbl->setText(~aux_rel->getReferenceTable()->getPartitioningType());

			// Default partitions can't be used by foreign tables
			default_part_chk->setChecked(!has_foreign_tab && aux_rel->getPartitionBoundingExpr().isEmpty());
			default_part_chk->setEnabled(!has_foreign_tab);
		}
	}

	disable_sql_chk->setVisible(base_rel->getObjectType()==ObjectType::Relationship);

	if(aux_rel)
	{
		single_pk_chk->setChecked(aux_rel->isSiglePKColumn());
		table1_mand_chk->setChecked(aux_rel->isTableMandatory(BaseRelationship::SrcTable));
		table2_mand_chk->setChecked(aux_rel->isTableMandatory(BaseRelationship::DstTable));
		identifier_chk->setChecked(aux_rel->isIdentifier());
		relnn_tab_name_edt->setText(aux_rel->getTableNameRelNN());
		attributes_tab->setButtonsEnabled(CustomTableWidget::AllButtons, !aux_rel->isProtected());
		constraints_tab->setButtonsEnabled(CustomTableWidget::AllButtons, !aux_rel->isProtected());

		//Lists the relationship attributes
		listObjects(ObjectType::Column);

		//Lists the relationship constraints
		listObjects(ObjectType::Constraint);

		listSpecialPkColumns();

		if(rel_type!=BaseRelationship::RelationshipNn)
		{
			if(rel_type==BaseRelationship::RelationshipDep)
			{
				CopyOptions copy_op=aux_rel->getCopyOptions();

				including_rb->setChecked(copy_op.isIncluding());
				excluding_rb->setChecked(copy_op.isExcluding());
				all_chk->setChecked(copy_op.isOptionSet(CopyOptions::All));
				defaults_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Defaults));
				constraints_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Constraints));
				storage_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Storage));
				comments_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Comments));
				indexes_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Indexes));
				identity_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Identity));
				statistics_chk->setChecked(!all_chk->isChecked() && copy_op.isOptionSet(CopyOptions::Statistics));
			}
			else if(rel_type == BaseRelationship::RelationshipPart)
			{
				if(this->new_object)
					generateBoundingExpr();
				else
					part_bound_expr_txt->setPlainText(aux_rel->getPartitionBoundingExpr());
			}
		}
	}

	rel1n=(rel_type==BaseRelationship::Relationship11 ||
				 rel_type==BaseRelationship::Relationship1n);

	relnn=(rel_type==BaseRelationship::RelationshipNn);

	relgen_dep=(rel_type==BaseRelationship::RelationshipDep ||
							rel_type==BaseRelationship::RelationshipGen ||
							rel_type==BaseRelationship::RelationshipPart ||
							rel_type==BaseRelationship::RelationshipFk);

	use_name_patterns = (rel1n || relnn || (relgen_dep && base_rel->getObjectType() == ObjectType::Relationship));
	name_patterns_grp->setVisible(use_name_patterns);

	dst_col_pattern_txt->setEnabled(relnn);
	dst_fk_pattern_txt->setEnabled(relnn);
	dst_col_pattern_lbl->setEnabled(relnn);
	dst_fk_pattern_lbl->setEnabled(relnn);
	src_col_pattern_lbl->setEnabled(!relgen_dep);
	src_col_pattern_txt->setEnabled(!relgen_dep);
	src_fk_pattern_lbl->setEnabled(!relgen_dep);
	src_fk_pattern_txt->setEnabled(!relgen_dep);
	uq_pattern_lbl->setEnabled(!relgen_dep);
	uq_pattern_txt->setEnabled(!relgen_dep);

	fk_idx_pattern_lbl->setEnabled(rel1n || relnn);
	fk_idx_pattern_txt->setEnabled(rel1n || relnn);

	pk_col_pattern_lbl->setEnabled(relnn);
	pk_col_pattern_txt->setEnabled(relnn);

	table1_mand_chk->setEnabled(rel1n);
	table1_mand_chk->setVisible(rel1n);
	table2_mand_chk->setEnabled(rel_type == BaseRelationship::Relationship11);
	table2_mand_chk->setVisible(rel1n);

	identifier_wgt->setVisible(rel1n && !base_rel->isSelfRelationship());
	foreign_key_gb->setVisible(rel1n || relnn);
	single_pk_wgt->setVisible(relnn);

	relnn_tab_name_lbl->setVisible(relnn);
	relnn_tab_name_edt->setVisible(relnn);

	part_bound_expr_gb->setVisible(rel_type == BaseRelationship::RelationshipPart);

	for(unsigned i=SettingsTab; i <= AdvancedTab; i++)
		rel_attribs_tbw->setTabVisible(i, false);

	if(!relgen_dep)
	{
		for(unsigned i=SettingsTab; i <= SpecialPkTab; i++)
			rel_attribs_tbw->setTabVisible(i, true);
	}
	else if(relgen_dep && base_rel->getObjectType()==ObjectType::Relationship && !has_foreign_tab)
	{
		rel_attribs_tbw->setTabVisible(SettingsTab, true);
		rel_attribs_tbw->setTabVisible(SpecialPkTab, true);
	}

	if(base_rel->getObjectType() == ObjectType::Relationship ||
		 (base_rel->getObjectType() == ObjectType::BaseRelationship &&
			base_rel->getRelationshipType() == BaseRelationship::RelationshipFk))
		rel_attribs_tbw->setTabVisible(AdvancedTab, true);

	copy_options_grp->setVisible(base_rel->getObjectType() == ObjectType::Relationship &&
															 base_rel->getRelationshipType() == BaseRelationship::RelationshipDep);

	custom_color_chk->setChecked(base_rel->getCustomColor()!=Qt::transparent);
	color_picker->setColor(0, base_rel->getCustomColor());
	listAdvancedObjects();

	if(rel1n || relnn)
	{
		fk_gconf_chk->blockSignals(true);
		fk_gconf_chk->setChecked(this->new_object);
		useFKGlobalSettings(this->new_object);
		fk_gconf_chk->blockSignals(false);
		rel_attribs_tbw->widget(SettingsTab)->setFocusProxy(fk_gconf_chk);
	}
	else if(relgen_dep)
		rel_attribs_tbw->widget(SettingsTab)->setFocusProxy(defaults_rb);
	else
		rel_attribs_tbw->widget(SettingsTab)->setFocusProxy(patterns_gconf_chk);

	if(use_name_patterns)
	{
		patterns_gconf_chk->blockSignals(true);
		patterns_gconf_chk->setChecked(this->new_object);
		usePatternGlobalSettings(this->new_object);
		patterns_gconf_chk->blockSignals(false);
	}
}

QSize RelationshipWidget::getIdealSize()
{
	BaseRelationship::RelType rel_type = BaseRelationship::Relationship11;

	if(this->object)
		rel_type = dynamic_cast<BaseRelationship *>(this->object)->getRelationshipType();

	if(rel_type == BaseRelationship::RelationshipFk ||
		 (rel_type == BaseRelationship::RelationshipDep &&
			this->object && this->object->getObjectType()==ObjectType::BaseRelationship))
		return { 640, 320 };

	if(rel_type == BaseRelationship::RelationshipGen)
		return { 640, 520 };

	return { 640, 680 };
}

void RelationshipWidget::useFKGlobalSettings(bool value)
{
	int idx = -1;
	fk_wgt->setEnabled(!value);

	if(value)
	{
		std::map<QString, attribs_map> confs=RelationshipConfigWidget::getConfigurationParams();
		deferrable_chk->setChecked(confs[Attributes::ForeignKeys][Attributes::Deferrable]==Attributes::True);
		deferral_cmb->setCurrentText(confs[Attributes::ForeignKeys][Attributes::DeferType]);
		upd_action_cmb->setCurrentText(confs[Attributes::ForeignKeys][Attributes::UpdAction]);
		del_action_cmb->setCurrentText(confs[Attributes::ForeignKeys][Attributes::DelAction]);

		idx = index_type_cmb->findText(confs[Attributes::ForeignKeys][Attributes::FkIdxType]);
		index_type_cmb->setCurrentIndex(idx < 0 ? 0 : idx);
	}
	else
	{
		Relationship *rel = dynamic_cast<Relationship *>(this->object);

		//Using the settings of the relatinship itself
		if(rel)
		{
			deferrable_chk->setChecked(rel->isDeferrable());
			idx = deferral_cmb->findText(~rel->getDeferralType());
			deferral_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

			idx = del_action_cmb->findText(~rel->getActionType(Constraint::DeleteAction));
			del_action_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

			idx = upd_action_cmb->findText(~rel->getActionType(Constraint::UpdateAction));
			upd_action_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

			idx = index_type_cmb->findText(~rel->getFKIndexType());
			index_type_cmb->setCurrentIndex(idx < 0 ? 0 : idx);
		}
	}
}

void RelationshipWidget::usePatternGlobalSettings(bool value)
{
	Relationship *rel=dynamic_cast<Relationship *>(this->object);
	patterns_wgt->setEnabled(!value);

	if(rel)
	{
		if(value)
		{
			std::map<QString, attribs_map> confs=RelationshipConfigWidget::getConfigurationParams();
			QString rel_type=rel->getRelTypeAttribute();

			//Using the global settings
			pk_pattern_txt->setPlainText(confs[rel_type][Attributes::PkPattern]);
			src_fk_pattern_txt->setPlainText(confs[rel_type][Attributes::SrcFkPattern]);
			dst_fk_pattern_txt->setPlainText(confs[rel_type][Attributes::DstFkPattern]);
			uq_pattern_txt->setPlainText(confs[rel_type][Attributes::UqPattern]);
			src_col_pattern_txt->setPlainText(confs[rel_type][Attributes::SrcColPattern]);
			dst_col_pattern_txt->setPlainText(confs[rel_type][Attributes::DstColPattern]);
			pk_col_pattern_txt->setPlainText(confs[rel_type][Attributes::PkColPattern]);
			fk_idx_pattern_txt->setPlainText(confs[rel_type][Attributes::FkIdxPattern]);
		}
		else
		{
			//Using the settings of the relatinship itself
			pk_pattern_txt->setPlainText(rel->getNamePattern(Relationship::PkPattern));
			src_fk_pattern_txt->setPlainText(rel->getNamePattern(Relationship::SrcFkPattern));
			dst_fk_pattern_txt->setPlainText(rel->getNamePattern(Relationship::DstFkPattern));
			uq_pattern_txt->setPlainText(rel->getNamePattern(Relationship::UqPattern));
			src_col_pattern_txt->setPlainText(rel->getNamePattern(Relationship::SrcColPattern));
			dst_col_pattern_txt->setPlainText(rel->getNamePattern(Relationship::DstColPattern));
			pk_col_pattern_txt->setPlainText(rel->getNamePattern(Relationship::PkColPattern));
			fk_idx_pattern_txt->setPlainText(rel->getNamePattern(Relationship::FkIdxPattern));
		}
	}
}

void RelationshipWidget::generateBoundingExpr()
{
	PartitioningType part_type = part_type_lbl->text();
	QString tmpl;

	if(part_type == PartitioningType::List)
		tmpl = "IN (value)";
	else if(part_type == PartitioningType::Range)
		tmpl = "FROM (value) TO (value)";
	else
		tmpl = "WITH (MODULUS m, REMAINDER r)";

	part_bound_expr_txt->setPlainText("");
	part_bound_expr_txt->setPlainText(tmpl);
	default_part_chk->setChecked(false);
}

void RelationshipWidget::listObjects(ObjectType obj_type)
{
	CustomTableWidget *tab=nullptr;
	Relationship *rel=nullptr;
	unsigned count, i;

	try
	{
		if(obj_type==ObjectType::Column)
			tab=attributes_tab;
		else
			tab=constraints_tab;

		rel=dynamic_cast<Relationship *>(this->object);

		tab->blockSignals(true);
		tab->removeRows();

		count=rel->getObjectCount(obj_type);
		for(i=0; i < count; i++)
		{
			tab->addRow();
			showObjectData(rel->getObject(i, obj_type), i);
		}
		tab->clearSelection();
		tab->blockSignals(false);

		constraints_tab->setButtonsEnabled(CustomTableWidget::AddButton,
										   attributes_tab->getRowCount() > 0);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::listAdvancedObjects()
{
	BaseRelationship *base_rel=nullptr;
	Relationship *rel=nullptr;
	Table *tab=nullptr;
	std::vector<Column *> cols;
	std::vector<Constraint *> constrs;
	unsigned count=0, i,i1;

	try
	{
		base_rel=dynamic_cast<BaseRelationship *>(this->object);
		rel=dynamic_cast<Relationship *>(base_rel);

		advanced_objs_tab->blockSignals(true);
		advanced_objs_tab->removeRows();

		if(rel)
		{
			if(rel->getRelationshipType() != BaseRelationship::RelationshipNn)
			{
				cols = rel->getGeneratedColumns();
				count = cols.size();

				for(i = 0; i < count; i++)
				{
					advanced_objs_tab->addRow();
					advanced_objs_tab->setCellText(cols[i]->getName(), i, 0);
					advanced_objs_tab->setCellText(cols[i]->getTypeName(), i, 1);
					advanced_objs_tab->setRowData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(cols[i])), i);
				}

				constrs = rel->getGeneratedConstraints();
				count = constrs.size();

				for(i = 0, i1 = advanced_objs_tab->getRowCount(); i < count; i++,i1++)
				{
					advanced_objs_tab->addRow();
					advanced_objs_tab->setCellText(constrs[i]->getName(), i1, 0);
					advanced_objs_tab->setCellText(constrs[i]->getTypeName(), i1, 1);
					advanced_objs_tab->setRowData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constrs[i])), i1);
				}

				if(rel->getGeneratedIndex())
				{
					Index *idx = rel->getGeneratedIndex();
					advanced_objs_tab->addRow();
					advanced_objs_tab->setCellText(idx->getName(), i1, 0);
					advanced_objs_tab->setCellText(idx->getTypeName(), i1, 1);
					advanced_objs_tab->setRowData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(idx)), i1);
				}
			}
			else
			{
				tab = rel->getGeneratedTable();
				if(tab)
				{
					advanced_objs_tab->addRow();
					advanced_objs_tab->setCellText(tab->getName(),0,0);
					advanced_objs_tab->setCellText(tab->getTypeName(),0,1);
					advanced_objs_tab->setRowData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(tab)), 0);
				}
			}
		}
		else if(base_rel->getRelationshipType()==BaseRelationship::RelationshipFk)
		{
			Constraint *fk = base_rel->getReferenceForeignKey();

			if(fk)
			{
				int row = 0;

				advanced_objs_tab->addRow();
				row = advanced_objs_tab->getRowCount() - 1;

				advanced_objs_tab->setCellText(fk->getName(), row ,0);
				advanced_objs_tab->setCellText(fk->getTypeName(), row, 1);
				advanced_objs_tab->setRowData(QVariant::fromValue<void *>(fk), row);
			}
		}

		advanced_objs_tab->clearSelection();
		advanced_objs_tab->blockSignals(false);

	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::showAdvancedObject(int row)
{
	try
	{
		BaseObject *object=reinterpret_cast<BaseObject *>(advanced_objs_tab->getRowData(row).value<void *>());
		ObjectType obj_type=object->getObjectType();
		bool is_protected = false;

		if(obj_type == ObjectType::Column)
		{
			Column *col = dynamic_cast<Column *>(object);
			//is_protected = col->isProtected();
			openEditingForm<Column,ColumnWidget>(col, col->getParentTable());
		}
		else if(obj_type == ObjectType::Constraint)
		{
			Constraint *constr = dynamic_cast<Constraint *>(object);

			if(!constr->isAddedByRelationship())
			{
				is_protected = constr->isProtected();
				constr->setProtected(true);
			}

			openEditingForm<Constraint, ConstraintWidget>(constr, constr->getParentTable());

			if(!constr->isAddedByRelationship())
				constr->setProtected(is_protected);
		}
		else if(obj_type == ObjectType::Index)
		{
			Index *index = dynamic_cast<Index *>(object);

			if(!index->isAddedByRelationship())
			{
				is_protected = index->isProtected();
				index->setProtected(true);
			}

			openEditingForm<Index, IndexWidget>(index, index->getParentTable());

			if(!index->isAddedByRelationship())
				index->setProtected(is_protected);
		}
		else
		{
			Table *tab = dynamic_cast<Table *>(object);
			TableWidget *table_wgt=new TableWidget;			
			BaseForm editing_form(this);

			tab->setProtected(true);

			table_wgt->setAttributes(this->model, this->op_list, dynamic_cast<Schema *>(tab->getSchema()),
																tab,	tab->getPosition().x(), tab->getPosition().y());

			editing_form.setMainWidget(table_wgt);

			GeneralConfigWidget::restoreWidgetGeometry(&editing_form, table_wgt->metaObject()->className());
			editing_form.exec();
			GeneralConfigWidget::saveWidgetGeometry(&editing_form, table_wgt->metaObject()->className());

			tab->setProtected(false);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

template<class Class, class WidgetClass>
int RelationshipWidget::openEditingForm(TableObject *object, BaseObject *parent)
{
	BaseForm editing_form(this);
	WidgetClass *object_wgt=new WidgetClass;
	BaseObject *parent_aux = nullptr;
	int res = 0;

	if(this->object->getObjectType() == ObjectType::BaseRelationship)
		parent_aux = dynamic_cast<BaseRelationship *>(this->object)->getTable(BaseRelationship::SrcTable);
	else
		parent_aux = !parent ? this->object : parent;

	if constexpr (std::is_same_v<Class, Index>)
	{
		object_wgt->setAttributes(this->model, this->op_list,
															dynamic_cast<BaseTable *>(parent_aux),
															dynamic_cast<Class *>(object));
	}
	else
		object_wgt->setAttributes(this->model, this->op_list, parent_aux, dynamic_cast<Class *>(object));

	editing_form.setMainWidget(object_wgt);

	GeneralConfigWidget::restoreWidgetGeometry(&editing_form, object_wgt->metaObject()->className());
	res = editing_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&editing_form, object_wgt->metaObject()->className());

	return res;
}

void RelationshipWidget::addObject()
{
	ObjectType obj_type=ObjectType::BaseObject;

	try
	{
		if(sender()==attributes_tab)
		{
			obj_type=ObjectType::Column;
			tab=attributes_tab;
		}
		else
		{
			obj_type=ObjectType::Constraint;
			tab=constraints_tab;
		}

		if(obj_type==ObjectType::Column)
			openEditingForm<Column,ColumnWidget>(nullptr);
		else
			openEditingForm<Constraint,ConstraintWidget>(nullptr);

		listObjects(obj_type);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::duplicateObject(int curr_row, int new_row)
{
	ObjectType obj_type=ObjectType::BaseObject;
	BaseObject *object = nullptr, *dup_object = nullptr;
	Relationship *rel = dynamic_cast<Relationship *>(this->object);
	std::vector<TableObject *> obj_list;
	CustomTableWidget *tab = nullptr;
	int op_id = -1;

	if(!rel)
		return;

	try
	{
		if(sender()==attributes_tab)
		{
			obj_type=ObjectType::Column;
			tab=attributes_tab;
			obj_list = rel->getAttributes();
		}
		else
		{
			obj_type=ObjectType::Constraint;
			tab=constraints_tab;
			obj_list = rel->getConstraints();
		}

		//Gets the object reference if there is an item select on table
		if(curr_row >= 0)
			object = reinterpret_cast<BaseObject *>(tab->getRowData(curr_row).value<void *>());

		CoreUtilsNs::copyObject(&dup_object, object, obj_type);
		dup_object->setName(CoreUtilsNs::generateUniqueName(dup_object, obj_list, false, "_cp"));
		op_id=op_list->registerObject(dup_object, Operation::ObjCreated, new_row, rel);

		dynamic_cast<TableObject*>(dup_object)->setParentTable(nullptr);
		rel->addObject(dynamic_cast<TableObject *>(dup_object));
		listObjects(obj_type);
	}
	catch(Exception &e)
	{
		//If operation was registered
		if(op_id >= 0)
		{
			op_list->ignoreOperationChain(true);
			op_list->removeLastOperation();
			op_list->ignoreOperationChain(false);
		}

		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::editObject(int row)
{
	ObjectType obj_type=ObjectType::Column;
	TableObject *tab_obj=nullptr;

	try
	{
		op_list->ignoreOperationChain(true);

		if(sender()==attributes_tab)
		{
			obj_type=ObjectType::Column;
			tab_obj=reinterpret_cast<TableObject *>(attributes_tab->getRowData(row).value<void *>());
			openEditingForm<Column,ColumnWidget>(tab_obj);
		}
		else
		{
			obj_type=ObjectType::Constraint;
			tab_obj=reinterpret_cast<TableObject *>(constraints_tab->getRowData(row).value<void *>());
			openEditingForm<Constraint,ConstraintWidget>(tab_obj);
		}

		listObjects(obj_type);
		op_list->ignoreOperationChain(false);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		op_list->ignoreOperationChain(false);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::showObjectData(TableObject *object, int row)
{
	CustomTableWidget *tab=nullptr;

	if(object->getObjectType()==ObjectType::Column)
	{
		tab=attributes_tab;
		attributes_tab->setCellText(*dynamic_cast<Column *>(object)->getType(),row,1);
	}
	else
	{
		tab=constraints_tab;
		constraints_tab->setCellText(~dynamic_cast<Constraint *>(object)->getConstraintType(),row,1);
	}

	tab->setCellText(object->getName(),row,0);
	tab->setRowData(QVariant::fromValue<void *>(object), row);
}

void RelationshipWidget::removeObjects()
{
	Relationship *rel=nullptr;
	ObjectType obj_type=ObjectType::BaseObject;
	unsigned count, op_count=0, i;
	TableObject *object=nullptr;

	try
	{
		rel=dynamic_cast<Relationship *>(this->object);

		if(sender()==attributes_tab)
		{
			obj_type=ObjectType::Column;
			count=rel->getAttributeCount();
		}
		else
		{
			obj_type=ObjectType::Constraint;
			count=rel->getConstraintCount();
		}

		op_count=op_list->getCurrentSize();

		for(i=0; i < count; i++)
		{
			object=rel->getObject(0, obj_type);
			op_list->registerObject(object, Operation::ObjRemoved, 0, rel);
			rel->removeObject(object);
		}

		if(obj_type==ObjectType::Column)
			listSpecialPkColumns();
	}
	catch(Exception &e)
	{
		if(op_count < op_list->getCurrentSize())
		{
			count=op_list->getCurrentSize()-op_count;
			op_list->ignoreOperationChain(true);

			for(i=0; i < count; i++)
			{
				op_list->undoOperation();
				op_list->removeLastOperation();
			}

			op_list->ignoreOperationChain(false);
		}

		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::removeObject(int row)
{
	Relationship *rel=nullptr;
	ObjectType obj_type=ObjectType::BaseObject;
	TableObject *object=nullptr;
	int op_id=-1;

	try
	{
		rel=dynamic_cast<Relationship *>(this->object);

		if(sender()==attributes_tab)
			obj_type=ObjectType::Column;
		else
			obj_type=ObjectType::Constraint;

		object=rel->getObject(row, obj_type);
		op_id=op_list->registerObject(object, Operation::ObjRemoved, 0, rel);
		rel->removeObject(object);

		if(obj_type==ObjectType::Column)
			listSpecialPkColumns();
	}
	catch(Exception &e)
	{
		//If operation was registered
		if(op_id >= 0)
		{
			op_list->ignoreOperationChain(true);
			op_list->removeLastOperation();
			op_list->ignoreOperationChain(false);
		}

		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::selectCopyOptions()
{
	all_chk->setEnabled(!defaults_rb->isChecked());

	defaults_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	constraints_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	storage_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	comments_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	indexes_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	identity_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());
	statistics_chk->setEnabled(!all_chk->isChecked() && !defaults_rb->isChecked());

	if(all_chk->isChecked() || defaults_rb->isChecked())
	{
		if(defaults_rb->isChecked())
			all_chk->setChecked(false);

		defaults_chk->setChecked(false);
		constraints_chk->setChecked(false);
		storage_chk->setChecked(false);
		comments_chk->setChecked(false);
		indexes_chk->setChecked(false);
		identity_chk->setChecked(false);
		statistics_chk->setChecked(false);
	}
}

void RelationshipWidget::listSpecialPkColumns()
{
	Relationship *aux_rel=dynamic_cast<Relationship *>(this->object);

	if(aux_rel)
	{
		std::vector<Column *> cols;
		std::vector<unsigned> col_ids;
		int count, idx;
		QListWidgetItem *item=nullptr;

		rel_columns_lst->clear();

		if(aux_rel->getRelationshipType()!=BaseRelationship::RelationshipNn)
			cols=aux_rel->getGeneratedColumns();

		for(auto &attrib : aux_rel->getAttributes())
			cols.push_back(dynamic_cast<Column *>(attrib));

		//Get the special primary key columns ids
		col_ids=aux_rel->getSpecialPrimaryKeyCols();

		count=cols.size();
		for(idx=0; idx < count; idx++)
		{
			rel_columns_lst->addItem(cols[idx]->getName().toUtf8() +
									 " (" + *cols[idx]->getType() + ")");
			item=rel_columns_lst->item(idx);
			item->setCheckState(Qt::Unchecked);
		}

		count=col_ids.size();
		for(idx=0; idx < count; idx++)
		{
			if(col_ids[idx] < static_cast<unsigned>(rel_columns_lst->count()))
				rel_columns_lst->item(col_ids[idx])->setCheckState(Qt::Checked);
		}
	}
}

void RelationshipWidget::applyConfiguration()
{
	try
	{
		Relationship *rel=nullptr;
		BaseRelationship *base_rel=dynamic_cast<BaseRelationship *>(this->object);
		BaseRelationship::RelType rel_type;
		unsigned count, i, copy_ops = CopyOptions::NoOpts;
		CopyOptions::CopyMode copy_mode = CopyOptions::NoMode;
		std::vector<unsigned> col_ids;

		/* Due to the complexity of the Relationship class and the strong link between all
		 * the relationships on the model is necessary to store the XML of the special objects
		 * and disconnect all relationships, edit the relationshi and revalidate all the relationships again */
		if(this->object->getObjectType()==ObjectType::Relationship)
		{
			model->storeSpecialObjectsXML();
			model->disconnectRelationships();
		}

		if(!this->new_object && this->object->getObjectType()==ObjectType::Relationship)
			op_list->registerObject(this->object, Operation::ObjModified);
		else
			registerNewObject();

		BaseObjectWidget::applyConfiguration();

		if(custom_color_chk->isChecked())
			base_rel->setCustomColor(color_picker->getColor(0));
		else
			base_rel->setCustomColor(Qt::transparent);

		if(this->object->getObjectType() == ObjectType::Relationship)
		{
			std::vector<QPlainTextEdit *> pattern_fields={ src_col_pattern_txt, dst_col_pattern_txt,
																										 src_fk_pattern_txt, dst_fk_pattern_txt,
																										 pk_pattern_txt, uq_pattern_txt,
																										 pk_col_pattern_txt, fk_idx_pattern_txt };

			std::vector<Relationship::PatternId> pattern_ids= { Relationship::SrcColPattern, Relationship::DstColPattern,
																													Relationship::SrcFkPattern, Relationship::DstFkPattern,
																													Relationship::PkPattern, Relationship::UqPattern,
																													Relationship::PkColPattern, Relationship::FkIdxPattern };

			rel = dynamic_cast<Relationship *>(base_rel);

			if(use_name_patterns)
			{
				for(auto &patt_id : pattern_ids)
					rel->setNamePattern(patt_id, pattern_fields[patt_id]->toPlainText());
			}

			rel_type=rel->getRelationshipType();
			rel->blockSignals(true);

			rel->setPartitionBoundingExpr(default_part_chk->isChecked() ? "" : part_bound_expr_txt->toPlainText());

			if(!defaults_rb->isChecked())
			{
				if(including_rb->isChecked())
					copy_mode = CopyOptions::Including;
				else
					copy_mode = CopyOptions::Excluding;

				copy_ops+=(all_chk->isChecked() ? CopyOptions::All : CopyOptions::NoOpts);
				copy_ops+=(defaults_chk->isChecked() ? CopyOptions::Defaults : CopyOptions::NoOpts);
				copy_ops+=(constraints_chk->isChecked() ? CopyOptions::Constraints : CopyOptions::NoOpts);
				copy_ops+=(comments_chk->isChecked() ? CopyOptions::Comments : CopyOptions::NoOpts);
				copy_ops+=(indexes_chk->isChecked() ? CopyOptions::Indexes : CopyOptions::NoOpts);
				copy_ops+=(storage_chk->isChecked() ? CopyOptions::Storage : CopyOptions::NoOpts);
				copy_ops+=(identity_chk->isChecked() ? CopyOptions::Identity : CopyOptions::NoOpts);
				copy_ops+=(statistics_chk->isChecked() ? CopyOptions::Statistics : CopyOptions::NoOpts);
			}

			rel->setCopyOptions(CopyOptions(copy_mode, static_cast<CopyOptions::CopyOpts>(copy_ops)));
			rel->setMandatoryTable(BaseRelationship::SrcTable, false);
			rel->setMandatoryTable(BaseRelationship::DstTable, false);

			if(table1_mand_chk->isEnabled())
				rel->setMandatoryTable(BaseRelationship::SrcTable, table1_mand_chk->isChecked());

			if(table2_mand_chk->isEnabled())
				rel->setMandatoryTable(BaseRelationship::DstTable, table2_mand_chk->isChecked());

			if(rel_type==BaseRelationship::Relationship1n ||
					rel_type==BaseRelationship::Relationship11)
				rel->setIdentifier(identifier_chk->isChecked());
			else if(rel_type==BaseRelationship::RelationshipNn)
				rel->setTableNameRelNN(relnn_tab_name_edt->text());

			if(rel_type==BaseRelationship::Relationship1n ||
				 rel_type==BaseRelationship::Relationship11 ||
				 rel_type==BaseRelationship::RelationshipNn)
			{
				rel->setDeferrable(deferrable_chk->isChecked());
				rel->setDeferralType(DeferralType(deferral_cmb->currentText()));
				rel->setActionType((del_action_cmb->currentIndex() != 0 ? ActionType(del_action_cmb->currentText()) : ActionType::Null), Constraint::DeleteAction);
				rel->setActionType((upd_action_cmb->currentIndex() != 0 ? ActionType(upd_action_cmb->currentText()) : ActionType::Null), Constraint::UpdateAction);
				rel->setFKIndexType(index_type_cmb->currentIndex() != 0 ? IndexingType(index_type_cmb->currentText()) : IndexingType::Null);

				if(rel_type == BaseRelationship::RelationshipNn)
					rel->setSiglePKColumn(single_pk_chk->isChecked());
			}

			count=rel_columns_lst->count();
			for(i=0; i < count; i++)
			{
				if(rel_columns_lst->item(i)->checkState()==Qt::Checked)
					col_ids.push_back(i);
			}

			rel->setSpecialPrimaryKeyCols(col_ids);

			try
			{
				//Checking if there is relationship redundancy
				if(rel_type==BaseRelationship::RelationshipDep ||
						rel_type==BaseRelationship::RelationshipGen ||
						rel_type==BaseRelationship::RelationshipPart ||
						rel->isIdentifier())
					model->checkRelationshipRedundancy(rel);

				if(rel_type != BaseRelationship::RelationshipFk)
					model->validateRelationships();

				rel->blockSignals(false);
			}
			catch(Exception &e)
			{
				if(e.getErrorCode()==ErrorCode::RemInvalidatedObjects)
					Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
				else
					throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
			}
		}

		op_list->finishOperationChain();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		model->validateRelationships();
		//qApp->restoreOverrideCursor();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void RelationshipWidget::cancelConfiguration()
{
	BaseObjectWidget::cancelChainedOperation();
}
