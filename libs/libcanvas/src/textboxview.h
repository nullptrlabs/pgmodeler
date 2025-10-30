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
\ingroup libcanvas
\class TextboxView
\brief Represents the textbox in a graphical way on the object scene
*/

#ifndef TEXTBOX_VIEW_H
#define TEXTBOX_VIEW_H

#include "textbox.h"
#include "baseobjectview.h"
#include "textpolygonitem.h"

class __libcanvas TextboxView: public BaseObjectView {
	Q_OBJECT

	private:
		//! \brief Indicates the the font / color styles will be overriden (need to call setColorStyle, setFontStyle)
		bool override_style;

		QString txtbox_tooltip;

	protected:
		TextPolygonItem *text_item;

		//! \brief Configures the shadow for the textbox
		void configureObjectShadow();

		//! \brief Configures the selection for the textbox
		void configureObjectSelection();

		//! \brief Configures the basic attributes for textbox
		void __configureObject();

	public:
		TextboxView(Textbox *txtbox, bool override_style = false);

		~TextboxView() override;

		/*! \brief Sets the fill and border color for the text box. This method has effect only when
		 the style can be overriden (via constructor) */
		void setColorStyle(const QBrush &fill_style, const QPen &border_style);

		/*! \brief Sets the font style for the text box. This method has effect only when
		 the style can be overriden (via constructor) */
		void setFontStyle(const QTextCharFormat &fmt);

		void setToolTip(const QString &tooltip);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

	protected slots:
		void configureObject() override;
};

#endif
