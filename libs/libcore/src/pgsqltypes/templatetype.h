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

/**
\ingroup libcore/pgsqltypes
\class TemplateType
\brief Implements the basic operations to handle specialized types.

In order to reuse this class' implementation on derived classes we need to
use CRTP (Curiosly Recurring Template Pattern) which allows Base classes to access Derived classes
members and attributes. In the particular case of this class we need to create a new static type_names
attribute for each time the class is derived. See the derived classes any class *Type derived from TemplateType
to see the CRTP working.

Interesting readings:
https://stackoverflow.com/questions/12796580/static-variable-for-each-derived-class
https://stackoverflow.com/questions/1390913/are-static-variables-in-a-base-class-shared-by-all-derived-classes
https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
https://www.fluentcpp.com/2017/05/12/curiously-recurring-template-pattern/

**/

#ifndef TEMPLATE_BASE_TYPE_H
#define TEMPLATE_BASE_TYPE_H

#include "../coreglobal.h"
#include <QStringList>
#include "exception.h"

template<class Class>
class __libcore TemplateType {
	protected:	
		//! \brief Index of the type on the type_list vector
		unsigned type_idx;

		/*! \brief Sets an id to the type according to the limit stablished by the attribute
		 offset and type_count from each class */
		unsigned setType(unsigned type_id, const QStringList &type_list);
		unsigned setType(const QString &type_name, const QStringList &type_list);

		//! \brief Checks if the type id is valid according to the offset/count for the class
		bool isTypeValid(unsigned type_id, const QStringList &type_list);

		//! \brief Returns the string list for all types after removing the null ("") reserved item
		static QStringList getTypes(const QStringList &type_list);

		//! \brief Returns the type id searching by its name. Returns null when not found
		static unsigned getType(const QString &type_name, const QStringList &type_list);

		//! \brief Returns the type name/string at the specified type_idx
		static QString getTypeName(unsigned type_id, const QStringList &type_list);

	public:
		static constexpr unsigned Null = 0;

		TemplateType();
		virtual ~TemplateType();

		virtual unsigned setType(unsigned type_id) = 0;
		virtual unsigned setType(const QString &type_name) = 0;
		virtual QString getTypeName(unsigned type_id) = 0;

		virtual unsigned operator = (unsigned type_id);
		virtual unsigned operator = (const QString &type_name);
		virtual QString operator ~();

		//! \brief Returns the code (id) of the type
		virtual unsigned operator ! ();

		//! \brief Returns the code (id) of the type
		virtual unsigned getTypeId();

		virtual bool operator == (Class &type);
		virtual bool operator == (unsigned type_id);
		virtual bool operator != (Class &type);
		virtual bool operator != (unsigned type_id);
};

template<class Class>
TemplateType<Class>::TemplateType()
{
	type_idx = Class::Null;
}

template<class Class>
TemplateType<Class>::~TemplateType() = default;

template<class Class>
unsigned TemplateType<Class>::operator = (unsigned type_id)
{
	return setType(type_id);
}

template<class Class>
unsigned TemplateType<Class>::operator = (const QString &type_name)
{
	return setType(type_name);
}

template<class Class>
QString TemplateType<Class>::operator ~ ()
{
	return getTypeName(type_idx);
}

template<class Class>
unsigned TemplateType<Class>::setType(unsigned type_id, const QStringList &type_list)
{
	//Raises an error if the type count is invalid
	if(type_list.isEmpty())
		throw Exception(ErrorCode::ObtTypesInvalidQuantity, PGM_FUNC, PGM_FILE, PGM_LINE);

	//Raises an error if the type id is invalid
	if(!isTypeValid(type_id, type_list))
	{
		throw Exception(ErrorCode::AsgInvalidTypeObject, PGM_FUNC, PGM_FILE, PGM_LINE,
										nullptr, QString(QT_TR_NOOP("Type id: %1")).arg(static_cast<int>(type_id)));
	}

	type_idx = type_id;
	return type_idx;
}

template<class Class>
unsigned TemplateType<Class>::setType(const QString &type_name, const QStringList &type_list)
{
	try
	{
	return setType(static_cast<unsigned>(type_list.indexOf(type_name)), type_list);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e,
										QString(QT_TR_NOOP("Type name: %1")).arg(type_name));
	}
}

template<class Class>
bool TemplateType<Class>::isTypeValid(unsigned type_id, const QStringList &type_list)
{
	//Returns if the type id is valid according to the specified interval (offset-count)
	return (type_id < static_cast<unsigned>(type_list.size()) || type_id==Class::Null);
}

template<class Class>
QStringList TemplateType<Class>::getTypes(const QStringList &type_list)
{
	QStringList types = type_list;
	types.removeAll("");
	return types;
}

template<class Class>
unsigned TemplateType<Class>::getType(const QString &type_name, const QStringList &type_list)
{
	if(type_name.isEmpty())
		return Class::Null;

	int idx = type_list.indexOf(type_name);

	if(idx >= 0)
		return static_cast<unsigned>(idx);

	return Class::Null;
}

template<class Class>
QString TemplateType<Class>::getTypeName(unsigned type_id, const QStringList &type_list)
{
	if(type_id > static_cast<unsigned>(type_list.size()))
		throw Exception(ErrorCode::RefTypeInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

	return type_list[static_cast<int>(type_id)];
}

template<class Class>
unsigned TemplateType<Class>::operator ! ()
{
	return type_idx;
}

template<class Class>
unsigned TemplateType<Class>::getTypeId()
{
	return type_idx;
}

template<class Class>
bool TemplateType<Class>::operator == (Class &type)
{
	return (type.type_idx == type_idx);
}

template<class Class>
bool TemplateType<Class>::operator == (unsigned type_id)
{
	return (type_idx==type_id);
}

template<class Class>
bool TemplateType<Class>::operator != (Class &type)
{
	return (type.type_idx != type_idx);
}

template<class Class>
bool TemplateType<Class>::operator != (unsigned type_id)
{
	return (type_idx!=type_id);
}

#endif
