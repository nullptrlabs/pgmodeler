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
#include "widgets/customtablewidget.h"
#include "utils/syntaxhighlighter.h"

class __libgui AppearanceConfigWidget: public BaseConfigWidget, public Ui::AppearanceConfigWidget  {
	Q_OBJECT

	private:
		//! \brief Holds the currently loaded config params
		static std::map<QString, attribs_map> config_params;

		//! \brief Holds the QPalette instances for each available theme
		static std::map<QString, QPalette> theme_palettes;

		//! \brief Holds the table widget item colors for each available theme
		static std::map<QString, std::map<CustomTableWidget::TableItemColor, QColor>> theme_tab_item_colors;

		//! \brief Colors used for CustomTableWidget items when in system default dark theme
		static QStringList dark_tab_item_colors,

		//! \brief Colors used for CustomTableWidget items when in system default light theme
		light_tab_item_colors;

		//! \brief Holds the current user interface theme id (light/dark)
		static QString UiThemeId;

		static QPalette system_pal;

		//! \brief Auxiliary class that stores the formating data of each element
		class AppearanceConfigItem {
			public:
				QString conf_id;
				QTextCharFormat font_fmt;
				QColor colors[3];
				bool obj_conf;
		};
		
		QButtonGroup *ico_sz_btn_grp;

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

		void loadThemesConfiguration();

		//! \brief Loads the example model from file (conf/exampledb.dbm)
		void loadExampleModel();
		
		//! \brief Updates the color configuration for the placeholder item
		void updatePlaceholderItem();
		
		//! \brief Applies the color/font settings loaded from file to BaseObjectView instances
		void applyObjectsStyle();

		//! \brief Applies the design and code settings loaded from file
		void applyDesignCodeStyle();

		void applyDesignCodeTheme();

		void applyUiStyleSheet();

		//! \brief Returns the theme id depending on the selection in theme_cmb
		QString getThemeId();

	protected:
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		AppearanceConfigWidget(QWidget * parent = nullptr);

		~AppearanceConfigWidget() override;

		void showEvent(QShowEvent *) override;

		void hideEvent(QHideEvent *) override;

		void saveConfiguration() override;

		void loadConfiguration() override;

		//! \brief Applies the selected ui theme to the whole application
		void applyUiTheme();

		static std::map<QString, attribs_map> getConfigurationParams();

	private slots:
		void enableConfigElement();
		void applyElementFontStyle();
		void applyElementColor(unsigned color_idx, QColor color);
		void previewCodeFontStyle();
		void previewCanvasColors();
		void applySyntaxHighlightTheme();

		void applyConfiguration() override;

		//! \brief Applies temporarily all the settings related to the UI
		void previewUiSettings();

	public slots:
		void restoreDefaults() override;
};

#endif
