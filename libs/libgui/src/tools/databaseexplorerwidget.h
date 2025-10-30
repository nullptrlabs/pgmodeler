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
\class DatabaseExplorerWidge
\brief Implements the operations to browse and manipulate database instances
*/

#ifndef DATABASE_EXPLORER_WIDGET_H
#define DATABASE_EXPLORER_WIDGET_H

#include "ui_databaseexplorerwidget.h"
#include "databaseimporthelper.h"
#include "schemaparser.h"
#include <QMenu>

class __libgui DatabaseExplorerWidget: public QWidget, public Ui::DatabaseExplorerWidget {
	Q_OBJECT

	private:
		/*! \brief Holds the current vertical scrollbar position/value before the tree state saving is executed.
		 * When the tree state is restored this value is used to return the scrollbar value to its original */
		int curr_scroll_value;

		/*! \brief Stores the expanded status of all items in the tree.
		 * The elements in this list have the form (oid|groupid):(0|1).
		 * The first part is the id/group id of the tree item and the second (after :)
		 * is the expanded status (0->false, 1->true).
		 * This attribute is used by saveTreeState() and restoreTreeState() */
		QStringList items_state;

		static const QString DepNotDefined,
		DepNotFound,
		DefaultSourceCode;
		
		//! \brief Stores the translations of all used attributes at properties panel
		static const attribs_map attribs_i18n;
		
		/*! \brief Connection used to handle objects on database. This connection is copied
		whenever a new operation must be performed on database */
		Connection connection;

		QString default_db;
		
		//! \brief Database import helper used to list objects from current connection
		DatabaseImportHelper import_helper;
		
		//! \brief Catalog instance used to retrieve object's attributes
		Catalog catalog;
		
		SchemaParser schparser;
		
		//! \brief Stores the actions to be performed over the object
		QMenu handle_menu,
		
		//! \brief Stores the snippets' actions
		snippets_menu,

		toggle_disp_menu;
		
		QAction *copy_action, *drop_action, *drop_cascade_action,
		*handle_data_action, *refresh_action, *properties_action,
		*trunc_cascade_action, *truncate_action, *rename_action,
		*source_action, *show_sys_objs, *show_ext_objs;
		
		QTreeWidgetItem *rename_item;
		
		int sort_column;

		void configureImportHelper();
		
		//! \brief Drops the object represented by the specified item
		void dropObject(QTreeWidgetItem *item, bool cascade);
		
		//! \brief Truncates the table represented by the specified item
		void truncateTable(QTreeWidgetItem *item, bool cascade);
		
		//! \brief Marks the item as editable so that the user can change its name
		void startObjectRename(QTreeWidgetItem *item);

		bool eventFilter(QObject *object, QEvent *event) override;

		/*! \brief Returns a properly formatted object name by querying using its OID and different object types.
		 * Optional schema and table names can be specified to filter the results */
		QString getObjectName(const std::vector<ObjectType> &types, const QString &oid, const QString &sch_name="", const QString tab_name="");

		/*! \brief Returns a properly formatted object name by querying using its OID and type.
		 * Optional schema and table names can be specified to filter the results */
		QString getObjectName(ObjectType obj_type, const QString &oid, const QString &sch_name = "", const QString tab_name = "");
		
		/*! \brief Returns the properly formatted list of object names by querying them using their OIDs and different object types.
		 * Optional schema and table names can be specified to filter the results */
		QStringList getObjectsNames(const std::vector<ObjectType> &types, const QStringList &oids, const QString &sch_name="", const QString tab_name="");

		/*! \brief Returns a properly formatted list of object names by querying them using their OIDs and type.
		 * Optional schema and table names can be specified to filter the results */
		QStringList getObjectsNames(ObjectType obj_type, const QStringList &oids, const QString &sch_name="", const QString tab_name="");
		
		//! \brief Format the object's name based upon the passed attributes
		QString formatObjectName(attribs_map &attribs);
		
		/*! \brief Formats the keys and values for the passed object attributes returning a new map with the formatted attributes.
		This method do basic formattation on commom attributes and internally call the format[OBJECT]Attribs()
		method according to the object being currently processed. NOTE: the original 'attribs' has its value changed but the
		keys are preserved. */
		attribs_map formatObjectAttribs(attribs_map &attribs);
		
		//! \brief Formats the boolean attributes translating the 1 ou "" values to true or false
		void formatBooleanAttribs(attribs_map &attribs, QStringList bool_attrs);
		
		//! \brief Convert oid attributes (or array of oids) in object names by querying it on catalog
		void formatOidAttribs(attribs_map &attribs, QStringList oid_attrs, ObjectType obj_type, bool is_oid_array);

		/*! \brief Convert oid attributes (or array of oids) in object names by querying on catalog.
		 * This version, instead of use just one object type to be queried, uses a vector of types which causes
		 * the query to be performed on the specified type catalog tables. In that case, the first occurrence found
		 * will be used as object name */
		void formatOidAttribs(attribs_map &attribs, QStringList oid_attrs, const std::vector<ObjectType> &obj_types, bool is_oid_array);
		
		void formatDatabaseAttribs(attribs_map &attribs);
		void formatCastAttribs(attribs_map &attribs);
		void formatLanguageAttribs(attribs_map &attribs);
		void formatRoleAttribs(attribs_map &attribs);
		void formatEventTriggerAttribs(attribs_map &attribs);
		void formatAggregateAttribs(attribs_map &attribs);
		void formatConversionAttribs(attribs_map &attribs);
		void formatDomainAttribs(attribs_map &attribs);
		void formatExtensionAttribs(attribs_map &attribs);
		void formatFunctionAttribs(attribs_map &attribs);
		void formatOperatorAttribs(attribs_map &attribs);
		void formatTableAttribs(attribs_map &attribs);
		void formatSequenceAttribs(attribs_map &attribs);
		void formatViewAttribs(attribs_map &attribs);
		void formatTypeAttribs(attribs_map &attribs);
		void formatOperatorClassAttribs(attribs_map &attribs);
		void formatTriggerAttribs(attribs_map &attribs);
		void formatRuleAttribs(attribs_map &attribs);
		void formatColumnAttribs(attribs_map &attribs);
		void formatConstraintAttribs(attribs_map &attribs);
		void formatIndexAttribs(attribs_map &attribs);
		void formatPolicyAttribs(attribs_map &attribs);
		void formatForeignDataWrapperAttribs(attribs_map &attribs);
		void formatServerAttribs(attribs_map &attribs);
		void formatUserMappingAttribs(attribs_map &attribs);
		void handleSelectedSnippet(const QString &snip_id);
		void formatBaseFunctionAttribs(attribs_map &attribs);
		void formatProcedureAttribs(attribs_map &attribs);
		
		//! \brief Extract an attribute map containing the basic attributes for drop/rename commands
		attribs_map extractAttributesFromItem(QTreeWidgetItem *item);

		//! \brief Updates the selected tree item
		void updateItem(QTreeWidgetItem *item, bool restore_tree_state);
		
		//! \brief Generate the SQL code for the specified object appending the permissions code for it as well
		QString getObjectSource(BaseObject *object, DatabaseModel *dbmodel);

	protected:
		/*! \brief Add a custom tool button in the set of buttons in the top of database tree.
		 *  This can be used to add custom actions for the current database explorer instance.
		 *  the method will perform the need operations to create a copy of the provided button
		 *  (icon, connect signal/slots, etc). */
		void installPluginButton(QToolButton *btn);

	public:
		DatabaseExplorerWidget(QWidget * parent = nullptr);
		
		//! \brief Configures the connection used to retrieve and manipulate objects on database
		void setConnection(Connection conn, const QString &default_db);
		
		//! \brief Returns a copy of the connection used by this explorer instance
		Connection getConnection();
		
		//! \brief Clears the object's properties table
		void clearObjectProperties();
		
		//! \brief Truncates a named table (in cascade mode or not) using the provided connection
		static bool truncateTable(const QString &sch_name, const QString &obj_name, bool cascade, Connection connection);

		//! \brief Saves the current state of all items in the database tree.
		void saveTreeState();

		//! \brief Restores a previous saved state of all items in the database tree
		void restoreTreeState();

	public slots:
		//! \brief Lists all objects for the current selected database
		void listObjects();
		
	private slots:
		//! \brief Shows the menu to drop/show data
		void handleObject(QTreeWidgetItem *item, int);

		/*! \brief Loads the catalog properties of a selected object and stores them in the current selected item,
		the force_reload parameter is used to ignore the cached properties and retrieve them again */
		void loadObjectProperties(bool force_reload=false);
		
		/*! \brief Loads (calling loadObjectProperties) and expose the attributes of the object in the properties grid,
		the force_reload parameter is used to ignore the cached properties and retrieve them again */
		void showObjectProperties(bool force_reload=false);
		
		//! \brief Executes the rename command on the database
		void finishObjectRename();
		
		//! \brief Cancels the rename and restore the original item's name
		void cancelObjectRename();

		void loadObjectSource(bool show_code);

		void filterObjects();

	signals:
		//! \brief This signal is emmited to indicate that a sql execution widget need to be opened
		void s_sqlExecutionRequested();
		
		//! \brief This signal is emmited to indicate that the named database need to be dropped
		void s_databaseDropRequested(QString dbname);
		
		//! \brief This signal is emmited containing the processed snippet to be shown in an input field
		void s_snippetShowRequested(QString snippet);

		//! \brief This signal is emmited containing the source code to be shown in an input field
		void s_sourceCodeShowRequested(QString source, bool force_display);

	friend class SQLToolWidget;
};

#endif
