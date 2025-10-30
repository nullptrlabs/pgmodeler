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

#include "pgmodelerapp.h"
#include "mainwindow.h"
#include <signal.h>
#include <QSplashScreen>

#ifndef Q_OS_WIN
	#include "execinfo.h"
#endif

namespace {
	void startCrashHandler(int signal)
	{
		QFile output;
		QString lin, cmd;

		/** At the moment the backtrace function does not exists on MingW (Windows) this way
		 the code that generates the stacktrace is available only on Linux/Unix systems */
	#ifndef Q_OS_WIN
		void *stack[30];
		size_t stack_size;
		char **symbols=nullptr;
		stack_size = backtrace(stack, 30);
		symbols = backtrace_symbols(stack, stack_size);
	#endif

		cmd=QString("\"%1\"").arg(GlobalAttributes::getPgModelerCHandlerPath()) + " -style " + GlobalAttributes::DefaultQtStyle;

		//Creates the stacktrace file
		output.setFileName(GlobalAttributes::getTemporaryFilePath(GlobalAttributes::StacktraceFile));
		output.open(QFile::WriteOnly);

		if(output.isOpen())
		{
			lin=QString("** pgModeler crashed after receive signal: %1 **\n\nDate/Time: %2 \nVersion: %3 \nBuild: %4 \n")
				.arg(signal)
				.arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
				.arg(GlobalAttributes::PgModelerVersion)
				.arg(GlobalAttributes::PgModelerBuildNumber);

			lin+=QString("Compilation Qt version: %1\nRunning Qt version: %2\n\n")
				 .arg(QT_VERSION_STR)
				 .arg(qVersion());

			output.write(lin.toStdString().c_str(), lin.size());

	#ifndef Q_OS_WIN
			for(size_t i=0; i < stack_size; i++)
			{
				lin=QString("[%1] ").arg(stack_size-1-i) + QString(symbols[i]) + "\n";
				output.write(lin.toStdString().c_str(), lin.size());
			}
			free(symbols);
	#else
			lin = "** Stack trace unavailable on Windows system **";
			output.write(lin.toStdString().c_str(), lin.size());
	#endif

			output.close();
		}

		/* Changing the working dir to the main executable in order to call the crash handler
		if the PGMODELER_CHANDLER_PATH isn't set */
		QDir dir;
		dir.cd(QApplication::applicationDirPath());

		exit(1 + system(cmd.toStdString().c_str()));
	}
}

int main(int argc, char **argv)
{
	try
	{		
		//Install a signal handler to start crashhandler when SIGSEGV or SIGABRT is emitted
		signal(SIGSEGV, startCrashHandler);
		signal(SIGABRT, startCrashHandler);

		GlobalAttributes::init(argv[0], true);
		PgModelerApp app(argc,argv);
		int res=0;

		// Loading the application splash screen
		QSplashScreen splash;
		QPixmap pix(":images/images/pgmodeler_splash.png");

		if(qApp->primaryScreen()->devicePixelRatio() > 1)
			pix.setDevicePixelRatio(qApp->primaryScreen()->devicePixelRatio());
		else
			pix = pix.scaledToWidth(320, Qt::SmoothTransformation);

		splash.setPixmap(pix);
		splash.show();
		splash.raise();
		app.processEvents();

		//Creates the main form
		MainWindow fmain;

		// Displaying the splash for one second after displaying the main window
		QTimer::singleShot(1000, &splash, [&splash, &fmain]() {
			fmain.show();
			splash.finish(&fmain);
		 });

		//Loading models via command line on MacOSX are disabled until the file association work correclty on that system
		#ifndef Q_OS_MACOS
			QStringList params=app.arguments();
			params.pop_front();

			//If the user specifies a list of files to be loaded
			if(!params.isEmpty())
				fmain.loadModels(params);
		#endif

		res = app.exec();
		app.closeAllWindows();

		return res;
	}
	catch(Exception &e)
	{
		QTextStream ts(stdout);
		ts << e.getExceptionsText();
		return enum_t(e.getErrorCode());
	}
}
