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
\class GenericSQL
\brief This class is used to represent generic sql commands as database objects
*/

#ifndef GENERIC_SQL_H
#define GENERIC_SQL_H

#include "reference.h"

class __libcore GenericSQL: public BaseObject{
	private:
		//! \brief The SQL definition of the generic object
		QString definition;

		/*! \brief Indicate whether the SQL comment description (-- object: ... type: ...)
		 * of the generic object must be displayed in the SQL code generation */
		bool hide_description;

		//! \brief The list of references to other object in the model
		std::vector<Reference> objects_refs;

		/*! \brief Returns the index of a object reference searching by its name.
		 * A negative return value indicates the reference doens't exist */
		int getObjectRefNameIndex(const QString &ref_name);

		/*! \brief Check if the provided object reference is correclty configured.
		 * The method will raise exceptions if any validation rule is broken.
		 * The parameter ignore_duplic makes the method ignore duplicated references names */
		void validateReference(const Reference &ref, bool ignore_duplic);

	public:
		GenericSQL();
		~GenericSQL() override = default;

		void setDefinition(const QString &def);

		QString getDefinition();

		void setHideDescription(bool value);

		void addReference(const Reference &ref);
		void addReferences(const std::vector<Reference> &refs);

		void removeObjectReference(const QString &ref_name);
		void removeObjectReferences();

		/*! \brief Returns whether the object references columns or constraints added
		 * by relationship to their parent tables. This method is used as auxiliary
		 * to control which generic SQL object references objects added by the
		 * relationship in order to avoid referece breaking due constants
		 * connections and disconnections of relationships */
		bool isReferRelationshipAddedObject();

		//! \brief Returns a copy of the objects references list
		std::vector<Reference> getObjectsReferences();

		void validateReferences(bool ignore_duplic);

		QString getSourceCode(SchemaParser::CodeType def_type) override;

		void updateDependencies() override;

		friend class GenericSQLWidget;
};

/* Registering the Reference struct as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(Reference)

#endif
