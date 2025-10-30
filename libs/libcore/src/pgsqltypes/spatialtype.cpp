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

#include "spatialtype.h"

const QStringList SpatialType::type_names {
	"", // Reserved for Class::Null
	"POINT",	"LINESTRING",
	"POLYGON",	"MULTIPOINT",
	"MULTILINESTRING",	"MULTIPOLYGON",
	"GEOMETRY",	"GEOMETRYCOLLECTION",
	"POLYHEDRALSURFACE",	"TRIANGLE",
	"TIN",	"CIRCULARSTRING",
	"COMPOUNDCURVE",	"CURVEPOLYGON",
	"MULTICURVE",	"MULTISURFACE",
};

SpatialType::SpatialType(const QString &type_name, int srid, VariationId variation_id)
{
	QString name=type_name;

	if(name.endsWith("ZM"))
	{
		variation_id=VarZm;
		name.remove("ZM");
	}
	else if(name.endsWith("M"))
	{
		variation_id=VarM;
		name.remove("M");
	}
	else if(name.endsWith("Z"))
	{
		variation_id=VarZ;
		name.remove("Z");
	}

	setType(name);
	setVariation(variation_id);
	setSRID(srid);
}

SpatialType::SpatialType(unsigned type_id, int srid, VariationId var_id)
{
	setType(type_id);
	setVariation(var_id);
	setSRID(srid);
}

SpatialType::SpatialType()
{
	type_idx=Null;
	variation=NoVar;
	srid=0;
}

void SpatialType::setVariation(VariationId var)
{
	if(var > VarZm)
		variation=VarZm;
	else
		variation=var;
}

SpatialType::VariationId SpatialType::getVariation()
{
	return variation;
}

void SpatialType::setSRID(int srid)
{
	if(srid < -1) srid=-1;
	this->srid=srid;
}

int SpatialType::getSRID()
{
	return srid;
}

QString SpatialType::operator * ()
{
	if(type_idx != Null)
	{
		QString var_str;

		switch(variation)
		{
			case VarZ: var_str+="Z"; break;
			case VarM: var_str+="M"; break;
			case VarZm: var_str+="ZM"; break;
			default: var_str=""; break;
		}

		if(srid > 0)
			return (QString("(%1%2, %3)").arg(type_names[type_idx]).arg(var_str)).arg(srid);

		return (QString("(%1%2)").arg(type_names[type_idx]).arg(var_str));
	}

	return "";
}

QStringList SpatialType::getTypes()
{
	return TemplateType<SpatialType>::getTypes(type_names);
}

unsigned SpatialType::setType(unsigned type_id)
{
	return TemplateType<SpatialType>::setType(type_id, type_names);
}

unsigned SpatialType::setType(const QString &type_name)
{
	return TemplateType<SpatialType>::setType(type_name, type_names);
}

QString SpatialType::getTypeName(unsigned type_id)
{
	return TemplateType<SpatialType>::getTypeName(type_id, type_names);
}
