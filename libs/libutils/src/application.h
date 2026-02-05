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
\class Application
\brief This class inherits from QApplication and just sets up a search path in order to initialize GlobalAttribute static class
\note <strong>Creation date:</strong> 30/08/2007
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QDir>
#include "globalattributes.h"

extern void logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

class __libutils Application: public QApplication {
	Q_OBJECT

	private:
		//! \brief A custom message handler to capture and treat messages from qDebug/qInfo/qWarning
		static QtMessageHandler message_handler;

		//! \brief Copy files from a path to another recursively
		void copyFilesRecursively(const QString &src_path, const QString &dst_path, bool missing_only, bool incl_subdirs);

	protected:
		/*! \brief Creates the pgModeler's configuration directory on user's local storage.
		 * This method, in the first run of the tool, copies config files from previous versions.
		 * For next exectutions, the method only recreate missing configuration files that were
		 * envetually removed */
		void createUserConfiguration();

		/*! \brief Loads the translation file by its id (e.g. pt_BR, en_US, etc) in the provided directory.
		 * By default, the files are searched in the pgModeler's default lang files location.
		 * In case of success installs a translator object in the application */
		void loadTranslation(const QString &lang_id, const QString &directory);

	public:
		Application(int & argc, char ** argv);

		//! \brief Loads both UI translations and addition translations provided by plugins (incl_plugins_tr = true)
		void loadTranslations(const QString &lang_id, bool incl_plugins_tr);

	signals:
		void s_messageLogged(QtMsgType, const QMessageLogContext &, const QString &);
};

#if !defined(pgApp)
	/*! \brief A constant similar to qApp to reference a instance of Application
	 *  class to give quick access to the signal s_messageLogged */
	#define pgApp (static_cast<Application *>(Application::instance()))
#endif

#endif
