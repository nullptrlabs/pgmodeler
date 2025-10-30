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

#include "databaseexplorerwidget.h"
#include "customuistyle.h"
#include "databaseimportwidget.h"
#include "sqlexecutionwidget.h"
#include "settings/snippetsconfigwidget.h"
#include "utils/plaintextitemdelegate.h"
#include "utilsns.h"
#include "guiutilsns.h"
#include "settings/generalconfigwidget.h"
#include "tools/datahandlingform.h"
#include "pgmodelerguiplugin.h"
#include <QScrollBar>

const QString DatabaseExplorerWidget::DepNotDefined;
const QString DatabaseExplorerWidget::DepNotFound { QT_TR_NOOP("(not found, OID: %1)") };
const QString DatabaseExplorerWidget::DefaultSourceCode {
	QString("-- %1 --").arg(QT_TR_NOOP("Source code not generated! Press F7 or middle-click the item to load it."))
};

const attribs_map DatabaseExplorerWidget::attribs_i18n {
	{Attributes::AdminRoles, QT_TR_NOOP("Administrative roles")},	{Attributes::Alignment, QT_TR_NOOP("Alignment")},
	{Attributes::AnalyzeFunc, QT_TR_NOOP("Analyze function")},	{Attributes::ArgCount, QT_TR_NOOP("Argument count")},
	{Attributes::ArgDefCount, QT_TR_NOOP("Argument default count")},	{Attributes::ArgDefaults, QT_TR_NOOP("Argument defaults")},
	{Attributes::ArgModes, QT_TR_NOOP("Argument modes")},	{Attributes::ArgNames, QT_TR_NOOP("Argument names")},
	{Attributes::ArgTypes, QT_TR_NOOP("Argument types")},	{Attributes::Attribute, QT_TR_NOOP("Attribute")},
	{Attributes::BehaviorType, QT_TR_NOOP("Behavior type")},	{Attributes::ByValue, QT_TR_NOOP("By value")},
	{Attributes::CastType, QT_TR_NOOP("Cast type")},	{Attributes::Category, QT_TR_NOOP("Category")},
	{Attributes::Collatable, QT_TR_NOOP("Collatable")},	{Attributes::Collation, QT_TR_NOOP("Collation")},
	{Attributes::Comment, QT_TR_NOOP("Comment")},	{Attributes::CommutatorOp, QT_TR_NOOP("Commutator operator")},
	{Attributes::Configuration, QT_TR_NOOP("Configuration")},	{Attributes::ConnLimit, QT_TR_NOOP("Connection limit")},
	{Attributes::Constraint, QT_TR_NOOP("Constraint")},	{Attributes::CreateDb, QT_TR_NOOP("Create database")},
	{Attributes::CreateRole, QT_TR_NOOP("Create role")},	{Attributes::CurVersion, QT_TR_NOOP("Current version")},
	{Attributes::Default, QT_TR_NOOP("Default")},	{Attributes::DefaultValue, QT_TR_NOOP("Default value")},
	{Attributes::Definition, QT_TR_NOOP("Definition")},	{Attributes::Delimiter, QT_TR_NOOP("Delimiter")},
	{Attributes::DestType, QT_TR_NOOP("Destination type")},	{Attributes::Dimension, QT_TR_NOOP("Dimension")},
	{Attributes::Directory, QT_TR_NOOP("Directory")},	{Attributes::DstEncoding, QT_TR_NOOP("Destination encoding")},
	{Attributes::Element, QT_TR_NOOP("Element")},	{Attributes::Encoding, QT_TR_NOOP("Encoding")},
	{Attributes::Encrypted, QT_TR_NOOP("Encrypted")},	{Attributes::Labels, QT_TR_NOOP("Labels")},
	{Attributes::ExecutionCost, QT_TR_NOOP("Execution cost")},	{Attributes::Expression, QT_TR_NOOP("Expression")},
	{Attributes::Family, QT_TR_NOOP("Operator family")},	{Attributes::FinalFunc, QT_TR_NOOP("Final function")},
	{Attributes::Function, QT_TR_NOOP("Function")},	{Attributes::FunctionType, QT_TR_NOOP("Function type")},
	{Attributes::HandlerFunc, QT_TR_NOOP("Handler function")},
	{Attributes::Hashes, QT_TR_NOOP("Hashes")},	{Attributes::IndexType, QT_TR_NOOP("Index type")},
	{Attributes::Inherit, QT_TR_NOOP("Inherit")},	{Attributes::InitialCond, QT_TR_NOOP("Initial condition")},
	{Attributes::InlineFunc, QT_TR_NOOP("Inline function")},	{Attributes::InputFunc, QT_TR_NOOP("Input function")},
	{Attributes::InternalLength, QT_TR_NOOP("Internal length")},	{Attributes::IntervalType, QT_TR_NOOP("Interval type")},
	{Attributes::IoCast, QT_TR_NOOP("I/O cast")},	{Attributes::JoinFunc, QT_TR_NOOP("Join function")},
	{Attributes::Language, QT_TR_NOOP("Language")},	{Attributes::LcCollate, QT_TR_NOOP("LC COLLATE")},
	{Attributes::LcCtype, QT_TR_NOOP("LC CTYPE")},	{Attributes::LeakProof, QT_TR_NOOP("Leak proof")},
	{Attributes::LeftType, QT_TR_NOOP("Left type")},	{Attributes::Length, QT_TR_NOOP("Length")},
	{Attributes::Library, QT_TR_NOOP("Library")},	{Attributes::Login, QT_TR_NOOP("Login allowed")},
	{Attributes::Materialized, QT_TR_NOOP("Materialized")},	{Attributes::MemberRoles, QT_TR_NOOP("Member roles")},
	{Attributes::Merges, QT_TR_NOOP("Merges")},	{Attributes::Name, QT_TR_NOOP("Name")},
	{Attributes::NegatorOp, QT_TR_NOOP("Negator operator")},	{Attributes::NotNull, QT_TR_NOOP("Not null")},
	{Attributes::ObjectType, QT_TR_NOOP("Object type")},	{Attributes::Oid, QT_TR_NOOP("OID")},
	{Attributes::Oids, QT_TR_NOOP("With OIDs")},	{Attributes::OldVersion, QT_TR_NOOP("Old version")},
	{Attributes::Operator, QT_TR_NOOP("Operator")},	{Attributes::OperatorFunc, QT_TR_NOOP("Operator function")},
	{Attributes::OutputFunc, QT_TR_NOOP("Output function")},	{Attributes::Owner, QT_TR_NOOP("Owner")},
	{Attributes::OwnerColumn, QT_TR_NOOP("Owner column")},	{Attributes::Parents, QT_TR_NOOP("Parents")},
	{Attributes::Password, QT_TR_NOOP("Password")},	{Attributes::Permission, QT_TR_NOOP("Permissions")},
	{Attributes::Precision, QT_TR_NOOP("Precision")},	{Attributes::Preferred, QT_TR_NOOP("Preferred")},
	{Attributes::RangeAttribs, QT_TR_NOOP("Range attributes")},	{Attributes::RecvFunc, QT_TR_NOOP("Receive function")},
	{Attributes::Replication, QT_TR_NOOP("Replication")},
	{Attributes::RestrictionFunc, QT_TR_NOOP("Restriction function")},	{Attributes::ReturnType, QT_TR_NOOP("Return type")},
	{Attributes::ReturnsSetOf, QT_TR_NOOP("Returns SETOF")},	{Attributes::RightType, QT_TR_NOOP("Right type")},
	{Attributes::RowAmount, QT_TR_NOOP("Rows amount")},	{Attributes::Schema, QT_TR_NOOP("Schema")},
	{Attributes::SecurityType, QT_TR_NOOP("Security type")},	{Attributes::SendFunc, QT_TR_NOOP("Send function")},
	{Attributes::SortOp, QT_TR_NOOP("Sort operator")},	{Attributes::SourceType, QT_TR_NOOP("Source type")},
	{Attributes::SrcEncoding, QT_TR_NOOP("Source encoding")},	{Attributes::StateType, QT_TR_NOOP("State type")},
	{Attributes::Storage, QT_TR_NOOP("Storage")},	{Attributes::Superuser, QT_TR_NOOP("Superuser")},
	{Attributes::Tablespace, QT_TR_NOOP("Tablespace")},	{Attributes::TpmodInFunc, QT_TR_NOOP("Type modifier input function")},
	{Attributes::TpmodOutFunc, QT_TR_NOOP("Type modifier output function")},	{Attributes::TransitionFunc, QT_TR_NOOP("Transition function")},
	{Attributes::Trusted, QT_TR_NOOP("Trusted")},	{Attributes::Type, QT_TR_NOOP("Type")},
	{Attributes::TypeAttribute, QT_TR_NOOP("Type attribute")},	{Attributes::Types, QT_TR_NOOP("Types")},
	{Attributes::Unlogged, QT_TR_NOOP("Unlogged")},	{Attributes::Validator, QT_TR_NOOP("Validator function")},
	{Attributes::Validity, QT_TR_NOOP("Validity")},	{Attributes::WindowFunc, QT_TR_NOOP("Window function")},
	{Attributes::False, QT_TR_NOOP("false")},	{Attributes::True, QT_TR_NOOP("true")},
	{Attributes::Cache, QT_TR_NOOP("Cache value")},	{Attributes::Cycle, QT_TR_NOOP("Cycle")},
	{Attributes::Increment, QT_TR_NOOP("Increment")},	{Attributes::MaxValue, QT_TR_NOOP("Maximum value")},
	{Attributes::MinValue, QT_TR_NOOP("Minimum value")},	{Attributes::Start, QT_TR_NOOP("Start value")},
	{Attributes::LastValue, QT_TR_NOOP("Last value")},	{Attributes::Subtype, QT_TR_NOOP("Subtype")},
	{Attributes::OpClass, QT_TR_NOOP("Operator class")},	{Attributes::CanonicalFunc, QT_TR_NOOP("Canonical function")},
	{Attributes::SubtypeDiffFunc, QT_TR_NOOP("Subtype difference function")},	{Attributes::Deferrable, QT_TR_NOOP("Deferrable")},
	{Attributes::PerRow, QT_TR_NOOP("For each row")},	{Attributes::FiringType, QT_TR_NOOP("Firing")},
	{Attributes::InsEvent, QT_TR_NOOP("On insert")},	{Attributes::DelEvent, QT_TR_NOOP("On delete")},
	{Attributes::UpdEvent, QT_TR_NOOP("On update")},	{Attributes::TruncEvent, QT_TR_NOOP("On truncate")},
	{Attributes::Arguments, QT_TR_NOOP("Arguments")},	{Attributes::Table, QT_TR_NOOP("Table")},
	{Attributes::TriggerFunc, QT_TR_NOOP("Trigger function")},	{Attributes::Columns, QT_TR_NOOP("Columns")},
	{Attributes::Condition, QT_TR_NOOP("Condition")},	{Attributes::DeferType, QT_TR_NOOP("Deferment")},
	{Attributes::EventType, QT_TR_NOOP("Event")},	{Attributes::ExecType, QT_TR_NOOP("Execution mode")},
	{Attributes::Commands, QT_TR_NOOP("Commands")},	{Attributes::Position, QT_TR_NOOP("Position")},
	{Attributes::ComparisonType, QT_TR_NOOP("Comparison type")},	{Attributes::DelAction, QT_TR_NOOP("On delete")},
	{Attributes::DstColumns, QT_TR_NOOP("Referenced columns")},	{Attributes::Expressions, QT_TR_NOOP("Expressions")},
	{Attributes::Factor, QT_TR_NOOP("Fill factor")},	{Attributes::NoInherit, QT_TR_NOOP("No inherit")},
	{Attributes::OpClasses, QT_TR_NOOP("Operator classes")},	{Attributes::Operators, QT_TR_NOOP("Operators")},
	{Attributes::RefTable, QT_TR_NOOP("Referenced table")},	{Attributes::Columns, QT_TR_NOOP("Columns")},
	{Attributes::UpdAction, QT_TR_NOOP("On update")},	{Attributes::SrcColumns, QT_TR_NOOP("Columns")},
	{Attributes::Unique, QT_TR_NOOP("Unique")},	{Attributes::Predicate, QT_TR_NOOP("Predicate")},
	{Attributes::Collations, QT_TR_NOOP("Collations")},	{Attributes::Inherited, QT_TR_NOOP("Inherited")},
	{Attributes::ClientEncoding, QT_TR_NOOP("Client encoding")},	{Attributes::ConfigFile, QT_TR_NOOP("Configuration file")},
	{Attributes::DataDirectory, QT_TR_NOOP("Data directory")},	{Attributes::DynamicLibraryPath, QT_TR_NOOP("Dynamic library path")},
	{Attributes::DynamicSharedMemory, QT_TR_NOOP("Dynamic shared memory")},	{Attributes::HbaFile, QT_TR_NOOP("HBA file")},
	{Attributes::ListenAddresses, QT_TR_NOOP("Listen addresses")},	{Attributes::MaxConnections, QT_TR_NOOP("Maximum connections")},
	{Attributes::Port, QT_TR_NOOP("Listen port")},	{Attributes::ServerEncoding, QT_TR_NOOP("Server encoding")},
	{Attributes::Ssl, QT_TR_NOOP("SSL")},	{Attributes::SslCaFile, QT_TR_NOOP("SSL CA file")},
	{Attributes::SslCertFile, QT_TR_NOOP("SSL certificate file")},	{Attributes::SslCrlFile, QT_TR_NOOP("SSL CRL file")},
	{Attributes::SslKeyFile, QT_TR_NOOP("SSL key file")},	{Attributes::ServerVersion, QT_TR_NOOP("Server version")},
	{Attributes::IdentFile, QT_TR_NOOP("Ident file")},	{Attributes::PasswordEncryption, QT_TR_NOOP("Password encryption")},
	{Attributes::Connection, QT_TR_NOOP("Connection ID")},	{Attributes::ServerPid, QT_TR_NOOP("Server PID")},
	{Attributes::ServerProtocol, QT_TR_NOOP("Server protocol")},	{Attributes::Referrers, QT_TR_NOOP("Referrers")},
	{Attributes::IdentityType, QT_TR_NOOP("Identity")},	{Attributes::Command, QT_TR_NOOP("Command")},
	{Attributes::UsingExp, QT_TR_NOOP("USING expression")},	{Attributes::CheckExp, QT_TR_NOOP("CHECK expression")},
	{Attributes::Roles, QT_TR_NOOP("Roles")},	{Attributes::RlsEnabled, QT_TR_NOOP("RLS enabled")},
	{Attributes::RlsForced, QT_TR_NOOP("RLS forced")},	{Attributes::LastAnalyze, QT_TR_NOOP("Last analyze")},
	{Attributes::LastAutovacuum, QT_TR_NOOP("Last autovacuum")},	{Attributes::LastVacuum, QT_TR_NOOP("Last vacuum")},
	{Attributes::TuplesDel, QT_TR_NOOP("Tuples deleted")},	{Attributes::TyplesUpd, QT_TR_NOOP("Tuples updated")},
	{Attributes::TuplesIns, QT_TR_NOOP("Tuples inserted")},	{Attributes::IsPartitioned, QT_TR_NOOP("Partitioned")},
	{Attributes::PartitionedTable, QT_TR_NOOP("Partition of")},	{Attributes::PartitionBoundExpr, QT_TR_NOOP("Partition bound expression")},
	{Attributes::DeadRowsAmount, QT_TR_NOOP("Dead rows amount")},	{Attributes::PartitionKey, QT_TR_NOOP("Partition keys")},
	{Attributes::Partitioning, QT_TR_NOOP("Partitioning")}, {Attributes::Options, QT_TR_NOOP("Options")},
	{Attributes::Fdw, QT_TR_NOOP("Foreign data wrapper")}, 	{Attributes::Server, QT_TR_NOOP("Server")},
	{Attributes::BypassRls, QT_TR_NOOP("Bypass RLS")}, {Attributes::FromSqlFunc, QT_TR_NOOP("From SQL function")},
	{Attributes::ToSqlFunc, QT_TR_NOOP("To SQL function")} , {Attributes::TransformTypes, QT_TR_NOOP("Transform types")},
	{Attributes::ConfigParams, QT_TR_NOOP("Configuration parameters") }, {Attributes::IsTemplate, QT_TR_NOOP("Is template")},
	{Attributes::TemplateDb, QT_TR_NOOP("Template database") }, {Attributes::AllowConns, QT_TR_NOOP("Allows connection")},
	{Attributes::Version, QT_TR_NOOP("Version")},	{Attributes::LcCollateMod, QT_TR_NOOP("LC COLLATE modifier")},
	{Attributes::LcCtype, QT_TR_NOOP("LC CTYPE modifier")}, {Attributes::Provider, QT_TR_NOOP("Provider")},
	{Attributes::IsExtType, QT_TR_NOOP("Is extension type")}, {Attributes::RefTables, QT_TR_NOOP("Referenced tables")},
	{Attributes::NullsNotDistinct, QT_TR_NOOP("Nulls not distinct")}, {Attributes::Constraints, QT_TR_NOOP("Constraints")},
	{Attributes::Deterministic, QT_TR_NOOP("Deterministic")}, {Attributes::LcCtypeMod, QT_TR_NOOP("LC CTYPE modifier")},
	{Attributes::TypeClass, QT_TR_NOOP("Type class")}, {Attributes::Locale, QT_TR_NOOP("Locale")}
};

DatabaseExplorerWidget::DatabaseExplorerWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);

	for(auto &btn : PgModelerGuiPlugin::getPluginsToolButtons())
		installPluginButton(btn);

	pg_version_alert_frm->setVisible(false);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, pg_version_alert_frm);

	curr_scroll_value = 0;
	filter_parent->setVisible(false);
	sort_column = 0;
	splitter->setSizes({ 80, 20 });

	properties_tbw->setItemDelegate(new PlainTextItemDelegate(this, true));
	rename_item=nullptr;

	data_grid_tb->setToolTip(data_grid_tb->toolTip() + QString(" (%1)").arg(data_grid_tb->shortcut().toString()));
	runsql_tb->setToolTip(runsql_tb->toolTip() + QString(" (%1)").arg(runsql_tb->shortcut().toString()));
	drop_db_tb->setToolTip(drop_db_tb->toolTip() + QString(" (%1)").arg(drop_db_tb->shortcut().toString()));

	QAction *act = nullptr;

	act = toggle_disp_menu.addAction(tr("Show objects filter"));
	act->setCheckable(true);
	connect(act, &QAction::toggled, filter_parent, &QWidget::setVisible);

	toggle_disp_menu.addSeparator();

	show_sys_objs = toggle_disp_menu.addAction(tr("Show system objects"));
	show_sys_objs->setCheckable(true);
	connect(show_sys_objs, &QAction::toggled, this, __slot(this, DatabaseExplorerWidget::listObjects));

	show_ext_objs = toggle_disp_menu.addAction(tr("Show extension objects"));
	show_ext_objs->setCheckable(true);
	connect(show_ext_objs, &QAction::toggled, this, __slot(this, DatabaseExplorerWidget::listObjects));

	toggle_display_tb->setMenu(&toggle_disp_menu);

	snippets_menu.setTitle(tr("Snippets"));
	snippets_menu.setIcon(QIcon(GuiUtilsNs::getIconPath("codesnippet")));

	drop_action=new QAction(QIcon(GuiUtilsNs::getIconPath("delete")), tr("Drop object"), &handle_menu);
	drop_action->setShortcut(QKeySequence(Qt::Key_Delete));

	drop_cascade_action=new QAction(QIcon(GuiUtilsNs::getIconPath("delcascade")), tr("Drop cascade"), &handle_menu);
	drop_cascade_action->setShortcut(QKeySequence("Shift+Del"));

	truncate_action=new QAction(QIcon(GuiUtilsNs::getIconPath("truncate")), tr("Truncate"), &handle_menu);
	trunc_cascade_action=new QAction(QIcon(GuiUtilsNs::getIconPath("trunccascade")), tr("Trunc. cascade"), &handle_menu);

	handle_data_action=new QAction(QIcon(GuiUtilsNs::getIconPath("editrows")), tr("Handle data"), &handle_menu);
	handle_data_action->setShortcut(QKeySequence(Qt::Key_Space));
	properties_action=new QAction(QIcon(GuiUtilsNs::getIconPath("reloadattribs")), tr("Reload properties"), &handle_menu);

	refresh_action=new QAction(QIcon(GuiUtilsNs::getIconPath("refresh")), tr("Update"), &handle_menu);
	refresh_action->setShortcut(QKeySequence(Qt::Key_F6));

	rename_action=new QAction(QIcon(GuiUtilsNs::getIconPath("rename")), tr("Rename"), &handle_menu);
	rename_action->setShortcut(QKeySequence(Qt::Key_F2));

	source_action=new QAction(QIcon(GuiUtilsNs::getIconPath("sqlcode")), tr("Source code"), &handle_menu);
	source_action->setShortcut(QKeySequence(Qt::Key_F7));

	objects_trw->installEventFilter(this);

	connect(refresh_tb, &QToolButton::clicked, this, __slot(this, DatabaseExplorerWidget::listObjects));
	connect(objects_trw, &QTreeWidget::itemPressed, this, __slot_n(this, DatabaseExplorerWidget::handleObject));

	connect(objects_trw, &QTreeWidget::currentItemChanged, this, __slot(this, DatabaseExplorerWidget::showObjectProperties));
	connect(raw_attrib_names_chk, &QCheckBox::toggled, this, __slot(this, DatabaseExplorerWidget::showObjectProperties));

	connect(objects_trw, &QTreeWidget::currentItemChanged, this, &DatabaseExplorerWidget::cancelObjectRename);

	connect(objects_trw, &QTreeWidget::itemCollapsed, this, [this](){
		objects_trw->resizeColumnToContents(0);
		cancelObjectRename();
	});

	connect(objects_trw, &QTreeWidget::itemExpanded, this, [this](){
		objects_trw->resizeColumnToContents(0);
		cancelObjectRename();
	});

	connect(data_grid_tb, &QToolButton::clicked, this, [this](){
		DataHandlingForm::openNewWindow(connection.getConnectionParams());
	});

	connect(collapse_all_tb, &QToolButton::clicked, objects_trw, &QTreeWidget::collapseAll);
	connect(by_oid_chk, &QCheckBox::toggled, this, &DatabaseExplorerWidget::filterObjects);
	connect(filter_edt, &QLineEdit::textChanged, this, &DatabaseExplorerWidget::filterObjects);

	connect(drop_db_tb,  &QToolButton::clicked, this, [this]() {
		emit s_databaseDropRequested(connection.getConnectionParam(Connection::ParamDbName));
	});

	connect(runsql_tb, &QToolButton::clicked, this, [this]() {
		emit s_sqlExecutionRequested();
	});

	connect(properties_tbw, &QTableWidget::itemPressed, this, [this]() {
		SQLExecutionWidget::copySelection(properties_tbw, true);
	});

	connect(expand_all_tb, &QToolButton::clicked, this, [this](){
		objects_trw->blockSignals(true);
		objects_trw->expandAll();
		objects_trw->blockSignals(false);
	});

	connect(objects_trw, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
		__trycatch (
			ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
			unsigned oid=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();

			if((obj_type==ObjectType::Schema || BaseTable::isBaseTable(obj_type)) && oid > 0 && item->childCount() <= 1)
			{
				updateItem(item, false);
			}
		)
	});

	connect(sort_by_name_tb, &QToolButton::clicked, this, [this]() {
			sort_column = sort_by_name_tb->isChecked() ? 0 : DatabaseImportWidget::ObjectId;
			objects_trw->sortByColumn(sort_column, Qt::AscendingOrder);
	});

	QMenu *refresh_menu=new QMenu(refresh_tb);

	act=refresh_menu->addAction(tr("Quick refresh"), QKeySequence("Alt+F5"), this, &DatabaseExplorerWidget::listObjects);
	act->setData(QVariant::fromValue<bool>(true));

	act=refresh_menu->addAction(tr("Full refresh"), QKeySequence("Ctrl+F5"), this, &DatabaseExplorerWidget::listObjects);
	act->setData(QVariant::fromValue<bool>(false));

	refresh_tb->setPopupMode(QToolButton::InstantPopup);
	refresh_tb->setMenu(refresh_menu);
}

bool DatabaseExplorerWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object==objects_trw && event->type()==QEvent::KeyPress)
	{
		QKeyEvent *k_event=dynamic_cast<QKeyEvent *>(event);

		if(k_event->key()==Qt::Key_Delete || k_event->key()==Qt::Key_F6 ||
				k_event->key()==Qt::Key_Space ||  k_event->key()==Qt::Key_F2 ||
				k_event->key()==Qt::Key_Escape ||  k_event->key()==Qt::Key_Return ||
				k_event->key()==Qt::Key_Enter || k_event->key()==Qt::Key_F7)
		{
			if(k_event->key()==Qt::Key_Space)
			{
				QTreeWidgetItem *item=objects_trw->currentItem();
				ObjectType obj_type=ObjectType::BaseObject;

				if(item)
				{
					unsigned oid=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();
					obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

					if(oid!=0 && BaseTable::isBaseTable(obj_type))
					{
						DataHandlingForm::openNewWindow(connection.getConnectionParams(),
																						item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString(),
																						item->text(0), obj_type);
					}
				}
			}
			else if(k_event->key()==Qt::Key_F6)
				updateItem(objects_trw->currentItem(), true);
			else if(k_event->key()==Qt::Key_F2)
				startObjectRename(objects_trw->currentItem());
			else if(k_event->key()==Qt::Key_F7)
				loadObjectSource(true);
			else if(k_event->key()==Qt::Key_Escape)
				cancelObjectRename();
			else if(k_event->key()==Qt::Key_Enter ||
					k_event->key()==Qt::Key_Return)
				finishObjectRename();
			else
				dropObject(objects_trw->currentItem(), k_event->modifiers()==Qt::ShiftModifier);

			return true;
		}

		return false;
	}

	return QWidget::eventFilter(object, event);
}

attribs_map DatabaseExplorerWidget::formatObjectAttribs(attribs_map &attribs)
{
	ObjectType obj_type=ObjectType::BaseObject;
	attribs_map fmt_attribs;
	QString attr_name, attr_value;
	QRegularExpression oid_regexp=QRegularExpression(QRegularExpression::anchoredPattern("\\d+"));
	std::map<QString, ObjectType> dep_types={{Attributes::Owner, ObjectType::Role},
										{Attributes::Schema, ObjectType::Schema},
										{Attributes::Tablespace, ObjectType::Tablespace},
										{Attributes::Collation, ObjectType::Collation},
										{Attributes::Table, ObjectType::Table}};

	if(attribs.count(Attributes::ObjectType)!=0)
		obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());

	try
	{
		switch(obj_type)
		{
			case ObjectType::Cast: formatCastAttribs(attribs); break;
			case ObjectType::EventTrigger: formatEventTriggerAttribs(attribs); break;
			case ObjectType::Language: formatLanguageAttribs(attribs); break;
			case ObjectType::Role: formatRoleAttribs(attribs); break;
			case ObjectType::Aggregate: formatAggregateAttribs(attribs); break;
			case ObjectType::Conversion: formatConversionAttribs(attribs); break;
			case ObjectType::Domain: formatDomainAttribs(attribs); break;
			case ObjectType::Extension: formatExtensionAttribs(attribs); break;
			case ObjectType::Function: formatFunctionAttribs(attribs); break;
			case ObjectType::Operator: formatOperatorAttribs(attribs); break;
			case ObjectType::OpClass: formatOperatorClassAttribs(attribs); break;
			case ObjectType::Table: formatTableAttribs(attribs); break;
			case ObjectType::Sequence: formatSequenceAttribs(attribs); break;
			case ObjectType::Type: formatTypeAttribs(attribs); break;
			case ObjectType::View: formatViewAttribs(attribs); break;
			case ObjectType::Trigger: formatTriggerAttribs(attribs); break;
			case ObjectType::Rule: formatRuleAttribs(attribs); break;
			case ObjectType::Column: formatColumnAttribs(attribs); break;
			case ObjectType::Constraint: formatConstraintAttribs(attribs); break;
			case ObjectType::Index: formatIndexAttribs(attribs); break;
			case ObjectType::Policy: formatPolicyAttribs(attribs); break;
			case ObjectType::ForeignDataWrapper: formatForeignDataWrapperAttribs(attribs); break;
			case ObjectType::ForeignServer: formatServerAttribs(attribs); break;
			case ObjectType::UserMapping: formatUserMappingAttribs(attribs); break;
			case ObjectType::Procedure: formatProcedureAttribs(attribs); break;
			case ObjectType::Database: formatDatabaseAttribs(attribs); break;
			default: break;
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}

	if(attribs.count(Attributes::Permission)!=0)
		attribs[Attributes::Permission]=Catalog::parseArrayValues(attribs[Attributes::Permission]).join(UtilsNs::DataSeparator);

	//Removing system schemas from object's name
	if(attribs.count(Attributes::Name)!=0 &&
		 (attribs[Attributes::Name].startsWith("pg_catalog.") ||
			attribs[Attributes::Name].startsWith("information_schema.")))
		attribs[Attributes::Name]=attribs[Attributes::Name].split('.').at(1);

	for(auto &attrib : attribs)
	{
		attr_name=attrib.first;
		attr_value=attrib.second;

		if(attr_name==Attributes::ObjectType)
			attr_value=BaseObject::getTypeName(static_cast<ObjectType>(attr_value.toUInt()));
		//If the current attribute is related to a dependency object, retreive its real name
		else if(dep_types.count(attr_name)!=0 && oid_regexp.match(attr_value).hasMatch())
			attr_value=getObjectName(dep_types[attr_name], attr_value);

		attribs[attr_name]=attr_value;

		//Applying translation on the attribute
		if(attribs_i18n.count(attr_name)!=0)
			attr_name=attribs_i18n.at(attr_name);

		fmt_attribs[attr_name]=attr_value;
	}

	if(attribs[Attributes::Oid].toUInt() > 0)
	{
		attribs[Attributes::SqlObject]=BaseObject::getSQLName(obj_type);
		attribs[Attributes::ObjectType]=BaseObject::getSchemaName(obj_type);

		if(attribs.count(Attributes::Signature)==0)
			attribs[Attributes::Signature]=BaseObject::formatName(attribs[Attributes::Name]);

		if(attribs.count(Attributes::Schema)!=0)
			attribs[Attributes::Signature]=QString("%1.%2")
																						.arg(BaseObject::formatName(attribs[Attributes::Schema]))
																						.arg(attribs[Attributes::Signature]);
	}

	return fmt_attribs;
}

void DatabaseExplorerWidget::formatBooleanAttribs(attribs_map &attribs, QStringList bool_attrs)
{
	for(auto &attr : bool_attrs)
		attribs[attr]=(attribs[attr].isEmpty() ?
							 attribs_i18n.at(Attributes::False) :
							 attribs_i18n.at(Attributes::True));
}

void DatabaseExplorerWidget::formatOidAttribs(attribs_map &attribs, QStringList oid_attrs, ObjectType obj_type, bool is_oid_array)
{
	try
	{
		formatOidAttribs(attribs, oid_attrs, std::vector<ObjectType>{ obj_type }, is_oid_array);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::formatOidAttribs(attribs_map &attribs, QStringList oid_attrs, const std::vector<ObjectType> &obj_types, bool is_oid_array)
{
	try
	{
		if(!is_oid_array)
		{
			for(auto &attr : oid_attrs)
				attribs[attr] = getObjectName(obj_types, attribs[attr]);
		}
		else
		{
			QStringList names;

			for(auto &attr : oid_attrs)
			{
				names = getObjectsNames(obj_types, Catalog::parseArrayValues(attribs[attr]));
				attribs[attr]= names.join(UtilsNs::DataSeparator);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::formatDatabaseAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::IsTemplate });
}

void DatabaseExplorerWidget::formatCastAttribs(attribs_map &attribs)
{  
	formatBooleanAttribs(attribs, { Attributes::IoCast });

	formatOidAttribs(attribs, { Attributes::DestType,
								Attributes::SourceType }, ObjectType::Type, false);

	attribs[Attributes::Function]=getObjectName(ObjectType::Function, attribs[Attributes::Function]);
}

void DatabaseExplorerWidget::formatEventTriggerAttribs(attribs_map &attribs)
{
	attribs[Attributes::Values]=Catalog::parseArrayValues(attribs[Attributes::Values]).join(UtilsNs::DataSeparator);
	attribs[Attributes::Function]=getObjectName(ObjectType::Function, attribs[Attributes::Function]);
}

void DatabaseExplorerWidget::formatAggregateAttribs(attribs_map &attribs)
{
	formatOidAttribs(attribs, { Attributes::FinalFunc,
								Attributes::TransitionFunc }, ObjectType::Function, false);

	formatOidAttribs(attribs, { Attributes::Types }, ObjectType::Type, true);
	attribs[Attributes::Signature]=(QString("%1(%2)")
											 .arg(BaseObject::formatName(attribs[Attributes::Name]))
											.arg(attribs[Attributes::Types])).replace(UtilsNs::DataSeparator, ",");

	attribs[Attributes::StateType]=getObjectName(ObjectType::Type, attribs[Attributes::StateType]);
	attribs[Attributes::SortOp]=getObjectName(ObjectType::Operator, attribs[Attributes::SortOp]);
	attribs[Attributes::InitialCond]=Catalog::parseArrayValues(attribs[Attributes::InitialCond]).join(UtilsNs::DataSeparator);
}

void DatabaseExplorerWidget::formatLanguageAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::Trusted });
	formatOidAttribs(attribs, { Attributes::ValidatorFunc,
															Attributes::HandlerFunc,
															Attributes::InlineFunc }, ObjectType::Function, false);
}

void DatabaseExplorerWidget::formatRoleAttribs(attribs_map &attribs)
{
	formatOidAttribs(attribs, { Attributes::AdminRoles,
															Attributes::MemberRoles }, ObjectType::Role, true);

	formatBooleanAttribs(attribs, { Attributes::Superuser, Attributes::Inherit,
																	Attributes::CreateRole, Attributes::CreateDb,
																	Attributes::Login, Attributes::Encrypted,
																	Attributes::Replication });
}

void DatabaseExplorerWidget::formatConversionAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::Default });
	attribs[Attributes::Function]=getObjectName(ObjectType::Function, attribs[Attributes::Function]);
}

void DatabaseExplorerWidget::formatDomainAttribs(attribs_map &attribs)
{
	QStringList contrs = Catalog::parseArrayValues(attribs[Attributes::Constraints]);

	contrs.replaceInStrings(UtilsNs::DataSeparator, QChar(':'));
	attribs[Attributes::Constraints] =  contrs.join(UtilsNs::DataSeparator);

	formatBooleanAttribs(attribs, { Attributes::NotNull });
	attribs[Attributes::Type]=getObjectName(ObjectType::Type, attribs[Attributes::Type]);
}

void DatabaseExplorerWidget::formatExtensionAttribs(attribs_map &attribs)
{
	attribs[Attributes::Types] = Catalog::parseArrayValues(attribs[Attributes::Types]).join(UtilsNs::DataSeparator);
}

void DatabaseExplorerWidget::formatBaseFunctionAttribs(attribs_map &attribs)
{
	attribs[Attributes::Language]=getObjectName(ObjectType::Language, attribs[Attributes::Language]);
	attribs[Attributes::ArgNames]=Catalog::parseArrayValues(attribs[Attributes::ArgNames]).join(UtilsNs::DataSeparator);
	attribs[Attributes::ArgModes]=Catalog::parseArrayValues(attribs[Attributes::ArgModes]).join(UtilsNs::DataSeparator);
	attribs[Attributes::ArgDefaults]=Catalog::parseArrayValues(attribs[Attributes::ArgDefaults]).join(UtilsNs::DataSeparator);
	attribs[Attributes::TransformTypes]=getObjectsNames(ObjectType::Type, Catalog::parseArrayValues(attribs[Attributes::TransformTypes])).join(UtilsNs::DataSeparator);

	formatOidAttribs(attribs, { Attributes::ArgTypes }, ObjectType::Type, true);
	attribs[Attributes::Signature]=(QString("%1(%2)")
																	.arg(BaseObject::formatName(attribs[Attributes::Name]))
																	.arg(attribs[Attributes::ArgTypes])).replace(UtilsNs::DataSeparator, ",");
}

void DatabaseExplorerWidget::formatProcedureAttribs(attribs_map &attribs)
{
	formatBaseFunctionAttribs(attribs);
}

void DatabaseExplorerWidget::formatFunctionAttribs(attribs_map &attribs)
{	
	formatBaseFunctionAttribs(attribs);
	attribs[Attributes::ReturnType]=getObjectName(ObjectType::Type, attribs[Attributes::ReturnType]);

	formatBooleanAttribs(attribs, { Attributes::WindowFunc,
																	Attributes::LeakProof,
																	Attributes::ReturnsSetOf });
}

void DatabaseExplorerWidget::formatOperatorAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::Hashes,
																	Attributes::Merges });

	formatOidAttribs(attribs, { Attributes::LeftType,
								Attributes::RightType}, ObjectType::Type, false);

	formatOidAttribs(attribs, { Attributes::CommutatorOp,
								Attributes::NegatorOp}, ObjectType::Operator, false);

	formatOidAttribs(attribs, { Attributes::OperatorFunc,
								Attributes::RestrictionFunc,
								Attributes::JoinFunc }, ObjectType::Function, false);

	attribs[Attributes::Signature]=(QString("%1(%2,%3)")
											 .arg(BaseObject::formatName(attribs[Attributes::Name], true))
											.arg(attribs[Attributes::LeftType])
			.arg(attribs[Attributes::RightType])).replace(UtilsNs::DataSeparator, ",");
}

void DatabaseExplorerWidget::formatTableAttribs(attribs_map &attribs)
{
	QStringList part_keys;

	formatBooleanAttribs(attribs, { Attributes::Oids,
																	Attributes::Unlogged,
																	Attributes::RlsEnabled,
																	Attributes::RlsForced});

	formatOidAttribs(attribs, { Attributes::Parents }, ObjectType::Table, true);
	formatOidAttribs(attribs, { Attributes::PartitionedTable }, ObjectType::Table, false);

	part_keys.push_back(getObjectsNames(ObjectType::Column,
																			Catalog::parseArrayValues(attribs[Attributes::PartKeyCols]),
																			getObjectName(ObjectType::Schema, attribs[Attributes::Schema]),
																			attribs[Attributes::Name]).join(UtilsNs::DataSeparator));

	part_keys.push_back(Catalog::parseArrayValues(attribs[Attributes::Expressions]).join(UtilsNs::DataSeparator));
	part_keys.removeAll("");

	attribs[Attributes::PartitionKey] = part_keys.join(UtilsNs::DataSeparator);
	attribs.erase(Attributes::PartKeyColls);
	attribs.erase(Attributes::PartKeyOpCls);
	attribs.erase(Attributes::PartKeyExprs);
	attribs.erase(Attributes::PartKeyCols);
}

void DatabaseExplorerWidget::formatSequenceAttribs(attribs_map &attribs)
{
	QStringList owner_col,
			seq_values=Catalog::parseArrayValues(attribs[Attributes::Attribute]),
			seq_attrs={ Attributes::Start, Attributes::MinValue,
						Attributes::MaxValue, Attributes::Increment,
						Attributes::Cache, Attributes::Cycle };
	QString sch_name=getObjectName(ObjectType::Schema, attribs[Attributes::Schema]);

	attribs.erase(Attributes::Attribute);
	for(int i=0; i < seq_values.size(); i++)
		attribs[seq_attrs[i]]=seq_values[i];

	formatBooleanAttribs(attribs, { Attributes::Cycle });

	owner_col=attribs[Attributes::OwnerColumn].split(':');
	if(owner_col.size()==2)
	{
		QStringList names=getObjectName(ObjectType::Table, owner_col[0]).split('.');
		std::vector<attribs_map> col_attribs=catalog.getObjectsAttributes(ObjectType::Column, names[0], names[1], { owner_col[1].toUInt() });

		if(!col_attribs.empty())
			attribs[Attributes::OwnerColumn]=QString("%1.%2.%3").arg(names[0], names[1], col_attribs[0].at(Attributes::Name));
	}

	//Retrieving the current value of the sequence by querying the database
	try
	{
		Connection conn=connection;
		ResultSet res;

		conn.connect();
		conn.executeDMLCommand(QString("SELECT last_value FROM \"%1\".\"%2\"").arg(sch_name).arg(BaseObject::formatName(attribs[Attributes::Name])), res);

		if(res.accessTuple(ResultSet::FirstTuple))
			attribs[Attributes::LastValue]=res.getColumnValue("last_value");

		conn.close();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::formatViewAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::Materialized });

	formatOidAttribs(attribs, { Attributes::RefTables },
										{ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable }, true);
}

void DatabaseExplorerWidget::formatTypeAttribs(attribs_map &attribs)
{
	QStringList range_attr=Catalog::parseArrayValues(attribs[Attributes::RangeAttribs]),
			type_attr=Catalog::parseArrayValues(attribs[Attributes::TypeAttribute]);

	formatBooleanAttribs(attribs, { Attributes::ByValue, Attributes::Collatable,
																	Attributes::Preferred, Attributes::IsExtType });

	formatOidAttribs(attribs, { Attributes::AnalyzeFunc,
								Attributes::InputFunc,
								Attributes::OutputFunc,
								Attributes::RecvFunc,
								Attributes::SendFunc,
								Attributes::TpmodInFunc,
								Attributes::TpmodOutFunc }, ObjectType::Function, false);

	attribs[Attributes::Element]=getObjectName(ObjectType::Type, attribs[Attributes::Element]);

	if(attribs[Attributes::Labels].isEmpty())
		attribs.erase(Attributes::Labels);
	else
		attribs[Attributes::Labels]=Catalog::parseArrayValues(attribs[Attributes::Labels]).join(UtilsNs::DataSeparator);

	attribs.erase(Attributes::RangeAttribs);
	if(!range_attr.isEmpty())
	{
		attribs[Attributes::Subtype]=getObjectName(ObjectType::Type, range_attr[0]);
		attribs[Attributes::Collation]=getObjectName(ObjectType::Collation, range_attr[1]);
		attribs[Attributes::OpClass]=getObjectName(ObjectType::OpClass, range_attr[2]);
		attribs[Attributes::CanonicalFunc]=getObjectName(ObjectType::Function, range_attr[3]);
		attribs[Attributes::SubtypeDiffFunc]=getObjectName(ObjectType::Function, range_attr[4]);
	}

	if(!type_attr.isEmpty())
	{
		QStringList list, fmt_attribs;

		for(auto &attr : type_attr)
		{
			list=attr.split(':');
			list.removeAt(2);
			fmt_attribs.push_back(list.join(QLatin1String(" ")));
		}

		attribs[Attributes::TypeAttribute]=fmt_attribs.join(UtilsNs::DataSeparator);
	}
	else
		attribs.erase(Attributes::TypeAttribute);
}

void DatabaseExplorerWidget::formatOperatorClassAttribs(attribs_map &attribs)
{
	QStringList list, array_vals, elems;

	attribs[Attributes::Family] = getObjectName(ObjectType::OpFamily, attribs[Attributes::Family]);
	formatBooleanAttribs(attribs, { Attributes::Default });
	formatOidAttribs(attribs, { Attributes::Storage,
								Attributes::Type }, ObjectType::Type, false);

	array_vals = Catalog::parseArrayValues(attribs[Attributes::Function]);

	if(!array_vals.isEmpty())
	{
		for(auto &array_val : array_vals)
		{
			list = array_val.split(':');
			elems.push_back(QString("[%1] %2").arg(list[0], getObjectName(ObjectType::Function, list[1])));
		}

		attribs[Attributes::Function]=elems.join(UtilsNs::DataSeparator);
		elems.clear();
	}

	array_vals=Catalog::parseArrayValues(attribs[Attributes::Operator]);

	if(!array_vals.isEmpty())
	{
		for(auto & array_val : array_vals)
		{
			list = array_val.split(':');
			elems.push_back(QString("[%1] [%2] [%3]")
											.arg(list[0],
													 getObjectName(ObjectType::Operator, list[1]),
													 getObjectName(ObjectType::Operator, list[2])));
		}

		attribs[Attributes::Operator]=elems.join(UtilsNs::DataSeparator);
		elems.clear();
	}
}

void DatabaseExplorerWidget::formatTriggerAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::Constraint,
									Attributes::Deferrable,
									Attributes::PerRow,
									Attributes::InsEvent,
									Attributes::DelEvent,
									Attributes::UpdEvent,
									Attributes::TruncEvent });

	attribs[Attributes::TriggerFunc]=getObjectName(ObjectType::Function, attribs[Attributes::TriggerFunc]);
	attribs[Attributes::Arguments]=attribs[Attributes::Arguments].split(Catalog::EscapedNullChar, Qt::SkipEmptyParts).join(UtilsNs::DataSeparator);
	attribs[Attributes::Columns]=Catalog::parseArrayValues(attribs[Attributes::Columns]).join(UtilsNs::DataSeparator);
}

void DatabaseExplorerWidget::formatRuleAttribs(attribs_map &attribs)
{
	attribs[Attributes::Commands]=Catalog::parseRuleCommands(attribs[Attributes::Commands]).join(';');
}

void DatabaseExplorerWidget::formatColumnAttribs(attribs_map &attribs)
{
	formatBooleanAttribs(attribs, { Attributes::NotNull,
									Attributes::Inherited });
	attribs[Attributes::Position]=attribs[Attributes::Oid];
	attribs.erase(Attributes::Oid);
	attribs.erase(Attributes::TypeOid);
}

void DatabaseExplorerWidget::formatConstraintAttribs(attribs_map &attribs)
{
	std::map<QString, ConstraintType> types={{Attributes::PkConstr, ConstraintType(ConstraintType::PrimaryKey)},
										{Attributes::FkConstr, ConstraintType(ConstraintType::ForeignKey)},
										{Attributes::UqConstr, ConstraintType(ConstraintType::Unique)},
										{Attributes::CkConstr, ConstraintType(ConstraintType::Check)},
										{Attributes::ExConstr, ConstraintType(ConstraintType::Exclude)}};

	ConstraintType constr_type=types[attribs[Attributes::Type]];
	QStringList names=getObjectName(ObjectType::Table, attribs[Attributes::Table]).split('.');

	formatBooleanAttribs(attribs, { Attributes::Deferrable,
																	Attributes::NoInherit });

	attribs[Attributes::Type]=~types[attribs[Attributes::Type]];

	attribs[Attributes::OpClasses]=getObjectsNames(ObjectType::OpClass,
																									 Catalog::parseArrayValues(attribs[Attributes::OpClasses])).join(UtilsNs::DataSeparator);

	attribs[Attributes::SrcColumns]=getObjectsNames(ObjectType::Column,
																										Catalog::parseArrayValues(attribs[Attributes::SrcColumns]),
																										names[0], names[1]).join(UtilsNs::DataSeparator);

	if(constr_type==ConstraintType::ForeignKey)
	{
		attribs[Attributes::RefTable]=getObjectName(ObjectType::Table, attribs[Attributes::RefTable]);
		names=attribs[Attributes::RefTable].split('.');
		attribs[Attributes::DstColumns]=getObjectsNames(ObjectType::Column,
																											Catalog::parseArrayValues(attribs[Attributes::DstColumns]),
																											names[0], names[1]).join(UtilsNs::DataSeparator);
	}
	else
	{
		attribs.erase(Attributes::DstColumns);
		attribs.erase(Attributes::RefTable);
		attribs.erase(Attributes::UpdAction);
		attribs.erase(Attributes::DelAction);
		attribs.erase(Attributes::ComparisonType);
	}

	attribs.erase(Attributes::TableType);

	if(constr_type==ConstraintType::Check)
	{
		attribs.erase(Attributes::Deferrable);
		attribs.erase(Attributes::DeferType);
	}
	else
		attribs.erase(Attributes::Expression);

	if(constr_type==ConstraintType::Exclude)
	{
		attribs[Attributes::Expressions]=Catalog::parseArrayValues(attribs[Attributes::Expressions]).join(UtilsNs::DataSeparator);
		attribs[Attributes::Operators]=getObjectsNames(ObjectType::Operator,
																Catalog::parseArrayValues(attribs[Attributes::Operators])).join(UtilsNs::DataSeparator);
	}
	else
	{
		attribs.erase(Attributes::Condition);
		attribs.erase(Attributes::Expressions);
		attribs.erase(Attributes::Operators);
	}
}

void DatabaseExplorerWidget::formatIndexAttribs(attribs_map &attribs)
{
	QStringList names=getObjectName(ObjectType::Table, attribs[Attributes::Table]).split('.');

	if(names.isEmpty() || names.size() == 1)
		names=getObjectName(ObjectType::View, attribs[Attributes::Table]).split('.');

	formatBooleanAttribs(attribs, { Attributes::Unique });

	attribs[Attributes::Expressions]=Catalog::parseIndexExpressions(attribs[Attributes::Expressions]).join(UtilsNs::DataSeparator);

	attribs[Attributes::Collations]=getObjectsNames(ObjectType::Collation,
															 Catalog::parseArrayValues(attribs[Attributes::Collations])).join(UtilsNs::DataSeparator);

	attribs[Attributes::OpClasses]=getObjectsNames(ObjectType::OpClass,
															 Catalog::parseArrayValues(attribs[Attributes::OpClasses])).join(UtilsNs::DataSeparator);

	attribs[Attributes::Columns]=getObjectsNames(ObjectType::Column,
														Catalog::parseArrayValues(attribs[Attributes::Columns]),	names[0], names[1]).join(UtilsNs::DataSeparator);
}

void DatabaseExplorerWidget::formatPolicyAttribs(attribs_map &attribs)
{
	attribs[Attributes::Roles] = getObjectsNames(ObjectType::Role, Catalog::parseArrayValues(attribs[Attributes::Roles])).join(UtilsNs::DataSeparator);
}

void DatabaseExplorerWidget::formatForeignDataWrapperAttribs(attribs_map &attribs)
{
	attribs[Attributes::Options]=Catalog::parseArrayValues(attribs[Attributes::Options]).join(UtilsNs::DataSeparator);
	formatOidAttribs(attribs, { Attributes::ValidatorFunc, Attributes::HandlerFunc }, ObjectType::Function, false);
}

void DatabaseExplorerWidget::formatServerAttribs(attribs_map &attribs)
{
	attribs[Attributes::Options]=Catalog::parseArrayValues(attribs[Attributes::Options]).join(UtilsNs::DataSeparator);
	formatOidAttribs(attribs, { Attributes::Fdw }, ObjectType::ForeignDataWrapper, false);
}

void DatabaseExplorerWidget::formatUserMappingAttribs(attribs_map &attribs)
{
	attribs[Attributes::Options]=Catalog::parseArrayValues(attribs[Attributes::Options]).join(UtilsNs::DataSeparator);
	formatOidAttribs(attribs, { Attributes::Owner }, ObjectType::Role, false);
	formatOidAttribs(attribs, { Attributes::Server }, ObjectType::ForeignServer, false);
}

QString DatabaseExplorerWidget::formatObjectName(attribs_map &attribs)
{
	try
	{
		if(attribs.empty() ||
				attribs[Attributes::Oid].isEmpty() ||
				attribs[Attributes::Oid]=="0" ||
				attribs[Attributes::Name].isEmpty())
			return DepNotDefined;

		ObjectType obj_type=static_cast<ObjectType>(attribs[Attributes::ObjectType].toUInt());
		attribs_map aux_attribs;
		QString oid=attribs[Attributes::Oid],
				obj_name=DepNotFound.arg(oid), sch_name;

		if(obj_type!=ObjectType::Type)
			obj_name=BaseObject::formatName(attribs[Attributes::Name], obj_type==ObjectType::Operator);
		else
			obj_name=attribs[Attributes::Name];

		//Retrieving the schema name
		if(!attribs[Attributes::Schema].isEmpty() &&
				attribs[Attributes::Schema]!="0")
		{
			aux_attribs=catalog.getObjectAttributes(ObjectType::Schema, attribs[Attributes::Schema].toUInt());
			sch_name=BaseObject::formatName(aux_attribs[Attributes::Name], false);

			if(!sch_name.isEmpty())
				obj_name=sch_name + "." + obj_name;
		}

		//Formatting paramenter types for function
		if(obj_type==ObjectType::Function)
		{
			QStringList names, arg_types=Catalog::parseArrayValues(attribs[Attributes::ArgTypes]);

			for(auto & arg_type : arg_types)
			{
				names = getObjectName(ObjectType::Type, arg_type).split('.');
				arg_type = names[names.size() - 1];
			}

			obj_name+=QString("(%1)").arg(arg_types.join(','));
		}
		//Formatting paramenter types for operator
		else if(obj_type==ObjectType::Operator)
		{
			QStringList arg_types, names;
			QString type_name;
			std::vector<QString> attrib_ids={ Attributes::LeftType, Attributes::RightType };

			for(auto &attr : attrib_ids)
			{
				names=getObjectName(ObjectType::Type, attribs[attr]).split('.');
				type_name=names[names.size()-1];

				if(type_name.isEmpty()) type_name="-";
				arg_types.push_back(type_name);
			}

			obj_name+=QString("(%1)").arg(arg_types.join(','));
		}

		return obj_name;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QStringList DatabaseExplorerWidget::getObjectsNames(ObjectType obj_type, const QStringList &oids, const QString &sch_name, const QString tab_name)
{
	try
	{
		return getObjectsNames(std::vector<ObjectType>{ obj_type }, oids, sch_name, tab_name);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QStringList DatabaseExplorerWidget::getObjectsNames(const std::vector<ObjectType> &types, const QStringList &oids, const QString &sch_name, const QString tab_name)
{
	try
	{
		if(oids.isEmpty())
			return QStringList{ DepNotDefined };

		std::vector<attribs_map> attribs_vect;
		std::vector<unsigned> oids_vect;
		std::map<QString, attribs_map> attrs_map;
		QStringList names;

		//Converting the oids to unsigned in order to filter them on Catalog
		for(auto &oid : oids)
			oids_vect.push_back(oid.toUInt());

		//Retrieve all the objects by their oids and put them on a auxiliary map in which key is their oids
		for(auto &type : types)
		{
			attribs_vect = catalog.getObjectsAttributes(type, sch_name, tab_name, oids_vect);

			for(auto & attr : attribs_vect)
				attrs_map[attr[Attributes::Oid]] = attr;
		}

		//Retreving the names from the auxiliary map using the provided oids
		for(auto &oid : oids)
			names.push_back(formatObjectName(attrs_map[oid]));

		return names;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString DatabaseExplorerWidget::getObjectName(ObjectType obj_type, const QString &oid, const QString &sch_name, const QString tab_name)
{
	try
	{
		return getObjectName(std::vector<ObjectType>{ obj_type }, oid, sch_name, tab_name);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

QString DatabaseExplorerWidget::getObjectName(const std::vector<ObjectType> &types, const QString &oid, const QString &sch_name, const QString tab_name)
{
	try
	{
		if(oid=="0" || oid.isEmpty())
			return DepNotDefined;

		attribs_map attribs;
		QString name;

		for(auto &type : types)
		{
			attribs = catalog.getObjectAttributes(type, oid.toUInt(), sch_name, tab_name);
			name = formatObjectName(attribs);

			if(!name.isEmpty())
				return name;
		}

		return DepNotDefined;
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::setConnection(Connection conn, const QString &default_db)
{
	this->connection=conn;
	this->default_db=(default_db.isEmpty() ? "postgres" : default_db);
}

Connection DatabaseExplorerWidget::getConnection()
{
	return connection;
}

void DatabaseExplorerWidget::clearObjectProperties()
{
	properties_tbw->clearContents();
	properties_tbw->setRowCount(0);
	emit s_sourceCodeShowRequested("", false);
}

void DatabaseExplorerWidget::listObjects()
{
	try
	{
		QAction *act=qobject_cast<QAction *>(sender());
		bool quick_refresh=(act ? act->data().toBool() : true);

		configureImportHelper();
		objects_trw->blockSignals(true);

		/* If the database version is ignored we display the
		 * alert message if the current db version is unsupported */
		pg_version_alert_frm->setVisible(Connection::isDbVersionIgnored() &&
																		 !catalog.isServerSupported());

		saveTreeState();
		clearObjectProperties();

		if(quick_refresh)
			qApp->setOverrideCursor(Qt::WaitCursor);

		DatabaseImportWidget::listObjects(import_helper, objects_trw, false,
																			GeneralConfigWidget::getConfigurationParam(Attributes::Configuration, Attributes::HideEmptyObjGroups) == Attributes::True ?
																			DatabaseImportWidget::HideEmptyGrps : DatabaseImportWidget::NoGrpsFlag,
																			true, quick_refresh, sort_column);

		QTreeWidgetItem *root = new QTreeWidgetItem, *curr_root = nullptr;

		//Changing the root item of the generated tree to be a special item containing info about the connected server
		curr_root = objects_trw->topLevelItem(0);
		objects_trw->takeTopLevelItem(0);
		root->setText(0, connection.getConnectionId(true));
		root->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("server")));
		root->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, -1);
		root->setData(DatabaseImportWidget::ObjectTypeId, Qt::UserRole, enum_t(ObjectType::BaseObject));
		root->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, tr("-- Source code unavailable for this kind of object --"));
		root->addChild(curr_root);
		objects_trw->addTopLevelItem(root);
		root->setExpanded(true);
		root->setSelected(true);
		objects_trw->setCurrentItem(root);

		restoreTreeState();

		qApp->restoreOverrideCursor();
		objects_trw->blockSignals(false);

		import_helper.closeConnection();
		catalog.closeConnection();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void DatabaseExplorerWidget::configureImportHelper()
{
	import_helper.setConnection(connection);
	import_helper.setCurrentDatabase(connection.getConnectionParam(Connection::ParamDbName));
	import_helper.setImportOptions(show_sys_objs->isChecked(),
																 show_ext_objs->isChecked(),
																 false, false, false, false, false, false);

	catalog.closeConnection();
	catalog.setQueryFilter(Catalog::ListAllObjects);
	catalog.setConnection(connection);
}

void DatabaseExplorerWidget::handleObject(QTreeWidgetItem *item, int)
{
	if(item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).toInt() < 0)
	{
		updateItem(item->parent(), true);
	}
	else if(QApplication::mouseButtons()==Qt::MiddleButton && item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt() >= 0)
	{
		loadObjectSource(true);
	}
	else if(QApplication::mouseButtons()==Qt::RightButton && item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt() >= 0)
	{
		ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
		unsigned obj_id=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();

		SnippetsConfigWidget::configureSnippetsMenu(&snippets_menu, { obj_type, ObjectType::BaseObject });

		for(auto &act : handle_menu.actions())
			handle_menu.removeAction(act);

		handle_menu.addAction(refresh_action);

		if(obj_id > 0)
		{
			if(BaseTable::isBaseTable(obj_type))
				handle_menu.addAction(handle_data_action);

			handle_menu.addAction(properties_action);
			handle_menu.addAction(source_action);

			if(obj_type!=ObjectType::Cast && obj_type!=ObjectType::Database)
				handle_menu.addAction(rename_action);

			if(obj_type!=ObjectType::Database)
			{
				handle_menu.addSeparator();
				handle_menu.addAction(drop_action);

				if(obj_type!=ObjectType::Role && obj_type!=ObjectType::ForeignTable && obj_type!=ObjectType::Tablespace)
					handle_menu.addAction(drop_cascade_action);

				if(obj_type==ObjectType::Table)
				{
					handle_menu.addAction(truncate_action);
					handle_menu.addAction(trunc_cascade_action);
				}
			}
		}

		handle_menu.addSeparator();
		handle_menu.addMenu(&snippets_menu);

		QAction *exec_action=handle_menu.exec(QCursor::pos());

		if(exec_action==drop_action || exec_action==drop_cascade_action)
			dropObject(item,  exec_action==drop_cascade_action);
		else if(exec_action==truncate_action || exec_action==trunc_cascade_action)
			truncateTable(item,  exec_action==trunc_cascade_action);
		else if(exec_action==refresh_action)
			updateItem(objects_trw->currentItem(), true);
		else if(exec_action==rename_action)
			startObjectRename(item);
		else if(exec_action==properties_action)
			showObjectProperties(true);
		else if(exec_action==source_action)
			loadObjectSource(true);
		else if(exec_action==handle_data_action)
		{
			DataHandlingForm::openNewWindow(connection.getConnectionParams(),
																			item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString(),
																			item->text(0), obj_type);
		}
		else if(exec_action)
			handleSelectedSnippet(exec_action->text());
	}
}

void DatabaseExplorerWidget::handleSelectedSnippet(const QString &snip_id)
{
	attribs_map attribs;
	QTreeWidgetItem *item=objects_trw->currentItem();
	ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

	loadObjectProperties();
	attribs=item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).value<attribs_map>();

	if(attribs.empty())
	{
		QString sch_name=item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString(),
				tab_name=item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString();

		//Formatting a schema qualified "table" attribute for table children objects
		if(TableObject::isTableObject(obj_type) && !sch_name.isEmpty() && !tab_name.isEmpty())
			attribs[Attributes::Table]=BaseObject::formatName(sch_name) + "." + BaseObject::formatName(tab_name);
	}
	//Formatting the "name" attribute if it is not schema qualified
	else if(attribs.count(Attributes::Schema) &&
			attribs.count(Attributes::Name) &&
			!attribs[Attributes::Name].contains('.'))
	{
		QString obj_name;

		if(obj_type!=ObjectType::Operator)
			obj_name=BaseObject::formatName(attribs[Attributes::Name]);
		else
			obj_name=attribs[Attributes::Name];

		attribs[Attributes::Name]=BaseObject::formatName(attribs[Attributes::Schema]) + "." + obj_name;
	}

	if(attribs.count(Attributes::SqlObject)==0)
	{
		attribs[Attributes::SqlObject]=BaseObject::getSQLName(obj_type);
		attribs[Attributes::ObjectType]=BaseObject::getSchemaName(obj_type);
	}

	for(auto &attr : attribs)
	{
		if(attr.second.contains(UtilsNs::DataSeparator))
			attribs[attr.first]=attr.second.replace(UtilsNs::DataSeparator,",");
	}

	emit s_snippetShowRequested(SnippetsConfigWidget::getParsedSnippet(snip_id, attribs));
}

attribs_map DatabaseExplorerWidget::extractAttributesFromItem(QTreeWidgetItem *item)
{
	if(!item)
		throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
	QString obj_name=item->text(0);
	int idx=0, idx1=0;
	attribs_map attribs;
	QStringList types;

	attribs[Attributes::SqlObject]=BaseObject::getSQLName(obj_type);
	attribs[Attributes::DeclInTable]="";
	attribs[BaseObject::getSchemaName(obj_type)]=Attributes::True;

	//For cast, operator and function is needed to extract the name and the params types
	if(obj_type==ObjectType::Operator || obj_type==ObjectType::Function || obj_type==ObjectType::Cast)
	{
		idx=obj_name.indexOf('(');
		idx1=obj_name.indexOf(')');
		types=obj_name.mid(idx+1, idx1-idx-1).split(',');
		types.removeAll("-");
		obj_name.remove(idx, obj_name.size());
	}
	else if(obj_type==ObjectType::OpFamily || obj_type==ObjectType::OpClass)
		obj_name.remove(QRegularExpression("( )+(\\[)(.)+(\\])"));

	//Formatting the names
	attribs[Attributes::Name]=BaseObject::formatName(obj_name, obj_type==ObjectType::Operator);
	attribs[Attributes::Table]=BaseObject::formatName(item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString());
	attribs[Attributes::Schema]=BaseObject::formatName(item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString());

	//For table objects the "table" attribute must be schema qualified
	if(obj_type!=ObjectType::Index && TableObject::isTableObject(obj_type))
	{
		attribs[Attributes::Table]=attribs[Attributes::Schema] + "." + attribs[Attributes::Table];
		attribs[Attributes::Signature]=attribs[Attributes::Name] + QString(" ON %1").arg(attribs[Attributes::Table]);
	}
	//For operators and functions there must exist the signature attribute
	else if(obj_type==ObjectType::Operator || obj_type==ObjectType::Function)
		attribs[Attributes::Signature]=attribs[Attributes::Schema] + QString(".") + attribs[Attributes::Name] + QString("(%1)").arg(types.join(UtilsNs::DataSeparator));
	else if(obj_type==ObjectType::Cast)
		attribs[Attributes::Signature]=QString("(%1 AS %2)").arg(types[0]).arg(types[1]);
	else if(obj_type==ObjectType::OpFamily || obj_type==ObjectType::OpClass)
	{
		attribs_map aux_attribs=item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).value<attribs_map>();
		attribs[Attributes::Signature]=QString("%1 USING %2").arg(attribs[Attributes::Name]).arg(aux_attribs[Attributes::IndexType]);
	}
	else if(obj_type == ObjectType::UserMapping)
	{
		QStringList names = attribs[Attributes::Name].remove('"').split('@');
		attribs[Attributes::Signature]=QString("FOR %1 SERVER %2")
																		.arg(BaseObject::formatName(names[0]))
																		.arg(BaseObject::formatName(names[1]));
	}
	else
	{
		/* If we are handling a view we need to append the MATERIALIZED keyword in the sql-object in order
		 * to construct DDL commands correctly for this kind of object */
		if(obj_type==ObjectType::View)
		{
			attribs_map aux_attribs=item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).value<attribs_map>();

			if(aux_attribs[Attributes::Materialized] == Attributes::True)
			{
				attribs[Attributes::SqlObject] =
						QString("%1 %2").arg(Attributes::Materialized.toUpper())
														.arg(BaseObject::getSQLName(ObjectType::View));
			}
		}

		if(!attribs[Attributes::Schema].isEmpty() &&
				attribs[Attributes::Name].indexOf(attribs[Attributes::Schema] + ".") < 0)
			attribs[Attributes::Signature]=attribs[Attributes::Schema] + "." + attribs[Attributes::Name];
		else
			attribs[Attributes::Signature]=attribs[Attributes::Name];
	}

	return attribs;
}

void DatabaseExplorerWidget::dropObject(QTreeWidgetItem *item, bool cascade)
{
	try
	{
		if(item && item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt() > 0)
		{
			Messagebox msg_box;
			ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

			//Roles, tablespaces and user mappings can't be removed in cascade mode
			if(cascade && (obj_type==ObjectType::Role || obj_type==ObjectType::Tablespace || obj_type == ObjectType::UserMapping))
				return;

			QString msg,
					parent_sch = item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString(),
					parent_tab =  item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString(),
					obj_name = item->data(DatabaseImportWidget::ObjectName, Qt::UserRole).toString(),
					parent_name;

			if(!parent_sch.isEmpty())
			{
				parent_name = parent_tab.isEmpty() ?
							BaseObject::getSchemaName(ObjectType::Schema).toLower() :
							tr("relation");
				parent_name += " <strong>" + parent_sch;
				parent_name += !parent_tab.isEmpty() ?  "." + parent_tab : "";
				parent_name += "</strong>, ";
			}

			parent_name +=
					BaseObject::getSchemaName(ObjectType::Database).toLower() +
					QString(" <strong>%1</strong>").arg(connection.getConnectionId(true, true));

			if(!cascade)
				msg=tr("Do you really want to drop the object <strong>%1</strong> <em>(%2)</em> in the %3?")
					.arg(obj_name,BaseObject::getTypeName(obj_type), parent_name);
			else
				msg=tr("Do you really want to <strong>cascade</strong> drop the object <strong>%1</strong> <em>(%2)</em> in the %3? This action will drop all the other objects that depends on it.")
					.arg(obj_name, BaseObject::getTypeName(obj_type), parent_name);

			if(Messagebox::isAccepted(Messagebox::confirm(msg)))
			{
				QTreeWidgetItem *parent=nullptr;
				attribs_map attribs;
				QString drop_cmd;
				Connection conn;

				attribs=extractAttributesFromItem(item);

				if(obj_type==ObjectType::Operator || obj_type==ObjectType::Function)
						attribs[Attributes::Signature].replace(UtilsNs::DataSeparator, QChar(','));

				//Generate the drop command
				schparser.ignoreEmptyAttributes(true);
				schparser.ignoreUnkownAttributes(true);
				drop_cmd=schparser.getSourceCode(Attributes::Drop, attribs, SchemaParser::SqlCode);

				if(cascade)
					drop_cmd.replace(';', " CASCADE;");

				//Executes the drop cmd
				conn=connection;
				conn.connect();
				conn.executeDDLCommand(drop_cmd);

				//Updates the object count on the parent item
				parent=item->parent();
				if(parent && parent->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt()==0)
				{
					unsigned cnt=parent->data(DatabaseImportWidget::ObjectCount, Qt::UserRole).toUInt();
					ObjectType parent_type=static_cast<ObjectType>(parent->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

					cnt--;
					parent->setText(0, BaseObject::getTypeName(parent_type) + QString(" (%1)").arg(cnt));
					parent->setData(DatabaseImportWidget::ObjectCount, Qt::UserRole, QVariant::fromValue<unsigned>(cnt));
				}

				if(parent)
					parent->takeChild(parent->indexOfChild(item));
				else
					objects_trw->takeTopLevelItem(objects_trw->indexOfTopLevelItem(item));

				objects_trw->setCurrentItem(nullptr);
			}
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

bool DatabaseExplorerWidget::truncateTable(const QString &sch_name, const QString &obj_name, bool cascade, Connection connection)
{
	try
	{
		Messagebox msg_box;
		QString msg;

		if(!cascade)
			msg=tr("Do you really want to truncate the table <strong>%1.%2</strong>?").arg(sch_name).arg(obj_name);
		else
			msg=tr("Do you really want to truncate in <strong>cascade</strong> mode the table <strong>%1.%2</strong>? This action will truncate all the tables that depends on it?").arg(sch_name).arg(obj_name);

		msg_box.setCustomOptionText(tr("Also restart sequences"));
		msg_box.show(msg, Messagebox::Confirm, Messagebox::YesNoButtons);

		if(msg_box.isAccepted())
		{
			attribs_map attribs;
			QString truc_cmd;
			Connection conn;
			SchemaParser schparser;

			attribs[Attributes::SqlObject]=BaseObject::getSQLName(ObjectType::Table);
			attribs[Attributes::Signature]=QString("%1.%2").arg(BaseObject::formatName(sch_name)).arg(BaseObject::formatName(obj_name));
			attribs[Attributes::Cascade]=(cascade ? Attributes::True : "");
			attribs[Attributes::RestartSeq]=(msg_box.isCustomOptionChecked() ? Attributes::True : "");

			//Generate the truncate command
			schparser.ignoreEmptyAttributes(true);
			schparser.ignoreUnkownAttributes(true);

			truc_cmd=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, Attributes::Truncate),
																					 attribs);

			//Executes the truncate cmd
			conn = connection;
			conn.connect();
			conn.executeDDLCommand(truc_cmd);
		}

		return msg_box.isAccepted();
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE);
	}
}

void DatabaseExplorerWidget::saveTreeState()
{
	QTreeWidgetItemIterator itr(objects_trw);
	QTreeWidgetItem *item = nullptr;
	int oid = 0, grp_id = 0;

	while(*itr)
	{
		item = *itr;
		oid = item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt();
		grp_id = item->data(DatabaseImportWidget::ObjectGroupId, Qt::UserRole).toInt();
		items_state.append(QString("%1:%2").arg(oid > 0 ? oid : grp_id).arg(item->isExpanded()));
		++itr;
	}

	curr_scroll_value = objects_trw->verticalScrollBar()->value();
}

void DatabaseExplorerWidget::restoreTreeState()
{
	if(items_state.isEmpty())
		return;

	QTreeWidgetItemIterator itr(objects_trw);
	QTreeWidgetItem *item = nullptr;	
	QStringList st_info;
	int oid = 0, grp_id = 0, idx = 0;

	objects_trw->setUpdatesEnabled(false);

	while(*itr)
	{
		item = *itr;
		oid = item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt();
		grp_id = item->data(DatabaseImportWidget::ObjectGroupId, Qt::UserRole).toInt();

		if(grp_id < 0)
			idx = items_state.indexOf(QRegularExpression(QString("(%1)(\\:)(.)+").arg(grp_id)));
		else
			idx = items_state.indexOf(QRegularExpression(QString("(%1)(\\:)(.)+").arg(oid)));

		if(idx >= 0)
		{
			st_info = items_state.at(idx).split(':');
			item->setExpanded(st_info[1].toInt() == 1);
		}

		++itr;
	}

	objects_trw->setUpdatesEnabled(true);
	items_state.clear();	
	objects_trw->verticalScrollBar()->setValue(curr_scroll_value);
}

void DatabaseExplorerWidget::installPluginButton(QToolButton *btn)
{
	if(!btn)
		return;

	QToolButton	*plugin_btn = new QToolButton(this);

	plugin_btn->setIcon(btn->icon());
	plugin_btn->setIconSize(btn->iconSize());
	plugin_btn->setToolTip(btn->toolTip());
	plugin_btn->setPopupMode(QToolButton::InstantPopup);
	plugin_btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
	plugin_btn->setAutoRaise(true);
	toolbuttons_lt->insertWidget(toolbuttons_lt->count() - 2, plugin_btn);

	/* Since plugins are singletons in pgModeler, we use the same tool button associated to
	 * a certain plugin to execute its action in different DatabaseExplorerWidget instances.
	 * Thus the action executed is the one connected to the clicked() event of the plugin's button. */
	connect(plugin_btn, &QToolButton::clicked, this, [this, btn](bool checked){
		/* We set general purpose button's properties (connection id and current database name)
		 * so this information can be processed by the plugin execution slot. */
		btn->setProperty(Attributes::Connection.toStdString().c_str(),
										 connection.getConnectionId());

		btn->setProperty(Attributes::Database.toStdString().c_str(),
										 connection.getConnectionParam(Connection::ParamDbName));

		// Emitting the signal btn->clicked() so the connected slot can be executed.
		emit btn->clicked(checked);
	});

	connect(plugin_btn, &QToolButton::triggered, this, [this, btn](QAction *act){
		btn->setProperty(Attributes::Connection.toStdString().c_str(),
										 connection.getConnectionId());

		btn->setProperty(Attributes::Database.toStdString().c_str(),
										 connection.getConnectionParam(Connection::ParamDbName));

		emit btn->triggered(act);
	});
}

void DatabaseExplorerWidget::truncateTable(QTreeWidgetItem *item, bool cascade)
{
	try
	{
		if(item && item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt() > 0)
		{
			QString obj_name, sch_name;
			obj_name=item->data(DatabaseImportWidget::ObjectName, Qt::UserRole).toString();
			sch_name=BaseObject::formatName(item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString());
			truncateTable(sch_name, obj_name, cascade, connection);
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DatabaseExplorerWidget::updateItem(QTreeWidgetItem *item, bool restore_tree_state)
{
	if(!item || item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt() < 0)
		return;

	ObjectType obj_type = static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
	QString signature = item->text(0);

	try
	{
		QTreeWidgetItem *root=nullptr, *parent=nullptr, *aux_item=nullptr;
		unsigned obj_id=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();
		std::vector<QTreeWidgetItem *> gen_items;
		QString sch_name, tab_name;

		qApp->setOverrideCursor(Qt::WaitCursor);

		if(restore_tree_state)
			saveTreeState();

		if(obj_type==ObjectType::Database)
			listObjects();
		else
		{
			clearObjectProperties();
			parent=item->parent();
			sch_name=item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString();
			tab_name=item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString();

			if(parent)
			{
				if(obj_id==0)
				{
					root=parent;
					parent->takeChild(parent->indexOfChild(item));
				}
				else
				{
					if(obj_type==ObjectType::Schema || BaseTable::isBaseTable(obj_type))
					{
						root=item;
						root->takeChildren();

						if(obj_type == ObjectType::Schema)
							sch_name=item->text(0);
						else
							tab_name=item->text(0);
					}
					else
					{
						root=parent->parent();
						root->takeChild(root->indexOfChild(parent));
					}
				}
			}

			if(!tab_name.isEmpty() && !BaseTable::isBaseTable(obj_type))
				signature.prepend(tab_name + ".");

			if(!sch_name.isEmpty())
				signature.prepend(sch_name + ".");

			configureImportHelper();

			//Updates the group type only
			if(obj_id == 0 || (!BaseTable::isBaseTable(obj_type) && obj_type!=ObjectType::Schema))
				gen_items = DatabaseImportWidget::updateObjectsTree(import_helper, objects_trw, { obj_type }, false,
																														DatabaseImportWidget::HideEmptyGrps, root, sch_name, tab_name);
			else
				//Updates all child objcts when the selected object is a schema or table or view
				gen_items = DatabaseImportWidget::updateObjectsTree(import_helper, objects_trw,
																														BaseObject::getChildObjectTypes(obj_type), false,
																														DatabaseImportWidget::HideEmptyGrps, root, sch_name, tab_name);

			//Creating dummy items for schemas and tables
			if(obj_type == ObjectType::Schema || BaseTable::isBaseTable(obj_type))
			{
				for(auto &item : gen_items)
				{
					aux_item=new QTreeWidgetItem(item);
					aux_item->setText(0, "...");
					aux_item->setData(DatabaseImportWidget::ObjectOtherData, Qt::UserRole, QVariant::fromValue<int>(-1));
				}
			}

			import_helper.closeConnection();
			objects_trw->sortItems(sort_column, Qt::AscendingOrder);
			objects_trw->setCurrentItem(nullptr);

			if(BaseTable::isBaseTable(obj_type))
			{
				objects_trw->blockSignals(true);
				objects_trw->setCurrentItem(item);
				showObjectProperties(true);
				objects_trw->setCurrentItem(nullptr);
				objects_trw->blockSignals(false);
			}
		}

		if(restore_tree_state)
			restoreTreeState();

		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		objects_trw->blockSignals(false);
		objects_trw->setUpdatesEnabled(true);
		//qApp->restoreOverrideCursor();

		throw Exception(tr("Failed to load the subitems of the object <strong>%1</strong> <em>(%2)</em>! Try to reload them manually by hitting <strong>F6</strong> on each one.")
										.arg(signature, BaseObject::getTypeName(obj_type)),
										ErrorCode::Custom, PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void DatabaseExplorerWidget::loadObjectProperties(bool force_reload)
{
	try
	{
		QTreeWidgetItem *item=objects_trw->currentItem();
		unsigned oid=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();

		if(oid != 0 || (item == objects_trw->topLevelItem(0)))
		{
			ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
			attribs_map orig_attribs, fmt_attribs;

			//First, retrieve the attributes stored on the item as a result of a previous properties listing
			orig_attribs=item->data(DatabaseImportWidget::ObjectAttribs, Qt::UserRole).value<attribs_map>();

			//In case of the cached attributes are empty
			if(orig_attribs.empty() || force_reload)
			{
				qApp->setOverrideCursor(Qt::WaitCursor);
				catalog.setConnection(connection);

				//Loading the server properties
				if(item == objects_trw->topLevelItem(0))
					orig_attribs=catalog.getServerAttributes();
				//Retrieve them from the catalog
				else if(obj_type!=ObjectType::Column)
				{
					orig_attribs=catalog.getObjectAttributes(obj_type, oid);

					if(obj_type == ObjectType::Table)
					{
						std::vector<attribs_map> ref_fks;
						attribs_map ref_table, ref_schema;
						QStringList tab_list;

						ref_fks = catalog.getObjectsAttributes(ObjectType::Constraint, "", "", {}, {{ Attributes::CustomFilter, QString("contype='f' AND cs.confrelid=%1").arg(orig_attribs[Attributes::Oid])}});

						for(auto &fk : ref_fks)
						{
							ref_table = catalog.getObjectAttributes(ObjectType::Table, fk[Attributes::Table].toUInt());
							ref_schema = catalog.getObjectAttributes(ObjectType::Schema, ref_table[Attributes::Schema].toUInt());
							tab_list.push_back(QString("%1.%2").arg(ref_schema[Attributes::Name]).arg(ref_table[Attributes::Name]));
						}

						if(!tab_list.isEmpty())
							orig_attribs[Attributes::Referrers] = tab_list.join(UtilsNs::DataSeparator);
					}
				}
				else
				{
					QString tab_name=item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString(),
							sch_name=item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString();
					std::vector<attribs_map> vect_attribs=catalog.getObjectsAttributes(obj_type, sch_name, tab_name, { oid });

					if(!vect_attribs.empty())
						orig_attribs=vect_attribs[0];
				}

				//Format values and translate the attribute names
				fmt_attribs=formatObjectAttribs(orig_attribs);
				fmt_attribs.erase(Attributes::Signature);

				//Store the original attributes on the item to permit value replacements when using code snippets
				item->setData(DatabaseImportWidget::ObjectOtherData, Qt::UserRole, QVariant::fromValue<attribs_map>(orig_attribs));

				//Store the attributes on the item to avoid repeatedly query the database
				item->setData(DatabaseImportWidget::ObjectAttribs, Qt::UserRole, QVariant::fromValue<attribs_map>(fmt_attribs));

				if(item != objects_trw->topLevelItem(0))
					item->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, DefaultSourceCode);

				catalog.closeConnection();
				qApp->restoreOverrideCursor();
			}
		}
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::showObjectProperties(bool force_reload)
{
	try
	{
		QTreeWidgetItem *item=objects_trw->currentItem();
		clearObjectProperties();

		if(item &&
			 ((item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toInt() >= 0) ||
				(item == objects_trw->topLevelItem(0))))
		{
			attribs_map cached_attribs;
			QTableWidgetItem *tab_item=nullptr;
			QStringList values;
			int row=0;
			QFont font;

			loadObjectProperties(force_reload);
			cached_attribs=item->data((raw_attrib_names_chk->isChecked() ?
											 DatabaseImportWidget::ObjectOtherData : DatabaseImportWidget::ObjectAttribs),
									  Qt::UserRole).value<attribs_map>();

			properties_tbw->setSortingEnabled(false);

			if(!cached_attribs.empty())
			{
				for(auto &attrib : cached_attribs)
				{
					properties_tbw->insertRow(properties_tbw->rowCount());
					row=properties_tbw->rowCount() - 1;

					//Creates the property name item
					tab_item=new QTableWidgetItem;
					font=tab_item->font();
					font.setItalic(true);
					tab_item->setText(attrib.first);
					tab_item->setFont(font);
					tab_item->setIcon(QIcon(GuiUtilsNs::getIconPath("attribute")));
					properties_tbw->setItem(row, 0, tab_item);

					values=attrib.second.split(UtilsNs::DataSeparator);

					//Creating the value item
					if(values.size() >= 2)
					{
						//If the values contatins more the one item, the a combo box will be placed instead of the text
						QComboBox *combo=new QComboBox;
						combo->setStyleSheet("border: 0px");
						combo->addItems(values);
						properties_tbw->setCellWidget(row, 1, combo);
					}
					else
					{
						tab_item=new QTableWidgetItem;
						tab_item->setText(attrib.second);
						properties_tbw->setItem(row, 1, tab_item);

						//If the value contains multiple lines, configures the tooltip to expose the complete form of the value
						if(attrib.second.contains('\n') || attrib.second.length() > 30)
							tab_item->setToolTip(attrib.second);
					}
				}

				cached_attribs=item->data(DatabaseImportWidget::ObjectOtherData,Qt::UserRole).value<attribs_map>();

				if(cached_attribs[Attributes::ObjectType]==BaseObject::getSchemaName(ObjectType::Constraint) &&  item->childCount()==0)
				{
					QTreeWidgetItem *fk_item=nullptr, *src_item=nullptr;

					if(cached_attribs[Attributes::Type]==~ConstraintType(ConstraintType::ForeignKey))
					{
						/* Creates two items denoting the source columns and referenced tables.
							These items have a negative id indicating that no popup menu will be show if user
							right-click them. */

						src_item=new QTreeWidgetItem(item);
						src_item->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, QVariant::fromValue<int>(-1));
						src_item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("column")));
						src_item->setText(0, QString("%1(%2)")
															.arg(cached_attribs[Attributes::Table])
															.arg(cached_attribs[Attributes::SrcColumns]));
						src_item->setToolTip(0, tr("Src. table: %1\nSrc. column(s): %2")
																	.arg(cached_attribs[Attributes::Table])
																	.arg(cached_attribs[Attributes::SrcColumns]));
						src_item->setFlags(Qt::ItemIsEnabled);

						fk_item=new QTreeWidgetItem(item);
						fk_item->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, QVariant::fromValue<int>(-1));
						fk_item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("referenced")));
						fk_item->setText(0, QString("%1(%2)")
														.arg(cached_attribs[Attributes::RefTable])
														.arg(cached_attribs[Attributes::DstColumns]));
						fk_item->setToolTip(0, tr("Ref. table: %1\nRef. column(s): %2")
																.arg(cached_attribs[Attributes::RefTable])
																.arg(cached_attribs[Attributes::DstColumns]));
						fk_item->setFlags(Qt::ItemIsEnabled);
					}
					else if(cached_attribs[Attributes::Type]==~ConstraintType(ConstraintType::Unique) ||
									cached_attribs[Attributes::Type]==~ConstraintType(ConstraintType::PrimaryKey))
					{
						QStringList columns=cached_attribs[Attributes::SrcColumns].split(UtilsNs::DataSeparator);

						for(auto &col : columns)
						{
							src_item=new QTreeWidgetItem(item);
							src_item->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, QVariant::fromValue<int>(-1));
							src_item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("column")));
							src_item->setText(0, col);
							src_item->setFlags(Qt::ItemIsEnabled);
						}
					}
				}
				else if(cached_attribs[Attributes::ObjectType] == BaseObject::getSchemaName(ObjectType::Table) &&
								!cached_attribs[Attributes::Referrers].isEmpty() && item->childCount() == 5)
				{
					QTreeWidgetItem *refs_item=nullptr, *tab_item=nullptr;
					QStringList ref_tab_names = cached_attribs[Attributes::Referrers].split(UtilsNs::DataSeparator);
					QFont font;

					refs_item=new QTreeWidgetItem(item);
					font = refs_item->font(0);
					font.setItalic(true);

					refs_item->setFont(0, font);
					refs_item->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, QVariant::fromValue<int>(-1));
					refs_item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("referrer")));
					refs_item->setText(0, QString("%1 (%2)")
															.arg(attribs_i18n.at(Attributes::Referrers))
															.arg(ref_tab_names.length()));

					for(auto &tab_name : ref_tab_names)
					{
						tab_item=new QTreeWidgetItem(refs_item);
						tab_item->setData(DatabaseImportWidget::ObjectId, Qt::UserRole, QVariant::fromValue<int>(-1));
						tab_item->setIcon(0, QPixmap(GuiUtilsNs::getIconPath("table")));
						tab_item->setText(0, tab_name);
						tab_item->setFlags(Qt::ItemIsEnabled);
					}
				}
			}

			emit s_sourceCodeShowRequested(item->data(DatabaseImportWidget::ObjectSource, Qt::UserRole).toString(), false);

			properties_tbw->setSortingEnabled(true);
			properties_tbw->sortByColumn(0, Qt::AscendingOrder);
			properties_tbw->resizeColumnToContents(0);
		}

		properties_tbw->horizontalHeader()->setVisible(properties_tbw->rowCount() > 0);
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void DatabaseExplorerWidget::startObjectRename(QTreeWidgetItem *item)
{
	if(item && item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt() > 0)
	{
		ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

		if(obj_type!=ObjectType::Cast && obj_type!=ObjectType::Database)
		{
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			objects_trw->openPersistentEditor(item);
			rename_item=item;
			rename_item->setData(DatabaseImportWidget::ObjectOtherData, Qt::UserRole, rename_item->text(0));
		}
	}
}

void DatabaseExplorerWidget::finishObjectRename()
{
	try
	{
		if(rename_item)
		{
			QString rename_cmd;
			Connection conn=connection;
			attribs_map attribs=extractAttributesFromItem(rename_item);
			ObjectType obj_type=static_cast<ObjectType>(rename_item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());

			objects_trw->closePersistentEditor(rename_item);
			attribs[Attributes::NewName]=BaseObject::formatName(rename_item->text(0), obj_type==ObjectType::Operator);

			//Generate the drop command
			schparser.ignoreEmptyAttributes(true);
			schparser.ignoreUnkownAttributes(true);

			rename_cmd=schparser.getSourceCode(GlobalAttributes::getSchemaFilePath(GlobalAttributes::AlterSchemaDir, Attributes::Rename),
													 attribs);

			//Executes the rename cmd
			conn.connect();
			conn.executeDDLCommand(rename_cmd);

			rename_item->setFlags(rename_item->flags() ^ Qt::ItemIsEditable);
			rename_item->setData(DatabaseImportWidget::ObjectName, Qt::UserRole, rename_item->text(0));
			rename_item=nullptr;
		}
	}
	catch(Exception &e)
	{
		cancelObjectRename();
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void DatabaseExplorerWidget::cancelObjectRename()
{
	if(rename_item)
	{
		objects_trw->closePersistentEditor(rename_item);
		rename_item->setFlags(rename_item->flags() ^ Qt::ItemIsEditable);
		rename_item->setText(0, rename_item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).toString());
		rename_item=nullptr;
	}
}

void DatabaseExplorerWidget::loadObjectSource(bool show_code)
{
	QTreeWidgetItem *item=objects_trw->currentItem();

	try
	{
		if(item == objects_trw->topLevelItem(0))
		{
			emit s_sourceCodeShowRequested(item->data(DatabaseImportWidget::ObjectSource, Qt::UserRole).toString(), show_code);
		}
		else if(item)
		{
			DatabaseModel dbmodel;
			DatabaseImportHelper import_hlp;
			ObjectType obj_type=static_cast<ObjectType>(item->data(DatabaseImportWidget::ObjectTypeId, Qt::UserRole).toUInt());
			QString sch_name, tab_name, name, source;
			QTreeWidgetItem *sch_item=nullptr;
			BaseObject *object=nullptr;
			BaseObject *schema=nullptr;
			attribs_map attribs=item->data(DatabaseImportWidget::ObjectOtherData, Qt::UserRole).value<attribs_map>();
			bool is_column=false;
			unsigned oid=item->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt(),
					db_oid=objects_trw->topLevelItem(0)->child(0)->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt(),
					sys_oid=0;
			int sbar_value=(objects_trw->verticalScrollBar() ? objects_trw->verticalScrollBar()->value() : 0);

			qApp->setOverrideCursor(Qt::WaitCursor);
			sch_name=item->data(DatabaseImportWidget::ObjectSchema, Qt::UserRole).toString();
			tab_name=item->data(DatabaseImportWidget::ObjectTable, Qt::UserRole).toString();
			name=item->data(DatabaseImportWidget::ObjectName, Qt::UserRole).toString();

			if(!sch_name.isEmpty() && obj_type!=ObjectType::Extension)
			{
				if(tab_name.isEmpty())
					name.prepend(sch_name + QChar('.'));
				else
					tab_name.prepend(sch_name + QChar('.'));
			}

			//Special case for columns. We will retrieve the table from database and then generate the code for the column
			if(obj_type==ObjectType::Column)
			{
				oid=item->parent()->parent()->data(DatabaseImportWidget::ObjectId, Qt::UserRole).toUInt();
				is_column=true;
				obj_type=ObjectType::Table;
			}

			//Importing the object and its dependencies
			dbmodel.createSystemObjects(false);
			import_hlp.setConnection(connection);
			import_hlp.setCurrentDatabase(connection.getConnectionParam(Connection::ParamDbName));

			import_hlp.setImportOptions(show_sys_objs->isChecked(),
																	show_ext_objs->isChecked(),
																	true, false, false, false, false, false);

			import_hlp.setSelectedOIDs(&dbmodel, {{ObjectType::Database, {db_oid}}, {obj_type, {oid}}}, {});

			sys_oid=import_hlp.getLastSystemOID();

			//Currently pgModeler does not support the visualization of base types and built-in ones
			if(obj_type==ObjectType::Type &&
				 (oid <= sys_oid || attribs[Attributes::Configuration]==Attributes::BaseType))
			{
				source=QString("-- %1 --").arg(tr("Source code genaration for built-in and base types currently unavailable."));
				emit s_sourceCodeShowRequested(source, show_code);
			}
			else
			{
				import_hlp.importDatabase();

				if(obj_type==ObjectType::Database)
					source=getObjectSource(&dbmodel, &dbmodel);
				else
				{
					/* Fixing the signature of opclasses and opfamilies.
							The name is in form "name [index type]", so we change it to "name USING [index type]" */
					if(obj_type==ObjectType::OpClass || obj_type==ObjectType::OpFamily)
					{
						QString idx_type=item->text(0);

						idx_type.remove(0, idx_type.indexOf(QChar('[')) + 1);
						idx_type.remove(QChar(']'));

						name=QString("%1 USING %2").arg(name).arg(idx_type);
					}

					//Generating the code for table child object
					if(TableObject::isTableObject(obj_type) || is_column)
					{
						BaseTable *table=nullptr;
						table = dynamic_cast<BaseTable *>(dbmodel.getObject(tab_name, {ObjectType::Table, ObjectType::ForeignTable, ObjectType::View}));
						QTreeWidgetItem *table_item=nullptr;

						//If the table was imported then the source code of it will be placed on the respective item
						if(table)
						{
							table_item=item->parent()->parent();
							objects_trw->setCurrentItem(item->parent()->parent());
							table_item->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, getObjectSource(table, &dbmodel));

							sch_item=table_item->parent()->parent();
							schema=table->getSchema();

							//Generate the code of table children objects as ALTER commands (only for columns and constraints)
							if(table->getObjectType() != ObjectType::View)
								dynamic_cast<PhysicalTable *>(table)->setGenerateAlterCmds(true);

							object = table->getObject(name, (is_column ? ObjectType::Column : obj_type));
						}
					}
					else
					{
						// First we try to locate the object by its OID
						object = dbmodel.getObjectByOid(oid, obj_type);

						// If we can't find it by the OID we try to search by its name
						if(!object)
							object = dbmodel.getObject(name, obj_type);

						schema = object ? object->getSchema() : nullptr;
					}

					if(object)
						source = getObjectSource(object, &dbmodel);
					else
						source = QString("-- %1 --").arg(tr("Source code unavailable for the object %1 (%2).").arg(name).arg(BaseObject::getTypeName(obj_type)));
				}
			}

			//Generating the schema code and assigning it to the respective items
			if(schema)
			{
				if(!sch_item) sch_item=item->parent()->parent();
				objects_trw->setCurrentItem(sch_item);
				sch_item->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, getObjectSource(schema, &dbmodel));
			}

			if(obj_type != ObjectType::Database)
			{
				//Generating the code for the database itself and storing it in the first child of the root item in the tree
				objects_trw->setCurrentItem(objects_trw->topLevelItem(0));
				objects_trw->topLevelItem(0)->child(0)->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, getObjectSource(&dbmodel, &dbmodel));
			}

			item->setData(DatabaseImportWidget::ObjectSource, Qt::UserRole, source);
			objects_trw->setCurrentItem(item);

			/* Restore the position of the scrollbar in the tree because the usage of setCurrentItem in previous lines
				 may cause the scrollbar to change its original value */
			if(objects_trw->verticalScrollBar())
				objects_trw->verticalScrollBar()->setValue(sbar_value);

			qApp->restoreOverrideCursor();
			emit s_sourceCodeShowRequested(source, show_code);
		}
	}
	catch (Exception &e)
	{
		//qApp->restoreOverrideCursor();
		emit s_sourceCodeShowRequested(QString("/* Could not generate source code due to one or more errors! \n \n %1 */").arg(e.getExceptionsText()), show_code);
	}
}

void DatabaseExplorerWidget::filterObjects()
{
	DatabaseImportWidget::filterObjects(objects_trw, filter_edt->text(),
																		(by_oid_chk->isChecked() ? DatabaseImportWidget::ObjectId : 0), false);
}

QString DatabaseExplorerWidget::getObjectSource(BaseObject *object, DatabaseModel *dbmodel)
{
	if(!object || !dbmodel)
		return "";

	std::vector<Permission *> perms;
	QString source;

	dbmodel->getPermissions(object, perms);
	object->setSystemObject(false);
	object->setSQLDisabled(false);
	object->setCodeInvalidated(true);

	if(object!=dbmodel)
		source=object->getSourceCode(SchemaParser::SqlCode);
	else
		source=dbmodel->__getSourceCode(SchemaParser::SqlCode);

	for(auto &perm : perms)
		source+=perm->getSourceCode(SchemaParser::SqlCode);

	return source;
}
