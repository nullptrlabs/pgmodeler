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
#include <QDirIterator>
#include <functional>
#include "databasemodel.h"
#include "attributes.h"
#include "pgmodelerunittest.h"

class SchemaFilesSyntaxTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		SchemaFilesSyntaxTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private:
		//! \brief Returns every schema file shipped with the application: SQL, XML, alter, data dictionary, catalog and configuration schemas
		QStringList allSchemaFiles();

		/*! \brief Parses all schema files with attributes built by the given function (attribute name, position -> value),
		 * failing on any error that is not expected for the configuration */
		void parseAllSchemas(const std::function<QString(const QString &, int)> &value_for, bool ignore_flags);

	private slots:
		void schemasDontGenerateSyntaxErrors();
		void schemasParseWithAllAttributesFilled();
		void schemasParseWithHalfOfTheAttributesEmpty();
		void schemasParseWithAllAttributesEmpty();
};

void SchemaFilesSyntaxTest::schemasDontGenerateSyntaxErrors()
{
	SchemaParser schparser;
	QString schname, path;
	attribs_map attrs;
	QStringList sch_files,
			sch_folders = { GlobalAttributes::SQLSchemaDir, GlobalAttributes::XMLSchemaDir,
											GlobalAttributes::AlterSchemaDir, GlobalAttributes::DataDictSchemaDir,
											GlobalAttributes::CatalogSchemasDir };
	QDir dir;

	for(auto folder : sch_folders)
	{
		path = GlobalAttributes::getSchemasRootPath() + GlobalAttributes::DirSeparator + folder;
		dir.setCurrent(path);
		sch_files = dir.entryList({ "*.sch" }, QDir::Files | QDir::NoDotAndDotDot);

		try
		{
			for(auto &sch_file : sch_files)
			{
				schparser.ignoreEmptyAttributes(true);
				schparser.ignoreUnkownAttributes(true);
				schparser.getSourceCode(path + GlobalAttributes::DirSeparator + sch_file, attrs);
			}
		}
		catch(Exception &e)
		{
			if(e.getErrorCode() == ErrorCode::InvalidSyntax ||
				 e.getErrorCode() == ErrorCode::InvalidInstruction ||
				 e.getErrorCode() == ErrorCode::InvalidMetacharacter ||
				 e.getErrorCode() == ErrorCode::InvalidAttribute ||
				 e.getErrorCode() == ErrorCode::InvalidOperatorInExpression)
			{
				QFAIL(e.getErrorMessage().toStdString().c_str());
			}

			QTextStream out(stdout);
			out << e.getErrorMessage() << Qt::endl;
		}
	}
}

QStringList SchemaFilesSyntaxTest::allSchemaFiles()
{
	QStringList files, dirs = { GlobalAttributes::getSchemasRootPath(),
															GlobalAttributes::getConfigurationsPath() + GlobalAttributes::DirSeparator + "schemas" };

	for(auto &dir : dirs)
	{
		QDirIterator itr(dir, { "*.sch" }, QDir::Files, QDirIterator::Subdirectories);

		while(itr.hasNext())
			files.append(itr.next());
	}

	files.sort();
	return files;
}

void SchemaFilesSyntaxTest::parseAllSchemas(const std::function<QString(const QString &, int)> &value_for, bool ignore_flags)
{
	QStringList files = allSchemaFiles(), errors;
	QString orig_cwd = QDir::currentPath();

	// Guards against the test silently checking nothing if the schemas are not installed
	QVERIFY2(files.size() > 200, qPrintable(QString("Only %1 schema files were found").arg(files.size())));

	for(auto &file : files)
	{
		SchemaParser schparser;
		attribs_map attrs;
		int idx = 0;

		try
		{
			schparser.loadFile(file);

			for(auto &name : schparser.extractAttributes())
			{
				if(name.isEmpty() || name == Attributes::PgSqlVersion)
					continue;

				attrs[name] = value_for(name, idx++);
			}

			schparser.ignoreUnkownAttributes(ignore_flags);
			schparser.ignoreEmptyAttributes(ignore_flags);
			schparser.getSourceCode(file, attrs);
		}
		catch(Exception &e)
		{
			errors.append(QString("%1: %2").arg(file, e.getErrorMessage()));
			schparser.restartParser();
		}

		QDir::setCurrent(orig_cwd);
	}

	QVERIFY2(errors.isEmpty(), qPrintable(errors.join('\n')));
}

void SchemaFilesSyntaxTest::schemasParseWithAllAttributesFilled()
{
	// With every attribute defined and not empty neither the unknown nor the undefined value errors can happen, even without the ignore flags
	parseAllSchemas([](const QString &name, int idx) { return QString("%1-value%2").arg(name).arg(idx); }, false);
}

void SchemaFilesSyntaxTest::schemasParseWithHalfOfTheAttributesEmpty()
{
	parseAllSchemas([](const QString &name, int idx) { return (idx % 2) ? QString("%1-value").arg(name) : QString(); }, true);
}

void SchemaFilesSyntaxTest::schemasParseWithAllAttributesEmpty()
{
	parseAllSchemas([](const QString &, int) { return QString(); }, true);
}

QTEST_MAIN(SchemaFilesSyntaxTest)
#include "schemafilessyntaxtest.moc"
