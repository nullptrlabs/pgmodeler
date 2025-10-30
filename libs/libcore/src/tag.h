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
\class Tag
\brief Implements a way to separate tables and views via tags. Each tag contain custom colors
that replaces the original colors of tables/views that owns them.
*/


#ifndef TAG_H
#define TAG_H

#include "basegraphicobject.h"
#include <QColor>
#include <QLinearGradient>

class __libcore Tag: public BaseObject {
	private:
		static unsigned tag_id;

		//! \brief Stores the object colors configuration
		std::map<QString, std::vector<QColor>> color_config;

		/*! \brief Validates the element id and the color id. This method will
	raise an error if some of parameters are invalid */
		void validateElementId(const QString &elem_id, ColorId color_id);

	public:
		Tag();
		~Tag() override = default;

		/*! \brief Set the tag name. Different from regular database model object there is no rule
		 * when setting the name. The only exception is that the name cannot be greater than
		 * BaseObject::OBJECT_NAME_MAX_LENGTH */
		void setName(const QString &name) final;

		QString getName(bool = false, bool = false) final;

		//! \brief Set the specified element id color
		void setElementColor(const QString &elem_id, const QColor &color, ColorId color_id);

		/*! \brief Set the specified element colors using a string that contains a color set
		 * on the format: [#RRGGBB],[#RRGGBB],[#RRGGBB]. Color on the string beyond position 3
		 * will be ignored */
		void setElementColors(const QString &elem_id, const QString &colors);

		//! \brief Returns a sigle color (at specified index) of the element.
		QColor getElementColor(const QString &elem_id, ColorId color_id);

		/*! \brief Returns a gradient configured for the current element id. This method will return
		 * an error if the current element does not have at least 2 colors configured */
		QLinearGradient getFillStyle(const QString &elem_id);

		QString getSourceCode(SchemaParser::CodeType def_type) override;
		QString getSourceCode(SchemaParser::CodeType def_type, bool reduced_form) final;

		static QStringList getColorAttributes(void);

		//! \brief Copy the attributes between two tags
		void operator = (Tag &tag);
};

#endif
