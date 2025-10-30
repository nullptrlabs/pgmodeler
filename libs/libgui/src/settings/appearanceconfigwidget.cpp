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

#include "appearanceconfigwidget.h"
#include "attributes.h"
#include "customuistyle.h"
#include "globalattributes.h"
#include "graphicalview.h"
#include "guiutilsns.h"
#include "relationshipview.h"
#include "styledtextboxview.h"
#include "tableview.h"
#include "widgets/customtablewidget.h"
#include "widgets/modelwidget.h"
#include <QButtonGroup>
#include <QToolTip>
#include <qcursor.h>

std::map<QString, QPalette> AppearanceConfigWidget::theme_palettes;
std::map<QString, attribs_map> AppearanceConfigWidget::config_params;
std::map<QString, std::map<CustomTableWidget::TableItemColor, QColor>> AppearanceConfigWidget::theme_tab_item_colors;
QString AppearanceConfigWidget::UiThemeId;
QPalette AppearanceConfigWidget::system_pal;

QStringList AppearanceConfigWidget::dark_tab_item_colors{
				"#b54225", "#fff", "#54a800", "#fff",
				"#54a800", "#fff", "#e2e236", "#000",
				"#b54225", "#fff", "#fa0000", "#00f000"};

QStringList AppearanceConfigWidget::light_tab_item_colors{
				"#ffb4b4", "#303030", "#a4f9b0", "#303030",
				"#c0ffc0", "#000", "#ffffc0", "#000",
				"#ffc0c0", "#000", "#fa0000", "#00f000"};

AppearanceConfigWidget::AppearanceConfigWidget(QWidget *parent) : BaseConfigWidget(parent)
{
	setupUi(this);
	show_grid = show_delimiters = false;

	/* Initialize system palette only when the lightness of window 
	 * and button roles are zero (black) which indicates that the 
	 * palette was not initialized yet */
	if(system_pal.color(QPalette::Window).lightness() == 0 && 
		 system_pal.color(QPalette::Button).lightness() == 0)
		system_pal= qApp->palette();

	QStringList conf_ids = {
			/* 00 */ Attributes::Global,
			/* 01 */ Attributes::Constraints,
			/* 02 */ Attributes::ObjSelection,
			/* 03 */ Attributes::ObjShadow,
			/* 04 */ Attributes::PositionInfo,
			/* 05 */ Attributes::PositionInfo,
			/* 06 */ Attributes::ObjectType,
			/* 07 */ Attributes::LockerArc,
			/* 08 */ Attributes::LockerBody,
			/* 09 */ Attributes::TableSchemaName,
			/* 10 */ Attributes::TableName,
			/* 11 */ Attributes::TableBody,
			/* 12 */ Attributes::TableExtBody,
			/* 13 */ Attributes::TableTitle,
			/* 14 */ Attributes::TableTogglerButtons,
			/* 15 */ Attributes::TableTogglerBody,
			/* 16 */ BaseObject::getSchemaName(ObjectType::Rule),
			/* 17 */ BaseObject::getSchemaName(ObjectType::Rule),
			/* 18 */ BaseObject::getSchemaName(ObjectType::Index),
			/* 19 */ BaseObject::getSchemaName(ObjectType::Index),
			/* 20 */ BaseObject::getSchemaName(ObjectType::Trigger),
			/* 21 */ BaseObject::getSchemaName(ObjectType::Trigger),
			/* 22 */ BaseObject::getSchemaName(ObjectType::Constraint),
			/* 23 */ BaseObject::getSchemaName(ObjectType::Constraint),
			/* 24 */ BaseObject::getSchemaName(ObjectType::Policy),
			/* 25 */ BaseObject::getSchemaName(ObjectType::Policy),
			/* 26 */ Attributes::ViewSchemaName,
			/* 27 */ Attributes::ViewName,
			/* 28 */ Attributes::ViewBody,
			/* 29 */ Attributes::ViewExtBody,
			/* 30 */ Attributes::ViewTitle,
			/* 31 */ Attributes::Alias,
			/* 32 */ Attributes::RefColumn,
			/* 33 */ Attributes::RefTable,
			/* 34 */ Attributes::Reference,
			/* 35 */ Attributes::ViewTogglerButtons,
			/* 36 */ Attributes::ViewTogglerBody,
			/* 37 */ BaseObject::getSchemaName(ObjectType::Textbox),
			/* 38 */ Attributes::Column,
			/* 39 */ Attributes::Column,
			/* 40 */ Attributes::InhColumn,
			/* 41 */ Attributes::ProtColumn,
			/* 42 */ Attributes::PkColumn,
			/* 43 */ Attributes::PkColumn,
			/* 44 */ Attributes::FkColumn,
			/* 45 */ Attributes::FkColumn,
			/* 46 */ Attributes::UqColumn,
			/* 47 */ Attributes::UqColumn,
			/* 48 */ Attributes::NnColumn,
			/* 49 */ Attributes::NnColumn,
			/* 50 */ Attributes::Relationship,
			/* 51 */ Attributes::Label,
			/* 52 */ Attributes::Label,
			/* 53 */ Attributes::Attribute,
			/* 54 */ Attributes::Attribute,
			/* 55 */ Attributes::Tag,
			/* 56 */ Attributes::Tag,
			/* 57 */ Attributes::Placeholder,
			/* 58 */ Attributes::ForeignTableSchemaName,
			/* 59 */ Attributes::ForeignTableName,
			/* 60 */ Attributes::ForeignTableBody,
			/* 61 */ Attributes::ForeignTableExtBody,
			/* 62 */ Attributes::ForeignTableTitle,
			/* 63 */ Attributes::ForeignTableTogglerButtons,
			/* 64 */ Attributes::ForeignTableTogglerBody };

	int i = 0, count = element_cmb->count();

	// This auxiliary vector stores the id of elements that represents color/font confing of graphical objects
	std::vector<int> obj_conf_ids { 2, 3, 5, 7, 8, 11, 12, 13, 14, 15,
																	17, 19, 21, 23, 25, 28, 29, 30,
																	34, 35, 36, 37, 39, 43, 45, 47, 49,
																	50, 52, 54, 56, 57, 60, 61, 62, 63, 64 };

	conf_items.resize(count);

	for(i = 0; i < count; i++)
	{
		conf_items[i].conf_id = conf_ids[i];
		conf_items[i].obj_conf = (std::find(obj_conf_ids.begin(), obj_conf_ids.end(), i) != obj_conf_ids.end());
	}

	elem_color_cp = new ColorPickerWidget(3, this);

	model = new DatabaseModel;
	scene = new ObjectsScene;
	placeholder = new RoundedRectItem;

	viewp = new QGraphicsView(scene);
	viewp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	viewp->setRenderHint(QPainter::Antialiasing);
	viewp->setRenderHint(QPainter::TextAntialiasing);
	viewp->setRenderHint(QPainter::SmoothPixmapTransform);
	viewp->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	viewp->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
	viewp->centerOn(0, 0);

	grid_color_cp = new ColorPickerWidget(1, this);
	grid_color_cp->setButtonToolTip(0, tr("Define a custom color for the grid lines"));
	grid_color_lt->addWidget(grid_color_cp);

	canvas_color_cp = new ColorPickerWidget(1, this);
	canvas_color_cp->setButtonToolTip(0, tr("Define a custom color for the canvas area"));
	canvas_color_lt->addWidget(canvas_color_cp);

	delimiters_color_cp = new ColorPickerWidget(1, this);
	delimiters_color_cp->setButtonToolTip(0, tr("Define a custom color for the page delimiter lines"));
	delimiters_color_lt->insertWidget(0, delimiters_color_cp);

	QGridLayout *grid = dynamic_cast<QGridLayout *>(objects_gb->layout());
	grid->addWidget(elem_color_cp, 3, 1, 1, 4);
	grid->addWidget(viewp, 4, 0, 1, 5);

	line_numbers_cp = new ColorPickerWidget(1, this);
	line_numbers_cp->setButtonToolTip(0, tr("Line numbers' font color"));

	line_numbers_bg_cp = new ColorPickerWidget(1, this);
	line_numbers_bg_cp->setButtonToolTip(0, tr("Line numbers' background color"));

	line_highlight_cp = new ColorPickerWidget(1, this);
	line_highlight_cp->setButtonToolTip(0, tr("Highlighted line color"));

	code_wgt_colors_lt->insertWidget(0, line_numbers_cp);
	code_wgt_colors_lt->insertWidget(0, line_numbers_bg_cp);
	code_wgt_colors_lt->insertWidget(0, line_highlight_cp);

	font_preview_txt = new NumberedTextEditor(this);
	font_preview_txt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	font_preview_txt->setPlainText(
					"-- object: public.foo | type: TABLE --\n\
CREATE TABLE public.table_b (\n \
\tid serial NOT NULL,\n \
\tsku integer NOT NULL,\n \
\tCONSTRAINT foo_pk PRIMARY KEY (id)\n\
);\n");

	font_preview_hl = new SyntaxHighlighter(font_preview_txt, false, true);

	QVBoxLayout *layout = new QVBoxLayout(code_preview_gb);
	layout->setContentsMargins(GuiUtilsNs::LtMargins);
	layout->addWidget(font_preview_txt);

	ui_theme_gb->layout()->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	code_style_gb->layout()->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	ico_sz_btn_grp = new QButtonGroup(this);
	ico_sz_btn_grp->setExclusive(true);

	icon_small_tb->setProperty(Attributes::IconsSize.toLatin1(), Attributes::Small);
	icon_medium_tb->setProperty(Attributes::IconsSize.toLatin1(), Attributes::Medium);
	icon_big_tb->setProperty(Attributes::IconsSize.toLatin1(), Attributes::Big);

	ico_sz_btn_grp->addButton(icon_small_tb);
	ico_sz_btn_grp->addButton(icon_medium_tb);
	ico_sz_btn_grp->addButton(icon_big_tb);

	theme_cmb->installEventFilter(this);

	connect(ico_sz_btn_grp, &QButtonGroup::buttonToggled, this, __slot(this, AppearanceConfigWidget::previewUiSettings));

	connect(element_cmb, &QComboBox::currentTextChanged, this, &AppearanceConfigWidget::enableConfigElement);
	connect(elem_font_cmb, &QFontComboBox::currentFontChanged, this, &AppearanceConfigWidget::applyElementFontStyle);
	connect(elem_font_size_spb, &QDoubleSpinBox::textChanged, this, &AppearanceConfigWidget::applyElementFontStyle);
	connect(bold_chk, &QToolButton::toggled, this, &AppearanceConfigWidget::applyElementFontStyle);
	connect(underline_chk, &QToolButton::toggled, this, &AppearanceConfigWidget::applyElementFontStyle);
	connect(italic_chk, &QToolButton::toggled, this, &AppearanceConfigWidget::applyElementFontStyle);

	connect(code_font_size_spb, &QDoubleSpinBox::textChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(code_font_cmb, &QFontComboBox::currentFontChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_numbers_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_numbers_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_numbers_bg_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_numbers_bg_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_highlight_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(line_highlight_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(disp_line_numbers_chk, &QCheckBox::toggled, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(hightlight_lines_chk, &QCheckBox::toggled, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(tab_width_spb, &QSpinBox::textChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(tab_width_chk, &QCheckBox::toggled, tab_width_spb, &QSpinBox::setEnabled);
	connect(tab_width_chk, &QCheckBox::toggled, this, &AppearanceConfigWidget::previewCodeFontStyle);
	connect(font_preview_txt, &NumberedTextEditor::cursorPositionChanged, this, &AppearanceConfigWidget::previewCodeFontStyle);

	connect(elem_color_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::applyElementColor);

	connect(elem_color_cp, &ColorPickerWidget::s_colorsChanged, this, [this]()
					{
		for(unsigned i=0; i < elem_color_cp->getColorCount(); i++)
			applyElementColor(i, elem_color_cp->getColor(i)); });

	connect(canvas_color_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(canvas_color_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(delimiters_color_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(delimiters_color_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(grid_color_cp, &ColorPickerWidget::s_colorChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(grid_color_cp, &ColorPickerWidget::s_colorsChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(grid_size_spb, &QSpinBox::textChanged, this, &AppearanceConfigWidget::previewCanvasColors);
	connect(grid_pattern_cmb, &QComboBox::currentIndexChanged, this, &AppearanceConfigWidget::previewCanvasColors);

	connect(theme_cmb, &QComboBox::activated, this, __slot(this, AppearanceConfigWidget::previewUiSettings));

	connect(custom_scale_chk, &QCheckBox::toggled, this, [this](bool toggled)
					{
		custom_scale_spb->setEnabled(toggled);
		setConfigurationChanged(true); });

	connect(custom_scale_spb, &QDoubleSpinBox::valueChanged, this, [this]()
					{ setConfigurationChanged(true); });

	connect(expansion_factor_spb, &QSpinBox::valueChanged, this, [this]()
					{ setConfigurationChanged(true); });

	connect(min_obj_opacity_spb, &QSpinBox::valueChanged, this, [this]()
					{ setConfigurationChanged(true); });

	connect(ext_attribs_per_page_spb, &QSpinBox::valueChanged, this, [this]()
					{ setConfigurationChanged(true); });

	connect(attribs_per_page_spb, &QSpinBox::valueChanged, this, [this]()
					{ setConfigurationChanged(true); });
}

AppearanceConfigWidget::~AppearanceConfigWidget()
{
	scene->blockSignals(true);
	scene->removeItem(placeholder);
	scene->blockSignals(false);

	delete placeholder;
	delete viewp;
	delete scene;
	delete model;
}

void AppearanceConfigWidget::showEvent(QShowEvent *)
{
	/* We store the current changed state of the configurations
	 * because the call to previewCanvasColors() always set
	 * the settings as changed and since the show event
	 * doens't need to change settings we do a small workaround.
	 *
	 * This will avoid reload appearance settings when the user
	 * just displayed the appearance settings but don't changed
	 * anything. */
	bool conf_changed = isConfigurationChanged();

	// Store the current state of grid/delimiters display
	show_grid = ObjectsScene::isShowGrid();
	show_delimiters = ObjectsScene::isShowPageDelimiters();
	previewCanvasColors();

	setConfigurationChanged(conf_changed);
}

void AppearanceConfigWidget::hideEvent(QHideEvent *)
{
	// Restore the original state of grid/delimiters display
	ObjectsScene::setShowGrid(show_grid);
	ObjectsScene::setShowPageDelimiters(show_delimiters);
}

std::map<QString, attribs_map> AppearanceConfigWidget::getConfigurationParams()
{
	return config_params;
}

void AppearanceConfigWidget::loadThemesConfiguration()
{
	QDir themes_root_dir { GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir) };
	QString theme_dir, pal_file, dtd_file { GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir, 
																																												 GlobalAttributes::ThemeConf) + 
																																												 GlobalAttributes::ObjectDTDExt };
	QStringList ignored_themes;
	std::vector<Exception> errors;
	QFileInfo fi;
	QStringList theme_files { GlobalAttributes::AppearanceConf, GlobalAttributes::PatternHighlightConf,
													  GlobalAttributes::SQLHighlightConf, GlobalAttributes::XMLHighlightConf,
													  GlobalAttributes::ThemeConf };

	theme_cmb->blockSignals(true);
	theme_cmb->clear();
	theme_cmb->blockSignals(false);
	theme_palettes.clear();
	theme_tab_item_colors.clear();

	/* Storing the original system palette as a fallback theme if none of the the
	 * other themes can't be loaded */
	QPalette pal = system_pal;

	/* A small adjustment in the system palette need to be made.
	 * In CustomUiStyle, the semantics of color roles Light, Midlight, Mid and Dark
	 * are the same from light color schemes (light is the brightest color and dark the darkest)
	 * to make the implementation more simple.
	 *
	 * The majority of dark palettes invert the semantics of that color roles being light the darkest one
	 * and dark the lightest one. So below we just adjust the palette to work as needed by CustomUiStyle. */
	if(CustomUiStyle::isDarkPalette(pal))
	{
		QColor light_cl;

		for(auto cl_group : { QPalette::Active, QPalette::Inactive, QPalette::Disabled })
		{
			light_cl = pal.color(cl_group, QPalette::Light);

			// If the light color is too dark in a dark palette, we set a minimum luminance to avoid very dark colors
			if(light_cl.lightness() < 40)
				light_cl.setHsl(light_cl.hue(), light_cl.saturation(), 40);

			pal.setColor(cl_group, QPalette::Light, pal.color(cl_group, QPalette::Dark));
			pal.setColor(cl_group, QPalette::Midlight, pal.color(cl_group, QPalette::Mid));
			pal.setColor(cl_group, QPalette::Mid, pal.color(cl_group, QPalette::Midlight));
			pal.setColor(cl_group, QPalette::Dark, light_cl);
			pal.setColor(cl_group, QPalette::Accent, pal.color(cl_group, QPalette::Highlight).lighter(CustomUiStyle::MinFactor));
		}
	}
	else
	{
		// Adjusting some color roles to have a minimum luminance in light palettes
		static const std::map<QPalette::ColorRole, int> role_ids{
						{QPalette::Light, 225}, {QPalette::Midlight, 200},
						{QPalette::Mid, 190}, {QPalette::Dark, 185},
						{QPalette::Button, 235}, {QPalette::Highlight, 180}};

		for(auto [rl_id, min_lum] : role_ids)
		{
			QColor cl = pal.color(QPalette::Active, rl_id);
			cl.setHsl(cl.hue(), cl.saturation(), min_lum);
			pal.setColor(QPalette::Active, rl_id, cl);
			pal.setColor(QPalette::Inactive, rl_id, cl);
			pal.setColor(QPalette::Disabled, rl_id, cl.darker(CustomUiStyle::XMinFactor));
		}

		pal.setColor(QPalette::Active, QPalette::Accent, pal.color(QPalette::Active, QPalette::Highlight));
		pal.setColor(QPalette::Inactive, QPalette::Accent, pal.color(QPalette::Inactive, QPalette::Highlight));
		pal.setColor(QPalette::Disabled, QPalette::Accent, pal.color(QPalette::Disabled, QPalette::Highlight).darker(CustomUiStyle::MinFactor));
	}

	theme_palettes[Attributes::System] = pal;

	theme_cmb->blockSignals(true);
	theme_cmb->addItem(tr("System default"), Attributes::System);
	theme_cmb->setItemData(0, tr("A theme based on current system color settings"), Qt::ToolTipRole);
	theme_cmb->blockSignals(false);

	static const std::map<QString, QPalette::ColorRole> role_ids{
								{Attributes::Light, QPalette::Light}, {Attributes::Midlight, QPalette::Midlight},
								{Attributes::Mid, QPalette::Mid}, {Attributes::Button, QPalette::Button},
								{Attributes::Dark, QPalette::Dark}, {Attributes::Base, QPalette::Base},
								{Attributes::Window, QPalette::Window}, {Attributes::Shadow, QPalette::Shadow},
								{Attributes::Text, QPalette::Text}, {Attributes::BrightText, QPalette::BrightText},
								{Attributes::ButtonText, QPalette::ButtonText}, {Attributes::WindowText, QPalette::WindowText},
								{Attributes::Highlight, QPalette::Highlight}, {Attributes::HighlightedText, QPalette::HighlightedText},
								{Attributes::Link, QPalette::Link}, {Attributes::LinkVisited, QPalette::LinkVisited},
								{Attributes::AlternateBase, QPalette::AlternateBase}, {Attributes::ToolTipBase, QPalette::ToolTipBase},
								{Attributes::ToolTipText, QPalette::ToolTipText}, {Attributes::PlaceholderText, QPalette::PlaceholderText},
								{Attributes::Accent, QPalette::Accent}};

	static const std::map<QString, QList<CustomTableWidget::TableItemColor>> tab_item_ids{
					{ Attributes::ProtItem, { CustomTableWidget::ProtItemBgColor, CustomTableWidget::ProtItemFgColor } },
					{ Attributes::ProtItemAlt, { CustomTableWidget::ProtItemAltFgColor } },
					{ Attributes::RelAddedItem, { CustomTableWidget::RelAddedItemBgColor, CustomTableWidget::RelAddedItemFgColor } },
					{ Attributes::RelAddedItemAlt, { CustomTableWidget::RelAddedItemAltFgColor } },
					{ Attributes::AddedItem, { CustomTableWidget::AddedItemBgColor, CustomTableWidget::AddedItemFgColor } },
					{ Attributes::UpdatedItem, { CustomTableWidget::UpdatedItemBgColor, CustomTableWidget::UpdatedItemFgColor } },
					{ Attributes::RemovedItem, { CustomTableWidget::RemovedItemBgColor, CustomTableWidget::RemovedItemFgColor } }}	;

	std::map<QString, attribs_map> theme_conf;

	for(auto &theme_id : themes_root_dir.entryList({"*"}, QDir::Dirs | QDir::NoDotAndDotDot))
	{
		/* Skip the system theme because it's has a different directory structure
		 * which is used as fallback theme (see applyUiTheme) */
		if(theme_id == Attributes::System)
			continue;

		theme_dir = themes_root_dir.absolutePath() + GlobalAttributes::DirSeparator + theme_id;

		for(auto &file : theme_files)
		{
			fi.setFile(theme_dir + GlobalAttributes::DirSeparator + file + GlobalAttributes::ConfigurationExt);

			if(!fi.exists() || !fi.isReadable())
			{
				ignored_themes.append(theme_id);
				break;
			}

			if(file == GlobalAttributes::ThemeConf)
			{
				pal_file = fi.absoluteFilePath();

				try
				{	
					BaseConfigWidget::loadConfiguration(pal_file, GlobalAttributes::ThemeConf, theme_conf, 
																							{ Attributes::Role, GlobalAttributes::ThemeConf });

					// Storing theme palette colors
					for(auto &[role_attr, cl_role] : role_ids)
					{
						pal.setColor(QPalette::Active, cl_role, QColor(theme_conf[role_attr][Attributes::Active]));
						pal.setColor(QPalette::Inactive, cl_role, QColor(theme_conf[role_attr][Attributes::Inactive]));
						pal.setColor(QPalette::Disabled, cl_role, QColor(theme_conf[role_attr][Attributes::Disabled]));
					}

					theme_palettes[theme_id] = pal;

					// Storing tab widget item colors for the current theme
					for(auto &[item_attr, item_ids] : tab_item_ids)
					{
						
						if(item_attr.endsWith("Alt"))
						{
							theme_tab_item_colors[theme_id][item_ids.front()] =
								QColor(theme_conf[item_attr][Attributes::FgColor]);
						}
						else
						{
							theme_tab_item_colors[theme_id][item_ids.front()] =
								QColor(theme_conf[item_attr][Attributes::BgColor]);

							theme_tab_item_colors[theme_id][item_ids.back()] =
								QColor(theme_conf[item_attr][Attributes::FgColor]);
						}
					}

					theme_cmb->blockSignals(true);
					theme_cmb->addItem(theme_conf[GlobalAttributes::ThemeConf][Attributes::Name], theme_id);
					theme_cmb->setItemData(theme_cmb->count() - 1,
																 theme_conf[Attributes::Description][Attributes::Contents].trimmed(),
																 Qt::ToolTipRole);
					theme_cmb->blockSignals(false);
				}
				catch(Exception &e)
				{
					ignored_themes.append(theme_id);
					errors.push_back(Exception(e, __PRETTY_FUNCTION__, __FILE__, __LINE__));
				}
			}
		}
	}

	if(!ignored_themes.isEmpty())
	{
		Messagebox::error(tr("Unable to load theme(s) <strong>%1</strong> in path <strong>%2</strong> due to some missing or corrputed configuration file(s)! pgModeler will fall back to the system default theme.")
										  .arg(ignored_themes.join(", "), themes_root_dir.absolutePath()),
						ErrorCode::Custom, __PRETTY_FUNCTION__, __FILE__, __LINE__, errors);
	}
}

void AppearanceConfigWidget::loadExampleModel()
{
	try
	{
		if(model->getObjectCount() != 0)
			return;

		RelationshipView *rel = nullptr;
		StyledTextboxView *txtbox = nullptr;
		TableView *tab = nullptr;
		GraphicalView *view = nullptr;
		unsigned count = 0, i = 0;

		model->loadModel(GlobalAttributes::getTmplConfigurationFilePath("", GlobalAttributes::ExampleModel));

		count = model->getObjectCount(ObjectType::Table);
		for(i = 0; i < count; i++)
		{
			tab = new TableView(model->getTable(i));
			scene->addItem(tab);
			tab->setEnabled(false);
		}

		count = model->getObjectCount(ObjectType::ForeignTable);
		for(i = 0; i < count; i++)
		{
			tab = new TableView(model->getForeignTable(i));
			scene->addItem(tab);
			tab->setEnabled(false);
		}

		count = model->getObjectCount(ObjectType::View);
		for(i = 0; i < count; i++)
		{
			view = new GraphicalView(model->getView(i));
			scene->addItem(view);
			view->setEnabled(false);
		}

		count = model->getObjectCount(ObjectType::Relationship);
		for(i = 0; i < count; i++)
		{
			rel = new RelationshipView(model->getRelationship(i, ObjectType::Relationship));
			scene->addItem(rel);
			rel->setEnabled(false);
		}

		count = model->getObjectCount(ObjectType::BaseRelationship);
		for(i = 0; i < count; i++)
		{
			rel = new RelationshipView(model->getRelationship(i, ObjectType::BaseRelationship));
			scene->addItem(rel);
			rel->setEnabled(false);
		}

		count = model->getObjectCount(ObjectType::Textbox);
		for(i = 0; i < count; i++)
		{
			txtbox = new StyledTextboxView(model->getTextbox(i));
			scene->addItem(txtbox);
			txtbox->setEnabled(false);
		}

		placeholder->setRect(QRectF(400, 150, 200, 150));
		updatePlaceholderItem();
		scene->addItem(placeholder);
		scene->setActiveLayers(QList<unsigned>({0}));
		scene->setSceneRect(scene->itemsBoundingRect(false));
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void AppearanceConfigWidget::updatePlaceholderItem()
{
	placeholder->setBrush(BaseObjectView::getFillStyle(Attributes::Placeholder));
	QPen pen = BaseObjectView::getBorderStyle(Attributes::Placeholder);
	pen.setStyle(Qt::DashLine);
	placeholder->setPen(pen);
}

void AppearanceConfigWidget::loadConfiguration()
{
	try
	{
		/* Load the available themes palettes before loading the appearance config file
		 * because the appearance config file contains the name of the theme to use */
		loadThemesConfiguration();

		BaseConfigWidget::loadConfiguration(GlobalAttributes::AppearanceConf, config_params, {Attributes::Id}, true);

		theme_cmb->blockSignals(true);
		ico_sz_btn_grp->blockSignals(true);

		QString icon_size = config_params[GlobalAttributes::AppearanceConf][Attributes::IconsSize];

		int idx = theme_cmb->findData(config_params[GlobalAttributes::AppearanceConf][Attributes::UiTheme], Qt::UserRole, Qt::MatchExactly);

		theme_cmb->setCurrentIndex(idx < 0 ? 0 : idx);

		if(icon_size == Attributes::Big)
			icon_big_tb->setChecked(true);
		if(icon_size == Attributes::Medium)
			icon_medium_tb->setChecked(true);
		else
			icon_small_tb->setChecked(true);

		theme_cmb->blockSignals(false);
		ico_sz_btn_grp->blockSignals(false);

		custom_scale_chk->setChecked(config_params[GlobalAttributes::AppearanceConf].count(Attributes::CustomScale));
		custom_scale_spb->setValue(config_params[GlobalAttributes::AppearanceConf][Attributes::CustomScale].toDouble());
		expansion_factor_spb->setValue(config_params[Attributes::Design][Attributes::ExpansionFactor].toUInt());

		applyConfiguration();
		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e, e.getExtraInfo());
	}
}

void AppearanceConfigWidget::applyDesignCodeStyle()
{
	grid_pattern_cmb->setCurrentIndex((config_params[Attributes::Design][Attributes::GridPattern].isEmpty() || config_params[Attributes::Design][Attributes::GridPattern] == Attributes::Square) ? 0 : 1);
	grid_size_spb->setValue((config_params[Attributes::Design][Attributes::GridSize]).toUInt());
	min_obj_opacity_spb->setValue(config_params[Attributes::Design][Attributes::MinObjectOpacity].toUInt());
	attribs_per_page_spb->setValue(config_params[Attributes::Design][Attributes::AttribsPerPage].toUInt());
	ext_attribs_per_page_spb->setValue(config_params[Attributes::Design][Attributes::ExtAttribsPerPage].toUInt());

	/* If we can't identify at least one of the colors that compose the grid then we use default colors
	 * avoiding black canvas or black grid color */
	if(config_params[Attributes::Design].count(Attributes::GridColor) == 0 || config_params[Attributes::Design].count(Attributes::CanvasColor) == 0 || config_params[Attributes::Design].count(Attributes::DelimitersColor) == 0)
	{
		grid_color_cp->setColor(0, ObjectsScene::DefaultGridColor);
		canvas_color_cp->setColor(0, ObjectsScene::DefaultCanvasColor);
		delimiters_color_cp->setColor(0, ObjectsScene::DefaultDelimitersColor);
	}
	else
	{
		grid_color_cp->setColor(0, QColor(config_params[Attributes::Design][Attributes::GridColor]));
		canvas_color_cp->setColor(0, QColor(config_params[Attributes::Design][Attributes::CanvasColor]));
		delimiters_color_cp->setColor(0, QColor(config_params[Attributes::Design][Attributes::DelimitersColor]));
	}

	code_font_cmb->setCurrentFont(QFont(config_params[Attributes::Code][Attributes::Font]));
	code_font_size_spb->setValue(config_params[Attributes::Code][Attributes::FontSize].toDouble());
	disp_line_numbers_chk->setChecked(config_params[Attributes::Code][Attributes::DisplayLineNumbers] == Attributes::True);
	hightlight_lines_chk->setChecked(config_params[Attributes::Code][Attributes::HighlightLines] == Attributes::True);
	line_numbers_cp->setColor(0, config_params[Attributes::Code][Attributes::LineNumbersColor]);
	line_numbers_bg_cp->setColor(0, config_params[Attributes::Code][Attributes::LineNumbersBgColor]);
	line_highlight_cp->setColor(0, config_params[Attributes::Code][Attributes::LineHighlightColor]);

	int tab_width = (config_params[Attributes::Code][Attributes::TabWidth]).toInt();
	tab_width_chk->setChecked(tab_width > 0);
	tab_width_spb->setEnabled(tab_width_chk->isChecked());
	tab_width_spb->setValue(tab_width);
}

void AppearanceConfigWidget::applyObjectsStyle()
{
	QTextCharFormat font_fmt;
	QFont font;
	attribs_map attribs;
	QStringList list, colors;
	QString elem;

	for(auto &itr : config_params)
	{
		elem = itr.first;
		attribs = itr.second;

		if(elem == Attributes::Global)
		{
			font.setFamily(attribs[Attributes::Font]);
			font.setPointSizeF(attribs[Attributes::Size].toDouble());
			font.setBold(attribs[Attributes::Bold] == Attributes::True);
			font.setItalic(attribs[Attributes::Italic] == Attributes::True);
			font.setUnderline(attribs[Attributes::Underline] == Attributes::True);
			font_fmt.setFont(font);
			BaseObjectView::setFontStyle(elem, font_fmt);
		}
		else if(elem.startsWith(Attributes::Font + "-"))
		{
			elem.remove(Attributes::Font + "-");
			font = font_fmt.font();
			font.setBold(attribs[Attributes::Bold] == Attributes::True);
			font.setItalic(attribs[Attributes::Italic] == Attributes::True);
			font.setUnderline(attribs[Attributes::Underline] == Attributes::True);
			font_fmt.setFont(font);
			font_fmt.setForeground(QColor(attribs[Attributes::Color]));
			BaseObjectView::setFontStyle(elem, font_fmt);
		}
		else if(elem.startsWith(Attributes::Object + "-"))
		{
			elem.remove(Attributes::Object + "-");
			list = attribs[Attributes::FillColor].split(',');

			colors.clear();
			colors.append(!list.isEmpty() ? list.at(0) : "#000");
			colors.append(list.size() == 2 ? list.at(1) : colors.at(0));
			BaseObjectView::setElementColor(elem, QColor(colors.at(0)), ColorId::FillColor1);
			BaseObjectView::setElementColor(elem, QColor(colors.at(1)), ColorId::FillColor2);
			BaseObjectView::setElementColor(elem, QColor(attribs[Attributes::BorderColor]), ColorId::BorderColor);
		}
	}

	for(auto &cnf_item : conf_items)
	{
		if(cnf_item.obj_conf)
		{
			BaseObjectView::getFillStyle(cnf_item.conf_id, cnf_item.colors[0], cnf_item.colors[1]);
			cnf_item.colors[2] = BaseObjectView::getBorderStyle(cnf_item.conf_id).color();
		}
		else
			cnf_item.font_fmt = BaseObjectView::getFontStyle(cnf_item.conf_id);
	}

	enableConfigElement();
	elem_font_cmb->setCurrentFont(BaseObjectView::getFontStyle(Attributes::Global).font());
}

void AppearanceConfigWidget::saveConfiguration()
{
	try
	{
		attribs_map attribs;
		AppearanceConfigItem item;
		QString attrib_id;
		QFont font;

		config_params.erase(GlobalAttributes::AppearanceConf);
		attribs[Attributes::UiTheme] = theme_cmb->currentData(Qt::UserRole).toString();
		attribs[Attributes::IconsSize] = ico_sz_btn_grp->checkedButton()->property(Attributes::IconsSize.toLatin1()).toString();

		attribs[Attributes::CustomScale] = custom_scale_chk->isChecked() ? QString::number(custom_scale_spb->value(), 'g', 2) : "";

		config_params[Attributes::UiTheme] = attribs;
		attribs.clear();

		attribs[Attributes::GridSize] = QString::number(grid_size_spb->value());
		attribs[Attributes::GridPattern] = grid_pattern_cmb->currentIndex() == 0 ? Attributes::Square : Attributes::Dot;
		attribs[Attributes::MinObjectOpacity] = QString::number(min_obj_opacity_spb->value());
		attribs[Attributes::AttribsPerPage] = QString::number(attribs_per_page_spb->value());
		attribs[Attributes::ExtAttribsPerPage] = QString::number(ext_attribs_per_page_spb->value());
		attribs[Attributes::GridColor] = grid_color_cp->getColor(0).name();
		attribs[Attributes::CanvasColor] = canvas_color_cp->getColor(0).name();
		attribs[Attributes::DelimitersColor] = delimiters_color_cp->getColor(0).name();
		attribs[Attributes::ExpansionFactor] = QString::number(expansion_factor_spb->value());

		config_params[Attributes::Design] = attribs;
		attribs.clear();

		attribs[Attributes::Font] = code_font_cmb->currentText();
		attribs[Attributes::FontSize] = QString::number(code_font_size_spb->value());
		attribs[Attributes::DisplayLineNumbers] = (disp_line_numbers_chk->isChecked() ? Attributes::True : "");
		attribs[Attributes::HighlightLines] = (hightlight_lines_chk->isChecked() ? Attributes::True : "");
		attribs[Attributes::LineNumbersColor] = line_numbers_cp->getColor(0).name();
		attribs[Attributes::LineNumbersBgColor] = line_numbers_bg_cp->getColor(0).name();
		attribs[Attributes::LineHighlightColor] = line_highlight_cp->getColor(0).name();
		attribs[Attributes::TabWidth] = QString::number(tab_width_chk->isChecked() ? tab_width_spb->value() : 0);

		config_params[Attributes::Code] = attribs;
		attribs.clear();

		for(auto &item : conf_items)
		{
			// If the item is a object color config
			if(item.obj_conf)
			{
				// Creates an attribute that stores the fill color
				attrib_id = item.conf_id + "-color";
				if(item.colors[0] == item.colors[1])
					attribs[attrib_id] = item.colors[0].name();
				else
					attribs[attrib_id] = item.colors[0].name() + "," + item.colors[1].name();

				// Creates an attribute that stores the border color
				attrib_id = item.conf_id + "-bcolor";
				attribs[attrib_id] = item.colors[2].name();
			}
			// If the item is a font config
			else if(item.conf_id != Attributes::Global && !item.obj_conf)
			{
				font = item.font_fmt.font();

				// Creates an attribute to store the font color
				attrib_id = item.conf_id + "-fcolor";
				attribs[attrib_id] = item.font_fmt.foreground().color().name();

				attrib_id = item.conf_id + "-" + Attributes::Italic;
				attribs[attrib_id] = (font.italic() ? Attributes::True : Attributes::False);

				attrib_id = item.conf_id + "-" + Attributes::Bold;
				attribs[attrib_id] = (font.bold() ? Attributes::True : Attributes::False);

				attrib_id = item.conf_id + "-" + Attributes::Underline;
				attribs[attrib_id] = (font.underline() ? Attributes::True : Attributes::False);
			}
			// Special case: treating the global font element
			else
			{
				attribs[Attributes::Global + "-font"] = QFontInfo(item.font_fmt.font()).family();
				attribs[Attributes::Global + "-font-size"] = QString("%1").arg(item.font_fmt.font().pointSizeF());
			}
		}

		config_params[Attributes::Objects] = attribs;
		BaseConfigWidget::saveConfiguration(GlobalAttributes::AppearanceConf, config_params);

		QString hl_theme = getThemeId();

		/* Copying the syntax highilighting files from the selected theme folder to the user's storage
		 * in order to reflect the new syntax highlighting setting in the whole application */
		QStringList theme_hl_files = {
									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + hl_theme,
													GlobalAttributes::SQLHighlightConf + GlobalAttributes::ConfigurationExt),

									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + hl_theme,
													GlobalAttributes::XMLHighlightConf + GlobalAttributes::ConfigurationExt),

									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + hl_theme,
													GlobalAttributes::SchHighlightConf + GlobalAttributes::ConfigurationExt),
					},

					orig_hl_files = {
									GlobalAttributes::getSQLHighlightConfPath(),
									GlobalAttributes::getXMLHighlightConfPath(),
									GlobalAttributes::getSchHighlightConfPath(),
									GlobalAttributes::getPatternHighlightConfPath(),
					};

		for(int i = 0; i < 3; i++)
		{
			QFile::remove(orig_hl_files[i]);

			if(!QFile::copy(theme_hl_files[i], orig_hl_files[i]))
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(orig_hl_files[i]),
								PGM_FUNC, PGM_FILE, PGM_LINE, nullptr,
								QFileInfo(theme_hl_files[i]).isReadable() ? tr("The template file `%1' could not be accessed!").arg(theme_hl_files[i]) : "");
			}
		}

		setConfigurationChanged(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void AppearanceConfigWidget::enableConfigElement()
{
	int idx = element_cmb->currentIndex();

	// Widgets enabled only when the global font element is selected (idx==0)
	elem_font_cmb->setEnabled(idx == 0);
	font_lbl->setEnabled(idx == 0);
	elem_font_size_spb->setEnabled(idx == 0);
	unity_lbl->setEnabled(idx == 0);

	// Widgets enabled when a font configuration element is selected
	underline_chk->setEnabled(idx != 0 && !conf_items[idx].obj_conf);
	bold_chk->setEnabled(idx != 0 && !conf_items[idx].obj_conf);
	italic_chk->setEnabled(idx != 0 && !conf_items[idx].obj_conf);

	colors_lbl->setVisible(idx != 0);
	elem_color_cp->setVisible(colors_lbl->isVisible());

	// Buttons visible when a object configuration element is selected
	elem_color_cp->setButtonVisible(1, conf_items[idx].obj_conf);
	/* The border color picker is hidden only for Attributes:ObjShadow since
	 * this element has no border drawn */
	elem_color_cp->setButtonVisible(2,
					conf_items[idx].obj_conf && conf_items[idx].conf_id != Attributes::ObjShadow);

	underline_chk->blockSignals(true);
	italic_chk->blockSignals(true);
	bold_chk->blockSignals(true);
	elem_font_cmb->blockSignals(true);
	elem_font_size_spb->blockSignals(true);

	if(!conf_items[idx].obj_conf)
	{
		QTextCharFormat fmt = BaseObjectView::getFontStyle(conf_items[idx].conf_id);
		elem_color_cp->setColor(0, fmt.foreground().color());
		underline_chk->setChecked(fmt.font().underline());
		italic_chk->setChecked(fmt.font().italic());
		bold_chk->setChecked(fmt.font().bold());
		elem_font_cmb->setCurrentFont(fmt.font());
		elem_font_size_spb->setValue(fmt.font().pointSizeF());
	}
	else
	{
		QColor color1, color2;
		BaseObjectView::getFillStyle(conf_items[idx].conf_id, color1, color2);

		elem_color_cp->setColor(0, color1);
		elem_color_cp->setColor(1, color2);
		elem_color_cp->setColor(2, BaseObjectView::getBorderStyle(conf_items[idx].conf_id).color());
		underline_chk->setChecked(false);
		italic_chk->setChecked(false);
		bold_chk->setChecked(false);
	}

	underline_chk->blockSignals(false);
	italic_chk->blockSignals(false);
	bold_chk->blockSignals(false);
	elem_font_cmb->blockSignals(false);
	elem_font_size_spb->blockSignals(false);
}

void AppearanceConfigWidget::applyElementColor(unsigned color_idx, QColor color)
{
	if(conf_items[element_cmb->currentIndex()].obj_conf)
	{
		conf_items[element_cmb->currentIndex()].colors[color_idx] = color;
		BaseObjectView::setElementColor(conf_items[element_cmb->currentIndex()].conf_id,
						color, static_cast<ColorId>(color_idx));
		updatePlaceholderItem();
	}
	else if(color_idx == 0)
	{
		conf_items[element_cmb->currentIndex()].font_fmt.setForeground(color);
		BaseObjectView::setFontStyle(conf_items[element_cmb->currentIndex()].conf_id,
						conf_items[element_cmb->currentIndex()].font_fmt);
	}

	model->setObjectsModified();
	scene->update();
	setConfigurationChanged(true);
}

void AppearanceConfigWidget::applyConfiguration()
{
	applyUiTheme();
	applyDesignCodeStyle();
	applyObjectsStyle();
	previewCanvasColors();

	BaseTableView::setAttributesPerPage(BaseTable::AttribsSection, attribs_per_page_spb->value());
	BaseTableView::setAttributesPerPage(BaseTable::ExtAttribsSection, ext_attribs_per_page_spb->value());
	ModelWidget::setMinimumObjectOpacity(min_obj_opacity_spb->value());
	ObjectsScene::setExpansionFactor(expansion_factor_spb->value());
	GuiUtilsNs::updateDropShadows(qApp->allWidgets());

	loadExampleModel();
	model->setObjectsModified();
	updatePlaceholderItem();
	scene->update();
}

void AppearanceConfigWidget::applyElementFontStyle()
{
	QFont font;

	font = elem_font_cmb->currentFont();
	font.setBold(bold_chk->isChecked());
	font.setItalic(italic_chk->isChecked());
	font.setUnderline(underline_chk->isChecked());
	font.setPointSizeF(elem_font_size_spb->value());

	conf_items[element_cmb->currentIndex()].font_fmt.setFont(font);
	BaseObjectView::setFontStyle(conf_items[element_cmb->currentIndex()].conf_id,
					conf_items[element_cmb->currentIndex()].font_fmt);

	model->setObjectsModified();
	scene->update();
	setConfigurationChanged(true);
}

void AppearanceConfigWidget::restoreDefaults()
{
	try
	{
		BaseConfigWidget::restoreDefaults(GlobalAttributes::AppearanceConf, false);
		loadConfiguration();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void AppearanceConfigWidget::previewCodeFontStyle()
{
	QFont fnt;

	fnt = code_font_cmb->currentFont();
	fnt.setPointSizeF(code_font_size_spb->value());

	SyntaxHighlighter::setDefaultFont(fnt);
	NumberedTextEditor::setDefaultFont(fnt);
	NumberedTextEditor::setLineNumbersVisible(disp_line_numbers_chk->isChecked());
	NumberedTextEditor::setLineHighlightColor(line_highlight_cp->getColor(0));
	NumberedTextEditor::setHighlightLines(hightlight_lines_chk->isChecked());
	NumberedTextEditor::setTabDistance(tab_width_chk->isChecked() ? tab_width_spb->value() : 0);
	LineNumbersWidget::setColors(line_numbers_cp->getColor(0), line_numbers_bg_cp->getColor(0));

	font_preview_txt->setReadOnly(false);
	font_preview_txt->resizeWidgets();
	font_preview_txt->updateLineNumbers();
	font_preview_txt->highlightCurrentLine();
	font_preview_txt->setReadOnly(true);
	font_preview_hl->rehighlight();

	setConfigurationChanged(true);
}

void AppearanceConfigWidget::previewCanvasColors()
{
	ObjectsScene::setShowGrid(true);
	ObjectsScene::setShowPageDelimiters(true);

	ObjectsScene::setCanvasColor(canvas_color_cp->getColor(0));
	ObjectsScene::setGridPattern(grid_pattern_cmb->currentIndex() == 0 ? ObjectsScene::SquarePattern : ObjectsScene::DotPattern);

	ObjectsScene::setGridColor(grid_color_cp->getColor(0));
	ObjectsScene::setPageDelimitersColor(delimiters_color_cp->getColor(0));
	ObjectsScene::setGridSize(grid_size_spb->value());

	scene->update();
	setConfigurationChanged(true);
}

void AppearanceConfigWidget::applyUiTheme()
{
	QPalette pal;
	QString ui_theme = theme_cmb->currentData(Qt::UserRole).toString();

	UiThemeId = ui_theme;

	// Configuring the tab item colors for the selected theme
	for(auto &[tb_item_id, tb_item_cl] : theme_tab_item_colors[ui_theme])
		CustomTableWidget::setTableItemColor(tb_item_id, tb_item_cl);

	pal = theme_palettes[ui_theme];
	qApp->setPalette(pal);

	/* if(CustomUiStyle::isDarkPalette(qApp->palette()))
	{
		// Forcing QMenu class to use a lighter base color
		pal.setColor(QPalette::Base, pal.color(QPalette::Base));
		qApp->setPalette(pal, "QMenu");
	} */

	applySyntaxHighlightTheme();
	applyUiStyleSheet();
	setConfigurationChanged(true);
}

QString AppearanceConfigWidget::getThemeId()
{
	QString theme_id = theme_cmb->currentData(Qt::UserRole).toString();

	if(theme_cmb->currentIndex() > 0)
		return theme_id;

	return CustomUiStyle::isDarkPalette(theme_palettes[theme_id]) ? Attributes::Dark : Attributes::Light;
}

bool AppearanceConfigWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == theme_cmb && event->type() == QEvent::ToolTip)
	{
		QToolTip::showText(QCursor::pos(), 
											 "<p>" + theme_cmb->currentData(Qt::ToolTipRole).toString() + "</p>",
											 theme_cmb);
		return true;
	}

	return BaseConfigWidget::eventFilter(object, event);
}

void AppearanceConfigWidget::previewUiSettings()
{
	qApp->setOverrideCursor(Qt::WaitCursor);
	applyUiTheme();
	applyDesignCodeTheme();

	model->setObjectsModified();
	scene->update();

	GuiUtilsNs::updateDropShadows(qApp->allWidgets());
	qApp->restoreOverrideCursor();
}

void AppearanceConfigWidget::applySyntaxHighlightTheme()
{
	QString filename = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + getThemeId(),
					GlobalAttributes::SQLHighlightConf + GlobalAttributes::ConfigurationExt);

	try
	{
		font_preview_hl->loadConfiguration(filename);
		font_preview_hl->rehighlight();
		setConfigurationChanged(true);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void AppearanceConfigWidget::applyDesignCodeTheme()
{
	QString filename = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + getThemeId(),
					GlobalAttributes::AppearanceConf + GlobalAttributes::ConfigurationExt);

	try
	{
		BaseConfigWidget::loadConfiguration(filename, GlobalAttributes::AppearanceConf, config_params, {Attributes::Id}, true);

		applyDesignCodeStyle();
		applyObjectsStyle();
		previewCodeFontStyle();
		previewCanvasColors();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void AppearanceConfigWidget::applyUiStyleSheet()
{
	QFile ui_style(GlobalAttributes::getTmplConfigurationFilePath("",
					GlobalAttributes::UiStyleConf + GlobalAttributes::ConfigurationExt));

	ui_style.open(QFile::ReadOnly);

	if(!ui_style.isOpen())
	{
		Messagebox::error(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(ui_style.fileName()),
						ErrorCode::FileDirectoryNotAccessed, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
	else
	{
		QByteArray ui_stylesheet = ui_style.readAll();

		QString icon_size = ico_sz_btn_grp->checkedButton()->property(Attributes::IconsSize.toLatin1()).toString(),
				ico_style_conf = GlobalAttributes::getTmplConfigurationFilePath("",
								"icons-" + icon_size + GlobalAttributes::ConfigurationExt);
		QString ui_theme = getThemeId(),
				extra_style_conf, prefix = "extra-";

		/* Special case for extra style sheet of System's theme:
		 * This theme has two extra style sheets: dark-extra-ui-style.conf
		 * and light-extra-ui-style.conf. We use the one according
		 * to the current system palette */
		if(theme_cmb->currentData(Qt::UserRole).toString() == Attributes::System)
		{
			prefix.prepend(CustomUiStyle::isDarkPalette(theme_palettes[Attributes::System]) ? "dark-" : "light-");

			// Forcing the theme id to be "system" so the files can be found correctly
			ui_theme = Attributes::System;
		}

		extra_style_conf = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ThemesDir + GlobalAttributes::DirSeparator + ui_theme,
						prefix + GlobalAttributes::UiStyleConf + GlobalAttributes::ConfigurationExt);

		if(QFileInfo::exists(extra_style_conf))
		{
			QFile extra_style(extra_style_conf);

			if(extra_style.open(QFile::ReadOnly))
				ui_stylesheet.append(extra_style.readAll());
		}

		if(!ico_style_conf.isEmpty())
		{
			QFile ico_style(ico_style_conf);
			ico_style.open(QFile::ReadOnly);

			if(!ico_style.isOpen())
			{
				Messagebox::error(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(ico_style_conf),
								ErrorCode::FileDirectoryNotAccessed, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
			else
				ui_stylesheet.append(ico_style.readAll());
		}

		/* Forcing the title element of group box to have a font size 85% of
		 * the app's original/global font size */
		qApp->setStyleSheet(ui_stylesheet);

		// Overriding pixel metrics of small icons in table headers, menu icons, etc
		int small_ico_sz = 0;

		if(icon_size == Attributes::Small)
			small_ico_sz = 16;
		else if(icon_size == Attributes::Medium)
			small_ico_sz = 20;
		else
			small_ico_sz = 24;

		CustomUiStyle::setPixelMetricValue(QStyle::PM_SmallIconSize, small_ico_sz);
	}
}
