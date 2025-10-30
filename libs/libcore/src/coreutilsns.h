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
\ingroup libcore
\namespace CoreUtilsNs
\brief This namespace is used to organize all functions or constants used in the libcore package.
*/

#ifndef CORE_UTILS_NS_H
#define CORE_UTILS_NS_H

#include "baseobject.h"
#include "utilsns.h"

namespace CoreUtilsNs {
	/*! \brief Holds the check mark character for use in data dictionary
	 *  to indicate constraints applied to the column */
	const QString DataDictCheckMark("&#10003;");

	/*! \brief Filters a list of objects by excluding the elements by their type.
	 * If the current object type is in the excl_types list then the object will not be present
	 * in the returned list.
	 * NOTE: This function returns a new list of objects letting the input object list unchanged. */
	extern __libcore std::vector<BaseObject *> filterObjectsByType(const std::vector<BaseObject *> &list,
																																	const std::vector<ObjectType> &excl_types);

	/*! \brief Template function that makes a copy from 'copy_obj' to 'psrc_obj' doing the cast to the
		 correct object type. If the source object (psrc_obj) is not allocated the function allocates the attributes
		 before copying. Both objects must be the same type if both are allocated.
		 -- Brainfuck syntax style! :p -- */
	template <class Class>
	extern __libcore void copyObject(BaseObject **psrc_obj, Class *copy_obj);

	/*! \brief This functions is a second way to make a copy between two objects. It simply calls
		 the template function above. */
	extern __libcore void copyObject(BaseObject **psrc_obj, BaseObject *copy_obj, ObjectType obj_type);

	//! \brief Returns true if the specified word is a PostgreSQL reserved word.
	extern __libcore bool isReservedKeyword(const QString &word);

	/*! \brief Generates an unique name based on the specified object and the list of objects of the same type.
	 * The user can specify a suffix for the generated name as well if the comparison inside the method must take into account
	 * the schema names of the involved objects comp_sch_names parameter. The optinal parameter use_suf_on_conflict indicates
	 * that the suffix should be used only in case of conflicts.
	 * Now, the discard_input_obj is used to indicate whether the input object should be considered or not in the unique
	 * name generation, in this case, when false the name of the input object (obj) will always be compared to itself if
	 * it is present in the provided list. When that parameter is true the comparison is not made. */
	template <class Class>
	QString generateUniqueName(BaseObject *obj, std::vector<Class *> &obj_vector,
											bool comp_sch_names = false, const QString &suffix = "",
											bool use_suf_on_conflict = false, bool discard_input_obj = false)
	{
		unsigned counter = 0;
		QString uniq_name, obj_name, id;
		Class *aux_obj = nullptr;
		typename std::vector<Class *>::iterator itr = obj_vector.begin(), itr_end = obj_vector.end();
		QChar oper_uniq_chr = '?'; //Char appended at end of operator names in order to resolve conflicts
		ObjectType obj_type;

		if(!obj)
			return("");
		
		//Cast objects will not have the name changed since their name are automatically generated
		if(obj->getObjectType()==ObjectType::Cast || obj->getObjectType()==ObjectType::Database)
			return(obj->getName());

		obj_name = obj->getName(false);
		obj_type = obj->getObjectType();

		if(!use_suf_on_conflict && obj_type != ObjectType::Operator)
			obj_name += suffix;

		counter = (use_suf_on_conflict && obj_type!= ObjectType::Operator? 0 : 1);
		uniq_name = obj_name;

					 //Check if the object's new name conflicts with some of the objects within the list
		while(itr != itr_end)
		{
			//If the name length exceeds the maximum size we need to truncate it
			if(uniq_name.size() > BaseObject::ObjectNameMaxLength)
			{
				/* Generating the hash of the current timestamp (in msecs) so we can extract
				 * the first 6 characters to use as a temporary disambiguation suffix */
				id = UtilsNs::getStringHash(QString::number(QDateTime::currentMSecsSinceEpoch())).mid(0,6);

				/* Truncates the name until 7 bytes before the maximum allowed length
				 * so we can append the id properly */
				uniq_name.truncate(BaseObject::ObjectNameMaxLength - id.size() - 1);

							 //Append the id of the object on its name (this is not applied to operators)
				if(obj_type != ObjectType::Operator)
					uniq_name += "_" + id;
			}

			aux_obj = (*itr);
			itr++;

			if(discard_input_obj && aux_obj == obj)
				continue;

						 //If a conflicting object is found
			if(aux_obj->getName() == uniq_name &&
					((!comp_sch_names) ||
					 (comp_sch_names && aux_obj->getSchema() == obj->getSchema())))
			{
				//For operators is appended a '?' on the name
				if(obj_type == ObjectType::Operator)
					uniq_name = QString("%1%2").arg(obj_name, QString("").leftJustified(counter++, oper_uniq_chr));
				else
				{
					uniq_name = QString("%1%2%3")
													.arg(obj_name,
																use_suf_on_conflict ? suffix : "",
																use_suf_on_conflict && counter == 0 ? "" : QString::number(counter));
					counter++;
				}

				itr = obj_vector.begin();
			}
		}

		return uniq_name;
	}
}

#endif
