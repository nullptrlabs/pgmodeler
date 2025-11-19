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
	Ui_ConversionWidget::setupUi(this);

	conv_func_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	conv_func_lt->addWidget(conv_func_sel);

	setRequiredField(src_encoding_lbl);
	setRequiredField(trg_encoding_lbl);
	setRequiredField(conv_func_lbl);
	setRequiredField(conv_func_sel);

	src_encoding_cmb->addItems(EncodingType::getTypes());
	trg_encoding_cmb->addItems(EncodingType::getTypes());

	configureTabbedLayout(false);
	configureTabOrder({ src_encoding_cmb, trg_encoding_cmb, conv_func_sel });
	setMinimumSize(600, 450);
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

