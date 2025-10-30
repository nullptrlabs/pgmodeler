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

#include "genericsql.h"
#include "tableobject.h"

GenericSQL::GenericSQL()
{
	obj_type = ObjectType::GenericSql;
	attributes[Attributes::Definition] = "";
	attributes[Attributes::Objects] = "";
	attributes[Attributes::HideDescription] = "";
	attributes[Attributes::References] = "";
	hide_description = false;
}

void GenericSQL::setDefinition(const QString &def)
{
	setCodeInvalidated(definition != def);
	definition = def;
}

QString GenericSQL::getDefinition()
{
	return definition;
}

void GenericSQL::setHideDescription(bool value)
{
	setCodeInvalidated(value != hide_description);
	hide_description = value;
}

std::vector<Reference> GenericSQL::getObjectsReferences()
{
	return objects_refs;
}

void GenericSQL::validateReferences(bool ignore_duplic)
{
	try
	{
		for(auto &ref : objects_refs)
			validateReference(ref, ignore_duplic);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE);
	}
}

int GenericSQL::getObjectRefNameIndex(const QString &ref_name)
{
	int idx = -1;
	std::vector<Reference>::iterator itr = objects_refs.begin(),
			itr_end = objects_refs.end();

	if(ref_name.isEmpty())
		return -1;

	while(itr != itr_end)
	{
		if((*itr).getRefName() == ref_name)
		{
			idx = itr - objects_refs.begin();
			break;
		}

		itr++;
	}

	return idx;
}

bool GenericSQL::isReferRelationshipAddedObject()
{
	bool found = false;
	std::vector<Reference>::iterator itr = objects_refs.begin(),
			itr_end = objects_refs.end();
	TableObject *tab_obj = nullptr;

	while(itr != itr_end && !found)
	{
		tab_obj = dynamic_cast<TableObject *>(itr->getObject());
		found = (tab_obj && tab_obj->isAddedByRelationship());
		itr++;
	}

	return found;
}

void GenericSQL::validateReference(const Reference &ref, bool ignore_duplic)
{
	if(!ref.getObject())
		throw Exception(ErrorCode::AsgNotAllocatedObjectReference,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!BaseObject::isValidName(ref.getRefName()) ||
		 !BaseObject::isValidName(ref.getRefName()))
		throw Exception(ErrorCode::AsgInvalidNameObjReference,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!ignore_duplic && getObjectRefNameIndex(ref.getRefName()) >= 0)
		throw Exception(Exception::getErrorMessage(ErrorCode::InsDuplicatedObjectReference).arg(ref.getRefName()),
										ErrorCode::InsDuplicatedObjectReference,PGM_FUNC,PGM_FILE,PGM_LINE);
}

void GenericSQL::addReference(const Reference &ref)
{
	try
	{
		validateReference(ref, false);
		objects_refs.push_back(ref);
		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void GenericSQL::addReferences(const std::vector<Reference> &refs)
{
	try
	{
		for(auto &ref : refs)
			addReference(ref);

		setCodeInvalidated(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void GenericSQL::removeObjectReference(const QString &ref_name)
{
	int idx = getObjectRefNameIndex(ref_name);

	if(idx >= 0)
		objects_refs.erase(objects_refs.begin() + idx);

	setCodeInvalidated(true);
}

void GenericSQL::removeObjectReferences()
{
	objects_refs.clear();
	setCodeInvalidated(true);
}

QString GenericSQL::getSourceCode(SchemaParser::CodeType def_type)
{
	QString code_def=getCachedCode(def_type, false);
	if(!code_def.isEmpty()) return code_def;

	QString fmt_definition = definition,
			name_attr = QString("%1%2%3").arg(SchemaParser::CharStartAttribute)
																	 .arg(Attributes::Name)
																	 .arg(SchemaParser::CharEndAttribute);

	if(!objects_refs.empty())
	{
		QString ref_name, ref_value, ref_alias;

		for(auto &ref : objects_refs)
		{
			if(def_type == SchemaParser::XmlCode)
			{
				attributes[Attributes::References] += ref.getXmlCode();
			}
			else
			{
				/* In order to use a reference name in the object's SQL code, the reference should be written in the for
				 * {ref_name} so it can be replaced by the corresponding value in the SQL code.
				 *
				 * By the same analogy, to use the alias of a reference, we need to write @{ref_name}. */
				ref_name = QString("%1%2%3").arg(SchemaParser::CharStartAttribute)
																		.arg(ref.getRefName())
																		.arg(SchemaParser::CharEndAttribute);

				ref_alias = QString("%1%2").arg(SchemaParser::CharValueOf).arg(ref_name);

				// Replacing the @{} tokens by the reference alias
				fmt_definition = fmt_definition.replace(ref_alias,
																								 ref.isFormatName() ?
																										 BaseObject::formatName(ref.getRefAlias()) :
																										 ref.getRefAlias());

				// Configuring the value of the reference
				ref_value = ref.isUseSignature() ?
										ref.getObject()->getSignature(ref.isFormatName()) :
										ref.getObject()->getName(ref.isFormatName());

				fmt_definition = fmt_definition.replace(ref_name, ref_value);
			}
		}
	}

	// Special case for the {name} attribute which is created automatically when there's no one defined by the user
	if(def_type == SchemaParser::SqlCode &&
		 fmt_definition.contains(name_attr) && getObjectRefNameIndex(Attributes::Name) < 0)
		fmt_definition = fmt_definition.replace(name_attr, this->getName(true));

	attributes[Attributes::Definition] = fmt_definition;

	return this->BaseObject::__getSourceCode(def_type);
}

void GenericSQL::updateDependencies()
{
	std::vector<BaseObject *> deps;

	for(auto &ref : objects_refs)
		deps.push_back(ref.getObject());

	BaseObject::updateDependencies(deps);
}
