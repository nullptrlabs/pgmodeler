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

#include "baseconfigwidget.h"
#include "messagebox.h"
#include <QDate>
#include "utilsns.h"
#include "attributes.h"
#include "globalattributes.h"

BaseConfigWidget::BaseConfigWidget(QWidget *parent) : QWidget(parent)
{
	config_changed=false;
}

void BaseConfigWidget::setConfigurationSection(std::map<QString, attribs_map> &config_params, const QString &section_id, const attribs_map &params)
{
	if(section_id.isEmpty() || params.empty())
		return;

	config_params[section_id]=params;
}

void BaseConfigWidget::appendConfigurationSection(std::map<QString, attribs_map> &config_params, const QString &section_id, const attribs_map &params)
{
	if(section_id.isEmpty() || params.empty())
		return;

	for(auto &itr : params)
		config_params[section_id][itr.first] = itr.second;
}

void BaseConfigWidget::showEvent(QShowEvent *)
{
	config_changed=false;
}

void BaseConfigWidget::setConfigurationChanged(bool changed)
{
	config_changed = changed;
	emit s_configurationChanged(changed);
}

bool BaseConfigWidget::isConfigurationChanged()
{
	return config_changed;
}

void BaseConfigWidget::saveConfiguration(const QString &conf_id, std::map<QString, attribs_map> &config_params)
{
	//Configures the schema filename for the configuration
	QString	sch_filename=GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir,
																																			conf_id + GlobalAttributes::SchemaExt),

			//Cofnigures the filename for the configuration file
			cfg_filename = GlobalAttributes::getConfigurationFilePath(conf_id);

	attribs_map attribs;

	try
	{
		for(auto &itr : config_params)
			attribs.insert(itr.second.begin(), itr.second.end());

		//Generates the configuration from the schema file
		schparser.ignoreEmptyAttributes(true);
		UtilsNs::saveFile(cfg_filename, schparser.getSourceCode(sch_filename, attribs).toUtf8());

		config_params.erase(conf_id);
	}
	catch(Exception &e)
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::FileNotWrittenInvalidDefinition).arg(cfg_filename),
										ErrorCode::FileNotWrittenInvalidDefinition,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void BaseConfigWidget::restoreDefaults(const QString &conf_id, bool silent)
{
	QString current_file, default_file;

	//Build the path to the current configuration (conf/[conf_id].conf
	current_file=GlobalAttributes::getConfigurationFilePath(conf_id);

	//Build the path to the default configuration file (conf/defaults/[conf_id].conf
	default_file=GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::DefaultConfsDir,
																															conf_id +
																															GlobalAttributes::ConfigurationExt);
	//Raises an error if the default file doesn't exists
	if(!QFile::exists(default_file))
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::DefaultConfigNotRestored).arg(default_file),
										ErrorCode::DefaultConfigNotRestored,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	bool bkp_saved = false;
	QFileInfo fi(current_file);
	QDir dir;
	QString bkp_dir = fi.absolutePath() + GlobalAttributes::DirSeparator + GlobalAttributes::ConfsBackupsDir,
			bkp_filename = bkp_dir + GlobalAttributes::DirSeparator +
										 QString("%1.bkp_%2").arg(fi.fileName()).arg(QDateTime::currentDateTime().toString("yyyyMMd_hhmmss"));

	dir.mkpath(bkp_dir);
	bkp_saved = QFile::rename(current_file, bkp_filename);
	QFile::copy(default_file, current_file);

	// Set write permissions when copying file with read-only permissions
	QFile file(current_file);

	if(!file.permissions().testFlag(QFile::WriteOwner))
		file.setPermissions(file.permissions() | QFileDevice::WriteOwner);

	if(bkp_saved && !silent)
		Messagebox::info(tr("A backup of the previous settings was saved into <strong>%1</strong>!").arg(bkp_filename));
}

void BaseConfigWidget::loadConfiguration(const QString &filename, const QString &dtd, std::map<QString, attribs_map> &config_params,
																				 const QStringList &key_attribs, bool incl_elem_name)
{
	try
	{
		config_params.clear();
		xmlparser.restartParser();

		xmlparser.setDTDFile(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir,
																																				dtd + GlobalAttributes::ObjectDTDExt), dtd);
		xmlparser.loadXMLFile(filename);

		// Get the attributes of the root element
		this->getConfigurationParams(config_params, key_attribs, incl_elem_name);

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			std::map<QString, QStringList> chld_elems_attrs;

			do
			{
				if(xmlparser.getElementType()==XML_ELEMENT_NODE)
				{
					this->getConfigurationParams(config_params, key_attribs, incl_elem_name);

					if(xmlparser.hasElement(XmlParser::ChildElement, XML_ELEMENT_NODE))
					{
						xmlparser.savePosition();
						xmlparser.accessElement(XmlParser::ChildElement);

						if(xmlparser.getElementType()!=XML_TEXT_NODE)
						{
							do
							{
								this->getConfigurationParams(config_params, key_attribs, incl_elem_name);
							}
							while(xmlparser.accessElement(XmlParser::NextElement));
						}

						xmlparser.restorePosition();
					}
				}
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, filename);
	}
}

void BaseConfigWidget::loadConfiguration(const QString &conf_id, std::map<QString, attribs_map> &config_params, const QStringList &key_attribs, bool incl_elem_name)
{
	try
	{
		QString filename = GlobalAttributes::getConfigurationFilePath(conf_id);
		loadConfiguration(filename, conf_id, config_params, key_attribs, incl_elem_name);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e, e.getExtraInfo());
	}
}

void BaseConfigWidget::getConfigurationParams(std::map<QString, attribs_map> &config_params, const QStringList &key_attribs, bool incl_elem_name)
{
	attribs_map aux_attribs;
	attribs_map::iterator itr, itr_end;
	QString key;

	xmlparser.getElementAttributes(aux_attribs);

	itr=aux_attribs.begin();
	itr_end=aux_attribs.end();

	while(itr != itr_end && key.isEmpty())
	{
		if(key.isEmpty() && std::find(key_attribs.begin(), key_attribs.end(), itr->first)!=key_attribs.end())
		{
			key = itr->second;

			if(incl_elem_name)
				key.prepend(xmlparser.getElementName() + "-");

			break;
		}

		itr++;
	}

	if(key.isEmpty())
		key = xmlparser.getElementName();

	//Extract the contents of the child element and create a special element on map called "contents"
	if(xmlparser.hasElement(XmlParser::ChildElement, XML_TEXT_NODE))
	{
		xmlparser.savePosition();
		xmlparser.accessElement(XmlParser::ChildElement);
		aux_attribs[Attributes::Contents]=xmlparser.getElementContent();
		xmlparser.restorePosition();
	}

	if(!aux_attribs.empty())
		config_params[key]=aux_attribs;
}

