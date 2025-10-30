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
\class FragmentInfo
\brief Auxiliary class used by TextBlockInfo and SyntaxHighlight that holds text fragments format info
and other useful information that helps in syntax highlighting process in a document.
*/

#ifndef FRAGMENT_INFO_H
#define FRAGMENT_INFO_H

#include "guiglobal.h"
#include <QString>
#include "matchinfo.h"

class	__libgui	FragmentInfo: public MatchInfo {
	private:
		//! \brief The name of the group that formatted the fragment
		QString group;

		//! \brief Indicates if the fragment is open (only for multiline expressions)
		bool open,

		//! \brief Indicates if the fragment is closed (only for multiline expressions)
		closed,

		/*! \brief Indicates if the fragment accepts code completion when the cursor is
		 *  between the start and end positions */
		allow_completion;

	public:
		FragmentInfo();

		FragmentInfo(const QString &grp, int start, int end,
								 bool open, bool closed, bool allow_compl);

		~FragmentInfo() = default;

		QString getGroup() const;
		bool isOpen() const;
		bool isClosed() const;
		bool isValid() const override;
		bool isCompletionAllowed() const;
};

#endif
