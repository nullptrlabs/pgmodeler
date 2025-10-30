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
\class Textbox
\brief This class is used to represent informative textboxes over the
model also used to represent relationship labels.
\note <strong>Creation date:</strong> 05/09/2007
*/

#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "basegraphicobject.h"
#include <QColor>

class __libcore Textbox: public BaseGraphicObject{
	private:
		//! \brief Stores the status of text attributes (bold / italic / underline)
		bool text_attributes[3];

		double font_size;

		double text_width;

		//! \brief Color used to draw the text
		QColor text_color;

	public:
		//! \brief Constants used to configure the text attributes
		enum TextAttrib: unsigned {
			ItalicText,
			BoldText,
			UnderlineText
		};

		/*! \brief To define the content of the textboxes
		 the method setComment() must be used and the getComment()
		 method used to get the current text */
		Textbox();
		~Textbox() override = default;

		//! \brief Sets the attributes of the text
		void setTextAttribute(TextAttrib attrib, bool value);

		//! \brief Sets the color used to draw the text of textbox
		void setTextColor(const QColor &color);

		void setFontSize(double size);

		void setTextWidth(double width);

		/*! \brief Since textboxes doesn't has SQL code definition this method will return a empty
		definition whenever the user try to generate a SQL for this object. */
		QString getSourceCode(SchemaParser::CodeType def_type) final;

		//! \brief Returns the current state of the passed text attribute
		bool getTextAttribute(TextAttrib attrib);

		QColor getTextColor();

		double getFontSize();

		double getTextWidth();

		void setZValue(int z_value) override;

		//! \brief Copies the attributes between textboxes
		void operator = (Textbox &txtbox);
};

#endif
