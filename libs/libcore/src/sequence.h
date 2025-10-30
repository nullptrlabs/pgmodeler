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
\class Sequence
\brief Implements the operations to manipulate sequences on the database.
\note <strong>Creation date:</strong> 26/04/2008
*/

#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "physicaltable.h"

class __libcore Sequence: public BaseObject {
	private:
		/*! \brief Indicates taht the sequence is cyclic
		 (the counter resets when maximum value is reached) */
		bool cycle;

		//! \brief Minimum value
		QString min_value,
		//! \brief Maximum value
		max_value,
		//! \brief Current sequence value
		start,
		//! \brief Sequence value increment
		increment,
		//! \brief Sequence cache value
		cache;

		//! \brief Column that owns the sequence
		Column *owner_col;

		//! \brief Returns true when the passed value is a valid number
		bool isValidValue(const QString &value);

		//! \brief Returns true when the passed value is null (zero)
		bool isZeroValue(const QString &value);

		//! \brief Returns the formated value excluding the aditional operators
		QString formatValue(const QString &value);

		/*! \brief Compares two values and returns:
		 -1 when: value1 < value2
			0 when: value1 = value2
			1 when: value1 > value2 */
		int compareValues(QString value1, QString value2);

	public:
		//! \brief Constants that indicates the maximum and minimum values accepted by sequence
		//For serial sequences
		static const QString
		MaxPositiveValue,
		MaxNegativeValue,

		//For smallserial sequences
		MaxSmallPositiveValue,
		MaxSmallNegativeValue,

		//For bigserial sequences
		MaxBigPositiveValue,
		MaxBigNegativeValue;

		Sequence();
		~Sequence() override = default;

		//! \brief Defines if the sequence is a cycle
		void setCycle(bool value);

		//! \brief Sets at once all the necessary fields to define a sequence
		void setValues(QString minv, QString maxv, QString inc, QString start, QString cache);

		/*! \brief Sets all values at once based on the serial type specified (smallserial, serial or bigserial).
		If other type the three serial types are passed the method will consider as 'serial' */
		void setDefaultValues(PgSqlType serial_type);

		//! \brief Defines the owner column using a table and a column name
		void setOwnerColumn(PhysicalTable *table, const QString &col_name);

		//! \brief Defines the owner column using a column itself
		void setOwnerColumn(Column *column);

		//! \brief Sets the sequence name
		void setName(const QString &name) override;

		/*! \brief Sets the schema that the sequence belongs. This method raises an error
		 when there is a owner column and the schema to be set is different from
		 the column parent table schema */
		void setSchema(BaseObject *schema) override;

		bool isCycle();
		QString getMaxValue();
		QString getMinValue();
		QString getIncrement();
		QString getStart();
		QString getCache();
		Column *getOwnerColumn();

		/*! \brief Returns whether the sequence references columns added
		 by relationship. This method is used as auxiliary
		 to control which sequence reference columns added by the
		 relationship in order to avoid referece breaking due constants
		 connections and disconnections of relationships */
		bool isReferRelationshipAddedColumn();

		//! \brief Makes a copy between two sequences
		void operator = (Sequence &seq);

		//! \brief Returns the SQL / XML definition for the sequence
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		QString getAlterCode(BaseObject *object) final;

		QString getDataDictionary(bool md_format, const attribs_map &extra_attribs = {});

		void updateDependencies() override;
};

#endif
