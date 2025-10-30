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

#include "fragmentinfo.h"

FragmentInfo::FragmentInfo()
{
	open = closed = allow_completion = false;
}

FragmentInfo::FragmentInfo(const QString &grp, int start, int end,
													 bool open, bool closed, bool allow_compl) : MatchInfo(start, end)
{
	group = grp;
	this->open = open;
	this->closed = closed;
	allow_completion = allow_compl;
}

QString FragmentInfo::getGroup() const
{
	return group;
}

bool FragmentInfo::isOpen() const
{
	return open;
}

bool FragmentInfo::isClosed() const
{
	return closed;
}

bool FragmentInfo::isCompletionAllowed() const
{
	return allow_completion;
}

bool FragmentInfo::isValid() const
{
	return !group.isEmpty() && MatchInfo::isValid();
}
