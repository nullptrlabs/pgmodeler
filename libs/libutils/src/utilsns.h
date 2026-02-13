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
\ingroup libutils
\namespace UtilsNs
\brief This namespace is used to reunite all functions or constants that can be used globally by all subprojects.
*/

#ifndef UTILS_NS_H
#define UTILS_NS_H

#include "utilsglobal.h"
#include <QString>
#include <QCryptographicHash>

namespace UtilsNs {
	inline const QString EntityAmp("&amp;"),
	EntityLt("&lt;"),
	EntityGt("&gt;"),
	EntityQuot("&quot;"),
	EntityApos("&apos;"),

	//! \brief Default char for data/value separator for special usage
	DataSeparator("•"),

	//! \brief Indicates the wildcard filtering mode in the object listing
	FilterWildcard("wildcard"),

	//! \brief Indicates the regexp (POSIX) filtering mode in the object listing
	FilterRegExp("regexp");

	//! \brief Default char used as unescaped value start delimiter
	static const QChar UnescValueStart='/',

	//! \brief Default char used as unescaped value end delimiter
	UnescValueEnd='/',

	//! \brief Indicates the default wildcard character expected to be found in wildcard patterns
	WildcardChar('*'),

	//! \brief Indicates the character used to separate filter fields in the filtering string
	FilterSeparator = ':';

	/*! \brief Writes the provided buffer to the file specified by its filename
	 * Raises an exception in case of the file couldn,t be open */
	__libutils void saveFile(const QString &filename, const QByteArray &buffer);

	/*! \brief Read the contents of the file specified by its filename returning its contents.
	 * Raises an exception in case of the file couldn't be open.
	 * The optional parameter max_len can be specified and determines the maximum length of
	 * data read from the file. If max_len is <= 0 then the function returns the entire content
	 * of the file. */
	__libutils QByteArray loadFile(const QString &filename, qint64 max_len = 0);

	//! \brief Converts any chars (operators) < > " to the respective XML entities.
	__libutils QString convertToXmlEntities(const QString value);

	__libutils QString getStringHash(const QString &string,
																					QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5);

	__libutils QString getStringHash(const QByteArray &string,
																					QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5);


	//! \brief Replaces the sequence of chars [`'] by html tags <strong></strong> and [()] by <em></em>
	__libutils QString formatMessage(const QString &msg);
}

#endif
