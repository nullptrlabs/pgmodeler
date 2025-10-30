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

#include "element.h"

Element::Element()
{
	parent_obj = nullptr;
	column=nullptr;
	operator_class=nullptr;
	sorting_attibs[NullsFirst]=false;
	sorting_attibs[AscOrder]=true;
	sorting_enabled=false;
}

void Element::setColumn(Column *column)
{
	if(!column)
		return;

	this->column=column;
	expression="";
	simple_col = SimpleColumn();
}

void Element::setExpression(const QString &expression)
{
	if(expression.isEmpty())
		return;

	this->expression=expression;
	column = nullptr;
	simple_col = SimpleColumn();
}

void Element::setOperatorClass(OperatorClass *oper_class)
{
	operator_class = oper_class;
}

void Element::setSimpleColumn(const SimpleColumn &col)
{
	if(!col.isValid())
		return;

	simple_col = col;
	column = nullptr;
	expression = "";
}

void Element::setSortingAttribute(SortOrder attrib, bool value)
{
	if(attrib > NullsFirst)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	sorting_attibs[attrib]=value;
}

void Element::setSortingEnabled(bool value)
{
	sorting_enabled=value;
}

bool Element::isSortingEnabled()
{
	return sorting_enabled;
}

bool Element::getSortingAttribute(SortOrder attrib)
{
	if(attrib > NullsFirst)
		throw Exception(ErrorCode::RefAttributeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return sorting_attibs[attrib];
}

Column *Element::getColumn()
{
	return column;
}

QString Element::getExpression()
{
	return expression;
}

OperatorClass *Element::getOperatorClass()
{
	return operator_class;
}

SimpleColumn Element::getSimpleColumn()
{
	return simple_col;
}

void Element::configureAttributes(attribs_map &attributes, SchemaParser::CodeType def_type)
{
	attributes[Attributes::Column]="";
	attributes[Attributes::Expression]="";
	attributes[Attributes::OpClass]="";
	attributes[Attributes::UseSorting]=(this->sorting_enabled ? Attributes::True : "");
	attributes[Attributes::NullsFirst]=(this->sorting_enabled && this->sorting_attibs[NullsFirst] ? Attributes::True : "");
	attributes[Attributes::AscOrder]=(this->sorting_enabled && this->sorting_attibs[AscOrder] ? Attributes::True : "");

	if(column)
		attributes[Attributes::Column]=column->getName(def_type == SchemaParser::SqlCode);
	else if(simple_col.isValid())
	{
		if(def_type == SchemaParser::SqlCode)
			attributes[Attributes::Column] = BaseObject::formatName(simple_col.getName());
		else
			attributes[Attributes::Column] = simple_col.getName();
	}
	else
		attributes[Attributes::Expression]=expression;

	if(operator_class)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::OpClass]=operator_class->getName(def_type == SchemaParser::SqlCode);
		else
			attributes[Attributes::OpClass]=operator_class->getSourceCode(def_type, true);
	}
}

bool Element::isEqualsTo(Element &elem)
{
  return (this->column == elem.column &&
					this->simple_col == elem.simple_col &&
		 this->expression == elem.expression &&
		 this->operator_class == elem.operator_class &&
		 this->sorting_enabled == elem.sorting_enabled &&
		 this->sorting_attibs[AscOrder] == elem.sorting_attibs[AscOrder] &&
		 this->sorting_attibs[NullsFirst] == elem.sorting_attibs[NullsFirst]);
}

bool Element::operator == (Element &elem)
{
	return isEqualsTo(elem);
}

std::vector<BaseObject *> Element::getDependencies()
{
	std::vector<BaseObject *> deps;

	if(operator_class)
		deps.push_back(operator_class);

	if(column)
		deps.push_back(column);

	return deps;
}

bool Element::operator == (const Element &elem)
{
	return isEqualsTo(const_cast<Element &>(elem));
}
