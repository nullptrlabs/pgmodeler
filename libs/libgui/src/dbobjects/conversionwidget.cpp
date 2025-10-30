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

#include "conversionwidget.h"

ConversionWidget::ConversionWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Conversion)
{
	QFrame *frame=nullptr;
	Ui_ConversionWidget::setupUi(this);

	conv_func_sel=nullptr;
	conv_func_sel=new ObjectSelectorWidget(ObjectType::Function, this);
	convcod_grid->addWidget(conv_func_sel,1,1,1,3);

	setRequiredField(src_encoding_lbl);
	setRequiredField(trg_encoding_lbl);
	setRequiredField(conv_func_lbl);
	setRequiredField(conv_func_sel);

	configureFormLayout(convcod_grid, ObjectType::Conversion);
	frame=generateInformationFrame(tr("The function to be assigned to an encoding conversion must have the following signature: <em>void function(integer, integer, cstring, internal, integer)</em>."));
	convcod_grid->addItem(new QSpacerItem(10,10,QSizePolicy::Minimum,QSizePolicy::Expanding), convcod_grid->count()+1, 0, 1, 0);
	convcod_grid->addWidget(frame, convcod_grid->count()+1, 0, 1, 0);
	frame->setParent(this);

	src_encoding_cmb->addItems(EncodingType::getTypes());
	trg_encoding_cmb->addItems(EncodingType::getTypes());

	configureTabOrder({ src_encoding_cmb, trg_encoding_cmb, conv_func_sel });

	setMinimumSize(500, 300);
}

void ConversionWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Conversion *conv)
{
	BaseObjectWidget::setAttributes(model, op_list, conv, schema);
	conv_func_sel->setModel(model);

	if(conv)
	{
		conv_func_sel->setSelectedObject(conv->getConversionFunction());
		default_conv_chk->setChecked(conv->isDefault());
		src_encoding_cmb->setCurrentIndex(trg_encoding_cmb->findText(~(conv->getEncoding(Conversion::SrcEncoding))));
		trg_encoding_cmb->setCurrentIndex(trg_encoding_cmb->findText(~(conv->getEncoding(Conversion::DstEncoding))));
	}
}

void ConversionWidget::applyConfiguration()
{
	try
	{
		Conversion *conv=nullptr;

		startConfiguration<Conversion>();
		conv=dynamic_cast<Conversion *>(this->object);

		BaseObjectWidget::applyConfiguration();

		conv->setEncoding(Conversion::SrcEncoding, src_encoding_cmb->currentText());
		conv->setEncoding(Conversion::DstEncoding, trg_encoding_cmb->currentText());
		conv->setDefault(default_conv_chk->isChecked());
		conv->setConversionFunction(dynamic_cast<Function*>(conv_func_sel->getSelectedObject()));

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

