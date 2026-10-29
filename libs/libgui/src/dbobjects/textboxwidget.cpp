/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

#include "textboxwidget.h"

TextboxWidget::TextboxWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Textbox)
{
	Ui_TextboxWidget::setupUi(this);

	text_txt->removeEventFilter(this);

	text_color_cp = new ColorPickerWidget(1, this);
	text_color_lt->addWidget(text_color_cp);

	configureTabbedLayout(false);
	setMinimumSize(550, 350);
}

void TextboxWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Textbox *txtbox, double obj_px, double obj_py)
{
	if(txtbox)
	{
		text_color_cp->setColor(0, txtbox->getTextColor());
		text_txt->setPlainText(txtbox->getComment());
		bold_chk->setChecked(txtbox->getTextAttribute(Textbox::BoldText));
		italic_chk->setChecked(txtbox->getTextAttribute(Textbox::ItalicText));
		underline_chk->setChecked(txtbox->getTextAttribute(Textbox::UnderlineText));
		font_size_sb->setValue(txtbox->getFontSize());
	}

	BaseObjectWidget::setAttributes(model, op_list, txtbox, nullptr, obj_px, obj_py);
}

void TextboxWidget::applyConfiguration()
{
	try
	{
		Textbox *txtbox=nullptr;

		startConfiguration<Textbox>();

		txtbox=dynamic_cast<Textbox *>(this->object);
		txtbox->setComment(text_txt->toPlainText().toUtf8());
		txtbox->setTextAttribute(Textbox::ItalicText, italic_chk->isChecked());
		txtbox->setTextAttribute(Textbox::BoldText, bold_chk->isChecked());
		txtbox->setTextAttribute(Textbox::UnderlineText, underline_chk->isChecked());
		txtbox->setTextColor(text_color_cp->getColor(0));
		txtbox->setFontSize(font_size_sb->value());

		if(txtbox->getTextWidth() <= 0)
			txtbox->setTextWidth(text_txt->viewport()->width());

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

