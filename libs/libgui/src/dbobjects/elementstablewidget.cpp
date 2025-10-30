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

#include "dbobjects/elementstablewidget.h"
#include "settings/generalconfigwidget.h"
#include "guiutilsns.h"

ElementsTableWidget::ElementsTableWidget(QWidget *parent) : QWidget(parent)
{
	handled_elem = nullptr;
	model = nullptr;
	parent_obj = nullptr;

	element_wgt = new ElementWidget;
	element_form.setMainWidget(element_wgt);
	element_form.setButtonConfiguration();
	connect(&element_form, &BaseForm::accepted, element_wgt, &ElementWidget::applyConfiguration);

	QVBoxLayout *vbox = new QVBoxLayout(this);
	elements_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
																			(CustomTableWidget::UpdateButton | CustomTableWidget::DuplicateButton), true, this);

	elements_tab->setColumnCount(7);
	elements_tab->setHeaderLabel(tr("Element"), 0);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("column")),0);
	elements_tab->setHeaderLabel(tr("Type"), 1);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);
	elements_tab->setHeaderLabel(tr("Operator"), 2);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("operator")),2);
	elements_tab->setHeaderLabel(tr("Operator Class"), 3);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("opclass")),3);
	elements_tab->setHeaderLabel(tr("Collation"), 4);
	elements_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("collation")),4);
	elements_tab->setHeaderLabel(tr("Sorting"), 5);
	elements_tab->setHeaderLabel(tr("Nulls"), 6);

	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(elements_tab);

	connect(elements_tab, &CustomTableWidget::s_rowAdded, this, &ElementsTableWidget::addElement);
	connect(elements_tab, &CustomTableWidget::s_rowEdited, this, &ElementsTableWidget::editElement);
}

ElementsTableWidget::~ElementsTableWidget()
{
	delete handled_elem;
}

void ElementsTableWidget::showElementData(Element *elem, int elem_idx)
{
	if(!elem)
		return;

	if(elem->getColumn())
	{
		elements_tab->setCellText(elem->getColumn()->getName(), elem_idx, 0);
		elements_tab->setCellText(elem->getColumn()->getTypeName(), elem_idx, 1);
	}
	else if(elem->getSimpleColumn().isValid())
	{
		elements_tab->setCellText(elem->getSimpleColumn().getName(), elem_idx, 0);
		elements_tab->setCellText(BaseObject::getTypeName(ObjectType::Column), elem_idx, 1);
	}
	else
	{
		elements_tab->setCellText(elem->getExpression(), elem_idx, 0);
		elements_tab->setCellText(tr("Expression"), elem_idx, 1);
	}

	elements_tab->clearCellText(elem_idx, 2);
	if(elem->getOperator())
		elements_tab->setCellText(elem->getOperator()->getSignature(true), elem_idx, 2);

	elements_tab->clearCellText(elem_idx, 3);
	if(elem->getOperatorClass())
		elements_tab->setCellText(elem->getOperatorClass()->getName(true), elem_idx, 3);

	elements_tab->clearCellText(elem_idx, 4);
	if(elem->getCollation())
		elements_tab->setCellText(elem->getCollation()->getName(true), elem_idx, 4);

	if(elem->isSortingEnabled())
	{
		if(elem->getSortingAttribute(IndexElement::AscOrder))
			elements_tab->setCellText(tr("Ascending"), elem_idx, 5);
		else
			elements_tab->setCellText(tr("Descending"), elem_idx, 5);

		if(elem->getSortingAttribute(IndexElement::NullsFirst))
			elements_tab->setCellText(tr("First"), elem_idx, 6);
		else
			elements_tab->setCellText(tr("Last"), elem_idx, 6);
	}
	else
	{
		elements_tab->setCellText(tr("Default"), elem_idx, 5);
		elements_tab->setCellText(tr("Default"), elem_idx, 6);
	}

	elements_tab->setRowData(copyElementData(elem), elem_idx);
}

QVariant ElementsTableWidget::copyElementData(Element *elem)
{
	if(dynamic_cast<IndexElement *>(elem))
		return QVariant::fromValue<IndexElement>(*dynamic_cast<IndexElement *>(elem));

	if(dynamic_cast<ExcludeElement *>(elem))
		return QVariant::fromValue<ExcludeElement>(*dynamic_cast<ExcludeElement *>(elem));

	if(dynamic_cast<PartitionKey *>(elem))
		return QVariant::fromValue<PartitionKey>(*dynamic_cast<PartitionKey *>(elem));

	return {};
}

int ElementsTableWidget::openElementForm(Element *elem)
{
	int res = 0;

	GeneralConfigWidget::restoreWidgetGeometry(&element_form, element_wgt->metaObject()->className());
	element_wgt->setAttributes(model, parent_obj, elem);
	element_form.setWindowTitle(element_wgt->windowTitle());
	res = element_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&element_form, element_wgt->metaObject()->className());

	return res;
}

void ElementsTableWidget::editElement(int elem_idx)
{
	QVariant data = elements_tab->getRowData(elem_idx);
	Element *elem = nullptr;
	IndexElement idx_elem;
	ExcludeElement exc_elem;
	PartitionKey part_key;
	int res = 0;

	if(data.canConvert<IndexElement>())
	{
		idx_elem = data.value<IndexElement>();
		elem = &idx_elem;
	}

	if(data.canConvert<ExcludeElement>())
	{
		exc_elem = data.value<ExcludeElement>();
		elem = &exc_elem;
	}

	if(data.canConvert<PartitionKey>())
	{
		part_key = data.value<PartitionKey>();
		elem = &part_key;
	}

	res = openElementForm(elem);

	if(elem && res == QDialog::Accepted)
		showElementData(element_wgt->getElement(), elem_idx);
}

void ElementsTableWidget::addElement(int elem_idx)
{
	if(openElementForm(handled_elem) == QDialog::Accepted)
		showElementData(element_wgt->getElement(), elem_idx);
	else
		elements_tab->removeRow(elem_idx);
}
