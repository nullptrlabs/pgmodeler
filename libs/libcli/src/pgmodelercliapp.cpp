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

#include "pgmodelercliapp.h"
#include "modelwidget.h"
#include "utilsns.h"
#include "settings/appearanceconfigwidget.h"
#include "tableview.h"
#include "graphicalview.h"
#include "schemaview.h"
#include "styledtextboxview.h"
#include "relationshipview.h"
#include "pgsqlversions.h"
#include "compat/compatns.h"
#include <QSettings>
#include <QPluginLoader>

QTextStream PgModelerCliApp::out {stdout};

const QRegularExpression PgModelerCliApp::PasswordRegExp { "(password)(=)(.)*( )" };
const QString PgModelerCliApp::PasswordPlaceholder { "password=******" };

const QString PgModelerCliApp::AllChildren {"all"};
const QString PgModelerCliApp::Input {"--input"};
const QString PgModelerCliApp::Output {"--output"};
const QString PgModelerCliApp::InputDb {"--input-db"};
const QString PgModelerCliApp::ExportToFile {"--export-to-file"};
const QString PgModelerCliApp::ExportToPng {"--export-to-png"};
const QString PgModelerCliApp::ExportToSvg {"--export-to-svg"};
const QString PgModelerCliApp::ExportToDbms {"--export-to-dbms"};
const QString PgModelerCliApp::ExportToDict {"--export-to-dict"};
const QString PgModelerCliApp::ImportDb {"--import-db"};
const QString PgModelerCliApp::NoIndex {"--no-index"};
const QString PgModelerCliApp::Split {"--split"};
const QString PgModelerCliApp::Markdown {"--markdown"};
const QString PgModelerCliApp::DependenciesSql {"--dependencies"};
const QString PgModelerCliApp::ChildrenSql {"--children"};
const QString PgModelerCliApp::GroupByType {"--group-by-type"};
const QString PgModelerCliApp::GenDropScript {"--gen-drop-script"};
const QString PgModelerCliApp::Diff {"--diff"};
const QString PgModelerCliApp::DropDatabase {"--drop-database"};
const QString PgModelerCliApp::DropObjects {"--drop-objects"};
const QString PgModelerCliApp::NonTransactional {"--non-transactional"};
const QString PgModelerCliApp::PgSqlVer {"--pgsql-ver"};
const QString PgModelerCliApp::Help {"--help"};
const QString PgModelerCliApp::ShowGrid {"--show-grid"};
const QString PgModelerCliApp::ShowDelimiters {"--show-delimiters"};
const QString PgModelerCliApp::PageByPage {"--page-by-page"};
const QString PgModelerCliApp::OverrideBgColor {"--override-bg-color"};
const QString PgModelerCliApp::IgnoreDuplicates {"--ignore-duplicates"};
const QString PgModelerCliApp::IgnoreErrorCodes {"--ignore-error-codes"};
const QString PgModelerCliApp::ConnAlias {"--conn-alias"};
const QString PgModelerCliApp::Host {"--host"};
const QString PgModelerCliApp::Port {"--port"};
const QString PgModelerCliApp::User {"--user"};
const QString PgModelerCliApp::Passwd {"--passwd"};
const QString PgModelerCliApp::InitialDb {"--initial-db"};
const QString PgModelerCliApp::Silent {"--silent"};
const QString PgModelerCliApp::ListConns {"--list-conns"};
const QString PgModelerCliApp::Simulate {"--simulate"};
const QString PgModelerCliApp::FixModel {"--fix-model"};
const QString PgModelerCliApp::FixTries {"--fix-tries"};
const QString PgModelerCliApp::ZoomFactor {"--zoom"};
const QString PgModelerCliApp::UseTmpNames {"--use-tmp-names"};
const QString PgModelerCliApp::DbmMimeType {"--dbm-mime-type"};
const QString PgModelerCliApp::Install {"install"};
const QString PgModelerCliApp::Uninstall {"uninstall"};
const QString PgModelerCliApp::SystemWide {"--system-wide"};
const QString PgModelerCliApp::IgnoreImportErrors {"--ignore-errors"};
const QString PgModelerCliApp::ImportSystemObjs {"--import-sys-objs"};
const QString PgModelerCliApp::ImportExtensionObjs {"--import-ext-objs"};
const QString PgModelerCliApp::DebugMode {"--debug-mode"};
const QString PgModelerCliApp::FilterObjects {"--filter-objects"};
const QString PgModelerCliApp::MatchByName {"--match-by-name"};
const QString PgModelerCliApp::ForceChildren {"--force-children"};
const QString PgModelerCliApp::OnlyMatching {"--only-matching"};
const QString PgModelerCliApp::CommentsAsAliases {"--comments-as-aliases"};
const QString PgModelerCliApp::PartialDiff {"--partial"};
const QString PgModelerCliApp::Force {"--force"};
const QString PgModelerCliApp::StartDate {"--start-date"};
const QString PgModelerCliApp::EndDate {"--end-date"};
const QString PgModelerCliApp::CompareDb {"--compare-db"};
const QString PgModelerCliApp::CompareFile {"--compare-file"};
const QString PgModelerCliApp::SaveDiff {"--save"};
const QString PgModelerCliApp::ApplyDiff {"--apply"};
const QString PgModelerCliApp::NoDiffPreview {"--no-preview"};
const QString PgModelerCliApp::DropClusterObjs {"--drop-cluster-objs"};
const QString PgModelerCliApp::RevokePermissions {"--revoke-perms"};
const QString PgModelerCliApp::DropMissingObjs {"--drop-missing"};
const QString PgModelerCliApp::ForceDropColsConstrs {"--force-drop-cols"};
const QString PgModelerCliApp::RenameDb {"--rename-db"};
const QString PgModelerCliApp::NoSequenceReuse {"--no-sequence-reuse"};
const QString PgModelerCliApp::NoCascadeDrop {"--no-cascade"};
const QString PgModelerCliApp::RecreateUnmod {"--recreate-unmod"};
const QString PgModelerCliApp::ReplaceModified {"--replace-mod"};
const QString PgModelerCliApp::ForceReCreateObjs { "--force-re-create" };
const QString PgModelerCliApp::CreateConfigs {"--create-configs"};
const QString PgModelerCliApp::MissingOnly {"--missing-only"};
const QString PgModelerCliApp::IgnoreFaultyPlugins {"--ignore-faulty"};
const QString PgModelerCliApp::ListPlugins {"--list-plugins"};

const QString PgModelerCliApp::ConnOptions {"connopts"};
const QString PgModelerCliApp::TagExpr {"<%1"};
const QString PgModelerCliApp::EndTagExpr {"</%1"};
const QString PgModelerCliApp::AttributeExpr {"(%1)( )*(=)(\")(\\w|\\d|,|\\.|\\&|\\;|\\)|\\(|\\-| )+(\")"};

const QString PgModelerCliApp::ModelFixLog {"model_fix.log"};

const QString PgModelerCliApp::MsgFileAssociated {
	QString(QT_TR_NOOP("Database model files (*%1) are already associated with pgModeler! Use the option `%2' to force the file association installation.")).arg(GlobalAttributes::DbModelExt, Force)
};

const QString PgModelerCliApp::MsgNoFileAssociation {
	QString(QT_TR_NOOP("No file association exists between pgModeler and *%1 files! Use the option `%2' to force the file association uninstallation.")).arg(GlobalAttributes::DbModelExt, Force)
};

std::map<QString, bool> PgModelerCliApp::long_opts {
	{ Input, true }, { Output, true }, { InputDb, true },
	{ ExportToFile, false },	{ ExportToPng, false },	{ ExportToSvg, false },
	{ ExportToDbms, false },	{ ImportDb, false },	{ Diff, false },
	{ DropDatabase, false },	{ DropObjects, false },	{ PgSqlVer, true },
	{ Help, false },	{ ShowGrid, false },	{ ShowDelimiters, false },
	{ PageByPage, false },	{ IgnoreDuplicates, false },	{ OverrideBgColor, false },
	{ IgnoreErrorCodes, true }, { ConnAlias, true },	{ Host, true },	{ Port, true },
	{ User, true },	{ Passwd, true },	{ InitialDb, true },
	{ ListConns, false },	{ Simulate, false },	{ FixModel, false },
	{ FixTries, true },	{ ZoomFactor, true },	{ UseTmpNames, false },
	{ DbmMimeType, true },	{ IgnoreImportErrors, false },	{ ImportSystemObjs, false },
	{ ImportExtensionObjs, false },	{ FilterObjects, true },	{ ForceChildren, true },
	{ OnlyMatching, false },	{ MatchByName, false },	{ DebugMode, false },
	{ PartialDiff, false },	{ StartDate, true },	{ EndDate, true },
	{ CompareDb, true }, { CompareFile, true },	{ SaveDiff, false },	{ ApplyDiff, false },
	{ NoDiffPreview, false },	{ DropClusterObjs, false },	{ RevokePermissions, false },
	{ DropMissingObjs, false },	{ ForceDropColsConstrs, false },	{ RenameDb, false },
	{ NoSequenceReuse, false },	{ NoCascadeDrop, false },
	{ RecreateUnmod, false }, { ReplaceModified, false },	{ ForceReCreateObjs, true },
	{ ExportToDict, false }, { NoIndex, false },	{ Split, false },	{ SystemWide, false },
	{ CreateConfigs, false }, { Force, false }, { MissingOnly, false },
	{ DependenciesSql, false }, { ChildrenSql, false }, { GenDropScript, false },
	{ GroupByType, false }, { CommentsAsAliases, false }, { IgnoreFaultyPlugins, false },
	{ ListPlugins, false }, { Markdown, false }, { NonTransactional, false }
};

attribs_map PgModelerCliApp::short_opts {
	{ Input, "-if" },		{ Output, "-of" },	{ InputDb, "-id" },
	{ ExportToFile, "-ef" },	{ ExportToPng, "-ep" },	{ ExportToSvg, "-es" },
	{ ExportToDbms, "-ed" },	{ ExportToDict, "-ec" },	{ ImportDb, "-im" },
	{ Diff, "-df" },	{ DropDatabase, "-dd" },	{ DropObjects, "-do" },
	{ PgSqlVer, "-v" },	{ Help, "-h" },	{ ShowGrid, "-sg" },
	{ ShowDelimiters, "-sl" },	{ PageByPage, "-pp" },
	{ IgnoreDuplicates, "-ir" }, { OverrideBgColor, "-oc" },
	{ IgnoreErrorCodes, "-ic" },	{ ConnAlias, "-ca" },	{ Host, "-H" },
	{ Port, "-p" },	{ User, "-u" },	{ Passwd, "-w" },
	{ InitialDb, "-D" },	{ Silent, "-s" },	{ ListConns, "-lc" },
	{ Simulate, "-sm" },	{ FixModel, "-fm" },	{ FixTries, "-ft" },
	{ ZoomFactor, "-zf" },	{ UseTmpNames, "-tn" },	{ DbmMimeType, "-mt" },
	{ IgnoreImportErrors, "-ie" },	{ ImportSystemObjs, "-is" },	{ ImportExtensionObjs, "-ix" },
	{ FilterObjects, "-fo" },	{ MatchByName, "-mn" },	{ ForceChildren, "-fc" },
	{ OnlyMatching, "-om" },	{ DebugMode, "-d" },	{ PartialDiff, "-pd" },
	{ StartDate, "-st" },	{ EndDate, "-et" },	{ CompareDb, "-cd" }, { CompareFile, "-cf" },
	{ SaveDiff, "-sd" },	{ ApplyDiff, "-ad" },	{ NoDiffPreview, "-np" },
	{ DropClusterObjs, "-dc" },	{ RevokePermissions, "-rv" },	{ DropMissingObjs, "-dm" },
	{ ForceDropColsConstrs, "-fd" },	{ RenameDb, "-rn" },
	{ NoSequenceReuse, "-ns" },	{ NoCascadeDrop, "-nd" }, { RecreateUnmod, "-ru" },
	{ ReplaceModified, "-rm" },	{ ForceReCreateObjs, "-fr" }, { NoIndex, "-ni" },	{ Split, "-sp" },
	{ SystemWide, "-sw" },	{ CreateConfigs, "-cc" }, { Force, "-ff" },
	{ MissingOnly, "-mo" }, { DependenciesSql, "-ds" }, { ChildrenSql, "-cs" },
	{ GroupByType, "-gt" },	{ GenDropScript, "-gd" }, { CommentsAsAliases, "-cl" },
	{ IgnoreFaultyPlugins, "-ip" }, { ListPlugins, "-lp" }, { Markdown, "-md" },
	{ NonTransactional, "-nt" }
};

std::map<QString, QStringList> PgModelerCliApp::accepted_opts {
	{{ ConnOptions }, { ConnAlias, Host, Port, User, Passwd, InitialDb }},
	{{ ExportToFile }, { Input, Output, PgSqlVer, Split, DependenciesSql, ChildrenSql, GroupByType, GenDropScript }},
	{{ ExportToPng },  { Input, Output, ShowGrid, ShowDelimiters, PageByPage, ZoomFactor, OverrideBgColor }},
	{{ ExportToSvg },  { Input, Output, ShowGrid, ShowDelimiters }},
	{{ ExportToDict }, { Input, Output, Split, NoIndex, Markdown }},

	{{ ExportToDbms }, { Input, PgSqlVer, IgnoreDuplicates, IgnoreErrorCodes,
												DropDatabase, DropObjects, Simulate, UseTmpNames, Force,
												NonTransactional }},

	{{ ImportDb }, { InputDb, Output, IgnoreImportErrors, ImportSystemObjs, ImportExtensionObjs,
										FilterObjects, OnlyMatching, MatchByName, ForceChildren, DebugMode, ConnAlias,
										Host, Port, User, Passwd, InitialDb, CommentsAsAliases }},

	{{ Diff }, { Input, PgSqlVer, IgnoreDuplicates, IgnoreErrorCodes, CompareDb, CompareFile,
							 PartialDiff, Force, StartDate, EndDate, SaveDiff, ApplyDiff, NoDiffPreview,
							 DropClusterObjs, RevokePermissions, DropMissingObjs, ForceDropColsConstrs,
							 RenameDb, NoCascadeDrop, NoSequenceReuse, RecreateUnmod, ReplaceModified,
							 ForceReCreateObjs, NonTransactional }},

	{{ DbmMimeType }, { SystemWide, Force }},
	{{ FixModel },	{ Input, Output, FixTries }},
	{{ ListConns }, { }},
	{{ CreateConfigs }, { MissingOnly, Force }},
	{{ ListPlugins }, { IgnoreFaultyPlugins }}
};

PgModelerCliApp::PgModelerCliApp(int argc, char **argv) : Application(argc, argv)
{
	try
	{
		QString op, value, orig_op;
		bool accepts_val = false;
		attribs_map opts;
		QStringList args = arguments();

		has_fix_log = false;
		buffer_size = 0;
		input_model = nullptr;
		scene = nullptr;
		xmlparser = nullptr;
		zoom = 1;

		fix_model = upd_mime = import_db = false;
		diff = create_configs = list_conns = false;
		list_plugins = plugin_op = false;
		export_op = false;

		export_hlp = nullptr;
		import_hlp = nullptr;
		diff_hlp = nullptr;
		conn_conf = nullptr;
		rel_conf = nullptr;
		general_conf = nullptr;

		loadPlugins();

		// We extract the options values only if the help option is not present
		if(args.size() > 1 && !args.contains(Help) && !args.contains(short_opts[Help]))
		{	
			for(int i=1; i < argc; i++)
			{
				op = orig_op = argv[i];

				//If the retrieved option starts with - it will be treated as a command option
				if(op[0]=='-')
				{
					value.clear();

					if(i < argc-1 && argv[i+1][0]!='-')
					{
						//If the next option does not starts with '-', is considered a value
						value=argv[++i];
					}

					//Raises an error if the option is not recognized
					if(!isOptionRecognized(op, accepts_val))
						throw Exception(tr("Unrecognized option `%1'.").arg(orig_op), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

					//Raises an error if the value is empty and the option accepts a value
					if(accepts_val && value.isEmpty())
						throw Exception(tr("No value specified for option `%1'.").arg(orig_op), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

					if(!accepts_val && !value.isEmpty())
						throw Exception(tr("Option `%1' does not accept values.").arg(orig_op), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

					/* If we find a filter object parameter we append its parameter index so
					 * its value is not replaced by the next filter parameter found */
					if(op == FilterObjects)
						opts[QString("%1%2").arg(op).arg(i)] = value;

					opts[op] = value;
				}
			}
		}

		//Validates and executes the options
		parseOptions(opts);

		if(!parsed_opts.empty())
		{
			input_model=new DatabaseModel;
			xmlparser=input_model->getXMLParser();
			silent_mode=(parsed_opts.count(Silent));

			//If the export is to png or svg loads additional configurations
			if(parsed_opts.count(ExportToPng) || parsed_opts.count(ExportToSvg) || parsed_opts.count(ImportDb))
			{
				connect(input_model, &DatabaseModel::s_objectAdded, this, &PgModelerCliApp::handleObjectAddition);
				connect(input_model, &DatabaseModel::s_objectRemoved, this, &PgModelerCliApp::handleObjectRemoval);

				//Load the appearance settings including grid and delimiter options
				AppearanceConfigWidget appearance_wgt;
				appearance_wgt.loadConfiguration();

				scene=new ObjectsScene;
				scene->setParent(this);
			}

			if(parsed_opts.count(ExportToDbms) || parsed_opts.count(ImportDb) || parsed_opts.count(Diff))
			{
				configureConnection(false);

				//Replacing the initial db parameter for the input database when reverse engineering
				if((parsed_opts.count(ImportDb) || parsed_opts.count(Diff)) && !parsed_opts[InputDb].isEmpty())
					connection.setConnectionParam(Connection::ParamDbName, parsed_opts[InputDb]);
			}

			if(parsed_opts.count(Diff) && parsed_opts.count(CompareDb))
			{
				configureConnection(true);

				if(!extra_connection.isConfigured())
					extra_connection = connection;

				extra_connection.setConnectionParam(Connection::ParamDbName, parsed_opts[CompareDb]);
			}

			if(!silent_mode && export_hlp && import_hlp && diff_hlp)
			{
				connect(export_hlp, &ModelExportHelper::s_progressUpdated, this, &PgModelerCliApp::updateProgress);
				connect(export_hlp, &ModelExportHelper::s_errorIgnored, this,  &PgModelerCliApp::printIgnoredError);
				connect(import_hlp, &DatabaseImportHelper::s_progressUpdated, this, &PgModelerCliApp::updateProgress);
				connect(diff_hlp, &ModelsDiffHelper::s_progressUpdated, this, &PgModelerCliApp::updateProgress);
			}
		}
	}
	catch(Exception &e)
	{
		throw e;
	}
}

PgModelerCliApp::~PgModelerCliApp()
{
	bool show_flush_msg = input_model && input_model->getObjectCount() > 0;

	if(show_flush_msg)
		printMessage(tr("Flushing used memory..."));

	delete scene;
	delete input_model;
	delete export_hlp;
	delete import_hlp;
	delete diff_hlp;
	delete conn_conf;
	delete rel_conf;
	delete general_conf;

	while(!plugins.empty())
	{
		delete plugins.back();
		plugins.pop_back();
	}

	if(show_flush_msg)
		printMessage(tr("Done!"));
}

void PgModelerCliApp::printText(const QString &txt)
{
	out << txt << Qt::endl;
}

void PgModelerCliApp::printMessage(const QString &txt)
{
	if(!silent_mode)
		printText(txt);
}

attribs_map PgModelerCliApp::getParsedOptions()
{
	return parsed_opts;
}

QString PgModelerCliApp::getParsedOptValue(const QString &opt)
{
	if(parsed_opts.count(opt) == 0)
	{
		throw Exception(tr("Attempting to retrieve the value of an unknown parsed option `%1'!").arg(opt),
										ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	return parsed_opts[opt];
}

void PgModelerCliApp::setParsedOptValue(const QString &opt, const QString &value)
{
	if(parsed_opts.count(opt) == 0)
	{
		throw Exception(tr("Attempting to set the value of an unknown parsed option `%1'!").arg(opt),
										 ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	parsed_opts[opt] = value;
}

void PgModelerCliApp::configureConnection(bool extra_conn)
{
	QString chr = (extra_conn ? "1" : "");
	Connection *conn = (extra_conn ? &extra_connection : &connection);

	//Getting the connection using its alias
	if(parsed_opts.count(ConnAlias + chr))
	{
		if(!connections.count(parsed_opts[ConnAlias + chr]))
			throw Exception(tr("Connection aliased as '%1' was not found in the configuration file.").arg(parsed_opts[ConnAlias + chr]),
							ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		//Make a copy of the named connection
		*conn = (*connections[parsed_opts[ConnAlias + chr]]);
	}
	else
	{
		conn->setConnectionParam(Connection::ParamServerFqdn, parsed_opts[Host + chr]);
		conn->setConnectionParam(Connection::ParamUser, parsed_opts[User + chr]);
		conn->setConnectionParam(Connection::ParamPort, parsed_opts[Port + chr]);
		conn->setConnectionParam(Connection::ParamPassword, parsed_opts[Passwd + chr]);
		conn->setConnectionParam(Connection::ParamDbName, parsed_opts[InitialDb + chr]);
	}
}

bool PgModelerCliApp::isOptionRecognized(QString &op, bool &accepts_val)
{
	bool found = false, append_chr = false;

	if(op.endsWith('1'))
	{
		op.chop(1);
		append_chr = true;
	}

	for(auto &itr : short_opts)
	{
		found = (op == itr.first || op == itr.second);
		accepts_val = (found && long_opts[itr.first]);

		if(found)
		{
			op = itr.first;
			break;
		}
	}

	if(append_chr)
		op += '1';

	return found;
}

void PgModelerCliApp::showVersionInfo()
{
	printMessage();
	printMessage(tr("pgModeler %1 command line interface.").arg(
							 #ifdef PRIVATE_PLUGINS_SYMBOLS
								"Plus"
							#else
								 ""
							#endif
							));
	printMessage(tr("Version ") + GlobalAttributes::PgModelerVersion + QString(" - %1 Qt %2").arg(GlobalAttributes::PgModelerBuildNumber, QT_VERSION_STR));
	printMessage(tr("PostgreSQL Database Modeler Project - pgmodeler.io") );
	printMessage(tr("Copyright 2006-%1 Raphael Araújo e Silva <raphael@pgmodeler.io>").arg(QDate::currentDate().year()));
	printMessage();
}

void PgModelerCliApp::showMenu()
{
	showVersionInfo();

	printText(tr("Usage: pgmodeler-cli [OPTIONS]"));
	printText(tr("This CLI tool provides operations with database models and databases without requiring the use of pgModeler's graphical interface. All available options are described below."));
	printText();

	printText(tr("Operation mode options: "));
	printText(tr(" %1, %2\t\t  Exports the input model to SQL script file(s).").arg(short_opts[ExportToFile], ExportToFile));
	printText(tr(" %1, %2\t\t  Exports the input model to a PNG image.").arg(short_opts[ExportToPng], ExportToPng));
	printText(tr(" %1, %2\t\t  Exports the input model to a SVG file.").arg(short_opts[ExportToSvg], ExportToSvg));
	printText(tr(" %1, %2\t\t  Exports the input model to a data dictionary in HTML format.").arg(short_opts[ExportToDict], ExportToDict));
	printText(tr(" %1, %2\t\t  Exports the input model directly to a PostgreSQL server.").arg(short_opts[ExportToDbms], ExportToDbms));
	printText(tr(" %1, %2\t\t  Lists the available connections in file %3.").arg(short_opts[ListConns], ListConns, GlobalAttributes::ConnectionsConf + GlobalAttributes::ConfigurationExt));
	printText(tr(" %1, %2\t\t  Imports a database to an output file.").arg(short_opts[ImportDb], ImportDb));
	printText(tr(" %1, %2\t\t\t  Compares a model and a database or two databases, generating an SQL script to sync the latter with the former.").arg(short_opts[Diff], Diff));
	printText(tr(" %1, %2\t\t  Tries to fix the structure of the input model file to make it loadable again.").arg(short_opts[FixModel], FixModel));
	printText(tr(" %1, %2\t\t  Creates pgModeler's configuration folder and files in the user's local storage.").arg(short_opts[CreateConfigs], CreateConfigs));

	#ifndef Q_OS_MACOS
		printText(tr(" %1, %2 [ACTION]\t  Handles the DBM file association to pgModeler binaries. The ACTION can be [%3 | %4].").arg(short_opts[DbmMimeType], DbmMimeType, Install, Uninstall));
	#endif

	printText(tr(" %1, %2\t\t\t  Shows this help menu.").arg(short_opts[Help], Help));
	printText();

	printText(tr("General options: "));
	printText(tr(" %1, %2 [FILE]\t\t  Input model file (%3). Required for export and model fix operations.").arg(short_opts[Input], Input, GlobalAttributes::DbModelExt));
	printText(tr(" %1, %2 [DBNAME]\t  Input database name. Required for import operation.").arg(short_opts[InputDb], InputDb));
	printText(tr(" %1, %2 [FILE|DIRECTORY]   Output file or directory. Required for model fix operations or exporting to SQL, HTML, PNG, or SVG formats.").arg(short_opts[Output], Output));
	printText(tr(" %1, %2\t\t  Forces PostgreSQL syntax to the specified version when generating SQL code. The version string must be in the format [major].[minor], e.g., %3.").arg(short_opts[PgSqlVer], PgSqlVer, PgSqlVersions::DefaulVersion));
	printText(tr(" %1, %2\t\t\t  Silent execution. Only critical messages and errors are displayed during the process.").arg(short_opts[Silent], Silent));
	printText();

	printText(tr("SQL file export options: "));
	printText(tr(" %1, %2\t\t\t  The database model SQL code is split into several files, one per object.").arg(short_opts[Split], Split));
	printText(tr(" %1, %2\t\t  Includes the object's dependencies SQL code in the generated file. (Only for split mode)").arg(short_opts[DependenciesSql], DependenciesSql));
	printText(tr(" %1, %2\t\t  Includes the object's children SQL code in the generated file. (Only for split mode)").arg(short_opts[ChildrenSql], ChildrenSql));
	printText(tr(" %1, %2\t\t  Instead of creating a separate SQL file per object, groups the SQL code of all objects of the same type in a single file. (Only for split mode)").arg(short_opts[GroupByType], GroupByType));
	printText(tr(" %1, %2\t\t  Creates a separate script containing DROP commands to destroy database objects.").arg(short_opts[GenDropScript], GenDropScript));
	printText();

	printText(tr("PNG and SVG export options: "));
	printText(tr(" %1, %2\t\t  Draws the grid in the exported image.").arg(short_opts[ShowGrid], ShowGrid));
	printText(tr(" %1, %2\t\t  Draws the page delimiters in the exported image.").arg(short_opts[ShowDelimiters], ShowDelimiters));
	printText(tr(" %1, %2\t\t  Each page will be exported in a separate image. (Only for PNG images)").arg(short_opts[PageByPage], PageByPage));
	printText(tr(" %1, %2\t  Overrides the original canvas color in the exported image, using a white background instead. (PNG images only)").arg(short_opts[OverrideBgColor], OverrideBgColor));
	printText(tr(" %1, %2 [FACTOR]\t\t  Applies zoom (in percent) before exporting to an image. Accepted zoom range: %3 to %4 (PNG images only)").arg(short_opts[ZoomFactor], ZoomFactor).arg(ModelWidget::MinimumZoom).arg(ModelWidget::MaximumZoom));
	printText();

	printText(tr("Data dictionary export options: "));
	printText(tr(" %1, %2\t\t\t  The data dictionaries are generated in separate files inside the specified output directory.").arg(short_opts[Split], Split));
	printText(tr(" %1, %2\t\t  Avoids generating the index that is used to help navigate through the data dictionary.").arg(short_opts[NoIndex], NoIndex));
	printText(tr(" %1, %2\t\t  Generates a data dictionary in Markdown format (.md) instead of HTML format.").arg(short_opts[Markdown], Markdown));
	printText();

	printText(tr("DBMS export options: "));
	printText(tr(" %1, %2\t  Ignores errors related to duplicate objects that may exist on the server.").arg(short_opts[IgnoreDuplicates], IgnoreDuplicates));
	printText(tr(" %1, %2 [LIST] Ignores additional errors by their error codes. Provide a comma-separated list of alphanumeric codes.").arg(short_opts[IgnoreErrorCodes], IgnoreErrorCodes));
	printText(tr(" %1, %2\t\t  Drops the database before executing the export process.").arg(short_opts[DropDatabase], DropDatabase));
	printText(tr(" %1, %2 \t\t\t  Forces termination of all connections to the target database before dropping it. This option is ignored when exporting to PostgreSQL 12 or earlier.").arg(short_opts[Force], Force));
	printText(tr(" %1, %2\t\t  Executes DROP commands attached to objects with enabled SQL code.").arg(short_opts[DropObjects], DropObjects));
	printText(tr(" %1, %2\t\t  Simulates the export process by executing all steps but undoing any modifications at the end.").arg(short_opts[Simulate], Simulate));
	printText(tr(" %1, %2\t\t  Generates temporary names for databases, roles, and tablespaces when in simulation mode.").arg(short_opts[UseTmpNames], UseTmpNames));
	printText(tr(" %1, %2\t  Executes the export process in non-transactional mode where changes are not rolled back in case of errors.").arg(short_opts[NonTransactional], NonTransactional));
	printText();

	printText(tr("Connection options: "));
	printText(tr(" %1, %2 [ALIAS]\t  Connection configuration alias to be used.").arg(short_opts[ConnAlias], ConnAlias));
	printText(tr(" %1, %2 [HOST]\t\t  PostgreSQL host on which the task will operate.").arg(short_opts[Host], Host));
	printText(tr(" %1, %2 [PORT]\t\t  PostgreSQL host listening port.").arg(short_opts[Port], Port));
	printText(tr(" %1, %2 [USER]\t\t  PostgreSQL username.").arg(short_opts[User], User));
	printText(tr(" %1, %2 [PASSWORD]\t  PostgreSQL user password.").arg(short_opts[Passwd], Passwd));
	printText(tr(" %1, %2 [DBNAME]\t  Connection's initial database.").arg(short_opts[InitialDb], InitialDb));
	printText();

	printText(tr("Database import options: "));
	printText(tr(" %1, %2\t\t  Ignores all errors and tries to create as many objects as possible.").arg(short_opts[IgnoreImportErrors], IgnoreImportErrors));
	printText(tr(" %1, %2\t\t  Imports built-in system objects. This option may increase model size due to importing unnecessary objects.").arg(short_opts[ImportSystemObjs], ImportSystemObjs));
	printText(tr(" %1, %2\t\t  Imports extension objects. This option may increase model size due to importing unnecessary objects.").arg(short_opts[ImportExtensionObjs], ImportExtensionObjs));
	printText(tr(" %1, %2\t  Uses objects' comments as aliases. This option affects objects graphically represented in the database model.").arg(short_opts[CommentsAsAliases], CommentsAsAliases));
	printText(tr(" %1, %2 [FILTER]   Imports only objects matching the filter(s). The FILTER must be formatted as type:pattern:mode.").arg(short_opts[FilterObjects], FilterObjects));
	printText(tr(" %1, %2\t\t  Imports only objects matching the provided filter(s). Objects not matching the filter(s) are discarded.").arg(short_opts[OnlyMatching], OnlyMatching));
	printText(tr(" %1, %2\t\t  Performs object matching based on names instead of signatures ([schema].[name]).").arg(short_opts[MatchByName], MatchByName));
	printText(tr(" %1, %2 [OBJECTS]  Forces importing children objects related to tables/views/foreign tables matched by the filter(s). Provide a comma-separated list of types.").arg(short_opts[ForceChildren], ForceChildren));
	printText(tr(" %1, %2\t\t  Executes the import in debug mode, printing all queries executed on the server.").arg(short_opts[DebugMode], DebugMode));
	printText();

	printText(tr("Diff options: "));
	printText(tr(" %1, %2 [FILE]\t  The database model file used in the comparison. This option implies the use of option %3.").arg(short_opts[CompareFile], CompareFile, SaveDiff));
	printText(tr(" %1, %2 [DBNAME]\t  The database used in the comparison. All generated SQL code is applied to it.").arg(short_opts[CompareDb], CompareDb));
	printText(tr(" %1, %2\t\t\t  Switches to partial diff operation. Provide object filters using the import option %3.").arg(short_opts[PartialDiff], PartialDiff, FilterObjects));
	printText(tr(" %1, %2\t\t\t  Forces a full diff if the provided filters fail to retrieve objects for a partial diff operation.").arg(short_opts[Force], Force));
	printText(tr(" %1, %2\t\t  Matches database model objects with modification dates starting from the specified date. (Only for partial diff)").arg(short_opts[StartDate], StartDate));
	printText(tr(" %1, %2\t\t  Matches database model objects with modification dates ending on the specified date. (Only for partial diff)").arg(short_opts[EndDate], EndDate));
	printText(tr(" %1, %2\t\t\t  Saves the generated diff code to the output file.").arg(short_opts[SaveDiff], SaveDiff));
	printText(tr(" %1, %2\t\t\t  Applies the generated diff code to the database server.").arg(short_opts[ApplyDiff], ApplyDiff));
	printText(tr(" %1, %2\t\t  Skips previewing the generated diff code before applying it to the server.").arg(short_opts[NoDiffPreview], NoDiffPreview));
	printText(tr(" %1, %2\t  Drops cluster-level objects like roles and tablespaces.").arg(short_opts[DropClusterObjs], DropClusterObjs));
	printText(tr(" %1, %2\t\t  Revokes existing permissions on the database. New permissions configured in the input model are still applied.").arg(short_opts[RevokePermissions], RevokePermissions));
	printText(tr(" %1, %2\t\t  Generates DROP commands for objects present in the source model but missing in the compared database.").arg(short_opts[DropMissingObjs], DropMissingObjs));
	printText(tr(" %1, %2\t\t  Forces dropping missing columns and constraints. Only affects columns and constraints; other missing objects are preserved.").arg(short_opts[ForceDropColsConstrs], ForceDropColsConstrs));
	printText(tr(" %1, %2\t\t  Renames the destination database when the involved databases have different names.").arg(short_opts[RenameDb], RenameDb));
	printText(tr(" %1, %2\t\t  Disables cascade mode when dropping objects.").arg(short_opts[NoCascadeDrop], NoCascadeDrop));
	printText(tr(" %1, %2\t  Disables sequence reuse on serial columns. Drops the existing sequence assigned to a serial column and creates a new one.").arg(short_opts[NoSequenceReuse], NoSequenceReuse));
	printText(tr(" %1, %2\t\t  Recreates unmodifiable objects (those that cannot be changed via ALTER command).").arg(short_opts[RecreateUnmod], RecreateUnmod));
	printText(tr(" %1, %2\t\t  Replaces modifiable objects (those that support CREATE OR REPLACE command).").arg(short_opts[ReplaceModified], ReplaceModified));
	printText(tr(" %1, %2 [OBJECTS] Uses DROP and CREATE commands to completely modify changed objects. Provide a comma-separated list of object types.").arg(short_opts[ForceReCreateObjs], ForceReCreateObjs));
	printText();

	printText(tr("Model fix options: "));
	printText(tr(" %1, %2 [NUMBER]\t  Model fix attempts. When reaching the maximum count, invalid objects will be discarded.").arg(short_opts[FixTries], FixTries));
	printText();

	#ifndef Q_OS_MACOS
		printText(tr("File association options: "));
		printText(tr(" %1, %2\t\t  Applies DBM file association system-wide instead of for the current user only.").arg(short_opts[SystemWide], SystemWide));
		printText(tr(" %1, %2 \t\t\t  Forces MIME type installation or uninstallation.").arg(short_opts[Force], Force));
		printText();
	#endif

	printText(tr("Config files creation options: "));
	printText(tr(" %1, %2 \t\t  Copies only missing configuration files to the user's local storage.").arg(short_opts[MissingOnly], MissingOnly));
	printText(tr(" %1, %2 \t\t\t  Forces recreation of all configuration files and backs up current settings.").arg(short_opts[Force], Force));
	printText();

	printText(tr("Plugins options: "));
	printText(tr(" %1, %2 \t\t  Lists available plugins.").arg(short_opts[ListPlugins], ListPlugins));
	printText(tr(" %1, %2 \t\t  Ignores errors from plugins that failed to load.").arg(short_opts[IgnoreFaultyPlugins], IgnoreFaultyPlugins));
	printText();

	// Displaying loaded plugin's options
	attribs_map p_short_opts, p_opts_desc;
	std::map<QString, bool> p_long_opts;

	for(auto &plugin : plugins)
	{
		p_short_opts = plugin->getShortOptions();
		p_long_opts =  plugin->getLongOptions();
		p_opts_desc = plugin->getOptsDescription();

		printText(tr("%1 options: ").arg(plugin->getPluginTitle()));

		for(auto &itr : p_opts_desc)
			printText(QString("  %1, %2 \t\t  %3").arg(p_short_opts[itr.first], itr.first, itr.second));

		printText();
	}

	printText();
	printText(tr("** When exporting to SQL file in split mode, if none of the options %1, %2, and %3\
\n   are specified, the generated files will be named to reflect the correct creation order.").arg(DependenciesSql, ChildrenSql, GroupByType));
	printText();
	printText(tr("** The FILTER value in the %1 option has the form type:pattern:mode. ").arg(FilterObjects));
	printText(tr("   * The section `type' is the type of object to be filtered and accepts the following values (invalid types ignored): "));

	QStringList list;
	QString child_list;

	for(auto &type : BaseObject::getChildObjectTypes(ObjectType::Table))
	{
		if(type == ObjectType::Column)
			continue;

		list.append(BaseObject::getSchemaName(type));
	}

	list.sort();
	child_list = list.join(", ");

	QStringList fmt_types, lines, type_list = Catalog::getFilterableObjectNames();
	int i = 0;

	type_list.prepend(Attributes::Any);

	for(auto &type : type_list)
	{
		fmt_types.append(type);
		i++;
		if(i % 8 == 0 || i == type_list.size() - 1)
		{
			lines.append("     > " + fmt_types.join(", "));
			fmt_types.clear();
		}
	}

	printText(lines.join('\n'));
	
	printText();
	printText(tr("   * The special type `%1' allows writing a single filter that applies to all object types.").arg(Attributes::Any));
	printText();
	printText(tr("   * The `pattern' section is the text pattern to match against object names."));
	printText();
	printText(tr("   * The `mode' section defines how the pattern matching is performed. It accepts two values:"));
	printText(tr("     > `%1' treats the pattern as a wildcard string when matching object names.").arg(UtilsNs::FilterWildcard));
	printText(tr("     > `%1' treats the pattern as a Perl-like regular expression when matching object names.").arg(UtilsNs::FilterRegExp));
	printText();
	printText(tr("   * The option %1 takes effect only when used with %2 and prevents discarding children of matched tables.").arg(ForceChildren, OnlyMatching));
	printText(tr("     Other tables imported as dependencies of matched objects will have their children discarded."));
	printText(tr("     The comma-separated list of table children objects accepts these values:"));
	printText(tr("     > %1").arg(child_list));
	printText(tr("     > Use the special keyword `%1' to include all children objects.").arg(AllChildren));
	printText();
	printText(tr("   * NOTE: All comparisons during filtering are case-insensitive."));
	printText(tr("     Using filtering options may import additional objects due to automatic dependency resolution."));
	printText();
	printText(tr("** The diff process supports all import-related options."));
	printText(tr("   It also accepts these export operation options: `%1', `%2', and `%3'.").arg(IgnoreDuplicates, IgnoreErrorCodes, NonTransactional));
	printText();
	printText(tr("** The partial diff operation always forces the options %1 and %2 = %3 for more reliable results.").arg(OnlyMatching, ForceChildren, AllChildren));
	printText(tr("   * The options %1 and %2 accept ISO8601 date/time format: `yyyy-MM-dd hh:mm:ss'.").arg(StartDate, EndDate));
	printText();
	printText(tr("** When diffing between two databases (%1 and %2), you can specify separate connections/aliases.").arg(InputDb, CompareDb));
	printText(tr("   If only one connection is specified, it will be used for both the input database and the comparison database."));
	printText(tr("   To specify a second connection, append `1' to any connection parameter listed above."));
	printText(tr("   This associates the connection exclusively with %1.").arg(CompareDb));
	printText();

	if(!plugin_load_errors.isEmpty())
	{
		printText();
		printText("**");
		printText(tr("** WARNING: Unable to retrieve plugin options due to errors!"));
		printText(tr("**          Execute pgmodeler-cli with `%1' for detailed error information.").arg(ListPlugins));
		printText("**");
		printText();
	}
}

void PgModelerCliApp::listConnections()
{
	std::map<QString, Connection *>::iterator itr=connections.begin();

	if(connections.empty())
		printText(tr("There are no connections configured."));
	else
	{
		unsigned id=0;

		printText(tr("Available connections (alias : connection string)"));
		printText();

		while(itr != connections.end())
		{
			printText("[" + QString::number(id++) + "] " + itr->first + " : " +
								itr->second->getConnectionString().replace(PasswordRegExp, PasswordPlaceholder));
			itr++;
		}
	}

	printText();
}

void PgModelerCliApp::parseOptions(attribs_map &opts)
{
	//Loading connections
	if(opts.count(ListConns) || opts.count(ExportToDbms) || opts.count(ImportDb) || opts.count(Diff))
	{
		conn_conf = new ConnectionsConfigWidget;
		conn_conf->loadConfiguration();
		conn_conf->getConnections(connections, false);
	}
	//Loading general and relationship settings when exporting to image formats
	else if(opts.count(ExportToPng) || opts.count(ExportToSvg))
	{
		general_conf = new GeneralConfigWidget;
		rel_conf = new RelationshipConfigWidget;

		general_conf->loadConfiguration();
		rel_conf->loadConfiguration();
	}

	//Creating the export/import/diff helpers when one of the operations are specified
	if(opts.count(ExportToDbms) || opts.count(ExportToFile) ||
		 opts.count(ExportToPng) || opts.count(ExportToSvg) ||
		 opts.count(ExportToDict) || opts.count(ImportDb) ||
		 opts.count(Diff))
	{
		export_hlp = new ModelExportHelper;
		import_hlp = new DatabaseImportHelper;
		diff_hlp = new ModelsDiffHelper;
	}

	if(opts.empty() || opts.count(Help))
		showMenu();
	else
	{
		QString curr_op_mode;
		int exp_mode_cnt = 0, other_modes_cnt = 0, plugin_modes_cnt = 0;

		bool export_dbms = (opts.count(ExportToDbms) > 0),
				export_file = (opts.count(ExportToFile) > 0);

		fix_model = (opts.count(FixModel) > 0);
		upd_mime = (opts.count(DbmMimeType) > 0);
		import_db = (opts.count(ImportDb) > 0);
		diff = (opts.count(Diff) > 0);
		create_configs= (opts.count(CreateConfigs) > 0);
		list_conns = (opts.count(ListConns) > 0);
		list_plugins = (opts.count(ListPlugins) > 0);
		plugin_op = false;
		export_op = false;

		plugin_modes_cnt = definePluginsExecOrder(opts);

		for(auto &itr : accepted_opts)
		{
			if(itr.first == Attributes::Connection)
				continue;

			if(opts.count(itr.first))
			{
				curr_op_mode = itr.first;

				if(itr.first == ExportToFile || itr.first == ExportToPng ||
					 itr.first == ExportToSvg || itr.first == ExportToDbms ||
					 itr.first == ExportToDict)
				{
					exp_mode_cnt++;
					export_op = true;
				}
				else
					other_modes_cnt++;
			}
		}

		plugin_op = (plugin_modes_cnt > 0);

		if(opts.count(ZoomFactor))
			zoom = opts[ZoomFactor].toDouble()/static_cast<double>(100);

		if(other_modes_cnt == 0 && exp_mode_cnt == 0)
			throw Exception(tr("No operation mode was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if((exp_mode_cnt > 0 && (fix_model || upd_mime || import_db || diff || create_configs || list_conns || list_plugins)) ||
			 (exp_mode_cnt == 0 && other_modes_cnt > 1))
			throw Exception(tr("Multiple operation modes were specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(!fix_model && !upd_mime && !plugin_op && exp_mode_cnt > 1)
			throw Exception(tr("Multiple export modes were specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(!plugin_op && !list_conns && !list_plugins && !upd_mime && !import_db &&
			 !diff && !create_configs && !opts.count(Input))
			throw Exception(tr("No input file was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(import_db && !opts.count(InputDb))
			throw Exception(tr("No input database was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(!plugin_op && !export_dbms && !upd_mime && !list_conns &&
			 !list_plugins && !diff && !create_configs && !opts.count(Output))
			throw Exception(tr("No output file was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(!export_dbms && !upd_mime && !import_db && !list_conns && !list_plugins &&
			 !create_configs && opts.count(Input) && opts.count(Output) &&
			 QFileInfo(opts[Input]).absoluteFilePath() == QFileInfo(opts[Output]).absoluteFilePath())
			throw Exception(tr("The input file must be different from the output file!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(export_dbms && !opts.count(ConnAlias) &&
			 (!opts.count(Host) || !opts.count(User) || !opts.count(Passwd) || !opts.count(InitialDb)) )
			throw Exception(tr("Incomplete connection information!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(export_dbms && opts.count(Force) && !opts.count(DropDatabase))
			throw Exception(tr("The option `%1' must be used only with `%2' when exporting to DBMS!").arg(Force, DropDatabase), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(export_dbms && opts.count(Simulate) && opts.count(NonTransactional))
			throw Exception(tr("The options `%1' and `%2' cannot be used together when exporting to DBMS!").arg(Simulate, NonTransactional), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(export_dbms && (opts.count(IgnoreErrorCodes) || opts.count(IgnoreDuplicates)) && !opts.count(NonTransactional))
			throw Exception(tr("The options `%1' and `%2' cannot be used in transactional export mode. Use `%3' to enable these options!").arg(IgnoreErrorCodes, IgnoreDuplicates, NonTransactional), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(opts.count(ExportToPng) && (zoom < ModelWidget::MinimumZoom || zoom > ModelWidget::MaximumZoom))
			throw Exception(tr("Invalid zoom factor specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		
		if(upd_mime && opts[DbmMimeType] != Install && opts[DbmMimeType] != Uninstall)
			throw Exception(tr("Invalid action specified for MIME type update option!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(create_configs && opts.count(Force) && opts.count(MissingOnly))
			throw Exception(tr("The options `%1' and `%2' cannot be used together when handling configuration files!").arg(Force, MissingOnly), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(opts.count(DependenciesSql) || opts.count(ChildrenSql) || opts.count(GroupByType))
		{
			unsigned num_opts = opts.count(DependenciesSql) ? 1 : 0;
			num_opts += opts.count(ChildrenSql) ? 1 : 0;
			num_opts += opts.count(GroupByType) ? 1 : 0;

			if(!export_file || (export_file && !opts.count(Split)))
				throw Exception(tr("The options `%1', `%2', and `%3' must be used together with the split mode option `%4'!").arg(DependenciesSql, ChildrenSql, GroupByType, Split), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(num_opts > 1)
				throw Exception(tr("The options `%1', `%2', and `%3' cannot be used simultaneously!").arg(DependenciesSql, ChildrenSql, GroupByType), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		if(diff)
		{
			// If the user wants to compare two models we force the use of --save
			if(opts.count(CompareFile) && !opts.count(SaveDiff))
				 opts[SaveDiff] = Attributes::True;

			if(!opts.count(Input) && !opts.count(InputDb))
				throw Exception(tr("No input file or database was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(Input) && opts.count(InputDb))
				throw Exception(tr("The input file and input database cannot be used simultaneously!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(!opts.count(CompareDb) && !opts.count(CompareFile))
				throw Exception(tr("No database or model file to compare was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(InputDb) && opts.count(CompareFile))
				throw Exception(tr("Diff between a database and a model is not supported!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(!opts.count(SaveDiff) && !opts.count(ApplyDiff))
				throw Exception(tr("No diff action (save or apply) was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(CompareFile) && opts.count(ApplyDiff))
				throw Exception(tr("The option `%1' cannot be used together with `%2'!").arg(CompareFile, ApplyDiff), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(SaveDiff) && opts[Output].isEmpty())
				throw Exception(tr("No output file for the diff code was specified!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(PartialDiff) && opts[Input].isEmpty() && (opts.count(StartDate) || opts.count(EndDate)))
				throw Exception(tr("Date filters are allowed only for partial diff operations using an input model!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(PartialDiff) && opts.count(FilterObjects) && (opts.count(StartDate) || opts.count(EndDate)))
				throw Exception(tr("Date filters and object filters cannot be used together!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			if(opts.count(PartialDiff) && !opts.count(FilterObjects) && !opts.count(StartDate) && !opts.count(EndDate))
				throw Exception(tr("Partial diff enabled but no object filter was provided!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

			// For partial diff we force the --only-matching option and --force-children = all
			if(opts.count(PartialDiff))
			{
				opts[ForceChildren] = AllChildren;
				opts[OnlyMatching] = "";
			}

			// Validating the date formats in the provided start/end dates
			QDateTime *dates[2] = { &start_date, &end_date };
			QStringList dt_params = { StartDate, EndDate };

			for(int idx = 0; idx < 2; idx++)
			{
				if(opts.count(dt_params[idx]))
				{
					*dates[idx] = QDateTime::fromString(opts[dt_params[idx]], Qt::ISODate);

					if(!dates[idx]->isValid())
						throw Exception(tr("Invalid date format `%1' in option `%2'!").arg(opts[dt_params[idx]], dt_params[idx]), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
				}
			}

			/* If any of the dates are correctly parsed we need to force
			 * the signature name matching, the forced filtering of all table children objects
			 * and the only-matching option, so the objects can be correctly retrieved from
			 * the destination database */
			if(start_date.isValid() || end_date.isValid())
			{
				parsed_opts.erase(MatchByName);
				parsed_opts[ForceChildren] = AllChildren;
				parsed_opts[OnlyMatching] = "";
			}
		}
		
		//Converting input and output files to absolute paths to avoid that they are read/written on the app's working dir
		if(opts.count(Input))
			opts[Input] = QFileInfo(opts[Input]).absoluteFilePath();

		if(opts.count(Output))
			opts[Output] = QFileInfo(opts[Output]).absoluteFilePath();

		/* Special treatment for filter parameters:
		 * Since several filter parameters can be specified, we need to join
		 * everything in a single string list so it can be passed to the import helper correctly */
		if(opts.count(FilterObjects))
		{
			opts.erase(FilterObjects);

			for(auto &op : opts)
			{
				if(op.first.contains(FilterObjects))
					obj_filters.append(op.second);
			}
		}

		/* Performing a final validation on the parsed options which consists
		 * in checking if all provided options are compatible with the operation mode selected */
		QStringList acc_opts = accepted_opts[curr_op_mode];
		QString long_opt;
		static QRegularExpression num_rx { "[0-9]+$" };

		// Diff, import and export (to DBMS) share the same connection options
		if(diff || import_db || export_dbms)
			acc_opts.append(accepted_opts[ConnOptions]);

		// Diff also accepts all import parameters
		if(diff)
			acc_opts.append(accepted_opts[ImportDb]);

		for(auto &itr : opts)
		{
			long_opt = itr.first;

			if(long_opt == curr_op_mode || long_opt == Silent)
				continue;

			/* Before validating the option we need to remove any appended number to the option name
			 * This happens for options related to object filters and connections */
			long_opt.remove(num_rx);

			if(!acc_opts.contains(long_opt))
			{
				throw Exception(tr("The option `%1' is not accepted by the operation mode `%2'!").arg(long_opt, curr_op_mode),
												ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
			}
		}

		parsed_opts = opts;
	}
}

int PgModelerCliApp::exec()
{
	try
	{
		if(!parsed_opts.empty())
		{
			showVersionInfo();

			if(list_conns)
				listConnections();
			else if(list_plugins)
				listPlugins();
			else
			{
				runPluginsPreOperations();

				if(fix_model)
					fixModel();
				else if(upd_mime)
					updateMimeType();
				else if(create_configs)
					createConfigurations();
				else if(import_db)
					importDatabase();
				else if(diff)
					diffModels();
				else if(export_op)
					exportModel();
				else
					runPluginsOperations();

				runPluginsPostOperations();
			}
		}

		return 0;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void PgModelerCliApp::updateProgress(int progress, QString msg, ObjectType)
{
	if(progress > 0)
		printMessage(QString("[%1%] ").arg(progress > 100 ? 100 : progress) + msg);
	else
		printMessage(msg);
}

void PgModelerCliApp::printIgnoredError(QString err_cod, QString err_msg, QString cmd)
{
	printText();
	printText(tr("** Error code `%1' found and ignored. Proceeding with export.").arg(err_cod));
	printText(tr("** Command: %1").arg(cmd));
	printText(err_msg);
	printText();
}

void PgModelerCliApp::handleObjectAddition(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		ObjectType obj_type=graph_obj->getObjectType();
		QGraphicsItem *item=nullptr;

		switch(obj_type)
		{
			case ObjectType::Table:
				item=new TableView(dynamic_cast<Table *>(graph_obj));
			break;

			case ObjectType::ForeignTable:
				item=new TableView(dynamic_cast<ForeignTable *>(graph_obj));
			break;

			case ObjectType::View:
				item=new GraphicalView(dynamic_cast<View *>(graph_obj));
			break;

			case ObjectType::Relationship:
			case ObjectType::BaseRelationship:
				item=new RelationshipView(dynamic_cast<BaseRelationship *>(graph_obj));
			break;

			case ObjectType::Schema:
				item=new SchemaView(dynamic_cast<Schema *>(graph_obj));
			break;

			default:
				item=new StyledTextboxView(dynamic_cast<Textbox *>(graph_obj));
			break;
		}

		scene->addItem(item);

		if(BaseTable::isBaseTable(obj_type))
			dynamic_cast<Schema *>(graph_obj->getSchema())->setModified(true);
	}
}

void PgModelerCliApp::handleObjectRemoval(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		scene->removeItem(dynamic_cast<QGraphicsItem *>(graph_obj->getOverlyingObject()));

		//Updates the parent schema if the removed object were a table or view
		if(graph_obj->getSchema() && BaseTable::isBaseTable(graph_obj->getObjectType()))
			dynamic_cast<Schema *>(graph_obj->getSchema())->setModified(true);
	}
}

void PgModelerCliApp::extractObjectXML()
{
	QString buf, lin, def_xml, end_tag;
	QTextStream ts;
	int start=-1, end=-1;
	bool open_tag=false, close_tag=false, is_rel=false, short_tag=false, end_extract_rel=false, is_change_log=false;

	printMessage(tr("Extracting object XML definitions..."));
	buf.append(UtilsNs::loadFile(parsed_opts[Input]));

	// Extracting pgModeler version from input model
	QRegularExpression ver_expr(AttributeExpr.arg(Attributes::PgModelerVersion));
	QRegularExpressionMatch match;
	static QRegularExpression val_delim_rx {"(\\\"|\\=| )+" },
	version_num_rx {"(\\d\\.\\d\\.\\d)((\\-)(alpha|beta)(\\d))?" };

	QStringList capt_txts;
	match = ver_expr.match(buf);
	//start = match.capturedStart();
	capt_txts = match.capturedTexts();
	model_version = !capt_txts.isEmpty() ? capt_txts.at(0) : "";
	model_version.remove(Attributes::PgModelerVersion);
	model_version.remove(val_delim_rx);

	if(!model_version.contains(version_num_rx))
	{
		printMessage(tr("** WARNING: Unable to determine the pgModeler version used to create the input model!"));
		printMessage(tr("            Some fix actions that depend on the model version will not be applied!"));
		model_version.clear();
	}

	QRegularExpressionMatch header_match;
	QRegularExpression header_regexp(QString("^<\\?xml.+<%1").arg(Attributes::DbModel),
														QRegularExpression::DotMatchesEverythingOption |
														QRegularExpression::DontCaptureOption);

	//Check if the file contains a valid header (for .dbm file)
	/* ATTENTION: PCRE2 (base implementation of QRegularExpression) limits the amount of groups (defined by  (expr) ) to
	 * be captured. So, in large buffer expression such (.)+ must be used with caution. In some cases the matching won't
	 * work even if it works on smaller text buffers. The workaround is to use a small portion of the buffer as well as avoiding
	 * capture groups. In the case below we just need to know the position of a certain regexp in the buffer,
	 * so we use a small portion of the entire buffer as subject of the searching.
	 *
	 * Reference: https://stackoverflow.com/questions/52980957/qregularexpression-lazy-matching-not-working-for-very-large-strings */
	header_match = header_regexp.match(buf.mid(0, 5000));
	start = header_match.capturedStart();

	if(start < 0)
		throw Exception(tr("Invalid input file! The file does not appear to be a pgModeler-generated model or is corrupted!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

	//Extracting layers informations from the tag <dbmodel>
	static QRegularExpression dbm_regexp { TagExpr.arg(Attributes::DbModel) },

			db_end_regexp { EndTagExpr.arg(Attributes::Database) },

			//[schema].[func_name](...OUT [type]...)
			func_signature{ "(\")(.)+(\\.)(.)+(\\()(.)*(OUT )(.)+(\\))(\")" },

			//[,]OUT [schema].[type]
			out_param { "(,)?(OUT )([a-z]|[0-9]|(\\.)|(\\_)|(\\-)|( )|(\\[)|(\\])|(&quot;))+((\\()([0-9])+(\\)))?" },

			open_tag_rx { "^(((\n)|(\t))*(<))" };

	int attr_start =-1, attr_end = -1, dbm_start = -1;
	QString aux_buf,
			layers, active_layers, attr_expr = "(%1)( )*(=)(\")";
	QList<unsigned> act_layers_ids;

	match = dbm_regexp.match(buf);
	dbm_start = match.capturedStart();
	aux_buf = buf.mid(dbm_start, buf.indexOf(db_end_regexp) - dbm_start);

	//Layers names
	attr_start = aux_buf.indexOf(Attributes::Layers);
	attr_end = aux_buf.indexOf(Attributes::ActiveLayers);
	layers = aux_buf.mid(attr_start, attr_end - attr_start);
	layers.remove(QRegularExpression(attr_expr.arg(Attributes::Layers)));
	layers.remove('"');

	QStringList aux_layers = layers.trimmed().split(',', Qt::SkipEmptyParts);

	/* In 0.9.x there was a "Default" layer created automatically in the model
	 * In order to not losing the correct layer arrangement, we include that default
	 * layer in the list of layers in the model so the objects doesn't put in the
	 * wrong layer when loaded after the model is fixed */
	if(model_version < "1.0.0" && !aux_layers.contains("Default"))
		aux_layers.prepend("Default,");

	input_model->setLayers(aux_layers);

	//Active layers
	attr_start = attr_end;
	attr_end = aux_buf.indexOf(Attributes::LayerNameColors, attr_start);

	if(attr_end < 0)
		attr_end = aux_buf.indexOf(">", attr_start);

	active_layers = aux_buf.mid(attr_start, attr_end - attr_start);
	active_layers.remove(QRegularExpression(attr_expr.arg(Attributes::ActiveLayers)));
	active_layers.remove('"');
	active_layers.replace(',', ';');

	for(auto &id : active_layers.trimmed().split(';', Qt::SkipEmptyParts))
		act_layers_ids.push_back(id.toUInt());

	input_model->setActiveLayers(act_layers_ids);
	buf.remove(0, dbm_start);

	//Checking if the header ends on a role declaration
	start = buf.indexOf(QString("<%1").arg(Attributes::Role)),
	end = buf.lastIndexOf(QString("/%1>").arg(Attributes::Role));

	// If we found role declarations we clear the header until there
	if(start >= 0 && end > start)
		buf.remove(0, start);
	else
		// Instead, we clear the header until the starting of database declaration
		buf.remove(0, buf.indexOf(QString("<%1").arg(Attributes::Database)));

	buf.remove(QString("<\\%1>").arg(Attributes::DbModel));
	ts.setString(&buf);

	//Extracts the objects xml line by line
	while(!ts.atEnd())
	{
		lin=ts.readLine();

		/* Collecting changelog entries if present and storing in a separated buffer
		 * so it can be restored in the fixed model during objects' reconstruction */
		if(!is_change_log && lin.contains(TagExpr.arg(Attributes::Changelog)))
			is_change_log = true;

		if(is_change_log)
		{
			changelog.append(lin);

			if(lin.contains(EndTagExpr.arg(Attributes::Changelog)))
				is_change_log = false;
			else
				continue;
		}

		/*  Special case for empty tags like <language />, they will be converted to
		<language></language> in order to be correctly extracted further. Currently only language has this
		behaviour, so additional object may be added in the future. */
		if(lin.contains(QString("<%1").arg(BaseObject::getSchemaName(ObjectType::Language))))
		{
			lin=lin.simplified();

			if(lin.contains("/>"))
				lin.replace("/>", QString("></%1>").arg(BaseObject::getSchemaName(ObjectType::Language)));
		}
		/* Special case for function signatures. In previous releases, the function's signature was wrongly
	 including OUT parameters and according to docs they are not part of the signature, so it is needed
	 to remove them if the current line contains a valid signature with parameters. */
		else if(lin.contains(func_signature))
			lin.remove(out_param);

		if(is_rel && (((short_tag && lin.contains("/>")) ||
									 (lin.contains("[a-z]+") && !containsRelAttributes(lin)))))
			open_tag=close_tag=true;
		else
		{
			//If the line contains an objects open tag
			if(lin.contains(open_tag_rx) && !open_tag)
			{
				//Check the flag indicating an open tag
				open_tag=true;

				start=lin.indexOf('<');
				end=lin.indexOf(' ');
				if(end < 0)	end=lin.indexOf('>');

				//Configures the end tag with the same word extracted from open tag
				end_tag=lin.mid(start, end-start+1).trimmed();
				end_tag.replace("<", "</");

				if(!end_tag.endsWith('>'))
					end_tag+=">";

				/* Checking if the line start a relationship. Relationships are treated
		a little different because they can be empty <relationship attribs /> or
		contain open and close tags <relationship attribs></relationship> */
				is_rel=lin.contains(Attributes::Relationship);

				if(is_rel)
				{
					end_extract_rel=short_tag=false;

					while(!end_extract_rel && !ts.atEnd())
					{
						def_xml+=lin + "\n";
						lin=lin.trimmed();

						//Checking if the current line is the end of a short-tag relationship
						if(!short_tag && !lin.startsWith('<') && lin.endsWith("/>"))
							short_tag=true;

						end_extract_rel=((!short_tag && lin.contains(end_tag)) || short_tag);

						if(!end_extract_rel)
							lin=ts.readLine();
					}

					close_tag=true;
				}
				else
					close_tag=lin.contains(end_tag);
			}
			else if(open_tag && lin.contains(end_tag))
				close_tag=true;
		}

		if(!is_rel && !lin.isEmpty())
			def_xml+=lin + "\n";
		else if(lin.isEmpty())
			def_xml+="\n";

		//If the iteration reached the end of the object's definition
		if(open_tag && close_tag)
		{
			//Pushes the extracted definition to the list (only if not empty)
			if(def_xml!="\n")
			{
				objs_xml.push_back(def_xml);
				buffer_size += def_xml.size();
			}

			def_xml.clear();
			open_tag=close_tag=is_rel=false;
		}
	}
}

void PgModelerCliApp::recreateObjects()
{
	QStringList fail_objs, constr, list;
	QString xml_def, aux_def, start_tag = "<%1", end_tag = "</%1>", aux_tag;
	BaseObject *object=nullptr;
	ObjectType obj_type=ObjectType::BaseObject;
	std::vector<ObjectType> types={ ObjectType::Index, ObjectType::Trigger, ObjectType::Rule };
	attribs_map attribs;
	bool use_fail_obj=false;
	unsigned tries=0, max_tries=parsed_opts[FixTries].toUInt();
	int start_pos=-1, end_pos=-1, len=0;
	qint64 curr_size = 0;

	printMessage(tr("Recreating objects..."));

	if(max_tries==0)
		max_tries=1;

	input_model->createSystemObjects(false);

	while(!objs_xml.isEmpty())
	{
		//If there are failed objects and the flag is set
		if(use_fail_obj && !fail_objs.isEmpty())
		{
			xml_def = fail_objs.front();
			fail_objs.pop_front();
			use_fail_obj=false;
		}
		else
		{
			xml_def = objs_xml.front();
			objs_xml.pop_front();
			fixObjectAttributes(xml_def);
		}

		try
		{
			/* Converting views in the older format created in versions <= 1.1.0-alpha)
			 * to the new format introduced by 1.1.0-beta */
			if(model_version <= "1.1.0-alpha1" && xml_def.contains(start_tag.arg(BaseObject::getSchemaName(ObjectType::View))))
			{
				CompatNs::View * view = CompatNs::createLegacyView(xml_def, input_model);
				xml_def = CompatNs::convertToNewView(view);
			}

			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(xml_def);
			obj_type=BaseObject::getObjectType(xmlparser->getElementName());

			xmlparser->getElementAttributes(attribs);

			if(obj_type==ObjectType::Database)
				input_model->configureDatabase(attribs);
			else
			{
				if(obj_type==ObjectType::Table)
				{
					//Before create a table extract it's foreign keys
					list=extractForeignKeys(xml_def);

					/* If fks were extracted insert them on the main constraints list
					 * and restarts the XMLParser with the modified buffer */
					if(!list.isEmpty())
					{
						constr.append(list);
						xmlparser->restartParser();
						xmlparser->loadXMLBuffer(xml_def);
					}
				}

				//Discarding fk relationships
				if(obj_type!=ObjectType::Relationship ||
						(obj_type==ObjectType::Relationship && !xml_def.contains(QString("\"%1\"").arg(Attributes::RelationshipFk))))
				{
					object=input_model->createObject(obj_type);

					if(object)
					{
						if(!dynamic_cast<TableObject *>(object) && obj_type!=ObjectType::Relationship && obj_type!=ObjectType::BaseRelationship)
							input_model->addObject(object);

						curr_size += xml_def.size();
						printMessage(QString("[%1%] %2")
												 .arg(static_cast<int>((curr_size/static_cast<double>(buffer_size)) * 100))
												 .arg(tr("Object recreated: `%1' (%2)").arg(object->getName(true), object->getTypeName())));
					}

					//For each sucessful created object the method will try to create a failed one
					use_fail_obj=(!fail_objs.isEmpty());
				}				
				else if(obj_type == ObjectType::Relationship &&
								xml_def.contains(QString("\"%1\"").arg(Attributes::RelationshipFk)))
					curr_size += xml_def.size();

				/* Additional step to extract indexes/triggers/rules from within tables/views
				 * and putting their xml on the list of object to be created */
				static QRegularExpression idx_trg_rl_rx { "(<)(index|trigger|rule)" };

				if(BaseTable::isBaseTable(obj_type) && xml_def.contains(idx_trg_rl_rx))
				{
					for(auto &type : types)
					{
						do
						{
							//Checking where the object starts and ends
							aux_tag=start_tag.arg(BaseObject::getSchemaName(type));
							start_pos=xml_def.indexOf(aux_tag);
							end_pos=(start_pos >=0 ? xml_def.indexOf(end_tag.arg(BaseObject::getSchemaName(type))) : -1);

							if(start_pos >=0 && end_pos >= 0)
							{
								//Extracts the xml code
								len=(end_pos - start_pos) + end_tag.arg(BaseObject::getSchemaName(type)).length() + 1;
								aux_def=xml_def.mid(start_pos, len);

								//Remove the code from original table's definition
								xml_def.remove(start_pos, len);

								//If the extract object doesn't contains the 'table=' attribute it'll be added.
								if(!aux_def.contains("table="))
								{
									aux_def.replace(aux_tag, QString("%1 table=\"%2\"")
																	.arg(aux_tag, UtilsNs::convertToXmlEntities(object->getName(true))));
								}

								objs_xml.push_back(aux_def);
							}
						}
						while(start_pos >= 0);
					}
				}
			}
		}
		catch(Exception &e)
		{
			if(obj_type != ObjectType::Database)
			{
				QString error = tr("** WARNING: Failed to recreate the object!");

				fail_objs.push_back(xml_def);
				printText(QString("%1\n** %2\n").arg(error, e.getErrorMessage()));
				error += QString("%1\n\n%2\n***").arg(e.getExceptionsText(), xml_def);

				// Store the error in the log file as well as the XML code of the failed object
				QFile fix_log;
				fix_log.setFileName(GlobalAttributes::getTemporaryFilePath(ModelFixLog));
				fix_log.open(QFile::Append);
				fix_log.write(error.toUtf8());
				fix_log.close();

				has_fix_log = true;
			}
			else
				throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		if(objs_xml.isEmpty() && (!fail_objs.isEmpty() || !constr.isEmpty()))
		{
			tries++;

			//If the maximum creation tries reaches the maximum value
			if(tries > max_tries)
			{
				//Outputs the code of the objects that wasn't created
				printText();
				printText(tr("** A total of %1 object(s) could not be fixed: ").arg(fail_objs.size()));

				while(!fail_objs.isEmpty())
				{
					printText(fail_objs.front());
					fail_objs.pop_front();
				}

				break;
			}
			
			printMessage(tr("** WARNING: Some objects may not be fixable. Retrying... (attempt %1/%2)").arg(tries, max_tries));
			input_model->validateRelationships();
			objs_xml = fail_objs;
			objs_xml.append(constr);
			fail_objs.clear();
			constr.clear();
		}
	}

	// Fixing the roles memberships.
	Role *role = nullptr, *mem_role = nullptr;
	bool member_fixed = false;

	for(auto &rl : member_roles)
	{
		role = input_model->getRole(rl.first);

		if(!role)
		{
			printMessage(tr("** WARNING: Could not find role `%1'! Ignoring it...").arg(rl.first));
			continue;
		}

		for(auto &name : rl.second)
		{
			mem_role = input_model->getRole(name);

			if(!mem_role)
			{
				printMessage(tr("** WARNING: Could not find role `%1' of `%2'! Ignoring it...").arg(name, rl.first));
				continue;
			}

			role->addRole(Role::MemberRole, mem_role);
			member_fixed = true;
		}
	}

	if(member_fixed)
	{
		printMessage(tr("** WARNING: Role memberships were fixed but their creation order is not guaranteed!"));
		printMessage(tr("            It may be necessary to run the fix tool again on the file `%1'.").arg(parsed_opts[Output]));
	}

	// Reconstructing the persisted change log if present
	if(!changelog.isEmpty())
	{
		input_model->setPersistedChangelog(true);
		xmlparser->restartParser();
		xmlparser->loadXMLBuffer(changelog);

		if(xmlparser->accessElement(XmlParser::ChildElement))
		{
			do
			{
				try
				{
					xmlparser->getElementAttributes(attribs);
					input_model->addChangelogEntry(attribs[Attributes::Signature], attribs[Attributes::Type],
																		attribs[Attributes::Action], attribs[Attributes::Date]);
				}
				catch(Exception &e)
				{
					printMessage();
					printMessage(tr("** WARNING: Failed to process a changelog entry due to invalid values! Ignoring it."));
					printMessage(tr("** Entry: `%1'.").arg(e.getExtraInfo()));
					printMessage();
				}
			}
			while(xmlparser->accessElement(XmlParser::NextElement));
		}
	}
}

void PgModelerCliApp::fixObjectAttributes(QString &obj_xml)
{
	//Placing objects <index>, <rule>, <trigger> outside of <table>
	if(!obj_xml.startsWith(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Tablespace))) &&
			obj_xml.startsWith(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Table))))
	{
		int start_idx=-1, end_idx=-1, len=0;
		ObjectType obj_types[3]={ ObjectType::Rule, ObjectType::Trigger, ObjectType::Index  };
		QString  curr_tag, curr_end_tag, def, tab_name, sch_name,
				name_attr="name=\"",
				sch_name_attr=TagExpr.arg(BaseObject::getSchemaName(ObjectType::Schema)) + " " + name_attr;

		//Extracting the table's name
		start_idx=obj_xml.indexOf(name_attr);
		end_idx=obj_xml.indexOf("\"", start_idx + name_attr.size());
		tab_name=obj_xml.mid(start_idx, end_idx - start_idx).remove(name_attr);

		//Extracting the table's schema name
		start_idx=obj_xml.indexOf(sch_name_attr);
		end_idx=obj_xml.indexOf('"', start_idx + sch_name_attr.size());
		sch_name=obj_xml.mid(start_idx, end_idx - start_idx).remove(sch_name_attr);

		//Configuring the table=[name] attribute to be included on rule objects
		tab_name=QString("table=\"%1.%2\"").arg(sch_name, tab_name);

		for(auto & obj_type : obj_types)
		{
			curr_tag=TagExpr.arg(BaseObject::getSchemaName(obj_type));
			curr_end_tag=EndTagExpr.arg(BaseObject::getSchemaName(obj_type)) + ">";
			start_idx=obj_xml.indexOf(curr_tag);

			while(start_idx >=0)
			{
				end_idx=obj_xml.indexOf(curr_end_tag);
				len=(end_idx - start_idx) + curr_end_tag.size();
				def=obj_xml.mid(start_idx, len) + "\n\n";
				obj_xml.remove(start_idx, len);

				//Remove the code from original table's definition
				if(def.startsWith(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Rule))))
				{
					start_idx=def.indexOf('>');
					def.replace(start_idx, 1, " " + tab_name + ">");
				}

				start_idx=obj_xml.indexOf(curr_tag);

				if(!def.isEmpty())
					//Puts the object's defintion to the list in order to be evaluated in the main process
					objs_xml.push_back(def);
			}
		}
	}

	//Remove recheck attribute from <element> tags.
	if(obj_xml.contains(TagExpr.arg(Attributes::Element)))
		obj_xml.remove(QRegularExpression(AttributeExpr.arg("recheck")));

	//Remove values greater-op, less-op, sort-op or sort2-op from ref-type attribute from <operator> tags.
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Operator))))
	{
		obj_xml.remove("greater-op");
		obj_xml.remove("less-op");
		obj_xml.remove("sort-op");
		obj_xml.remove("sort2-op");
	}

	//Replacing attribute owner by onwer-col for sequences
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Sequence))))
		obj_xml.replace(QRegularExpression(QString("(%1)( )*(=)(\")").arg(Attributes::Owner)), QString("%1 = \"").arg(Attributes::OwnerColumn));

	/* Remove sysid attribute and encrypted from <role> tags and storing the referenced roles (ref-roles)
	 * for later re-assignment. */
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Role))))
	{
		obj_xml.remove(QRegularExpression(AttributeExpr.arg("sysid")));
		obj_xml.remove(QRegularExpression(AttributeExpr.arg("encrypted")));

		static QRegularExpression ref_roles_expr { QString("(<%1)(.)+(%2)( )*(=)(\")(%3)(\")(.)+(>)").arg(Attributes::Roles, Attributes::RoleType, Attributes::Refer) };
		QRegularExpressionMatch match;
		int pos = -1;

		match = ref_roles_expr.match(obj_xml);
		pos = match.capturedStart();

		if(pos >= 0)
		{
			static QRegularExpression ref_role_rx { "^(.)+(names=\")" };
			QString buf = obj_xml.mid(pos, match.capturedLength()),
					name_attr = "name=\"", role_name;
			int start_idx = 0, end_idx = 0;

			// Extracting the role name
			start_idx = obj_xml.indexOf(name_attr);
			end_idx = obj_xml.indexOf("\"", start_idx + name_attr.size());
			role_name = obj_xml.mid(start_idx, end_idx - start_idx).remove(name_attr);

			// Removing the element <roles ... role-type="refer"/>
			obj_xml.remove(pos, match.capturedLength());

			// Retrieve the name of the ref-roles
			buf.remove(ref_role_rx);
			buf.remove(buf.indexOf("\""), buf.size());

			/* Storing the association between the current role and the ref-roles
			 * in a map for further processing */
			for(auto &rl_name : buf.split(',', Qt::SkipEmptyParts))
				member_roles[rl_name].append(role_name);
		}
	}

	//Replace <parameter> tag by <typeattrib> on <usertype> tags.
	if(obj_xml.contains(TagExpr.arg("usertype")))
	{
		obj_xml.replace(TagExpr.arg(Attributes::Parameter), TagExpr.arg(Attributes::TypeAttribute));
		obj_xml.replace(EndTagExpr.arg(Attributes::Parameter), EndTagExpr.arg(Attributes::TypeAttribute));

		// Fixing the enumeration type labels
		if(!model_version.isEmpty() && model_version <= "0.9.4-beta1")
		{
			/* Until pgModeler 0.9.3, enum labels separators where commas.
			 * In pgModeler 0.9.4, enum labels separators where UtilsNs::DataSeparator */
			QString sep = (model_version == "0.9.4-beta1" ? UtilsNs::DataSeparator : ","),
					values, labels;
			static QRegularExpression enum_start_expr("(" + TagExpr.arg(Attributes::EnumType) + ")( )*(values)( )*(=)( )*(\\\")"),
					enum_end_expr("(\\\")( )*(\\/>)"),
					enum_tag_expr("(" + TagExpr.arg(Attributes::EnumType) + ")(.)+(/>)");
			int start = -1, end = -1;
			QRegularExpressionMatch match;

			match = enum_start_expr.match(obj_xml);
			start = match.capturedStart() + match.capturedLength();

			match = enum_end_expr.match(obj_xml, start);
			end = match.capturedStart();
			values = obj_xml.mid(start, end - start);

			if(!values.isEmpty())
			{
				// Converting each value extract into a separated <enumeration> tag
				for(auto &label : values.split(sep, Qt::SkipEmptyParts))
					labels.append(QString("\t<%1 label=\"%2\"/>\n").arg(Attributes::EnumType, label));

				obj_xml.replace(enum_tag_expr, labels);
			}
		}
	}

	static QRegularExpression auto_suf_rx { AttributeExpr.arg("auto-sufix") },
	src_suf_rx { AttributeExpr.arg("src-sufix") },
	dst_suf_rx { AttributeExpr.arg("dst-sufix") },
	col_ind_rx { AttributeExpr.arg("col-indexes") },
	constr_ind_rx { AttributeExpr.arg("constr-indexes") },
	attr_ind_rx { AttributeExpr.arg("attrib-indexes") };

	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Relationship))))
	{
		//Remove auto-sufix, src-sufix, dst-sufix, col-indexes, constr-indexes, attrib-indexes from <relationship> tags.
		obj_xml.remove(auto_suf_rx);
		obj_xml.remove(src_suf_rx);
		obj_xml.remove(dst_suf_rx);
		obj_xml.remove(col_ind_rx);
		obj_xml.remove(constr_ind_rx);
		obj_xml.remove(attr_ind_rx);

		obj_xml.replace("line-color", Attributes::CustomColor);
	}

	//Renaming the tag <condition> to <predicate> on indexes
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Index))))
	{
		obj_xml.replace(TagExpr.arg(Attributes::Condition), TagExpr.arg(Attributes::Predicate));
		obj_xml.replace(EndTagExpr.arg(Attributes::Condition), EndTagExpr.arg(Attributes::Predicate));
	}

	//Renaming the attribute default to default-value on domain
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Domain))))
		obj_xml.replace(Attributes::Default, Attributes::DefaultValue);

	//Renaming the tag <grant> to <permission>
	if(obj_xml.contains(TagExpr.arg("grant")))
	{
		obj_xml.replace(TagExpr.arg("grant"), TagExpr.arg(BaseObject::getSchemaName(ObjectType::Permission)));
		obj_xml.replace(EndTagExpr.arg("grant"), EndTagExpr.arg(BaseObject::getSchemaName(ObjectType::Permission)));
	}

	/* If the domain doesn't have the tag <constraint> we need to replace the "constraint" attribute
	 * and the tag <expression> by <constraint> tag in <domain> */
	if(!obj_xml.contains(TagExpr.arg(Attributes::Constraint)) &&
			obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Domain))) &&
			obj_xml.contains(TagExpr.arg(Attributes::Expression)))
	{
		int start_idx=-1, end_idx=-1;
		QRegularExpression regexp = QRegularExpression(AttributeExpr.arg(Attributes::Constraint));
		QString constr_name;
		QRegularExpressionMatch match;

		match = regexp.match(obj_xml);

		/* In pgModeler 0.8.2, there wasn't the constraint attribute in domains
		 * so we just create a name for it based on the domain's name */
		if(!match.hasMatch())
		{
			QString	name_attr="name=\"";
			start_idx = obj_xml.indexOf(name_attr);
			end_idx = obj_xml.indexOf("\"", start_idx + name_attr.size());
			constr_name = obj_xml.mid(start_idx, end_idx - start_idx).remove(name_attr) + "_ck";
		}
		else
		{
			constr_name = match.capturedTexts().at(0);
			constr_name.remove(QString("%1=\"").arg(Attributes::Constraint));
			constr_name.remove(constr_name.length() - 1, 1);
			obj_xml.remove(QRegularExpression(AttributeExpr.arg(Attributes::Constraint)));
		}

		start_idx = obj_xml.indexOf(TagExpr.arg(Attributes::Expression));
		obj_xml.insert(start_idx, QString("\n\t<constraint name=\"%1\" type=\"check\">\n\t\t").arg(constr_name));

		end_idx = obj_xml.indexOf(EndTagExpr.arg(Attributes::Expression));
		obj_xml.insert(end_idx + EndTagExpr.arg(Attributes::Expression).length() + 1, "\n\t</constraint>\n");
	}

	//Replace the deprecated attribute hide-ext-attribs="false|true" from <table> and <views> by collapse-mode="0|1"
	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Table))) ||
		 obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::View))))
	{
		obj_xml.replace(QString("%1=\"false\"").arg(Attributes::HideExtAttribs), QString("%1=\"0\"").arg(Attributes::CollapseMode));
		obj_xml.replace(QString("%1=\"true\"").arg(Attributes::HideExtAttribs), QString("%1=\"1\"").arg(Attributes::CollapseMode));
	}

	/* Fixing extension attributes.
	 * If it contains handles-type="true" then it will be added <object name="extension-name" type="usertype"
	 * If it contains <type name="foo" then it will be replaced by <object type="usertype" name="foo" */
	static QRegularExpression handle_type_rx = QRegularExpression(AttributeExpr.arg("handles-type"));
	bool is_handle_type = obj_xml.contains(handle_type_rx),
			is_multi_types = obj_xml.contains(TagExpr.arg(Attributes::Type));

	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Extension))) &&
		 (is_handle_type || is_multi_types))
	{
		if(is_handle_type)
		{
			int end_sch_idx = -1, start_idx = -1, end_idx = -1;
			QString name_attr="name=\"", ext_name;

			//Extracting the extension's name
			start_idx=obj_xml.indexOf(name_attr);
			end_idx=obj_xml.indexOf("\"", start_idx + name_attr.size());
			ext_name=obj_xml.mid(start_idx, end_idx - start_idx).remove(name_attr);

			obj_xml.remove(handle_type_rx);
			end_sch_idx = obj_xml.indexOf(EndTagExpr.arg(BaseObject::getSchemaName(ObjectType::Extension)));
			obj_xml.insert(end_sch_idx, QString("\n<object type=\"usertype\" name=\"%1\"/>\n").arg(ext_name));
		}
		else
		{
			obj_xml.replace("<type", "<object type=\"usertype\"");
		}
	}

	//Remove the usage of IN keyword in functions' signatures since it is the default if absent
	QRegularExpression regexp = QRegularExpression(AttributeExpr.arg("signature"));
	QRegularExpressionMatch match;
	int sig_idx = -1,	len = 0;
	QString signature, in_keyw = "IN ";

	match = regexp.match(obj_xml);
	sig_idx = match.capturedStart();

	while(sig_idx >= 0)
	{
		signature = obj_xml.mid(sig_idx,  match.capturedLength());
		len = signature.length();

		if(!signature.contains(in_keyw))
		{
			match = regexp.match(obj_xml, sig_idx + len);
			sig_idx = match.capturedStart();
			continue;
		}

		signature.remove(in_keyw);
		obj_xml.remove(sig_idx, len);
		obj_xml.insert(sig_idx, signature);

		match = regexp.match(obj_xml, sig_idx + len);
		sig_idx = match.capturedStart();
	}

	//Renaming the function's behavior-type value STRICT to RETURNS NULL ON NULL INPUT
	QString btype = QString(Attributes::BehaviorType + "=\"%1\"");
	if(obj_xml.contains(Attributes::BehaviorType))
		obj_xml.replace(btype.arg("STRICT"), btype.arg("RETURNS NULL ON NULL INPUT"));

	//Rename the attribute layer to layers
	static QRegularExpression layer_regexp("(layer)( )*(=)");
	if(obj_xml.contains(QRegularExpression(layer_regexp)))
		obj_xml.replace(layer_regexp, Attributes::Layers + "=");

	//Fix the references to op. classes and families if needed
	fixOpClassesFamiliesReferences(obj_xml);
}

void PgModelerCliApp::fixOpClassesFamiliesReferences(QString &obj_xml)
{
	ObjectType ref_obj_type;
	static QRegularExpression contr_typ_rx {
		QString("(%1)(.)+(type=)(\")(%2)(\")")
		.arg(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Constraint)), Attributes::ExConstr)
	};

	if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::Index))) ||
			obj_xml.contains(contr_typ_rx))
		ref_obj_type=ObjectType::OpClass;
	else if(obj_xml.contains(TagExpr.arg(BaseObject::getSchemaName(ObjectType::OpClass))))
		ref_obj_type=ObjectType::OpFamily;
	else
		return;

	QString ref_obj_name=BaseObject::getSchemaName(ref_obj_type);
	if(!obj_xml.contains(TagExpr.arg(ref_obj_name)))
		return;

	QString obj_name, aux_obj_name, signature=("%1 USING %2");

	static QRegularExpression sign_regexp { AttributeExpr.arg("signature") },
	sign_val_regexp { "(signature)( )*(=)" };

	QRegularExpressionMatch match;
	QStringList index_types;
	int pos=0;

	obj_xml.replace(TagExpr.arg(ref_obj_name) + " name=",
					TagExpr.arg(ref_obj_name) +" signature=");

	index_types = IndexingType::getTypes();

	do
	{
		match = sign_regexp.match(obj_xml, pos);
		pos = match.capturedStart();

		if(pos >= 0)
		{
			//Extracting the signature attribute
			obj_name=obj_xml.mid(pos, match.capturedLength());

			//Removing useless portions signature=" in order to retrive only the object's name
			obj_name.remove(sign_val_regexp);
			obj_name.remove('"');

			//Transforming xml entity for quote into the char
			obj_name.replace(UtilsNs::EntityQuot, "\"");

			for(auto &idx_type : index_types)
			{
				aux_obj_name=signature.arg(obj_name, idx_type);

				if(input_model->getObjectIndex(aux_obj_name, ref_obj_type) >= 0)
				{
					//Replacing the old signature with the corrected form
					aux_obj_name.replace("\"", UtilsNs::EntityQuot);
					obj_xml.replace(pos, match.capturedLength(), QString("signature=\"%1\"").arg(aux_obj_name));
					break;
				}
			}

			pos += match.capturedLength();
		}
	}
	while(pos >= 0);
}

void PgModelerCliApp::fixModel()
{
	printMessage(tr("Starting model fix operation..."));
	printMessage(tr("Loading input file: %1").arg(parsed_opts[Input]));
	printMessage(tr("Fixed model file: %1").arg(parsed_opts[Output]));

	QString fix_log = GlobalAttributes::getTemporaryFilePath(ModelFixLog);
	QFile::remove(fix_log);
	extractObjectXML();
	recreateObjects();

	printMessage(tr("Updating relationships..."));

	// Forcing a full relationship revalidation so the special objects can be created properly
	if(input_model->getObjectCount(ObjectType::Relationship) > 0)
	{
		input_model->storeSpecialObjectsXML();
		input_model->disconnectRelationships();
		input_model->validateRelationships();
	}

	input_model->updateTablesFKRelationships();

	printMessage(tr("Saving fixed model..."));
	input_model->saveModel(parsed_opts[Output], SchemaParser::XmlCode);

	if(!has_fix_log)
		printMessage(tr("Model successfully fixed!"));
	else
	{
		printMessage(tr("Model fixed with some errors!"));
		printMessage(tr("Errors logged in file: %1").arg(fix_log));
	}
}

void PgModelerCliApp::loadModel()
{
	//Create the systems objects on model before loading it
	input_model->createSystemObjects(false);

	//Load the model file
	input_model->loadModel(parsed_opts[Input]);

	/* The scene object is created only when some options are used
	 * so we need to check it if is not null to avoid segfaults */
	if(scene)
	{
		scene->blockSignals(true);

		scene->addLayers(input_model->getLayers(), false);
		scene->setActiveLayers(input_model->getActiveLayers());
		scene->setLayerColors(ObjectsScene::LayerNameColor, input_model->getLayerNameColors());
		scene->setLayerColors(ObjectsScene::LayerRectColor, input_model->getLayerRectColors());
		scene->setLayerNamesVisible(input_model->isLayerNamesVisible());
		scene->setLayerRectsVisible(input_model->isLayerRectsVisible());

		scene->adjustSceneRect(true);
		input_model->setObjectsModified({ ObjectType::Schema });

		scene->blockSignals(false);
	}
}

void PgModelerCliApp::exportModel()
{
	printMessage(tr("Starting model export operation..."));
	printMessage(tr("Loading input file: %1").arg(parsed_opts[Input]));

	loadModel();

	//Export to PNG
	if(parsed_opts.count(ExportToPng))
	{
		printMessage(tr("Exporting to PNG image: %1").arg(parsed_opts[Output]));

		export_hlp->exportToPNG(scene, parsed_opts[Output], zoom,
								 parsed_opts.count(ShowGrid) > 0,
								 parsed_opts.count(ShowDelimiters) > 0,
								 parsed_opts.count(PageByPage) > 0,
								 parsed_opts.count(OverrideBgColor) > 0);
	}
	//Export to SVG
	else if(parsed_opts.count(ExportToSvg))
	{
		printMessage(tr("Exporting to SVG file: %1").arg(parsed_opts[Output]));

		export_hlp->exportToSVG(scene, parsed_opts[Output],
													 parsed_opts.count(ShowGrid) > 0,
													 parsed_opts.count(ShowDelimiters) > 0);
	}
	//Export to SQL file
	else if(parsed_opts.count(ExportToFile))
	{
		DatabaseModel::CodeGenMode code_gen_option = DatabaseModel::OriginalSql;

		if(parsed_opts.count(DependenciesSql))
			code_gen_option = DatabaseModel::DependenciesSql;
		else if(parsed_opts.count(ChildrenSql))
			code_gen_option = DatabaseModel::ChildrenSql;
		else if(parsed_opts.count(GroupByType))
			code_gen_option = DatabaseModel::GroupByType;

		if(!parsed_opts.count(Split))
			printMessage(tr("Exporting to SQL script file: %1").arg(parsed_opts[Output]));
		else
			printMessage(tr("Exporting to output directory: %1").arg(parsed_opts[Output]));

		export_hlp->exportToSQL(input_model, parsed_opts[Output], parsed_opts[PgSqlVer],
														parsed_opts.count(Split) > 0,
														code_gen_option,
														parsed_opts.count(GenDropScript) > 0);
	}
	//Export data dictionary
	else if(parsed_opts.count(ExportToDict))
	{
		printMessage(tr("Exporting to data dictionary: %1").arg(parsed_opts[Output]));
		export_hlp->exportToDataDict(input_model, parsed_opts[Output],
																 parsed_opts.count(NoIndex) == 0,
																 parsed_opts.count(Split) > 0,
																 parsed_opts.count(Markdown) > 0);
	}
	//Export to DBMS
	else
	{
		printMessage(tr("Exporting to DBMS: %1").arg(connection.getConnectionString().replace(PasswordRegExp, PasswordPlaceholder)));

		if(parsed_opts.count(IgnoreErrorCodes))
			export_hlp->setIgnoredErrors(parsed_opts[IgnoreErrorCodes].split(','));

		export_hlp->exportToDBMS(input_model, connection, parsed_opts[PgSqlVer],
								parsed_opts.count(IgnoreDuplicates),
								parsed_opts.count(DropDatabase),
								parsed_opts.count(DropObjects),
								parsed_opts.count(Simulate),
								parsed_opts.count(UseTmpNames),
								parsed_opts.count(Force),
								!parsed_opts.count(Simulate) && !parsed_opts.count(NonTransactional));
	}

	printMessage(tr("Export operation completed successfully!\n"));
}

void PgModelerCliApp::importDatabase()
{
	printMessage(tr("Starting database import operation..."));
	printMessage(tr("Source database: %1").arg(connection.getConnectionId(true, true)));

	ModelWidget *model_wgt = new ModelWidget;

	importDatabase(model_wgt->getDatabaseModel(), connection);
	model_wgt->rearrangeSchemasInGrid();

	printMessage(tr("Saving imported database to file..."));

	model_wgt->getDatabaseModel()->saveModel(parsed_opts[Output], SchemaParser::XmlCode);

	printMessage(tr("Import operation completed successfully!\n"));

	delete model_wgt;
}

void PgModelerCliApp::importDatabase(DatabaseModel *model, Connection conn)
{
	try
	{
		std::map<ObjectType, std::vector<unsigned>> obj_oids;
		std::map<unsigned, std::vector<unsigned>> col_oids;
		Catalog catalog;
		QString db_oid;
		QStringList force_tab_objs;
		bool imp_sys_objs = (parsed_opts.count(ImportSystemObjs) > 0),
				imp_ext_objs = (parsed_opts.count(ImportExtensionObjs) > 0);

		if(parsed_opts[ForceChildren] == AllChildren)
		{
			for(auto &type : BaseObject::getChildObjectTypes(ObjectType::Table))
			{
				if(type == ObjectType::Column)
					continue;

				force_tab_objs.append(BaseObject::getSchemaName(type));
			}
		}
		else
			force_tab_objs = parsed_opts[ForceChildren].split(',', Qt::SkipEmptyParts);

		Connection::setPrintSQL(parsed_opts.count(DebugMode) > 0);

		catalog.setConnection(conn);

		catalog.setQueryFilter(Catalog::ListAllObjects | Catalog::ExclBuiltinArrayTypes |
													 Catalog::ExclExtensionObjs | Catalog::ExclSystemObjs);

		catalog.setObjectFilters(obj_filters, parsed_opts.count(OnlyMatching) > 0,
														 parsed_opts.count(MatchByName) == 0, force_tab_objs);

		catalog.getObjectsOIDs(obj_oids, col_oids, {{Attributes::FilterTableTypes, Attributes::True}});

		db_oid = catalog.getObjectOID(conn.getConnectionParam(Connection::ParamDbName), ObjectType::Database);
		obj_oids[ObjectType::Database].push_back(db_oid.toUInt());
		catalog.closeConnection();

		import_hlp->setConnection(conn);
		import_hlp->setImportOptions(imp_sys_objs,
																 imp_ext_objs,
																 true,
																 parsed_opts.count(IgnoreImportErrors) > 0,
																 parsed_opts.count(DebugMode) > 0,
																 !parsed_opts.count(Diff),
																 !parsed_opts.count(Diff),
																 parsed_opts.count(CommentsAsAliases) > 0);

		model->createSystemObjects(true);
		import_hlp->setSelectedOIDs(model, obj_oids, col_oids);
		import_hlp->importDatabase();
		import_hlp->closeConnection();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PgModelerCliApp::diffModels()
{
	DatabaseModel *compared_model = new DatabaseModel();
	QString dbname;
	std::vector<BaseObject *> filtered_objs;
	bool is_compare_db = parsed_opts.count(CompareDb);

	try
	{
		printMessage(tr("Starting diff operation..."));

		if(!parsed_opts[Input].isEmpty())
			printMessage(tr("Source model: %1").arg(parsed_opts[Input]));
		else
			printMessage(tr("Source database: %1").arg(connection.getConnectionId(true, true)));

		if(is_compare_db)
		{
			dbname = extra_connection.getConnectionId(true, true);
			printMessage(tr("Target database: %1").arg(dbname));
		}
		else
			printMessage(tr("Target model: %1").arg(parsed_opts[CompareFile]));

		if(!parsed_opts[Input].isEmpty())
		{
			printMessage(tr("Loading source model..."));
			loadModel();

			if(parsed_opts.count(PartialDiff))
			{
				QString search_attr = parsed_opts.count(MatchByName) ? Attributes::Name : Attributes::Signature;

				// Filtering by modification date always forces the signature matching
				if(start_date.isValid() || end_date.isValid())
					obj_filters.append(input_model->getFiltersFromChangelog(start_date, end_date));

				filtered_objs = input_model->findObjects(obj_filters, search_attr);

				/* We need to finish the diff if no object was found based on the filters
				 * this will avoid the diff between an empty database model and a full database model
				 * which may produce unexpected results like try to recreate all objects from the database
				 * model that contains objects */
				if(filtered_objs.empty())
				{
					printMessage(tr("No objects were retrieved using the provided filter(s)."));

					if(!parsed_opts.count(Force))
					{
						printMessage(tr("Use the option `%1' to force a full diff in this case.").arg(Force));
						printMessage(tr("The diff operation will not continue!\n"));
						return;
					}
					
					printMessage(tr("Switching to full diff operation..."));
				}
				else
				{
					/* Special case: when performing a partial diff between a model and a database
					 * and in the set of filtered model objects we have one or more many-to-many, inheritance or partitioning
					 * relationships we need to inject filters to force the retrieval of the all involved tables in those relationships
					 * from the destination database,this way we avoid the diff try to create everytime all tables
					 * in the those relationships. */
					obj_filters.append(ModelsDiffHelper::getRelationshipFilters(filtered_objs, search_attr == Attributes::Signature));
				}
			}
		}
		else
		{
			printMessage(tr("Importing database `%1'...").arg(connection.getConnectionId(true, true)));
			importDatabase(input_model, connection);
		}

		// Importing the compared database
		if(parsed_opts.count(CompareDb))
		{
			printMessage(tr("Importing database `%1'...").arg(dbname));
			importDatabase(compared_model, extra_connection);
		}
		// Otherwise we load the model from file (param CompareFile)
		else
		{
			printMessage(tr("Loading target model..."));
			compared_model->createSystemObjects(false);
			compared_model->loadModel(parsed_opts[CompareFile]);
		}

		diff_hlp->setModels(input_model, compared_model);
		diff_hlp->setFilteredObjects(filtered_objs);
		diff_hlp->setDiffOption(ModelsDiffHelper::OptKeepClusterObjs, !parsed_opts.count(DropClusterObjs));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptCascadeMode, !parsed_opts.count(NoCascadeDrop));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptRecreateUnmodifiable, parsed_opts.count(RecreateUnmod));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptReplaceModified, parsed_opts.count(ReplaceModified));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptKeepObjectPerms, !parsed_opts.count(RevokePermissions));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptReuseSequences, !parsed_opts.count(NoSequenceReuse));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptPreserveDbName, !parsed_opts.count(RenameDb));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptDontDropMissingObjs, !parsed_opts.count(DropMissingObjs));
		diff_hlp->setDiffOption(ModelsDiffHelper::OptDropMissingColsConstr, parsed_opts.count(ForceDropColsConstrs));

		diff_hlp->setForcedRecreateTypeNames(parsed_opts[ForceReCreateObjs].split(',', Qt::SkipEmptyParts));

		if(!parsed_opts[PgSqlVer].isEmpty())
			diff_hlp->setPgSQLVersion(parsed_opts[PgSqlVer]);
		else if(is_compare_db)
		{
			extra_connection.connect();
			diff_hlp->setPgSQLVersion(extra_connection.getPgSQLVersion(true));
			extra_connection.close();
		}

		printMessage(tr("Comparing the models..."));
		diff_hlp->diffModels();

		if(diff_hlp->getDiffDefinition().isEmpty())
			printMessage(tr("No differences detected."));
		else
		{
			if(parsed_opts.count(SaveDiff))
			{
				printMessage(tr("Saving diff to file `%1'").arg(parsed_opts[Output]));
				UtilsNs::saveFile(parsed_opts[Output], diff_hlp->getDiffDefinition().toUtf8());
			}
			else
			{
				bool apply_diff = true;

				if(!parsed_opts.count(NoDiffPreview))
				{
					QString res, buff, line;
					QTextStream in(stdin), preview;

					buff += "\n** Press ENTER to scroll the preview **\n";
					buff += "\n### DIFF PREVIEW ###\n\n";
					buff += diff_hlp->getDiffDefinition();
					buff += "\n### END OF PREVIEW  ###\n\n";

					preview.setString(&buff, QIODevice::ReadOnly);

					while(!preview.atEnd())
					{
						line = preview.readLine();
						res.append(line + '\n');

						if(res.count(QChar('\n')) >= 30 || preview.atEnd())
						{
							out << res;
							out.flush();
							res.clear();

							if(!preview.atEnd())
								in.readLine();
						}
					}

					out << Qt::endl;
					out << tr("** WARNING: You are about to apply the generated diff code to the server. Data loss may occur!") << Qt::endl;

					do
					{
						out << tr("** Proceed with the diff applying? (yes/no) > ");
						out.flush();

						in.skipWhiteSpace();
						res = in.readLine();
					}
					while(res.toLower() != tr("yes") && res.toLower() != tr("no"));

					if(res.toLower() == tr("no"))
					{
						apply_diff = false;
						printMessage(tr("Diff code not applied to server."));
					}
				}

				if(apply_diff)
				{
					printMessage(tr("Applying diff to database `%1'...").arg(dbname));
					export_hlp->setExportToDBMSParams(diff_hlp->getDiffDefinition(),
													 &extra_connection,
													 parsed_opts[CompareDb],
													 parsed_opts.count(IgnoreDuplicates),
													 !parsed_opts.count(NonTransactional));

					if(parsed_opts.count(IgnoreErrorCodes))
						export_hlp->setIgnoredErrors(parsed_opts[IgnoreErrorCodes].split(','));

					export_hlp->exportToDBMS();
				}
			}
		}

		printMessage(tr("Diff operation completed successfully!\n"));
		delete compared_model;
	}
	catch(Exception &e)
	{
		delete compared_model;
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PgModelerCliApp::updateMimeType()
{
#ifndef Q_OS_MACOS
	try
	{
		printMessage(tr("Starting MIME type update operation..."));

		handleMimeDatabase(parsed_opts[DbmMimeType]==Uninstall, parsed_opts.count(SystemWide) != 0, parsed_opts.count(Force) != 0);

		printMessage(tr("MIME database updated successfully!\n"));
	}
	catch (Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
#endif
}

QStringList PgModelerCliApp::extractForeignKeys(QString &obj_xml)
{
	QStringList constr_lst;
	int start=0, end=0, pos=0, count=0;
	QString start_tag=QString("<%1").arg(Attributes::Constraint),
			end_tag=QString("</%1>").arg(Attributes::Constraint),
			constr;

	do
	{
		start=obj_xml.indexOf(start_tag, pos);
		end=obj_xml.indexOf(end_tag, start);

		if(start > 0 && end > 0)
		{
			count=(end - start) + end_tag.size() + 1;
			constr=obj_xml.mid(start, count);

			if(constr.contains(Attributes::FkConstr))
			{
				obj_xml.remove(start, count);
				constr_lst.push_back(constr);
				pos=0;
			}
			else
				pos=end;
		}
		else
			break;
	}
	while(pos >= 0 && pos < obj_xml.size());

	return constr_lst;
}

bool PgModelerCliApp::containsRelAttributes(const QString &str)
{
	bool found=false;
	static std::vector<QString> attribs={ Attributes::Relationship,
									 Attributes::Type, Attributes::SrcRequired, Attributes::DstRequired,
									 Attributes::SrcTable, Attributes::DstTable,	Attributes::Points,
									 Attributes::Columns,	Attributes::Column, Attributes::Constraint,
									 Attributes::Label, Attributes::Line, Attributes::Position,
									 Attributes::Identifier, Attributes::Deferrable, Attributes::DeferType,
									 Attributes::TableName, Attributes::SpecialPkCols, Attributes::Table,
									 Attributes::AncestorTable, Attributes::CopyOptions, Attributes::CopyMode,
									 Attributes::SrcColPattern, Attributes::DstColPattern, Attributes::PkPattern,
									 Attributes::UqPattern, Attributes::SrcFkPattern, Attributes::DstFkPattern };

	for(unsigned i=0; i < attribs.size() && !found; i++)
		found=str.contains(attribs[i]);

	return found;
}

void PgModelerCliApp::handleMimeDatabase(bool uninstall, bool system_wide, bool force)
{
	printMessage(tr("Mime database operation: %1").arg(uninstall ? "uninstall" : "install"));

	#ifdef Q_OS_LINUX
		handleLinuxMimeDatabase(uninstall, system_wide, force);
	#else
		#ifdef Q_OS_WIN
			handleWindowsMimeDatabase(uninstall, system_wide, force);
		#endif
	#endif
}

void PgModelerCliApp::handleLinuxMimeDatabase(bool uninstall, bool system_wide, bool force)
{
	SchemaParser schparser;
	attribs_map attribs;
	QString str_aux,

			share_path = !system_wide ? QDir::homePath() + "/.local/share" : "/usr/share",

			//Configures the path to the application logo
			exec_icon=GlobalAttributes::getTmplConfigurationFilePath("", "pgmodeler_logo.png"),

			//Configures the path to the document logo
			dbm_icon=GlobalAttributes::getTmplConfigurationFilePath("", "pgmodeler_dbm.png"),

			sch_icon=GlobalAttributes::getTmplConfigurationFilePath("", "pgmodeler_sch.png"),

			//Path to directory that register mime types
			mime_db_dir=QString("%1/mime").arg(share_path),

			//Path to the file that associates apps to mimetypes
			mimeapps=QString("%1/applications/mimeapps.list").arg(share_path);

		//Files generated after update file association (application-dbm.xml and pgModeler.desktop)
		QStringList	files = { QString("%1/applications/pgModeler.desktop").arg(share_path),
													QString("%1/applications/pgModelerSchEditor.desktop").arg(share_path),
													mime_db_dir + "/packages/application-dbm.xml",
													mime_db_dir + "/packages/application-sch.xml"},

			schemas = { GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "desktop" + GlobalAttributes::SchemaExt),
									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "desktop-sch" + GlobalAttributes::SchemaExt),
									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "application-dbm" + GlobalAttributes::SchemaExt),
									GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "application-sch" + GlobalAttributes::SchemaExt) },

			icons = { exec_icon, sch_icon, dbm_icon, sch_icon };

	QByteArray buf, buf_aux;
	QFile out;

	for(unsigned i=0; i < 4; i++)
	{
		//When installing, check if the necessary file exists. If exists, raises an error and abort.
		if(!uninstall && QFileInfo::exists(files[i]) && !force)
			throw Exception(MsgFileAssociated, ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(uninstall && !QFileInfo::exists(files[i]) && !force)
			throw Exception(MsgNoFileAssociation, ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
	}

	if(!uninstall && !system_wide)
		attribs[Attributes::WorkingDir]=QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
	else
		attribs[Attributes::WorkingDir]="";

	try
	{
		for(unsigned i=0; i < 4; i++)
		{
			if(uninstall)
			{
				if(!QFile(files[i]).remove() && !force)
				{
					throw Exception(tr("Cannot delete file %1! Check if the current user has delete permissions and verify the file exists.").arg(files[i]),
													ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
				}
			}
			else
			{
				attribs[Attributes::Application]=(i == 0 ? GlobalAttributes::getPgModelerAppPath() : GlobalAttributes::getPgModelerSchemaEditorPath());
				attribs[Attributes::Icon] = icons[i];

				schparser.loadFile(schemas[i]);
				schparser.ignoreEmptyAttributes(true);
				buf.append(schparser.getSourceCode(attribs).toUtf8());
				QDir(".").mkpath(QFileInfo(files[i]).absolutePath());

				UtilsNs::saveFile(files[i], buf);
				buf.clear();
			}
		}

		out.setFileName(mimeapps);

		//If the file mimeapps.list doesn't exists (generally in Ubuntu) creates a new one
		if(!uninstall && !QFileInfo::exists(mimeapps))
		{
			out.open(QFile::WriteOnly);
			out.write(QByteArray("[Added Associations]\napplication/dbm=pgModeler.desktop;\n"));
			out.write(QByteArray("\n[Default Applications]\napplication/dbm=pgModeler.desktop;\n"));
			out.write(QByteArray("\n[Added Associations]\napplication/sch=pgModelerStxEditor.desktop;\n"));
			out.write(QByteArray("\n[Default Applications]\napplication/sch=pgModelerStxEditor.desktop;\n"));
			out.close();
		}
		else
		{
			out.open(QFile::ReadOnly);

			if(!out.isOpen())
				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(mimeapps),
												ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);

			//Opens the mimeapps.list to add a entry linking pgModeler to .dbm files
			buf=out.readAll();
			out.close();

			QTextStream ts(&buf);
			static QRegularExpression mime_rx { QRegularExpression::wildcardToRegularExpression("application/dbm*") };

			while(!ts.atEnd())
			{
				//Remove any reference to application/dbm mime from file
				str_aux=ts.readLine();
				str_aux.replace(mime_rx, "");

				if(!str_aux.isEmpty())
				{
					//Updates the application/dbm mime association
					if(!uninstall && (str_aux.contains("[Added Associations]") ||
										str_aux.contains("[Default Applications]")))
						str_aux.append("\napplication/dbm=pgModeler.desktop;\n");
					else
						str_aux+="\n";

					if(str_aux.startsWith("[") && !str_aux.contains("Added Associations"))
						str_aux="\n" + str_aux;

					buf_aux.append(str_aux.toUtf8());
				}
			}

			//Write a new copy of the mimeapps.list file
			out.open(QFile::Truncate | QFile::WriteOnly);
			out.write(buf_aux.data(), buf_aux.size());
			out.close();
		}

		//Update the mime database
		printMessage(tr("Executing update-mime-database command..."));

		QProcess::execute("update-mime-database", QStringList { mime_db_dir });
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void PgModelerCliApp::handleWindowsMimeDatabase(bool uninstall, bool system_wide, bool force)
{
	SchemaParser schparser;
	QString base_reg_key = system_wide ? "HKEY_LOCAL_MACHINE\\SOFTWARE" : "HKEY_CURRENT_USER\\Software";

	//Checking if the .dbm registry key exists
	QSettings dbm_ext(QString("%1\\Classes\\%2").arg(base_reg_key, GlobalAttributes::DbModelExt), QSettings::NativeFormat),
				sch_ext(QString("%1\\Classes\\.sch").arg(base_reg_key), QSettings::NativeFormat);
	QString exe_path=QDir::toNativeSeparators(GlobalAttributes::getPgModelerAppPath()),
			sc_exe_path=QDir::toNativeSeparators(GlobalAttributes::getPgModelerSchemaEditorPath());

	//If there is no value assigned to (.dbm | .sch)/Default key and the user wants to uninstall file association, raises an error
	if(uninstall && !force &&
		 (dbm_ext.value("Default").toString().isEmpty() ||
			sch_ext.value("Default").toString().isEmpty()))
		throw Exception(MsgNoFileAssociation, ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!uninstall && !force &&
		 (!dbm_ext.value("Default").toString().isEmpty() ||
			!sch_ext.value("Default").toString().isEmpty()))
		throw Exception(MsgFileAssociated, ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(!uninstall)
	{
		//Write the default value for .dbm registry key
		dbm_ext.setValue("Default", "dbm_auto_file");
		sch_ext.setValue("Default", "sch_auto_file");
	}
	else
	{
		dbm_ext.remove("");
		sch_ext.remove("");
	}

	dbm_ext.sync();
	sch_ext.sync();

	//Other registry keys values
	std::map<QString, QStringList> confs = {
				{ QString("\\%1\\Classes\\dbm_auto_file").arg(base_reg_key), { "FriendlyTypeName" , "pgModeler Database Model" } },
				{ QString("\\%1\\Classes\\dbm_auto_file\\DefaultIcon").arg(base_reg_key), { "Default" , QString("%1,1").arg(exe_path) } },
				{ QString("\\%1\\Classes\\dbm_auto_file\\shell\\open\\command").arg(base_reg_key), { "Default", QString("\"%1\" \"%2\"").arg(exe_path, "%1") } },
				{ QString("\\%1\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%2").arg(base_reg_key, GlobalAttributes::DbModelExt), { "OpenWithList/a", "pgmodeler.exe", "OpenWithList/MRUList", "a"} },
				{ QString("\\%1\\Classes\\sch_auto_file").arg(base_reg_key), { "FriendlyTypeName" , "pgModeler Schema File" } },
				{ QString("\\%1\\Classes\\sch_auto_file\\DefaultIcon").arg(base_reg_key), { "Default" , QString("%1,1").arg(sc_exe_path) } },
				{ QString("\\%1\\Classes\\sch_auto_file\\shell\\open\\command").arg(base_reg_key), { "Default" , QString("\"%1\" \"%2\"").arg(sc_exe_path, "%1") } },
				{ QString("\\%1\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.sch").arg(base_reg_key), { "OpenWithList/a", "pgmodeler-sc.exe", "OpenWithList/MRUList", "a"} }
	};

	std::map<QString, QStringList>::iterator itr;
	itr=confs.begin();

	//Iterates over the configuration map writing the other keys on registry
	while(itr!=confs.end())
	{
		QSettings s(itr->first, QSettings::NativeFormat);

		if(uninstall)
			s.remove("");
		else
		{
			for(int i=0; i < itr->second.size(); i+=2)
				s.setValue(itr->second[i], itr->second[i+1]);
		}

		s.sync();
		itr++;
	}
}

void PgModelerCliApp::createConfigurations()
{
	QString conf_dir = GlobalAttributes::getConfigurationsPath();

	printMessage(tr("Creating configuration files..."));
	printMessage(tr("Destination path: %1").arg(conf_dir));

	bool missing_only = parsed_opts.count(MissingOnly) > 0,
			force = parsed_opts.count(Force) > 0;

	if(!missing_only && !force && QDir(GlobalAttributes::getConfigurationsPath()).exists())
		throw Exception(tr("Configuration files already exist!"), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);

	try
	{
		if(force)
		{
			QDir dir;
			QString bkp_conf_dir = conf_dir + QDateTime::currentDateTime().toString("_yyyyMMdd_hhmmss");

			printMessage(tr("Configuration files already exist! Creating backup..."));
			printMessage(tr("Backup path: %1").arg(bkp_conf_dir));

			if(!dir.rename(conf_dir, bkp_conf_dir))
				throw Exception(tr("Failed to create backup of configuration files!").arg(bkp_conf_dir), ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		createUserConfiguration();
		printMessage(tr("Configuration files created successfully!\n"));
	}
	catch (Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void PgModelerCliApp::loadPlugins()
{
	std::vector<Exception> errors;
	QString lib,
			dir_plugins = GlobalAttributes::getPluginsPath() +
										GlobalAttributes::DirSeparator;
	QPluginLoader plugin_loader;
	QStringList dir_list;
	PgModelerCliPlugin *plugin = nullptr;
	QFileInfo fi;
	attribs_map p_short_opts;
	std::map<QString, bool> p_long_opts;
	QJsonObject metadata;

	//The plugin loader must resolve all symbols otherwise return an error if some symbol is missing on library
	plugin_loader.setLoadHints(QLibrary::ResolveAllSymbolsHint);

	/* Configures an QDir instance to list only directories on the plugins/ subdir.
		If the user does not put the plugin in it's directory the file is ignored  */
	dir_list = QDir(dir_plugins, "*", QDir::Name, QDir::AllDirs | QDir::NoDotAndDotDot).entryList();

	/* Removing the "schemas" dir from the list since it is reserved to shared schema files
	 * for configuration file generation */
	dir_list.removeAll(GlobalAttributes::SchemasDir);

	for(auto &plugin_name : dir_list)
	{
		/* Configures the basic path to the library on the form:
		 [PLUGINS ROOT DIR]/[PLUGIN NAME]/lib[PLUGIN NAME].[EXTENSION] */
		#ifdef Q_OS_WIN
				lib = dir_plugins + plugin_name +
							GlobalAttributes::DirSeparator  +
							plugin_name + ".dll";
		#else
			#ifdef Q_OS_MACOS
					lib = dir_plugins + plugin_name +
								GlobalAttributes::DirSeparator  +
								"lib" + plugin_name + ".dylib";
			#else
					lib = dir_plugins + plugin_name +
								GlobalAttributes::DirSeparator  +
								"lib" + plugin_name + ".so";
			#endif
		#endif

		//Try to load the library
		plugin_loader.setFileName(lib);
		metadata = plugin_loader.metaData();

		/* Ignores the plugin if it doesn't implement the correct interface,
		 * in this case PgModelerCliPlugin */
		if(metadata["IID"] != "PgModelerCliPlugin")
			continue;

		plugin = qobject_cast<PgModelerCliPlugin *>(plugin_loader.instance());

		if(plugin)
		{
			if(!isPluginOptsValid(plugin))
			{
				errors.push_back(Exception(Exception::getErrorMessage(ErrorCode::PluginNotLoaded)
																	 .arg(plugin_name, lib, tr("The plugin contains a list of options that are malformed or conflict with pgmodeler-cli default options!")),
																	 ErrorCode::PluginNotLoaded, PGM_FUNC,PGM_FILE,PGM_LINE,
																	 nullptr, tr("Plugin ID: %1").arg(plugin_name)));
			}
			else if(plugin->getOperationId() != PgModelerCliPlugin::CustomCliOp &&
							!plugin->getOpModeOptions().isEmpty())
			{
				errors.push_back(Exception(Exception::getErrorMessage(ErrorCode::PluginNotLoaded)
																	 .arg(plugin_name, lib, tr("The plugin does not implement a custom CLI operation but has operation modes defined!")),
																	 ErrorCode::PluginNotLoaded, PGM_FUNC,PGM_FILE,PGM_LINE,
																	 nullptr, tr("Plugin ID: %1").arg(plugin_name)));
			}
			else if(plugin->getOperationId() == PgModelerCliPlugin::CustomCliOp &&
							plugin->getOpModeOptions().isEmpty())
			{
				errors.push_back(Exception(Exception::getErrorMessage(ErrorCode::PluginNotLoaded)
																	 .arg(plugin_name, lib, tr("The plugin implements a custom CLI operation but does not specify operation modes!")),
																	 ErrorCode::PluginNotLoaded, PGM_FUNC,PGM_FILE,PGM_LINE,
																	 nullptr, tr("Plugin ID: %1").arg(plugin_name)));
			}
			else
			{
				fi.setFile(lib);
				plugin->initPlugin(this);
				plugin->setLibraryName(fi.fileName());
				plugin->setPluginName(plugin_name);
				plugins.push_back(plugin);

				p_short_opts = plugin->getShortOptions();
				p_long_opts = plugin->getLongOptions();

				short_opts.insert(p_short_opts.begin(), p_short_opts.end());
				long_opts.insert(p_long_opts.begin(), p_long_opts.end());
			}
		}
		else
		{
			errors.push_back(Exception(Exception::getErrorMessage(ErrorCode::PluginNotLoaded)
																 .arg(plugin_name, lib, plugin_loader.errorString()),
																	 ErrorCode::PluginNotLoaded, PGM_FUNC,PGM_FILE,PGM_LINE,
																	 nullptr, tr("Plugin ID: %1").arg(plugin_name)));
		}
	}

	if(!errors.empty())
	{
		plugin_load_errors = Exception(Exception::getErrorMessage(ErrorCode::PluginsNotLoaded) + " " +
																	 tr("HINT: Use the option `%1' to ignore faulty plugin errors.").arg(IgnoreFaultyPlugins),
																	 ErrorCode::PluginsNotLoaded,
																	 PGM_FUNC, PGM_FILE, PGM_LINE, errors).getExceptionsText();
	}
}

void PgModelerCliApp::listPlugins()
{
	if(plugins.empty())
	{
		printText(tr("No plugins are loaded."));
		printText();
	}
	else
	{
		printText(tr("Available plugins:"));
		printText();

		for(auto &plugin : plugins)
		{
			printText(QString("* Id     : %1").arg(plugin->getPluginName()));
			printText(QString("* Name   : %1").arg(plugin->getPluginTitle()));
			printText(QString("* Version: %1").arg(plugin->getPluginVersion()));
			printText(QString("* Library: %1").arg(plugin->getLibraryName()));
			printText(QString("* Author : %1").arg(plugin->getPluginAuthor()));
			printText(QString("* %1").arg(plugin->getPluginDescription()));
			printText();
		}
	}

	if(!plugin_load_errors.isEmpty() && !parsed_opts.count(IgnoreFaultyPlugins))
	{
		printText("** Plugin loading errors:");
		printText();
		printText(plugin_load_errors);
	}
}

bool PgModelerCliApp::isPluginOptsValid(PgModelerCliPlugin *plugin)
{
	QString opt;
	static QRegularExpression s_opt_rx("^\\-(\\w){1,3}$"),
			l_opt_rx("^\\-\\-(\\w|\\-)+$");
	attribs_map s_opts = plugin->getShortOptions();
	std::map<QString, bool> l_opts = plugin->getLongOptions();
	QStringList op_mode_opts = plugin->getOpModeOptions();

	/* Short options must start with an dash (-) and
	 * contain no more than three alphanumeric characters */
	for(auto &[_, s_op] : s_opts)
	{
		opt = s_op.trimmed();

		if(!opt.contains(s_opt_rx))
			return false;

		// Validates if the option conflicts with the original CLI list of short options
		for(auto &[aux_s_op, _] : short_opts)
		{
			if(aux_s_op == opt)
				return false;
		}
	}

	/* Long options must start with two dashes (--) and
	 * contain a free number of alphanumeric characters */
	for(auto &[l_op, _] : l_opts)
	{
		opt = l_op.trimmed();

		if(!opt.contains(l_opt_rx))
			return false;

		// Validates if the option conflicts with the original CLI list of long options
		if(long_opts.count(opt))
			return false;
	}

	/* Validate if the long options keys are compatible with
	 * short options keys. */
	for(auto &[l_op, _] : l_opts)
	{
		if(!s_opts.count(l_op))
			return false;
	}

	/* Validate if the options used to trigger the main plugin operation
	 * exists in the short options */
	for(auto &opt : op_mode_opts)
	{
		if(!s_opts.count(opt))
			return false;
	}

	return true;
}

int PgModelerCliApp::definePluginsExecOrder(const attribs_map &opts)
{
	int plug_op_modes = 0;
	PgModelerCliPlugin::OperationId op_id;
	QString acc_op_key;
	bool is_op_mode = false;

	QStringList valid_opts, export_opts = {
		ExportToFile, ExportToPng, ExportToSvg,
		ExportToDbms, ExportToDict
	};

	std::map<PgModelerCliPlugin::OperationId, QString> cli_op = {
		{ PgModelerCliPlugin::ExportToFile, ExportToFile},
		{ PgModelerCliPlugin::ExportToPng, ExportToPng },
		{ PgModelerCliPlugin::ExportToSvg, ExportToSvg },
		{ PgModelerCliPlugin::ExportToDbms, ExportToDbms },
		{ PgModelerCliPlugin::ExportToDict, ExportToDict },
		{ PgModelerCliPlugin::ImportDb, ImportDb },
		{ PgModelerCliPlugin::Diff, Diff },
		{ PgModelerCliPlugin::FixModel, FixModel },
		{ PgModelerCliPlugin::CreateConfigs, CreateConfigs }
	};

	for(auto &[opt, _] : opts)
	{
		for(auto &plugin : plugins)
		{
			valid_opts = plugin->getValidOptions();
			is_op_mode = plugin->isOpModeOption(opt);

			if(!valid_opts.contains(opt) ||
				 !is_op_mode ||
				 plug_exec_order.contains(plugin))
				continue;

			plug_exec_order.append(plugin);
			op_id = plugin->getOperationId();

			if(op_id == PgModelerCliPlugin::CustomCliOp &&
				 is_op_mode && !accepted_opts.count(opt))
			{
				acc_op_key = opt;
				plug_op_modes++;
			}
			else if(op_id == PgModelerCliPlugin::Export && export_opts.contains(opt))
				acc_op_key = opt;
			else if(cli_op.count(op_id))
				acc_op_key = cli_op[op_id];

			accepted_opts[acc_op_key].append(valid_opts);
			accepted_opts[acc_op_key].append(IgnoreFaultyPlugins);
		}
	}

	return plug_op_modes;
}

void PgModelerCliApp::runPluginsPreOperations()
{
	for(auto &plugin : plug_exec_order)
	{
		try
		{
			plugin->runPreOperation();
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e,
											tr("Plug-in id: %1").arg(plugin->getPluginName()));
		}
	}
}

void PgModelerCliApp::runPluginsOperations()
{
	for(auto &plugin : plug_exec_order)
	{
		try
		{
			plugin->runOperation();
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e,
											tr("Plug-in id: %1").arg(plugin->getPluginName()));
		}
	}
}

void PgModelerCliApp::runPluginsPostOperations()
{
	for(auto &plugin : plug_exec_order)
	{
		try
		{
			plugin->runPostOperation();
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e,
											tr("Plug-in id: %1").arg(plugin->getPluginName()));
		}
	}
}
