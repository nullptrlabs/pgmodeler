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
\class ModelValidationHelper
\brief Implements the base operations to validate models
*/

#ifndef MODEL_VALIDATION_HELPER_H
#define MODEL_VALIDATION_HELPER_H

#include <QObject>
#include "validationinfo.h"
#include "databasemodel.h"
#include "connection.h"
#include "tools/modelexporthelper.h"

class __libgui ModelValidationHelper: public QObject {
	Q_OBJECT

	private:
		static const QString SignalMsg;

		//! \brief Reference database model
		DatabaseModel *db_model;

		//! \brief Connection used to validate model on DBMS
		Connection *conn;

		//! \brief Selected PostgreSQL syntax to validate model on DBMS
		QString pgsql_ver;

		//! \brief DBMS export helper used to execute SQL code validation
		ModelExportHelper export_helper;

		QThread *export_thread;

		//! \brief Warning and error counters
		unsigned warn_count, error_count;

		//! \brief Validation progress variables
		int handled_objs, total_objs;

		//! \brief Indicates if the validation was canceled by the user
		bool  valid_canceled,

		//! \brief Indicates if the validation is on fix mode.
		fix_mode,

		use_tmp_names;

		/*! \brief Stores the validation infos generated during validation steps.
		This vector is read when applying fixes */
		std::vector<ValidationInfo> val_infos;

		//! \brief Stores the analyzed relationship marked as invalidated
		std::vector<BaseObject *> inv_rels;

		void generateValidationInfo(ValidationInfo::ValType val_type, BaseObject *object, std::vector<BaseObject *> refs);

		void checkRelationshipTablesIds(BaseObject *object);

		void checkObjectBrokenRefs(BaseObject *object);

		void checkSpObjectBrokenRefs(BaseObject *object);

		void checkConstrNameConflicts();

		void checkMissingPostgisExt();

		void checkInvalidatedRels();

		void checkUselessUqConstrs();

	public:
		ModelValidationHelper();
		~ModelValidationHelper() override;

		/*! \brief Validates the specified model. If a connection is specifies executes the
		SQL validation directly on DBMS */
		void setValidationParams(DatabaseModel *model, Connection *conn=nullptr, const QString &pgsql_ver="", bool use_tmp_names=false);

		//! \brief Switch the validator to fix mode
		void switchToFixMode(bool value);

		//! \brief Returns if the validator is on fix mode
		bool isInFixMode();

		//! \brief Returns the error count (only when executing SQL validation)
		unsigned getErrorCount();

		//! \brief Returns the warning count
		unsigned getWarningCount();

		//! \brief Try to resolve the conflict specified by validation info
		void resolveConflict(ValidationInfo &info);

		bool isValidationCanceled();

	private slots:
		void redirectExportProgress(int prog, QString msg, ObjectType obj_type, QString cmd, bool is_code_gen);
		void captureThreadError(Exception e);
		void emitValidationCanceled();
		void emitValidationFinished();

	public slots:
		void validateModel();
		void applyFixes();
		void cancelValidation();

	signals:
		//! \brief This signal is emitted when a validation info is generated
		void s_validationInfoGenerated(ValidationInfo val_info);

		//! \brief This signal is emitted when the validation progress changes
		void s_progressUpdated(int prog, QString msg, ObjectType obj_type=ObjectType::BaseObject, QString cmd="", bool is_code_gen=false);

		//! \brief This signal is emitted when the object is processed by the validator
		void s_objectProcessed(QString obj_name, ObjectType obj_type);

		//! \brief This signal is emitted when the validation was sucessfully finished
		void s_validationFinished();

		//! \brief This signal is emitted when the validation was canceled by user
		void s_validationCanceled();

		//! \brief This signal is emitted when the dbms export thread start to run
		void s_sqlValidationStarted();

		//! \brief This signal is emitted when the validator applied some fix on validation info
		void s_fixApplied();

		//! \brief This signal is emitted when the validator failed to apply some fix via resolveConflict
		void s_fixFailed(Exception e);

		//! \brief This signal is emitted when the validator changes some objects id by swapping or update operations
		void s_objectIdChanged(BaseObject *object);

		/*! \brief This signal is emitted when the validator need the validation of relationship.
		This process must be performed outside the current thread, this explains the usage of this signal */
		void s_relsValidationRequested();
};

#endif
