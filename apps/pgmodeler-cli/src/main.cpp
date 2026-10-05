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

#include <QTranslator>
#include "pgmodelercliapp.h"
#include "utilsns.h"
#include <signal.h>

namespace {
	void dumpStackTrace(int signal)
	{
		qDebug().noquote() << "**";
		qDebug().noquote() << UtilsNs::generateStackTrace(signal);
		qDebug().noquote() << "**";
		exit(1);
	}
}

int main(int argc, char **argv)
{
	//Install a signal handler to dump stacktrace when SIGSEGV or SIGABRT is emitted
	signal(SIGSEGV, dumpStackTrace);
	signal(SIGABRT, dumpStackTrace);

	QTextStream out(stdout);

	try
	{
		#ifdef Q_OS_LINUX
			/* Workaround to make the CLI work on Linux systems without graphical interface.
			 * In that case, we just check if there's a DISPLAY env var defined. If not defined
			 * we force the usage of the "offscreen" plugin via QT_QPA_PLATFORM so the portions of
			 * the application that contains GUI elements can work properly.
			 *
			 * Details at https://github.com/pgmodeler/pgmodeler/issues/1604 */
			if(qgetenv("DISPLAY").isEmpty())
				qputenv("QT_QPA_PLATFORM", "offscreen");
		#endif

		GlobalAttributes::init(argv[0], false);
		PgModelerCliApp pgmodeler_cli(argc, argv);
		pgmodeler_cli.loadTranslations(true);

		//Executes the cli
		return pgmodeler_cli.exec();
	}
	catch(Exception &e)
	{
		out << Qt::endl;
		out << e.getExceptionsText();
		out << UtilsNs::pgmTr(PGM_TR2("Global", "** pgmodeler-cli aborted due to critical error(s). **")) << Qt::endl << Qt::endl;
		return (e.getErrorCode()==ErrorCode::Custom ? -1 : enum_t(e.getErrorCode()));
	}
}
