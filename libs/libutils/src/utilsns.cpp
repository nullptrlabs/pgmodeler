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

#include "utilsns.h"
#include "exception.h"
#include <QFile>
#include <QRegularExpression>
#include <QCryptographicHash>

namespace UtilsNs {
	void saveFile(const QString &filename, const QByteArray &buffer)
	{
		QFile output;

		output.setFileName(filename);
		output.open(QFile::WriteOnly);

		if(!output.isOpen())
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(output.fileName()),
											ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE,
											nullptr, output.errorString());
		}

		output.write(buffer);
		output.close();
	}

	QByteArray loadFile(const QString &filename)
	{
		QFile input;

		input.setFileName(filename);
		input.open(QFile::ReadOnly);

		if(!input.isOpen())
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(input.fileName()),
											ErrorCode::FileDirectoryNotAccessed,PGM_FUNC,PGM_FILE,PGM_LINE,
											nullptr, input.errorString());
		}

		/* In order to avoid storing the contents of the file in a local variable
		 * and returning it making two copies we just return the result of readAll().
		 * The file descriptor will be closed in the destructor of QFile */
		return input.readAll();
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

	QString getStringHash(const QString& string)
	{
		QCryptographicHash hash_gen(QCryptographicHash::Md5);
		hash_gen.addData(string.toUtf8());
		return hash_gen.result().toHex();
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
}
