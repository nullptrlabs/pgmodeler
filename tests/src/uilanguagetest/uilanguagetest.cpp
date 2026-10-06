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

#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QProcess>
#include <QLibraryInfo>
#include <QComboBox>
#include "application.h"
#include "settings/generalconfigwidget.h"
#include "attributes.h"
#include "pgmodelerunittest.h"

/*! \brief English (US) is the language of the original UI texts. It must always be offered in the
UI language list of the general settings (it has no dictionary) and selecting it must make the
application keep the original texts even when the system language has a dictionary. */
class UiLanguageTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		UiLanguageTest() : PgModelerUnitTest(SCHEMASDIR){}

		//! \brief Folder used as the one of the dictionaries (PGMODELER_LANG_PATH)
		static QTemporaryDir lang_dir;

		//! \brief Folder used as the one of the plugins (PGMODELER_PLUGINS_PATH). It has the plugin "plugin_a" with the dictionary fr_FR
		static QTemporaryDir plugins_dir;

		//! \brief Folder used as the one of the configuration files (PGMODELER_CONF_PATH)
		static QTemporaryDir conf_dir;

		//! \brief Indicates if the dictionaries in lang_dir are real (they could be compiled)
		static bool real_dictionaries;

		/*! \brief Creates the folder of dictionaries: en_US.qm, pt_BR.qm and de_DE.qm. When lrelease is available
		 * the dictionaries are real ones that translate "Hello" (context UiLanguageTest) to "Hello (<id>)".
		 * Otherwise they are empty files, enough to check the list of languages but not the loading. */
		static void prepareDictionaries();

	private slots:
		void generalSettingsAlwaysListsEnglishUsRightAfterSystemDefault();
		void englishUsIsListedOnlyOnceEvenWithItsOwnDictionaryFile();
		void otherDictionariesAreListedAfterEnglishUs();
		void selectingEnglishUsKeepsTheOriginalTextsEvenWithADictionary();
		void otherLanguagesStillLoadTheirDictionaries();
		void uiLanguageParameterIsTheOneOfTheConfigFiles();
		void uiLanguageIsTheOneConfiguredInTheMainApp();
		void uiLanguageIsTheSystemOneWhenNoneIsConfigured();
		void uiLanguageCanBeEnglishUs();
		void auxiliaryExecutablesLoadTheLanguageConfiguredInTheMainApp();
		void pluginsDictionariesAreLoadedOnlyWhenRequested();
};

QTemporaryDir UiLanguageTest::lang_dir;
QTemporaryDir UiLanguageTest::conf_dir;
QTemporaryDir UiLanguageTest::plugins_dir;
bool UiLanguageTest::real_dictionaries = false;

void UiLanguageTest::prepareDictionaries()
{
	const QStringList ids { "en_US", "pt_BR", "de_DE" };
	QString lrelease = QLibraryInfo::path(QLibraryInfo::BinariesPath) + "/lrelease";

	for(auto &id : ids)
	{
		QString qm_file = lang_dir.filePath(id + ".qm"),
				ts_file = lang_dir.filePath(id + ".ts");

		QFile ts(ts_file);

		if(QFileInfo::exists(lrelease) && ts.open(QFile::WriteOnly))
		{
			ts.write(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE TS><TS version=\"2.1\" language=\"%1\">"
											 "<context><name>UiLanguageTest</name><message><source>Hello</source>"
											 "<translation>Hello (%1)</translation></message></context></TS>").arg(id).toUtf8());
			ts.close();

			QProcess lrel;
			lrel.start(lrelease, { ts_file, "-qm", qm_file });
			lrel.waitForFinished(30000);
			real_dictionaries = (lrel.exitCode() == 0 && QFileInfo::exists(qm_file));
		}

		if(!QFileInfo::exists(qm_file))
		{
			QFile empty(qm_file);
			empty.open(QFile::WriteOnly);
			real_dictionaries = false;
		}
	}

	// The dictionary of the plugin: plugins_dir/plugin_a/lang/fr_FR.qm
	QDir().mkpath(plugins_dir.filePath("plugin_a/lang"));

	QString plugin_ts = plugins_dir.filePath("plugin_a/lang/fr_FR.ts");
	QFile ts(plugin_ts);

	if(real_dictionaries && ts.open(QFile::WriteOnly))
	{
		ts.write("<?xml version=\"1.0\" encoding=\"utf-8\"?><!DOCTYPE TS><TS version=\"2.1\" language=\"fr_FR\">"
						 "<context><name>UiLanguageTestPlugin</name><message><source>Hi</source>"
						 "<translation>Hi (plugin_a)</translation></message></context></TS>");
		ts.close();

		QProcess lrel;
		lrel.start(lrelease, { plugin_ts, "-qm", plugins_dir.filePath("plugin_a/lang/fr_FR.qm") });
		lrel.waitForFinished(30000);
	}
}

/*! \brief Writes the general configuration file as the main GUI executable does, with the
UI language specified. An empty id removes the parameter's value, a null one removes the file. */
static void configureUiLanguage(const QString &lang_id)
{
	QString filename = GlobalAttributes::getConfigurationFilePath(GlobalAttributes::GeneralConf);

	QFile::remove(filename);

	if(lang_id.isNull())
		return;

	QFile file(filename);

	QVERIFY(file.open(QFile::WriteOnly));
	file.write(QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<configuration %1=\"%2\" other-param=\"x\"/>\n")
						 .arg(GlobalAttributes::UiLanguageParam, lang_id).toUtf8());
}

void UiLanguageTest::generalSettingsAlwaysListsEnglishUsRightAfterSystemDefault()
{
	GeneralConfigWidget conf_wgt;
	QComboBox *cmb = conf_wgt.ui_language_cmb;

	QVERIFY(cmb->count() >= 2);
	QVERIFY2(cmb->itemData(0).toString().isEmpty(), "the first item must be the system default");
	QCOMPARE(cmb->itemData(1).toString(), GlobalAttributes::OriginalUiLanguage);
	QCOMPARE(GlobalAttributes::OriginalUiLanguage, QString("en_US"));
	QVERIFY2(cmb->itemText(1).contains("en_US"), qPrintable(cmb->itemText(1)));
}

void UiLanguageTest::englishUsIsListedOnlyOnceEvenWithItsOwnDictionaryFile()
{
	GeneralConfigWidget conf_wgt;
	int cnt = 0;

	QVERIFY(QFileInfo::exists(lang_dir.filePath("en_US.qm")));

	for(int i = 0; i < conf_wgt.ui_language_cmb->count(); i++)
	{
		if(conf_wgt.ui_language_cmb->itemData(i).toString() == "en_US")
			cnt++;
	}

	QCOMPARE(cnt, 1);
}

void UiLanguageTest::otherDictionariesAreListedAfterEnglishUs()
{
	GeneralConfigWidget conf_wgt;
	QStringList ids;

	for(int i = 0; i < conf_wgt.ui_language_cmb->count(); i++)
		ids.append(conf_wgt.ui_language_cmb->itemData(i).toString());

	QCOMPARE(ids, QStringList({ "", "en_US", "de_DE", "pt_BR" }));
}

void UiLanguageTest::selectingEnglishUsKeepsTheOriginalTextsEvenWithADictionary()
{
	if(!real_dictionaries)
		QSKIP("lrelease is not available, the dictionaries can't be compiled.");

	Application *app = pgApp;

	app->loadTranslations(GlobalAttributes::OriginalUiLanguage, false);

	QCOMPARE(QCoreApplication::translate("UiLanguageTest", "Hello"), QString("Hello"));
}

/*! \brief Positive control of the previous test: the same dictionary folder, with another language, is loaded. */
void UiLanguageTest::otherLanguagesStillLoadTheirDictionaries()
{
	if(!real_dictionaries)
		QSKIP("lrelease is not available, the dictionaries can't be compiled.");

	pgApp->loadTranslations("pt_BR", false);

	QCOMPARE(QCoreApplication::translate("UiLanguageTest", "Hello"), QString("Hello (pt_BR)"));
}

void UiLanguageTest::uiLanguageParameterIsTheOneOfTheConfigFiles()
{
	//The parameter name is repeated in libutils, where Attributes is not available, so they must not diverge
	QCOMPARE(GlobalAttributes::UiLanguageParam, Attributes::UiLanguage);
}

void UiLanguageTest::uiLanguageIsTheOneConfiguredInTheMainApp()
{
	configureUiLanguage("pt_BR");
	QCOMPARE(Application::getUiLanguage(), QString("pt_BR"));

	configureUiLanguage("de_DE");
	QCOMPARE(Application::getUiLanguage(), QString("de_DE"));
}

void UiLanguageTest::uiLanguageIsTheSystemOneWhenNoneIsConfigured()
{
	//No configuration file at all
	configureUiLanguage(QString());
	QCOMPARE(Application::getUiLanguage(), QLocale::system().name());

	//Configuration file with the language left blank ("System default" in the settings)
	configureUiLanguage("");
	QCOMPARE(Application::getUiLanguage(), QLocale::system().name());
}

void UiLanguageTest::uiLanguageCanBeEnglishUs()
{
	configureUiLanguage(GlobalAttributes::OriginalUiLanguage);
	QCOMPARE(Application::getUiLanguage(), QString("en_US"));
}

/*! \brief The executables other than the main one call loadTranslations(bool) without a language:
they must get the dictionary of the language configured in the main one, whatever the system's language is. */
void UiLanguageTest::auxiliaryExecutablesLoadTheLanguageConfiguredInTheMainApp()
{
	if(!real_dictionaries)
		QSKIP("lrelease is not available, the dictionaries can't be compiled.");

	configureUiLanguage("de_DE");
	pgApp->loadTranslations(false);

	QCOMPARE(QCoreApplication::translate("UiLanguageTest", "Hello"), QString("Hello (de_DE)"));
}

/*! \brief The dictionaries in the "lang" folder of each plugin are loaded only when the plugins
translations are requested. */
void UiLanguageTest::pluginsDictionariesAreLoadedOnlyWhenRequested()
{
	if(!real_dictionaries || !QFileInfo::exists(plugins_dir.filePath("plugin_a/lang/fr_FR.qm")))
		QSKIP("lrelease is not available, the dictionaries can't be compiled.");

	// fr_FR has no dictionary in the main folder, only in the plugin
	QVERIFY(!QFileInfo::exists(lang_dir.filePath("fr_FR.qm")));

	pgApp->loadTranslations("fr_FR", false);
	QCOMPARE(QCoreApplication::translate("UiLanguageTestPlugin", "Hi"), QString("Hi"));

	pgApp->loadTranslations("fr_FR", true);
	QCOMPARE(QCoreApplication::translate("UiLanguageTestPlugin", "Hi"), QString("Hi (plugin_a)"));
}

int main(int argc, char *argv[])
{
	/* The folders of dictionaries, plugins and configuration files must be defined before the unit test
	 * class is created, because it's where the global attributes get the paths from */
	if(!UiLanguageTest::lang_dir.isValid() || !UiLanguageTest::conf_dir.isValid() ||
		 !UiLanguageTest::plugins_dir.isValid())
		return 1;

	UiLanguageTest::prepareDictionaries();
	qputenv(GlobalAttributes::EnvLangPath.toUtf8(), UiLanguageTest::lang_dir.path().toUtf8());
	qputenv(GlobalAttributes::EnvConfPath.toUtf8(), UiLanguageTest::conf_dir.path().toUtf8());
	qputenv(GlobalAttributes::EnvPluginsPath.toUtf8(), UiLanguageTest::plugins_dir.path().toUtf8());

	Application app(argc, argv);
	UiLanguageTest test;

	return QTest::qExec(&test, argc, argv);
}

#include "uilanguagetest.moc"
