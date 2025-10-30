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

#include "operatorclass.h"

OperatorClass::OperatorClass()
{
	obj_type=ObjectType::OpClass;
	family=nullptr;
	is_default=false;
	attributes[Attributes::Family]="";
	attributes[Attributes::Elements]="";
	attributes[Attributes::IndexType]="";
	attributes[Attributes::Type]="";
	attributes[Attributes::Default]="";
}

OperatorClass::~OperatorClass()
{
	elements.clear();
}

void OperatorClass::setDataType(PgSqlType data_type)
{
	//A null datatype is not accepted by the operator class
	if(data_type==PgSqlType::Null)
		throw Exception(ErrorCode::AsgInvalidTypeObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	data_type.reset();
	setCodeInvalidated(this->data_type != data_type);
	this->data_type=data_type;
}

void OperatorClass::setFamily(OperatorFamily *family)
{
	setCodeInvalidated(this->family != family);
	this->family=family;
}

void OperatorClass::setIndexingType(IndexingType index_type)
{
	setCodeInvalidated(indexing_type != index_type);
	this->indexing_type=index_type;
}

void OperatorClass::setDefault(bool value)
{
	setCodeInvalidated(is_default != value);
	is_default=value;
}

void OperatorClass::setElementsAttribute(SchemaParser::CodeType def_type)
{
	QString str_elems;
	unsigned i, count;

	/* Concatenates the code definition from elements separating
		them by comma. */
	count=elements.size();
	for(i=0; i < count; i++)
	{
		str_elems+=elements[i].getSourceCode(def_type);
		if(def_type==SchemaParser::SqlCode &&
				i < count-1) str_elems+=",\n";
	}

	attributes[Attributes::Elements]=str_elems;
}

void OperatorClass::addElement(OperatorClassElement elem)
{
	elements.push_back(elem);
}

void OperatorClass::removeElement(unsigned elem_idx)
{
	//Raises an error in case the element index is out of bound
	if(elem_idx >= elements.size())
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Removes the item from the elements list
	elements.erase(elements.begin() + elem_idx);
	setCodeInvalidated(true);
}

void OperatorClass::removeElements()
{
	elements.clear();
	setCodeInvalidated(true);
}

OperatorClassElement OperatorClass::getElement(unsigned elem_idx)
{
	//Raises an error in case the element index is out of bound
	if(elem_idx >= elements.size())
		throw Exception(ErrorCode::RefElementInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Returns the element on the specified index
	return elements[elem_idx];
}

bool OperatorClass::isElementExists(OperatorClassElement elem)
{
	bool exists=false;
	std::vector<OperatorClassElement>::iterator itr, itr_end;
	OperatorClassElement elem_aux;

	itr=elements.begin();
	itr_end=elements.end();

	while(itr!=itr_end && !exists)
	{
		elem_aux=(*itr);
		exists=(elem_aux==elem);
		itr++;
	}

	return exists;
}

unsigned OperatorClass::getElementCount()
{
	return elements.size();
}

PgSqlType OperatorClass::getDataType()
{
	return data_type;
}

OperatorFamily *OperatorClass::getFamily()
{
	return family;
}

IndexingType OperatorClass::getIndexingType()
{
	return indexing_type;
}

bool OperatorClass::isDefault()
{
	return is_default;
}

QString OperatorClass::getSourceCode(SchemaParser::CodeType def_type)
{
	return this->getSourceCode(def_type, false);
}

QString OperatorClass::getSourceCode(SchemaParser::CodeType def_type, bool reduced_form)
{
	QString code_def=getCachedCode(def_type, reduced_form);
	if(!code_def.isEmpty()) return code_def;

	setElementsAttribute(def_type);
	attributes[Attributes::IndexType]=(~indexing_type);
	attributes[Attributes::Default]=(is_default ? Attributes::True : "");

	if(def_type==SchemaParser::SqlCode)
		attributes[Attributes::Type]=(*data_type);
	else
		attributes[Attributes::Type]=data_type.getSourceCode(def_type);

	if(family)
	{
		if(def_type==SchemaParser::SqlCode)
			attributes[Attributes::Family]=family->getName(true);
		else
			attributes[Attributes::Family]=family->getSignature();
	}

	attributes[Attributes::Signature]=getSignature();
	return BaseObject::getSourceCode(def_type, reduced_form);
}

QString OperatorClass::getSignature(bool format)
{
	return BaseObject::getSignature(format) + QString(" USING %1").arg(~indexing_type);
}

void OperatorClass::updateDependencies()
{
	std::vector<BaseObject *> deps, elem_deps;

	deps.push_back(family);
	deps.push_back(data_type.getObject());

	for(auto &elem : elements)
	{
		elem_deps = elem.getDependencies();
		deps.insert(deps.cend(), elem_deps.begin(), elem_deps.end());
	}

	BaseObject::updateDependencies(deps);
}
