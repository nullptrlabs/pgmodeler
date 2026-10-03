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

#include "utilsns.h"
#include "exception.h"
#include "globalattributes.h"
#include <QFile>
#include <QRegularExpression>
#include <cstdlib>
#include <QTemporaryFile>
#include <QDir>

#ifndef Q_OS_WIN
	#include "execinfo.h"
	#include <cxxabi.h>
#endif

#ifndef Q_OS_WIN
namespace {
	QString demangleStackSymbol(const char *raw_symbol)
	{
		QString symbol = QString::fromUtf8(raw_symbol);
		int open_paren = symbol.indexOf('('),
			plus_sign = symbol.indexOf('+', open_paren);

		if(open_paren < 0 || plus_sign < 0 || plus_sign <= open_paren + 1)
			return symbol;

		QByteArray mangled_name = symbol.mid(open_paren + 1, plus_sign - open_paren - 1).toUtf8();
		int demangle_status = -1;
		char *demangled_name = abi::__cxa_demangle(mangled_name.constData(), nullptr, nullptr, &demangle_status);

		if(demangle_status == 0 && demangled_name)
			symbol.replace(open_paren + 1, plus_sign - open_paren - 1, QString::fromUtf8(demangled_name));

		if(demangled_name)
			free(demangled_name);

		return symbol;
	}
}
#endif

namespace UtilsNs {

	void saveFile(const QString &filename, const QByteArray &buffer, bool mk_path)
	{
		QFile output;

		// Trying to create the full path to the file
		if(mk_path)
		{
			QFileInfo fi(filename);
			QDir dir = fi.absoluteDir();

			if(!dir.exists())
				dir.mkpath(fi.absolutePath());
		}

		output.setFileName(filename);

		if(!output.open(QFile::WriteOnly))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(output.fileName()),
											ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE,
											nullptr, output.errorString());
		}

		output.write(buffer);
		output.close();
	}

	QByteArray loadFile(const QString &filename, qint64 max_len)
	{
		QFile input;

		input.setFileName(filename);

		if(!input.open(QFile::ReadOnly))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(input.fileName()),
											ErrorCode::FileDirectoryNotAccessed,PGM_FUNC,PGM_FILE,PGM_LINE,
											nullptr, input.errorString());
		}

		/* In order to avoid storing the contents of the file in a local variable
		 * and returning it making two copies we just return the result of readAll().
		 * The file descriptor will be closed in the destructor of QFile */
		if(max_len <= 0)
			// Read all contents if the max_len is not set
			return input.readAll();

		// Reading only the first max_len bytes of the file
		return input.read(max_len);
	}

	QString pgmTr(const char *text)
	{
		return QCoreApplication::translate(TrContext, text);
	}

	QString pgmTr(const QString &text)
	{
		return pgmTr(text.toUtf8().constData());
	}

	QString convertToXmlEntities(QString value)
	{
		/* If the extracted value has one of the expected special chars
		 * in order to perform the replacemnt to xml entities */
		if(value.contains(QRegularExpression("(&|\\<|\\>|\")")))
		{
			if(!value.contains(UtilsNs::EntityQuot) && !value.contains(UtilsNs::EntityLt) &&
				 !value.contains(UtilsNs::EntityGt) && !value.contains(UtilsNs::EntityAmp) &&
				 !value.contains(UtilsNs::EntityApos) && value.contains('&'))
					value.replace('&', UtilsNs::EntityAmp);

				value.replace('"', UtilsNs::EntityQuot);
				value.replace('<', UtilsNs::EntityLt);
				value.replace('>', UtilsNs::EntityGt);
		}

		return value;
	}

	QString getStringHash(const QString &string, QCryptographicHash::Algorithm algorithm)
	{
		return getStringHash(string.toUtf8(), algorithm);
	}

	QString getStringHash(const QByteArray &string, QCryptographicHash::Algorithm algorithm)
	{
		return QCryptographicHash::hash(string, algorithm).toHex();
	}

	QString formatMessage(const QString &msg)
	{
		QString fmt_msg=msg;
		QChar start_chrs[2]={'`','('},
				end_chrs[2]={'\'', ')'};
		QStringList start_tags={ "<strong>", "<em>(" },
				end_tags={ "</strong>", ")</em>" };
		int pos=-1, pos1=-1;

					 // Replacing the form `' by <strong></strong> and () by <em></em>
		for(int chr_idx=0; chr_idx < 2; chr_idx++)
		{
			pos=0;
			do
			{
				pos=fmt_msg.indexOf(start_chrs[chr_idx], pos);
				pos1=fmt_msg.indexOf(end_chrs[chr_idx], pos);

				if(pos >= 0 && pos1 >=0)
				{
					fmt_msg.replace(pos, 1 , start_tags[chr_idx]);
					pos1 += start_tags[chr_idx].length() - 1;
					fmt_msg.replace(pos1, 1, end_tags[chr_idx]);
				}
				else
					break;

				pos=pos1;
			}
			while(pos >= 0 && pos < fmt_msg.size());
		}

		fmt_msg.replace("\n", "<br/>");

		return fmt_msg;
	}

	QString generateStackTrace(int signal)
	{
		#ifndef Q_OS_WIN
			void *stack[30];
			size_t stack_size;
			char **symbols=nullptr;
			stack_size = backtrace(stack, 30);
			symbols = backtrace_symbols(stack, stack_size);
		#endif

		QStringList s_trace;

		s_trace.append(QString("** pgModeler crashed after receive signal: %1 **\n\nDate/Time: %2 \nVersion: %3 \nBuild: %4 \n")
									 .arg(signal)
									 .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
												GlobalAttributes::PgModelerVersion,
												GlobalAttributes::PgModelerBuildNumber));

		s_trace.append(QString("Compilation Qt version: %1\nRunning Qt version: %2\n")
									 .arg(QT_VERSION_STR)
									 .arg(qVersion()));

		#ifndef Q_OS_WIN
			for(size_t i = 0; i < stack_size; i++)
				s_trace.append(QString("[%1] ").arg(stack_size-1-i) + demangleStackSymbol(symbols[i]));

			free(symbols);
		#else
			s_trace.append("** Stack trace unavailable on Windows system **");
		#endif

		return s_trace.join('\n');
	}

	QString getTemporaryFilePath(const QString &abs_filepath_tmpl)
	{
		QTemporaryFile temp(abs_filepath_tmpl);

		if(!temp.open())
			return "";

		QString temp_file = temp.fileName();
		temp.close();

		return temp_file;
	}
}
