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
\ingroup libgui
\class AppearanceConfigWidget
\brief Implements the operations to manage graphical objects appearance configuration.
*/

#ifndef APPEARANCE_CONFIG_WIDGET_H
#define APPEARANCE_CONFIG_WIDGET_H

#include "ui_appearanceconfigwidget.h"
#include "baseconfigwidget.h"
#include "widgets/colorpickerwidget.h"
#include "objectsscene.h"
#include "databasemodel.h"
#include "widgets/numberedtexteditor.h"
#include "utils/syntaxhighlighter.h"
#include <algorithm>

class __libgui AppearanceConfigWidget: public BaseConfigWidget, public Ui::AppearanceConfigWidget  {
	Q_OBJECT

	private:
		/*! \brief This palette instance holds the current system palette. It is used
		 * to detect if the system uses dark or light theme and helps pgModeler to
		 * determine the proper colors when user used System default theme setting */
		static QPalette system_pal;
		
		//! \brief Holds the currently loaded config params
		static std::map<QString, attribs_map> config_params;

		/*! \brief Holds the QPalette settings that defines dark theme.
		 * This map key is a color role which value is a string list that
		 * contains 3 elements: active color, inactive color and disabled color. */
		static std::map<QPalette::ColorRole, QStringList> dark_ui_colors;

		/*! \brief Holds the QPalette settings that defines light theme.
		 * This map key is a color role which value is a string list that
		 * contains 3 elements: active color, inactive color and disabled color. */
		static std::map<QPalette::ColorRole, QStringList> light_ui_colors;

		//! \brief Holds the default/system QPalette settings.
		static std::map<QPalette::ColorRole, QStringList> system_ui_colors;

		//! \brief Colors used for ObjectTableWidget items when in dark theme
		static QStringList dark_tab_item_colors;

		//! \brief Colors used for ObjectTableWidget items when in light theme
		static QStringList light_tab_item_colors;

		//! \brief Holds the current user interface theme id (light/dark)
		static QString UiThemeId;

		//! \brief Auxiliary class that stores the formating data of each element
		class AppearanceConfigItem {
			public:
				QString conf_id;
				QTextCharFormat font_fmt;
				QColor colors[3];
				bool obj_conf;
		};
		
		NumberedTextEditor *font_preview_txt;

		SyntaxHighlighter *font_preview_hl;

		RoundedRectItem *placeholder;
					
		ColorPickerWidget *elem_color_cp,

		*line_numbers_cp,

		*line_numbers_bg_cp,

		*line_highlight_cp,

		*grid_color_cp,

		*canvas_color_cp,

		*delimiters_color_cp;
		
		//! \brief Color picker dialog
		QColorDialog color_dlg;
		
		//! \brief Viewport used to show the example model
		QGraphicsView *viewp;
		
		//! \brief Object scene used to store the graphical objects
		ObjectsScene *scene;
		
		//! \brief Database model used to store the example base objects
		DatabaseModel *model;

		//! \brief Stores the element configuration items
		std::vector<AppearanceConfigItem> conf_items;
		
		bool show_grid, show_delimiters;

		//! \brief Loads the example model from file (conf/exampledb.dbm)
		void loadExampleModel();
		
		//! \brief Updates the color configuration for the placeholder item
		void updatePlaceholderItem();
		
		//! \brief Applies the color/font settings loaded from file to BaseObjectView instances
		void applyObjectsStyle();

		//! \brief Applies the design and code settings loaded from file
		void applyDesignCodeStyle();

		//! \brief Stores in system_ui_colors the default colors of ui elements
		void storeSystemUiColors();

		void applyDesignCodeTheme();

		void applyUiStyleSheet();

		//! \brief Returns the theme id (light/dark) depending on the selection in ui_theme_cmb
		QString __getUiThemeId();

	public:
		AppearanceConfigWidget(QWidget * parent = nullptr);

		virtual ~AppearanceConfigWidget();
		
		virtual void showEvent(QShowEvent *) override;

		virtual void hideEvent(QHideEvent *) override;

		virtual void saveConfiguration() override;

		virtual void loadConfiguration() override;

		//! \brief Applies the selected ui theme to the whole application
		void applyUiTheme();

		//! \brief Returns whether the currently used theme is a dark one
		static bool isDarkUiTheme();

		/*! \brief Returns the currently UI palette lightness.
		 * Return values can Attributes::Light or Attributes::Dark */
		static QString getUiLightness(const QPalette &pal);

		static std::map<QString, attribs_map> getConfigurationParams();

	private slots:
		void enableConfigElement();
		void applyElementFontStyle();
		void applyElementColor(unsigned color_idx, QColor color);
		void previewCodeFontStyle();
		void previewCanvasColors();
		void applySyntaxHighlightTheme();

		virtual void applyConfiguration() override;

		//! \brief Applies temporarily all the settings related to the UI
		void previewUiSettings();

	public slots:
		virtual void restoreDefaults() override;
};

#endif
