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

#include "customuistyle.h"
#include "enumtype.h"
#include <QAbstractItemView>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QFrame>
#include <QHeaderView>
#include <QPainterPath>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStyleOption>
#include <QStyleOptionSpinBox>
#include <QStyleOptionViewItem>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QPalette>
#include <QPen>
#include <QPoint>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <cmath>

QMap<QStyle::PixelMetric, int> CustomUiStyle::pixel_metrics;

CustomUiStyle::CustomUiStyle(const QString &key) : QProxyStyle(key) {}

void CustomUiStyle::addEdgeWithCorner(QPainterPath &path, const QRectF &rect, OpenEdge side, int radius) const
{
	qreal x = rect.x(), y = rect.y(),
		  w = rect.width(), h = rect.height();

	if(side == OpenTop)
	{
		// Top edge from current position to top-right corner
		if(radius > 0)
		{
			path.lineTo(x + w - radius, y);
			path.quadTo(x + w, y, x + w, y + radius);
		}
		else
		{
			path.lineTo(x + w, y);
		}
	}
	else if(side == OpenRight)
	{
		// Right edge from current position to bottom-right corner
		if(radius > 0)
		{
			path.lineTo(x + w, y + h - radius);
			path.quadTo(x + w, y + h, x + w - radius, y + h);
		}
		else
		{
			path.lineTo(x + w, y + h);
		}
	}
	else if(side == OpenBottom)
	{
		// Bottom edge from current position to bottom-left corner
		if(radius > 0)
		{
			path.lineTo(x + radius, y + h);
			path.quadTo(x, y + h, x, y + h - radius);
		}
		else
		{
			path.lineTo(x, y + h);
		}
	}
	else if(side == OpenLeft)
	{
		// Left edge from current position to top-left corner
		if(radius > 0)
		{
			path.lineTo(x, y + radius);
			path.quadTo(x, y, x + radius, y);
		}
		else
		{
			path.lineTo(x, y);
		}
	}
}

QPainterPath CustomUiStyle::createControlShape(const QRect &rect, int radius, CustomUiStyle::CornerFlag corners,
																							 qreal dx, qreal dy, qreal dw, qreal dh, OpenEdge open_edge) const
{
	QPainterPath path;

	// Apply adjustments to the rectangle
	QRectF adj_rect = QRectF(rect).adjusted(dx, dy, dw, dh);

	qreal x = adj_rect.x(), y = adj_rect.y(),
		  w = adj_rect.width(), h = adj_rect.height();

	// Extract individual corner radii using bitwise operations
	int tl_radius = (corners & TopLeft) ? radius : 0,
		tr_radius = (corners & TopRight) ? radius : 0,
		bl_radius = (corners & BottomLeft) ? radius : 0,
		br_radius = (corners & BottomRight) ? radius : 0;

	// If all radii are 0 and closed, create a simple rectangle
	if(open_edge == NotOpen && radius <= 0)
	{
		path.addRect(adj_rect);
		return path;
	}

	if(open_edge == NotOpen)
	{
		// Closed rectangle - start from top-left, go clockwise
		path.moveTo(x + tl_radius, y);
		addEdgeWithCorner(path, adj_rect, OpenTop, tr_radius);
		addEdgeWithCorner(path, adj_rect, OpenRight, br_radius);
		addEdgeWithCorner(path, adj_rect, OpenBottom, bl_radius);
		addEdgeWithCorner(path, adj_rect, OpenLeft, tl_radius);
	}
	// Open rectangle at top edge
	else if(open_edge == OpenTop)
	{
		// Open at top - start from top-right, go clockwise, end at top-left
		path.moveTo(x + w, y + tr_radius);
		addEdgeWithCorner(path, adj_rect, OpenRight, br_radius);
		addEdgeWithCorner(path, adj_rect, OpenBottom, bl_radius);
		addEdgeWithCorner(path, adj_rect, OpenLeft, tl_radius);
	}
	else if(open_edge == OpenRight)
	{
		// Open at right - start from bottom-right, go clockwise, end at top-right
		path.moveTo(x + w - br_radius, y + h);
		addEdgeWithCorner(path, adj_rect, OpenBottom, bl_radius);
		addEdgeWithCorner(path, adj_rect, OpenLeft, tl_radius);
		addEdgeWithCorner(path, adj_rect, OpenTop, tr_radius);
	}
	else if(open_edge == OpenBottom)
	{
		// Open at bottom - start from bottom-left, go clockwise, end at bottom-right
		path.moveTo(x, y + h - bl_radius);
		addEdgeWithCorner(path, adj_rect, OpenLeft, tl_radius);
		addEdgeWithCorner(path, adj_rect, OpenTop, tr_radius);
		addEdgeWithCorner(path, adj_rect, OpenRight, br_radius);
	}
	else if(open_edge == OpenLeft)
	{
		// Open at left - start from top-left, go clockwise, end at bottom-left
		path.moveTo(x + tl_radius, y);
		addEdgeWithCorner(path, adj_rect, OpenTop, tr_radius);
		addEdgeWithCorner(path, adj_rect, OpenRight, br_radius);
		addEdgeWithCorner(path, adj_rect, OpenBottom, bl_radius);
	}

	return path;
}

QPixmap CustomUiStyle::createGrayMaskedPixmap(const QPixmap &original) const
{
	if(original.isNull())
		return original;

	// Convert to QImage for desaturation and color blending
	QImage image = original.toImage().convertToFormat(QImage::Format_ARGB32);
	QRgb *line = nullptr, pixel;
	QColor mask_color = qApp->palette().color(QPalette::Disabled, QPalette::Window);
	int gray = 0,
		final_r = 0, final_g = 0, final_b = 0, alpha = 0,
		mask_r = mask_color.red(),
		mask_g = mask_color.green(),
		mask_b = mask_color.blue();

	// Apply desaturation (grayscale conversion) and blend with color
	for(int y = 0; y < image.height(); ++y)
	{
		// Get pointer to the start of the scan line
		line = reinterpret_cast<QRgb *>(image.scanLine(y));

		for(int x = 0; x < image.width(); ++x)
		{
			pixel = line[x];
			alpha = qAlpha(pixel);

			// Apply only to non-transparent pixels
			if(alpha > 0)
			{
				// Convert to grayscale using standard luminance formula
				gray = QColor::fromRgb(pixel).lightness(); // QColor::lightness() uses standard luminance calculation

				// Blend grayscale with the target mask color
				final_r = (gray * (1.0 - BlendFactor)) + (mask_r * BlendFactor);
				final_g = (gray * (1.0 - BlendFactor)) + (mask_g * BlendFactor);
				final_b = (gray * (1.0 - BlendFactor)) + (mask_b * BlendFactor);

				// Ensure values are in valid range [0-255]
				final_r = qBound(0, final_r, 255);
				final_g = qBound(0, final_g, 255);
				final_b = qBound(0, final_b, 255);

				line[x] = qRgba(final_r, final_g, final_b, alpha);
			}
		}
	}

	return QPixmap::fromImage(image);
}

void CustomUiStyle::drawCCComboBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionComboBox *combo_opt =
					qstyleoption_cast<const QStyleOptionComboBox *>(option);

	if(control != CC_ComboBox || !combo_opt || !painter || !widget)
		return;

	// Check if this is an editable combo box
	const QComboBox *combo_widget = qobject_cast<const QComboBox *>(widget);
	bool is_editable = combo_widget && combo_widget->isEditable();

	if(is_editable)
	{
		// For editable combo boxes, draw custom background and border
		drawEditableComboBox(combo_opt, painter, widget);
	}
	else
	{
		// For non-editable combo boxes, use default implementation but with custom border
		QProxyStyle::drawComplexControl(control, option, painter, widget);
		
		// Draw custom focus border if focused
		WidgetState wgt_st(option, widget);
		
		if(wgt_st.is_focused && wgt_st.is_enabled)
		{
			QColor border_color = getStateColor(QPalette::Highlight, option);

			QPainterPath border_shape = createControlShape(combo_opt->rect, InputRadius, AllCorners,
																										0.5, 0.5, -0.5, -0.5);

			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setPen(QPen(border_color, PenWidth));
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(border_shape);
			painter->restore();
		}
	}

	// Draw custom arrow if the drop down button is visible
	if(combo_opt->subControls & SC_ComboBoxArrow)
	{
		QRect arrow_rect = subControlRect(CC_ComboBox, combo_opt, SC_ComboBoxArrow, widget);

		if(!arrow_rect.isEmpty())
		{
			QStyleOption arrow_option = *option;
			arrow_option.rect = arrow_rect;

			// ComboBox arrow always points down
			drawControlArrow(&arrow_option, painter, widget, DownArrow);
		}
	}
}

void CustomUiStyle::drawCCGroupBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionGroupBox *group_box_opt =
					qstyleoption_cast<const QStyleOptionGroupBox *>(option);

	if(control != CC_GroupBox || !option || !painter || !widget)
		return;

	painter->save();

	const QGroupBox *group_box = qobject_cast<const QGroupBox *>(widget);
	bool is_checkable = group_box && group_box->isCheckable(),
		is_checked = group_box && group_box->isChecked();

	QRect group_rect = group_box_opt->rect,
			title_rect, frame_rect = group_rect;

	// Calculate title area if there's text
	bool has_title = !group_box_opt->text.isEmpty();

	// Get checkbox size if the GroupBox is checkable
	int checkbox_size = 0,
		checkbox_spacing = 4; // Space between checkbox and text

	if(is_checkable)
		checkbox_size = pixelMetric(PM_IndicatorWidth, group_box_opt, widget);

	if(has_title)
	{
		// Create bold font with 80% size to calculate text height
		QFont title_font = painter->font();
		title_font.setBold(true);
		title_font.setPointSizeF(title_font.pointSizeF() * GrpBoxTitleFontSize);

		QFontMetrics fm(title_font);
		int text_height = fm.height();
		int total_title_height = text_height + (2 * GrpBoxTitlePadding);

		// Title takes the top portion including padding
		title_rect = QRect(group_rect.left(), group_rect.top(),
											 group_rect.width(), total_title_height);

		// Frame starts below the title and extends to the bottom of the group box
		int frame_top = group_rect.top() + total_title_height;
		int frame_height = group_rect.bottom() - frame_top + 1;
		frame_rect = QRect(group_rect.left(), frame_top,
											 group_rect.width(), frame_height);
	}

	// Draw the frame below the title
	if(!frame_rect.isEmpty())
	{
		QStyleOptionFrame frame_opt;
		frame_opt.QStyleOption::operator=(*group_box_opt);
		frame_opt.features = QStyleOptionFrame::None;
		frame_opt.rect = frame_rect; // Use adjusted frame rectangle
		drawPrimitive(PE_FrameGroupBox, &frame_opt, painter, widget);
	}

	// Draw the title above the frame
	if(has_title && !title_rect.isEmpty())
	{
		// Create bold font with 80% size
		QFont title_font = painter->font();

		title_font.setBold(true);
		title_font.setPointSizeF(title_font.pointSizeF() * 0.80);
		painter->setFont(title_font);

		// Use state-aware text color
		painter->setPen(getStateColor(QPalette::WindowText, group_box_opt));

		// Calculate text rectangle with padding
		QRect text_rect = title_rect.adjusted(0, GrpBoxTitlePadding, 0, -GrpBoxTitlePadding);

		// Draw checkbox if the GroupBox is checkable
		if(is_checkable && checkbox_size > 0)
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);

			// Calculate checkbox position - vertically centered in title area
			int checkbox_x = text_rect.left();
			int checkbox_y = text_rect.top() + ((text_rect.height() - checkbox_size) / 2);
			QRectF checkbox_rect(checkbox_x, checkbox_y, checkbox_size, checkbox_size);

			// Prepare colors for checkbox drawing (same as drawPECheckBoxRadioBtn)
			QColor border_color = getAdjustedColor(getStateColor(QPalette::Dark, group_box_opt), MidFactor, NoFactor),
				   bg_color = getStateColor(QPalette::Base, group_box_opt),
				   ind_color = getAdjustedColor(getStateColor(QPalette::Highlight, group_box_opt), MidFactor, -XMinFactor);

			WidgetState wgt_st(group_box_opt, widget);

			if(!wgt_st.is_enabled)
				ind_color = getStateColor(QPalette::Mid, group_box_opt);

			// Draw checkbox background
			painter->setBrush(bg_color);
			painter->setPen(QPen(border_color, PenWidth));
			checkbox_rect.adjust(0.5, 0.5, -0.5, -0.5);
			painter->drawRoundedRect(checkbox_rect, 2, 2);

			// Draw indicator if checked
			if(is_checked)
			{
				painter->setBrush(ind_color);
				painter->setPen(Qt::NoPen);
				checkbox_rect.adjust(2, 2, -2, -2);
				painter->drawRoundedRect(checkbox_rect, 1, 1);
			}

			painter->restore();

			// Offset the text to the right of the checkbox
			text_rect.adjust(checkbox_size + checkbox_spacing, 0, 0, 0);
		}

		// Draw the text in the title area (centered vertically)
		painter->drawText(text_rect,
						group_box_opt->textAlignment | Qt::AlignVCenter,
						group_box_opt->text);
	}

	painter->restore();
}

void CustomUiStyle::drawCCSpinBox(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionSpinBox *spin_opt = qstyleoption_cast<const QStyleOptionSpinBox *>(option);

	if(control != CC_SpinBox || !spin_opt || !painter || !widget)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRect edit_field_rect = subControlRect(CC_SpinBox, spin_opt, SC_SpinBoxEditField, widget);
	QStyleOptionSpinBox aux_sp_opt = *spin_opt;

	// Draw edit field if visible
	if(spin_opt->subControls & SC_SpinBoxEditField && !edit_field_rect.isEmpty())
		drawSpinBoxEditField(spin_opt, painter, widget);

	// Draw up buttons if visible
	drawSpinBoxButton(spin_opt, painter, widget, SC_SpinBoxUp);
	drawSpinBoxButton(spin_opt, painter, widget, SC_SpinBoxDown);

	painter->restore();
}

void CustomUiStyle::drawCETabBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionTab *tab_opt = qstyleoption_cast<const QStyleOptionTab *>(option);

	// Handle individual QTabBar tabs with flat design styling
	if(element != CE_TabBarTab || !tab_opt || !painter || !widget)
		return;

	QColor bg_color = getAdjustedColor(getStateColor(QPalette::Dark, option), MinFactor, MinFactor + 5),
				 border_color = getAdjustedColor(getStateColor(QPalette::Mid, option), MinFactor, -XMinFactor);

	WidgetState wgt_st(tab_opt, widget);
	QRect tab_rect = tab_opt->rect;
	QTabBar::Shape shape = tab_opt->shape;

	if(!wgt_st.is_selected)
	{
		bg_color = getAdjustedColor(bg_color, -MinFactor, -XMinFactor - 5);
		border_color = getAdjustedColor(border_color, -MidFactor, -XMinFactor);
	}

	// Determine corner flags and edge opening based on tab shape
	CornerFlag corner_flags = NoCorners;
	OpenEdge open_edge = NotOpen;
	int dh = 0, dw = 0, dx = 0, dy = 0;

	if(shape == QTabBar::RoundedNorth || shape == QTabBar::RoundedSouth)
	{
		// North tabs: round top corners, open bottom edge
		corner_flags = (shape == QTabBar::RoundedNorth ? (TopLeft | TopRight) : (BottomLeft | BottomRight));
		open_edge = (shape == QTabBar::RoundedNorth ? OpenBottom : OpenTop);

		dh = wgt_st.is_selected ? 3 : 6;
		dy = wgt_st.is_selected ? 1 : 2;

		// For North tabs, we need to move the tab down
		if(shape == QTabBar::RoundedNorth)
			tab_rect.moveTop(tab_rect.top() + dy);

		tab_rect.setHeight(tab_rect.height() - dh);
		tab_rect.translate(0, dy);
	}
	else if(shape == QTabBar::RoundedWest || shape == QTabBar::RoundedEast)
	{
		// West tabs: round left corners, open right edge
		corner_flags = (shape == QTabBar::RoundedWest ? (TopLeft | BottomLeft) : (TopRight | BottomRight));
		open_edge = (shape == QTabBar::RoundedWest ? OpenRight : OpenLeft);

		dw = wgt_st.is_selected ? 2 : 4;
		dx = wgt_st.is_selected ? 1 : 2;
		tab_rect.setWidth(tab_rect.width() - dw);
		tab_rect.translate(dx, 0);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw tab with appropriate corners rounded and edge opened
	if(corner_flags != NoCorners)
	{
		QPainterPath bg_path, border_path;

		border_path = createControlShape(tab_rect, TabBarRadius,
						corner_flags, 0.5, 0.5, -0.5, -0.5, open_edge);

		qreal ajx = 0, ajy = 0, ajw = 0, ajh = 0;

		/* We create a slightly larger background path to avoid artifacts at the border
		 * at the junction of the tab bar and tab widget body. For each open side we have
		 * specific adjustments factors at x, y, width and height */
		if(open_edge == OpenBottom)
		{
			ajx = 1;
			ajy = 0;
			ajw = -1;
			ajh = wgt_st.is_selected ? 2 : 0;
		}
		else if(open_edge == OpenTop)
		{
			ajx = 1;
			ajy = wgt_st.is_selected ? -2 : 0;
			ajw = 0;
			ajh = -1;
		}
		else if(open_edge == OpenRight)
		{
			ajx = 1;
			ajy = 1;
			ajw = wgt_st.is_selected ? 2 : 0;
			ajh = -1;
		}
		else if(open_edge == OpenLeft)
		{
			ajx = wgt_st.is_selected ? -2 : 0;
			ajy = 1;
			ajw = -1;
			ajh = -1;
		}

		bg_path = createControlShape(tab_rect, TabBarRadius,
						corner_flags, ajx, ajy, ajw, ajh, open_edge);

		// Draw background
		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawPath(bg_path);

		// Draw border
		painter->setPen(QPen(border_color, PenWidth, Qt::SolidLine, Qt::FlatCap));
		painter->drawPath(border_path);
	}
	else
	{
		// Fallback for unsupported shapes
		qDebug() << "CustomUiStyle::drawCETabBar():" << shape << "not fully implemented, drawing rectangle instead.";
		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawRect(tab_rect);

		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawRect(tab_rect);
	}

	painter->restore();

	// Draw the tab text with proper contrast
	QProxyStyle::drawControl(CE_TabBarTabLabel, tab_opt, painter, widget);
}

void CustomUiStyle::drawComplexControl(ComplexControl control, const QStyleOptionComplex *option, QPainter *painter, const QWidget *widget) const
{
	if(control == CC_GroupBox)
	{
		drawCCGroupBox(control, option, painter, widget);
		return;
	}

	if(control == CC_SpinBox)
	{
		drawCCSpinBox(control, option, painter, widget);
		return;
	}

	if(control == CC_ComboBox)
	{
		drawCCComboBox(control, option, painter, widget);
		return;
	}

	if(control == CC_ScrollBar)
	{
		drawCCScrollBar(option, painter, widget);
		return;
	}

	QProxyStyle::drawComplexControl(control, option, painter, widget);
}

void CustomUiStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == CE_HeaderSection)
	{
		drawCEHeaderSection(element, option, painter, widget);
		return;
	}

	if(element == CE_ProgressBar ||
		 element == CE_ProgressBarContents ||
		 element == CE_ProgressBarGroove ||
		 element == CE_ProgressBarLabel)
	{
		drawCEProgressBar(element, option, painter, widget);
		return;
	}

	if(element == CE_TabBarTab)
	{
		drawCETabBar(element, option, painter, widget);
		return;
	}

	if(element == CE_Splitter)
	{
		drawCESplitter(element, option, painter, widget);
		return;
	}

	if(element == CE_MenuItem)
	{
		drawCEMenuItem(element, option, painter, widget);
		return;
	}

	if(element == CE_CheckBox || element == CE_RadioButton)
	{
		drawCECheckBoxRadioButton(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
	qreal curr_opacity = painter->opacity();

	/*If opacity is low, draw grayed pixmap and return
	 *indicating a disabled state */
	if(curr_opacity < 0.9)
	{
		painter->save();
		QProxyStyle::drawItemPixmap(painter, rect, alignment,
						createGrayMaskedPixmap(pixmap));
		painter->restore();
		return;
	}

	QProxyStyle::drawItemPixmap(painter, rect, alignment, pixmap);
}

void CustomUiStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element == PE_PanelButtonTool || element == PE_PanelButtonCommand)
	{
		QStyleOption adjusted_opt;
		const QStyleOption *curr_opt = option;

		// Adjust button rect for QTabBar scroll buttons to fit inside the tab bar		
		if(widget &&
			 (widget->objectName() == "ScrollLeftButton" ||
				widget->objectName() == "ScrollRightButton"))
		{
			adjusted_opt = *option;
			adjusted_opt.rect.adjust(1, 1, -1, -1);
			adjusted_opt.rect.moveTo(adjusted_opt.rect.left() + 1, adjusted_opt.rect.top() + 1);
			curr_opt = &adjusted_opt;
		}
		
		drawPEButtonPanel(element, curr_opt, painter, widget);
		drawPEGenericElemFrame(PE_FrameButtonTool, curr_opt, painter, widget, ButtonRadius);
		
		return;
	}

	if(element == PE_PanelLineEdit)
	{
		/* Don't draw panel and frame if this LineEdit is part of a SpinBox or ComboBox.
		 * The method drawSpinBoxEditField handle all drawing for SpinBox controls
		 * and drawCCComboBox handles all drawing for ComboBox controls */
		if(!widget ||
			 (!qobject_cast<const QAbstractSpinBox *>(widget->parentWidget()) &&
				!qobject_cast<const QComboBox *>(widget->parentWidget())))
		{
			drawPELineEditPanel(element, option, painter, widget);
			drawPEGenericElemFrame(PE_FrameLineEdit, option, painter, widget, InputRadius);
		}

		return;
	}

	if(element == PE_FrameTabWidget)
	{
		drawPETabWidgetFrame(element, option, painter, widget);
		return;
	}

	if(element == PE_FrameGroupBox)
	{
		drawPEGroupBoxFrame(element, option, painter, widget);
		return;
	}

	if(element == PE_IndicatorCheckBox || element == PE_IndicatorRadioButton)
	{
		drawPECheckBoxRadioBtn(element, option, painter, widget);
		return;
	}

	if(element == PE_Frame)
	{
		// Don't draw frame if this is part of a SpinBox edit field
		if(!widget || !qobject_cast<const QAbstractSpinBox *>(widget))
		{
			drawPEHintFramePanel(element, option, painter, widget);
			drawPEGenericElemFrame(element, option, painter, widget, NoRadius);
		}

		return;
	}

	// Handle progress bar primitive elements
	if(element == PE_IndicatorProgressChunk)
	{
		drawPEProgressChunk(option, painter, widget);
		return;
	}

	// Handle header sort arrow indicators
	if(element == PE_IndicatorHeaderArrow)
	{
		drawPEHeaderArrow(option, painter, widget);
		return;
	}

	// Handle QTabBar scroll button arrows (left/right navigation)
	if((element == PE_IndicatorArrowLeft ||
			element == PE_IndicatorArrowRight) &&
			widget &&
		 (widget->objectName() == "ScrollLeftButton" ||
			widget->objectName() == "ScrollRightButton"))
	{
		// Adjust arrow position to match the button size and position
		QStyleOption scroll_btn_opt = *option;

		// Same adjustment as button: reduce height and move down
		scroll_btn_opt.rect.adjust(1, 1, -1, -1);
		scroll_btn_opt.rect.moveTo(scroll_btn_opt.rect.left() + 1,
															 scroll_btn_opt.rect.top() + 1);
		
		ArrowType arrow_type = (element == PE_IndicatorArrowLeft) ? LeftArrow : RightArrow;
		drawControlArrow(&scroll_btn_opt, painter, widget, arrow_type);
		
		return;
	}

	// Handle QToolButton and QPushButton menu arrow positioning
	if(element == PE_IndicatorArrowDown && widget &&
		 (qobject_cast<const QToolButton *>(widget) ||
			qobject_cast<const QPushButton *>(widget)))
	{
		drawButtonMenuArrow(option, painter, widget);
		return;
	}

	// Handle tooltip rendering with custom border
	if(element == PE_PanelTipLabel)
	{
		drawPEToolTip(element, option, painter, widget);
		return;
	}

	// Handle menu panel rendering with custom background and border
	if(element == PE_PanelMenu)
	{
		drawPEMenuPanel(element, option, painter, widget);
		return;
	}

	QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void CustomUiStyle::setPixelMetricValue(QStyle::PixelMetric metric, int value)
{
	pixel_metrics[metric] = value;
}

int CustomUiStyle::pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
	if(pixel_metrics.contains(metric))
		return pixel_metrics[metric];

	// Use the default pixel metric attribute value if there's no custom value defined
	return QProxyStyle::pixelMetric(metric, option, widget);
}

QRect CustomUiStyle::subControlRect(ComplexControl control, const QStyleOptionComplex *option, SubControl sub_control, const QWidget *widget) const
{
	QRect rect = QProxyStyle::subControlRect(control, option, sub_control, widget);

	// Adjust GroupBox content rectangle to account for custom title height
	if(control == CC_GroupBox && sub_control == SC_GroupBoxContents)
	{
		const QStyleOptionGroupBox *group_box_opt = qstyleoption_cast<const QStyleOptionGroupBox *>(option);

		if(group_box_opt && !group_box_opt->text.isEmpty())
		{
			// Create bold font with 80% size to calculate text height (same as in drawCCGroupBox)
			QFont title_font = widget ? widget->font() : QApplication::font();
			title_font.setBold(true);
			title_font.setPointSizeF(title_font.pointSizeF() * GrpBoxTitleFontSize);

			QFontMetrics fm(title_font);
			int text_height = fm.height(),
					total_title_height = text_height + (2 * GrpBoxTitlePadding);

			// Get the default rect from the base style
			QRect default_rect = QProxyStyle::subControlRect(control, option, sub_control, widget);

			/* Adjust the top position to account for our custom title height
			 * The frame starts at total_title_height, so content should start a bit below that */
			int frame_margin = pixelMetric(PM_DefaultFrameWidth, option, widget) + 1;

			// Adjusting the rect so the top/bottom margins are the same as left/right margins
			rect = QRect(default_rect.left() - 1,
									 group_box_opt->rect.top() + total_title_height + frame_margin,
									 default_rect.width() + 2,
									 group_box_opt->rect.height() - total_title_height - (frame_margin * 2));
		}
	}

	return rect;
}

void CustomUiStyle::polish(QWidget *widget)
{
	QProxyStyle::polish(widget);

	/* Apply custom styling to QTableView/QTableWidget
   * this is necessary to ensure that the grid lines
	 * adapt to the current theme (light/dark) */
	if(widget->metaObject()->className() == QString("QTableView") ||
		 widget->metaObject()->className() == QString("QTableWidget"))
	{
		QPalette pal = widget->palette();
		QColor grid_color = getAdjustedColor(pal.color(QPalette::Mid), NoFactor, MaxFactor);
		pal.setColor(QPalette::Window, grid_color);
		widget->setPalette(pal);
		widget->setAutoFillBackground(true);
	}

	// Install event filter for widgets with hover effects
	if(qobject_cast<QLineEdit *>(widget) ||
		 qobject_cast<QPlainTextEdit *>(widget) ||
		 qobject_cast<QTextEdit *>(widget) ||
		 qobject_cast<QPushButton *>(widget) ||
		 qobject_cast<QToolButton *>(widget) ||
		 qobject_cast<QCheckBox *>(widget) ||
		 qobject_cast<QRadioButton *>(widget) ||
		 qobject_cast<QComboBox *>(widget) ||
		 qobject_cast<QSpinBox *>(widget) ||
		 qobject_cast<QDoubleSpinBox *>(widget))
	{
		// Enable hover events for these widgets
		widget->setAttribute(Qt::WA_Hover);
		widget->installEventFilter(const_cast<CustomUiStyle *>(this));
	}
}

QPolygonF CustomUiStyle::rotatePolygon(const QPolygonF &polygon, qreal degrees)
{
	if(polygon.isEmpty())
		return polygon;

	QPointF center = polygon.boundingRect().center();

	// Create transformation matrix for rotation around center
	QTransform transform;
	transform.translate(center.x(), center.y());
	transform.rotate(degrees);
	transform.translate(-center.x(), -center.y());

	// Apply transformation to the polygon
	return transform.map(polygon);
}

void CustomUiStyle::drawButtonMenuArrow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QToolButton *tool_btn = qobject_cast<const QToolButton *>(widget);
	const QPushButton *push_btn = qobject_cast<const QPushButton *>(widget);

	if(!option || !painter || !tool_btn && !push_btn)
		return;

	/* Check if the button has a menu associated otherwise no arrow is drawn
	 * QToolButton that are icon only but have a popup menu will not have
	 * the arrow drawn as well */
	if((tool_btn && (tool_btn->toolButtonStyle() == Qt::ToolButtonIconOnly || (tool_btn->popupMode() != QToolButton::InstantPopup && tool_btn->popupMode() != QToolButton::DelayedPopup)) || (push_btn && !push_btn->menu())))
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRect btn_rect = widget->rect();
	int v_spc = 4, h_spc = 3;

	// Determine arrow arr_type and position based on button type and style
	ArrowType arr_type;
	QRect arr_rect;
	bool btn_txt_under_icon = (tool_btn && tool_btn->toolButtonStyle() == Qt::ToolButtonTextUnderIcon);

	// For QToolButton, check toolButtonStyle; for QPushButton, always use down arrow at bottom-right
	if(btn_txt_under_icon)
	{
		// TextUnderIcon QToolButton: arrow pointing right, positioned at right-center
		arr_type = RightArrow;
		arr_rect = QRect(btn_rect.right() - h_spc - ArrowWidth,
						btn_rect.center().y() - (ArrowHeight / 2),
						ArrowWidth, ArrowHeight);
	}
	else
	{
		// Other QToolButton layouts or any QPushButton: arrow pointing down, positioned at right-center
		arr_type = DownArrow;
		arr_rect = QRect(btn_rect.right() - h_spc - ArrowWidth,
						btn_rect.bottom() - v_spc - ArrowHeight,
						ArrowWidth, ArrowHeight);
	}

	// Create modified option with new rect
	QStyleOption arrow_opt = *option;
	arrow_opt.rect = arr_rect;

	// Use our custom arrow drawing
	drawControlArrow(&arrow_opt, painter, widget, arr_type, !btn_txt_under_icon);

	painter->restore();
}

void CustomUiStyle::drawControlArrow(const QStyleOption *option, QPainter *painter, const QWidget *,
																		 ArrowType arr_type, bool small_sz) const
{
	if(!option || !painter)
		return;

	// Use text color that adapts to the button state and theme
	QColor arr_color = getStateColor(QPalette::ButtonText, option);
	WidgetState wgt_st(option, nullptr);

	// Adjust arrow color based on button state for better visibility
	if(wgt_st.is_enabled && wgt_st.is_pressed)
		arr_color = arr_color.darker(MinFactor); // Slightly darker when pressed

	// Calculate arrow geometry - fixed size calculation to ensure consistency
	QRect btn_rect = option->rect;

	// Calculate precise center to avoid rounding issues
	QPointF center = QPointF(btn_rect.x() + (btn_rect.width() / 2.0),
					btn_rect.y() + (btn_rect.height() / 2.0));

	// Round to pixel boundaries for crisp rendering
	qreal half_w = qRound(ArrowWidth * 0.5) * (small_sz ? 0.70 : 1.0),
		  half_h = qRound(ArrowHeight * 0.5) * (small_sz ? 0.70 : 1.0);

	// Create base arrow pointing UP (triangle pointing up)
	QPolygonF base_arrow;
	base_arrow << QPointF(center.x(), center.y() - half_h)			 // Top point
			   << QPointF(center.x() - half_w, center.y() + half_h)	 // Bottom left
			   << QPointF(center.x() + half_w, center.y() + half_h); // Bottom right

	// Apply rotations based on arrow arr_type
	QPolygonF arrow;

	switch(arr_type)
	{
	case UpArrow:
		// Arrow pointing up: Use base arrow
		arrow = base_arrow;
		break;

	case DownArrow:
		// Arrow pointing down: Rotate base arrow 180°
		arrow = rotatePolygon(base_arrow, 180);
		break;

	case LeftArrow:
		// Arrow pointing left: Rotate base arrow 270° clockwise
		arrow = rotatePolygon(base_arrow, 270);
		break;

	case RightArrow:
		// Arrow pointing right: Rotate base arrow 90° clockwise
		arrow = rotatePolygon(base_arrow, 90);
		break;
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw the arrow
	painter->setBrush(arr_color);
	painter->setPen(Qt::NoPen);
	painter->drawPolygon(arrow);

	painter->restore();
}

void CustomUiStyle::drawScrollBarButton(const QStyleOptionSlider *option, QPainter *painter, const QWidget *widget,
				QStyle::SubControl button_type, const QColor &bg_color, const QColor &border_color) const
{
	// Get the appropriate rectangle for this button type
	QRectF btn_rect = subControlRect(CC_ScrollBar, option, button_type, widget);

	if(!option || !painter || !widget || btn_rect.isEmpty() || (button_type != SC_ScrollBarAddLine && button_type != SC_ScrollBarSubLine))
		return;

	WidgetState wgt_st(option, widget);

	// Start with base colors and apply state-based modifications
	QColor btn_bg = bg_color,
		   btn_border = border_color;

	// Apply state-based colors if this specific button is active
	if(option->activeSubControls & button_type)
	{
		if(wgt_st.is_pressed)
		{
			// Pressed: darker in both themes
			btn_bg = getAdjustedColor(bg_color, -XMinFactor, -XMinFactor);
			btn_border = getAdjustedColor(border_color, -XMinFactor, -XMinFactor);
		}
		else if(wgt_st.is_hovered)
		{
			// Hover: lighter in both themes (subtle effect)
			btn_bg = getAdjustedColor(bg_color, XMinFactor, XMinFactor);
			btn_border = getAdjustedColor(border_color, XMinFactor, XMinFactor);
		}
	}

	// Determine scrollbar orientation and open edge for button border
	const QScrollBar *scrollbar = qobject_cast<const QScrollBar *>(widget);
	bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
	
	OpenEdge open_edge = NotOpen;
	
	if(is_horizontal)
	{
		// Horizontal scrollbar
		if(button_type == SC_ScrollBarAddLine)
			open_edge = OpenLeft;   // AddLine (right button): omit left border
		else
			open_edge = OpenRight;  // SubLine (left button): omit right border
	}
	else
	{
		// Vertical scrollbar
		if(button_type == SC_ScrollBarAddLine)
			open_edge = OpenTop;    // AddLine (bottom button): omit top border
		else
			open_edge = OpenBottom; // SubLine (top button): omit bottom border
	}

	// Create button shape with omitted edge (no rounded corners)
	QPainterPath btn_shape = createControlShape(btn_rect.toRect(), NoRadius, NoCorners, 
																							0, 0, 0, 0, open_edge);

	// Draw button background
	painter->setBrush(btn_bg);
	painter->setPen(Qt::NoPen);
	painter->drawPath(btn_shape);

	// Draw button border with omitted edge for fusion effect
	QPainterPath btn_border_shape = createControlShape(btn_rect.toRect(), NoRadius, NoCorners,
																											0.5, 0.5, -0.5, -0.5, open_edge);
	
	painter->setPen(QPen(btn_border, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(btn_border_shape);

	// Create option for arrow drawing with proper state
	QStyleOption arrow_opt = *option;

	arrow_opt.rect = btn_rect.toRect();

	// Set proper arrow state based on button activity
	if(option->activeSubControls & button_type)
	{
		if(wgt_st.is_pressed)
			arrow_opt.state |= State_Sunken;
		else if(wgt_st.is_hovered)
			arrow_opt.state |= State_MouseOver;
	}
	else
		arrow_opt.state &= ~(State_MouseOver | State_Sunken);

	// Determine arrow direction based on button type and scrollbar orientation
	ArrowType arrow_dir;

	if(button_type == SC_ScrollBarAddLine)
		// AddLine is bottom/right button
		arrow_dir = is_horizontal ? RightArrow : DownArrow;
	else // SC_ScrollBarSubLine
		// SubLine is top/left button
		arrow_dir = is_horizontal ? LeftArrow : UpArrow;

	// Draw the arrow
	drawControlArrow(&arrow_opt, painter, widget, arrow_dir);
}

void CustomUiStyle::drawEditableComboBox(const QStyleOptionComboBox *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Get colors for background and border
	QColor bg_color = getStateColor(QPalette::Base, option);
	QColor border_color = getStateColor(QPalette::Dark, option).lighter(MaxFactor);

	WidgetState wgt_st(option, widget);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.is_focused)
			border_color = getStateColor(QPalette::Highlight, option);
		else if(wgt_st.is_hovered)
		{
			bg_color = bg_color.lighter(MaxFactor);
			border_color = border_color.lighter(MaxFactor);
		}
	}

	// Create shape with all corners rounded
	QPainterPath combo_shape = createControlShape(option->rect, InputRadius, AllCorners);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(combo_shape);

	// Draw border
	QPainterPath border_shape = createControlShape(option->rect, InputRadius, AllCorners,
					0.5, 0.5, -0.5, -0.5);
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(border_shape);

	painter->restore();
}

void CustomUiStyle::drawPEButtonPanel(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_PanelButtonTool && element != PE_PanelButtonCommand) || !option || !painter || !widget)
		return;

	WidgetState wgt_st(option, widget);
	QColor bg_color = getStateColor(QPalette::Button, option);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.has_custom_color && !wgt_st.is_pressed)
			bg_color = getStateColor(widget->palette(), QPalette::Button, option);
		else if(!wgt_st.is_pressed && !wgt_st.is_focused &&
						(wgt_st.is_default || wgt_st.is_checked))
		{
			QColor base_bg_color = getStateColor(QPalette::Highlight, option);
			
			if(wgt_st.is_hovered)
				bg_color = getAdjustedColor(base_bg_color, MinFactor, MinFactor);
			else
				bg_color = getAdjustedColor(base_bg_color, NoFactor, NoFactor);
		}
		else if(wgt_st.is_pressed)
			bg_color = getAdjustedColor(getStateColor(QPalette::Dark, option), NoFactor, NoFactor);
		else if(wgt_st.is_hovered)
			bg_color = getAdjustedColor(getStateColor(QPalette::Light, option), NoFactor, XMinFactor);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawRoundedRect(option->rect, ButtonRadius, ButtonRadius);
	painter->restore();
}

void CustomUiStyle::drawPECheckBoxRadioBtn(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != PE_IndicatorCheckBox && element != PE_IndicatorRadioButton) || !option || !painter)
		return;

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	QRectF obj_rect = option->rect;
	WidgetState wgt_st(option, widget);

	QColor border_color = getAdjustedColor(getStateColor(QPalette::Dark, option), MidFactor, NoFactor),
		   bg_color = getStateColor(QPalette::Base, option),
		   ind_color = getAdjustedColor(getStateColor(QPalette::Highlight, option), MidFactor, -XMinFactor);

	if(!wgt_st.is_enabled)
		ind_color = getStateColor(QPalette::Mid, option);
	else if(wgt_st.is_pressed)
	{
		ind_color = getStateColor(QPalette::Midlight, option);
		bg_color = getAdjustedColor(bg_color, MidFactor, -XMinFactor);
		border_color = getAdjustedColor(border_color, MidFactor, -XMinFactor);
	}

	// Draw checkbox background
	painter->setBrush(bg_color);
	painter->setPen(QPen(border_color, PenWidth));

	obj_rect.adjust(0.5, 0.5, -0.5, -0.5);
	obj_rect.translate(0, 1); // Ensure using float coordinates

	if(element == PE_IndicatorCheckBox)
		painter->drawRoundedRect(obj_rect, 2, 2);
	else // PE_IndicatorRadioButton
		painter->drawEllipse(obj_rect);

	if(wgt_st.is_checked)
	{
		// Draw the indicator rectangle
		painter->setBrush(ind_color);
		painter->setPen(Qt::NoPen);
		obj_rect.adjust(2, 2, -2, -2);

		if(element == PE_IndicatorCheckBox)
			painter->drawRoundedRect(obj_rect, 1, 1);
		else
			painter->drawEllipse(obj_rect);
	}

	painter->restore();
}

void CustomUiStyle::drawCECheckBoxRadioButton(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if((element != CE_CheckBox && element != CE_RadioButton) || !option || !painter || !widget)
		return;

	WidgetState wgt_st(option, widget);

	// Draw hover background first (behind everything)
	if(wgt_st.is_hovered && wgt_st.is_enabled)
	{
		// Use button-like hover color with adjustments for theme
		QColor hover_color = getAdjustedColor(getStateColor(QPalette::Button, option), MinFactor, -XMinFactor);
		hover_color.setAlpha(100); // Semi-transparent

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setBrush(hover_color);
		painter->setPen(Qt::NoPen);

		QRectF hover_rect = option->rect;
		painter->drawRoundedRect(hover_rect, ChkRadioFocusRadius, ChkRadioFocusRadius);

		painter->restore();
	}

	// Then, let the default implementation draw the control (indicator + text)
	QProxyStyle::drawControl(element, option, painter, widget);

	// Finally, draw focus border around the entire widget if focused
	if(wgt_st.is_focused && wgt_st.is_enabled)
	{
		QColor border_color = getStateColor(QPalette::Highlight, option);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		// Draw focus border around the entire control (indicator + text)
		painter->setBrush(Qt::NoBrush);
		painter->setPen(QPen(border_color, PenWidth));

		QRectF focus_rect = option->rect;
		focus_rect.adjust(0.5, 0.5, -0.5, -0.5);

		painter->drawRoundedRect(focus_rect, ChkRadioFocusRadius, ChkRadioFocusRadius);

		painter->restore();
	}
}

void CustomUiStyle::drawPEHintFramePanel(PrimitiveElement element, const QStyleOption *option,
																			 QPainter *painter, const QWidget *widget) const
{
	// Check if widget is a QFrame and has a StyleHint set
	const QFrame *frame = qobject_cast<const QFrame *>(widget);
	StyleHint hint = frame ? static_cast<StyleHint>(frame->property(StyleHintProp).toInt()) : NoHint;

	if(element != PE_Frame || !option || !painter || !widget || hint == NoHint)
		return;

	// Get the style hint color	
	WidgetState wgt_st(option, frame);

	// Get the light color from palette for blending
	QColor base_color = getStateColor(QPalette::Light, option),

	/* Blend the hint color with the light color
	 * Use a lower blend factor (0.3) to give more weight to light */
	bg_color;

	if(!wgt_st.is_enabled)
		bg_color = getStateColor(QPalette::Dark, option);
	else 
	{	
		// For DefaultFrmHint we use the midlight color as background
		if(hint == DefaultFrmHint)
			bg_color = getStateColor(isDarkPalette() ? QPalette::Midlight : QPalette::Light, option);
		// For AltDefFrmHint we use the same background as QGroupBox
		else if(hint == AltDefaultFrmHint)
			bg_color = getAdjustedColor(getStateColor(QPalette::Dark, option), XMinFactor, MinFactor);
		else
		{
			if(isDarkPalette())
				bg_color = getAdjustedColor(bg_color, XMinFactor, NoFactor);
			else
				bg_color = getAdjustedColor(bg_color, NoFactor, -XMinFactor);

			QColor hint_color = frame->property(StyleHintColor).value<QColor>();

			// Blend hint color with base light color
			bg_color.setRedF((hint_color.redF() * 0.25) + (base_color.redF() * 0.75));
			bg_color.setGreenF((hint_color.greenF() * 0.25) + (base_color.greenF() * 0.75));
			bg_color.setBlueF((hint_color.blueF() * 0.25) + (base_color.blueF() * 0.75));
		}
	}

	// Create the shape with frame radius plus one pixel for a better context
	QPainterPath shape = createControlShape(option->rect, HintFrameRadius, AllCorners);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(Qt::NoPen);
	painter->setBrush(bg_color);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPEGenericElemFrame(PrimitiveElement element, const QStyleOption *option,
				QPainter *painter, const QWidget *widget, int border_radius) const
{
	if(!option || !painter || !widget)
		return;

	QColor border_color = getAdjustedColor(getStateColor(QPalette::Midlight, option), NoFactor, -XMinFactor);
	WidgetState wgt_st(option, widget);
	qreal pen_width = PenWidth;

	/* Detecting if its a line edit frame, because some states
	 * are not rendered for it, like hover and pressed */
	bool is_edit_frm = (element == PE_FrameLineEdit),
		 is_basic_frm = (element == PE_Frame);

	/* The widget has a style hint property set, we use it to 
	 * render a specific border color and radius. */
	StyleHint hint = static_cast<StyleHint>(widget->property(StyleHintProp).toInt());

	if(hint != NoHint)
	{
		if(wgt_st.is_enabled)
		{
			// For DefaultFrmHint we use a standard border color based on theme
			if(hint == DefaultFrmHint)
				border_color = getStateColor(isDarkPalette() ? QPalette::Light : QPalette::Midlight, option);
			// For AltDefFrmHint we use the same border color as QGroupBox
			else if(hint == AltDefaultFrmHint)
				border_color = getAdjustedColor(getStateColor(QPalette::Mid, option), XMinFactor, -XMinFactor);
			else
				// For other hints, use the custom color with slight adjustments
				border_color = getAdjustedColor(widget->property(StyleHintColor).value<QColor>(), XMinFactor, -XMinFactor);
		}

		border_radius = HintFrameRadius;
	}

	if(wgt_st.is_enabled)
	{
		if(wgt_st.has_custom_color && !wgt_st.is_pressed)
		{
			border_color = getStateColor(widget->palette(), QPalette::Button, option);
			border_color = border_color.lighter(QColor(border_color).lightness() < 128 ? MidFactor : MaxFactor);
		}
		// Handle default/checked buttons first (they have special coloring)
		else if(!wgt_st.is_focused && !wgt_st.is_pressed && (wgt_st.is_default || wgt_st.is_checked))
		{
			QColor base_border_cl = getStateColor(QPalette::Highlight, option);

			if(wgt_st.is_hovered)
				border_color = getAdjustedColor(base_border_cl, MidFactor, XMinFactor);
			else
				border_color = getAdjustedColor(base_border_cl, MidFactor, -MinFactor);
		}
		// Handle pressed state
		else if(wgt_st.is_pressed && !is_edit_frm && !is_basic_frm)
			border_color = getAdjustedColor(getStateColor(QPalette::Button, option), NoFactor, -MidFactor);
		// Handle focused state for non-default buttons
		else if(wgt_st.is_focused && !wgt_st.is_default && !is_edit_frm && !is_basic_frm)
			border_color = getStateColor(QPalette::Highlight, option);
		// Handle hover state for buttons
		else if(wgt_st.is_hovered && !is_edit_frm && !is_basic_frm)
			border_color = getAdjustedColor(getStateColor(QPalette::Light, option), MinFactor, -XMinFactor);
		// Handle hover state for edit fields and frames (lighter border) but not when focused
		else if(wgt_st.is_hovered && !wgt_st.is_focused && (is_edit_frm || is_basic_frm))
			border_color = border_color.lighter(MaxFactor);
		// Handle focused state for edit fields and frames
		else if(wgt_st.is_focused)
			border_color = getStateColor(QPalette::Highlight, option);
	}

	QPainterPath shape;

	if(border_radius > 0)
		shape = createControlShape(option->rect, border_radius,
						CustomUiStyle::AllCorners, 0.5, 0.5, -0.5, -0.5);
	else
		shape = createControlShape(option->rect, 0,
						CustomUiStyle::NoCorners, 1, 1, -1, -1);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setPen(QPen(border_color, pen_width));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPEGroupBoxFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameGroupBox || !option || !painter || !widget)
		return;

	QColor bg_color = getAdjustedColor(getStateColor(QPalette::Dark, option), XMinFactor, MinFactor),
		   border_color = getAdjustedColor(getStateColor(QPalette::Mid, option), XMinFactor, -XMinFactor);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners));

	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(createControlShape(option->rect, FrameRadius, AllCorners,
																			 0.5, 0.5, -0.5, -0.5));

	painter->restore();
}

void CustomUiStyle::drawPELineEditPanel(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_PanelLineEdit || !option || !painter || !widget)
		return;

	QColor bg_color = getStateColor(QPalette::Base, option);
	WidgetState wgt_st(option, widget);

	// Apply hover effect (lighter background) when hovered but not focused
	if(wgt_st.is_enabled && wgt_st.is_hovered && !wgt_st.is_focused)
		bg_color = bg_color.lighter(MaxFactor);

	// Check if this LineEdit is part of a SpinBox
	bool is_spinbox_child =
					widget && qobject_cast<const QAbstractSpinBox *>(widget->parentWidget());

	// For spinbox children, only round left corners
	CornerFlag corner_flags = is_spinbox_child ? (CustomUiStyle::TopLeft | CustomUiStyle::BottomLeft) : CustomUiStyle::AllCorners;

	QPainterPath shape = createControlShape(option->rect, InputRadius, corner_flags);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(bg_color);

	painter->setPen(Qt::NoPen);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPEMenuPanel(PrimitiveElement element, const QStyleOption *option,
																		QPainter *painter, const QWidget *) const
{
	if(element != PE_PanelMenu || !option || !painter)
		return;

	QPalette pal = option->palette;
	QColor bg_color, border_color;

	// Different colors based on theme
	if(isDarkPalette(pal))
	{
		// Dark theme: Dark background, Mid border
		bg_color = getStateColor(QPalette::Dark, option);
		border_color = getStateColor(QPalette::Mid, option);
	}
	else
	{
		// Light theme: Light background, Mid border
		bg_color = getStateColor(QPalette::Window, option);
		border_color = getStateColor(QPalette::Mid, option);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->fillRect(option->rect, bg_color);

	// Draw border
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5));

	painter->restore();
}

void CustomUiStyle::drawCEMenuItem(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionMenuItem *menu_item_opt = qstyleoption_cast<const QStyleOptionMenuItem *>(option);

	if(!menu_item_opt || !painter)
		return;

	// Custom rendering for menu separators
	if(menu_item_opt->menuItemType == QStyleOptionMenuItem::Separator)
	{
		QColor sep_color = getStateColor(QPalette::Mid, option);

		painter->save();
		painter->setPen(QPen(sep_color, PenWidth));

		// Draw horizontal line with margins
		int margin = 5;
		QPoint p1(menu_item_opt->rect.left() + margin, menu_item_opt->rect.center().y()),
			     p2(menu_item_opt->rect.right() - margin, menu_item_opt->rect.center().y());

		painter->drawLine(p1, p2);
		painter->restore();
	}
	else
		QProxyStyle::drawControl(element, option, painter, widget);
}

void CustomUiStyle::drawPEToolTip(PrimitiveElement element, const QStyleOption *option,
																	QPainter *painter, const QWidget *) const
{
	if(element != PE_PanelTipLabel || !option || !painter)
		return;

	QColor bg_color = getStateColor(QPalette::ToolTipBase, option),
				 border_color = getAdjustedColor(bg_color, MidFactor, -MinFactor);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->fillRect(option->rect, bg_color);
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5));

	painter->restore();
}

void CustomUiStyle::drawCEProgressBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionProgressBar *pb_opt = qstyleoption_cast<const QStyleOptionProgressBar *>(option);

	if(!pb_opt || !painter || !widget)
		return;

	QPainterPath shape;
	QColor bg_color, border_color, fill_color;
	bool has_progress = (pb_opt->progress > pb_opt->minimum),
		 is_horizontal = qobject_cast<const QProgressBar *>(widget)->orientation() == Qt::Horizontal,
		 is_busy = (pb_opt->minimum == 0 && pb_opt->maximum == 0);

	// Handle different progress bar elements
	if(element == CE_ProgressBarGroove)
	{
		// Draw the background groove
		bg_color = getStateColor(QPalette::Base, pb_opt);
		border_color = getStateColor(QPalette::Mid, pb_opt);
		shape = createControlShape(pb_opt->rect, InputRadius, AllCorners,
						0.5, 0.5, -0.5, -0.5);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawPath(shape);

		// Draw border
		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(shape);
		painter->restore();

		return;
	}

	if(element == CE_ProgressBarContents && (has_progress || is_busy))
	{
		// Setup busy indicator animation timer if in busy mode
		if(is_busy)
		{
			// Property names as constants to avoid repetition
			static constexpr char BusyAnimTimerProp[] = "busy-anim-timer",
														BusyElapsedTimerProp[] = "busy-elapsed-timer";
			
			QProgressBar *pb = const_cast<QProgressBar *>(qobject_cast<const QProgressBar *>(widget));
			
			// Check if animation timer exists, create if not
			QTimer *anim_timer = pb->findChild<QTimer *>(BusyAnimTimerProp, Qt::FindDirectChildrenOnly);

			if(!anim_timer)
			{
				anim_timer = new QTimer(pb);
				anim_timer->setObjectName(BusyAnimTimerProp);
				anim_timer->setInterval(16); // ~60 FPS (16ms = 62.5 FPS)
				anim_timer->setTimerType(Qt::PreciseTimer); // Use precise timer for smoother animation

				QObject::connect(anim_timer, &QTimer::timeout, pb, [pb]() {
					pb->update();
				});

				// Initialize animation elapsed timer (more precise than QDateTime)
				QElapsedTimer *elapsed_timer = new QElapsedTimer();
				elapsed_timer->start();
				pb->setProperty(BusyElapsedTimerProp, QVariant::fromValue(static_cast<void*>(elapsed_timer)));
				anim_timer->start();
			}

			// Calculate busy indicator position using precise elapsed timer
			QElapsedTimer *elapsed_timer = static_cast<QElapsedTimer*>(pb->property(BusyElapsedTimerProp).value<void*>());
			qint64 elapsed = elapsed_timer ? elapsed_timer->elapsed() : 0;

			// Animation parameters
			static constexpr qreal AnimDuration = 10000.0; // Duration for one full cycle in ms (slower movement)
			static constexpr qreal BusyIndicatorSize = 0.05; // 5% of total width/height

			// Calculate position (0.0 to beyond 1.0 to allow full exit/entry)
			// Position goes from -BusyIndicatorSize (fully outside left) to 1.0 (fully outside right)
			qreal time_normalized = static_cast<qreal>(elapsed % static_cast<qint64>(AnimDuration)) / AnimDuration;
			
			// Simple linear movement from left (-size) to right (1.0), then reset
			// This allows the indicator to fully exit on the right and re-enter from the left
			qreal position = -BusyIndicatorSize + (time_normalized * (1.0 + BusyIndicatorSize));

			// Calculate indicator rectangle
			QRect content_rect = option->rect;
			QRectF indicator_rect; // Use QRectF for sub-pixel precision
			static constexpr qreal MinIndicatorSize = 20.0; // Minimum size in pixels

			if(is_horizontal)
			{
				qreal indicator_width = content_rect.width() * BusyIndicatorSize;
				// Ensure minimum size of 20px
				indicator_width = std::max(indicator_width, MinIndicatorSize);
				
				// Position can be negative (off-screen left) to > 1.0 (off-screen right)
				qreal x_pos = content_rect.x() + (content_rect.width() * position);
				indicator_rect = QRectF(x_pos, content_rect.y(), indicator_width, content_rect.height());
			}
			else
			{
				qreal indicator_height = content_rect.height() * BusyIndicatorSize;
				// Ensure minimum size of 20px
				indicator_height = std::max(indicator_height, MinIndicatorSize);
				
				// For vertical: move from bottom to top (reverse direction)
				qreal y_pos = content_rect.y() + content_rect.height() - (content_rect.height() * (position + BusyIndicatorSize));
				indicator_rect = QRectF(content_rect.x(), y_pos, content_rect.width(), indicator_height);
			}

			// Draw the busy indicator
			fill_color = getStateColor(QPalette::Highlight, option);
			border_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);
			
			// Use QRectF for sub-pixel rendering (smoother animation)
			QPainterPath shape_f;
			qreal radius = InputRadius;
			shape_f.addRoundedRect(indicator_rect.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);

			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
			
			// Clip to content rect to avoid drawing artifacts outside the progress bar
			painter->setClipRect(content_rect);
			
			painter->setBrush(fill_color);
			painter->setPen(Qt::NoPen);
			painter->drawPath(shape_f);

			// Draw border
			painter->setPen(QPen(border_color, PenWidth));
			painter->setBrush(Qt::NoBrush);
			painter->drawPath(shape_f);
			painter->restore();
			

			return;
		}

		// Calculate progress percentage and content rectangle
		int range = pb_opt->maximum - pb_opt->minimum;
		qreal prog_ratio = range > 0 ? static_cast<qreal>(pb_opt->progress - pb_opt->minimum) / range : 0.0;
		QRect content_rect = option->rect;

		if(is_horizontal)
			// For horizontal progress bars, adjust width
			content_rect.setWidth(int(content_rect.width() * prog_ratio));
		else
		{
			// For vertical progress bars, adjust height from bottom
			int new_height = static_cast<int>(content_rect.height() * prog_ratio);

			content_rect.setY(content_rect.bottom() - new_height);
			content_rect.setHeight(new_height);
		}

		fill_color = getStateColor(QPalette::Highlight, option);
		border_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);
		shape = createControlShape(content_rect, InputRadius, AllCorners,
						0.5, 0.5, -0.5, -0.5);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setBrush(fill_color);
		painter->setPen(Qt::NoPen);
		painter->drawPath(shape);

		// Draw border
		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(shape);
		painter->restore();

		return;
	}

	if(element == CE_ProgressBarLabel)
	{
		// Let Qt handle the label
		QProxyStyle::drawControl(element, option, painter, widget);
		return;
	}

	// For CE_ProgressBar, draw both groove and contents
	if(element == CE_ProgressBar)
	{
		if(has_progress)
		{
			// Use Highlight colors when there is progress
			bg_color = getStateColor(QPalette::Highlight, option);
			border_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);
		}
		else
		{
			// Use input styling when no progress (same as line edit)
			bg_color = getStateColor(QPalette::Base, option);
			border_color = getStateColor(QPalette::Mid, option);
		}

		shape = createControlShape(option->rect, InputRadius, AllCorners);

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->setBrush(bg_color);

		painter->setPen(Qt::NoPen);
		painter->drawPath(shape);

		// Draw border
		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawPath(shape);

		painter->restore();

		return;
	}
}

void CustomUiStyle::drawCEHeaderSection(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionHeader *header_opt = qstyleoption_cast<const QStyleOptionHeader *>(option);

	if(element != CE_HeaderSection || !header_opt || !painter || !widget)
		return;

	// Determine background and border colors based on state
	QColor bg_color, border_color;
	WidgetState wgt_st(header_opt, widget);

	if(wgt_st.is_pressed)
	{
		// Pressed state (for sorting interaction)
		bg_color = getStateColor(QPalette::Button, header_opt).darker(MinFactor);
		border_color = getStateColor(QPalette::Midlight, header_opt).lighter(MinFactor);
	}
	else
	{
		// Normal state
		bg_color = getStateColor(QPalette::Button, header_opt);
		border_color = getStateColor(QPalette::Midlight, header_opt);
	}

	// Try to determine column information from the header widget
	const QHeaderView *header_view = qobject_cast<const QHeaderView *>(widget);

	painter->save();

	// Fill the background
	painter->fillRect(header_opt->rect, bg_color);

	// Draw bottom and right borders
	painter->setPen(QPen(border_color, 1));
	painter->drawLine(header_opt->rect.bottomLeft(), header_opt->rect.bottomRight());
	painter->drawLine(header_opt->rect.topRight(), header_opt->rect.bottomRight());

	painter->restore();
}

void CustomUiStyle::drawCESplitter(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QSplitter *splitter = qobject_cast<const QSplitter *>(widget);

	if(element != CE_Splitter || !option || !painter || !splitter)
		return;

	WidgetState wgt_st(option, splitter);
	Qt::Orientation orientation = splitter->orientation();
	QRect sp_rect = option->rect,
		  hnd_rect;
	QColor bg_color = getStateColor(QPalette::Highlight, option),
		   border_color = getStateColor(QPalette::Highlight, option).lighter(MinFactor);

	// Make the splitter handle slightly thinner for better aesthetics
	if(orientation == Qt::Horizontal)
	{
		sp_rect.setWidth(sp_rect.width() - 2);
		sp_rect.translate(1, 0);
	}
	else
	{
		sp_rect.setHeight(sp_rect.height() - 2);
		sp_rect.translate(0, 1);
	}

	hnd_rect = sp_rect;

	/* Configuring the handle rectangle based when in normal state
	 * In that case the handle should be centered within the splitter */
	if(!wgt_st.is_hovered)
	{
		int half_sz = SplitterSize / 2;

		if(orientation == Qt::Horizontal)
		{
			hnd_rect.setHeight(SplitterSize);
			hnd_rect.moveTo(sp_rect.left(), sp_rect.center().y() - half_sz);
		}
		else
		{
			hnd_rect.setWidth(SplitterSize);
			hnd_rect.moveTo(sp_rect.center().x() - half_sz, sp_rect.top());
		}
	}
	else if(wgt_st.is_hovered)
	{
		bg_color = bg_color.lighter(MinFactor);
		border_color = border_color.lighter(MinFactor);
	}
	else if(wgt_st.is_pressed)
	{
		bg_color = bg_color.darker(MinFactor);
		border_color = border_color.darker(MinFactor);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);

	painter->setBrush(bg_color);
	painter->setPen(QPen(border_color, 1));
	painter->drawRect(hnd_rect);

	painter->restore();
}

void CustomUiStyle::drawPETabWidgetFrame(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(element != PE_FrameTabWidget || !option || !painter || !widget)
		return;

	QColor bg_color = getAdjustedColor(getStateColor(QPalette::Dark, option), MinFactor, MinFactor + 5),
		   border_color = getAdjustedColor(getStateColor(QPalette::Mid, option), MinFactor, -XMinFactor);

	int radius = TabWgtRadius * 2; // Larger radius for smoothness

	const QTabWidget *tab_widget =
					qobject_cast<const QTabWidget *>(widget);

	QTabWidget::TabPosition tab_position = tab_widget->tabPosition();

	/* Determine which corners to round based on tab position
	 * Rule: Keep only ONE corner straight (where tabs connect), round all others */
	CornerFlag corners_to_round = AllCorners;

	switch(tab_position)
	{
	case QTabWidget::North:
		// Keep top-left corner straight, round others
		corners_to_round = TopRight | BottomLeft | BottomRight;
		break;

	case QTabWidget::South:
		// Keep bottom-left corner straight, round others
		corners_to_round = TopLeft | TopRight | BottomRight;
		break;

	case QTabWidget::West:
		// Keep top-left corner straight, round others
		corners_to_round = TopRight | BottomLeft | BottomRight;
		break;

	case QTabWidget::East:
		// Keep top-right corner straight, round others
		corners_to_round = TopLeft | BottomLeft | BottomRight;
		break;
	}

	QPainterPath path = createControlShape(option->rect, radius, corners_to_round,
					0.5, 0.5, -0.5, -0.5, NotOpen);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(path);

	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(path);

	painter->restore();
}

void CustomUiStyle::drawCCScrollBar(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionSlider *sbar_opt =
					qstyleoption_cast<const QStyleOptionSlider *>(option);

	if(!sbar_opt || !painter || !widget)
		return;

	WidgetState wgt_st(option, widget);

	// Handle and buttons use same color as QToolButton in normal state
	QColor bg_color = getStateColor(QPalette::Button, sbar_opt),
		   	 border_color = getAdjustedColor(getStateColor(QPalette::Mid, sbar_opt), MinFactor, NoFactor);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw the scroll bar background/groove
	QRectF sub_ctrl_rect = subControlRect(CC_ScrollBar, sbar_opt, SC_ScrollBarGroove, widget);

	if(!sub_ctrl_rect.isEmpty())
	{
		// Groove darker than handle: more contrast in dark theme, subtle in light theme
		QColor groove_bg = getAdjustedColor(bg_color, -MidFactor, -XMinFactor - 5),
					 groove_border = getAdjustedColor(border_color, -XMinFactor, -XMinFactor - 5);

		painter->setBrush(groove_bg);
		painter->setPen(Qt::NoPen);
		painter->drawRect(sub_ctrl_rect);

		// Draw groove border (straight, no rounded corners)
		painter->setPen(QPen(groove_border, PenWidth));
		painter->setBrush(Qt::NoBrush);

		sub_ctrl_rect.adjust(0.5, 0.5, -0.5, -0.5);
		painter->drawRect(sub_ctrl_rect);
	}

	// Draw the scroll bar handle/slider
	sub_ctrl_rect = subControlRect(CC_ScrollBar, sbar_opt, SC_ScrollBarSlider, widget);

	if(!sub_ctrl_rect.isEmpty())
	{
		// Apply special highlighting for pressed/hovered handle
		QColor slider_bg = bg_color,
			   slider_border = border_color;

		if(sbar_opt->activeSubControls & SC_ScrollBarSlider)
		{
			if(wgt_st.is_pressed)
			{
				// Pressed: darker in both themes
				slider_bg = getAdjustedColor(bg_color, -XMinFactor, -XMinFactor);
				slider_border = getAdjustedColor(border_color, -XMinFactor, -XMinFactor);
			}
			else if(wgt_st.is_hovered)
			{
				// Hover: lighter in both themes (subtle effect)
				slider_bg = getAdjustedColor(bg_color, MidFactor, XMinFactor);
				slider_border = getAdjustedColor(border_color, MidFactor, XMinFactor);
			}
		}

		painter->setBrush(slider_bg);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);

		// Draw handle border
		painter->setPen(QPen(slider_border, PenWidth));
		painter->setBrush(Qt::NoBrush);

		sub_ctrl_rect.adjust(0.5, 0.5, -0.5, -0.5);
		painter->drawRoundedRect(sub_ctrl_rect, ScrollBarRadius, ScrollBarRadius);
	}

	// Draw scroll bar buttons using the unified method
	drawScrollBarButton(sbar_opt, painter, widget, SC_ScrollBarAddLine, bg_color, border_color);
	drawScrollBarButton(sbar_opt, painter, widget, SC_ScrollBarSubLine, bg_color, border_color);

	painter->restore();
}

void CustomUiStyle::drawCEScrollBar(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Handle scroll bar control elements
	if(element == CE_ScrollBarAddLine || element == CE_ScrollBarSubLine)
	{
		// Cast to complex option to access activeSubControls
		const QStyleOptionComplex *complex_option = qstyleoption_cast<const QStyleOptionComplex *>(option);

		// Create a copy of the option to modify state for proper hover/pressed detection
		QStyleOption btn_opt = *option;

		// Determine the correct SubControl for this element
		QStyle::SubControl sub_control = (element == CE_ScrollBarAddLine) ? SC_ScrollBarAddLine : SC_ScrollBarSubLine;

		// Check if this specific button is active (hovered or pressed)
		bool is_active = complex_option && (complex_option->activeSubControls & sub_control);

		if(is_active)
		{
			// Apply the correct state based on the main option state
			if(option->state & State_Sunken)
				btn_opt.state |= State_Sunken;
			else if(option->state & State_MouseOver)
				btn_opt.state |= State_MouseOver;
		}
		else
		{
			// Remove hover/pressed states if this button is not active
			btn_opt.state &= ~(State_MouseOver | State_Sunken);
		}

		// Get colors based on the modified button state
		WidgetState wgt_st(&btn_opt, widget);
		QColor bg_color = getStateColor(QPalette::Button, &btn_opt);
		QColor border_color = getStateColor(QPalette::Midlight, &btn_opt);

		if(wgt_st.is_enabled)
		{
			if(wgt_st.is_pressed)
			{
				bg_color = getStateColor(QPalette::Dark, &btn_opt);
				border_color = getStateColor(QPalette::Mid, &btn_opt);
			}
			else if(wgt_st.is_hovered)
			{
				bg_color = bg_color.lighter(MaxFactor);
				border_color = border_color.lighter(MaxFactor);
			}
		}

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		// Draw button background
		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, ScrollBarRadius, ScrollBarRadius);

		// Draw button border
		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5),
						ScrollBarRadius, ScrollBarRadius);

		// Use drawControlArrow to draw the arrow with proper state
		// Convert SubControl to ArrowDirection
		ArrowType arrow_dir;
		if(sub_control == SC_ScrollBarAddLine)
		{
			// AddLine: bottom/right button
			const QScrollBar *scrollbar = qobject_cast<const QScrollBar *>(widget);
			bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
			arrow_dir = is_horizontal ? RightArrow : DownArrow;
		}
		else // SC_ScrollBarSubLine
		{
			// SubLine: top/left button
			const QScrollBar *scrollbar = qobject_cast<const QScrollBar *>(widget);
			bool is_horizontal = scrollbar && scrollbar->orientation() == Qt::Horizontal;
			arrow_dir = is_horizontal ? LeftArrow : UpArrow;
		}

		drawControlArrow(&btn_opt, painter, widget, arrow_dir);

		painter->restore();
	}
	else if(element == CE_ScrollBarSlider)
	{
		// This is handled by drawCCScrollBar, but we can provide fallback
		WidgetState wgt_st(option, widget);
		QColor bg_color = getStateColor(QPalette::Button, option);
		QColor border_color = getStateColor(QPalette::Midlight, option);

		if(wgt_st.is_enabled)
		{
			if(wgt_st.is_pressed)
			{
				bg_color = getStateColor(QPalette::Dark, option);
				border_color = getStateColor(QPalette::Mid, option);
			}
			else if(wgt_st.is_hovered)
			{
				bg_color = bg_color.lighter(MaxFactor);
				border_color = border_color.lighter(MaxFactor);
			}
		}

		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);

		painter->setBrush(bg_color);
		painter->setPen(Qt::NoPen);
		painter->drawRoundedRect(option->rect, ScrollBarRadius, ScrollBarRadius);

		painter->setPen(QPen(border_color, PenWidth));
		painter->setBrush(Qt::NoBrush);
		painter->drawRoundedRect(QRectF(option->rect).adjusted(0.5, 0.5, -0.5, -0.5),
						ScrollBarRadius, ScrollBarRadius);

		painter->restore();
	}
	else
	{
		// For other scroll bar elements, use default rendering
		QProxyStyle::drawControl(element, option, painter, widget);
	}
}

void CustomUiStyle::drawSpinBoxButton(const QStyleOptionSpinBox *option, QPainter *painter, const QWidget *widget, QStyle::SubControl btn_sc_id) const
{
	if(!option || !painter || !widget || (btn_sc_id != SC_SpinBoxUp && btn_sc_id != SC_SpinBoxDown))
		return;

	/* Create a copy of the option to modify some
	 * properties for button drawing */
	QStyleOptionSpinBox btn_opt = *option;
	WidgetState wgt_st(option, widget);
	QRect rect;

	// Get button rectangle according to sub-control id
	if(btn_sc_id == SC_SpinBoxUp)
		rect = subControlRect(CC_SpinBox, option, SC_SpinBoxUp, widget);
	else
		rect = subControlRect(CC_SpinBox, option, SC_SpinBoxDown, widget);

	if(option->activeSubControls & btn_sc_id)
	{
		if(wgt_st.is_pressed)
			btn_opt.state |= State_Sunken;
		else if(wgt_st.is_hovered)
			btn_opt.state |= State_MouseOver;
	}
	else
		// Remove mouse over and sunken states if this button is not active
		btn_opt.state &= ~(State_MouseOver | State_Sunken);

	// Use the same color logic as button panels
	QColor bg_color = getStateColor(QPalette::Button, option),
		   border_color = getStateColor(QPalette::Midlight, option);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.is_focused)
			border_color = getStateColor(QPalette::Highlight, option);
		else if(wgt_st.is_pressed)
		{
			bg_color = getStateColor(QPalette::Dark, option);
			border_color = getStateColor(QPalette::Mid, option);
		}
		else if(wgt_st.is_hovered)
		{
			bg_color = bg_color.lighter(MaxFactor);
			border_color = border_color.lighter(MaxFactor);
		}
	}

	QPainterPath btn_path;
	int radius = ButtonRadius - 2;

	if(btn_sc_id == SC_SpinBoxUp)
	{
		// Up button: only top-right corner rounded, extend slightly upward
		btn_path = createControlShape(rect, radius, CustomUiStyle::TopRight, 0, -1.5, 0, 1.5);
	}
	else
	{
		// Down button: only bottom-right corner rounded, extend slightly downward
		btn_path = createControlShape(rect, radius, CustomUiStyle::BottomRight, 0, 0, 0, 1.5);
	}

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	// Draw background
	painter->setBrush(bg_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(btn_path);

	// Draw border normally (all edges)
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(btn_path);

	/* Draw arrow symbol.
	 * For the up button, move the arrow slightly upwards to make
	 * it symmetrically aligned with the down button */
	if(btn_sc_id == SC_SpinBoxUp)
		rect.translate(0, -1);

	btn_opt.rect = rect;
	ArrowType arrow_dir = (btn_sc_id == SC_SpinBoxUp) ? UpArrow : DownArrow;
	drawControlArrow(&btn_opt, painter, widget, arrow_dir);
	painter->restore();
}

void CustomUiStyle::drawSpinBoxEditField(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	if(!option || !painter || !widget)
		return;

	// Get background color - no border needed for edit field
	QPalette pal = qApp->palette();
	QColor bg_color = getStateColor(pal, QPalette::Base, option),
		   border_color = getStateColor(pal, QPalette::Midlight, option);
	WidgetState wgt_st(option, widget);

	if(wgt_st.is_enabled)
	{
		if(wgt_st.is_focused)
			border_color = getStateColor(pal, QPalette::Highlight, option);
		else if(wgt_st.is_hovered)
		{
			bg_color = bg_color.lighter(MaxFactor);
			border_color = border_color.lighter(MaxFactor);
		}
	}

	// Create custom path for edit field with specific rounded corners
	QPainterPath edit_path = createControlShape(option->rect, InputRadius - 1,
					CustomUiStyle::TopLeft | CustomUiStyle::BottomLeft,
					1.5, 1.5, -1.5, -1.5);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);

	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(bg_color);
	painter->drawPath(edit_path);

	painter->restore();
}

QPixmap CustomUiStyle::generatedIconPixmap(QIcon::Mode icon_mode, const QPixmap &pixmap, const QStyleOption *option) const
{
	return icon_mode == QIcon::Disabled ? createGrayMaskedPixmap(pixmap) : QProxyStyle::generatedIconPixmap(icon_mode, pixmap, option);
}

QColor CustomUiStyle::getStateColor(const QPalette &pal, QPalette::ColorRole role, const QStyleOption *option)
{
	if(!option)
		return pal.color(role);

	// Determine color group based on widget state
	QPalette::ColorGroup group = QPalette::Active;
	WidgetState wgt_st(option, nullptr);

	if(!wgt_st.is_enabled)
		group = QPalette::Disabled;
	else if(!wgt_st.is_active)
		group = QPalette::Inactive;

	return pal.color(group, role);
}

QColor CustomUiStyle::getStateColor(QPalette::ColorRole role, const QStyleOption *option)
{
	return getStateColor(qApp->palette(), role, option);
}

QColor CustomUiStyle::getAdjustedColor(const QColor &color, int dark_ui_factor, int light_ui_factor)
{
	if(!isDarkPalette())
	{
		if(light_ui_factor > 0)
			return color.lighter(light_ui_factor);

		if(light_ui_factor < 0)
			return color.darker(-light_ui_factor);
	}
	else
	{
		if(dark_ui_factor > 0)
			return color.lighter(dark_ui_factor);

		if(dark_ui_factor < 0)
			return color.darker(-dark_ui_factor);
	}

	return color;
}

bool CustomUiStyle::isDarkPalette(const QPalette &pal)
{
	/* If text is lighter than background, it's a dark theme
	 * otherwise, it's a light theme */
	return pal.color(QPalette::WindowText).lightness() > pal.color(QPalette::Window).lightness();
}

bool CustomUiStyle::isDarkPalette()
{
	return isDarkPalette(qApp->palette());
}

void CustomUiStyle::drawPEProgressChunk(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	// This handles the progress fill part
	const QStyleOptionProgressBar *pb_opt =
					qstyleoption_cast<const QStyleOptionProgressBar *>(option);

	if(!pb_opt || !painter || !widget || pb_opt->progress <= pb_opt->minimum)
		return;

	// Calculate progress percentage and content rectangle
	int range = pb_opt->maximum - pb_opt->minimum;
	qreal prog_ratio = range > 0 ? qreal(pb_opt->progress - pb_opt->minimum) / range : 0.0;
	QRect content_rect = pb_opt->rect;

	// Get orientation from the widget
	const QProgressBar *prog_bar = qobject_cast<const QProgressBar *>(widget);
	bool is_horizontal = !prog_bar || prog_bar->orientation() == Qt::Horizontal;

	if(is_horizontal)
	{
		// For horizontal progress bars, adjust width
		content_rect.setWidth(int(content_rect.width() * prog_ratio));
	}
	else
	{
		// For vertical progress bars, adjust height from bottom
		int new_height = int(content_rect.height() * prog_ratio);
		content_rect.setY(content_rect.bottom() - new_height);
		content_rect.setHeight(new_height);
	}

	QColor fill_color = getStateColor(QPalette::Highlight, option),
		   border_color = getStateColor(QPalette::Highlight, option).lighter(MidFactor);

	QPainterPath shape = createControlShape(content_rect, InputRadius, AllCorners,
					0.5, 0.5, -0.5, -0.5);

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, true);
	painter->setBrush(fill_color);
	painter->setPen(Qt::NoPen);
	painter->drawPath(shape);

	// Draw border
	painter->setPen(QPen(border_color, PenWidth));
	painter->setBrush(Qt::NoBrush);
	painter->drawPath(shape);
	painter->restore();
}

void CustomUiStyle::drawPEHeaderArrow(const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
	const QStyleOptionHeader *header_opt =
					qstyleoption_cast<const QStyleOptionHeader *>(option);

	if(!header_opt || !painter || !widget || header_opt->sortIndicator == QStyleOptionHeader::None)
		return;

	// Calculate arrow position - right side of header with some margin
	int arrow_margin = 5;
	QRect arrow_rect(header_opt->rect.right() - arrow_margin - ArrowWidth,
					header_opt->rect.center().y() - (ArrowHeight / 2),
					ArrowWidth, ArrowHeight);

	// Create option for arrow drawing
	QStyleOption arrow_opt = *header_opt;
	ArrowType arrow_type;

	arrow_opt.rect = arrow_rect;

	if(header_opt->sortIndicator == QStyleOptionHeader::SortUp)
		arrow_type = UpArrow;
	else
		arrow_type = DownArrow;

	// Draw the sort arrow using our standard method
	drawControlArrow(&arrow_opt, painter, widget, arrow_type);
}

void CustomUiStyle::setStyleHint(StyleHint hint, const QList<QFrame *> &frames)
{
	for(auto &frm : frames)
		setStyleHint(hint, frm);
}

void CustomUiStyle::setStyleHint(StyleHint hint, QFrame *frame)
{
	if(!frame || hint == NoHint)
		return;

	static const std::map<StyleHint, QColor> frm_colors = {
		{ ErrorFrmHint, "#eb4848" },
		{ InfoFrmHint, "#52d0eb" },
		{ AlertFrmHint, "#ebdc4a" },
		{ ConfirmFrmHint, "#52d0eb" },
		{ SuccessFrmHint, "#4aeb5c" }
	};

	frame->setProperty(StyleHintProp, static_cast<int>(hint));

	QColor hint_color;
	bool is_def_hint = (hint == DefaultFrmHint ||
											hint == AltDefaultFrmHint);

	if(!is_def_hint)
		hint_color = frm_colors.at(hint);

	frame->setProperty(StyleHintColor, hint_color);

	// Extract the frame shape using Shape_Mask to ignore shadow
	QFrame::Shape shape = static_cast<QFrame::Shape>(frame->frameShape() & QFrame::Shape_Mask);

	// For HLine/VLine frames, apply border color via stylesheet
	if(shape == QFrame::HLine || shape == QFrame::VLine)
	{
		QString color_role = is_def_hint ? "light" : "midlight";
		frame->setStyleSheet(QString("QFrame { border: %1px solid palette(%2); }")
												 .arg(PenWidth)
												 .arg(color_role));
	}
	// For other frames we force the shape to StyledPanel
	else
		frame->setFrameShape(QFrame::StyledPanel);
}
