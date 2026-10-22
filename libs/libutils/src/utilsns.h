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
#include <QCoreApplication>

/*! \brief Marks a text, written as a literal, to be extracted by lupdate in the translation context
 * ctx. It expands to the literal itself, nothing is translated here: the translation is done where the
 * text is used, by UtilsNs::pgmTr(). It's meant to texts that can't call tr() where they are declared,
 * like static members (initialized before the translator is installed). The context is always
 * UtilsNs::TrContext, so a text is written as PGM_TR2("Global", "Text").
 *
 * IMPORTANT: lupdate only knows this macro as an alias of QT_TRANSLATE_NOOP, so it must be run with
 * the option -tr-function-alias "QT_TRANSLATE_NOOP+=PGM_TR2". The context must be a literal at the
 * macro call, lupdate doesn't expand macros so it can't be hidden inside this one. */
#define PGM_TR2(ctx, text) QT_TRANSLATE_NOOP(ctx, text)

namespace UtilsNs {
	static constexpr char OrigFontProp[] { "original-font" },

	//! \brief The translation context shared by all the texts marked with PGM_TR2 and translated by pgmTr()
	TrContext[] { "Global" };

	inline const QString EntityAmp("&amp;"),
	EntityLt("&lt;"),
	EntityGt("&gt;"),
	EntityQuot("&quot;"),
	EntityApos("&apos;"),

	/*! \brief Default char for data/value separator for special usage.
	 * Uses middle dot (·, U+00B7) instead of bullet (•, U+2022) to ensure compatibility
	 * with non-UTF8 database encodings such as LATIN1/ISO-8859-1. The middle dot exists
	 * in LATIN1 (0xB7) while bullet does not, preventing encoding conversion errors when
	 * connecting to databases with different character encodings.
	 * \warning This change may break compatibility with models created in previous versions
	 * that used bullet (•) as the data separator. */
	DataSeparator("·"),

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
	 * When mk_path is true, the function tries to create the base path until the filename
	 * Raises an exception in case of the file couldn,t be open */
	__libutils void saveFile(const QString &filename, const QByteArray &buffer, bool mk_path = false);

	/*! \brief Restricts the access to the file to its owner only (read/write), used in files that hold sensitive
	 * data like passwords. Does nothing if the file doesn't exist and in Windows, where the permissions model is different.
	 * Returns false if the permissions couldn't be changed. */
	__libutils bool setOwnerOnlyAccess(const QString &filename);

	/*! \brief Read the contents of the file specified by its filename returning its contents.
	 * Raises an exception in case of the file couldn't be open.
	 * The optional parameter max_len can be specified and determines the maximum length of
	 * data read from the file. If max_len is <= 0 then the function returns the entire content
	 * of the file. */
	__libutils QByteArray loadFile(const QString &filename, qint64 max_len = 0);

	__libutils QString getTemporaryFilePath(const QString &abs_filepath_tmpl);

	/*! \brief Translates the text in the context TrContext, the one used by PGM_TR2. Texts not found
	 * in the loaded translations are returned as they are */
	__libutils QString pgmTr(const char *text);

	//! \brief Convenience version of pgmTr() for texts stored in a QString (e.g. static members marked with PGM_TR2)
	__libutils QString pgmTr(const QString &text);

	//! \brief Converts any chars (operators) < > " to the respective XML entities.
	__libutils QString convertToXmlEntities(const QString value);

	__libutils QString getStringHash(const QString &string,
																					QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5);

	__libutils QString getStringHash(const QByteArray &string,
																					QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5);

	//! \brief Replaces the sequence of chars [`'] by html tags <strong></strong> and [()] by <em></em>
	__libutils QString formatMessage(const QString &msg);

	//! \brief This function dumps a stack trace based on the OS signal receveid
	__libutils QString generateStackTrace(int signal);
}

#endif
