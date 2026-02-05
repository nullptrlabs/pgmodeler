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
\ingroup libparsers
\class CsvDocument
\brief This class implements basic operations to store a parsed CSV document in memory.
*/

#ifndef CSV_DOCUMENT_H
#define CSV_DOCUMENT_H

#include "parsersglobal.h"
#include <QStringList>

class __libparsers CsvDocument {
	private:
		//! \brief Indicates the character used as values separator
		QChar separator,

		//! \brief Indicates the character used as text delimiter
		text_delim,

		//! \brief Indicates the character used as line break
		line_break;

		//! \brief Stores the columns names
		QStringList columns;

		//! \brief Stores the rows of the parsed CSV document
		QList<QStringList> rows;

		//! \brief Defines the column names
		void setColumns(const QStringList &cols);

		/*! \brief Add a single row to the document. This method will check
		 * if the number of colums of the provided row matches the document's
		 * number of column, raising an error if they don't match. */
		void addRow(const QStringList &row);

		void setSpecialChars(const QChar &sep, const QChar &txt_delim, const QChar &ln_break);

		CsvDocument(const QChar &sep, const QChar &txt_delim, const QChar &ln_break);

	public:
		//! \brief Default character used as text delimiter
		static const QChar TextDelimiter,

		//! \brief Default character used as value separator
		Separator,

		//! \brief Default character used as line break
		LineBreak;

		CsvDocument();

		int getRowCount() const;

		int getColumnCount() const;

		QStringList getColumnNames() const;

		QString getValue(int row, int col) const;

		bool isEmpty() const;

		/*! \brief Saves the parsed document into a file.
		 * This method use thes original document's separator, delimiter and line break
		 * characters when creating the buffer that is saved to the file */
		void saveToFile(const QString &filename) const;

		friend class CsvParser;
};

#endif
