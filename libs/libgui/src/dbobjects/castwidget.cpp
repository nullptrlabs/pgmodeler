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

CastWidget::CastWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Cast)
{
	QFrame *frame=nullptr;
	QSpacerItem *spacer=new QSpacerItem(10,1,QSizePolicy::Fixed,QSizePolicy::Expanding);

	Ui_CastWidget::setupUi(this);

	src_datatype=new PgSQLTypeWidget(this, tr("Source data type"));
	trg_datatype=new PgSQLTypeWidget(this, tr("Target data type"));
	conv_func_sel=new ObjectSelectorWidget(ObjectType::Function, this);

	cast_grid->addWidget(conv_func_sel,1,1,1,4);
	cast_grid->addWidget(src_datatype,2,0,1,5);
	cast_grid->addWidget(trg_datatype,3,0,1,5);

	configureFormLayout(cast_grid, ObjectType::Cast);

	frame=generateInformationFrame(tr("The function to be assigned to a cast from <em><strong>typeA</strong></em> to <em><strong>typeB</strong></em> must have the following signature: <em><strong>typeB</strong> function(<strong>typeA</strong>, integer, boolean)</em>."));
	cast_grid->addItem(spacer, cast_grid->count()+1, 0, 1, 0);
	cast_grid->addWidget(frame, cast_grid->count()+1, 0, 1, 0);
	frame->setParent(this);

	setRequiredField(src_datatype);
	setRequiredField(trg_datatype);

	configureTabOrder({ explicit_rb, implicit_rb, assignment_rb, input_output_chk,
											 conv_func_sel, src_datatype, trg_datatype });

	setMinimumSize(520, 460);
}

void CastWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Cast *cast)
{
	PgSqlType src_type, trg_type;

	BaseObjectWidget::setAttributes(model, op_list, cast);
	conv_func_sel->setModel(model);

	if(cast)
	{
		src_type=cast->getDataType(Cast::SrcType);
		trg_type=cast->getDataType(Cast::DstType);

		conv_func_sel->setSelectedObject(cast->getCastFunction());
		input_output_chk->setChecked(cast->isInOut());
		explicit_rb->setChecked(cast->getCastType()==Cast::Explicit);
		implicit_rb->setChecked(cast->getCastType()==Cast::Implicit);
		assignment_rb->setChecked(cast->getCastType()==Cast::Assignment);
	}

	src_datatype->setAttributes(src_type, model, false);
	trg_datatype->setAttributes(trg_type, model, false);
}

void CastWidget::applyConfiguration()
{
	try
	{
		Cast *cast=nullptr;

		startConfiguration<Cast>();

		cast=dynamic_cast<Cast *>(this->object);
		cast->setDataType(Cast::SrcType, src_datatype->getPgSQLType());
		cast->setDataType(Cast::DstType, trg_datatype->getPgSQLType());
		cast->setInOut(input_output_chk->isChecked());

		if(implicit_rb->isChecked())
			cast->setCastType(Cast::Implicit);
		else if(assignment_rb->isChecked())
			cast->setCastType(Cast::Assignment);
		else
			cast->setCastType(Cast::Explicit);

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

