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
#include <QRegularExpression>
#include "schemaparser.h"
#include "legacyschemaparser.h"
#include "attributes.h"
#include "databasemodel.h"
#include <QCryptographicHash>
#include "exception.h"
#include "pgmodelerunittest.h"

/*
 * Differential test: runs the current SchemaParser and a frozen copy of it (LegacySchemaParser, the implementation
 * that existed before the performance work) over the same inputs and requires the very same outcome: the same text or
 * the same error (code, message and position).
 *
 * Inputs: (1) every schema file shipped with pgModeler under several sets of attributes and parser flags;
 * (2) random mutations of those files, which produce mostly invalid programs; (3) random programs built from the
 * grammar of the schema micro language, some of them corrupted on purpose.
 *
 * All randomness comes from a fixed seed so a divergence can be reproduced.
 */
class SchemaParserDiffTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	private:
		struct Outcome {
			bool ok {false};
			QString text, message;
			int code {-1}, line {0}, column {0};

			bool operator == (const Outcome &o) const
			{
				return ok == o.ok && text == o.text && code == o.code &&
							 message == o.message && line == o.line && column == o.column;
			}

			QString toString() const
			{
				if(ok)
					return QString("OK `%1'").arg(QString(text).replace('\n', "\\n"));

				return QString("ERR %1 @%2:%3 `%4'").arg(code).arg(line).arg(column).arg(QString(message).replace('\n', " "));
			}
		};

		//! \brief Small deterministic generator (xorshift) so the corpus does not depend on the Qt version
		struct Rnd {
			quint64 state;

			explicit Rnd(quint64 seed) : state(seed * 2685821657736338717ULL + 88172645463325252ULL) {}

			quint64 next()
			{
				state ^= state << 13;
				state ^= state >> 7;
				state ^= state << 17;
				return state;
			}

			int below(int n) { return n <= 0 ? 0 : static_cast<int>(next() % static_cast<quint64>(n)); }
			bool chance(int percent) { return below(100) < percent; }
		};

		struct Setup {
			bool ignore_unk {false}, ignore_empty {false};
			QString pgsql_ver {"15.0"};
			attribs_map attribs;
		};

		QStringList mismatches;
		int compared {0}, compared_ok {0};

		template<class Parser>
		Outcome run(const QString &file, const QString &buffer, const QString &search_path, const Setup &setup)
		{
			Outcome out;
			Parser parser;
			QString orig_cwd = QDir::currentPath();
			attribs_map attribs = setup.attribs;

			try
			{
				parser.setPgSQLVersion(setup.pgsql_ver, false);
				parser.ignoreUnkownAttributes(setup.ignore_unk);
				parser.ignoreEmptyAttributes(setup.ignore_empty);

				if(!file.isEmpty())
				{
					parser.loadFile(file);
					parser.ignoreUnkownAttributes(setup.ignore_unk);
					parser.ignoreEmptyAttributes(setup.ignore_empty);
					out.text = parser.getSourceCode(file, attribs);
				}
				else
				{
					attribs[Attributes::PgSqlVersion] = setup.pgsql_ver;
					parser.setSearchPath(search_path);
					parser.loadBuffer(buffer);
					out.text = parser.getSourceCode(attribs);
				}

				out.ok = true;
			}
			catch(Exception &e)
			{
				out.code = static_cast<int>(e.getErrorCode());
				out.message = e.getErrorMessage();

				/* The legacy parser kept the position of the error in the instance and the current one restarts itself
				 * when a parsing fails, so the position is not compared: it is part of the message */
			}

			QDir::setCurrent(orig_cwd);
			return out;
		}

		void compare(const QString &origin, const QString &file, const QString &buffer, const QString &search_path, const Setup &setup)
		{
			/* Debug aid: when DIFF_TRACE names a file, the input being compared is written to it before
			 * the parsing, so the input that hangs or crashes a parser can be recovered */
			if(!qEnvironmentVariableIsEmpty("DIFF_TRACE"))
			{
				QFile trace(qEnvironmentVariable("DIFF_TRACE"));

				if(trace.open(QFile::WriteOnly | QFile::Truncate))
					trace.write(QString("%1\n--\n%2\n--\nunk=%3 empty=%4 attribs=%5\n").arg(origin, file.isEmpty() ? buffer : file).arg(setup.ignore_unk).arg(setup.ignore_empty).arg(setup.attribs.size()).toUtf8());
			}

			Outcome cur = run<SchemaParser>(file, buffer, search_path, setup),
					ref = run<LegacySchemaParser>(file, buffer, search_path, setup);

			/* KNOWN DIFFERENCE: the legacy message of InvalidAttribute had its arguments shifted (the attribute
			 * name was missing) and the position can't even be read from it, so only the error code is compared */
			if(!cur.ok && !ref.ok && cur.code == static_cast<int>(ErrorCode::InvalidAttribute) && ref.code == cur.code)
				cur.message = ref.message;

			/* KNOWN DIFFERENCE: the legacy parser reported the lines that come after an @include shifted by the size of the
			 * included file, attributed the first of them to the included file (and so added the "Failed to parse the file..."
			 * prefix, with another column) and named the memory buffer instead of the file in the errors of the include
			 * statements. For the failures of inputs that have includes the prefix, the file names and the positions are masked */
			if(!cur.ok && !ref.ok && hasInclude(file, buffer))
			{
				static const QRegularExpression prefix_regexp("^Failed to parse the file .*? Error detected:\\s*", QRegularExpression::DotMatchesEverythingOption),
						line_regexp("line `?\\d+"), column_regexp("column \\d+"), file_regexp("`[^`']*(\\.sch|\\[memory buffer\\])'");
				cur.line = ref.line;
				cur.column = ref.column;

				for(Outcome *outcome : { &cur, &ref })
				{
					outcome->message.remove(prefix_regexp);
					outcome->message.replace(line_regexp, "line N");
					outcome->message.replace(column_regexp, "column N");
					outcome->message.replace(file_regexp, "`FILE'");
				}
			}

			compared++;
			compared_ok += cur.ok ? 1 : 0;

			if(!(cur == ref) && mismatches.size() < 25)
			{
				mismatches.append(QString("%1 (unk=%2 empty=%3 ver=%4)\n  current: %5\n  legacy : %6\n  input  : %7")
													.arg(origin).arg(setup.ignore_unk).arg(setup.ignore_empty).arg(setup.pgsql_ver)
													.arg(cur.toString(), ref.toString(), file.isEmpty() ? QString(buffer).replace('\n', "\\n") : file));
			}
			else if(!(cur == ref))
				mismatches.append("...");
		}

		bool hasInclude(const QString &file, const QString &buffer)
		{
			static QHash<QString, bool> cache;

			if(file.isEmpty())
				return buffer.contains("@include");

			if(!cache.contains(file))
				cache[file] = readFile(file).contains("@include");

			return cache[file];
		}

		void checkNoMismatches()
		{
			QVERIFY2(mismatches.isEmpty(),
							 qPrintable(QString("%1 divergence(s) in %2 comparisons:\n%3").arg(mismatches.size()).arg(compared).arg(mismatches.join("\n"))));
			QVERIFY(compared > 0);
			qInfo().noquote() << QString("%1 comparisons: %2 parsed fine, %3 raised errors").arg(compared).arg(compared_ok).arg(compared - compared_ok);
		}

		QStringList schemaFiles()
		{
			QStringList files;
			QStringList dirs = { GlobalAttributes::getSchemasRootPath(),
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

		QString readFile(const QString &file)
		{
			QFile f(file);
			return f.open(QFile::ReadOnly) ? QString::fromUtf8(f.readAll()) : QString();
		}

		//! \brief Attributes referenced in the text, found the same way SchemaParser::extractAttributes() does
		QStringList attributeNames(const QString &buffer, const QString &search_path)
		{
			QStringList names;
			SchemaParser parser;

			try
			{
				QString orig_cwd = QDir::currentPath();
				parser.setSearchPath(search_path);
				parser.loadBuffer(buffer);
				names = parser.extractAttributes();
				QDir::setCurrent(orig_cwd);
			}
			catch(Exception &)
			{
			}

			return names;
		}

		Setup makeSetup(const QStringList &names, int config, int flags, const QString &ver, Rnd &rnd)
		{
			static const QStringList pool = { "value", "a&b", "<tag>", "say \"hi\"", "&amp;", "12", "9.5", "0", "abc", "x y", "-", "" };
			Setup setup;
			int idx = 0;

			setup.ignore_unk = flags & 1;
			setup.ignore_empty = flags & 2;
			setup.pgsql_ver = ver;

			for(auto &name : names)
			{
				QString value;

				if(name.isEmpty() || name == Attributes::PgSqlVersion)
					continue;

				switch(config)
				{
					case 0: value = QString("value%1").arg(idx); break;
					case 1: value = (idx % 2) ? QString("value%1").arg(idx) : QString(); break;
					case 2: value = QString(); break;
					case 3: value = pool[idx % (pool.size() - 1)]; break;
					case 4: value = pool[5 + idx % 4]; break;
					case 5:
						// Every third attribute is missing from the map
						if(idx % 3 == 2) { idx++; continue; }
						value = QString("value%1").arg(idx);
					break;
					default: value = pool[rnd.below(pool.size())]; break;
				}

				setup.attribs[name] = value;
				idx++;
			}

			return setup;
		}

		QString mutate(const QString &src, Rnd &rnd)
		{
			static const QStringList snippets = {
				"%if", "%end", "%else", "%then", "%and", "%or", "%not", "{", "}", "[", "]", "$", "\\", "(", ")", "\"", "&", "&{",
				"@", "#", "%set {zz} 1\n", "%unset {zz}\n", "%set @{", "==", "{zz}", "$br", "\\n", "@include \"x\"\n", "\n", " ", "%",
				"%if {zz} %then", "%set {s1}", ">=f \"1\"", "({", "\t"
			};

			QString text = src;
			int ops = 1 + rnd.below(3);

			for(int i = 0; i < ops && !text.isEmpty(); i++)
			{
				int pos = rnd.below(text.size());

				switch(rnd.below(7))
				{
					case 0: text.remove(pos, 1 + rnd.below(3)); break;
					case 1: text.insert(pos, snippets[rnd.below(snippets.size())]); break;
					case 2:
					{
						QStringList lines = text.split('\n');
						int ln = rnd.below(lines.size());
						lines.insert(ln, lines[ln]);
						text = lines.join('\n');
					}
					break;
					case 3:
					{
						QStringList lines = text.split('\n');
						lines.removeAt(rnd.below(lines.size()));
						text = lines.join('\n');
					}
					break;
					case 4:
					{
						QStringList lines = text.split('\n');

						if(lines.size() > 1)
						{
							int ln = rnd.below(lines.size() - 1);
							lines.swapItemsAt(ln, ln + 1);
							text = lines.join('\n');
						}
					}
					break;
					case 5: text.truncate(pos); break;
					default: text.replace(pos, 1, snippets[rnd.below(snippets.size())]); break;
				}
			}

			return text;
		}

		QString genBlock(Rnd &rnd, int depth, const QStringList &names)
		{
			static const QStringList ops = { "==", "!=", "<", ">", "<=", ">=" }, casts = { "", "f", "i" },
			words = { "foo", "bar", "ALTER", "(x)", ",", ";" }, metas = { "$br", "$tb", "$sp", "$ds", "$ob", "$cb", "$oc", "$cc" },
			escs = { "\\n", "\\t", "\\s", "\\#", "\\[", "\\]", "\\*", "\\%" };
			QString out;
			int items = 1 + rnd.below(5);

			auto attr = [&]() { return rnd.chance(8) ? QString("zz") : names[rnd.below(names.size())]; };

			auto expr = [&]() {
				QString e;
				int n = 1 + rnd.below(3);

				for(int i = 0; i < n; i++)
				{
					if(i > 0)
						e += rnd.chance(50) ? " %and " : " %or ";

					if(rnd.chance(25))
						e += "%not ";

					if(rnd.chance(35))
						e += QString("({%1} %2%3 \"%4\")").arg(attr(), ops[rnd.below(ops.size())], casts[rnd.below(casts.size())], rnd.chance(50) ? "10" : "value1");
					else
						e += QString(rnd.chance(15) ? "&{%1}" : "{%1}").arg(attr());
				}

				return e;
			};

			for(int i = 0; i < items; i++)
			{
				switch(rnd.below(depth < 4 ? 11 : 9))
				{
					case 0: out += words[rnd.below(words.size())] + " "; break;
					case 1: out += "[ " + words[rnd.below(words.size())] + " # kept\n x ] "; break;
					case 2: out += QString("{%1} ").arg(attr()); break;
					case 3: out += QString("&{%1} ").arg(attr()); break;
					case 4: out += metas[rnd.below(metas.size())] + " "; break;
					case 5: out += escs[rnd.below(escs.size())] + " "; break;
					case 6: out += QString("\n%set {s%1} {%2} x\n").arg(rnd.below(3)).arg(attr()); break;
					case 7: out += QString("\n%set @{%1} v\n").arg(attr()); break;
					case 8: out += QString("\n%unset {s%1}\n").arg(rnd.below(3)); break;
					default:
						out += "\n%if " + expr() + " %then\n" + genBlock(rnd, depth + 1, names);

						if(rnd.chance(50))
							out += "\n%else\n" + genBlock(rnd, depth + 1, names);

						out += "\n%end\n";
					break;
				}
			}

			return out;
		}

	public:
		SchemaParserDiffTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void init() { mismatches.clear(); compared = compared_ok = 0; }

		void shippedSchemasMatchTheLegacyParser();
		void mutatedSchemasMatchTheLegacyParser();
		void generatedProgramsMatchTheLegacyParser();
		void sampleModelsGenerateTheGoldenCode();
};

void SchemaParserDiffTest::shippedSchemasMatchTheLegacyParser()
{
	QStringList files = schemaFiles();
	QStringList versions = { "9.6", "12.0", "15.0" };
	Rnd rnd(1);

	QVERIFY2(files.size() > 200, qPrintable(QString("Only %1 schema files found").arg(files.size())));

	for(auto &file : files)
	{
		QString dir = QFileInfo(file).absolutePath();
		QStringList names = attributeNames(readFile(file), dir);

		for(int config = 0; config <= 6; config++)
		{
			for(int flags = 0; flags < 4; flags++)
			{
				for(auto &ver : versions)
					compare(QFileInfo(file).fileName() + QString(" config=%1").arg(config), file, QString(), dir,
									makeSetup(names, config, flags, ver, rnd));
			}
		}
	}

	checkNoMismatches();
}

void SchemaParserDiffTest::mutatedSchemasMatchTheLegacyParser()
{
	QStringList files = schemaFiles();
	Rnd rnd(2);

	for(auto &file : files)
	{
		QString dir = QFileInfo(file).absolutePath(), src = readFile(file);
		QStringList names = attributeNames(src, dir);

		for(int i = 0; i < 40; i++)
		{
			QString text = mutate(src, rnd);

			for(int config : { 0, 1, 5 })
			{
				for(int flags : { 0, 1, 3 })
					compare(QFileInfo(file).fileName() + QString(" mutation=%1 config=%2").arg(i).arg(config), QString(), text, dir,
									makeSetup(names, config, flags, "15.0", rnd));
			}
		}
	}

	checkNoMismatches();
}

void SchemaParserDiffTest::generatedProgramsMatchTheLegacyParser()
{
	Rnd rnd(3);
	QStringList names;

	for(int i = 0; i < 8; i++)
		names.append(QString("a%1").arg(i));

	names.append({ "s0", "s1", "s2", "pgsql-ver" });

	for(int i = 0; i < 4000; i++)
	{
		QString text = genBlock(rnd, 0, names);

		if(rnd.chance(30))
			text = mutate(text, rnd);

		for(int flags = 0; flags < 4; flags++)
			compare(QString("program=%1").arg(i), QString(), text, QString(),
							makeSetup(names, rnd.below(7), flags, rnd.chance(50) ? "11.0" : "15.0", rnd));
	}

	checkNoMismatches();
}

/*
 * Golden test: the SQL and XML code generated for every sample model must keep the same SHA-256 recorded in golden.txt
 * (created from the code before the performance work). To refresh it after an intentional change run the test with
 * the environment variable GOLDEN_UPDATE=1.
 */
void SchemaParserDiffTest::sampleModelsGenerateTheGoldenCode()
{
	QDir samples(SAMPLESDIR);
	QStringList models = samples.entryList({ "*.dbm" }, QDir::Files, QDir::Name), lines;
	QString golden_file = QString(GOLDEN_DIR) + "/golden.txt";
	QHash<QString, QString> golden;
	QFile file(golden_file);
	bool update = !qEnvironmentVariableIsEmpty("GOLDEN_UPDATE");

	QVERIFY2(models.size() >= 7, "The sample models were not found");

	if(!update)
	{
		QVERIFY2(file.open(QFile::ReadOnly), qPrintable(QString("Could not open %1").arg(golden_file)));

		for(auto &line : QString::fromUtf8(file.readAll()).split('\n', Qt::SkipEmptyParts))
			golden[line.section(' ', 0, 1)] = line.section(' ', 2);

		file.close();
	}

	for(auto &model : models)
	{
		DatabaseModel dbmodel;
		dbmodel.createSystemObjects(false);
		dbmodel.loadModel(samples.filePath(model));

		for(auto code_type : { SchemaParser::SqlCode, SchemaParser::XmlCode })
		{
			QString code = dbmodel.getSourceCode(code_type),
					canonical = code,
					key = QString("%1 %2").arg(model, code_type == SchemaParser::SqlCode ? "sql" : "xml"),
					hash;

			/* The order of the relationships in the XML code changes from one execution to another (it doesn't depend on the
			 * parser), so the XML is compared by its sorted lines. The SQL code keeps its order. */
			if(code_type == SchemaParser::XmlCode)
			{
				QStringList xml_lines = code.split('\n');
				xml_lines.sort();
				canonical = xml_lines.join('\n');
			}

			hash = QString("%1:%2").arg(QString(QCryptographicHash::hash(canonical.toUtf8(), QCryptographicHash::Sha256).toHex())).arg(code.size());

			lines.append(key + " " + hash);

			if(!update)
			{
				QString dump = QDir::temp().filePath(QString("golden_%1_%2.txt").arg(model, code_type == SchemaParser::SqlCode ? "sql" : "xml"));
				QVERIFY2(golden.contains(key), qPrintable(QString("No golden entry for %1").arg(key)));

				if(golden[key] != hash)
				{
					QFile out(dump);

					if(out.open(QFile::WriteOnly | QFile::Truncate))
						out.write(code.toUtf8());
				}

				QVERIFY2(golden[key] == hash, qPrintable(QString("%1 differs from the golden code (actual output saved in %2)").arg(key, dump)));
			}
		}

		dbmodel.destroyObjects();
	}

	if(update)
	{
		QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
		file.write(lines.join('\n').toUtf8() + "\n");
		file.close();
	}
}

QTEST_MAIN(SchemaParserDiffTest)
#include "schemaparserdifftest.moc"
