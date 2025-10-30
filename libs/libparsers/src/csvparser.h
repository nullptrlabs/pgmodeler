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
\ingroup libparsers
\class CsvParser
\brief This class implements basic operations to parse CSV documents based upon RFC 4180 (https://www.rfc-editor.org/rfc/rfc4180)
*/

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <QString>
#include <QList>
#include "csvdocument.h"

class __libparsers CsvParser {
	private:
		//! \brief Indicates the character used as values separator
		QChar separator,

		//! \brief Indicates the character used as text delimiter
		text_delim,

		//! \brief Indicates the character used as line break
		line_break;

		//! \brief Indicates if the parsed document contains the column names in the first row
		bool cols_in_first_row;

		//! \brief The CSV document which is stored in memory and parsed
		QString buffer;

		//! \brief Indicates the current linear position in which the parser is in.
		int curr_pos,

		//! \brief Indicates the current row in which the parser is in.
		curr_row;

		//! \brief Extract and returns a single value from a row in the buffer
		QString extractValue();

		//! \brief Extract and returns a list of values that defines a single row in the buffer
		QStringList extractRow();

	public:
		CsvParser();

		void setSpecialChars(const QChar &sep, const QChar &txt_delim, const QChar &ln_break);

		void setColumnInFirstRow(bool value);

		//! \brief Parses a CSV document by loading the contents from a file
		CsvDocument parseFile(const QString &filename);

		//! \brief Parses a CSV document defined in a string buffer
		CsvDocument parseBuffer(const QString &csv_buf);
};

#endif
