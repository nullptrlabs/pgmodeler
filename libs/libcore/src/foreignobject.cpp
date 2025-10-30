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

#include "foreignobject.h"
#include "exception.h"
#include "attributes.h"
#include "schemaparser.h"

const QString ForeignObject::OptionsSeparator {","};
const QString ForeignObject::OptionValueSeparator {"="};

ForeignObject::ForeignObject() = default;

void ForeignObject::setOption(const QString &opt, const QString &value)
{
	if(opt.isEmpty())
		throw Exception(ErrorCode::AsgOptionInvalidName,PGM_FUNC,PGM_FILE,PGM_LINE);

	options[opt] = value;
}

void ForeignObject::setOptions(const attribs_map &options)
{
	for(auto &itr : options)
	{
		if(itr.first.isEmpty())
			throw Exception(ErrorCode::AsgOptionInvalidName,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	this->options = options;
}

void ForeignObject::removeOption(const QString &opt)
{
	options.erase(opt);
}

void ForeignObject::removeOptions()
{
	options.clear();
}

attribs_map ForeignObject::getOptions()
{
	return options;
}

void ForeignObject::getAlteredAttributes(ForeignObject *fobj, attribs_map &fo_attribs)
{
	if(!fobj)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	attribs_map attribs;
	QStringList opts;

	// Comparing options (to be modified or added)
	for(auto &opt : fobj->options)
	{
		if(this->options.count(opt.first) == 0)
			opts.push_back(QString("ADD %1 '%2'").arg(opt.first).arg(opt.second));
		else if(this->options[opt.first] != opt.second)
			opts.push_back(QString("SET %1 '%3'").arg(opt.first).arg(opt.second));
	}

	// Comparing options (to be removed)
	for(auto &opt : this->options)
	{
		if(fobj->options.count(opt.first) == 0)
			opts.push_back(QString("DROP %1").arg(opt.first));
	}

	if(!opts.isEmpty())
		fo_attribs[Attributes::Options] = opts.join(OptionsSeparator);
}

QString ForeignObject::getOptionsAttribute(unsigned def_type)
{
	QStringList fmt_options;

	for(auto &itr : options)
		fmt_options += def_type == SchemaParser::SqlCode ?
																QString("%1 '%2'").arg(itr.first).arg(itr.second) :
																QString("%1%2%3").arg(itr.first).arg(OptionValueSeparator).arg(itr.second);

	return fmt_options.join(OptionsSeparator);
}
