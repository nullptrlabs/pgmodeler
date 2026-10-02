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
#include "pgmodelerunittest.h"

// Runs the fix storing the fixed file path in var, skipping the test when the executable can't run
#define FIX_MODEL(var, input, out_name) \
	QString var; \
	if(!fixModel(input, out_name, var)) \
		QSKIP("pgmodeler-cli refused to run due to the license validation")

/*! \brief Exercises the model fix operation (--fix-model) of the pgmodeler-cli executable by running it
as an external process over small model files and inspecting the fixed files it writes */
class CliTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	private:
		QTemporaryDir tmp_dir;

		/*! \brief Runs the fix operation over the input file saving the result in the temporary
		directory. On success the path of the fixed file is stored in output. Returns false when the
		executable can't run due to the license validation (the test must then be skipped) */
		bool fixModel(const QString &input, const QString &out_name, QString &output);

		/*! \brief Names of the objects declared at the root of a model file (excluding relationships, which
		can be generated), as "<tag>:<name>" */
		static QStringList objectNames(const QString &filename);

	public:
		CliTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void objectsAfterIdCommentsAreNotDiscarded();
};

bool CliTest::fixModel(const QString &input, const QString &out_name, QString &output)
{
	output = tmp_dir.filePath(out_name);
	QProcess cli;
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

	/* The executable is run straight from the build tree, so it needs to find the libraries
	 * that were built alongside it */
	#if defined(Q_OS_WIN)
		static const QString LibPathVar { "PATH" };
	#elif defined(Q_OS_MACOS)
		static const QString LibPathVar { "DYLD_LIBRARY_PATH" };
	#else
		static const QString LibPathVar { "LD_LIBRARY_PATH" };
	#endif

	env.insert(LibPathVar, QString(CLI_LIBS_PATH) + QDir::listSeparator() + env.value(LibPathVar));
	env.insert("QT_QPA_PLATFORM", "offscreen");
	env.insert(GlobalAttributes::EnvTmpPath, tmp_dir.path());

	cli.setProcessEnvironment(env);
	cli.start(CLI_BIN, { "--fix-model", "--input", input, "--output", output });

	if(!cli.waitForFinished(120000))
		qFatal("The pgmodeler-cli process did not finish: %s", qPrintable(cli.errorString()));

	QString out = QString::fromUtf8(cli.readAllStandardOutput() + cli.readAllStandardError());

	// Builds with the license check can't run the CLI unless the license was validated by the user
	if(out.contains("license", Qt::CaseInsensitive) && !QFile::exists(output))
		return false;

	if(cli.exitCode() != 0 || !QFile::exists(output))
		qFatal("The model fix failed:\n%s", qPrintable(out));

	return true;
}

QStringList CliTest::objectNames(const QString &filename)
{
	QStringList names;
	QFile file(filename);

	if(!file.open(QIODevice::ReadOnly))
		return names;

	QRegularExpressionMatchIterator itr = QRegularExpression("^<(\\w+) name=\"([^\"]*)\"",
																													 QRegularExpression::MultilineOption).globalMatch(QString::fromUtf8(file.readAll()));

	while(itr.hasNext())
	{
		QRegularExpressionMatch match = itr.next();

		if(match.captured(1) != "relationship")
			names.append(match.captured(1) + ":" + match.captured(2));
	}

	names.sort();
	return names;
}

void CliTest::objectsAfterIdCommentsAreNotDiscarded()
{
	// Each object of a split model's file starts with an <!-- ID --> line, which joined in a single model look like this
	QFile file(QString("%1/northwind.dbm").arg(SAMPLESDIR));
	QStringList lines;
	int id = 1;

	QVERIFY(file.open(QIODevice::ReadOnly));
	lines = QString::fromUtf8(file.readAll()).split('\n');
	file.close();

	// Inserting the comment before each root level opening tag that follows the <dbmodel> tag
	QStringList result;
	bool in_dbmodel_tag = false, past_dbmodel_tag = false;
	static QRegularExpression root_tag_rx("^<[a-z]+[ >]");

	for(auto &line : lines)
	{
		if(past_dbmodel_tag && root_tag_rx.match(line).hasMatch())
			result.append(QString("<!-- %1 -->").arg(id++));

		result.append(line);

		// The <dbmodel> tag spans a few lines and ends in the first one that closes it
		if(!past_dbmodel_tag)
		{
			in_dbmodel_tag = in_dbmodel_tag || line.startsWith("<dbmodel");
			past_dbmodel_tag = in_dbmodel_tag && line.trimmed().endsWith(">");
		}
	}

	QVERIFY2(id > 10, "the model must have enough objects to make the test meaningful");

	QString with_ids = tmp_dir.filePath("northwind_ids.dbm");
	QFile out(with_ids);

	QVERIFY(out.open(QIODevice::WriteOnly));
	out.write(result.join('\n').toUtf8());
	out.close();

	FIX_MODEL(fixed_plain, QString("%1/northwind.dbm").arg(SAMPLESDIR), "northwind_plain_fixed.dbm");
	FIX_MODEL(fixed_ids, with_ids, "northwind_ids_fixed.dbm");

	QStringList expected = objectNames(fixed_plain);

	QVERIFY(expected.size() > 10);
	QCOMPARE(objectNames(fixed_ids), expected);
}

QTEST_MAIN(CliTest)
#include "clitest.moc"
