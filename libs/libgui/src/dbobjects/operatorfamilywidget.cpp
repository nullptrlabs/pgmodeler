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

#include "operatorfamilywidget.h"

OperatorFamilyWidget::OperatorFamilyWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::OpFamily)
{
	Ui_OperatorFamilyWidget::setupUi(this);
	configureFormLayout(opfamily_grid, ObjectType::OpFamily);

	indexing_cmb->addItems(IndexingType::getTypes());

	setRequiredField(indexing_lbl);	
	opfamily_grid->addItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding), opfamily_grid->count()+1, 0, 1, 0);

	configureTabOrder();
	setMinimumSize(500, 290);
}

void OperatorFamilyWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, OperatorFamily *op_family)
{
	BaseObjectWidget::setAttributes(model, op_list, op_family, schema);

	if(op_family)
		indexing_cmb->setCurrentIndex(indexing_cmb->findText(~(op_family->getIndexingType())));
}

void OperatorFamilyWidget::applyConfiguration()
{
	try
	{
		OperatorFamily *op_family=nullptr;

		startConfiguration<OperatorFamily>();
		op_family=dynamic_cast<OperatorFamily *>(this->object);
		op_family->setIndexingType(IndexingType(indexing_cmb->currentText()));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

