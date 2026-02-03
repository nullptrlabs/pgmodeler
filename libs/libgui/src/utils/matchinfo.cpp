/*
# PostgreSQL Database Modeler (pgModeler)
#
# (c) Copyright 2006-2026 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# DEVELOPMENT, MAINTENANCE AND COMMERCIAL DISTRIBUTION BY:
# Nullptr Labs Software e Tecnologia LTDA <contact@nullptrlabs.io>
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

#include "matchinfo.h"

MatchInfo::MatchInfo()
{
	start = end = -1;
}

MatchInfo::MatchInfo(int start, int end)
{
	this->start = start;
	this->end = end;
}

int MatchInfo::getLength() const
{
	if(!isValid())
		return 0;

	return end - start + 1;
}

bool MatchInfo::isEmpty() const
{
	return getLength() == 0;
}

bool MatchInfo::isValid() const
{
	return start >= 0 && start <= end;
}

void MatchInfo::clear()
{
	start = end = -1;
}

bool MatchInfo::contains(int pos) const
{
	return pos >= 0 && pos >= start && pos <= end;
}

int MatchInfo::getStart() const
{
	return start;
}

int MatchInfo::getEnd() const
{
	return end;
}

bool MatchInfo::operator == (const MatchInfo &m_info) const
{
	return start == m_info.start && end == m_info.end;
}
