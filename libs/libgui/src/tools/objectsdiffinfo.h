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
\ingroup libgui
\class ObjectsDiffInfo
\brief Stores information about difference between two objects when the models are being compared. Instances of this class
are emitted during the diff process on ModelsDiffHelper class.
*/

#ifndef OBJECTS_DIFF_INFO_H
#define OBJECTS_DIFF_INFO_H

#include "guiglobal.h"
#include "tableobject.h"

class __libgui ObjectsDiffInfo {
	public:
		enum DiffType: unsigned {
			CreateObject,
			DropObject,
			AlterObject,
			IgnoreObject,
			NoDifference
		};

	private:
		//! \brief Difference type (see constants below)
		DiffType diff_type;

		BaseObject *object, *old_object;

	public:
		ObjectsDiffInfo();
		ObjectsDiffInfo(DiffType diff_type, BaseObject *object, BaseObject *old_obj);

		DiffType getDiffType();
		QString getInfoMessage();
		QString getDiffTypeString();
		BaseObject *getObject();
		BaseObject *getOldObject();

		bool operator == (ObjectsDiffInfo &info);

		friend class ModelsDiffHelper;
		friend class QVariant;
};

/* Registering the ObjectsDiffInfo class as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(ObjectsDiffInfo)

#endif
