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

		//! \brief Attributes of the <dbmodel> tag of a model file
		static QMap<QString, QString> headerAttributes(const QString &filename);

		//! \brief Rectangles of the <scenes> element of a model file, as the text of each <scene> tag
		static QStringList sceneRects(const QString &filename);

		//! \brief Scene index (the scene attribute) of each table, view, foreign table and textbox, by object name
		static QMap<QString, QString> objectScenes(const QString &filename);

		/*! \brief Names of the objects declared at the root of a model file (excluding relationships, which
		can be generated), as "<tag>:<name>" */
		static QStringList objectNames(const QString &filename);

		static QString fixtureFile(const QString &name);

	public:
		CliTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void fixedModelKeepsSchemaSegmentation();
		void fixedModelKeepsThresholdSegmentation();
		void fixedModelKeepsOtherHeaderAttributes();
		void fixedModelKeepsTheHideRelationshipsOption();
		void fixingModelWithoutSegmentationKeepsItUnsegmented();
		void objectsAfterIdCommentsAreNotDiscarded();
};

QString CliTest::fixtureFile(const QString &name)
{
	return QString("%1/fixtures/%2").arg(CLI_TEST_DIR, name);
}

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

QMap<QString, QString> CliTest::headerAttributes(const QString &filename)
{
	QMap<QString, QString> attrs;
	QFile file(filename);

	if(!file.open(QIODevice::ReadOnly))
		return attrs;

	QString xml = QString::fromUtf8(file.readAll());
	QRegularExpressionMatch tag = QRegularExpression("<dbmodel(.*?)>", QRegularExpression::DotMatchesEverythingOption).match(xml);
	QRegularExpressionMatchIterator itr = QRegularExpression("([\\w-]+)=\"([^\"]*)\"").globalMatch(tag.captured(1));

	while(itr.hasNext())
	{
		QRegularExpressionMatch attr = itr.next();
		attrs[attr.captured(1)] = attr.captured(2);
	}

	return attrs;
}

QStringList CliTest::sceneRects(const QString &filename)
{
	QStringList rects;
	QFile file(filename);

	if(!file.open(QIODevice::ReadOnly))
		return rects;

	QRegularExpressionMatchIterator itr = QRegularExpression("<scene [^>]*>").globalMatch(QString::fromUtf8(file.readAll()));

	while(itr.hasNext())
		rects.append(itr.next().captured(0));

	return rects;
}

QMap<QString, QString> CliTest::objectScenes(const QString &filename)
{
	QMap<QString, QString> scenes;
	QFile file(filename);

	if(!file.open(QIODevice::ReadOnly))
		return scenes;

	QRegularExpressionMatchIterator itr = QRegularExpression("^<(table|view|foreigntable|textbox) ([^>]*)>",
																													 QRegularExpression::MultilineOption).globalMatch(QString::fromUtf8(file.readAll()));

	while(itr.hasNext())
	{
		QString attrs = itr.next().captured(2);
		QRegularExpressionMatch name = QRegularExpression("name=\"([^\"]*)\"").match(attrs),
				scene = QRegularExpression("(?<![\\w-])scene=\"([^\"]*)\"").match(attrs);

		scenes[name.captured(1)] = scene.captured(1);
	}

	return scenes;
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

void CliTest::fixedModelKeepsSchemaSegmentation()
{
	QString input = fixtureFile("segmented_by_schema.dbm");
	FIX_MODEL(fixed, input, "schema_fixed.dbm");
	QMap<QString, QString> in_attrs = headerAttributes(input),
			out_attrs = headerAttributes(fixed);

	QCOMPARE(in_attrs["seg-mode"], QString("1"));

	for(auto &attr : { "seg-mode", "seg-threshold", "last-scene", "scene-rect" })
		QCOMPARE(out_attrs[attr], in_attrs[attr]);

	QVERIFY(!sceneRects(input).isEmpty());
	QCOMPARE(sceneRects(fixed), sceneRects(input));
	QCOMPARE(objectScenes(fixed), objectScenes(input));
}

void CliTest::fixedModelKeepsThresholdSegmentation()
{
	QString input = fixtureFile("segmented_by_threshold.dbm");
	FIX_MODEL(fixed, input, "threshold_fixed.dbm");
	QMap<QString, QString> in_attrs = headerAttributes(input),
			out_attrs = headerAttributes(fixed);

	QCOMPARE(in_attrs["seg-mode"], QString("2"));

	// seg-obj-count isn't copied, it's recomputed from the recreated objects, which must match the input's
	for(auto &attr : { "seg-mode", "seg-threshold", "seg-obj-count", "last-scene", "scene-rect" })
		QCOMPARE(out_attrs[attr], in_attrs[attr]);

	QCOMPARE(sceneRects(fixed), sceneRects(input));
	QCOMPARE(objectScenes(fixed), objectScenes(input));
}

void CliTest::fixedModelKeepsOtherHeaderAttributes()
{
	QString input = fixtureFile("segmented_by_schema.dbm");
	FIX_MODEL(fixed, input, "header_fixed.dbm");
	QMap<QString, QString> in_attrs = headerAttributes(input),
			out_attrs = headerAttributes(fixed);

	// The fixture uses values different from the model's defaults for all of these
	QCOMPARE(in_attrs["author"], QString("Test &amp; Co"));

	for(auto &attr : { "author", "default-schema", "default-owner", "last-position", "last-zoom",
										 "show-sys-schemas-rects", "gen-disabled-objs-code", "layer-name-colors",
										 "layer-rect-colors", "show-layer-names", "show-layer-rects", "layers", "active-layers" })
	{
		QVERIFY2(out_attrs.contains(attr), attr);
		QCOMPARE(out_attrs[attr], in_attrs[attr]);
	}
}

void CliTest::fixedModelKeepsTheHideRelationshipsOption()
{
	// The option is only written to the file when enabled, so a model that doesn't use it must stay without the attribute
	QString plain_input = fixtureFile("segmented_by_schema.dbm");
	FIX_MODEL(plain_fixed, plain_input, "plain_fixed.dbm");

	QVERIFY(!headerAttributes(plain_fixed).contains("hide-rels"));

	QFile file(plain_input);
	QVERIFY(file.open(QIODevice::ReadOnly));

	QString xml = QString::fromUtf8(file.readAll());
	file.close();

	QVERIFY(xml.contains(" seg-mode=\"1\""));
	xml.replace(" seg-mode=\"1\"", " seg-mode=\"1\" hide-rels=\"true\"");

	QString hide_input = tmp_dir.filePath("hide_rels.dbm");
	QFile out_file(hide_input);

	QVERIFY(out_file.open(QIODevice::WriteOnly));
	out_file.write(xml.toUtf8());
	out_file.close();

	FIX_MODEL(hide_fixed, hide_input, "hide_fixed.dbm");

	QCOMPARE(headerAttributes(hide_fixed)["hide-rels"], QString("true"));
}

void CliTest::fixingModelWithoutSegmentationKeepsItUnsegmented()
{
	// A model created before the segmentation modes existed
	FIX_MODEL(fixed, QString("%1/demo.dbm").arg(SAMPLESDIR), "demo_fixed.dbm");
	QMap<QString, QString> attrs = headerAttributes(fixed);

	QCOMPARE(attrs["seg-mode"], QString("0"));
	QVERIFY(sceneRects(fixed).isEmpty());

	for(auto &scene : objectScenes(fixed))
		QVERIFY(scene.isEmpty());
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
