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
#include "application.h"
#include "exception.h"
#include <QTranslator>
#include "customuistyle.h"

void logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString message = qFormatLogMessage(type, context, msg);

	if(type == QtInfoMsg || type == QtDebugMsg)
	{
		QTextStream out(stdout);
		out << message << Qt::endl;
	}
	else
	{
		QTextStream err(stderr);
		err << message << Qt::endl;
	}

	if(pgApp)
		emit pgApp->s_messageLogged(type, context, msg);
}

QtMessageHandler Application::message_handler = qInstallMessageHandler(logMessage);

Application::Application(int &argc, char **argv) : QApplication(argc,argv)
{
	/* Checking if the user specified another widget style using the -style param
	 * If no custom style is specified we force the usage of Fusion (the default for Qt and pgModeler)
	 * via CustomUiStyle so additional UI effects can be applied */
	if(!arguments().contains(GlobalAttributes::UiStyleOption))
		setStyle(new CustomUiStyle(GlobalAttributes::DefaultQtStyle));
}

void Application::loadTranslation(const QString &lang_id, const QString &directory)
{
	if(lang_id.isEmpty() || !QFileInfo::exists(directory + GlobalAttributes::DirSeparator + lang_id + ".qm"))
		return;

	QTranslator *translator = new QTranslator(this);

	/* Tries to load the ui translation according to the system's locale,
	 * and in case of success install it in the application */
	if(!translator->load(lang_id, directory) || !installTranslator(translator))
		delete(translator);
}

void Application::loadTranslations(const QString &lang_id, bool incl_plugins_tr)
{
	// Loading the main UI translations
	loadTranslation(lang_id, GlobalAttributes::getLanguagesPath());

	// Trying to load the plugins translations
	if(incl_plugins_tr)
	{
		QStringList dir_list = QDir(GlobalAttributes::getPluginsPath() +
																GlobalAttributes::DirSeparator,
																"*", QDir::Name, QDir::AllDirs | QDir::NoDotAndDotDot).entryList();

		QString plugin_name, plugin_lang_dir;

		while(!dir_list.isEmpty())
		{
			plugin_name = dir_list.front();
			dir_list.pop_front();

			//Configure the path to "lang" subdir at current plugin directory
			plugin_lang_dir = GlobalAttributes::getPluginsPath() +
											GlobalAttributes::DirSeparator + plugin_name +
											GlobalAttributes::DirSeparator + "lang" +
											GlobalAttributes::DirSeparator;

			loadTranslation(lang_id, plugin_lang_dir);
		}
	}
}

void Application::createUserConfiguration()
{
	QDir new_cfg_dir(GlobalAttributes::getConfigurationsPath()),
			old_cfg_dir(GlobalAttributes::getConfigurationsPath().replace(GlobalAttributes::PgModelerAppName,
																																		GlobalAttributes::PgModelerOldAppName));

	bool first_run = !new_cfg_dir.exists();

	/* First, we check if there are config files of previous version (GlobalAttributes::PgModelerOldAppName)
	 * in the user's local storage. If that's the case, we copy the files to the folder reserved for the
	 * new version (GlobalAttributes::PgModelerAppName) */
	if(first_run && old_cfg_dir.exists() && !new_cfg_dir.exists())
	{
		new_cfg_dir.mkpath(new_cfg_dir.absolutePath());
		copyFilesRecursively(old_cfg_dir.absolutePath(),
												 new_cfg_dir.absolutePath(), false, true);
	}

	try
	{
		copyFilesRecursively(GlobalAttributes::getTmplConfigurationPath(),
												 GlobalAttributes::getConfigurationsPath(),
												 !first_run, false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(ErrorCode::InitialUserConfigNotCreated)
										.arg(GlobalAttributes::getConfigurationsPath(),
												 GlobalAttributes::getTmplConfigurationPath()),
										ErrorCode::InitialUserConfigNotCreated, PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void Application::copyFilesRecursively(const QString &src_path, const QString &dst_path, bool missing_only, bool incl_subdirs)
{
	QFileInfo src_file(src_path);

	if(!src_file.exists())
	{
		throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotAccessed).arg(src_path),
										PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(src_file.isDir())
	{
		QDir dst_dir(dst_path), src_dir(src_path);

		if(!dst_dir.exists() && !dst_dir.mkpath(dst_path))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(dst_path),
											 PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		QString new_src_path, new_dst_path;
		QStringList filenames;
		QFileInfo fi;
		QDir::Filters filter = QDir::Files | QDir::NoDotAndDotDot;

		static const QStringList ignored_dirs = {// Subdirs in user's local storage
																							"backups", "tmp",
																							// Subdirs in template config files directory
																							"defaults", "themes", "schemas" };

		if(incl_subdirs)
			filter |= QDir::Dirs;

		// Retrieve all directories and files in the old path
		filenames = src_dir.entryList({ "*" }, filter);

		for(auto &filename : filenames)
		{
			new_src_path = src_path + src_dir.separator() + filename;
			new_dst_path = dst_path + dst_dir.separator() + filename;
			fi.setFile(new_src_path);

			// Ignoring the files icons-*.conf, ui-*.conf, *-highlight.conf, appearance.conf
			if((filename.startsWith("icons-") ||
					filename.startsWith(GlobalAttributes::UiStyleConf)) ||

					/* Ignore the file if it exists in the destination and we are creating
					 * the missing files */
					(missing_only && QFileInfo::exists(new_dst_path)) ||

					// Ignoring the config file backup dir and temp models dir
					(incl_subdirs && fi.isDir() && ignored_dirs.contains(filename)) ||

					// Ignoring files that are not config file
					(fi.isFile() && !new_src_path.endsWith(GlobalAttributes::ConfigurationExt)))
				continue;

			copyFilesRecursively(new_src_path, new_dst_path, missing_only, incl_subdirs);
		}
	}
	else
	{
		bool file_exists = QFileInfo::exists(dst_path),
				file_copied = false;

		/* Forcing the removal of files that are not backward compatible
		 * if they already exists in the destination folder */
		if(dst_path.contains(GlobalAttributes::HighlightFileSuffix) ||
			 dst_path.contains(GlobalAttributes::DiffPresetsConf) ||
			 dst_path.contains(GlobalAttributes::HighlightFileSuffix) ||
			 dst_path.contains(GlobalAttributes::AppearanceConf))
			QFile::remove(dst_path);

		file_copied = QFile::copy(src_path, dst_path);

		if(!file_exists && !file_copied)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(dst_path),
											PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		if(file_exists || file_copied)
		{
			// Set write permissions when copying file with read-only permissions
			QFile file(dst_path);

			if(!file.permissions().testFlag(QFile::WriteOwner))
				file.setPermissions(file.permissions() | QFileDevice::WriteOwner);
		}
	}
}
