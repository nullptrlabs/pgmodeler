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
\class MatchInfo
\brief Auxiliary class that holds where a word matching starts and finishes in syntax highlighting process.
*/

#ifndef MATCH_INFO_H
#define MATCH_INFO_H

class MatchInfo {
	protected:
		int start, end;

	public:
		MatchInfo();
		MatchInfo(int start, int end);
		~MatchInfo() = default;

		int getStart() const;
		int getEnd() const;
		int getLength() const;

		/*! \brief Returns true when the provided position is within
		 * boundaries of the this match info */
		bool contains(int pos) const;

		virtual bool isEmpty() const;

		//! \brief Returns true when the start/end are non-negative and start < end
		virtual bool isValid() const;

		virtual void clear();

		virtual bool operator == (const MatchInfo &m_info) const;

		friend class SyntaxHighlighter;
};

#endif
