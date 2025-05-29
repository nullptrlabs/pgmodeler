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

#include <QtTest/QtTest>
#include "databasemodel.h"
#include "pgmodelerunittest.h"

class SchemaFilesSyntaxTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		SchemaFilesSyntaxTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void schemasDontGenerateSyntaxErrors();
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
				schparser.ignoreUnknownAttributes(true);
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

QTEST_MAIN(SchemaFilesSyntaxTest)
#include "schemafilessyntaxtest.moc"
