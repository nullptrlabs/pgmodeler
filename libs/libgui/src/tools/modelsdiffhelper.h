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
\class DatabaseModelsDiffHelper
\brief Implements the operations to compare and generate a diff between two database models.
*/

#ifndef MODELS_DIFF_HELPER_H
#define MODELS_DIFF_HELPER_H

#include <QObject>
#include "databasemodel.h"
#include "objectsdiffinfo.h"

class __libgui ModelsDiffHelper: public QObject {
	Q_OBJECT

	private:
		//! \brief List of attributes ignored when comparing XML code of table children objects
		static const QStringList

		TableObjsIgnoredAttribs,

		//! \brief List of tags ignored when comparing XML code of roles specifically
		RolesIgnoredTags,

		//! \brief List of attributes ignored when comparing XML code of other database objects
		ObjectsIgnoredAttribs,

		//! \brief List of tags ignored when comparing XML code of other database objects
		ObjectsIgnoredTags;

		//! \brief Stores the SQL code that represents the diff between model and database
		QString diff_def,

		//! \brief PostgreSQL version used to generate the diff
		pgsql_version;

		//! \brief Indicates if the diff was cancelled by user
		bool diff_canceled,

		//!brief Diff options. See OPT_??? constants
		diff_opts[10];

		//! \brief Stores the count of objects to be dropped, changed or created
		unsigned diffs_counter[4];

		//! \brief Reference model from which all changes are generated
		DatabaseModel *source_model,

		//! \brief Model which is compared to the source one
		*imported_model;

		//! \brief Stores all generated diff information during the process
		std::vector<ObjectsDiffInfo> diff_infos;

		//! \brief Stores all temporary objects created during the diff process
		std::vector<BaseObject *> tmp_objects;

		//! \brief Stores all objects filtered by the partial diff filters
		std::map<unsigned, BaseObject *> filtered_objs;

		/*! \brief Stores all objects types that are forced to be recreated
		 *  by the option OptForceRecreation */
		std::vector<ObjectType> forced_recreate_types;

		/*! note The parameter diff_type in any methods below is one of the values in
		ObjectsDiffInfo::CreateObject|AlterObject|DropObject */

		//! \brief Compares two tables storing the diff between them in the diff_infos vector.
		void diffTables(PhysicalTable *src_table, PhysicalTable *imp_table, ObjectsDiffInfo::DiffType diff_type);

		//! \brief Compares the two models storing the diff between them in the diff_infos vector.
		void diffModels(ObjectsDiffInfo::DiffType diff_type);

		/*! \brief Compares the specified table object against the ones on the source model or imported
		model depending on the diff_type parameter. */
		void diffTableObject(TableObject *tab_obj, ObjectsDiffInfo::DiffType diff_type);

		/*! \brief Compares the two tables' columns and if needed generates the CREATE statments for the missing ones in child_tab.
		 * This is used when a new inheritance relationship is detected between two tables that previously were not parent and child.
		 * In that case, PostgreSQL obligates that the child has at least the same columns as the parent so the inheritance can be
		 * created correctly. */
		void diffColsInheritance(PhysicalTable *parent_tab, PhysicalTable *child_tab);

		//! \brief Creates a diff info instance storing in o diff_infos vector
		void generateDiffInfo(ObjectsDiffInfo::DiffType diff_type, BaseObject *object, BaseObject *old_object=nullptr);

		/*! \brief Processes the generated diff infos resulting in a SQL buffer with the needed commands
		to synchronize both model and database */
		void processDiffInfos();

		/*! \brief Generates the proper DROP and CREATE for the specified object and its references. This method
		is used when the force_recreation is true and the object in the parameter is marked with an ALTER_OBJECT */
		void recreateObject(BaseObject *object, std::vector<BaseObject *> &drop_objs, std::vector<BaseObject *> &create_objs);

		/*! \brief Returns if a diff information exists for the object. The exact_match parameter is used to force the
		comparison of all values on the paramenter against the diff infos. When false the exact_match parameter
		considers one of parameters object or old_object to be used (if not null) */
		bool isDiffInfoExists(ObjectsDiffInfo::DiffType diff_type, BaseObject *object, BaseObject *old_object, bool exact_match = true);

		/*! \brief Generate the proper code definition for the table's child objects. If drop_cmd is true a DROP command
		will be generated otherwise a CREATE is generated. */
		QString getSourceCode(BaseObject *object, bool drop_cmd);

		//! \brief Destroy the temporary objects and clears the diff info list
		void destroyTempObjects();

		BaseObject *getRelNNTable(const QString &obj_name, DatabaseModel *model);

	public:
		enum DiffOptions: unsigned {
			OptKeepClusterObjs,

			//! \brief Indicates if any DROP/TRUNCATE generated must be in cascade mode
			OptCascadeMode,

			//! \brief Recreates only objects that can't be modified using ALTER commands
			OptRecreateUnmodifiable,

			//! \brief Recreates/replace the objects that can't be modified using ALTER commands but accepts CREATE OR REPLACE
			OptReplaceModified,

			//! \brief Indicates if permissions must be preserved on database
			OptKeepObjectPerms,

			/*! \brief Indicates that existing sequences must be reused in serial columns. Since serial columns are converted
			into integer and a new sequence created and assigned as nextval(sequence) default value for those columns,
			if reuse is enabled, new sequences will not be created instead the ones which name matches the column's default
			value will be reused */
			OptReuseSequences,

			//! \brief Indicates to not generate and execute commands to rename the destination database
			OptPreserveDbName,

			/*! \brief Indicates to not generate and execute commands to drop missing objects. For instance, if user
			try to diff a partial model against the original database DROP commands will be generated, this option
			will avoid this situation and preserve the missing (not imported) objects. */
			OptDontDropMissingObjs,

			/*! \brief Indicates to generate and execute commands to drop missing columns and constraints. For instance, if user
			try to diff a partial model against the original database and the OPT_DONT_DROP_MISSING_OBJS is set, DROP commands will not be generated,
			except for columns and constraints. This option is only considered in the process when OPT_DONT_DROP_MISSING_OBJS is enabled. */
			OptDropMissingColsConstr
		};

		ModelsDiffHelper();

		~ModelsDiffHelper() override;

		/*! \brief Configures the models to be compared. It is assumed that src_model is the reference model
		from which all changes must be collected and applied to the database. The imp_model is the
		database model that represents the current arrange of the database. */
		void setModels(DatabaseModel *src_model, DatabaseModel *imp_model);

		/*! \brief Specifies a list of filtered objects to be used in the partial diff
		 * This is only useful when performing a partial diff between a model and a database.
		 * This method makes extra treatment in the provided objects list in order to specify to the
		 * diff process their correct creation order */
		void setFilteredObjects(const std::vector<BaseObject *> &objects);

		/*! \brief This utility method scans the provided list of filtered objects and produces filters
		 * for peer tables related to inheritance/partitioning or the generated tables of many-to-many
		 * relationships. The parameter use_signature indicates that the filter pattern should be constructed
		 * based on the object's signatures instead their names. This is used by the diff form and cli to
		 * perform partial diffs between a database model and a database. */
		static QStringList getRelationshipFilters(const std::vector<BaseObject *> &objects, bool use_signature);

		//! \brief Toggles a diff option throught the OPT_xxx constants
		void setDiffOption(DiffOptions opt_id, bool value);

		/*! \brief Defines the object types that must be recreated forcebly.
		 *  These object types are only used when OptForceRecreation option is set */
		void setForcedRecreateTypes(const std::vector<ObjectType> &forced_rec_types);
		void setForcedRecreateTypeNames(const QStringList &forced_rec_types);

		//! \brief Returns if the provided type is one of the forced recreation ones
		bool isForcedRecreateType(ObjectType obj_type);

		//! \brief Configures the PostgreSQL version used in the diff generation
		void setPgSQLVersion(const QString pgsql_ver);

		//! \brief Returns the count of diff infos of the specified diff_type
		unsigned getDiffTypeCount(ObjectsDiffInfo::DiffType diff_type);

		//! \brief Reset all the diff info counters in order to restart the diff process
		void resetDiffCounter();

		//! \brief Returns the diff containing all the SQL commands needed to synchronize the model and database
		QString getDiffDefinition();

	public slots:
		void diffModels();
		void cancelDiff();

	signals:
		//! \brief This singal is emitted whenever the diff progress changes
		void s_progressUpdated(int progress, QString msg, ObjectType obj_type=ObjectType::BaseObject);

		//! \brief This signal is emited when the diff has finished
		void s_diffFinished();

		//! \brief This signal is emited when the diff has been canceled
		void s_diffCanceled();

		//! \brief This signal is emited when the diffhas encountered a critical error (only in thread mode)
		void s_diffAborted(Exception e);

		//! \brief This signal is emitted when a diff info is generated
		void s_objectsDiffInfoGenerated(ObjectsDiffInfo diff_info);
};

#endif
