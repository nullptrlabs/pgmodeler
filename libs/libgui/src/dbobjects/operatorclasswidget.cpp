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

#include "operatorclasswidget.h"
#include "guiutilsns.h"

OperatorClassWidget::OperatorClassWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::OpClass)
{
	QGridLayout *grid=nullptr;
	Ui_OperatorClassWidget::setupUi(this);

	family_sel=new ObjectSelectorWidget(ObjectType::OpFamily, this);
	data_type=new PgSQLTypeWidget(this);
	operator_sel=new ObjectSelectorWidget(ObjectType::Operator, this);
	elem_family_sel=new ObjectSelectorWidget(ObjectType::OpFamily, this);
	function_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	storage_type=new PgSQLTypeWidget(this, tr("Storage Type"));
	elements_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^ CustomTableWidget::DuplicateButton, true, this);

	elements_tab->setColumnCount(4);
	elements_tab->setHeaderLabel(tr("Object"),0);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("table")),0);

	elements_tab->setHeaderLabel(tr("Type"),1);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);

	elements_tab->setHeaderLabel(tr("Support/Strategy"),2);
	elements_tab->setHeaderLabel(tr("Operator Family"),3);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("opfamily")),3);

	grid=new QGridLayout;
	grid->setContentsMargins(0,0,0,0);
	grid->addWidget(def_class_lbl,0,2,1,1);
	grid->addWidget(def_class_chk,0,3,1,1);
	grid->addWidget(indexing_lbl,0,0,1,1);
	grid->addWidget(indexing_cmb,0,1,1,1);
	grid->addWidget(family_lbl,2,0,1,1);
	grid->addWidget(family_sel,2,1,1,4);
	grid->addWidget(data_type,4,0,1,5);
	grid->addWidget(elements_grp,5,0,1,5);
	this->setLayout(grid);
	configureFormLayout(grid, ObjectType::OpClass);

	grid=dynamic_cast<QGridLayout *>(elements_grp->layout());
	grid->addWidget(function_sel, 1,1,1,4);
	grid->addWidget(operator_sel, 2,1,1,4);
	grid->addWidget(elem_family_sel, 3,1,1,4);
	grid->addWidget(storage_type, 5,0,1,5);
	grid->addWidget(elements_tab, 6,0,1,4);

	connect(elem_type_cmb, &QComboBox::currentIndexChanged, this, &OperatorClassWidget::selectElementType);
	connect(elements_tab, &CustomTableWidget::s_rowAdded, this, __slot_n(this, OperatorClassWidget::handleElement));
	connect(elements_tab, &CustomTableWidget::s_rowUpdated, this, __slot_n(this, OperatorClassWidget::handleElement));
	connect(elements_tab, &CustomTableWidget::s_rowEdited, this, &OperatorClassWidget::editElement);

	selectElementType(0);
	indexing_cmb->addItems(IndexingType::getTypes());

	setRequiredField(elements_grp);
	configureTabOrder({ indexing_cmb, def_class_chk , family_sel, data_type, elem_type_cmb,
											operator_sel, elem_family_sel, function_sel, stg_num_sb, storage_type,
											elements_tab });

	setMinimumSize(640, 730);
}

void OperatorClassWidget::selectElementType(int elem_type)
{
	unsigned sel_idx=static_cast<unsigned>(elem_type);

	function_lbl->setVisible(sel_idx==OperatorClassElement::FunctionElem);
	function_sel->setVisible(sel_idx==OperatorClassElement::FunctionElem);

	operator_lbl->setVisible(sel_idx==OperatorClassElement::OperatorElem);
	operator_sel->setVisible(sel_idx==OperatorClassElement::OperatorElem);
	elem_family_lbl->setVisible(sel_idx==OperatorClassElement::OperatorElem);
	elem_family_sel->setVisible(sel_idx==OperatorClassElement::OperatorElem);

	storage_type->setVisible(sel_idx==OperatorClassElement::StorageElem);
	stg_num_lbl->setVisible(sel_idx!=OperatorClassElement::StorageElem);
	stg_num_sb->setVisible(sel_idx!=OperatorClassElement::StorageElem);
}

void OperatorClassWidget::editElement(int lin_idx)
{
	OperatorClassElement elem;

	//Get the element from the selected line
	elem=elements_tab->getRowData(lin_idx).value<OperatorClassElement>();

	elem_type_cmb->setCurrentIndex(elem.getElementType());
	selectElementType(elem_type_cmb->currentIndex());

	function_sel->setSelectedObject(elem.getFunction());
	operator_sel->setSelectedObject(elem.getOperator());
	stg_num_sb->setValue(elem.getStrategyNumber());
	storage_type->setAttributes(elem.getStorage(), this->model, false);
}

void OperatorClassWidget::showElementData(OperatorClassElement elem, int lin_idx)
{
	unsigned elem_type;

	elem_type=elem.getElementType();

	if(elem_type==OperatorClassElement::FunctionElem)
	{
		elements_tab->setCellText(elem.getFunction()->getSignature(), lin_idx, 0);
		elements_tab->setCellText(elem.getFunction()->getTypeName(), lin_idx, 1);
	}
	else if(elem_type==OperatorClassElement::OperatorElem)
	{
		elements_tab->setCellText(elem.getOperator()->getSignature(), lin_idx, 0);
		elements_tab->setCellText(elem.getOperator()->getTypeName(), lin_idx, 1);
	}
	else
	{
		elements_tab->setCellText(*elem.getStorage(), lin_idx, 0);
		elements_tab->setCellText(BaseObject::getTypeName(ObjectType::Type), lin_idx, 1);
	}

	if(elem_type!=OperatorClassElement::StorageElem)
		elements_tab->setCellText(QString("%1").arg(elem.getStrategyNumber()), lin_idx, 2);
	else
		elements_tab->setCellText(" ", lin_idx, 2);

	if(elem_type==OperatorClassElement::OperatorElem && elem.getOperatorFamily())
		elements_tab->setCellText(elem.getOperatorFamily()->getName(true), lin_idx, 3);
	else
		elements_tab->clearCellText(lin_idx, 3);

	//Define as the line data the element itself
	elements_tab->setRowData(QVariant::fromValue<OperatorClassElement>(elem), lin_idx);
}

void OperatorClassWidget::handleElement(int lin_idx)
{
	OperatorClassElement elem;
	unsigned elem_type;

	elem_type=elem_type_cmb->currentIndex();

	try
	{
		if(elem_type==OperatorClassElement::FunctionElem)
			elem.setFunction(dynamic_cast<Function *>(function_sel->getSelectedObject()), stg_num_sb->value());
		else  if(elem_type==OperatorClassElement::OperatorElem)
		{
			elem.setOperator(dynamic_cast<Operator *>(operator_sel->getSelectedObject()), stg_num_sb->value());
			elem.setOperatorFamily(dynamic_cast<OperatorFamily *>(elem_family_sel->getSelectedObject()));
		}
		else
			elem.setStorage(storage_type->getPgSQLType());

		showElementData(elem, lin_idx);

		function_sel->clearSelector();
		operator_sel->clearSelector();
		stg_num_sb->setValue(1);
		elements_tab->clearSelection();
	}
	catch(Exception &e)
	{
		//In case of error removes the recently added table line
		if(elements_tab->getCellText(lin_idx, 0).isEmpty())
			elements_tab->removeRow(lin_idx);

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void OperatorClassWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, OperatorClass *op_class)
{
	PgSqlType type;
	unsigned i, count;

	BaseObjectWidget::setAttributes(model, op_list, op_class, schema);

	family_sel->setModel(model);
	function_sel->setModel(model);
	operator_sel->setModel(model);
	elem_family_sel->setModel(model);
	storage_type->setAttributes(type, model, false);

	if(op_class)
	{
		type=op_class->getDataType();
		family_sel->setSelectedObject(op_class->getFamily());
		def_class_chk->setChecked(op_class->isDefault());
		indexing_cmb->setCurrentIndex(indexing_cmb->findText(~(op_class->getIndexingType())));

		elements_tab->blockSignals(true);
		count=op_class->getElementCount();
		for(i=0; i < count; i++)
		{
			elements_tab->addRow();
			showElementData(op_class->getElement(i), i);
		}
		elements_tab->blockSignals(false);
		elements_tab->clearSelection();
	}

	data_type->setAttributes(type, model, false);
}

void OperatorClassWidget::applyConfiguration()
{
	try
	{
		OperatorClass *op_class=nullptr;
		unsigned i, count;

		startConfiguration<OperatorClass>();

		op_class=dynamic_cast<OperatorClass *>(this->object);
		op_class->setDefault(op_class->isDefault());
		op_class->setFamily(dynamic_cast<OperatorFamily *>(family_sel->getSelectedObject()));
		op_class->setIndexingType(IndexingType(indexing_cmb->currentText()));
		op_class->setDataType(data_type->getPgSQLType());

		op_class->removeElements();
		count=elements_tab->getRowCount();

		for(i=0; i < count; i++)
			op_class->addElement(elements_tab->getRowData(i).value<OperatorClassElement>());

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

