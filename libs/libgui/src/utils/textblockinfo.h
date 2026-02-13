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

/**
\ingroup libgui
\class TextBlockInfo
\brief Auxiliary class used by SyntaxHighlight which stores highlighting states of text blocks in a document.
*/

#ifndef TEXT_BLOCK_INFO_H
#define TEXT_BLOCK_INFO_H

#include <QTextBlockUserData>
#include <QList>
#include "fragmentinfo.h"

class __libgui TextBlockInfo: public QTextBlockUserData {
	private:
		//! \brief Holds all text fragment infos on the current text block
		QList<FragmentInfo> frag_infos;

		/*! \brief Holds the name of the group that was left open
		 *  when the syntax highlighter parsed the block. This is
		 *  use to indicate that the next block need to be formatted
		 *  like this one. See SyntaxHighlighter::highlightBlock() */
		QString open_group;

	public:
		TextBlockInfo();

		//! \brief Clears the group name and set all flags to false
		void reset();

		//! \brief Register a text fragment
		void addFragmentInfo(const FragmentInfo &f_info);

		/*! \brief Returns a text fragment in which the provided start/end
		 * positions are between the fragment's start and end positions */
		const FragmentInfo *getFragmentInfo(int start, int end);

		void setOpenGroup(const QString &grp);

		QString getOpenGroup();

		/*! \brief Return true if the position in the text block accepts
		 *  the code completion widget to be triggered */
		bool isCompletionAllowed(int pos);
};

#endif
