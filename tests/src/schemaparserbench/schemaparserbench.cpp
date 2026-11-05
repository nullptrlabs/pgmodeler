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

#include <QCoreApplication>
#include <QDirIterator>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <vector>
#include "databasemodel.h"
#include "physicaltable.h"
#include "globalattributes.h"
#include "schemaparser.h"
#include "attributes.h"

/*
 * Benchmark and output dump tool for SchemaParser (not a test: it asserts nothing, it only measures).
 *
 * Usage: schemaparserbench [--iterations N] [--repetitions N] [--no-files] [--no-models] [--dump DIR]
 *
 * 1) Schema files: every .sch file shipped with the application is parsed N times with all attributes filled and
 *    with half of them empty. The time reported is the sum of the mean time per file (microseconds).
 * 2) Sample models: each model in the samples folder is loaded and its SQL and XML code is generated N times with
 *    the code cache of all objects invalidated before each generation, otherwise only the cached text is returned.
 *    The best time of the repetitions is reported (milliseconds).
 * 3) --dump DIR writes the generated texts so two builds can be compared byte by byte (the order of the
 *    relationships in the XML code varies between executions, so sort the lines before comparing XML dumps).
 *
 * Without arguments it runs a single iteration of everything, which keeps it harmless when executed by runtests.
 */

static QString readTextFile(const QString &file)
{
	QFile f(file);
	return f.open(QFile::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
}

static void writeTextFile(const QString &file, const QString &contents)
{
	QFile f(file);

	if(f.open(QFile::WriteOnly | QFile::Truncate))
		f.write(contents.toUtf8());
}

static void invalidateCode(DatabaseModel &model)
{
	for(auto type : BaseObject::getObjectTypes(false, { ObjectType::Database }))
	{
		for(auto obj : *model.getObjectList(type))
		{
			obj->setCodeInvalidated(true);
			auto tab = dynamic_cast<PhysicalTable *>(obj);

			if(tab)
			{
				for(auto child : tab->getObjects())
					child->setCodeInvalidated(true);
			}
		}
	}
}

static void benchFiles(int iterations, const QString &dump_dir)
{
	struct Item { QString file; double full_us, half_us; };
	std::vector<Item> items;
	QStringList files, dirs = { GlobalAttributes::getSchemasRootPath(),
															GlobalAttributes::getConfigurationsPath() + GlobalAttributes::DirSeparator + "schemas" };
	QElapsedTimer timer;
	QString orig_cwd = QDir::currentPath();
	double total_full = 0, total_half = 0;
	int errors = 0;

	for(auto &dir : dirs)
	{
		QDirIterator itr(dir, { "*.sch" }, QDir::Files, QDirIterator::Subdirectories);

		while(itr.hasNext())
			files.append(itr.next());
	}

	files.sort();

	for(auto &file : files)
	{
		SchemaParser parser;
		attribs_map full, half;
		Item item { file, 0, 0 };
		int idx = 0;

		try
		{
			parser.loadFile(file);

			for(auto &name : parser.extractAttributes())
			{
				if(name.isEmpty() || name == Attributes::PgSqlVersion)
					continue;

				full[name] = QString("value%1").arg(idx);
				half[name] = (idx % 2) ? QString("value%1").arg(idx) : QString();
				idx++;
			}
		}
		catch(Exception &)
		{
			errors++;
			continue;
		}

		for(int config = 0; config < 2; config++)
		{
			attribs_map &base = config == 0 ? full : half;
			QString text;
			timer.start();

			for(int i = 0; i < iterations; i++)
			{
				try
				{
					attribs_map attribs = base;
					parser.loadFile(file);
					parser.ignoreUnkownAttributes(true);
					parser.ignoreEmptyAttributes(true);
					text = parser.getSourceCode(file, attribs);
				}
				catch(Exception &)
				{
					errors++;
					parser.restartParser();
				}
			}

			(config == 0 ? item.full_us : item.half_us) = timer.nsecsElapsed() / 1000.0 / iterations;

			if(!dump_dir.isEmpty())
				writeTextFile(QString("%1/files/%2.%3").arg(dump_dir, QString(file).replace('/', '_'), config == 0 ? "full" : "half"), text);
		}

		QDir::setCurrent(orig_cwd);
		total_full += item.full_us;
		total_half += item.half_us;
		items.push_back(item);
	}

	std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) { return a.full_us > b.full_us; });

	QTextStream out(stdout);
	out << "== schema files (microseconds per parsing, mean of " << iterations << ") ==\n";

	for(size_t i = 0; i < items.size() && i < 10; i++)
		out << QString("%1 full=%2 half=%3\n").arg(items[i].file.right(60), 60).arg(items[i].full_us, 8, 'f', 1).arg(items[i].half_us, 8, 'f', 1);

	out << QString("TOTAL files=%1 full=%2us half=%3us errors=%4\n")
				 .arg(items.size()).arg(total_full, 0, 'f', 0).arg(total_half, 0, 'f', 0).arg(errors);
}

static void benchModels(int iterations, int repetitions, const QString &dump_dir)
{
	QTextStream out(stdout);
	QDir samples(SAMPLESDIR);
	QElapsedTimer timer;

	out << "== sample models (milliseconds, best of " << repetitions << " generations) ==\n";

	for(auto &model_file : samples.entryList({ "*.dbm" }, QDir::Files, QDir::Name))
	{
		DatabaseModel model;
		double load_ms = 1e18, sql_ms = 1e18, xml_ms = 1e18;
		QString sql, xml;

		model.createSystemObjects(false);

		try
		{
			timer.start();
			model.loadModel(samples.filePath(model_file));
			load_ms = timer.nsecsElapsed() / 1e6;

			for(int rep = 0; rep < repetitions; rep++)
			{
				invalidateCode(model);
				timer.start();
				sql = model.getSourceCode(SchemaParser::SqlCode);
				sql_ms = std::min(sql_ms, timer.nsecsElapsed() / 1e6);

				invalidateCode(model);
				timer.start();
				xml = model.getSourceCode(SchemaParser::XmlCode);
				xml_ms = std::min(xml_ms, timer.nsecsElapsed() / 1e6);
			}

			out << QString("%1 load=%2ms sql=%3ms xml=%4ms (%5 / %6 chars)\n")
						 .arg(model_file, -18).arg(load_ms, 0, 'f', 1).arg(sql_ms, 0, 'f', 1).arg(xml_ms, 0, 'f', 1).arg(sql.size()).arg(xml.size());

			if(!dump_dir.isEmpty())
			{
				writeTextFile(QString("%1/models/%2.sql").arg(dump_dir, model_file), sql);
				writeTextFile(QString("%1/models/%2.xml").arg(dump_dir, model_file), xml);
			}
		}
		catch(Exception &e)
		{
			out << model_file << ": " << e.getErrorMessage() << "\n";
		}

		model.destroyObjects();
	}
}

int main(int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	int iterations = 1, repetitions = 1;
	bool files = true, models = true;
	QString dump_dir;

	for(int i = 1; i < argc; i++)
	{
		QString arg = argv[i];

		if(arg == "--iterations" && i + 1 < argc)
			iterations = std::max(1, QString(argv[++i]).toInt());
		else if(arg == "--repetitions" && i + 1 < argc)
			repetitions = std::max(1, QString(argv[++i]).toInt());
		else if(arg == "--dump" && i + 1 < argc)
			dump_dir = argv[++i];
		else if(arg == "--no-files")
			files = false;
		else if(arg == "--no-models")
			models = false;
	}

	GlobalAttributes::init(SCHEMASDIR, false);

	if(!dump_dir.isEmpty())
	{
		QDir().mkpath(dump_dir + "/files");
		QDir().mkpath(dump_dir + "/models");
	}

	if(files)
		benchFiles(iterations, dump_dir);

	if(models)
		benchModels(iterations, repetitions, dump_dir);

	return 0;
}
