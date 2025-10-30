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
\class StyledTextboxView
\brief This class is only a styled representation of the TextboxView class
*/

#ifndef STYLED_TEXTBOX_VIEW_H
#define STYLED_TEXTBOX_VIEW_H

#include "textboxview.h"
#include <QGraphicsSceneMouseEvent>

class __libcanvas StyledTextboxView: public TextboxView {
	Q_OBJECT

	private:
		//! \brief Fold indicator appended at bottom-right corner of the object
		QGraphicsPolygonItem *fold;

		QGraphicsTextItem *styled_txt_item;

	public:
		static constexpr double DefaultWidth = 40;

		StyledTextboxView(Textbox *txtbox, bool override_style=false);

		void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

		~StyledTextboxView() override;

	protected slots:
		void configureObject() override;
};

#endif
