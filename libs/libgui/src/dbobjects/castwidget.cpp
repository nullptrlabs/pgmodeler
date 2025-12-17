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

#include "castwidget.h"
#include "guiutilsns.h"

CastWidget::CastWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Cast)
{
	Ui_CastWidget::setupUi(this);

	src_datatype = new PgSQLTypeWidget(this, tr("Source data type"));
	trg_datatype = new PgSQLTypeWidget(this, tr("Target data type"));
	conv_func_sel = GuiUtilsNs::createWidgetInParent<ObjectSelectorWidget>(GuiUtilsNs::LtMargin,
																																				 ObjectType::Function, conv_func_gb);
	data_types_lt->addWidget(src_datatype);
	data_types_lt->addWidget(trg_datatype);

	configureTabbedLayout(false);
	cast_lt->addItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::MinimumExpanding));

	cast_type_cmb->addItem(tr("Implict"), Cast::Implicit);
	cast_type_cmb->addItem(tr("Explict"), Cast::Explicit);
	cast_type_cmb->addItem(tr("Assignment"), Cast::Assignment);

	setRequiredField(src_datatype);
	setRequiredField(trg_datatype);

	configureTabOrder({ cast_type_cmb, input_output_chk,
											 conv_func_sel, src_datatype, trg_datatype });

	setMinimumSize(650, 500);
}

void CastWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Cast *cast)
{
	PgSqlType src_type, trg_type;

	BaseObjectWidget::setAttributes(model, op_list, cast);
	conv_func_sel->setModel(model);

	if(cast)
	{
		src_type = cast->getDataType(Cast::SrcType);
		trg_type = cast->getDataType(Cast::DstType);

		conv_func_sel->setSelectedObject(cast->getCastFunction());
		input_output_chk->setChecked(cast->isInOut());
		cast_type_cmb->setCurrentIndex(cast_type_cmb->findData(cast->getCastType()));
	}

	src_datatype->setAttributes(src_type, model, false);
	trg_datatype->setAttributes(trg_type, model, false);
}

void CastWidget::applyConfiguration()
{
	try
	{
		Cast *cast = nullptr;

		startConfiguration<Cast>();

		cast = dynamic_cast<Cast *>(this->object);
		cast->setDataType(Cast::SrcType, src_datatype->getPgSQLType());
		cast->setDataType(Cast::DstType, trg_datatype->getPgSQLType());
		cast->setInOut(input_output_chk->isChecked());

		cast->setCastType(static_cast<Cast::CastType>(cast_type_cmb->currentData().toInt()));
		cast->setCastFunction(dynamic_cast<Function*>(conv_func_sel->getSelectedObject()));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

