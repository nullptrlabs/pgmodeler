/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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
\class ValidationInfo
\brief Stores information of a single object when model is being validated. Instances of this class
are emitted during the validation process on ModelValidationHelper class.
*/

#ifndef VALIDATION_INFO_H
#define VALIDATION_INFO_H

#include "guiglobal.h"
#include "baseobject.h"
#include <vector>

class __libgui ValidationInfo {
	public:
		enum ValType: unsigned {
			NoUniqueName,
			BrokenReference,
			SpObjBrokenReference,
			BrokenRelConfig,
			MissingExtension,
			UniqueSameAsPk,
			SqlValidationError,
			ValidationAborted
		};

	private:
		//! \brief Validation type (see constants below)
		ValType val_type;

		//! \brief Object which the validation info belongs to
		BaseObject *object;

		/*! \brief References to the validated object. This vector isn't filled when
		the info is about SQL validation */
		std::vector<BaseObject *> references;

		QStringList errors;

	public:
		ValidationInfo();
		ValidationInfo(ValType val_type, BaseObject *object, std::vector<BaseObject *> references);
		ValidationInfo(const QString &msg);
		ValidationInfo(Exception e);

		//! \brief Returns the validation type
		ValType getValidationType();

		//! \brief Returns the object which the validation info belongs to
		BaseObject *getObject();

		//! \brief Returns the objects that somehow references the validation info object
		std::vector<BaseObject *> getReferences();

		//! \brief Returns the SQL errors generated during the SQL validation
		QStringList getErrors();

		/*! \brief Returns if the content of validation info is valid. To be valid
		the info must have the object not null when it is NO_UNIQUE_NAME or BROKEN_REFERENCE,
		and when its SQL_VALIDATION_ERR the sql error message must be set */
		bool isValid();

		friend class ModelValidationHelper;
		friend class QVariant;
};
#endif

/* Registering the ValidationInfo class as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(ValidationInfo)
