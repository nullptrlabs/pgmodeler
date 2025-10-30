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

#include "tag.h"

unsigned Tag::tag_id {4000};

Tag::Tag()
{
	obj_type=ObjectType::Tag;
	object_id=Tag::tag_id++;
	attributes[Attributes::Styles]="";

	for(auto &attr : getColorAttributes())
	{
		if(attr!=Attributes::TableName && attr!=Attributes::TableSchemaName)
			color_config[attr] = { QColor(0,0,0), QColor(0,0,0), QColor(0,0,0) };
		else
			color_config[attr] = { QColor(0,0,0) };
	}
}

void Tag::setName(const QString &name)
{
	if(name.isEmpty())
		throw Exception(ErrorCode::AsgEmptyNameObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(name.size() > BaseObject::ObjectNameMaxLength)
		throw Exception(ErrorCode::AsgLongNameObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->obj_name=name;
}

QString Tag::getName(bool, bool)
{
	return this->obj_name;
}

void Tag::setElementColor(const QString &elem_id, const QColor &color, ColorId color_id)
{
	try
	{
		validateElementId(elem_id, color_id);
		color_config[elem_id][enum_t(color_id)]=color;
		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Tag::setElementColors(const QString &elem_id, const QString &colors)
{
	try
	{
		QStringList color_lst=colors.split(',');
		unsigned color_id = enum_t(ColorId::FillColor1);

		for(auto &color : color_lst)
		{
			validateElementId(elem_id, static_cast<ColorId>(color_id));
			color_config[elem_id][color_id]=QColor(color);
			color_id++;
		}

		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QColor Tag::getElementColor(const QString &elem_id, ColorId color_id)
{
	try
	{
		validateElementId(elem_id, color_id);
		return color_config[elem_id][enum_t(color_id)];
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void Tag::validateElementId(const QString &id, ColorId color_id)
{
	if(color_config.count(id) == 0)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::OprInvalidElementId).arg(id),
										ErrorCode::OprInvalidElementId ,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if((color_id > ColorId::BorderColor) ||
		 (color_id > ColorId::FillColor1 &&
		 (id==Attributes::TableName || id==Attributes::TableSchemaName)))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::RefInvalidElementColorId).arg(id).arg(enum_t(color_id)),
										ErrorCode::RefInvalidElementColorId ,PGM_FUNC,PGM_FILE,PGM_LINE);
	}
}

QLinearGradient Tag::getFillStyle(const QString &elem_id)
{
	try
	{
		validateElementId(elem_id, ColorId::FillColor2);
		QLinearGradient grad(QPointF(0,0),QPointF(0,1));

		grad.setCoordinateMode(QGradient::ObjectBoundingMode);
		grad.setColorAt(0, color_config[elem_id][enum_t(ColorId::FillColor1)]);
		grad.setColorAt(1, color_config[elem_id][enum_t(ColorId::FillColor2)]);

		return grad;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString Tag::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

QString Tag::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	if(def_type == SchemaParser::SqlCode)
		return "";

	QString code_def = getCachedCode(def_type, reduced_form);

	if(!code_def.isEmpty())
		return code_def;

	try
	{
		attribs_map attribs;

		for(auto &itr : color_config)
		{
			attribs[Attributes::Id]=itr.first;
			attribs[Attributes::Colors]="";

			if(itr.first==Attributes::TableName || itr.first==Attributes::TableSchemaName)
				attribs[Attributes::Colors]=itr.second[enum_t(ColorId::FillColor1)].name();
			else
				attribs[Attributes::Colors]=itr.second[enum_t(ColorId::FillColor1)].name() + "," +
																		itr.second[enum_t(ColorId::FillColor2)].name() + "," +
																		itr.second[enum_t(ColorId::BorderColor)].name();

			attributes[Attributes::Styles]+=schparser.getSourceCode(Attributes::Style, attribs, SchemaParser::XmlCode);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}

	return BaseObject::getSourceCode(def_type, reduced_form);
}

QStringList Tag::getColorAttributes()
{
	static QStringList attribs={ Attributes::TableName,  Attributes::TableSchemaName,
															 Attributes::TableTitle, Attributes::TableBody,
															 Attributes::TableExtBody, Attributes::TableTogglerButtons,
															 Attributes::TableTogglerBody };

	return attribs;
}

void Tag::operator = (Tag &tag)
{
	(*dynamic_cast<BaseObject *>(this))=dynamic_cast<BaseObject &>(tag);

	for(auto &attr : tag.color_config)
		this->color_config[attr.first]=attr.second;
}
