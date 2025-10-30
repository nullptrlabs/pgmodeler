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

#include "textbox.h"

Textbox::Textbox()
{
	obj_type=ObjectType::Textbox;
	font_size = 11.0;
	text_width = 0;
	text_attributes[0]=text_attributes[1]=text_attributes[2]=false;
	attributes[Attributes::Italic]="";
	attributes[Attributes::Bold]="";
	attributes[Attributes::Underline]="";
	attributes[Attributes::Color]="";
	attributes[Attributes::FontSize]="";
}

QString Textbox::getSourceCode(SchemaParser::CodeType def_type)
{
	if(def_type==SchemaParser::SqlCode)
		return "";

	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	setPositionAttribute();
	setFadedOutAttribute();
	setLayersAttribute();

	if(text_attributes[ItalicText])
		attributes[Attributes::Italic]=Attributes::True;

	if(text_attributes[BoldText])
		attributes[Attributes::Bold]=Attributes::True;

	if(text_attributes[UnderlineText])
		attributes[Attributes::Underline]=Attributes::True;

	if(text_color.name()!="#000000")
		attributes[Attributes::Color]=text_color.name();

	attributes[Attributes::FontSize]=QString("%1").arg(font_size);
	attributes[Attributes::ZValue]=QString::number(z_value);
	attributes[Attributes::Width]=QString::number(text_width);

	return this->BaseObject::__getSourceCode(SchemaParser::XmlCode);

}

void Textbox::operator = (Textbox &txtbox)
{
	(*dynamic_cast<BaseGraphicObject *>(this))=reinterpret_cast<BaseGraphicObject &>(txtbox);
	this->comment=txtbox.comment;
	this->text_attributes[0]=txtbox.text_attributes[0];
	this->text_attributes[1]=txtbox.text_attributes[1];
	this->text_attributes[2]=txtbox.text_attributes[2];
	this->text_color=txtbox.text_color;
	this->font_size=txtbox.font_size;
	this->layers = txtbox.layers;
}

void Textbox::setTextAttribute(TextAttrib attrib, bool value)
{
	if(attrib > UnderlineText)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	text_attributes[attrib]=value;
}

void Textbox::setTextColor(const QColor &color)
{
	text_color=color;
}

QColor Textbox::getTextColor()
{
	return text_color;
}

bool Textbox::getTextAttribute(TextAttrib attrib)
{
	if(attrib > UnderlineText)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return text_attributes[attrib];
}

void Textbox::setFontSize(double size)
{
	font_size = (size <= 0 ? 1 : size);
}

void Textbox::setTextWidth(double width)
{
	setCodeInvalidated(text_width != width);
	text_width = width;

	if(text_width < 0)
		text_width = 0;
}

double Textbox::getFontSize()
{
	return font_size;
}

double Textbox::getTextWidth()
{
	return text_width;
}

void Textbox::setZValue(int z_value)
{
	setCodeInvalidated(this->z_value != z_value);
	BaseGraphicObject::setZValue(z_value);
}
