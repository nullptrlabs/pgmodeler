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
\class PgSqlType
\brief Implements the operations to manipulate PostgreSQL built-in data types.
**/

#ifndef PGSQL_TYPE
#define PGSQL_TYPE

#include "usertypeconfig.h"
#include "intervaltype.h"
#include "spatialtype.h"
#include "templatetype.h"
#include "schemaparser.h"

class __libcore PgSqlType: public TemplateType<PgSqlType>{
	private:
		static const QStringList type_names;

		//! \brief Offset for all PostGiS types
		static constexpr unsigned PostGiSStart = 64,
		PostGiSEnd = 82;

		//! \brief Offset for oid types
		static constexpr unsigned OidStart = 94,
		OidEnd = 108;

		//! \brief Offset for pseudo types
		static constexpr unsigned PseudoStart = 109,
		PseudoEnd = 133;

		//! \brief Configuration for user defined types
		static std::vector<UserTypeConfig> user_types;

		//! \brief Dimension of the type if it's configured as array
		unsigned dimension,

		//! \brief Type's length (used for types like varchar, date e bit)
		length;

		//! \brief Type's precison (used by numeric/decimal)
		int precision;

		//! \brief Indicates that the type (when used as timestamp or time) must considers timezones
		bool with_timezone;

		//! \brief Time interval used by 'interval' type
		IntervalType interval_type;

		//! \brief Spatial type used by the PostGiS types
		SpatialType spatial_type;

		// Disabled method.
		QString getTypeName(unsigned) override { return ""; }

	protected:
		//! \brief Adds a new reference to the user defined type
		//static void addUserType(const QString &type_name, BaseObject *ptype, DatabaseModel *pmodel, UserTypeConfig::TypeConf type_conf);
		static void addUserType(const QString &type_name, BaseObject *ptype, UserTypeConfig::TypeConf type_conf);

		//! \brief Removes a reference to the user defined type
		static void removeUserType(const QString &type_name, BaseObject *ptype);

		//! \brief Renames a user defined type
		static void renameUserType(const QString &type_name, BaseObject *ptype, const QString &new_name);

		/*! \brief Removes all registered types for the specified database model. Caution:
		This method must be called only when destroying the model. Calling it in any other
		situation can cause unexpected results */
		static void removeUserTypes(BaseObject *pmodel);

		//! \brief Returns the name of the type using its id
		static QString getUserTypeName(unsigned type_id);

		/*! \brief Sets the type based on the user defined type id. This
		 * method searches exclusively on the user_types vector */
		unsigned setUserType(unsigned type_id);

		/*! \brief Sets the type based on the object (user defined type) address. This
		 * method searches exclusively on the user_types vector */
		unsigned setUserType(BaseObject *ptype);

		/*! \brief Sets the type based on the id. This version also looks into the user_types vector
		 * in order to check if the type id being assigend belongs to an user defined type */
		unsigned setType(unsigned type_id) override;

		/*! \brief Sets the type based on the name. This version also looks into the user_types vector
		 * in order to check if the name being assigend belongs to an user defined type */
		unsigned setType(const QString &type_name) override;

	public:
		enum TypeCategory: unsigned {
			OidType,
			PolymorphicType,
			PseudoType,
			TimezoneType,
			DateTimeType,
			NumericType,
			IntegerType,
			FloatPointType,
			CharacterType,
			NetworkType,
			MonetaryType,
			BinaryType,
			BooleanType,
			GeometricType,
			BitStringType,
			TextSearchType,
			UuidType,
			XmlType,
			JsonType,
			RangeType,
			PostGiSType,
			OtherType,
			SerialType,
			UserType
		};

		PgSqlType();

		/*! \brief Creates a type from a simple string containing the name of the type.
				\note This method works in different way than PgSQLType::parserString() */
		PgSqlType(const QString &type_name);

		//! \brief Creates a type from a pointer that references an user defined type (Type class)
		PgSqlType(BaseObject *ptype);

		/*! \brief Creates a type from a type name and a series of data like
		 * dimension, length, precision, timezone option, interval type and spatial type.
		 * All parameters are optional except type_name and dimension which can be used to quicly create
		 * array of a certain type. */
		PgSqlType(const QString &type_name, unsigned dimension,
							unsigned length = 0, int precision = -1,
							bool with_timezone = false, IntervalType interv_type = IntervalType::Null,
							SpatialType spatial_type = SpatialType());

		/*! \brief Creates a type from a pointer to a data type (generally a user defined type, see UserTypeConfig class)
		 * and a series of data like dimension, length, precision, timezone option, interval type and spatial type.
		 * All parameters are optional except ptype and dimension which can be used to quickly create
		 * array of a certain type. */
		PgSqlType(BaseObject *ptype, unsigned dimension,
							unsigned length = 0, int precision = -1,
							bool with_timezone = false, IntervalType interv_type = IntervalType::Null,
							SpatialType spatial_type = SpatialType());

		/*! \brief Creates a type from a type id and a series of data like
		 * dimension, length, precision, timezone option, interval type and spatial type.
		 * All parameters are optional except type_id and dimension which can be used to quickly create
		 * array of a certain type. */
		PgSqlType(unsigned type_id, unsigned dimension,
							unsigned length = 0, int precision = -1,
							bool with_timezone = false, IntervalType interv_type = IntervalType::Null,
							SpatialType spatial_type = SpatialType());

		/*! \brief Creates a configured instance of PgSQLType from a string
		in SQL canonical form, e.g, varchar(255), timestamp with timezone, smallint[] and so on.
		If the string specifies arrays and length descriptors in wrong positions the method will
		try to return the correct type. The method will raise errors if the type could not be configured */
		static PgSqlType parseString(const QString &str);

		/*! \brief Returns the index of a user-defined type.
		 *  It's is possible to search by type name (type_name) or by the type's reference (ptype).
		 * Also, if a reference to a database model is provided the search is limited only to the types of that database model,
		 * otherwise the searching occurs on all types registered for all database models. */
		static unsigned getUserTypeIndex(const QString &type_name, BaseObject *ptype, BaseObject *pmodel = nullptr);

		static unsigned getBaseTypeIndex(const QString &type_name);

		/*! \brief Returns if the type is registered in the list of valid types (built-in one and user defined).
		The optional parameter 'pmodel' is used to filter user defined type of a specific database model */
		static bool isRegistered(const QString &type, BaseObject *pmodel = nullptr);

		static void getUserTypes(QStringList &type_list, BaseObject *pmodel, unsigned inc_usr_types);
		static void getUserTypes(std::vector<BaseObject *> &ptypes, BaseObject *pmodel, unsigned inc_usr_types);
		static QStringList getTypes(bool oids = true, bool pseudos = true);

		void setDimension(unsigned dim);
		void setLength(unsigned len);
		void setPrecision(int prec);
		void setWithTimezone(bool with_tz);
		void setIntervalType(IntervalType interv_type);
		void setSpatialType(SpatialType spat_type);

		unsigned getDimension();
		unsigned getLength();
		int getPrecision();
		IntervalType getIntervalType();
		SpatialType getSpatialType();

		//! \brief Returns the type category code based upon the current type id
		TypeCategory getCategory();

		bool isWithTimezone();		
		bool isPseudoType();
		bool isOidType();
		bool isArrayType();
		bool isRangeType();
		bool isSerialType();
		bool isDateTimeType();
		bool isTimezoneType();
		bool isNumericType();
		bool isIntegerType();
		bool isFloatPointType();
		bool isCharacterType();
		bool isNetworkType();
		bool isPolymorphicType();
		bool isMonetaryType();
		bool isBinaryType();
		bool isBooleanType();
		bool isGeometricType();
		bool isBitStringType();
		bool isTextSearchType();
		bool isUuidType();
		bool isXmlType();
		bool isJsonType();
		bool isPostGiSType();
		bool isUserType();

		bool isPostGisGeoType();
		bool isPostGisBoxType();
		static bool isPostGisGeoType(const QString &type_name);
		static bool isPostGisBoxType(const QString &type_name);

		bool hasVariableLength();
		bool acceptsPrecision();
		bool acceptsTimezone();

		/*! \brief Resets the length, precision, interval and spatial attributes of the type to their default values.
		 * If the all_attrs is true then the dimension and timezone info is reset too.
		 * This method preserves the dimension attribute and the name (idx) and is useful when you want to grantee that
		 * type being associated to an object need to have only name and dimesion info discarding everything else.
		 * Example of such objects are function parameters, casts, operators, aggregates and many others. */
		void reset(bool all_attrs = false);

		//! \brief Indicates if the 'this' type can be casted to 'type'
		bool canCastTo(PgSqlType type);

		/*! \brief Returns if the "this" type is equivalent to the specified type.
		In order to be compatible the "this" and "type" must be an alias from each other,
		for instance, "varchar" is compatible with "character varying" and vice-versa,
		smallint is compatible with int2, and so on. */
		bool isEquivalentTo(PgSqlType type);

		/*! \brief Returns true if the provided type is exactly the same as the "this".
		 * This method compares ALL attributes of the type. Note that this method is
		 * different from the operatores == (PgSqlType) because this latter compares only
		 * the indexes of the types. This method is useful if one need to fully compare the types */
		bool isExactTo(PgSqlType type);

		PgSqlType getAliasType();

		QString getSourceCode(SchemaParser::CodeType def_type, QString ref_type="");

		QString operator ~ () override;

		//! \brief Returns the complet SQL definition for the type (same as calling getSQLTypeName(true))
		QString operator * ();

		unsigned operator << (BaseObject *ptype);
		unsigned operator = (unsigned type_id) override;
		unsigned operator = (const QString &type_name) override;

		//! \brief Compares the index of the "this" with the provided type index. If an exact match is needed use isExactTo()
		bool operator == (unsigned type_idx) override;

		//! \brief Compares the index of the "this" with the provided type. If an exact match is needed use isExactTo()
		bool operator == (PgSqlType type);

		//! \brief Compares the index of the "this" with the provided type name index. If an exact match is needed use isExactTo()
		bool operator == (const QString &type_name);

		//! \brief Compares the index of the "this" with the provided type reference. If an exact match is needed use isExactTo()
		bool operator == (BaseObject *ptype);

		bool operator != (const QString &type_name);
		bool operator != (PgSqlType type);
		bool operator != (unsigned type_idx) override;

		//! \brief Returns the reference to the database model object that is used as user-defined type
		BaseObject *getObject();

		//! \brief Returns the configuration id for the user defined type
		unsigned getUserTypeConfig();

		//! \brief Returns the code (id) of the type. This is equivalent to call !type
		unsigned getTypeId() override;

		/*! \brief Returns the name of the type. This is equivalent to call ~type.
		 * If incl_dimension is true then returns only the type name appending the dimension descriptor [] if the type's dimension is > 0.
		 * Other attributes of the type are discarded. */
		QString getTypeName(bool incl_dimension);

		/*! \brief Returns the complete SQL form of the type
		* including the length, precision and other available quantifiers. */
		QString getTypeSql();

		static QStringList getTypes();

		friend class Type;
		friend class Domain;
		friend class PhysicalTable;
		friend class Table;
		friend class Sequence;
		friend class View;
		friend class Extension;
		friend class DatabaseModel;
};

/* Registering the PgSqlType class as a Qt MetaType in order to make
 * it liable to be sent through signal parameters as well as to be
 * to be used by QVariant */
Q_DECLARE_METATYPE(PgSqlType)

#endif 
