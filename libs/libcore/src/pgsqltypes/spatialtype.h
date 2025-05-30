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
\class SpatialType
\brief Implements the operations to manipulate geospatial (postgis) data types.
**/

#ifndef SPATIAL_TYPE
#define SPATIAL_TYPE

#include "templatetype.h"

class __libcore SpatialType: public TemplateType<SpatialType>{
	private:
		static const QStringList type_names;

	public:
		enum VariationId: unsigned {
			NoVar,
			VarZ,
			VarM,
			VarZm
		};

	private:
		VariationId variation;

		/*! \brief Used in conjunction with spatial_type, and denotes the SRID value
	 for the spatial type. This value goes from -1 to n. */
		int srid;

	public:
		static constexpr unsigned Point = 1;
		static constexpr unsigned LineString = 2;
		static constexpr unsigned Polygon = 3;
		static constexpr unsigned MultiPoint = 4;
		static constexpr unsigned MultiLineString = 5;
		static constexpr unsigned MultiPolygon = 6;
		static constexpr unsigned Geometry = 7;
		static constexpr unsigned GeometryCollection = 8;

		SpatialType(const QString &type_name, int srid, VariationId variation_id=SpatialType::NoVar);
		SpatialType(unsigned type_id, int srid, VariationId var_id=SpatialType::NoVar);
		SpatialType();

		void setVariation(VariationId var);
		VariationId getVariation();

		void setSRID(int srid);
		int getSRID();

		QString operator * ();

		static QStringList getTypes();

		unsigned setType(unsigned type_id) override;
		unsigned setType(const QString &type_name) override;
		QString getTypeName(unsigned type_id) override;
};

#endif 
