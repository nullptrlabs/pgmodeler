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

#include "constraintwidget.h"
#include "guiutilsns.h"

ConstraintWidget::ConstraintWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Constraint)
{
	QStringList list;
	std::map<QString, std::vector<QWidget *> > fields_map;
	std::map<QWidget *, std::vector<QString> > values_map;
	QGridLayout *grid=nullptr;

	Ui_ConstraintWidget::setupUi(this);

	excl_elems_tab=new ElementsTableWidget(this);
	grid=new QGridLayout;
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	grid->addWidget(excl_elems_tab,0,0);
	excl_elems_grp->setLayout(grid);

	expression_hl=new SyntaxHighlighter(expression_txt, false, true, font().pointSizeF());
	expression_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	ref_table_sel=new ObjectSelectorWidget(ObjectType::Table, this);
	col_picker_wgt = new ColumnPickerWidget(this);
	ref_col_picker_wgt = new ColumnPickerWidget(this);

	QVBoxLayout *vbox = new QVBoxLayout(columns_tbw->widget(0));
	vbox->addWidget(col_picker_wgt);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);

	dynamic_cast<QGridLayout *>(columns_tbw->widget(1)->layout())->addWidget(ref_table_sel, 0,1,1,2);
	dynamic_cast<QGridLayout *>(columns_tbw->widget(1)->layout())->addWidget(ref_col_picker_wgt, 3,0,1,3);

	configureFormLayout(constraint_grid, ObjectType::Constraint);

	constr_type_cmb->addItems(ConstraintType::getTypes());
	match_cmb->addItems(MatchType::getTypes());
	deferral_cmb->addItems(DeferralType::getTypes());
	indexing_cmb->addItems(IndexingType::getTypes());

	list = ActionType::getTypes();
	on_delete_cmb->addItems(list);
	on_update_cmb->addItems(list);

	info_frm=generateInformationFrame(tr("Columns which were included by relationship can not be added / removed manually from the primary key. If done such changes they can raise errors. To create primary key using columns included by relationship use the following options: identifier field, attributes & constraints tab or primary key tab on the relationship form."));
	constraint_grid->addWidget(info_frm, constraint_grid->count()+1, 0, 1, 0);
	info_frm->setParent(this);

	connect(constr_type_cmb, &QComboBox::currentIndexChanged, this, &ConstraintWidget::selectConstraintType);
	connect(deferrable_chk, &QCheckBox::toggled, deferral_cmb, &QComboBox::setEnabled);
	connect(deferrable_chk, &QCheckBox::toggled, deferral_lbl, &QLabel::setEnabled);
	connect(indexing_chk, &QCheckBox::toggled, indexing_cmb, &QComboBox::setEnabled);
	connect(fill_factor_chk, &QCheckBox::toggled, fill_factor_sb, &QSpinBox::setEnabled);

	connect(ref_table_sel, &ObjectSelectorWidget::s_selectorCleared, this, __slot(this, ConstraintWidget::selectReferencedTable));
	connect(ref_table_sel, &ObjectSelectorWidget::s_objectSelected, this, __slot(this, ConstraintWidget::selectReferencedTable));

	selectConstraintType();
	configureTabOrder();
	setMinimumSize(540, 600);
}

void ConstraintWidget::selectReferencedTable()
{
	ref_col_picker_wgt->setParentObject(ref_table_sel->getSelectedObject());
}

void ConstraintWidget::selectConstraintType()
{
	ConstraintType constr_type=ConstraintType(constr_type_cmb->currentText());

	tablespace_lbl->setVisible(constr_type==ConstraintType::PrimaryKey || constr_type==ConstraintType::Unique);
	tablespace_sel->setVisible(constr_type==ConstraintType::PrimaryKey || constr_type==ConstraintType::Unique);

	if(!tablespace_sel->isVisible()) tablespace_sel->clearSelector();

	expression_lbl->setVisible(constr_type==ConstraintType::Check || constr_type==ConstraintType::Exclude);
	expression_txt->setVisible(constr_type==ConstraintType::Check || constr_type==ConstraintType::Exclude);
	no_inherit_chk->setVisible(constr_type==ConstraintType::Check);

	nulls_not_distinct_chk->setVisible(constr_type==ConstraintType::Unique);

	fill_factor_chk->setVisible(constr_type==ConstraintType::Unique ||
								constr_type==ConstraintType::PrimaryKey ||
								constr_type==ConstraintType::Exclude);
	fill_factor_sb->setVisible(constr_type==ConstraintType::Unique ||
							   constr_type==ConstraintType::PrimaryKey ||
							   constr_type==ConstraintType::Exclude);

	info_frm->setVisible(constr_type==ConstraintType::PrimaryKey);

	deferrable_chk->setVisible(constr_type!=ConstraintType::Check);
	deferral_cmb->setVisible(constr_type!=ConstraintType::Check);
	deferral_lbl->setVisible(constr_type!=ConstraintType::Check);

	match_lbl->setVisible(constr_type==ConstraintType::ForeignKey);
	match_cmb->setVisible(constr_type==ConstraintType::ForeignKey);
	on_delete_cmb->setVisible(constr_type==ConstraintType::ForeignKey);
	on_delete_lbl->setVisible(constr_type==ConstraintType::ForeignKey);
	on_update_cmb->setVisible(constr_type==ConstraintType::ForeignKey);
	on_update_lbl->setVisible(constr_type==ConstraintType::ForeignKey);

	columns_tbw->setVisible(constr_type!=ConstraintType::Check &&
										 constr_type!=ConstraintType::Exclude);

	indexing_chk->setVisible(constr_type==ConstraintType::Exclude);
	indexing_cmb->setVisible(constr_type==ConstraintType::Exclude);

	if(constr_type!=ConstraintType::ForeignKey)
	{
		columns_tbw->setTabEnabled(1, false);
		columns_tbw->setCurrentIndex(0);
		ref_table_sel->clearSelector();
	}
	else
		columns_tbw->setTabEnabled(1, true);

	excl_elems_grp->setVisible(constr_type==ConstraintType::Exclude);
}

void ConstraintWidget::setAttributes(DatabaseModel *model, OperationList *op_list,  BaseObject *parent_obj, Constraint *constr)
{
	ObjectType obj_type;
	std::vector<ExcludeElement> excl_elems;

	if(!parent_obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	obj_type = parent_obj->getObjectType();

	if(!PhysicalTable::isPhysicalTable(obj_type) && obj_type!=ObjectType::Relationship)
		throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObjectWidget::setAttributes(model, op_list, constr, parent_obj);

	info_frm->setVisible(this->table != nullptr);
	ref_table_sel->setModel(model);
	col_picker_wgt->setParentObject(parent_obj);

	if(constr)
	{
		excl_elems = constr->getExcludeElements();

		indexing_chk->setChecked(constr->getIndexType()!=IndexingType::Null);
		indexing_cmb->setCurrentIndex(indexing_cmb->findText(~constr->getIndexType()));

		constr_type_cmb->setCurrentIndex(constr_type_cmb->findText(~constr->getConstraintType()));
		constr_type_cmb->setEnabled(false);
		constr_type_lbl->setEnabled(false);

		expression_txt->setPlainText(constr->getExpression());
		no_inherit_chk->setChecked(constr->isNoInherit());
		deferrable_chk->setChecked(constr->isDeferrable());
		nulls_not_distinct_chk->setChecked(constr->isNullsNotDistinct());
		deferral_cmb->setCurrentIndex(deferral_cmb->findText(~constr->getDeferralType()));
		match_cmb->setCurrentIndex(match_cmb->findText(~constr->getMatchType()));
		on_delete_cmb->setCurrentIndex(on_delete_cmb->findText(~constr->getActionType(Constraint::DeleteAction)));
		on_update_cmb->setCurrentIndex(on_update_cmb->findText(~constr->getActionType(Constraint::UpdateAction)));

		fill_factor_chk->setChecked(constr->getFillFactor()!=0);
		if(fill_factor_chk->isChecked())
			fill_factor_sb->setValue(constr->getFillFactor());

		ref_table_sel->setSelectedObject(constr->getReferencedTable());
		col_picker_wgt->setColumns(constr->getColumns(Constraint::SourceCols));
		ref_col_picker_wgt->setColumns(constr->getColumns(Constraint::ReferencedCols));
	}

	excl_elems_tab->setAttributes<ExcludeElement>(model, parent_obj);
	excl_elems_tab->setElements<ExcludeElement>(excl_elems);
}

void ConstraintWidget::applyConfiguration()
{
	try
	{
		Constraint *constr = nullptr;
		std::vector<ExcludeElement> excl_elems;

		startConfiguration<Constraint>();

		constr=dynamic_cast<Constraint *>(this->object);
		constr->setConstraintType(ConstraintType(constr_type_cmb->currentText()));
		constr->setExpression(expression_txt->toPlainText().toUtf8());

		if(fill_factor_chk->isChecked())
			constr->setFillFactor(fill_factor_sb->value());
		else
			constr->setFillFactor(0);

		constr->setMatchType(MatchType(match_cmb->currentText()));
		constr->setDeferrable(deferrable_chk->isChecked());
		constr->setDeferralType(DeferralType(deferral_cmb->currentText()));
		constr->setActionType(ActionType(on_delete_cmb->currentText()),Constraint::DeleteAction);
		constr->setActionType(ActionType(on_update_cmb->currentText()),Constraint::UpdateAction);
		constr->setNoInherit(no_inherit_chk->isChecked());
		constr->setNullsNotDistinct(nulls_not_distinct_chk->isChecked());

		if(indexing_chk->isChecked())
			constr->setIndexType(IndexingType(indexing_cmb->currentText()));
		else
			constr->setIndexType(IndexingType::Null);

		if(constr->getConstraintType()==ConstraintType::ForeignKey)
			constr->setReferencedTable(dynamic_cast<BaseTable *>(ref_table_sel->getSelectedObject()));

		constr->addColumns(col_picker_wgt->getColumns(), Constraint::SourceCols);
		constr->addColumns(ref_col_picker_wgt->getColumns(), Constraint::ReferencedCols);

		excl_elems_tab->getElements<ExcludeElement>(excl_elems);
		constr->addExcludeElements(excl_elems);

		//Raises an error if the user try to create a primary key that has columns added by relationship (not supported)
		if(constr->getConstraintType()==ConstraintType::PrimaryKey &&
				constr->isReferRelationshipAddedColumns())
			throw Exception(ErrorCode::UnsupportedPKColsAddedByRel,PGM_FUNC,PGM_FILE,PGM_LINE);

		BaseObjectWidget::applyConfiguration();

		/* Raises an error if the constraint type requires at least one column to be assinged and
		there is no columns configured on the form */
		if(((constr->getConstraintType()==ConstraintType::ForeignKey ||
			 constr->getConstraintType()==ConstraintType::PrimaryKey) &&
			constr->getColumnCount(Constraint::SourceCols)==0) ||
				(constr->getConstraintType()==ConstraintType::ForeignKey &&
				 constr->getColumnCount(Constraint::ReferencedCols)==0))
			throw Exception(ErrorCode::InvConstratintNoColumns,PGM_FUNC,PGM_FILE,PGM_LINE);

		finishConfiguration();

		//For the foreign keys and uniques, updates the fk relationships on the model
		if(this->table &&
			 (constr->getConstraintType()==ConstraintType::ForeignKey ||
			 constr->getConstraintType()==ConstraintType::Unique))
			this->model->updateTableFKRelationships(dynamic_cast<Table *>(this->table));
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

