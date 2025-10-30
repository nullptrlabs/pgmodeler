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
\class BaseConfigWidget
\brief Implements base operations to load/save XML based configuration files. This class reunites basic methods to perform
	   the load and save operation. Each subclass must satisfy the contract (implement abstract methods) and have their own
	   static configuration parameter set as following: std::map<QStrig, attribs_map>
*/

#ifndef BASE_CONF_WIDGET_H
#define BASE_CONF_WIDGET_H

#include "guiglobal.h"
#include "xmlparser.h"
#include "schemaparser.h"
#include <QWidget>

class __libgui BaseConfigWidget: public QWidget {
	Q_OBJECT

	private:	
		bool config_changed;
		
	protected:
		XmlParser xmlparser;
		SchemaParser schparser;
		
		/*! \brief Saves the configuration params on file. The conf_id param indicates the type of
			configuration to be saved. (see GlobalAttributes::*_CONF) and config_params the map containing the
			configuration values */
		void saveConfiguration(const QString &conf_id, std::map<QString, attribs_map> &config_params);
		
		/*! \brief Loads a configuration from filename.
		 *
		 * The parameter dtd indicates the DTD prefix/file that will be used to validate the config file being loaded
		 *
		 * The vector key_attribs is used to specify the xml element name  considered as a key on the configuration map
		 *
		 * The parametre incl_elem_name is used only when key attribs is not empty and when true, causes the current tag/element name to be
		 * prepended to the values retrieved by using key_attribs.
		 *
		 * For example, having the xml:
		 *	<objects>
		 *			<font id="obj1" attr="foo">
		 *			<font id="obj2" attr="bar">
		 *			<object id="obj1" attr="foo">
		 *			<object id="obj2" attr="bar">
		 *	</object>
		 *
		 * If incl_elem_name = false then the resulting attribs map would contain only 2 elements for font and objects,
		 * because we have two elements identified by "obj1" and "obj2". Now, when incl_elem_name=true the resulting attribs map
		 * would contain four elements which keys are font-obj1, font-obj2, object-obj1, object-obj2. */
		void loadConfiguration(const QString &filename, const QString &dtd, std::map<QString, attribs_map> &config_params,
													 const QStringList &key_attribs = {}, bool incl_elem_name = false);

		/*! \brief This is a convinience method that takes only the configuration file id instead of the filename and the DTD root element name.
		 *  The conf_id is used to identify both the config file and its related DTD file. */
		void loadConfiguration(const QString &conf_id, std::map<QString, attribs_map> &config_params,
													 const QStringList &key_attribs={}, bool incl_elem_name = false);
		
		//! \brief Get a configuratoin key from the xml parser
		void getConfigurationParams(std::map<QString, attribs_map> &config_params, const QStringList &key_attribs = {}, bool incl_elem_name = false);
		
		/*! \brief Restore the configuration specified by conf_in loading them from the original file (conf/defaults)
		 * The silent parameter indicates that the restoration should not emit a message box informing the restoration sucess */
		void restoreDefaults(const QString &conf_id, bool silent);
		
		//! \brief Set the configuration parameters to a configuration section
		static void setConfigurationSection(std::map<QString, attribs_map> &config_params, const QString &section_id, const attribs_map &params);
		
		//! \brief Appends/overwrites the configuration parameters of the provided section_id.
		static void appendConfigurationSection(std::map<QString, attribs_map> &config_params, const QString &section_id, const attribs_map &params);

		void showEvent(QShowEvent *) override;

	public:
		BaseConfigWidget(QWidget *parent = nullptr);
		~BaseConfigWidget() override = default;

		bool isConfigurationChanged();
		
		//! \brief Applies the configuration to object
		virtual void applyConfiguration()=0;
		
		//! \brief Loads a set of configurations from a file
		virtual void loadConfiguration()=0;
		
		//! \brief Saves the current settings to a file
		virtual void saveConfiguration()=0;
		
		//! \brief Destroy the current configuration file and makes a copy of the default one located at conf/defaults
		virtual void restoreDefaults()=0;
		
	public slots:
		virtual void setConfigurationChanged(bool changed = true);

	signals:
		void s_configurationChanged(bool);
};

#endif
