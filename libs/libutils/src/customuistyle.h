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
\ingroup libutils
\class CustomUiStyle
\brief Implements a derivative class of QProxyStyle that can be used to override the application UI style at rendering level.
One instance of this class is created in the moment the application is instantiated and the custom style is applied to
all widgets in the application.
*/

#ifndef CUSTOM_UI_STYLE_H
#define CUSTOM_UI_STYLE_H

#include "utilsglobal.h"
#include <QProxyStyle>
#include <QMap>
#include <QStyleOption>
#include <QStyleOptionComboBox>
#include <QPainter>
#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QIcon>
#include <QPushButton>

class __libutils CustomUiStyle : public QProxyStyle {
	private:
		// Enum to control which side of the path should be open (only one side at a time)
		enum OpenEdge {
			NotOpen = 0,   // Closed path (default)
			OpenLeft,   // Open on the left edge
			OpenTop,    // Open on the top edge
			OpenRight,  // Open on the right edge
			OpenBottom  // Open on the bottom edge
		};

		// Enum to control which corners should have rounded edges using bitwise operations
		enum CornerFlag: unsigned {
			NoCorners = 0,
			TopLeft = 1,
			TopRight = 2,
			BottomLeft = 4,
			BottomRight = 8,
			AllCorners = TopLeft | TopRight | BottomLeft | BottomRight
		};

		// Enum to specify arrow direction for drawing control arrows
		enum ArrowType {
			UpArrow,
			DownArrow,
			LeftArrow,
			RightArrow
		};

		struct WidgetState {
			const bool is_enabled,
								 is_active,
								 is_hovered,
								 is_checked,
								 is_selected,
								 is_pressed,
								 is_focused,
								 is_default,
								 has_custom_color;

			WidgetState(const QStyleOption *option, const QWidget *widget) :
					is_enabled(option->state & State_Enabled),
					is_active(option->state & State_Active),
					is_hovered(option->state & State_MouseOver),
					is_checked(option->state & State_On),
					is_selected(option->state & State_Selected),
					is_pressed(option->state & State_Sunken),
					is_focused(option->state & State_HasFocus),
					is_default(is_enabled && widget &&
										 qobject_cast<const QPushButton *>(widget) &&
										 qobject_cast<const QPushButton *>(widget)->isDefault()),

					has_custom_color(widget &&
													 widget->styleSheet().contains("background-color"))
					{};
		};									 

		static QMap<PixelMetric, int> pixel_metrics;

		static constexpr qreal BlendFactor = 0.7,
							   					 PenWidth = 1.2;

		static constexpr int ArrowWidth = 9, // Complex control up arrow width
							 					 ArrowHeight = 5,  // Complex control up arrow height
												 SplitterSize = 20;

		// Helper method to add edge with optional rounded corner to QPainterPath
		void addEdgeWithCorner(QPainterPath &path, const QRectF &rect, OpenEdge side, int radius) const;

		// Generic method to create QPainterPath with configurable corner radius and open sides
		QPainterPath createControlShape(const QRect &rect, int radius, CornerFlag corners = AllCorners,
																		qreal dx = 0, qreal dy = 0, qreal dw = 0, qreal dh = 0,
																		OpenEdge open_edge = NotOpen) const;

		// Draws complex control (CC) of combo boxes
		void drawCCComboBox(ComplexControl control, const QStyleOptionComplex *option,
												QPainter *painter, const QWidget *widget) const;

		// Draws complex control (CC) of group boxes
		void drawCCGroupBox(ComplexControl control, const QStyleOptionComplex *option,
												QPainter *painter, const QWidget *widget) const;

		// Draws complex control (CC) of spin boxes
		void drawCCSpinBox(ComplexControl control, const QStyleOptionComplex *option,
											 QPainter *painter, const QWidget *widget) const;

		// Draws control elements (CE) of tab bars
		void drawCETabBar(ControlElement element, const QStyleOption *option,	
											QPainter *painter, const QWidget *widget) const;

		void drawControlArrow(const QStyleOption *option, QPainter *painter, const QWidget *,
													ArrowType arr_type, bool small_sz = false) const;

		// Draws menu arrow for QToolButton and QPushButton with menus (returns true if handled)
		void drawButtonMenuArrow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

		// Draws a scrollbar button (AddLine or SubLine) intelligently handling both types
		void drawScrollBarButton(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget, 
														 QStyle::SubControl button_type, const QColor &bg_color, const QColor &border_color) const;

		// Draws editable ComboBox with custom background and border
		void drawEditableComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of buttons
		void drawPEButtonPanel(PrimitiveElement element, const QStyleOption *option,
													 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of checkboxes and radio buttons
		void drawPECheckBoxRadioBtn(PrimitiveElement element, const QStyleOption *option,
															 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of frame background (when StyleHint is set)
		void drawPEHintFramePanel(PrimitiveElement element, const QStyleOption *option,
															QPainter *painter, const QWidget *widget) const;

		void drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option,	
															 QPainter *painter, const QWidget *widget, int border_radius) const;

		void drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option,	
														 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of line edits
		void drawPELineEditPanel(PrimitiveElement element, const QStyleOption *option,
														 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of tooltips
		void drawPEToolTip(PrimitiveElement element, const QStyleOption *option,
											 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of menu panels
		void drawPEMenuPanel(PrimitiveElement element, const QStyleOption *option,
												 QPainter *painter, const QWidget *) const;

		// Draws control elements (CE) of menu separators. Returns true if a separator was drawn
		void drawCEMenuItem(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

		// Draws control elements (CE) of progress bars
		void drawCEProgressBar(ControlElement element, const QStyleOption *option,
													 QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of progress chunk indicator
		void drawPEProgressChunk(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

		// Draws primitive elements (PE) of header sort arrows
		void drawPEHeaderArrow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

	// Draws control elements (CE) of header sections (table/tree headers)
	void drawCEHeaderSection(ControlElement element, const QStyleOption *option,
													 QPainter *painter, const QWidget *widget) const;

	// Draws control elements (CE) of splitters with custom styling
	void drawCESplitter(ControlElement element, const QStyleOption *option,
											QPainter *painter, const QWidget *widget) const;

	// Draws primitive elements (PE) of tabs, group boxes and other framed elements
	void drawPETabWidgetFrame(PrimitiveElement element, const QStyleOption *option,
														QPainter *painter, const QWidget *widget) const;
		void drawCCScrollBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

		// Draws control elements (CE) of scroll bars
		void drawCEScrollBar(ControlElement element, const QStyleOption *option,
												 QPainter *painter, const QWidget *widget) const;

		void drawSpinBoxButton(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, QStyle::SubControl btn_sc_id) const;

		// Draws SpinBox sub-components with specialized styling
		void drawSpinBoxEditField(const QStyleOption *option, QPainter *painter, const QWidget *widget) const;

		//! \brief Helper function to rotate a QPolygonF around its center
		static QPolygonF rotatePolygon(const QPolygonF &polygon, qreal degrees);

		//! \brief Helper function to get color from palette considering widget state
		static QColor getStateColor(const QPalette& pal, QPalette::ColorRole role, const QStyleOption* option);

		//! \brief Helper function to get color from application palette considering widget state
		static QColor getStateColor(QPalette::ColorRole role, const QStyleOption *option);

		static constexpr char StyleHintProp[] = "style-hint",
													StyleHintColor[] = "style-hint-color";

	public:
		static constexpr int NoRadius = 0,
							 ButtonRadius = 4,
							 InputRadius = 5,
							 FrameRadius = 4,
							 HintFrameRadius = 6,
							 TabWgtRadius = 2,
							 TabBarRadius = 5,
							 ScrollBarRadius = 2;

		static constexpr int NoFactor = 0,
												 XMinFactor = 105,
												 MinFactor = 120,
							 					 MidFactor = 135,
							 					 MaxFactor = 150;

		//! \brief Enum for setting style hints on widgets via setProperty method			
		enum StyleHint {
			NoHint,
			DefaultFrmHint, // Default frame
			InfoFrmHint, // Blueish border (informational)
			ConfirmFrmHint, // Green border (confirmation)
			AlertFrmHint, // Yellow border (alert)
			ErrorFrmHint, // Red border (error)
			SuccessFrmHint // Greenish border (success)
		};

		CustomUiStyle() = default;

		CustomUiStyle(const QString &key);

		~CustomUiStyle() override = default;

		QPixmap createGrayMaskedPixmap(const QPixmap &original) const;

		static QColor getAdjustedColor(const QColor &color, int dark_ui_factor, int light_ui_factor);

		void drawComplexControl(ComplexControl control, const QStyleOptionComplex *option,
														QPainter *painter, const QWidget *widget) const override;

		void drawControl(ControlElement element, const QStyleOption *option,
										 QPainter *painter, const QWidget *widget) const override;

		void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const override;

	void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
										 QPainter *painter, const QWidget *widget) const override;

	void polish(QWidget *widget) override;

	QPixmap generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap &pixmap, const QStyleOption *option) const override;	int pixelMetric(PixelMetric metric, const QStyleOption * option = nullptr, const QWidget * widget = nullptr) const override;

	/*! \brief Sets a style hint on a QFrame to customize its border color and radius
	 * So it can be rendered as a inlined alert/info/error frame. 
	 * This method forces the frame shape to StyledPanel. */
	static void setStyleHint(StyleHint hint, QFrame *frame);

	//! \brief Checks if the current application palette is dark (dark theme)
	static bool isDarkPalette();

	//! \brief Checks if the current palette is dark (dark theme)
	static bool isDarkPalette(const QPalette& pal);

	/*! \brief Defines a custom pixel metric attribute value globally.
	* Which means, all instances of this class will share the same pixel metrics values */
	static void setPixelMetricValue(PixelMetric metric, int value);
};

#endif
