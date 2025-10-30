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

/**
\ingroup libgui
\class ModelExportHelper
\brief Implements the base operations to export models (forward engineering)
*/

#ifndef MODEL_EXPORT_HELPER_H
#define MODEL_EXPORT_HELPER_H

#include "guiglobal.h"
#include <QGraphicsView>
#include "objectsscene.h"
#include "databasemodel.h"
#include "connection.h"

class __libgui ModelExportHelper: public QObject {
	Q_OBJECT

	private:
		//! \brief  Stores the total progress
		int progress,

		//! \brief  Stores the sql generation progress
		sql_gen_progress;

		/*! \brief Indicates if the database was created on the server (only dbms export).
		This attribute is used to drop the database from server */
		bool db_created;

		//! \brief PostgreSQL version used by the exporter (only in thread mode)
		QString pgsql_ver;

		//! \brief The SQL code generation mode. (only in thread mode)
		DatabaseModel::CodeGenMode code_gen_mode;

		//! \brief Indicates to the exporter to ignore object duplicity (only in thread mode)
		bool ignore_dup,

		//! \brief Indicates to the exporter to run as a simulator[model validation] (only in thread mode)
		simulate,

		//! \brief Indicates to the exporter to drop database before export
		drop_db,

		//! \brief Indicates to the exporter to drop the objects that are enabled and have a DROP command attached (see SQL source for objects)
		drop_objs,

		//! \brief Indicates to the exporter to generate random names for database, roles and tablespaces before export (only in thread mode)
		use_tmp_names,

		//! \brief Indicates if the exporting thread was canceled by the user (only in thread mode)
		export_canceled,

		//! \brief Indicates if the SQL code of the database model was reenabled to avoid exporting errors
		db_sql_reenabled,

		//! \brief Indicates if the grid should be displayed during exporting to graphical format
		show_grid,

		//! \brief Indicates if the page delimiters should be displayed during exporting to graphical format
		show_delim,

		//! \brief Indicates if the original background (canvas) color should be used in the graphics file
		override_bg_color,

		//! \brief Indicates if the export to png should be done pageby page
		page_by_page,

		//! \brief Indicates if the data dictionary or sql export should be split into separated files
		split,

		//! \brief Indicates if the data dictionary should be browsable (include an index)
		browsable,

		//! \brief Changes the data dictionary output format from HTML to MD (Markdown)
		md_format,

		//! \brief Indicates if the database must be dropped before the export
		force_db_drop,

		//! \brief Indicates if the script containing DROP commandos of all objects must be created
		gen_drop_file,

		/*! \brief Indicates if the export to DBMS must be run inside a transaction block. This option
				has no effect when creating the database itself as well tablespaces */
		transactional;

		//! \brief Database model used as reference on export operation (only in thread mode)
		DatabaseModel *db_model;

		//! \brief Database connection used to export data to DBMS (only in thread mode)
		Connection *connection;

		QString sql_buffer, db_name;

		//! \brief List of ignored error codes
		QStringList ignored_errors;

		std::vector<Exception> errors;

		/*! \brief Indicates which role / tablespaces were created on server (only dbms export).
		This attribute is used to drop the created roles / tablespaces from server */
		std::map<ObjectType, int> created_objs;

		//! \brief Stores the current state of ALTER command generation for table columns/constraints
		std::map<PhysicalTable *, bool> alter_cmds_status;

		//! \brief Stores the original object names before the call of generateRandomObjectNames()
		std::map<BaseObject *, QString> orig_obj_names;

		ObjectsScene *scene;

		QGraphicsView *viewp;

		QString filename;

		double zoom;

		//! \brief Saves the current state of ALTER command generaton for table columns/constraints
		void saveGenAtlerCmdsStatus(DatabaseModel *db_model);

		//! \brief Retores the previous ALTER command generation state for table columns/constraints
		void restoreGenAtlerCmdsStatus();

		//! \brief Revert the dbms export process, removing the created database, roles and tablespaces
		void undoDBMSExport(DatabaseModel *db_model, Connection &conn, bool use_tmp_names);

		/*! \brief Cause the names of the database, roles and tablespaces to be replaced by a temporary name in order
		to avoid duplicity error when exporting. This feature is only useful when validating the model against a
		server which some of the objects (at cluster level) still exists */
		void generateTempObjectNames(DatabaseModel *db_model);

		//! \brief Restore the original name of the database, roles and tablespaces
		void restoreObjectNames();

		//! \brief Exports the contents of the buffer to a previously opened connection
		void exportBufferToDBMS(const QString &buffer, Connection &conn, bool drop_objs=false, bool transactional = false);

		//! \brief Returns if the error code is one of the treated by the export process as object duplication error
		bool isDuplicationError(const QString &error_code);

		//! \brief Restore the export parameters to their default values
		void resetExportParams();

		/*! \brief Aborts the export process by redirecting the provided exception in form of a signal or
		to the main loop, depending on the mode the helper is being used (in a thread or locally) */
		void abortExport(Exception &e);

		/*! brief Handles the provided exception related to the sql command can
		1) emit a ignored error signal if the error code in the exception is one of the ignored ones
		2) append the error in a list of errors generated during the export process
		3) abort the export by immediatelly redirecting the error to the user */
		void handleSQLError(Exception &e, const QString &sql_cmd, bool ignore_dup);

	public:
		ModelExportHelper(QObject *parent = nullptr);

		/*! \brief Determines which error codes must be ignored during the export process.
		There must be some caution when ignore some error codes because the export may
		create an incomplete database or even reach unknown behaviors.
		Error catalog is available at: postgresql.org/docs/current/static/errcodes-appendix.html */
		void setIgnoredErrors(const QStringList &err_codes);

		//! \brief Exports the model to a named SQL file. The PostgreSQL version syntax must be specified.
		void exportToSQL(DatabaseModel *db_model, const QString &filename, const QString &pgsql_ver, bool split, DatabaseModel::CodeGenMode code_gen_mode, bool gen_drop_file);

		/*! \brief Exports the model to a named PNG image. The boolean parameters controls the grid exhibition
		as well the page delimiters on the output image. The zoom parameter controls the zoom applied to the viewport
		before draw it on the pixmap. It is possible to specified an viewport (QGraphicsView instance) previously allocated
		and the method will use it instead of allocate a local one. This is a workaround to error raised by QCoreApplication::sendPostedEvents
		when running the helper in a thread */
		void exportToPNG(ObjectsScene *scene, const QString &filename, double zoom, bool show_grid, bool show_delim,
										 bool page_by_page, bool override_bg_color, QGraphicsView *viewp=nullptr);

		//! \brief Exports the model to a named SVG file.
		void exportToSVG(ObjectsScene *scene, const QString &filename, bool show_grid, bool show_delim);

		/*! \brief Exports the model directly to the DBMS. A valid connection must be specified. The PostgreSQL
		version is optional, since the helper identifies the version from the server. The boolean parameter
		make the helper to ignore object duplicity errors.
		\note The params drop_db and drop_objs can't be true at the same time. */
		void exportToDBMS(DatabaseModel *db_model, Connection conn, const QString &pgsql_ver="", bool ignore_dup=false,
											bool drop_db=false, bool drop_objs=false, bool simulate=false, bool use_tmp_names=false,
											bool forced_db_drop = false, bool transactional = false);

		/*! \brief Exports the model to a named data dictionary. The options browsable and splitted indicate,
		 * respectively, that the data dictionary should have an object index and the dictionary should be split
		 * in different files per table */
		void exportToDataDict(DatabaseModel *db_model, const QString &path, bool browsable, bool split, bool md_format);

		/*! \brief Configures the DBMS export params before start the export thread (when in thread mode).
		This form receive a database model as input and the sql code to be exported will be generated from it.
		\note The params drop_db and drop_objs can't be true at the same time. */
		void setExportToDBMSParams(DatabaseModel *db_model, Connection *conn, const QString &pgsql_ver="", bool ignore_dup=false,
															 bool drop_db=false, bool drop_objs=false, bool simulate=false, bool use_rand_names=false,
															 bool force_db_drop = false, bool transactional = false);

		/*! \brief Configures the DBMS export params before start the export thread (when in thread mode).
		This form receive a previously generated sql buffer to be exported the the helper */
		void setExportToDBMSParams(const QString &sql_buffer, Connection *conn, const QString &db_name, bool ignore_dup=false, bool transactional = false);

		/*! \brief Configures the SQL export params before start the export thread (when in thread mode).
		This form receive the model, output filename and pgsql version to be used */
		void setExportToSQLParams(DatabaseModel *db_model, const QString &filename, const QString &pgsql_ver, bool split, DatabaseModel::CodeGenMode code_gen_mode, bool gen_drop_file);

		/*! \brief Configures the PNG export params before start the export thread (when in thread mode).
		This form receive the objects scene, a viewport, the output filename, zoom factor, grid options and page by page export options */
		void setExportToPNGParams(ObjectsScene *scene, QGraphicsView *viewp, const QString &filename, double zoom,
															bool show_grid, bool show_delim, bool page_by_page, bool override_bg_color);

		/*! \brief Configures the SVG export params before start the export thread (when in thread mode).
		This form receive the objects scene, the output filename, grid options. */
		void setExportToSVGParams(ObjectsScene *scene, const QString &filename, bool show_grid, bool show_delim);

		/*! \brief Configures the Data Dictionary export params before start the export thread (when in thread mode).
		This form receive the database model, the output path and browsabe and split options. */
		void setExportToDataDictParams(DatabaseModel *db_model, const QString &path, bool browsable, bool split, bool md_format);

	signals:
		//! \brief This singal is emitted whenever the export progress changes
		void s_progressUpdated(int progress, QString msg, ObjectType obj_type=ObjectType::BaseObject, QString cmd="", bool is_code_gen = false);

		//! \brief This signal is emited when the export has finished
		void s_exportFinished();

		//! \brief This signal is emited when the export has been cancelled
		void s_exportCanceled();

		//! \brief This signal is emited when the export has encountered a critical error (only in thread mode)
		void s_exportAborted(Exception e);

		//! \brief This signal is emited when the export has encountered a ignorable error (only in thread mode)
		void s_errorIgnored(QString err_code, QString err_msg, QString cmd);

	public slots:
		void exportToDBMS();
		void exportToPNG();
		void exportToSVG();
		void exportToSQL();
		void exportToDataDict();
		void cancelExport();

	private slots:
		//! \brief Updates the exporting progress with the internal progress of sql generation of objects
		void updateProgress(int progress, QString object_id, unsigned obj_type);

		friend class ModelValidationHelper;
		friend class ModelExportForm;
		friend class ModelDatabaseDiffForm;
};

#endif
