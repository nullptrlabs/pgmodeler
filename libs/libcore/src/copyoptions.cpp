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

#include "copyoptions.h"
#include "exception.h"
#include "enumtype.h"

CopyOptions::CopyOptions()
{
	copy_mode = NoMode;
	copy_opts = NoOpts;
}

CopyOptions::CopyOptions(CopyMode copy_mode, CopyOpts copy_opts)
{
	if(copy_mode > Excluding || copy_opts > All)
		throw Exception(ErrorCode::RefInvalidLikeOptionType,PGM_FUNC,PGM_FILE,PGM_LINE);

	this->copy_mode = copy_mode;
	this->copy_opts = copy_opts;
}

CopyOptions::CopyMode CopyOptions::getCopyMode()
{
	return copy_mode;
}

bool CopyOptions::isOptionSet(CopyOpts op)
{
	if(op > All)
		throw Exception(ErrorCode::RefInvalidLikeOptionType,PGM_FUNC,PGM_FILE,PGM_LINE);

	return ((copy_opts & op) == op);
}

bool CopyOptions::isIncluding()
{
	return copy_mode == Including;
}

bool CopyOptions::isExcluding()
{
	return copy_mode == Excluding;
}

CopyOptions::CopyOpts CopyOptions::getCopyOptions()
{
	return copy_opts;
}

QString CopyOptions::getSQLDefinition()
{
	QString def, mode, op_name;
	CopyOpts op_id,
			opts[] ={ All, Defaults, Constraints, Indexes,
								Storage, Comments, Identity, Statistics };

	mode = (copy_mode == Including ? " INCLUDING" : " EXCLUDING");

	if(copy_mode != NoMode && copy_opts != NoOpts)
	{
		for(auto opt : opts)
		{
			op_id = copy_opts & opt;

			switch(op_id)
			{
				case All: op_name=" ALL"; break;
				case Defaults: op_name=" DEFAULTS"; break;
				case Constraints: op_name=" CONSTRAINTS"; break;
				case Indexes: op_name=" INDEXES"; break;
				case Storage: op_name=" STORAGE"; break;
				case Comments: op_name=" COMMENTS"; break;
				case Identity: op_name=" IDENTITY"; break;
				case Statistics: op_name=" STATISTICS"; break;
				default: op_name = ""; break;
			}

			if(!op_name.isEmpty())
			{
				def += mode + op_name;
				op_name.clear();
			}

			if(op_id == All) break;
		}
	}

	return def;
}

bool CopyOptions::operator != (CopyOptions &cp)
{
	return (this->copy_mode!= cp.copy_mode && this->copy_opts!=cp.copy_opts);
}
