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
#include "application.h"
#include "exception.h"
#include <QTranslator>
#include <QFontDatabase>
#include "customuistyle.h"
#include "utilsns.h"

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

#ifdef	Q_OS_MAC
	setAttribute(Qt::AA_DontShowIconsInMenus, false);
#endif
}

void Application::loadTranslation(const QString &lang_id, const QString &directory)
{
	/* The original UI texts are in English (US), so no dictionary is loaded for it even if
	 * there is a file with that name. This is how the user can force the original texts */
	if(lang_id.isEmpty() || lang_id == GlobalAttributes::OriginalUiLanguage ||
		 !QFileInfo::exists(GlobalAttributes::getPath(directory, lang_id + ".qm")))
		return;

	QTranslator *translator = new QTranslator(this);

	/* Tries to load the ui translation according to the system's locale,
	 * and in case of success install it in the application */
	if(!translator->load(lang_id, directory) || !installTranslator(translator))
		delete(translator);
}

void Application::loadCustomFonts(const QString &def_font_attr, const QString &def_size_attr)
{
	QString def_font_fam = GlobalAttributes::getConfigParamFromFile(def_font_attr, GlobalAttributes::AppearanceConf),
			def_font_size = GlobalAttributes::getConfigParamFromFile(def_size_attr, GlobalAttributes::AppearanceConf);

	loadCustomFonts(def_font_fam, def_font_size.toDouble());
}

void Application::loadCustomFonts(const QString &def_font, double def_size)
{
	//Install the custom fonts in .qrc file(s)
	static const QStringList font_names { "exo2", "montserrat", "source-code-pro" };
	static const QString path { QString(":fonts/fonts/%1%2.ttf") };

	for(auto &f_name : font_names)
	{
		QFontDatabase::addApplicationFont(path.arg(f_name, ""));
		QFontDatabase::addApplicationFont(path.arg(f_name, "-bold"));
		QFontDatabase::addApplicationFont(path.arg(f_name, "-italic"));
		QFontDatabase::addApplicationFont(path.arg(f_name, "-bold-italic"));
	}

	// Checking if the provided default font name exists in the font database
	QStringList families = QFontDatabase::families();
	int font_idx = families.indexOf(def_font);

	// If exists we use it as the default font for the whole application
	if(font_idx >= 0)
	{
		// Adjusting the default font size if its invalid
		if(def_size <= 0)
			def_size = font().pointSizeF();

		QFont custom_fnt(def_font, def_size);

		// Workaround on Windows to avoid distorted fonts on some resolutions
		custom_fnt.setHintingPreference(QFont::PreferNoHinting);
		custom_fnt.setStyleStrategy(QFont::PreferAntialias);

		/* We save the custom font in the original-font property
		 * of the application instance so it can be retrieved
		 * when needed, e.g., during on-the-fly UI font preview */
		setProperty(UtilsNs::OrigFontProp, custom_fnt);
		setFont(custom_fnt);
	}
	else
		setProperty(UtilsNs::OrigFontProp, font());
}

QString Application::getUiLanguage()
{
	QString lang_id = GlobalAttributes::getConfigParamFromFile(GlobalAttributes::UiLanguageParam, GlobalAttributes::GeneralConf);

	if(lang_id.isEmpty())
		lang_id = QLocale::system().name();

	return lang_id;
}

void Application::loadTranslations(bool incl_plugins_tr)
{
	loadTranslations(getUiLanguage(), incl_plugins_tr);
}

void Application::loadTranslations(const QString &lang_id, bool incl_plugins_tr)
{
	// Loading the main UI translations
	loadTranslation(lang_id, GlobalAttributes::getLanguagesPath());

	// Trying to load the plugins translations
	if(incl_plugins_tr)
	{
		QStringList dir_list = QDir(GlobalAttributes::getPluginsPath(),
																"*", QDir::Name, QDir::AllDirs | QDir::NoDotAndDotDot).entryList();

		for(auto &plugin_name : dir_list)
		{
			//The dictionaries are in the "lang" subdir at current plugin directory
			loadTranslation(lang_id, GlobalAttributes::getPath(GlobalAttributes::getPluginsPath(), plugin_name, "lang"));
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
