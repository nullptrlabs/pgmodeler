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

#include "textboxwidget.h"
#include <QColorDialog>

TextboxWidget::TextboxWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Textbox)
{
	Ui_TextboxWidget::setupUi(this);
	configureFormLayout(textbox_grid, ObjectType::Textbox);

	text_txt->removeEventFilter(this);
	connect(color_select_tb, &QToolButton::clicked, this, &TextboxWidget::selectTextColor);

	setMinimumSize(500, 200);
}

void TextboxWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Textbox *txtbox, double obj_px, double obj_py)
{
	if(txtbox)
	{
		QPalette palette;
		palette.setColor(QPalette::Button, txtbox->getTextColor());
		color_select_tb->setPalette(palette);

		text_txt->setPlainText(txtbox->getComment());
		bold_chk->setChecked(txtbox->getTextAttribute(Textbox::BoldText));
		italic_chk->setChecked(txtbox->getTextAttribute(Textbox::ItalicText));
		underline_chk->setChecked(txtbox->getTextAttribute(Textbox::UnderlineText));
		font_size_sb->setValue(txtbox->getFontSize());
	}

	BaseObjectWidget::setAttributes(model, op_list, txtbox, nullptr, obj_px, obj_py);
}

void TextboxWidget::selectTextColor()
{
	QColorDialog color_dlg;
	QPalette palette;

	color_dlg.setWindowTitle(tr("Select text color"));
	color_dlg.setCurrentColor(color_select_tb->palette().color(QPalette::Button));
	color_dlg.exec();

	if(color_dlg.result()==QDialog::Accepted)
	{
		palette.setColor(QPalette::Button, color_dlg.selectedColor());
		color_select_tb->setPalette(palette);
	}
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
		txtbox->setTextColor(color_select_tb->palette().color(QPalette::Button));
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

