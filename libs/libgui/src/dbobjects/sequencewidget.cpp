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

#include "sequencewidget.h"

SequenceWidget::SequenceWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Sequence)
{
	Ui_SequenceWidget::setupUi(this);

	column_sel=nullptr;
	column_sel=new ObjectSelectorWidget(ObjectType::Column, this);
	sequence_grid->addWidget(column_sel, 4, 1, 1, 3);

	configureFormLayout(sequence_grid, ObjectType::Sequence);
	sequence_grid->addItem(new QSpacerItem(10,0,QSizePolicy::Minimum,QSizePolicy::Expanding), sequence_grid->count(), 0);
	configureTabOrder();

	def_values_cmb->addItem(tr("User defined"));
	def_values_cmb->addItem("smallserial");
	def_values_cmb->addItem("serial");
	def_values_cmb->addItem("bigserial");
	setMinimumSize(520, 350);

	connect(def_values_cmb, &QComboBox::currentIndexChanged, this, &SequenceWidget::setDefaultValues);
}

void SequenceWidget::setAttributesReadonly(DatabaseModel *model, OperationList *op_list, Schema *schema, Sequence *sequence, Column *column)
{
	setAttributes(model, sequence);
	BaseObjectWidget::setAttributes(model, op_list, sequence, schema, DNaN, DNaN, false);

	name_edt->setReadOnly(true);
	comment_edt->setEnabled(false);
	schema_sel->setEnabled(false);
	append_sql_tb->setEnabled(false);
	edt_perms_tb->setEnabled(false);
	column_sel->setSelectedObject(column);
	column_sel->setEnabled(false);
	disable_sql_chk->setEnabled(false);
	owner_sel->setSelectedObject(column && column->getParentTable() ?
															 column->getParentTable()->getOwner() : nullptr);
	owner_sel->setEnabled(false);
}

void SequenceWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Sequence *sequence)
{
	setAttributes(model, sequence);
	BaseObjectWidget::setAttributes(model, op_list, sequence, schema);
}

void SequenceWidget::setAttributes(DatabaseModel *model, Sequence *sequence)
{
	column_sel->setModel(model);

	if(sequence)
	{
		column_sel->setSelectedObject(sequence->getOwnerColumn());
		cyclic_chk->setChecked(sequence->isCycle());
		start_edt->setText(sequence->getStart());
		maximum_edt->setText(sequence->getMaxValue());
		minimum_edt->setText(sequence->getMinValue());
		cache_edt->setText(sequence->getCache());
		increment_edt->setText(sequence->getIncrement());
	}
	else
	{
		cyclic_chk->setChecked(false);
		start_edt->setText("1");
		maximum_edt->setText(Sequence::MaxPositiveValue);
		minimum_edt->setText("0");
		cache_edt->setText("1");
		increment_edt->setText("1");
	}
}

void SequenceWidget::applyConfiguration()
{
	try
	{
		Sequence *sequence=nullptr;

		startConfiguration<Sequence>();
		sequence=dynamic_cast<Sequence *>(this->object);

		BaseObjectWidget::applyConfiguration();

		sequence->setCycle(cyclic_chk->isChecked());
		sequence->setValues(minimum_edt->text(), maximum_edt->text(), increment_edt->text(),
							start_edt->text(), cache_edt->text());
		sequence->setOwnerColumn(dynamic_cast<Column *>(column_sel->getSelectedObject()));
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void SequenceWidget::setDefaultValues()
{
	Sequence *seq = dynamic_cast<Sequence *>(this->object);

	if(seq && def_values_cmb->currentIndex() == 0)
	{
		start_edt->setText(seq->getStart());
		cache_edt->setText(seq->getCache());
		minimum_edt->setText(seq->getMinValue());
		maximum_edt->setText(seq->getMaxValue());
		increment_edt->setText(seq->getIncrement());
	}
	else
	{
		QString min = "0", max = Sequence::MaxPositiveValue;

		if(def_values_cmb->currentText() == "smallserial")
			max = Sequence::MaxSmallPositiveValue;
		else if(def_values_cmb->currentText() == "bigserial")
			max = Sequence::MaxBigPositiveValue;

		start_edt->setText("1");
		cache_edt->setText("1");
		increment_edt->setText("1");
		minimum_edt->setText(min);
		maximum_edt->setText(max);
	}
}

