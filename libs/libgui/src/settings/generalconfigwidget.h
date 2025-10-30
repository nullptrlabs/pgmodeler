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
\class GeneralConfigWidget
\brief Implements the operations to manage general configurations.
*/

#ifndef GENERAL_CONFIG_WIDGET_H
#define GENERAL_CONFIG_WIDGET_H

#include "ui_generalconfigwidget.h"
#include "baseconfigwidget.h"
#include "widgets/fileselectorwidget.h"

class __libgui GeneralConfigWidget: public BaseConfigWidget, public Ui::GeneralConfigWidget {
	Q_OBJECT

	private:
		/* This simple struct is used to store the widgets geometry and maximized state
		 * in order to save this info to configuration file being possible to restore
		 * it when reloading the application */
		struct WidgetState
		{
		  QRect geometry;
		  bool maximized;
		  WidgetState() { maximized = false; }
		};

		QWidgetList child_wgts;

		FileSelectorWidget *confs_dir_sel,
		*source_editor_sel;

		static std::map<QString, WidgetState> widgets_geom;

		static std::map<QString, attribs_map> config_params;

		enum UnitId: unsigned {
			UnitMilimeters,
			UnitPoint,
			UnitInches,
			UnitCentimeters
		};

	protected:
		void showEvent(QShowEvent *) override;

	public:
		//! \brief Maximum number of files listed as recent models
		static constexpr int MaxRecentModels=15;

		GeneralConfigWidget(QWidget * parent = nullptr);

		void saveConfiguration() override;

		void loadConfiguration() override;

		/*! \brief Fills the configuration section section_id with the provided attributes map.
		 *  This method replaces an entire configuration section, if you need to just append
		 *  a configuration parameter without erasing the existing ones use appendConfigurationParam() */
		static void setConfigurationSection(const QString &section_id, const attribs_map &params);

		//! \brief Removes a configuration section by matching its name with the provided regular expression
		static void removeConfigurationSection(const QRegularExpression &section_regex);

		//! \brief Appends/overwrites a configuration parameter of the provided configuration section
		static void appendConfigurationSection(const QString &section_id, const attribs_map &params);

		//! \brief Returns a map containing all the configuration parameters
		static std::map<QString, attribs_map> getConfigurationParams();

		/*! \brief Returns a single value of a configuration param in the specified section id.
		Section id can be <configuration>, <dock-widget>, <file[n]> or <recent[n]> */
		static QString getConfigurationParam(const QString &section_id, const QString &param_name);

		static void saveWidgetGeometry(QWidget *widget, const QString &custom_wgt_name = "");
		static bool restoreWidgetGeometry(QWidget *widget, const QString &custom_wgt_name = "");

	public slots:
		void applyConfiguration() override;
		void restoreDefaults() override;
		void selectPaperSize();

	private slots:
		void setConfigurationChanged(bool changed = true) override;
		void convertMarginUnity();
		void resetDialogsSizes();
		void resetAlertChoices();
};

#endif
