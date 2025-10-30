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

#include "baseform.h"
#include "modelwidget.h"
#include "customuistyle.h"
#include "sourcecodewidget.h"
#include "dbobjects/databasewidget.h"
#include "dbobjects/schemawidget.h"
#include "dbobjects/rolewidget.h"
#include "dbobjects/tablespacewidget.h"
#include "dbobjects/languagewidget.h"
#include "dbobjects/functionwidget.h"
#include "dbobjects/castwidget.h"
#include "dbobjects/conversionwidget.h"
#include "dbobjects/domainwidget.h"
#include "dbobjects/aggregatewidget.h"
#include "dbobjects/sequencewidget.h"
#include "dbobjects/operatorwidget.h"
#include "dbobjects/operatorfamilywidget.h"
#include "dbobjects/operatorclasswidget.h"
#include "dbobjects/typewidget.h"
#include "dbobjects/viewwidget.h"
#include "dbobjects/textboxwidget.h"
#include "dbobjects/columnwidget.h"
#include "dbobjects/constraintwidget.h"
#include "dbobjects/rulewidget.h"
#include "dbobjects/triggerwidget.h"
#include "dbobjects/indexwidget.h"
#include "dbobjects/relationshipwidget.h"
#include "dbobjects/tablewidget.h"
#include "taskprogresswidget.h"
#include "objectdepsrefswidget.h"
#include "objectrenamewidget.h"
#include "dbobjects/permissionwidget.h"
#include "dbobjects/collationwidget.h"
#include "dbobjects/extensionwidget.h"
#include "customsqlwidget.h"
#include "dbobjects/tagwidget.h"
#include "dbobjects/eventtriggerwidget.h"
#include "guiutilsns.h"
#include "swapobjectsidswidget.h"
#include "dbobjects/genericsqlwidget.h"
#include "dbobjects/policywidget.h"
#include "tabledatawidget.h"
#include "settings/generalconfigwidget.h"
#include "dbobjects/foreigndatawrapperwidget.h"
#include "dbobjects/foreignserverwidget.h"
#include "dbobjects/usermappingwidget.h"
#include "dbobjects/transformwidget.h"
#include "dbobjects/procedurewidget.h"
#include "coreutilsns.h"
#include "graphicalview.h"
#include "relationshipview.h"
#include "schemaview.h"
#include "styledtextboxview.h"
#include "tableview.h"
#include "pgmodelerguiplugin.h"
#include <QTemporaryFile>
#include <QScrollBar>

QList<const PgModelerGuiPlugin *> ModelWidget::plugins;

bool ModelWidget::cut_operation {false};
bool ModelWidget::save_restore_pos {true};
bool ModelWidget::simple_obj_creation {true};
bool ModelWidget::disable_render_smooth {false};

double ModelWidget::min_object_opacity {0.10};

ModelWidget *ModelWidget::src_model {};
std::vector<BaseObject *> ModelWidget::copied_objects;
std::vector<BaseObject *> ModelWidget::cut_objects;

ModelWidget::ModelWidget(QWidget *parent) : QWidget(parent)
{
	#ifdef DEMO_VERSION
		obj_actions_cnt = 0;
	#endif

	QFont font;
	QLabel *label=nullptr;
	QGridLayout *grid=nullptr;
	QAction *action=nullptr;
	QString str_ico;
	QStringList rel_types_cod={"11", "1n", "nn", "dep", "gen", "part" };
	unsigned i;
	BaseRelationship::RelType
			rel_types_id[]={ BaseRelationship::Relationship11, BaseRelationship::Relationship1n,
											 BaseRelationship::RelationshipNn, BaseRelationship::RelationshipDep,
											 BaseRelationship::RelationshipGen, BaseRelationship::RelationshipPart };

	std::vector<ObjectType> types_vect = BaseObject::getObjectTypes(true, { ObjectType::Database, ObjectType::Permission,
																																					ObjectType::BaseRelationship, ObjectType::Relationship });

	plugins_actions = PgModelerGuiPlugin::getPluginsActions(PgModelerGuiPlugin::ModelAction);
	current_zoom = 1;
	modified = panning_mode = wheel_move = false;
	scene_moving = blink_new_objs = false;
	curr_show_grid = curr_show_delim = true;
	new_obj_type = ObjectType::BaseObject;

	//Generating a temporary file name for the model
	QTemporaryFile tmp_file;

	//Configuring the template mask which includes the full path to temporary dir
	tmp_file.setFileTemplate(GlobalAttributes::getTemporaryFilePath("model_XXXXXX" + GlobalAttributes::DbModelExt));
	tmp_file.open();
	tmp_filename=tmp_file.fileName();
	tmp_file.close();

	protected_model_frm = new QFrame(this);
	protected_model_frm->setObjectName("protected_model_frm");
	protected_model_frm->setGeometry(QRect(20, 10, 500, 25));
	protected_model_frm->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	protected_model_frm->setMinimumSize(QSize(0, 25));
	protected_model_frm->setFrameShape(QFrame::StyledPanel);
	protected_model_frm->setFrameShadow(QFrame::Raised);
	protected_model_frm->setVisible(false);

	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, protected_model_frm);

	label = new QLabel(protected_model_frm);
	label->setMinimumSize(QSize(20, 20));
	label->setMaximumSize(QSize(20, 20));
	label->setScaledContents(true);
	label->setPixmap(QPixmap(GuiUtilsNs::getIconPath("alert")));
	label->setObjectName("icon_lbl");

	grid = new QGridLayout;
	grid->addWidget(label, 0, 0, 1, 1);

	label = new QLabel(protected_model_frm);

	font.setBold(false);
	font.setItalic(false);
	font.setUnderline(false);
	font.setWeight(QFont::Normal);
	font.setStrikeOut(false);
	font.setKerning(true);
	label->setFont(font);
	label->setObjectName("message_lbl");
	label->setWordWrap(true);
	//label->setText(tr("<strong>ATTENTION:</strong> The database model is protected! Any modification over it is disabled!"));

	grid->addWidget(label, 0, 1, 1, 1);
	protected_model_frm->setLayout(grid);
	protected_model_frm->adjustSize();

	db_model = new DatabaseModel(this);
	xmlparser = db_model->getXMLParser();
	op_list = new OperationList(db_model);
	scene = new ObjectsScene;
	scene->installEventFilter(this);

	viewport = new QGraphicsView(scene, this);
	updateRenderHints();
	viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	/* Since we constantly work with objects (re)configuration which leads to bounding
	 * rectangle changes, the viewport area update is done by using bounding rects
	 * as reference in order to perform updates on the areas covered by the modified
	 * objects bounding rects. */
	viewport->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

	//Force the scene to be drawn from the left to right and from top to bottom
	viewport->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	viewport->setCacheMode(QGraphicsView::CacheBackground);
	viewport->centerOn(0,0);
	viewport->setMouseTracking(true);
	viewport->installEventFilter(this);
	viewport->setAcceptDrops(true);

	grid=new QGridLayout;
	grid->addWidget(protected_model_frm, 0,0,1,1);
	grid->addWidget(viewport, 1,0,1,1);
	grid->setContentsMargins(20,20,20,20);
	this->setLayout(grid);

	int sz = (std::min<int>(qApp->primaryScreen()->size().width(),
												 qApp->primaryScreen()->size().height()))/2;

	magnifier_rect = QRect(0, 0,
												 sz * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor(),
												 sz * BaseObjectView::getFontFactor() * BaseObjectView::getScreenDpiFactor());

	magnifier_area_lbl = new QLabel(this);
	magnifier_area_lbl->raise();
	magnifier_area_lbl->setAutoFillBackground(false);
	magnifier_area_lbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	magnifier_area_lbl->setStyleSheet(QString("background-color: #C8f0f0f0;\
										border: 1px solid #C8808080;"));
	magnifier_area_lbl->setVisible(false);
	magnifier_area_lbl->setGeometry(magnifier_rect);

	magnifier_area_lbl->setCursor(Qt::BlankCursor);
	magnifier_area_lbl->installEventFilter(this);
	magnifier_area_lbl->setMouseTracking(true);

	zoom_info_lbl=new QLabel(this);
	zoom_info_lbl->raise();
	zoom_info_lbl->setAutoFillBackground(false);
	zoom_info_lbl->setText(QString("Zoom: 100%"));
	zoom_info_lbl->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	zoom_info_lbl->setStyleSheet(QString("color: #C8000000; \
								 background-color: #C8FFFFa0;\
								 border: 1px solid #C8B16380;"));

	font=zoom_info_lbl->font();
	font.setBold(true);
	font.setPointSizeF(12);
	zoom_info_lbl->setFont(font);
	zoom_info_lbl->adjustSize();
	zoom_info_lbl->setVisible(false);
	zoom_info_timer.setInterval(3000);

	action_edit_data=new QAction(QIcon(GuiUtilsNs::getIconPath("editrows")), tr("Edit data"), this);

	action_source_code=new QAction(QIcon(GuiUtilsNs::getIconPath("sourcecode")), tr("Source"), this);
	action_source_code->setShortcut(QKeySequence(tr("Alt+S")));
	action_source_code->setToolTip(tr("Show object source code"));

	action_edit=new QAction(QIcon(GuiUtilsNs::getIconPath("edit")), tr("Properties"), this);
	action_edit->setShortcut(QKeySequence(tr("Space")));
	action_edit->setToolTip(tr("Edit the object properties"));

	action_protect=new QAction(QIcon(GuiUtilsNs::getIconPath("protect")), tr("Protect"), this);
	action_unprotect=new QAction(QIcon(GuiUtilsNs::getIconPath("unprotect")), tr("Unprotect"), this);
	action_protect->setToolTip(tr("Protects object(s) from modifications"));

	action_remove=new QAction(QIcon(GuiUtilsNs::getIconPath("delete")), tr("Delete"), this);
	action_remove->setShortcut(QKeySequence(tr("Del")));
	action_remove->setMenuRole(QAction::NoRole);

	action_cascade_del=new QAction(QIcon(GuiUtilsNs::getIconPath("delcascade")), tr("Del. cascade"), this);
	action_cascade_del->setShortcut(QKeySequence(tr("Shift+Del")));
	action_cascade_del->setMenuRole(QAction::NoRole);

	action_select_all = select_all_menu.menuAction();
	action_select_all->setIcon(QIcon(GuiUtilsNs::getIconPath("selectmove")));
	action_select_all->setText(tr("Select all"));
	action_select_all->setToolTip(tr("Selects all the graphical objects in the model"));

	action_convert_relnn=new QAction(QIcon(GuiUtilsNs::getIconPath("convrelnn")), tr("Convert"), this);
	action_convert_rel1n=new QAction(QIcon(GuiUtilsNs::getIconPath("convrel1n")), tr("Convert"), this);

	action_copy = copy_menu.menuAction();
	action_copy->setText(tr("Copy"));
	action_copy->setMenuRole(QAction::NoRole);
	action_copy->setIcon(QIcon(GuiUtilsNs::getIconPath("copy")));

	action = copy_menu.addAction(tr("Selected only"));
	action->setShortcut(QKeySequence(tr("Ctrl+C")));

	connect(action, &QAction::triggered, this, [this]() {
		__trycatch( copyObjects(false, false); )
	});

	action = copy_menu.addAction(tr("Sel. + dependencies"));
	action->setShortcut(QKeySequence(tr("Ctrl+Shift+C")));

	connect(action, &QAction::triggered, this, [this]() {
		__trycatch( copyObjects(false, true); )
	});

	action_paste=new QAction(QIcon(GuiUtilsNs::getIconPath("paste")), tr("Paste"), this);
	action_paste->setShortcut(QKeySequence(tr("Ctrl+V")));
	action_paste->setMenuRole(QAction::NoRole);

	action_cut = cut_menu.menuAction();
	action_cut->setText(tr("Cut"));
	action_cut->setMenuRole(QAction::NoRole);
	action_cut->setIcon(QIcon(GuiUtilsNs::getIconPath("cut")));

	action = cut_menu.addAction(tr("Selected only"));
	action->setShortcut(QKeySequence(tr("Ctrl+X")));

	connect(action, &QAction::triggered, this, [this]() {
		__trycatch( cutObjects(false); )
	});

	action = cut_menu.addAction(tr("Sel. + dependencies"));
	action->setShortcut(QKeySequence(tr("Ctrl+Shift+X")));

	connect(action, &QAction::triggered, this, [this]() {
		__trycatch( cutObjects(true); )
	});

	action_deps_refs=new QAction(QIcon(GuiUtilsNs::getIconPath("depsrefs")), tr("Deps && Referrers"), this);

	action_new_object = new_object_menu.menuAction();
	action_new_object->setIcon(QIcon(GuiUtilsNs::getIconPath("newobject")));
	action_new_object->setText(tr("New"));
	action_new_object->setToolTip(tr("Add a new object in the model"));

	action_quick_actions = quick_actions_menu.menuAction();
	action_quick_actions->setIcon(QIcon(GuiUtilsNs::getIconPath("quickactions")));
	action_quick_actions->setText(tr("Quick"));
	action_quick_actions->setToolTip(tr("Quick action for the selected object"));

	action_rename=new QAction(QIcon(GuiUtilsNs::getIconPath("rename")), tr("Rename"), this);
	action_rename->setShortcut(QKeySequence(tr("F2")));
	action_rename->setToolTip(tr("Quick rename the object"));

	action_moveto_schema = schemas_menu.menuAction();
	action_moveto_schema->setIcon(QIcon(GuiUtilsNs::getIconPath("movetoschema")));
	action_moveto_schema->setText(tr("Move to schema"));

	action_set_layer = new QAction(QIcon(GuiUtilsNs::getIconPath("layers")), tr("Set layers"), this);
	action_set_layer->setShortcut(QKeySequence("F3"));
	layers_wgt = new LayersWidget(this);

	action_set_tag = tags_menu.menuAction();
	action_set_tag->setIcon(QIcon(GuiUtilsNs::getIconPath("tag")));
	action_set_tag->setText(tr("Set tag"));

	action_edit_perms=new QAction(QIcon(GuiUtilsNs::getIconPath("permission")), tr("Edit permissions"), this);
	action_edit_perms->setShortcut(QKeySequence(tr("Ctrl+E")));

	action_change_owner = owners_menu.menuAction();
	action_change_owner->setIcon(QIcon(GuiUtilsNs::getIconPath("changeowner")));
	action_change_owner->setText(tr("Change owner"));

	action_sel_sch_children=new QAction(QIcon(GuiUtilsNs::getIconPath("selectmove")), tr("Select children"), this);
	action_sel_tagged_tabs=new QAction(QIcon(GuiUtilsNs::getIconPath("selectmove")), tr("Select tagged"), this);
	action_sel_table_rels=new QAction(QIcon(GuiUtilsNs::getIconPath("selectmove")), tr("Select relationships"), this);

	action_select_object=new QAction(QIcon(GuiUtilsNs::getIconPath("moved")), tr("Select"), this);
	action_parent_rel=new QAction(QIcon(GuiUtilsNs::getIconPath("relationship")), tr("Open relationship"), this);

	action_append_sql=new QAction(QIcon(GuiUtilsNs::getIconPath("sqlappend")), tr("Custom SQL"), this);
	action_append_sql->setShortcut(QKeySequence(tr("Alt+Q")));

	action_create_seq_col=new QAction(QIcon(GuiUtilsNs::getIconPath("sequence")), tr("Convert to sequence"), this);
	action_conv_int_serial=new QAction(QIcon(GuiUtilsNs::getIconPath("sequence")), tr("Convert to serial"), this);

	action_remove_rel_points=new QAction(QIcon(GuiUtilsNs::getIconPath("removepoints")), tr("Remove points"), this);

	action_enable_sql=new QAction(QIcon(GuiUtilsNs::getIconPath("enablesql")), tr("Enable SQL"), this);
	action_disable_sql=new QAction(QIcon(GuiUtilsNs::getIconPath("disablesql")), tr("Disable SQL"), this);

	action_duplicate=new QAction(QIcon(GuiUtilsNs::getIconPath("duplicate")), tr("Duplicate"), this);
	action_duplicate->setShortcut(QKeySequence(tr("Ctrl+D")));
	action_duplicate->setMenuRole(QAction::NoRole);

	action_pagination = pagination_menu.menuAction();
	action_pagination->setIcon(QIcon(GuiUtilsNs::getIconPath("pagination")));
	action_pagination->setText(tr("Pagination"));

	action = pagination_menu.addAction(tr("Enable"));
	action->setData(true);
	connect(action, &QAction::triggered, this, &ModelWidget::togglePagination);

	action = pagination_menu.addAction(tr("Disable"));
	action->setData(false);
	connect(action, &QAction::triggered, this, &ModelWidget::togglePagination);

	action_jump_to_table = jump_to_tab_menu.menuAction();
	action_jump_to_table->setIcon(QIcon(GuiUtilsNs::getIconPath("jumptotable")));
	action_jump_to_table->setText(tr("Jump to table"));

	action_collapse_mode = toggle_attrs_menu.menuAction();
	action_collapse_mode->setIcon(QIcon(GuiUtilsNs::getIconPath("collapse")));
	action_collapse_mode->setText(tr("Collapse"));

	action_no_collapse_attribs=new QAction(tr("Not collapsed"), this);
	action_no_collapse_attribs->setData(BaseTable::NotCollapsed);
	action_collapse_ext_attribs=new QAction(tr("Extended attributes"), this);
	action_collapse_ext_attribs->setData(BaseTable::ExtAttribsCollapsed);
	action_collpase_all_attribs=new QAction(tr("All attributes"), this);
	action_collpase_all_attribs->setData(BaseTable::AllAttribsCollapsed);

	toggle_attrs_menu.addAction(action_no_collapse_attribs);
	toggle_attrs_menu.addAction(action_collapse_ext_attribs);
	toggle_attrs_menu.addAction(action_collpase_all_attribs);

	action_schemas_rects = toggle_sch_rects_menu.menuAction();
	action_schemas_rects->setIcon(QIcon(GuiUtilsNs::getIconPath("schemarect")));
	action_schemas_rects->setText(tr("Schemas rectangles"));
	action_show_schemas_rects=new QAction(tr("Show"), this);
	action_hide_schemas_rects=new QAction(tr("Hide"), this);
	toggle_sch_rects_menu.addAction(action_show_schemas_rects);
	toggle_sch_rects_menu.addAction(action_hide_schemas_rects);

	action_fade = fade_menu.menuAction();
	action_fade->setIcon(QIcon(GuiUtilsNs::getIconPath("fade")));
	action_fade->setText(tr("Fade in/out"));

	action_fade_in = new QAction(QIcon(GuiUtilsNs::getIconPath("fadein")), tr("Fade in"), this);
	action_fade_out = new QAction(QIcon(GuiUtilsNs::getIconPath("fadeout")), tr("Fade out"), this);

	action_fade_rels = fade_rels_menu.menuAction();
	action_fade_rels->setIcon(QIcon(GuiUtilsNs::getIconPath("relationship")));
	action_fade_rels->setText(tr("Relationships"));
	action_fade_rels_in=new QAction(QIcon(GuiUtilsNs::getIconPath("fadein")), tr("Fade in"), this);
	action_fade_rels_out=new QAction(QIcon(GuiUtilsNs::getIconPath("fadeout")), tr("Fade out"), this);

	action_fade_peer_tables = fade_peer_tables_menu.menuAction();
	action_fade_peer_tables->setIcon(QIcon(GuiUtilsNs::getIconPath("table")));
	action_fade_peer_tables->setText(tr("Peer tables"));
	action_fade_peer_tables_in = new QAction(QIcon(GuiUtilsNs::getIconPath("fadein")), tr("Fade in"), this);
	action_fade_peer_tables_out = new QAction(QIcon(GuiUtilsNs::getIconPath("fadeout")), tr("Fade out"), this);

	action_fade_tabs_rels = fade_both_objs_menu.menuAction();
	action_fade_tabs_rels->setText(tr("Tables && Relationships"));
	action_fade_tabs_rels_in = new QAction(QIcon(GuiUtilsNs::getIconPath("fadein")), tr("Fade in"), this);
	action_fade_tabs_rels_out = new QAction(QIcon(GuiUtilsNs::getIconPath("fadeout")), tr("Fade out"), this);

	fade_rels_menu.addAction(action_fade_rels_in);
	fade_rels_menu.addAction(action_fade_rels_out);

	fade_peer_tables_menu.addAction(action_fade_peer_tables_in);
	fade_peer_tables_menu.addAction(action_fade_peer_tables_out);

	fade_both_objs_menu.addAction(action_fade_tabs_rels_in);
	fade_both_objs_menu.addAction(action_fade_tabs_rels_out);

	action_edit_creation_order=new QAction(QIcon(GuiUtilsNs::getIconPath("swapobjs")), tr("Swap ids"), this);
	action_edit_creation_order->setToolTip(tr("Edit the objects creation order by swapping their ids"));
	connect(action_edit_creation_order, &QAction::triggered, this, &ModelWidget::swapObjectsIds);

	action_break_rel_line = break_rel_menu.menuAction();
	action_break_rel_line->setIcon(QIcon(GuiUtilsNs::getIconPath("breakrelline")));
	action_break_rel_line->setText(tr("Break line"));

	action=new QAction(QIcon(GuiUtilsNs::getIconPath("breakline_90dv")), tr("90° (vertical)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakVertNinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(GuiUtilsNs::getIconPath("breakline_90dh")), tr("90° (horizontal)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakHorizNinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(GuiUtilsNs::getIconPath("breakline_290dv")), tr("90° + 90° (vertical)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakVert2NinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(QIcon(GuiUtilsNs::getIconPath("breakline_290dh")), tr("90° + 90° (horizontal)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakHoriz2NinetyDegrees);
	break_rel_menu.addAction(action);

	//Alocatting the object creation actions
	for(auto &type : types_vect)
	{
		actions_new_objects[type]=new QAction(QIcon(GuiUtilsNs::getIconPath(type)), BaseObject::getTypeName(type), this);
		actions_new_objects[type]->setData(QVariant(enum_t(type)));
		connect(actions_new_objects[type], &QAction::triggered, this, &ModelWidget::addNewObject);
	}

	// Configuring the submenu of database level objects
	action_database_category = database_category_menu.menuAction();
	action_database_category->setIcon(QIcon(GuiUtilsNs::getIconPath(ObjectType::Database)));
	action_database_category->setText(tr("Database object"));
	types_vect = BaseObject::getChildObjectTypes(ObjectType::Database);

	for(auto &type : types_vect)
		database_category_menu.addAction(actions_new_objects[type]);

	// Configuring the submenu of schema level objects
	action_schema_category = schema_category_menu.menuAction();
	action_schema_category->setIcon(QIcon(GuiUtilsNs::getIconPath(ObjectType::Schema)));
	action_schema_category->setText(tr("Schema object"));
	types_vect = BaseObject::getChildObjectTypes(ObjectType::Schema);

	for(auto &type : types_vect)
		schema_category_menu.addAction(actions_new_objects[type]);

	//Creating the relationship submenu
	rels_menu=new QMenu(this);
	actions_new_objects[ObjectType::Relationship] = rels_menu->menuAction();
	actions_new_objects[ObjectType::Relationship]->setIcon(QIcon(GuiUtilsNs::getIconPath(ObjectType::Relationship)));
	actions_new_objects[ObjectType::Relationship]->setText(BaseObject::getTypeName(ObjectType::Relationship));
	actions_new_objects[ObjectType::Relationship]->setData(QVariant(enum_t(ObjectType::Relationship)));

	for(int i=0; i < rel_types_cod.size(); i++)
	{
		str_ico=BaseObject::getSchemaName(ObjectType::Relationship) + rel_types_cod[i];

		action=new QAction(QIcon(GuiUtilsNs::getIconPath(str_ico)),
							 BaseRelationship::getRelationshipTypeName(rel_types_id[i], false), this);

		//Storing a unique identifier for the relationship type
		action->setData(QVariant(enum_t(ObjectType::Relationship) + rel_types_id[i]));

		connect(action, &QAction::triggered, this, &ModelWidget::addNewObject);
		rels_menu->addAction(action);
	}

	new_obj_overlay_wgt=new NewObjectOverlayWidget(this);
	new_obj_overlay_wgt->setObjectName("new_obj_overlay_wgt");
	new_obj_overlay_wgt->setVisible(false);
	GuiUtilsNs::createDropShadow(new_obj_overlay_wgt, 5, 5, 20);

	std::vector<ObjectType> graph_types = { ObjectType::BaseObject, ObjectType::Schema, ObjectType::Table, ObjectType::ForeignTable,
																		 ObjectType::View, ObjectType::Relationship, ObjectType::Textbox };
	QStringList labels = { tr("All objects"), tr("Schemas"), tr("Tables"), tr("Foreign Tables"),
												 tr("Views"), tr("Relationships"), tr("Textboxes") };

	i=0;
	for(auto &obj_type : graph_types)
	{
		if(obj_type == ObjectType::BaseObject)
		{
			action=new QAction(labels[i++], this);
			action->setShortcut(QKeySequence(tr("Ctrl+A")));
			select_all_menu.addAction(action);
			select_all_menu.addSeparator();
		}
		else
		{
			action=new QAction(QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(obj_type))), labels[i++], this);
			select_all_menu.addAction(action);
		}

		action->setData(QVariant(enum_t(obj_type)));
		connect(action, &QAction::triggered, this, &ModelWidget::selectAllObjects);
	}

	action_stacking = stacking_menu.menuAction();
	action_stacking->setIcon(QIcon(GuiUtilsNs::getIconPath("stacking")));
	action_stacking->setText(tr("Stacking"));

	action_send_to_back=new QAction(QIcon(GuiUtilsNs::getIconPath("sendtoback")), tr("Send to back"), this);
	action_send_to_back->setShortcut(QKeySequence("Shift+Down"));

	action_bring_to_front=new QAction(QIcon(GuiUtilsNs::getIconPath("bringtofront")), tr("Bring to front"), this);
	action_bring_to_front->setShortcut(QKeySequence("Shift+Up"));

	stacking_menu.addAction(action_send_to_back);
	stacking_menu.addAction(action_bring_to_front);

	QAction *act = nullptr;

	std::vector<ObjectType> act_types {
				ObjectType::Schema, ObjectType::Table,
				ObjectType::ForeignTable, ObjectType::View,
				ObjectType::Relationship, ObjectType::Textbox };

	QStringList act_labels {
				tr("Schemas"), tr("Tables"),
				tr("Foreign tables"), tr("Views"),
				tr("Relationships"), tr("Textboxes")
	};

	unsigned id = 0;

	for(auto &act_type : act_types)
	{
		act = fade_in_all_menu.addAction(QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(act_type))),	act_labels[id]);
		act->setData(enum_t(act_type));
		connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);

		act = fade_out_all_menu.addAction(QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(act_type))),	act_labels[id]);
		act->setData(enum_t(act_type));
		connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);

		id++;
	}

	act = fade_in_all_menu.addAction(tr("All objects"));
	act->setData(enum_t(ObjectType::BaseObject));
	fade_in_all_menu.insertSeparator(act);
	connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	fade_in_all_menu.menuAction()->setText("Fade in");
	fade_in_all_menu.menuAction()->setIcon(QIcon(GuiUtilsNs::getIconPath("fadein")));

	act = fade_out_all_menu.addAction(tr("All objects"));
	act->setData(enum_t(ObjectType::BaseObject));
	fade_out_all_menu.insertSeparator(act);
	connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	fade_out_all_menu.menuAction()->setText("Fade out");
	fade_out_all_menu.menuAction()->setIcon(QIcon(GuiUtilsNs::getIconPath("fadeout")));

	connect(action_send_to_back, &QAction::triggered, this, &ModelWidget::sendToBack);
	connect(action_bring_to_front, &QAction::triggered, this, &ModelWidget::bringToFront);
	connect(action_edit_data, &QAction::triggered, this, &ModelWidget::editTableData);
	connect(&zoom_info_timer, &QTimer::timeout, zoom_info_lbl, &QLabel::hide);
	connect(action_source_code, &QAction::triggered, this, &ModelWidget::showSourceCode);
	connect(action_edit, &QAction::triggered, this, &ModelWidget::editObject);
	connect(action_protect, &QAction::triggered, this, &ModelWidget::protectObject);
	connect(action_unprotect, &QAction::triggered, this, &ModelWidget::protectObject);
	connect(action_select_all, &QAction::triggered, this, &ModelWidget::selectAllObjects);
	connect(action_convert_relnn, &QAction::triggered, this, &ModelWidget::convertRelationshipNN);
	connect(action_convert_rel1n, &QAction::triggered, this, &ModelWidget::convertRelationship1N);
	connect(action_deps_refs, &QAction::triggered, this, &ModelWidget::showDependenciesReferences);

	connect(action_paste, &QAction::triggered, this, __slot(this, ModelWidget::pasteObjects));
	connect(action_duplicate, &QAction::triggered, this, &ModelWidget::duplicateObject);

	connect(action_rename, &QAction::triggered, this, &ModelWidget::renameObjects);
	connect(action_edit_perms, &QAction::triggered, this, &ModelWidget::editPermissions);
	connect(action_sel_sch_children, &QAction::triggered, this, &ModelWidget::selectSchemaChildren);
	connect(action_sel_table_rels, &QAction::triggered, this, &ModelWidget::selectTableRelationships);
	connect(action_sel_tagged_tabs, &QAction::triggered, this, &ModelWidget::selectTaggedTables);
	connect(action_select_object, &QAction::triggered, this, &ModelWidget::highlightObject);
	connect(action_parent_rel, &QAction::triggered, this, &ModelWidget::editObject);
	connect(action_append_sql, &QAction::triggered, this, &ModelWidget::editCustomSQL);
	connect(action_create_seq_col, &QAction::triggered, this, &ModelWidget::createSequenceFromColumn);
	connect(action_conv_int_serial, &QAction::triggered, this, &ModelWidget::convertIntegerToSerial);
	connect(action_remove_rel_points, &QAction::triggered, this, &ModelWidget::removeRelationshipPoints);
	connect(action_enable_sql, &QAction::triggered, this, &ModelWidget::toggleObjectSQL);
	connect(action_disable_sql, &QAction::triggered, this, &ModelWidget::toggleObjectSQL);

	connect(action_remove, &QAction::triggered, this, [this](){
		removeObjects(false);
	});

	connect(action_cascade_del, &QAction::triggered, this, [this](){
		removeObjects(true);
	});

	connect(action_fade_in, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	connect(action_fade_out, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	connect(action_fade_rels_in, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	connect(action_fade_rels_out, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	connect(action_fade_peer_tables_in, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	connect(action_fade_peer_tables_out, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	connect(action_fade_tabs_rels_in, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	connect(action_fade_tabs_rels_out, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	connect(action_collapse_ext_attribs, &QAction::triggered, this, &ModelWidget::setCollapseMode);
	connect(action_collpase_all_attribs, &QAction::triggered, this, &ModelWidget::setCollapseMode);
	connect(action_no_collapse_attribs, &QAction::triggered, this, &ModelWidget::setCollapseMode);
	connect(action_show_schemas_rects, &QAction::triggered, this, &ModelWidget::toggleSchemasRectangles);
	connect(action_hide_schemas_rects, &QAction::triggered, this, &ModelWidget::toggleSchemasRectangles);
	connect(action_set_layer, &QAction::triggered, this, &ModelWidget::updateObjectsLayers);

	connect(layers_wgt, &LayersWidget::s_newLayerRequested, this, &ModelWidget::s_newLayerRequested);

	connect(db_model, &DatabaseModel::s_objectAdded, this, &ModelWidget::handleObjectAddition);
	connect(db_model, &DatabaseModel::s_objectRemoved, this, &ModelWidget::handleObjectRemoval);

	connect(scene, &ObjectsScene::s_objectsMoved, this, &ModelWidget::handleObjectsMovement);
	connect(scene, &ObjectsScene::s_objectModified, this,  &ModelWidget::handleObjectModification);
	connect(scene, &ObjectsScene::s_objectDoubleClicked, this,  &ModelWidget::handleObjectDoubleClick);

	connect(scene, &ObjectsScene::s_objectSelected, this,  &ModelWidget::configureObjectSelection, Qt::QueuedConnection);
	connect(scene, qOverload<BaseObject *>(&ObjectsScene::s_popupMenuRequested), this, qOverload<BaseObject *>(&ModelWidget::configurePopupMenu), Qt::QueuedConnection);
	connect(scene, qOverload<>(&ObjectsScene::s_popupMenuRequested), this,  &ModelWidget::showObjectMenu, Qt::QueuedConnection);
	connect(scene, &ObjectsScene::s_childrenSelectionChanged, this, &ModelWidget::configureObjectSelection);
	connect(scene, &ObjectsScene::s_objectsSelectedInRange, this, &ModelWidget::configureObjectSelection);

	connect(scene, &ObjectsScene::s_collapseModeChanged, this, [this](){
		setModified(true);
	});

	connect(scene, &ObjectsScene::s_paginationToggled, this, [this](){
		setModified(true);
	});

	connect(scene, &ObjectsScene::s_currentPageChanged, this, [this](){
		setModified(true);
	});

	connect(scene, &ObjectsScene::s_objectsMovedLayer, this, [this](){
		setModified(true);
	});

	connect(scene, &ObjectsScene::s_ensureVisibleRequested, this, [this](const QRectF &rect){
		viewport->ensureVisible(rect);
	});

	connect(scene, &ObjectsScene::s_sceneRectChanged, this, [this](const QRectF &rect){
		db_model->setSceneRect(rect);
		viewport->resetCachedContent();
		setModified(true);
	});

	connect(scene, &ObjectsScene::s_layersChanged, this, &ModelWidget::updateModelLayersInfo);
	connect(scene, &ObjectsScene::s_activeLayersChanged, this, &ModelWidget::updateModelLayersInfo);
	connect(scene, qOverload<BaseObject *>(&ObjectsScene::s_popupMenuRequested), new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(scene, qOverload<>(&ObjectsScene::s_popupMenuRequested), new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(scene, &ObjectsScene::s_objectSelected, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(scene, &ObjectsScene::s_childrenSelectionChanged, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(scene, &ObjectsScene::s_scenePressed, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);

	wheel_timer.setInterval(300);

	connect(&wheel_timer, &QTimer::timeout, this, [this](){
		finishSceneMove();
		wheel_timer.stop();
		wheel_move = false;
	});

	viewport->installEventFilter(this);
	viewport->horizontalScrollBar()->installEventFilter(this);
	viewport->verticalScrollBar()->installEventFilter(this);
}

ModelWidget::~ModelWidget()
{
	/* If there are copied/cutted objects that belongs to the database model
	 being destroyed, then the cut/copy operation are cancelled by emptying
	 the lists, avoiding crashes when trying to paste them */
	if((!copied_objects.empty() && copied_objects[0]->getDatabase() == db_model) ||
			(!cut_objects.empty() && cut_objects[0]->getDatabase() == db_model))
	{
		cut_operation = false;
		copied_objects.clear();
		cut_objects.clear();
	}

	popup_menu.clear();
	new_object_menu.clear();
	quick_actions_menu.clear();
	schemas_menu.clear();
	owners_menu.clear();
	tags_menu.clear();
	break_rel_menu.clear();

	delete scene;
}

void ModelWidget::setModified(bool value)
{
	this->modified = value;
	emit s_modelModified(value);
}

void ModelWidget::resizeEvent(QResizeEvent *)
{
	zoom_info_lbl->move((this->width()/2) - (zoom_info_lbl->width()/2),
											(this->height()/2)  - (zoom_info_lbl->height()/2));

	magnifier_area_lbl->move(viewport->width() - magnifier_area_lbl->width(),
													 viewport->height() - magnifier_area_lbl->height());

	adjustOverlayPosition();

	emit s_modelResized();
}

bool ModelWidget::eventFilter(QObject *object, QEvent *event)
{
	/* Emitting a signal when the scene successfully made a drag & drop event
	 * This signal is handled in main window so the mime data containing the url
	 * to dbm files can be correctly loaded */
	if(object == scene && (event->type() == QEvent::GraphicsSceneDragEnter ||
												 event->type() == QEvent::GraphicsSceneDragMove ||
												 event->type() == QEvent::GraphicsSceneDrop))
	{
		QGraphicsSceneDragDropEvent *drg_event = dynamic_cast<QGraphicsSceneDragDropEvent *>(event);

		if(event->type() == QEvent::GraphicsSceneDrop)
			emit s_sceneDragDropped(drg_event->mimeData());

		return true;
	}

	if(!panning_mode &&
		 (object == viewport->horizontalScrollBar() ||
			object == viewport->verticalScrollBar()))
	{
		if(event->type() == QEvent::MouseButtonPress)
		{
			startSceneMove();
		}
		else if(event->type() == QEvent::MouseButtonRelease)
		{
			finishSceneMove();
		}
		//Filters the Wheel event if it is raised by the viewport scrollbars
		else if(event->type() == QEvent::Wheel)
		{
			QWheelEvent *w_event=dynamic_cast<QWheelEvent *>(event);
			wheel_timer.start();

			if(!wheel_move)
			{
				startSceneMove();
				wheel_move = true;
				panning_mode = false;
			}

			/* If the vertical scroll bar is the receiver of the wheel event but the shift key is held
			 * we redirect the event to horizontal scroll bar so the viewport can be scrolled horizontally */
			if(object == viewport->verticalScrollBar() && w_event->modifiers() == Qt::ShiftModifier)
			{
				QWheelEvent we(w_event->position(), w_event->globalPosition(), w_event->pixelDelta(),
												w_event->angleDelta(), w_event->buttons(), Qt::NoModifier,
												w_event->phase(), w_event->inverted());
				qApp->sendEvent(viewport->horizontalScrollBar(), &we);
				return true;
			}

			if(w_event->modifiers() != Qt::ControlModifier)
				return false;

			// Zooming in/out the scene when the control key is held
			double zoom_inc = round(fabs(w_event->angleDelta().y()/120.0)) * ZoomIncrement;

			if(w_event->angleDelta().y() < 0)
				this->applyZoom(this->current_zoom - zoom_inc);
			else
				this->applyZoom(this->current_zoom + zoom_inc);

			return true;
		}
	}
	else if(object == magnifier_area_lbl && event->type() == QEvent::MouseMove)
	{
		updateMagnifierArea();
		return true;
	}
	else if(object == scene)
	{
		QGraphicsSceneMouseEvent *m_event = dynamic_cast<QGraphicsSceneMouseEvent *>(event);

		if(!m_event)
			return false;

		if(m_event->button() != Qt::NoButton)
			new_obj_overlay_wgt->hide();

		if(event->type() == QEvent::GraphicsSceneMouseMove)
		{
			emit s_sceneInteracted(m_event->scenePos());

			if(!selected_objects.empty())
			{
				if(selected_objects.size() == 1)
				{
					BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(selected_objects[0]);
					emit s_sceneInteracted(graph_obj ? dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject()) : nullptr);
				}
				else
					emit s_sceneInteracted(static_cast<int>(selected_objects.size()), scene->itemsBoundingRect(true, true));
			}
		}

		//Forcing the panning mode using the middle mouse button
		if(m_event->buttons() == Qt::MiddleButton && event->type() == QEvent::GraphicsSceneMouseMove)
		{
			if(!panning_mode)
			{
				panning_mode = true;
				wheel_move = false;

				//Forcing the closed hand cursor because the default behavior of panning mode in QGraphicsView is to set an open hand cursor
				qApp->setOverrideCursor(Qt::ClosedHandCursor);
			}

			QPointF pos = m_event->lastScreenPos() - m_event->screenPos();
			int dx = viewport->horizontalScrollBar()->value() + pos.x(),
					dy = viewport->verticalScrollBar()->value() + pos.y();

			viewport->horizontalScrollBar()->setValue(dx);
			viewport->verticalScrollBar()->setValue(dy);

			return true;
		}

		if(m_event->button() == Qt::NoButton && event->type() == QEvent::GraphicsSceneMouseMove && magnifier_area_lbl->isVisible())
		{
			updateMagnifierArea();
		}
		//Activating the panning mode
		else if(m_event->button() == Qt::MiddleButton && event->type() == QEvent::GraphicsSceneMousePress)
		{
			startSceneMove();
			viewport->setDragMode(QGraphicsView::ScrollHandDrag);
			qApp->restoreOverrideCursor();
			qApp->setOverrideCursor(Qt::OpenHandCursor);
			return true;
		}
		//Deactivating the panning mode
		else if(m_event->button() == Qt::MiddleButton && event->type() == QEvent::GraphicsSceneMouseRelease)
		{
			panning_mode = false;
			viewport->setDragMode(QGraphicsView::NoDrag);
			finishSceneMove();

			qApp->restoreOverrideCursor();
			qApp->restoreOverrideCursor();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

void ModelWidget::keyPressEvent(QKeyEvent *event)
{
	if(event->key()==Qt::Key_Escape)
	{
		if(new_obj_overlay_wgt->isVisible())
			new_obj_overlay_wgt->hide();
		else
		{
			this->cancelObjectAddition();

			if(!scene->isMovingObjects())
				scene->clearSelection();
		}
	}
	else if(event->key()==Qt::Key_N)
	{
		toggleNewObjectOverlay();
	}
}

void ModelWidget::mousePressEvent(QMouseEvent *event)
{
	if(event->buttons()==Qt::LeftButton)
	{
		/* If the user is adding a graphical object, the left click will set the initial position and
		show the editing form related to the object type */
		if(!simple_obj_creation && (BaseTable::isBaseTable(new_obj_type) || new_obj_type==ObjectType::Textbox))
		{
			this->scene->enableRangeSelection(false);
			this->showObjectForm(new_obj_type, nullptr, nullptr, viewport->mapToScene(event->pos()));
			this->cancelObjectAddition();
			this->scene->enableRangeSelection(true);
		}
	}
}

void ModelWidget::hideEvent(QHideEvent *)
{
	showMagnifierArea(false);
}

bool ModelWidget::saveLastCanvasPosition()
{
	if(save_restore_pos)
	{
		QScrollBar *hscroll=viewport->horizontalScrollBar(),
				*vscroll=viewport->verticalScrollBar();
		QPoint pos=db_model->getLastPosition();

		//Save the zoom or position only one of these attributes has changed
		if(db_model->getLastZoomFactor()!=current_zoom ||
				pos.x()!=hscroll->value() || pos.y()!=vscroll->value())
		{
			db_model->setLastPosition(QPoint(viewport->horizontalScrollBar()->value(),
																			 viewport->verticalScrollBar()->value()));
			db_model->setLastZoomFactor(this->current_zoom);
			return true;
		}
	}

	return false;
}

void ModelWidget::setUpdatesEnabled(bool value)
{
	viewport->setUpdatesEnabled(value);
	QWidget::setUpdatesEnabled(value);
}

void ModelWidget::restoreLastCanvasPosition()
{
	if(save_restore_pos)
	{
		QScrollBar *hscroll=viewport->horizontalScrollBar(),
				*vscroll=viewport->verticalScrollBar();

		if(db_model->getLastZoomFactor() != 1.0)
			this->applyZoom(db_model->getLastZoomFactor());

		hscroll->setValue(db_model->getLastPosition().x());
		vscroll->setValue(db_model->getLastPosition().y());

		scene->update();
	}
}

void ModelWidget::applyZoom(double zoom)
{
	if(zoom < MinimumZoom)
		zoom = MinimumZoom;
	else if(zoom > MaximumZoom)
		zoom = MaximumZoom;

	viewport->resetTransform();
	viewport->scale(zoom, zoom);
	this->current_zoom=zoom;

	zoom_info_lbl->setText(tr("Zoom: %1%").arg(QString::number(this->current_zoom * 100, 'g' , 3)));
	zoom_info_lbl->setVisible(true);
	zoom_info_timer.start();

	if(current_zoom >= 1 && magnifier_area_lbl->isVisible())
		showMagnifierArea(false);

	emit s_zoomModified(zoom);
}

double ModelWidget::getCurrentZoom()
{
	return current_zoom;
}

void ModelWidget::handleObjectAddition(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		ObjectType obj_type = graph_obj->getObjectType();
		QGraphicsItem *item = nullptr;

		switch(obj_type)
		{
			case ObjectType::ForeignTable:
			case ObjectType::Table:
				item = new TableView(dynamic_cast<PhysicalTable *>(graph_obj));
			break;

			case ObjectType::View:
				item = new GraphicalView(dynamic_cast<View *>(graph_obj));
			break;

			case ObjectType::Relationship:
			case ObjectType::BaseRelationship:
				item = new RelationshipView(dynamic_cast<BaseRelationship *>(graph_obj));
			break;

			case ObjectType::Schema:
				item = new SchemaView(dynamic_cast<Schema *>(graph_obj));
			break;

			default:
				item = new StyledTextboxView(dynamic_cast<Textbox *>(graph_obj));
			break;
		}

		if(item)
		{
			scene->addItem(item, blink_new_objs);
			setModified(true);
			emit s_objectAdded(graph_obj);

			if(blink_new_objs)
				viewport->centerOn(item);
		}
	}
}

void ModelWidget::addNewObject()
{
	QAction *action=dynamic_cast<QAction *>(sender());

	if(action)
	{
		BaseObject *parent_obj=nullptr;
		ObjectType obj_type=static_cast<ObjectType>(action->data().toUInt());

		/* If the user wants to add a table object or a object inside a schema
		uses as parent object the selected object, because the user only can add
		these types after select a table or schema, respectively */
		if(selected_objects.size()==1 &&
				(TableObject::isTableObject(obj_type) ||
				 selected_objects[0]->getObjectType()==ObjectType::Schema))
			parent_obj=selected_objects[0];

		//Creating a table or view inside a schema
		if(parent_obj && parent_obj->getObjectType()==ObjectType::Schema && BaseTable::isBaseTable(obj_type))
		{
			BaseObjectView *sch_graph=dynamic_cast<BaseObjectView *>(dynamic_cast<Schema *>(parent_obj)->getOverlyingObject());
			QSizeF size = sch_graph->boundingRect().size();
			QPointF pos, menu_pos = viewport->mapToScene(this->mapFromGlobal(popup_menu.pos()));
			QRectF rect = QRectF(sch_graph->pos(), size);

			//Uses the popup menu position if it is inside the bounding rectangle
			if(rect.contains(menu_pos))
				pos=menu_pos;
			//Otherwise inserts the new object at the middle of bounding rect
			else
				pos=QPointF(sch_graph->pos().x() + (size.width()/2.0),
							sch_graph->pos().y() + (size.height()/2.0));

			this->showObjectForm(obj_type, nullptr, parent_obj, pos);
		}
		else if(!BaseTable::isBaseTable(obj_type) &&
						obj_type!=ObjectType::Textbox && obj_type <= ObjectType::BaseTable)
			this->showObjectForm(obj_type, nullptr, parent_obj);
		else
		{
			/* A small checking to enable the overlay widget to create relationships and
		 other graphical objects without the user click on the canvas area */
			if((obj_type > ObjectType::BaseObject &&
				selected_objects.size()==2 &&
				selected_objects.at(0)->getObjectType()==ObjectType::Table &&
				selected_objects.at(1)->getObjectType()==ObjectType::Table))
			{
				this->showObjectForm(obj_type);
			}
			else
			{
				//Simple table|view|textbox creation
				if(simple_obj_creation &&
						(BaseTable::isBaseTable(obj_type) || obj_type==ObjectType::Textbox))
				{
					QPointF pos = viewport->mapToScene(viewport->rect().center());
					this->showObjectForm(obj_type, nullptr, parent_obj, pos);

					/* If the calculated point (in scene coordinate) is
					 * not contained by the scene rect we adjust the scene rect to avoid
					 * the object to be placed outside the scene boundaries */
					if(!viewport->scene()->sceneRect().contains(pos))
						adjustSceneRect(true, true);
				}
				else
				{
					//For the graphical object, changes the cursor icon until the user click on the model to show the editing form
					QPixmap act_ico = action->icon().pixmap(QSize(32, 32)),
							cursor_ico = QPixmap(GuiUtilsNs::getIconPath("cursortmpl")).scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
					QPainter painter;

					painter.begin(&cursor_ico);
					painter.setPen(QPen(QColor(0,0,0), 2));
					painter.drawPixmap(QRect(16, 16, 32, 32), act_ico);
					painter.end();

					viewport->setCursor(QCursor(cursor_ico, 0, 0));
					this->new_obj_type=obj_type;
					this->enableModelActions(false);

					/* If a single table is selected and the user triggered a relationship creation action via popup menu,
					 * we force the enabling of the relationship creation steps. This will automatically selects the current table
					 * as source table of the relationship */
					if(selected_objects.size() == 1 &&
						 PhysicalTable::isPhysicalTable(selected_objects[0]->getObjectType()) && new_obj_type > ObjectType::BaseTable)
						configureObjectSelection();
				}
			}
		}
	}
}

void ModelWidget::handleObjectRemoval(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(graph_obj)
	{
		scene->removeItem(dynamic_cast<QGraphicsItem *>(graph_obj->getOverlyingObject()));

		//Updates the parent schema if the removed object were a table or view
		if(graph_obj->getSchema() &&
				(graph_obj->getObjectType()==ObjectType::Table || graph_obj->getObjectType()==ObjectType::View))
			dynamic_cast<Schema *>(graph_obj->getSchema())->setModified(true);
	}

	setModified(true);
}

void ModelWidget::handleObjectDoubleClick(BaseGraphicObject *object)
{
	if(object)
		this->showObjectForm(object->getObjectType(), object, nullptr, object->getPosition());
	else
		this->showObjectForm(ObjectType::Database, db_model);
}

void ModelWidget::handleObjectsMovement(bool end_moviment)
{
	std::vector<BaseObject *>::iterator itr, itr_end;
	std::vector<BaseObject *> reg_tables;
	QList<BaseObjectView *> tables;
	BaseGraphicObject *obj=nullptr;
	Schema *schema=nullptr;

	itr=selected_objects.begin();
	itr_end=selected_objects.end();

	if(!end_moviment)
	{
		op_list->startOperationChain();

		while(itr!=itr_end)
		{
			obj=dynamic_cast<BaseGraphicObject *>(*itr);
			itr++;
			if(!obj)	continue;

			if(!dynamic_cast<BaseRelationship *>(obj) && (obj && !obj->isProtected()))
			{
				schema=dynamic_cast<Schema *>(obj);

				//Register the object if it is not a schema or a table already registered
				if(!schema && std::find(reg_tables.begin(), reg_tables.end(), obj)==reg_tables.end())
					op_list->registerObject(obj, Operation::ObjMoved);
				else if(schema)
				{
					//For schemas, when they are moved, the original position of tables are registered instead of the position of schema itself
					tables=dynamic_cast<SchemaView *>(schema->getOverlyingObject())->getChildren();
					for(auto &tab : tables)
					{
						op_list->registerObject(tab->getUnderlyingObject(), Operation::ObjMoved);

						//Registers the table on a auxiliary list to avoid multiple registration on operation history
						reg_tables.push_back(tab->getUnderlyingObject());
					}
				}
			}			
		}
	}
	else
	{
		std::vector<Schema *> schemas;

		while(itr!=itr_end)
		{
			obj = dynamic_cast<BaseGraphicObject *>(*itr);
			itr++;
			if(!obj) continue;

			if(BaseTable::isBaseTable(obj->getObjectType()))
			{
				Schema *schema=dynamic_cast<Schema *>(dynamic_cast<BaseTable *>(obj)->getSchema());

				//Update the schema if this isn't done yet
				if(std::find(schemas.begin(),schemas.end(),schema)==schemas.end())
				{
					schema->setModified(true);

					//Insert the updated schema to a list to avoid a second (unnecessary) update
					schemas.push_back(schema);
				}
			}
		}

		op_list->finishOperationChain();
		setModified(true);

		emit s_objectsMoved();
	}
}

void ModelWidget::handleObjectModification(BaseGraphicObject *object)
{
	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: limiting the user interaction over objects."
		if(updateObjActionCounter())
			return;
	#endif

	op_list->registerObject(object, Operation::ObjModified);
	setModified(true);
	emit s_objectModified();
}

void ModelWidget::emitSceneInteracted()
{
	if(selected_objects.empty())
		emit s_sceneInteracted(nullptr);
	else if(selected_objects.size() == 1)
	{
		BaseGraphicObject *base_obj = dynamic_cast<BaseGraphicObject *>(selected_objects[0]);
		TableObject *tab_obj=dynamic_cast<TableObject *>(selected_objects[0]);

		if(base_obj)
			emit s_sceneInteracted(dynamic_cast<BaseObjectView *>(base_obj->getOverlyingObject()));
		else if(tab_obj)
			emit s_sceneInteracted(1, QRect());
		else
			emit s_sceneInteracted(nullptr);
	}
	else
		emit s_sceneInteracted(static_cast<int>(selected_objects.size()), scene->itemsBoundingRect(true, true));
}

void ModelWidget::setProtected(bool protect)
{
	QLabel *msg_lbl = protected_model_frm->findChild<QLabel *>("message_lbl");
	msg_lbl->setText(tr("<strong>ATTENTION:</strong> The database model is protected. Modifications are disabled!"));

	protected_model_frm->setVisible(protect);
	db_model->setProtected(protect);
}

void ModelWidget::setInteractive(bool interact)
{
	QLabel *msg_lbl = protected_model_frm->findChild<QLabel *>("message_lbl");
	msg_lbl->setText(tr("<strong>ATTENTION:</strong> The database model is being accessed by another operation (diff or import). Modifications are temporarily disabled until the operation finishes!"));

	protected_model_frm->setVisible(!interact);
	viewport->setInteractive(interact);
	new_obj_overlay_wgt->setEnabled(interact);

	for(auto &act : popup_menu.actions())
		act->setEnabled(interact);

	emit s_interactiveChanged(interact);
}

bool ModelWidget::isInteractive()
{
	return viewport->isInteractive();
}

void ModelWidget::startSceneMove()
{
	if(scene_moving)
		return;

	scene_moving = true;
	curr_show_grid = ObjectsScene::isShowGrid();
	curr_show_delim = ObjectsScene::isShowPageDelimiters();
	ObjectsScene::setShowGrid(false);
	ObjectsScene::setShowPageDelimiters(false);
	viewport->resetCachedContent();
	scene->setShowSceneLimits(false);
}

void ModelWidget::finishSceneMove()
{
	scene_moving = false;
	ObjectsScene::setShowGrid(curr_show_grid);
	ObjectsScene::setShowPageDelimiters(curr_show_delim);
	viewport->resetCachedContent();
	scene->setShowSceneLimits(true);
	scene->invalidate(viewport->sceneRect());
}

void ModelWidget::setBlinkAddedObjects(bool value)
{
	blink_new_objs = value;
}

void ModelWidget::configureObjectSelection()
{
	QList<QGraphicsItem *> items=scene->selectedItems();
	BaseObjectView *item=nullptr;
	std::map<unsigned, QGraphicsItem *> objs_map;
	std::map<unsigned, QGraphicsItem *>::iterator itr;

	selected_objects.clear();

	//Stores in a map each selected graphical object on the scene
	while(!items.isEmpty())
	{
		//Only store object that can be converted to BaseObjectView
		item=dynamic_cast<BaseObjectView *>(items.front());
		items.pop_front();

		if(item)
			objs_map[item->getSelectionOrder()]=item;
	}

	itr=objs_map.begin();
	while(itr!=objs_map.end())
	{
		item=dynamic_cast<BaseObjectView *>(itr->second);
		selected_objects.push_back(item->getUnderlyingObject());
		itr++;
	}

	/* Case the new_obj_type is a value greater the ObjectType::ObjBaseTable indicates that the user
	(un)selected a object using some "Relationship" action */
	if(new_obj_type > ObjectType::BaseTable)
	{
		unsigned count=selected_objects.size();
		ObjectType obj_type1, obj_type2;

		//If there is more than 2 object select cancel the operation
		if(count > 2 || count==0)
			this->cancelObjectAddition();
		else if(count >=1 && count <=2)
		{
			//Get the selected objects types
			obj_type1=selected_objects[0]->getObjectType();
			obj_type2=(count==2 ? selected_objects[1]->getObjectType() : ObjectType::BaseObject);

			//If there is only one selected object and this is a table, activates the relationship creation
			if(!scene->isRelationshipLineVisible() &&
				 count==1 && PhysicalTable::isPhysicalTable(obj_type1) &&
				 new_obj_type > ObjectType::BaseTable &&	QApplication::keyboardModifiers() == 0)
			{
				BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(selected_objects[0]);
				BaseObjectView *object=dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

				scene->showRelationshipLine(true,
																		QPointF(object->scenePos().x() + (object->boundingRect().width() / 2),
																						object->scenePos().y() + (object->boundingRect().height() / 2)));
			}
			//If the user has selected object that are not tables, cancel the operation
			else if(!PhysicalTable::isPhysicalTable(obj_type1) || (!PhysicalTable::isPhysicalTable(obj_type2) && obj_type2 != ObjectType::BaseObject))
			{
				this->cancelObjectAddition();
			}

			/* Case there is only one selected object (table) and the SHIFT key is pressed too, creates a self-relationship.
				 Case there is two selected objects, create a relationship between them */
			else if((count==1 && PhysicalTable::isPhysicalTable(obj_type1) && QApplication::keyboardModifiers()==Qt::ShiftModifier) ||
							(count==2 && PhysicalTable::isPhysicalTable(obj_type1) && PhysicalTable::isPhysicalTable(obj_type2)))
			{
				/* Forcing no signals to be emitted by the scene while the relationship is being configured to avoid this
				 * method to be called unecessarily */
				scene->blockSignals(true);
				this->showObjectForm(new_obj_type);
				scene->blockSignals(false);

				//Cancels the operation after showing the relationship editing form
				scene->clearSelection();
				this->cancelObjectAddition();
			}
		}
	}
	else
		this->configurePopupMenu(selected_objects);

	emitSceneInteracted();
}

void ModelWidget::selectAllObjects()
{
	QAction *act = qobject_cast<QAction *>(sender());

	if(!act)
		return;

	ObjectType obj_type = static_cast<ObjectType>(act->data().toUInt());

	if(obj_type == ObjectType::BaseObject)
	{
		QPainterPath pth;
		pth.addRect(scene->sceneRect());

		scene->blockItemsSignals(true);
		scene->setSelectionArea(pth);
		scene->blockItemsSignals(false);
	}
	else
	{
		BaseObjectView *obj_view = nullptr;
		std::vector<BaseObject *> objs = *db_model->getObjectList(obj_type);

		if(obj_type == ObjectType::Relationship)
			objs.insert(objs.end(), db_model->getObjectList(ObjectType::BaseRelationship)->begin(),  db_model->getObjectList(ObjectType::BaseRelationship)->end());

		for(auto &obj : objs)
		{
			obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(obj)->getOverlyingObject());

			if(obj_view)
			{
				obj_view->blockSignals(true);
				obj_view->setSelected(true);
				obj_view->blockSignals(false);
			}
		}
	}

	configureObjectSelection();
}

void ModelWidget::convertRelationshipNN()
{
	Relationship *rel=reinterpret_cast<Relationship *>(action_convert_relnn->data().value<void *>());

	if(rel)
	{
		//Converts only Many-to-Many relationship
		if(rel->getRelationshipType()==Relationship::RelationshipNn)
		{
			int res = Messagebox::confirm(tr("Do you really want to convert the relationship into an intermediate table?"));

			if(Messagebox::isRejected(res))
				return;

			unsigned op_count=0;

			try
			{
				Relationship *rel1 = nullptr, *rel2 = nullptr;
				Table *tab = nullptr, *tab_nn = nullptr,
						*src_tab = dynamic_cast<Table *>(rel->getTable(Relationship::SrcTable)),
						*dst_tab = dynamic_cast<Table *>(rel->getTable(Relationship::DstTable));
				Constraint *constr = nullptr, *aux_constr = nullptr, *pk = nullptr;
				Column *col = nullptr;
				bool src_mand = true,	dst_mand = true, self_rel = rel->isSelfRelationship();
				QString tab_name, xml_buf;
				QPointF pnt;
				std::vector<Constraint *> fks;
				std::vector<QString> pk_cols;
				QList<unsigned> layers = rel->getLayers();
				int attr_idx = -1;
				QColor rel_color = rel->getCustomColor();

				op_count = op_list->getCurrentSize();

				//Stores the XML code definition for the table generated by the relationship
				tab_nn = dynamic_cast<Table *>(rel->getReceiverTable());
				pk = tab_nn->getPrimaryKey();

				if(!self_rel)
				{
					/* Checking if there is some attribute that is a pk too. If so, store their
					 * names in a list in order to create the pk further in this method */
					for(auto &col : pk->getColumns(Constraint::SourceCols))
					{
						attr_idx = rel->getObjectIndex(col);

						if(attr_idx >= 0)
							pk_cols.push_back(col->getName());
					}
				}

				xml_buf = tab_nn->getSourceCode(SchemaParser::XmlCode);

				//Creates the table from the xml code
				xmlparser->restartParser();
				xmlparser->loadXMLBuffer(xml_buf);
				tab = db_model->createTable();
				tab_name = tab->getName();

				//Forcing the creation of the single pk column
				if(rel->isSiglePKColumn())
				{
					col = pk->getColumn(0, Constraint::SourceCols);
					pk_cols.push_back(col->getName());
				}

				if(self_rel)
				{
					//Copy the columns from the table generated by relationship to the newly created table
					for(auto &col_aux : *tab_nn->getObjectList(ObjectType::Column))
					{
						col = new Column;
						(*col) = (*dynamic_cast<Column *>(col_aux));
						col->setParentTable(nullptr);
						tab->addColumn(col);
					}

					/* Copy the constraints from the table generated by relationship to the
					 * newly created table using the xml definition */
					for(auto &constr_aux : *tab_nn->getObjectList(ObjectType::Constraint))
					{
						xml_buf = dynamic_cast<Constraint *>(constr_aux)->getSourceCode(SchemaParser::XmlCode, true);
						xmlparser->restartParser();
						xmlparser->loadXMLBuffer(xml_buf);
						constr = db_model->createConstraint(tab);
						tab->addConstraint(constr);
					}

					tab->getForeignKeys(fks, false, src_tab);
				}
				else
				{
					//Copies the relationship attributes to the created table
					for(auto &attr : rel->getAttributes())
					{
						col = new Column;
						(*col)=(*dynamic_cast<Column *>(attr));
						col->setParentTable(nullptr);
						tab->addColumn(col);
					}

					//Copies the relationship constraints to the created table
					for(auto &rel_constr : rel->getConstraints())
					{
						constr = new Constraint;
						aux_constr = dynamic_cast<Constraint *>(rel_constr);
						(*constr)=(*aux_constr);
						constr->removeColumns();
						constr->setParentTable(nullptr);

						for(unsigned col_id = Constraint::SourceCols; col_id <= Constraint::ReferencedCols; col_id++)
						{
							constr->addColumns(aux_constr->getColumns(static_cast<Constraint::ColumnsId>(col_id)),
																 static_cast<Constraint::ColumnsId>(col_id));
						}

						tab->addConstraint(constr);
					}
				}

				//Renames the table if there is other with the same name on the model avoiding conflicts
				tab->setName(tab_name);
				tab->setName(CoreUtilsNs::generateUniqueName(tab, *db_model->getObjectList(ObjectType::Table)));

				op_list->startOperationChain();

				//Removes the many-to-many relationship from the model
				op_list->registerObject(rel, Operation::ObjRemoved);

				//The default position for the table will be the middle point between the relationship participant tables
				pnt.setX((src_tab->getPosition().x() + dst_tab->getPosition().x())/2.0);
				pnt.setY((src_tab->getPosition().y() + dst_tab->getPosition().y())/2.0);
				tab->setPosition(pnt);

				//Adds the new table to the model
				db_model->addObject(tab);
				op_list->registerObject(tab, Operation::ObjCreated);

				if(self_rel)
				{
					for(auto &fk : fks)
						op_list->registerObject(fk, Operation::ObjCreated, -1, fk->getParentTable());
				}
				//If not self relationship creates two 1:n relationships
				else
				{
					//Creating the pk based upon the attributes of the relationship
					if(!pk_cols.empty())
					{
						aux_constr = new Constraint;

						for(QString pk_col : pk_cols)
							aux_constr->addColumn(tab->getColumn(pk_col), Constraint::SourceCols);

						aux_constr->setName(CoreUtilsNs::generateUniqueName(tab, *tab->getObjectList(ObjectType::Constraint), false, "_pk"));
						tab->addConstraint(aux_constr);

						op_list->registerObject(aux_constr, Operation::ObjCreated, -1, tab);
					}

					/* Creates a one-to-many relationship that links the source table of the many-to-many rel. to the created table
						The relationship will be identifier if the single pk column attribute of the original relationship is false */
					rel1 = new Relationship(Relationship::Relationship1n,
																	src_tab, tab, src_mand, false,
																	!rel->isSiglePKColumn());
					rel1->setLayers(layers);
					rel1->setCustomColor(rel_color);
					rel1->setLayers(layers);

					db_model->addRelationship(rel1);
					op_list->registerObject(rel1, Operation::ObjCreated);

					/*Creates a one-to-many relationship that links the destination table of the many-to-many rel. to the created table
						The relationship will be identifier if the single pk column attribute of the original relationship is false */
					rel2 = new Relationship(Relationship::Relationship1n,
																	dst_tab, tab, dst_mand,
																	false, !rel->isSiglePKColumn());
					rel2->setLayers(layers);
					rel2->setCustomColor(rel_color);
					rel2->setLayers(layers);

					db_model->addRelationship(rel2);
					op_list->registerObject(rel2, Operation::ObjCreated);
				}

				op_list->finishOperationChain();

				//Removes the n:n relationship after convert it
				db_model->removeObject(rel);

				if(self_rel)
				{
					db_model->updateTableFKRelationships(tab);

					for(auto &aux_rel : db_model->getRelationships(tab))
					{
						aux_rel->setLayers(layers);
						aux_rel->setCustomColor(rel_color);
					}
				}

				emit s_objectCreated();
			}
			catch(Exception &e)
			{
				if(op_count < op_list->getCurrentSize())
				{
					unsigned qtd = op_list->getCurrentSize() - op_count;
					op_list->ignoreOperationChain(true);

					for(unsigned i = 0; i < qtd; i++)
					{
						op_list->undoOperation();
						op_list->removeLastOperation();
					}

					op_list->ignoreOperationChain(false);
				}

				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}
	}
}

void ModelWidget::convertRelationship1N()
{
	Relationship *rel=reinterpret_cast<Relationship *>(action_convert_rel1n->data().value<void *>());

	if(!rel || (rel &&
							rel->getRelationshipType() != Relationship::Relationship11 &&
							rel->getRelationshipType() != Relationship::Relationship1n))
		return;

	Messagebox msg_box;

	msg_box.show(tr("<strong>WARNING:</strong> Converting a <strong>one-to-one</strong> or <strong>one-to-many</strong>\
 relationship can lead to unreversible changes or break other relationships in the linking chain! Do you want to proceed?"),
							 Messagebox::Alert, Messagebox::YesNoButtons);

	if(msg_box.isRejected())
		return;

	unsigned op_count = op_list->getCurrentSize();

	try
	{
		Table *recv_tab = dynamic_cast<Table *>(rel->getReceiverTable()),
				*ref_tab = dynamic_cast<Table *>(rel->getReferenceTable());
		QStringList constrs_xmls;
		Column *column = nullptr;		
		Constraint *constr = nullptr, *pk = recv_tab->getPrimaryKey();
		std::vector<Column *> columns;
		QString pk_name, rel_name = rel->getName(), fk_index_xml;
		bool register_pk = false;
		QColor rel_color = rel->getCustomColor();
		QList<unsigned> layers = rel->getLayers();

		/* Storing the XML definition of table's PK if it was added by the relationship being converted
		 * or the PK wasn't created by a relationship but references columns that were added by the relationship */
		if(pk &&
				((!pk->isAddedByRelationship() && pk->isReferRelationshipAddedColumns(rel->getGeneratedColumns())) ||
					(pk->isAddedByRelationship() && pk->getParentRelationship() == rel)))
		{
			/* This flag indicates that the pk removal (further in this method) should be registered in the operations list
			 * This will happen only if the pk wasn't added by the relationship */
			register_pk = !pk->isAddedByRelationship();
			pk_name = pk->getName();

			// Storing the pk XML definition so it can be recreated correctly even after the disconnection of all relationships
			constrs_xmls.append(recv_tab->getPrimaryKey()->getSourceCode(SchemaParser::XmlCode, true));
		}

		// Stores the XML definition of all generated constraints
		for(auto &constr : rel->getGeneratedConstraints())
		{
			if(constr->getConstraintType() == ConstraintType::PrimaryKey)
				continue;

			constrs_xmls.append(constr->getSourceCode(SchemaParser::XmlCode, true));
		}

		// Stores the XML definition of all relationship's constraints (added by the user)
		for(auto &obj : rel->getConstraints())
		{
			constr = dynamic_cast<Constraint *>(obj);
			constrs_xmls.append(constr->getSourceCode(SchemaParser::XmlCode, true));
		}

		// Copying all generated columns
		for(auto &col : rel->getGeneratedColumns())
		{
			column = new Column;
			*column = *col;
			columns.push_back(column);
		}

		// Copying all relationship attributes
		for(auto &col : rel->getAttributes())
		{
			column = new Column;
			*column = *(dynamic_cast<Column *>(col));
			columns.push_back(column);
		}

		if(rel->getGeneratedIndex())
			fk_index_xml = rel->getGeneratedIndex()->getSourceCode(SchemaParser::XmlCode);

		qApp->setOverrideCursor(Qt::WaitCursor);
		op_list->startOperationChain();

		db_model->storeSpecialObjectsXML();
		db_model->disconnectRelationships();
		db_model->__removeObject(rel);

		// Register the exclusion of the original relationship
		op_list->registerObject(rel, Operation::ObjRemoved);

		/* If after the relationships disconnection the table still have a PK
		 * it means that it was not added by relationship so we can remove it from the table
		 * so it can be recreated further with the correct settings */
		 pk = recv_tab->getConstraint(pk_name);

		if(pk)
		{
			if(register_pk)
				op_list->registerObject(pk, Operation::ObjRemoved, -1, recv_tab);

			recv_tab->removeObject(pk);
		}

		// Adding the copied columns to the receiver table
		for(auto &col : columns)
		{
			col->setParentRelationship(nullptr);
			col->setParentTable(nullptr);
			recv_tab->addColumn(col);
			op_list->registerObject(col, Operation::ObjCreated, - 1, recv_tab);
		}

		// Recreating the constraints from XML code
		for(auto &constr_xml : constrs_xmls)
		{
			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(constr_xml);
			constr = db_model->createConstraint(recv_tab);
			recv_tab->addConstraint(constr);
			op_list->registerObject(constr, Operation::ObjCreated, - 1, recv_tab);
		}

		if(!fk_index_xml.isEmpty())
		{
			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(fk_index_xml);
			Index *index = db_model->createIndex();
			op_list->registerObject(index, Operation::ObjCreated, - 1, recv_tab);
		}

		/* Resetting the relatinship added columns/constraint indexes in the table
		 * in order to avoid invalid indexes causing crashes */
		recv_tab->resetRelObjectsIndexes();
		recv_tab->setModified(true);

		db_model->validateRelationships();
		db_model->updateTableFKRelationships(recv_tab);

		// Setting up the same name and color of the generated relationship
		BaseRelationship *fk_rel = db_model->getRelationship(recv_tab, ref_tab);
		fk_rel->setName(rel_name);
		fk_rel->setCustomColor(rel_color);
		fk_rel->setLayers(layers);
		fk_rel->setModified(true);

		op_list->registerObject(fk_rel, Operation::ObjModified);
		op_list->finishOperationChain();

		qApp->restoreOverrideCursor();
		emit s_objectCreated();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();

		if(op_count < op_list->getCurrentSize())
		{
			unsigned qtd = op_list->getCurrentSize() - op_count;
			op_list->ignoreOperationChain(true);

			for(unsigned i=0; i < qtd; i++)
			{
				op_list->undoOperation();
				op_list->removeLastOperation();
			}

			op_list->ignoreOperationChain(false);
		}

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::loadModel(const QString &filename)
{
	TaskProgressWidget task_prog_wgt(this);

	try
	{
		#ifdef PGMODELER_DEBUG
			quint64 start = QDateTime::currentMSecsSinceEpoch();
		#endif

		connect(db_model, &DatabaseModel::s_objectLoaded, &task_prog_wgt, qOverload<int, QString, unsigned>(&TaskProgressWidget::updateProgress));
		task_prog_wgt.addIcon(enum_t(ObjectType::BaseObject), QPixmap(GuiUtilsNs::getIconPath("design")));
		task_prog_wgt.setWindowTitle(tr("Loading database model"));
		task_prog_wgt.show();

		db_model->loadModel(filename);
		this->filename = filename;
		updateObjectsOpacity();
		updateSceneLayers();
		adjustSceneRect(true);

		task_prog_wgt.close();
		//protected_model_frm->setVisible(db_model->isProtected());
		setProtected(db_model->isProtected());
		setModified(false);

		#ifdef PGMODELER_DEBUG
			quint64 end = QDateTime::currentMSecsSinceEpoch();
			double	total = end - start;
			QString unit = "ms";

			if(total > 1000)
			{
				total /= 1000;
				unit = "s";
			}

			qDebug().noquote() << "File: " << filename
			<< "\nLoaded in " << total << unit << "\n---";
		#endif
	}
	catch(Exception &e)
	{
		task_prog_wgt.close();
		setModified(false);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelWidget::updateSceneLayers()
{
	scene->blockSignals(true);

	scene->addLayers(db_model->getLayers(), false);
	scene->setActiveLayers(db_model->getActiveLayers());
	scene->setLayerColors(ObjectsScene::LayerNameColor, db_model->getLayerNameColors());
	scene->setLayerColors(ObjectsScene::LayerRectColor, db_model->getLayerRectColors());
	scene->setLayerNamesVisible(db_model->isLayerNamesVisible());
	scene->setLayerRectsVisible(db_model->isLayerRectsVisible());
	db_model->setObjectsModified({ ObjectType::Schema });

	scene->blockSignals(false);
}

void ModelWidget::adjustSceneRect(bool use_model_rect, bool expand_only)
{
	if(ObjectsScene::isAlignObjectsToGrid())
	{
		scene->alignObjectsToGrid();
		db_model->setObjectsModified();
	}

	QRectF rect = db_model->getSceneRect();

	if(use_model_rect && rect.isValid())
		scene->setSceneRect(rect);
	else
		rect = scene->adjustSceneRect(expand_only);

	setModified(true);

	emit s_sceneInteracted(rect.size());
}

void ModelWidget::expandSceneRect(ObjectsScene::ExpandDirection exp_dir)
{
	scene->expandSceneRect(exp_dir);
}

void ModelWidget::printModel(QPrinter *printer, bool print_grid, bool print_page_nums, bool resize_delims)
{
	if(!printer)
		return;

	bool show_grid = false, show_delims = false;
	unsigned page_cnt = 0, page = 0, h_page_cnt = 0, v_page_cnt = 0, h_pg_id = 0, v_pg_id = 0;
	QList<QRectF> pages;
	QMarginsF margins;
	QFont font;
	QString page_info;
	QColor color, bg_color;
	QRectF brect;

	//Make a backup of the current grid options
	bg_color = ObjectsScene::getCanvasColor();
	show_grid = ObjectsScene::isShowGrid();
	show_delims = ObjectsScene::isShowPageDelimiters();

	//Reconfigure the grid options based upon the passed settings
	ObjectsScene::setCanvasColor(QColor(255, 255, 255));
	ObjectsScene::setShowGrid(print_grid);
	ObjectsScene::setShowPageDelimiters(false);

	scene->setShowSceneLimits(false);
	scene->update();
	scene->clearSelection();

	//Get the pages rect for printing
	pages = scene->getPagesForPrinting(printer->pageLayout(), h_page_cnt, v_page_cnt,
																		 resize_delims && current_zoom < 1 ? current_zoom : 1);

	//Creates a painter to draw the model directly on the printer
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);
	font.setPointSizeF(BaseObjectView::DefaultFontSize * 0.75);
	font.setBold(true);
	painter.setFont(font);

	margins = printer->pageLayout().marginsPoints();
	page_cnt = pages.size();

	for(page = 0, h_pg_id = 1, v_pg_id = 1; page < page_cnt; page++)
	{
		//Render the current page on the printer
		scene->render(&painter, QRect(), pages[page]);

		//Print the current page number if this option is marked
		if(print_page_nums)
		{
			page_info = tr("Page #%1 / X:%2 Y:%3").arg(QString::number(page + 1)).arg(h_pg_id).arg(v_pg_id);
			color = ObjectsScene::getGridColor().darker();
			color.setAlpha(128);
			painter.setBrush(color);
			painter.setPen(ObjectsScene::getGridColor().lighter());
			brect = painter.fontMetrics().boundingRect(page_info);
			brect.adjust(-5, 0, 5, 0);

			painter.save();
			painter.translate(-margins.left() / 2, -margins.top() / 2);
			painter.drawRect(brect);
			painter.drawText(0, 0, page_info);
			painter.restore();

			h_pg_id++;

			if(h_pg_id > h_page_cnt)
			{
				h_pg_id = 1;
				v_pg_id++;
			}
		}

		if(page < page_cnt-1)
			printer->newPage();
	}

	//Restore the grid option backup
	ObjectsScene::setCanvasColor(bg_color);
	ObjectsScene::setShowGrid(show_grid);
	ObjectsScene::setShowPageDelimiters(show_delims);
	scene->setShowSceneLimits(true);
	scene->update();
}

void ModelWidget::updateRenderHints()
{
	viewport->setRenderHint(QPainter::Antialiasing, !disable_render_smooth);
	viewport->setRenderHint(QPainter::TextAntialiasing, !disable_render_smooth);
	viewport->setRenderHint(QPainter::SmoothPixmapTransform, !disable_render_smooth);
}

void ModelWidget::update()
{
	updateRenderHints();
	viewport->resetCachedContent();
	scene->update();
	QWidget::update();
}

void ModelWidget::bringToFront()
{
	moveObjectsInZStack(BringToFront);
}

void ModelWidget::sendToBack()
{
	moveObjectsInZStack(SendToBack);
}

void ModelWidget::saveModel()
{
	saveModel(this->filename);
}

void ModelWidget::saveModel(const QString &filename)
{
	TaskProgressWidget task_prog_wgt(this);
	QString bkpfile;
	QFileInfo fi(filename);
	bool has_bkp_file = false;

	try
	{
		connect(db_model, &DatabaseModel::s_objectLoaded, &task_prog_wgt, qOverload<int, QString, unsigned>(&TaskProgressWidget::updateProgress));
		task_prog_wgt.setWindowTitle(tr("Saving database model"));
		task_prog_wgt.show();

		/* If the original file exists we need to make a backup first to avoid
		 * data loss so we can recover it in case of saving failures */
		if(fi.exists())
		{
			QTemporaryFile tmpfile;

			// Generate a temporary backup filename
			tmpfile.setFileTemplate(fi.absolutePath() +
															GlobalAttributes::DirSeparator +
															QString("%1_XXXXXX%2").arg(db_model->getName(), GlobalAttributes::DbModelBkpExt));
			tmpfile.open();
			bkpfile = tmpfile.fileName();
			tmpfile.close();
			tmpfile.remove();

			/* Trying to rename the original model to the backup filename so
			 * we can write the new one in its place */
			if(!QFile::rename(filename, bkpfile))
				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(bkpfile),
												ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);

			has_bkp_file = true;
		}

		saveLastCanvasPosition();
		db_model->saveModel(filename, SchemaParser::XmlCode);
		this->filename=filename;

		task_prog_wgt.close();
		disconnect(db_model, nullptr, &task_prog_wgt, nullptr);
		setModified(false);

		/* Doing a final check to the file regarding its size.
		 * If we have a empty file something went wrong during the saving process (disk failure, thread errors, etc)
		 * so we raise an error to the user and restore the backup file to its original path */
		if(fi.size() == 0)
			throw Exception(Exception::getErrorMessage(ErrorCode::ModelFileInvalidSize).arg(filename),
											ErrorCode::ModelFileInvalidSize,PGM_FUNC,PGM_FILE,PGM_LINE);

		// Removing the backup file if the model was successfully saved
		if(has_bkp_file)
			QFile::remove(bkpfile);
	}
	catch(Exception &e)
	{
		task_prog_wgt.close();
		disconnect(db_model, nullptr, &task_prog_wgt, nullptr);

		/* If the original file was successfully rename as the backup file
		 * we just revert the name to the original one */
		if(has_bkp_file)
		{
			// Remove the failed file so we can restore the backup file
			QFile::remove(filename);
			QFile::copy(bkpfile, filename);

			throw Exception(Exception::getErrorMessage(ErrorCode::ModelFileSaveFailure).arg(filename).arg(bkpfile),
											ErrorCode::ModelFileSaveFailure,PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString ModelWidget::getFilename()
{
	return this->filename;
}

QString ModelWidget::getTempFilename()
{
	return this->tmp_filename;
}

template<class WidgetClass>
int ModelWidget::openEditingForm(WidgetClass *widget, Messagebox::ButtonsId button_conf)
{
	BaseForm editing_form(this);
	QString class_name = widget->metaObject()->className();
	int res = 0;

	/* If the widget specified can be converted to BaseObjectWidget
	 * means that we are handling a database object widget so
	 * we use the BaseObjectWidget version of BaseForm::setMainWidget */
	if(qobject_cast<BaseObjectWidget *>(widget))
	{
		BaseRelationship *rel = dynamic_cast<BaseRelationship *>(widget->getHandledObject());
		editing_form.setMainWidget(widget);

		if(rel)
			class_name.prepend(rel->getRelationshipTypeName().replace(QRegularExpression("( )+|(\\-)+"), ""));
	}
	else
		// Use the QWidget version of BaseForm::setMainWidget
		editing_form.setMainWidget(widget);

	editing_form.setButtonConfiguration(button_conf);

	GeneralConfigWidget::restoreWidgetGeometry(&editing_form, class_name);
	res = editing_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&editing_form, class_name);

	return res;
}

template<class Class, class WidgetClass>
int ModelWidget::openEditingForm(BaseObject *object)
{
	WidgetClass *object_wgt=new WidgetClass;
	object_wgt->setAttributes(db_model, op_list, dynamic_cast<Class *>(object));
	return openEditingForm(object_wgt);
}

template<class Class, class WidgetClass, class ParentClass>
int ModelWidget::openEditingForm(BaseObject *object, BaseObject *parent_obj)
{
	WidgetClass *object_wgt=new WidgetClass;
	object_wgt->setAttributes(db_model, op_list, dynamic_cast<ParentClass *>(parent_obj), dynamic_cast<Class *>(object));
	return openEditingForm(object_wgt);
}

template<class Class, class WidgetClass, class ParentClass>
int ModelWidget::openEditingForm(BaseObject *object, BaseObject *parent_obj, const QPointF &pos)
{
	WidgetClass *object_wgt=new WidgetClass;
	object_wgt->setAttributes(db_model, op_list, dynamic_cast<ParentClass *>(parent_obj), dynamic_cast<Class *>(object), pos.x(), pos.y());
	return openEditingForm(object_wgt);
}

int ModelWidget::openTableEditingForm(ObjectType tab_type, PhysicalTable *object, Schema *schema, const QPointF &pos)
{
	TableWidget *tab_wgt=new TableWidget(nullptr, tab_type);

	if(tab_type == ObjectType::Table)
		tab_wgt->setAttributes(db_model, op_list, schema, dynamic_cast<Table *>(object), pos.x(), pos.y());
	else
		tab_wgt->setAttributes(db_model, op_list, schema, dynamic_cast<ForeignTable *>(object), pos.x(), pos.y());

	return openEditingForm(tab_wgt);
}

void ModelWidget::configurePluginsActions()
{
	if(plugins_actions.isEmpty())
		return;

	PgModelerGuiPlugin *plugin = nullptr;
	std::map<PgModelerGuiPlugin::MenuSectionId, QAction *> menu_sects_seps;
	PgModelerGuiPlugin::MenuSectionId menu_sect;
	bool sep_added = false;

	for(auto &act : popup_menu.actions())
	{
		if(!act->isSeparator())
			continue;

		if(!menu_sects_seps.count(PgModelerGuiPlugin::TopSection))
			menu_sects_seps[PgModelerGuiPlugin::TopSection] = act;
		else if(!menu_sects_seps.count(PgModelerGuiPlugin::MiddleSection))
			menu_sects_seps[PgModelerGuiPlugin::MiddleSection] = act;
		else
		{
			menu_sects_seps[PgModelerGuiPlugin::BottomSection] = action_edit_creation_order;
			break;
		}
	}

	for(auto &act : plugins_actions)
	{
		/* If the action carries a reference its parent plugin
		 * via QAction::data() we call the method PgModelerGuiPlugin::isSelectionValid
		 * of that pluign to enable/disable the action depending on the selection
		 * in the model according to the plugin's rules */
		plugin = act->data().value<PgModelerGuiPlugin *>();
		menu_sect = PgModelerGuiPlugin::DefaultSection;

		if(plugin)
		{
			act->setEnabled(plugin->isSelectionValid());
			menu_sect = plugin->getMenuSection();
		}

		if(menu_sect == PgModelerGuiPlugin::DefaultSection)
		{
			if(!sep_added)
			{
				popup_menu.addSeparator();
				sep_added = true;
			}

			popup_menu.addAction(act);
		}
		else
			popup_menu.insertAction(menu_sects_seps[menu_sect], act);
	}
}

void ModelWidget::showObjectForm(ObjectType obj_type, BaseObject *object, BaseObject *parent_obj, const QPointF &pos)
{
	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: limiting the user interaction over objects."
		if(updateObjActionCounter())
			return;
	#endif

	try
	{
		BaseRelationship::RelType rel_type = BaseRelationship::Relationship11;
		int res = QDialog::Rejected;
		Schema *sel_schema = dynamic_cast<Schema *>(parent_obj);
		QPointF obj_pos = pos;

		/* Case the obj_type is greater than ObjectType::ObjBaseTable indicates that the object type is a
		 relationship. To get the specific relationship id (1-1, 1-n, n-n, gen, dep) is necessary
		 to subtract the ObjectType::ObjRelationship from the obj_type parameter, the result will point
		 to the BaseRelationship::RELATIONSHIP_??? constant. */
		if(obj_type > ObjectType::BaseTable)
		{
			rel_type = static_cast<BaseRelationship::RelType>(enum_t(obj_type) - enum_t(ObjectType::Relationship));
			obj_type = ObjectType::Relationship;
		}

		if(obj_type!=ObjectType::Permission)
		{
			if(object && obj_type!=object->getObjectType())
				throw Exception(ErrorCode::OprObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);

			//If the user try to call the table object form without specify a parent object
			if(!parent_obj && TableObject::isTableObject(obj_type))
				throw Exception(ErrorCode::OprNotAllocatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		if(object && dynamic_cast<BaseGraphicObject *>(object))
			obj_pos=dynamic_cast<BaseGraphicObject *>(object)->getPosition();

		/* Raises an error if the user try to edit a reserverd object.
		 * The only exception is for "public" schema that can be edited and permissions that can
		 * be assigned to system objects */
		if(object && object->isSystemObject() && obj_type != ObjectType::Permission &&
				(object->getObjectType() != ObjectType::Schema || object->getName() != "public"))
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
											.arg(object->getName(), object->getTypeName()),
											ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		setBlinkAddedObjects(BaseGraphicObject::isGraphicObject(obj_type));

		if(obj_type==ObjectType::Schema)
			res=openEditingForm<Schema,SchemaWidget>(object);
		else if(obj_type==ObjectType::Role)
			res=openEditingForm<Role,RoleWidget>(object);
		else if(obj_type==ObjectType::Tablespace)
			res=openEditingForm<Tablespace,TablespaceWidget>(object);
		else if(obj_type==ObjectType::Language)
			res=openEditingForm<Language,LanguageWidget>(object);
		else if(obj_type==ObjectType::Cast)
			res=openEditingForm<Cast, CastWidget>(object);
		else if(obj_type==ObjectType::Tag)
			res=openEditingForm<Tag,TagWidget>(object);
		else if(obj_type== ObjectType::EventTrigger)
			res=openEditingForm<EventTrigger,EventTriggerWidget>(object);
		else if(obj_type==ObjectType::Function)
			res=openEditingForm<Function, FunctionWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Conversion)
			res=openEditingForm<Conversion, ConversionWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Domain)
			res=openEditingForm<Domain, DomainWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Aggregate)
			res=openEditingForm<Aggregate, AggregateWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Sequence)
			res=openEditingForm<Sequence, SequenceWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Operator)
			res=openEditingForm<Operator, OperatorWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::OpFamily)
			res=openEditingForm<OperatorFamily, OperatorFamilyWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::OpClass)
			res=openEditingForm<OperatorClass, OperatorClassWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Type)
			res=openEditingForm<Type, TypeWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Collation)
			res=openEditingForm<Collation, CollationWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Extension)
			res=openEditingForm<Extension, ExtensionWidget, Schema>(object, sel_schema);
		else if(obj_type==ObjectType::Table)
			res=openTableEditingForm(obj_type, dynamic_cast<Table *>(object), sel_schema, obj_pos);
		else if(obj_type==ObjectType::ForeignTable)
			res=openTableEditingForm(obj_type, dynamic_cast<ForeignTable *>(object), sel_schema, obj_pos);
		else if(obj_type==ObjectType::View)
			res=openEditingForm<View,ViewWidget,Schema>(object, sel_schema, obj_pos);
		else if(obj_type==ObjectType::Rule)
			res=openEditingForm<Rule, RuleWidget, BaseTable>(object, parent_obj);
		else if(obj_type== ObjectType::Trigger)
			res=openEditingForm<Trigger, TriggerWidget, BaseTable>(object, parent_obj);
		else if(obj_type== ObjectType::Index)
			res=openEditingForm<Index, IndexWidget, BaseTable>(object, parent_obj);
		else if(obj_type== ObjectType::Policy)
			res=openEditingForm<Policy, PolicyWidget, BaseTable>(object, parent_obj);
		else if(obj_type==ObjectType::Column || obj_type==ObjectType::Constraint)
		{
			TableObject *tab_obj=dynamic_cast<TableObject *>(object);

			if(obj_type==ObjectType::Column)
				res=openEditingForm<Column, ColumnWidget, BaseObject>(object, parent_obj);
			else
				res=openEditingForm<Constraint, ConstraintWidget, BaseObject>(object, parent_obj);

			if(res==QDialog::Accepted)
			{
				if(tab_obj && parent_obj->getObjectType()==ObjectType::Table)
					db_model->validateRelationships(tab_obj, dynamic_cast<Table *>(parent_obj));
				else
					db_model->validateRelationships();
			}
		}
		else if(obj_type==ObjectType::BaseRelationship || obj_type==ObjectType::Relationship)
		{
			RelationshipWidget *relationship_wgt=new RelationshipWidget;

			if(!object && rel_type > 0 &&
					selected_objects.size() > 0 &&
					PhysicalTable::isPhysicalTable(selected_objects[0]->getObjectType()))
			{
				PhysicalTable *tab1 = dynamic_cast<PhysicalTable *>(selected_objects[0]),
											*tab2 = (selected_objects.size()==2 ?
															 dynamic_cast<PhysicalTable *>(selected_objects[1]) : tab1);
				relationship_wgt->setAttributes(db_model, op_list, tab1, tab2, rel_type);
			}
			else
				relationship_wgt->setAttributes(db_model, op_list, dynamic_cast<BaseRelationship *>(object));

			res=openEditingForm(relationship_wgt);
			scene->clearSelection();
		}
		else if(obj_type==ObjectType::Textbox)
		{
			TextboxWidget *textbox_wgt=new TextboxWidget;
			textbox_wgt->setAttributes(db_model, op_list, dynamic_cast<Textbox *>(object), obj_pos.x(), obj_pos.y());
			res=openEditingForm(textbox_wgt);
		}
		else if(obj_type==ObjectType::Permission)
		{
			PermissionWidget *permission_wgt=new PermissionWidget;
			Permission *perm=dynamic_cast<Permission *>(object);
			permission_wgt->setAttributes(db_model, nullptr, (perm ? perm->getObject() : object));
			res=openEditingForm(permission_wgt, Messagebox::CloseButton);
		}
		else if(obj_type==ObjectType::GenericSql)
		{
			GenericSQLWidget *genericsql_wgt=new GenericSQLWidget;
			genericsql_wgt->setAttributes(db_model, op_list, dynamic_cast<GenericSQL *>(object));
			res=openEditingForm(genericsql_wgt);
		}
		else if(obj_type==ObjectType::ForeignDataWrapper)
			res = openEditingForm<ForeignDataWrapper, ForeignDataWrapperWidget>(object);
		else if(obj_type==ObjectType::ForeignServer)
			res = openEditingForm<ForeignServer, ForeignServerWidget>(object);
		else if(obj_type==ObjectType::UserMapping)
			res = openEditingForm<UserMapping, UserMappingWidget>(object);
		else if(obj_type==ObjectType::Transform)
			res = openEditingForm<Transform, TransformWidget>(object);
		else if(obj_type==ObjectType::Procedure)
			res=openEditingForm<Procedure, ProcedureWidget, Schema>(object, sel_schema);
		else
		{
			DatabaseWidget *database_wgt=new DatabaseWidget;
			database_wgt->setAttributes(db_model);
			res=openEditingForm(database_wgt);
		}

		setBlinkAddedObjects(false);

		if(res==QDialog::Accepted)
		{
			setModified(true);
			db_model->setInvalidated(true);
			emit s_objectManipulated();
		}
		else
		  emit s_manipulationCanceled();

		this->setFocus();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::showDependenciesReferences()
{
	QAction *obj_sender=dynamic_cast<QAction *>(sender());

	if(obj_sender)
	{
		BaseObject *object=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

		if(object)
		{
			ObjectDepsRefsWidget *deps_refs_wgt=new ObjectDepsRefsWidget;
			deps_refs_wgt->setAttributes(this, object);
			openEditingForm(deps_refs_wgt, Messagebox::CloseButton);
		}
	}
}

void ModelWidget::showSourceCode()
{
	SourceCodeWidget *sourcecode_wgt = new SourceCodeWidget;
	sourcecode_wgt->setAttributes(db_model, selected_objects);
	openEditingForm(sourcecode_wgt, Messagebox::CloseButton);
}

void ModelWidget::cancelObjectAddition()
{
	//Reset the new object type to a invalid one forcing the user to select a correct type again
	new_obj_type=ObjectType::BaseObject;

	//Restore the cursor icon
	viewport->setCursor(QCursor(Qt::ArrowCursor));

	//Hide the line that simulates the relationship creation
	scene->showRelationshipLine(false);

	this->configurePopupMenu(this->selected_objects);
}

void ModelWidget::renameObjects()
{
	ObjectRenameWidget objectrename_wgt(this);

	if(selected_objects.empty())
		objectrename_wgt.setAttributes({ db_model }, db_model, op_list);
	else
		objectrename_wgt.setAttributes(selected_objects, db_model, op_list);

	objectrename_wgt.exec();

	if(objectrename_wgt.result() == QDialog::Accepted)
	{
		setModified(true);
		emit s_objectModified();
	}
}

void ModelWidget::moveToSchema()
{
	QAction *act=dynamic_cast<QAction *>(sender());
	Schema *schema=dynamic_cast<Schema *>(reinterpret_cast<BaseObject *>(act->data().value<void *>()));
	BaseGraphicObject *obj_graph=nullptr;
	std::vector<BaseObject *> ref_objs;
	int op_id=-1, op_curr_idx=op_list->getCurrentIndex();

	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		op_list->startOperationChain();

		for(BaseObject *obj : selected_objects)
		{
			//Change the object's schema only if the new schema is different from the current
			if(obj->acceptsSchema() && obj->getSchema()!=schema)
			{
				op_id=op_list->registerObject(obj, Operation::ObjModified, -1);

				obj->clearDependencies();
				obj->setSchema(schema);
				obj->updateDependencies();

				obj_graph=dynamic_cast<BaseGraphicObject *>(obj);

				//If the object is a graphical one, move it to a position near to the new schema box
				if(obj_graph)
				{
					SchemaView *dst_schema=dynamic_cast<SchemaView *>(schema->getOverlyingObject());
					QPointF p;

					if(dst_schema && dst_schema->isVisible())
					{
						p.setX(dst_schema->pos().x());
						p.setY(dst_schema->pos().y() + dst_schema->boundingRect().height() + BaseObjectView::VertSpacing);
						dynamic_cast<BaseObjectView *>(obj_graph->getOverlyingObject())->setPos(p);
					}
				}

				//Invalidating the code of the object's references
				for(auto &ref_obj : obj->getReferences())
					ref_obj->setCodeInvalidated(true);
			}
		}

		op_list->finishOperationChain();
		db_model->setObjectsModified();
		this->setModified(true);

		emit s_objectModified();

		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();

		if(op_id >=0 && op_id > op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::updateObjectsLayers()
{
	layers_wgt->setAttributes(this);

	if(layers_wgt->exec() == QDialog::Accepted &&
		 layers_wgt->isLayersChanged())
	{
		qApp->setOverrideCursor(Qt::WaitCursor);
		scene->updateActiveLayers();
		db_model->setObjectsModified({ ObjectType::Schema });
		qApp->restoreOverrideCursor();

		emit s_objectsLayerChanged();
	}
}

void ModelWidget::changeOwner()
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *owner=reinterpret_cast<BaseObject *>(act->data().value<void *>());
	std::vector<BaseObject *> sel_objs;
	int op_id=-1, op_curr_idx=op_list->getCurrentIndex();

	try
	{
		if(selected_objects.empty())
			sel_objs.push_back(this->db_model);
		else
			sel_objs=selected_objects;

		op_list->startOperationChain();

		for(BaseObject *obj : sel_objs)
		{
			if(obj->acceptsOwner() && obj->getOwner()!=owner)
			{
				if(obj->isSystemObject())
					throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
									.arg(obj->getName())
									.arg(obj->getTypeName()),
									ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

				//Register an operation only if the object is not the database itself
				if(obj->getObjectType()!=ObjectType::Database)
					op_id=op_list->registerObject(obj, Operation::ObjModified, -1);

				obj->clearDependencies();
				obj->setOwner(owner);
				obj->updateDependencies();
			}
		}

		op_list->finishOperationChain();
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		if(op_id >=0 && op_id >= op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::setTag()
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *tag=reinterpret_cast<BaseObject *>(act->data().value<void *>());
	BaseTable *tab=nullptr;
	int op_id=-1, op_curr_idx=op_list->getCurrentIndex();

	try
	{
		op_list->startOperationChain();

		for(BaseObject *obj : selected_objects)
		{
			tab=dynamic_cast<BaseTable *>(obj);

			if(tab)
			{
				op_id=op_list->registerObject(obj, Operation::ObjModified, -1);
				tab->clearDependencies();
				tab->setTag(dynamic_cast<Tag *>(tag));
				tab->updateDependencies();
			}
		}

		op_list->finishOperationChain();
		db_model->setObjectsModified(selected_objects);
		scene->clearSelection();

		emit s_objectModified();
	}
	catch(Exception &e)
	{
		if(op_id >=0 &&  op_id > op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::editPermissions()
{
	PermissionWidget *permission_wgt=new PermissionWidget;
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *obj=reinterpret_cast<BaseObject *>(act->data().value<void *>());

	permission_wgt->setAttributes(this->db_model, nullptr, obj);
	openEditingForm(permission_wgt, Messagebox::CloseButton);

	this->setModified(true);
	emit s_objectManipulated();
}

void ModelWidget::editObject()
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	TableObject *tab_obj=nullptr;
	BaseObject *object=nullptr;

	/* Workaround: To permit the object edition via double click on the ModelObjectWidget the sender
	is configured as the edit action of the model widget */
	if(!obj_sender)
		obj_sender=action_edit;

	object=reinterpret_cast<BaseObject *>(dynamic_cast<QAction *>(obj_sender)->data().value<void *>());
	tab_obj=dynamic_cast<TableObject *>(object);

	if(object)
		showObjectForm(object->getObjectType(), object, (tab_obj ? tab_obj->getParentTable() : nullptr));
}

void ModelWidget::selectSchemaChildren()
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	Schema *schema=nullptr;
	SchemaView *sch_view = nullptr;

	schema=dynamic_cast<Schema *>(
				 reinterpret_cast<BaseObject *>(
					 dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	scene->clearSelection();

	sch_view = 	dynamic_cast<SchemaView *>(
				dynamic_cast<BaseObjectView *>(schema->getOverlyingObject()));

	if(sch_view)
		sch_view->selectChildren();
}

void ModelWidget::selectTableRelationships()
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	BaseTable *table=nullptr;

	table=dynamic_cast<BaseTable *>(
				 reinterpret_cast<BaseObject *>(
					 dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	scene->clearSelection();

	dynamic_cast<BaseTableView *>(
				dynamic_cast<BaseObjectView *>(table->getOverlyingObject()))->selectRelationships();
}

void ModelWidget::selectTaggedTables()
{
	QObject *obj_sender = dynamic_cast<QAction *>(sender());
	Tag *tag = nullptr;
	BaseObjectView *obj_view = nullptr;

	tag=dynamic_cast<Tag *>(
				reinterpret_cast<BaseObject *>(
					dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	scene->clearSelection();

	for(auto &object : tag->getReferences())
	{
		obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(object)->getOverlyingObject());
		obj_view->setSelected(true);
	}
}

void ModelWidget::protectObject()
{
	try
	{
		QObject *obj_sender=sender();
		ObjectType obj_type;
		TableObject *tab_obj=nullptr;
		bool protect=false;
		QList<BaseGraphicObject *> upd_objects;

		scene->blockSignals(true);

		//Protects the whole model if there is no selected object
		if(this->selected_objects.empty())
		{
			if(obj_sender == action_protect || obj_sender == action_unprotect)
				//db_model->setProtected(!db_model->isProtected());
				setProtected(!db_model->isProtected());
		}
		//If there is more than one selected object, make a batch protection/unprotection
		else
		{
			protect=(!this->selected_objects[0]->isProtected());

			for(auto &object : selected_objects)
			{
				obj_type=object->getObjectType();

				if(obj_type==ObjectType::Column || obj_type==ObjectType::Constraint)
				{
					tab_obj=dynamic_cast<TableObject *>(object);

					if(tab_obj->isAddedByRelationship())
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::OprRelationshipAddedObject)
										.arg(object->getName()).arg(object->getTypeName()),
										ErrorCode::OprRelationshipAddedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
					}
				}

				// Applying protection status for the schema children objects
				if(obj_type==ObjectType::Schema)
				{
					Messagebox msgbox;

					if(!msgbox.isCustomOptionChecked())
					{
						msgbox.setCustomOptionText("Apply to all other selected schemas");
						msgbox.show(QString(QT_TR_NOOP("Do you want to %1 the children of the schema <strong>%2</strong> too?"))
												.arg(protect ? QT_TR_NOOP("protect") : QT_TR_NOOP("unprotect")).arg(object->getName()),
												Messagebox::Confirm, Messagebox::YesNoButtons);
					}

					if(msgbox.isAccepted() || msgbox.isCustomOptionChecked())
					{
						std::vector<BaseObject *> objects(db_model->getObjects(object));

						for(BaseObject *obj : objects)
							obj->setProtected(protect);
					}
				}

				object->setProtected(protect);
				tab_obj = dynamic_cast<TableObject *>(object);

				if(tab_obj && !upd_objects.contains(tab_obj->getParentTable()))
					upd_objects.push_back(tab_obj->getParentTable());
			}
		}

		for(auto &obj : upd_objects)
			obj->setModified(true);

		scene->blockSignals(false);
		scene->clearSelection();
		this->setModified(true);

		emit s_objectModified();
	}
	catch(Exception &e)
	{
		scene->blockSignals(false);
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::cutObjects(bool copy_deps)
{
	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: limiting the user interaction over objects."
		if(updateObjActionCounter())
			return;
	#endif

	//Set the flag indicating that a cut operation started
	ModelWidget::cut_operation=true;
	copyObjects(false, copy_deps);
}

void ModelWidget::copyObjects(bool duplicate_mode, bool copy_deps)
{
	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: limiting the user interaction over objects."
		if(updateObjActionCounter())
			return;
	#endif

	std::map<unsigned, BaseObject *> objs_map;
	std::vector<BaseObject *> sel_obj_deps, deps;
	BaseObject *object = nullptr;
	TableObject *tab_obj = nullptr;
	BaseTable *table = nullptr;
	Constraint *constr = nullptr;

	/* Store the source model as 'this'. This attribute is used on the paste method
	 * to remove the selected object and updated the source model when a cut operation
	 * is active */
	ModelWidget::src_model = this;

	if(selected_objects.size()==1)
	{
		//Raise an error if the user try to copy a reserved object
		if(selected_objects[0]->isSystemObject())
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
											.arg(selected_objects[0]->getName()).arg(selected_objects[0]->getTypeName()),
											ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}

	/* When in cut operation is necessary to store the selected objects in a separeted list
	in order to correclty cut (remove) the object on the source model */
	if(ModelWidget::cut_operation)
		cut_objects=selected_objects;

	for(auto &object : selected_objects)
	{
		//Table-view relationships and FK relationship aren't copied since they are created automatically when pasting the tables/views
		if(object->getObjectType() == ObjectType::BaseRelationship)
			continue;

		if(copy_deps)
		{
			deps = object->getDependencies(true, { ObjectType::Column });
			sel_obj_deps.insert(sel_obj_deps.end(), deps.begin(), deps.end());
		}

		sel_obj_deps.push_back(object);

		/* Copying the special objects (which references columns added by relationship) in order
		to be correclty created when pasted */
		if(object->getObjectType()==ObjectType::Table || object->getObjectType() == ObjectType::View)
		{
			table = dynamic_cast<BaseTable *>(object);

			for(auto &obj : table->getObjects({ ObjectType::Column }))
			{
				tab_obj = dynamic_cast<TableObject *>(obj);
				constr = dynamic_cast<Constraint *>(tab_obj);

				/* The object is only inserted at the list when it was not included by relationship but references
				columns added by relationship. Case the object is a constraint, it cannot be a primary key because
				this type of constraint is treated separetely by relationships */
				if(duplicate_mode ||
						(!duplicate_mode && !tab_obj->isAddedByRelationship() &&
						 (!constr ||	(((constr &&
								 (constr->getConstraintType()==ConstraintType::ForeignKey ||
									(constr->getConstraintType()==ConstraintType::Unique &&
									 constr->isReferRelationshipAddedColumns()))))))))
				{
					sel_obj_deps.push_back(tab_obj);
				}
			}
		}
	}

	//Storing the objects ids in a auxiliary map organizing them by creation order
	std::for_each(sel_obj_deps.begin(), sel_obj_deps.end(), [&objs_map](BaseObject *object) {
		objs_map[object->getObjectId()] = object;
	});

	copied_objects.clear();

	for(auto &obj_itr : objs_map)
	{
		object = obj_itr.second;

		//Reserved object aren't copied
		if(object->isSystemObject())
			continue;

		copied_objects.push_back(object);
	}
}

void ModelWidget::pasteObjects(bool duplicate_mode)
{
	std::map<BaseObject *, QString> xml_objs;
	BaseTable *orig_parent_tab = nullptr;
	std::vector<BaseObject *>::iterator itr, itr_end;
	std::map<BaseObject *, QString> orig_names, orig_fmt_names;
	BaseObject *object = nullptr, *aux_object = nullptr;
	BaseGraphicObject *graph_obj = nullptr;
	TableObject *tab_obj = nullptr;
	Table *sel_table = nullptr, *aux_table = nullptr;
	View *sel_view = nullptr;
	BaseTable *parent = nullptr;
	BaseFunction *func = nullptr;
	Constraint *constr = nullptr;
	Operator *oper = nullptr;
	QString aux_name, new_name;
	ObjectType obj_type;
	std::vector<Exception> errors;
	unsigned pos = 0;
	TaskProgressWidget task_prog_wgt(this);
	ObjectRenameWidget obj_rename_wgt(this);
	bool discard_input_obj = duplicate_mode || (src_model == this);

	task_prog_wgt.setWindowTitle(tr("Pasting objects..."));
	task_prog_wgt.show();
	task_prog_wgt.stackUnder(&obj_rename_wgt);

	itr = copied_objects.begin();
	itr_end = copied_objects.end();

	/* If there is only one object selected, check if its a table or view.
	Because if the user try to paste a table object the receiver object (selected)
	must be a table or view */
	if(selected_objects.size() == 1)
	{
		sel_table = dynamic_cast<Table *>(selected_objects[0]);
		sel_view = dynamic_cast<View *>(selected_objects[0]);
	}

	setBlinkAddedObjects(true);

	while(itr != itr_end)
	{
		object = (*itr);
		obj_type = object->getObjectType();
		tab_obj = dynamic_cast<TableObject *>(object);
		itr++;
		pos++;
		task_prog_wgt.updateProgress((pos/static_cast<double>(copied_objects.size()))*100,
									 tr("Validating object: `%1' (%2)").arg(object->getName())
									 .arg(object->getTypeName()),
									 enum_t(object->getObjectType()));

		if(!tab_obj || ((sel_table || sel_view) && tab_obj))
		{
			/* The first validation is to check if the object to be pasted does not conflict
			with any other object of the same type on the model */
			if(BaseFunction::isBaseFunction(obj_type))
			{
				dynamic_cast<BaseFunction *>(object)->createSignature(true);
				aux_name = object->getSignature();
			}
			else if(tab_obj)
				aux_name = tab_obj->getName(true);
			else
				aux_name = object->getSignature();

			if(!tab_obj)
				//Try to find the object on the model
				aux_object = db_model->getObject(aux_name, obj_type);
			else
			{
				if(sel_view && (obj_type == ObjectType::Trigger || obj_type == ObjectType::Rule || obj_type == ObjectType::Index))
					aux_object = sel_view->getObject(aux_name, obj_type);
				else if(sel_table)
					aux_object = sel_table->getObject(aux_name, obj_type);
			}

			/* The second validation is to check, when the object is found on the model, if the XML code of the found object
			 and the object to be pasted are different. When the XML defintion are the same the object isn't pasted because
			 the found object can be used as substitute of the object to be pasted. This operation is not applied to graphical
			 objects because they are ALWAYS pasted on the model. The only exception is that the below code is executed when the
			 found object is the same as the copied object (this means that user is copying and pasting the object in the same database) */
			if(tab_obj ||
					(aux_object &&
					 (dynamic_cast<BaseGraphicObject *>(object) ||
						(aux_object->getDatabase() == object->getDatabase()) ||
						(aux_object->getSourceCode(SchemaParser::SchemaParser::XmlCode) !=
						 object->getSourceCode(SchemaParser::SchemaParser::XmlCode)))))
			{
				//Resolving name conflicts
				if(obj_type != ObjectType::Cast)
				{
					obj_rename_wgt.setAttributes(object);
					obj_rename_wgt.use_defaults_chk->setChecked(GeneralConfigWidget::
																												getConfigurationParam(Attributes::Configuration,
																																							Attributes::UseDefDisambiguation) == Attributes::True);

					/* Ask the user a new object name by using an instance of ObjectRenameWidget
					 * If the user accept the dialog we use the typed name otherwise the
					 * original object name will be used and eventually disambigated */
					if(aux_object && !obj_rename_wgt.use_defaults_chk->isChecked() &&
						 obj_rename_wgt.exec() == QDialog::Accepted)
						new_name = obj_rename_wgt.getNewName();
					else
						new_name = object->getName();

					GeneralConfigWidget::appendConfigurationSection(Attributes::Configuration,
																													 {{ Attributes::UseDefDisambiguation,
																															obj_rename_wgt.use_defaults_chk->isChecked() ?
																															Attributes::True : Attributes::False }});

					func = nullptr; oper = nullptr;

					// Store the orignal object name on a map so we can restore it at the end of pasting operation
					orig_names[object] = object->getName();
					orig_fmt_names[object] = object->getName(true);

					// Set the name specified in the rename dialog so we can start the disambigation operation
					object->setName(new_name);

					/* For each object type as follow configures the name and the suffix and store them on the
						'copy_obj_name' variable. This string is used to check if there are objects with the same name
						on model. While the 'copy_obj_name' conflicts with other objects (of same type) this validation is made */
					if(BaseFunction::isBaseFunction(obj_type))
					{
						func = dynamic_cast<BaseFunction *>(object);
						func->setName(CoreUtilsNs::generateUniqueName(func,
																													*db_model->getObjectList(obj_type),
																													true, "_cp", true, false));
					}
					else if(obj_type == ObjectType::Operator)
					{
						oper = dynamic_cast<Operator *>(object);
						oper->setName(CoreUtilsNs::generateUniqueName(oper,
																													*db_model->getObjectList(ObjectType::Operator),
																													true, "", true, false));
					}
					else
					{
						if(tab_obj)
						{
							if(sel_table)
							{
								tab_obj->setName(CoreUtilsNs::generateUniqueName(tab_obj,
																																 *sel_table->getObjectList(tab_obj->getObjectType()),
																																 false, "_cp", true, discard_input_obj));
							}
							else
							{
								tab_obj->setName(CoreUtilsNs::generateUniqueName(tab_obj,
																																 *sel_view->getObjectList(tab_obj->getObjectType()),
																																 false, "_cp", true, discard_input_obj));
							}
						}
						else
						{
							object->setName(CoreUtilsNs::generateUniqueName(object,
																															*db_model->getObjectList(object->getObjectType()),
																															true, "_cp", true, discard_input_obj));
						}
					}
				}
			}
		}
	}

	/* The third step is get the XML code definition of the copied objects, is
	with the xml code that the copied object are created and inserted on the model */
	itr = copied_objects.begin();
	itr_end = copied_objects.end();
	pos = 0;

	while(itr != itr_end)
	{
		object = (*itr);
		object->setCodeInvalidated(true);

		tab_obj = dynamic_cast<TableObject *>(object);
		itr++;

		pos++;
		task_prog_wgt.updateProgress((pos/static_cast<double>(copied_objects.size()))*100,
									 tr("Generating XML for: `%1' (%2)").arg(object->getName())
									 .arg(object->getTypeName()),
									 enum_t(object->getObjectType()));

		if(!tab_obj)
		{
			aux_table =  dynamic_cast<Table *>(object);

			//Stores the XML definition on a xml buffer map
			if(duplicate_mode && aux_table)
			{
				xml_objs[object] = aux_table->__getSourceCode(SchemaParser::XmlCode, true);
			  object->setCodeInvalidated(true);
			}
			else
				xml_objs[object] = object->getSourceCode(SchemaParser::XmlCode);
		}

		//Store the original parent table of the object
		else if(tab_obj && (sel_table || sel_view))
		{
			if(sel_table)
				parent=sel_table;
			else
				parent=sel_view;

			/* Only generates the XML for a table object when the selected receiver object
			 * is a table or is a view and the current object is a trigger, index, or rule (because
			 * view's only accepts this two types) */
			if(sel_table ||
					(sel_view && (tab_obj->getObjectType()==ObjectType::Trigger ||
									tab_obj->getObjectType()==ObjectType::Rule ||
									tab_obj->getObjectType()==ObjectType::Index)))
			{
				//Backups the original parent table
				orig_parent_tab = tab_obj->getParentTable();

				constr = dynamic_cast<Constraint *>(tab_obj);

				//Set the parent table as the selected table/view
				tab_obj->setParentTable(parent);

				//Generates the XML code with the new parent table
				if(constr)
					xml_objs[object] = constr->getSourceCode(SchemaParser::XmlCode, duplicate_mode);
				else
					xml_objs[object] = object->getSourceCode(SchemaParser::XmlCode);

				//Restore the original parent table
				tab_obj->setParentTable(orig_parent_tab);
			}
		}
		else if(tab_obj)
		{
			//Generates the XML code with the new parent table
			constr = dynamic_cast<Constraint *>(tab_obj);

			if(constr)
			{
				xml_objs[object] = constr->getSourceCode(SchemaParser::XmlCode, duplicate_mode);
			  tab_obj->setCodeInvalidated(true);
			}
			else
				xml_objs[object] = tab_obj->getSourceCode(SchemaParser::XmlCode);
		}
	}

	//The fourth step is the restoration of original names of the copied objects
	itr = copied_objects.begin();
	itr_end = copied_objects.end();

	while(itr != itr_end)
	{
		object = (*itr);
		obj_type = object->getObjectType();
		itr++;

		if(!orig_names[object].isEmpty() && obj_type != ObjectType::Cast)
			object->setName(orig_names[object]);
	}

	//The last step is create the object from the stored xmls
	itr = copied_objects.begin();
	itr_end = copied_objects.end();
	pos = 0;

	op_list->startOperationChain();

	while(itr != itr_end)
	{
		object = *itr;
		itr++;

		if(xml_objs.count(object))
		{
			xmlparser->restartParser();
			xmlparser->loadXMLBuffer(xml_objs[object]);

			try
			{
				pos++;
				task_prog_wgt.updateProgress((pos/static_cast<double>(copied_objects.size()))*100,
											 tr("Pasting object: `%1' (%2)").arg(object->getName())
											 .arg(object->getTypeName()),
											 enum_t(object->getObjectType()));

				//Creates the object from the XML
				object = db_model->createObject(BaseObject::getObjectType(xmlparser->getElementName()));
				tab_obj = dynamic_cast<TableObject *>(object);
				constr = dynamic_cast<Constraint *>(tab_obj);
				graph_obj = dynamic_cast<BaseGraphicObject *>(object);

				/* Once created, the object is added on the model, except for relationships and table objects
				 * because they are inserted automatically */
				if(object && !tab_obj && !dynamic_cast<Relationship *>(object))
				{
					if(db_model->getObjectIndex(object->getSignature(), object->getObjectType()) >= 0)
						object->setName(CoreUtilsNs::generateUniqueName(object, *db_model->getObjectList(object->getObjectType()), false, "_cp"));

					if(graph_obj)
					{
						QPointF pnt = graph_obj->getPosition();
						graph_obj->setPosition(QPointF(pnt.x() + 50,
																					 pnt.y() + 50));
					}

					db_model->addObject(object);
				}

				//Special case for table objects
				if(tab_obj)
				{
					if(sel_table && tab_obj->getObjectType() == ObjectType::Column)
					{
						sel_table->addObject(tab_obj);
						sel_table->setModified(true);
					}
					else if(constr && duplicate_mode &&
							constr->getConstraintType() == ConstraintType::PrimaryKey &&
							constr->getParentTable()->getObjectIndex(constr) < 0)
					{
						constr->getParentTable()->addObject(constr);
						constr->getParentTable()->setModified(true);
					}

					//Updates the fk relationships if the constraint is a foreign-key
					if(constr && constr->getConstraintType() == ConstraintType::ForeignKey)
						db_model->updateTableFKRelationships(dynamic_cast<Table *>(tab_obj->getParentTable()));

					op_list->registerObject(tab_obj, Operation::ObjCreated, -1, tab_obj->getParentTable());
				}
				else
					op_list->registerObject(object, Operation::ObjCreated);
			}
			catch(Exception &e)
			{
				if(e.getErrorCode() != ErrorCode::AsgDuplicatedObject)
					errors.push_back(e);
			}
		}
	}
	op_list->finishOperationChain();

	//Validates the relationships to reflect any modification on the tables structures and not propagated columns
	db_model->validateRelationships();

	this->adjustSceneRect(false, true);
	task_prog_wgt.close();

	//If some error occur during the process show it to the user
	if(!errors.empty())
	{
		Messagebox msg_box;
		msg_box.show(Exception(tr("Not all objects were pasted to the model due to errors returned during the process! Refer to error stack for more details!"),
								 ErrorCode::Custom,PGM_FUNC,PGM_FILE,PGM_LINE, errors), "",
								 Messagebox::Alert);
	}

	if(!ModelWidget::cut_operation)
		emit s_objectCreated();
	else
	{
		//Remove the objects from the source model
		ModelWidget::src_model->selected_objects = ModelWidget::cut_objects;
		ModelWidget::src_model->removeObjects(false);

		//Uncheck the cut operation flag
		ModelWidget::cut_operation=false;

		copied_objects.clear();
		cut_objects.clear();

		if(this != ModelWidget::src_model)
			ModelWidget::src_model->configurePopupMenu();
	}

	ModelWidget::src_model = nullptr;
	this->configurePopupMenu();
	setModified(true);
	setBlinkAddedObjects(false);
}

void ModelWidget::duplicateObject()
{
	#ifdef DEMO_VERSION
		#warning "DEMO VERSION: limiting the user interaction over objects."
		if(updateObjActionCounter())
			return;
	#endif

	int op_id = -1;

	try
	{
		if(scene->hasOnlyTableChildrenSelection() ||
			 (selected_objects.size() == 1 && TableObject::isTableObject(selected_objects[0]->getObjectType())))
		{
			Schema *schema = nullptr;
			BaseObject *dup_object=nullptr;
			BaseTable *table = nullptr;
			ObjectType obj_type;
			QList<Schema *> upd_schemas;
			QList<BaseTable *> upd_view_ref_tables;
			QList<BaseTable *> upd_tables;
			QList<BaseTable *> upd_fk_rels;
			ObjectRenameWidget obj_rename_wgt(this);
			QString new_name;

			setBlinkAddedObjects(true);
			op_list->startOperationChain();

			for(auto &tab_obj : selected_objects)
			{
				dup_object = nullptr;
				obj_type = tab_obj->getObjectType();
				table = dynamic_cast<TableObject *>(tab_obj)->getParentTable();
				schema = dynamic_cast<Schema *>(table->getSchema());
				CoreUtilsNs::copyObject(&dup_object, tab_obj, obj_type);

				obj_rename_wgt.setAttributes(dup_object);		
				obj_rename_wgt.use_defaults_chk->setChecked(GeneralConfigWidget::
																										 getConfigurationParam(Attributes::Configuration,
																																						Attributes::UseDefDisambiguation) == Attributes::True);

				/* Ask the user a new object name by using an instance of ObjectRenameWidget
				 * If the user accept the dialog we use the typed name otherwise the
				 * original object name will be used and eventually disambigated */
				if(!obj_rename_wgt.use_defaults_chk->isChecked() &&
						obj_rename_wgt.exec() == QDialog::Accepted)
					dup_object->setName(obj_rename_wgt.getNewName());
				else
					new_name = dup_object->getName();

				GeneralConfigWidget::appendConfigurationSection(Attributes::Configuration,
																												 {{ Attributes::UseDefDisambiguation,
																														 obj_rename_wgt.use_defaults_chk->isChecked() ?
																																 Attributes::True : Attributes::False }});

				if(PhysicalTable::isPhysicalTable(table->getObjectType()))
				{
					dup_object->setName(CoreUtilsNs::generateUniqueName(dup_object,
																															*dynamic_cast<PhysicalTable *>(table)->getObjectList(obj_type),
																															false, "_cp", true));
				}
				else
				{
					dup_object->setName(CoreUtilsNs::generateUniqueName(dup_object,
																															*dynamic_cast<View *>(table)->getObjectList(obj_type),
																															false, "_cp", true));
				}

				op_id = op_list->registerObject(dup_object, Operation::ObjCreated, -1, table);
				table->addObject(dup_object);

				// Flagging the table to be repainted
				if(!upd_tables.contains(table))
					upd_tables.append(table);

				// Flagging the schema to be repainted
				if(!upd_schemas.contains(schema))
					upd_schemas.append(schema);

				// Flagging the table to have the view references (relationships) updated
				if(!upd_view_ref_tables.contains(table) && obj_type == ObjectType::Column)
					upd_view_ref_tables.append(table);
				// Flagging the table to have its fk relationships updated
				else if(!upd_fk_rels.contains(table) &&
								obj_type == ObjectType::Constraint &&
								dynamic_cast<Constraint *>(tab_obj)->getConstraintType() == ConstraintType::ForeignKey)
					upd_fk_rels.append(table);
			}

			op_list->finishOperationChain();
			scene->clearSelection();

			for(auto &tab : upd_tables)
				tab->setModified(true);

			for(auto &sch : upd_schemas)
				sch->setModified(true);

			for(auto &tab : upd_view_ref_tables)
			{
				db_model->validateRelationships();
				db_model->updateViewsReferencingTable(dynamic_cast<PhysicalTable *>(tab));
			}

			for(auto &tab : upd_fk_rels)
				db_model->updateTableFKRelationships(dynamic_cast<Table *>(tab));

			this->setModified(true);
			db_model->setInvalidated(true);
			emit s_objectCreated();
			setBlinkAddedObjects(false);
		}
		else if(!selected_objects.empty())
		{
			copyObjects(true, false);
			pasteObjects(true);
		}

		selected_objects.clear();
	}
	catch(Exception &e)
	{
		setBlinkAddedObjects(false);

		//If operation was registered
		if(op_id >= 0)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::removeObjects(bool cascade)
{
	int obj_idx=-1;
	unsigned count, op_count=0, obj_id=0;
	Table *aux_table=nullptr;
	BaseTable *table=nullptr, *src_table=nullptr, *dst_table=nullptr;
	BaseRelationship *rel=nullptr;
	TableObject *tab_obj=nullptr;
	ObjectType obj_type=ObjectType::BaseObject, parent_type=ObjectType::BaseObject;
	BaseObject *object=nullptr, *aux_obj=nullptr;
	std::vector<BaseObject *> sel_objs, aux_sel_objs;

	std::map<unsigned, std::tuple<BaseObject *, QString, ObjectType, QString, ObjectType>> objs_map;
	std::map<unsigned, std::tuple<BaseObject *, QString, ObjectType, QString, ObjectType>>::reverse_iterator ritr, ritr_end;
	QAction *obj_sender=dynamic_cast<QAction *>(sender());
	QString obj_name, parent_name;
	std::vector<Exception> errors;

	if(obj_sender)
		object=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

	if(!object)
		sel_objs=selected_objects;
	else
		sel_objs.push_back(object);

	if(!sel_objs.empty())
	{
		Messagebox msg_box;

		//Cancel the cut operation if the user try to delete an object in the middle of the process
		if(ModelWidget::cut_operation && (sender()==action_remove || sender()==action_cascade_del))
		{
			ModelWidget::cut_operation=false;
			copied_objects.clear();
		}

		//If the removal is not due to a cut operation, ask for permission to remove the objects
		if(!ModelWidget::cut_operation)
		{
			if(cascade)
				msg_box.show(tr("<strong>CAUTION:</strong> You are about to delete objects in cascade mode which means more objects than the selected will be dropped too. Do you really want to proceed?"),
							 Messagebox::Alert, Messagebox::YesNoButtons);
			else if(sel_objs.size() > 1)
			{
				msg_box.show(tr("<strong>CAUTION:</strong> Remove multiple objects at once can cause irreversible invalidations to other objects in the model causing such invalid objects to be deleted too. Do you really want to proceed?"),
							 Messagebox::Alert, Messagebox::YesNoButtons);
			}
			else
			{
				if(sel_objs[0]->getObjectType()==ObjectType::Relationship)
					msg_box.show(tr("<strong>CAUTION:</strong> Remove a relationship can cause irreversible invalidations to other objects in the model causing such invalid objects to be deleted too. Do you really want to proceed?"),
								 Messagebox::Alert, Messagebox::YesNoButtons);
				else
					msg_box.show(tr("Do you really want to delete the selected object?"),
								 Messagebox::Confirm, Messagebox::YesNoButtons);
			}
		}

		//If the user confirmed the removal or its a cut operation
		if(msg_box.isAccepted() || ModelWidget::cut_operation)
		{
			qApp->setOverrideCursor(Qt::WaitCursor);

			try
			{
				//If in cascade mode, retrieve all references to the object (direct and indirect)
				if(cascade)
				{
					for(BaseObject *sel_obj : sel_objs)
					{
						for(BaseObject *ref_obj : sel_obj->getReferences())
						{
							obj_id=ref_obj->getObjectId();
							tab_obj=dynamic_cast<TableObject *>(ref_obj);

							//Store the base relationships in a auxiliary list to be processed ahead
							if(ref_obj->getObjectType()==ObjectType::BaseRelationship)
							{
								aux_sel_objs.push_back(ref_obj);
							}
							//Insert the reference object to the list of objects to be removed
							else if(objs_map.count(obj_id)==0 &&
									(!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship())))
							{
								parent_type=(tab_obj ? tab_obj->getParentTable()->getObjectType() : ObjectType::Database);
								parent_name=(tab_obj ? tab_obj->getParentTable()->getName(true) : "");
								obj_name=(tab_obj ? tab_obj->getName() : ref_obj->getSignature());

								objs_map[ref_obj->getObjectId()]=std::make_tuple(ref_obj,
																				 obj_name,
																				 ref_obj->getObjectType(),
																				 parent_name,
																				 parent_type);
							}
						}
					}
				}

				sel_objs.insert(sel_objs.end(), aux_sel_objs.begin(), aux_sel_objs.end());

				for(BaseObject *object : sel_objs)
				{
					obj_type=object->getObjectType();
					obj_id=object->getObjectId();

					//If the object is as FK relationship remove the foreign keys that generates it
					if(obj_type==ObjectType::BaseRelationship)
					{
						rel = dynamic_cast<BaseRelationship *>(object);

						if(rel->getRelationshipType()==BaseRelationship::RelationshipFk)
						{
							tab_obj=rel->getReferenceForeignKey();
							obj_id=tab_obj->getObjectId();

							if(objs_map.count(obj_id)==0)
							{
								objs_map[tab_obj->getObjectId()]=std::make_tuple(tab_obj,
																				 tab_obj->getName(true),
																				 tab_obj->getObjectType(),
																				 tab_obj->getParentTable()->getName(true),
																				 ObjectType::Table);

							}
						}
					}
					else if(objs_map.count(obj_id)==0)
					{
						tab_obj=dynamic_cast<TableObject *>(object);
						obj_name=(tab_obj ? object->getName(true) : object->getSignature());

						parent_name=(tab_obj ? tab_obj->getParentTable()->getName(true) : "");
						parent_type=(tab_obj ? tab_obj->getParentTable()->getObjectType() : ObjectType::Database);

						objs_map[object->getObjectId()]=std::make_tuple(object,
																		obj_name,
																		obj_type,
																		parent_name,
																		parent_type);
					}
				}

				rel=nullptr;
				ritr=objs_map.rbegin();
				ritr_end=objs_map.rend();

				op_count=op_list->getCurrentSize();
				op_list->startOperationChain();

				do
				{
					object=std::get<0>(ritr->second);
					obj_name=std::get<1>(ritr->second);
					obj_type=std::get<2>(ritr->second);
					parent_name=std::get<3>(ritr->second);
					parent_type=std::get<4>(ritr->second);
					ritr++;

					if(obj_type==ObjectType::BaseRelationship)
						continue;

					if(parent_type!=ObjectType::Database)
					{
						/* If the parent table does not exist on the model of the object to be removed
						 * does not exists in parent table, it'll not be processed */
						table=dynamic_cast<BaseTable *>(db_model->getObject(parent_name, parent_type));
						if(!table || (table && table->getObjectIndex(obj_name, obj_type) < 0))
							continue;
					}
					else
					{
						//If the object does not exists on the model it'll not be processed.
						aux_obj = db_model->getObject(obj_name, obj_type);
						if(!aux_obj/* != object */)
							continue;
					}

					//Raises an error if the user try to remove a reserved object
					if(object->isSystemObject())
						throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
														.arg(object->getName()).arg(object->getTypeName()),
														ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

					//Raises an error if the user try to remove a protected object
					if(object->isProtected())
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
														.arg(object->getName(true))
														.arg(object->getTypeName()),
														ErrorCode::RemProtectedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
					}

					tab_obj=dynamic_cast<TableObject *>(object);

					if(tab_obj)
					{
						if(tab_obj->isAddedByRelationship())
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
															.arg(tab_obj->getName(true))
															.arg(tab_obj->getTypeName()),
															ErrorCode::RemProtectedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
						}

						table = tab_obj->getParentTable();
						obj_idx = table->getObjectIndex(tab_obj->getName(true), obj_type);

						try
						{
							//Register the removed object on the operation list
							op_list->registerObject(tab_obj, Operation::ObjRemoved, obj_idx, table);
							table->removeObject(obj_idx, obj_type);
							db_model->removePermissions(tab_obj);

							aux_table=dynamic_cast<Table *>(table);

							if(aux_table && obj_type==ObjectType::Constraint &&
								 (dynamic_cast<Constraint *>(tab_obj)->getConstraintType()==ConstraintType::ForeignKey ||
									dynamic_cast<Constraint *>(tab_obj)->getConstraintType()==ConstraintType::Unique))
								db_model->updateTableFKRelationships(aux_table);

							table->setModified(true);
							dynamic_cast<Schema *>(table->getSchema())->setModified(true);

							if(aux_table)
								db_model->validateRelationships(tab_obj, aux_table);

							if(obj_type == ObjectType::Column)
								db_model->updateViewsReferencingTable(aux_table);
						}
						catch(Exception &e)
						{
							if(cascade && (e.getErrorCode()==ErrorCode::RemInvalidatedObjects ||
														 e.getErrorCode()==ErrorCode::RemDirectReference ||
														 e.getErrorCode()==ErrorCode::RemInderectReference ||
														 e.getErrorCode()==ErrorCode::RemProtectedObject ||
														 e.getErrorCode()==ErrorCode::OprReservedObject))
								errors.push_back(e);
							else
								throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
						}
					}
					else
					{
						obj_idx=db_model->getObjectIndex(object);

						if(obj_idx >=0 )
						{
							if(obj_type==ObjectType::Relationship)
							{
								rel=dynamic_cast<BaseRelationship *>(object);
								src_table=rel->getTable(BaseRelationship::SrcTable);
								dst_table=rel->getTable(BaseRelationship::DstTable);
							}

							try
							{
								db_model->removeObject(object, obj_idx);
								op_list->registerObject(object, Operation::ObjRemoved, obj_idx);
							}
							catch(Exception &e)
							{
								if(cascade && (e.getErrorCode()==ErrorCode::RemInvalidatedObjects ||
															 e.getErrorCode()==ErrorCode::RemDirectReference ||
															 e.getErrorCode()==ErrorCode::RemInderectReference ||
															 e.getErrorCode()==ErrorCode::RemProtectedObject ||
															 e.getErrorCode()==ErrorCode::OprReservedObject))
									errors.push_back(e);
								else
									throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
							}

							if(rel)
							{
								src_table->setModified(true);
								dst_table->setModified(true);
								rel=nullptr;
								dst_table=src_table=nullptr;
							}
						}
					}
				}
				while(ritr!=ritr_end);

				op_list->finishOperationChain();
				scene->clearSelection();
				configurePopupMenu();
				setModified(true);
				emit s_objectRemoved();

				if(!errors.empty())
				{
					msg_box.show(Exception(ErrorCode::RemInvalidatedObjects, PGM_FUNC,PGM_FILE,PGM_LINE, errors),
								 tr("The cascade deletion found some problems when running! Some objects could not be deleted or registered in the operation's history! Please, refer to error stack for more details."),
								 Messagebox::Alert);
				}
			}
			catch(Exception &e)
			{
				if(op_list->isOperationChainStarted())
					op_list->finishOperationChain();

				if(op_count < op_list->getCurrentSize())
				{
					count=op_list->getCurrentSize()-op_count;
					op_list->ignoreOperationChain(true);

					for(unsigned i=0; i < count; i++)
						op_list->removeLastOperation();

					op_list->ignoreOperationChain(false);
				}

				scene->clearSelection();
				setModified(true);
				emit s_objectRemoved();

				//msg_box.show(e);
				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			}

			/* In case of any object removal we clear the copied objects list in order to avoid
			 * segfaults when trying to paste an object that was removed previously */
			copied_objects.clear();
			qApp->restoreOverrideCursor();
		}
	}
}

/* void ModelWidget::removeObjectsCascade()
{
	removeObjects(true);
} */

void ModelWidget::editCustomSQL()
{
	QAction *act=dynamic_cast<QAction *>(sender());
	BaseObject *obj=reinterpret_cast<BaseObject *>(act->data().value<void *>());
	CustomSQLWidget *customsql_wgt=new CustomSQLWidget;

	customsql_wgt->setAttributes(db_model, obj);
	setModified((openEditingForm(customsql_wgt)==QDialog::Accepted));
}

void ModelWidget::showObjectMenu()
{
	BaseTableView *tab=nullptr;

	/* When the popup is hidden check if there is a table object (colum, constraint, etc) selected,
		 if so, is necessary to reenable the table view deactivated before the menu activation */
	if(this->selected_objects.size()==1)
	{
		//Get the selected table object
		TableObject *tab_obj=dynamic_cast<TableObject *>(this->selected_objects[0]);

		if(tab_obj && tab_obj->getParentTable())
			//Get the graphical representation for table
			tab=dynamic_cast<BaseTableView *>(tab_obj->getParentTable()->getOverlyingObject());
	}

	showMagnifierArea(false);
	popup_menu.exec(QCursor::pos());

	//If the table object has a parent table
	if(tab)
	{
		//Reacitvates the table
		tab->setEnabled(true);
		//Calls the hoverLeaveEvent in order to hide the child selection
		tab->hoverLeaveEvent(nullptr);
	}
}

void ModelWidget::configurePopupMenu(BaseObject *object)
{
	std::vector<BaseObject *> vet_obj;

	if(object)
		vet_obj.push_back(object);

	configurePopupMenu(vet_obj);
}

void ModelWidget::enableModelActions(bool value)
{
	action_source_code->setEnabled(value);
	action_edit->setEnabled(value);
	action_protect->setEnabled(value);
	action_unprotect->setEnabled(value);
	action_select_all->setEnabled(value);
	action_convert_relnn->setEnabled(value);
	action_convert_rel1n->setEnabled(value);
	action_deps_refs->setEnabled(value);
	action_new_object->setEnabled(value);
	action_copy->setEnabled(value);
	action_duplicate->setEnabled(value);
	action_paste->setEnabled(value);
	action_cut->setEnabled(value);
	action_remove->setEnabled(value);
	action_cascade_del->setEnabled(value);
	action_quick_actions->setEnabled(value);
}

void ModelWidget::configureQuickMenu(BaseObject *object)
{
	QAction *act = nullptr;
	std::vector<BaseObject *> sel_objs;
	ObjectType obj_type=ObjectType::BaseObject;
	bool tab_or_view=false, is_graph_obj = false, accepts_owner=false, accepts_schema=false;

	if(object)
		sel_objs.push_back(object);
	else
		sel_objs = selected_objects;

	/* Determining if one or more selected objects accepts schema, owner or are table/views,
	 this is done to correctly show the actions to the user */
	for(auto &obj : sel_objs)
	{
		obj_type=obj->getObjectType();

		if(!tab_or_view)
			tab_or_view=BaseTable::isBaseTable(obj_type);

		if(!is_graph_obj)
			is_graph_obj = BaseGraphicObject::isGraphicObject(obj_type);

		if(!accepts_owner)
			accepts_owner=obj->acceptsOwner();

		if(!accepts_schema)
			accepts_schema=obj->acceptsSchema();

		if(tab_or_view && accepts_owner && accepts_schema)
			break;
	}

	if(!sel_objs.empty())
	{
		if(accepts_owner || accepts_schema)
		{
			std::vector<BaseObject *> obj_list;
			std::map<QString, QAction *> act_map;
			QMenu *menus[]={ &schemas_menu, &owners_menu, &tags_menu };
			ObjectType types[] { ObjectType::Schema, ObjectType::Role, ObjectType::Tag };

			for(unsigned i = 0; i < 3; i++)
			{
				menus[i]->clear();

				//Configuring actions "Move to schema", "Change Owner" and "Set tag"
				if((types[i] == ObjectType::Schema && accepts_schema) ||
						(types[i] == ObjectType::Role && accepts_owner) ||
						(types[i] == ObjectType::Tag && tab_or_view))
				{
					obj_list = db_model->getObjects(types[i]);

					if(obj_list.empty())
					{
						menus[i]->addAction(tr("(no objects)"));
						menus[i]->actions().at(0)->setEnabled(false);
					}
					else
					{
						if(types[i] == ObjectType::Tag)
						{
							menus[i]->addAction(tr("None"), this, &ModelWidget::setTag);
							menus[i]->addSeparator();
						}
						else if(types[i] == ObjectType::Role)
						{
							menus[i]->addAction(tr("None"), this, &ModelWidget::changeOwner);
							menus[i]->addSeparator();
						}

						for(auto &obj : obj_list)
						{
							//act = new QAction(obj_list.back()->getName(), menus[i]);
							act = new QAction(obj->getName(), menus[i]);
							act->setIcon(QIcon(GuiUtilsNs::getIconPath(types[i])));

							/* Check the current action only if there is only one selected object and the object representing
								 the action is assigned to the selected object */
							act->setCheckable(sel_objs.size() == 1);
							act->setChecked(sel_objs.size() == 1 &&
															(object->getSchema() == obj ||
															 object->getOwner() == obj ||
															 (tab_or_view && dynamic_cast<BaseTable *>(sel_objs[0])->getTag() == obj)));

							act->setEnabled(!act->isChecked());
							act->setData(QVariant::fromValue<void *>(obj));

							if(i == 0) {
								connect(act, &QAction::triggered, this, &ModelWidget::moveToSchema);
							}
							else if(i == 1)	{
								connect(act, &QAction::triggered, this, &ModelWidget::changeOwner);
							}
							else
								connect(act, &QAction::triggered, this, &ModelWidget::setTag);

							act_map[obj->getName()] = act;
						}

						for(auto &[act_name, act] : act_map)
							menus[i]->addAction(act);

						act_map.clear();
					}
				}
			}
		}

		//Display the quick rename action is a single object is selected
		if((object && obj_type != ObjectType::Cast) || (sel_objs.size() > 1))
		{
			quick_actions_menu.addAction(action_rename);			
			action_rename->setData(QVariant::fromValue<void *>(object));
		}

		if(is_graph_obj)
			quick_actions_menu.addAction(action_set_layer);

		if(accepts_schema)
			quick_actions_menu.addAction(action_moveto_schema);

		if(accepts_owner)
			quick_actions_menu.addAction(action_change_owner);

		if(tab_or_view)
			quick_actions_menu.addAction(action_set_tag);

		if(object && Permission::acceptsPermission(obj_type))
		{
			quick_actions_menu.addAction(action_edit_perms);
			action_edit_perms->setData(QVariant::fromValue<void *>(object));
		}

		if(object && PhysicalTable::isPhysicalTable(obj_type))
			quick_actions_menu.addAction(action_edit_data);

		if(object && BaseObject::acceptsCustomSQL(obj_type))
		{
			action_append_sql->setData(QVariant::fromValue<void *>(object));
			quick_actions_menu.addAction(action_append_sql);
		}

		if(object &&
				((!TableObject::isTableObject(obj_type) && obj_type!=ObjectType::Textbox && obj_type!=ObjectType::BaseRelationship) ||
				 (TableObject::isTableObject(obj_type) && !dynamic_cast<TableObject*>(object)->isAddedByRelationship())))
		{
			action_enable_sql->setData(QVariant::fromValue<void *>(object));
			action_disable_sql->setData(QVariant::fromValue<void *>(object));

			if(object->isSQLDisabled())
				quick_actions_menu.addAction(action_enable_sql);
			else
				quick_actions_menu.addAction(action_disable_sql);
		}

		//Include the quick actions if it is not empty and the model is not protected
		if(!db_model->isProtected() && !quick_actions_menu.isEmpty())
			popup_menu.addAction(action_quick_actions);
	}
}

void ModelWidget::configureFadeMenu()
{
	bool is_db_selected = (selected_objects.empty() ||
												 (selected_objects.size() == 1 && selected_objects[0]->getObjectType() == ObjectType::Database));
	fade_menu.clear();

	if(is_db_selected || (selected_objects.size() > 1 && !scene->hasOnlyTableChildrenSelection()))
	{
		if(is_db_selected)
		{
			fade_menu.addAction(fade_in_all_menu.menuAction());
			fade_menu.addAction(fade_out_all_menu.menuAction());
		}
		else
		{
			fade_menu.addAction(action_fade_in);
			fade_menu.addAction(action_fade_out);
		}
	}
	else if(selected_objects.size() == 1)
	{
		ObjectType obj_type = selected_objects[0]->getObjectType();

		if(obj_type == ObjectType::Tag)
		{
			fade_menu.addAction(action_fade_in);
			fade_menu.addAction(action_fade_out);
		}
		else
		{
			BaseObjectView *obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(selected_objects[0])->getOverlyingObject());

			if(obj_view)
			{
				if(obj_view->opacity() == 1)
					fade_menu.addAction(action_fade_out);
				else
					fade_menu.addAction(action_fade_in);
			}

			if(BaseTable::isBaseTable(obj_type))
			{
				fade_menu.addSeparator();
				fade_menu.addAction(action_fade_rels);
				fade_menu.addAction(action_fade_peer_tables);
				fade_menu.addAction(action_fade_tabs_rels);
			}
		}
	}
}

void ModelWidget::fadeObjects(const std::vector<BaseObject *> &objects, bool fade_in)
{
	BaseObjectView *obj_view = nullptr;
	Schema *schema = nullptr;

	for(auto obj : objects)
	{
	  schema = dynamic_cast<Schema *>(obj);

	  if(!BaseGraphicObject::isGraphicObject(obj->getObjectType()) ||
		 (schema && !schema->isRectVisible()))
			continue;

		obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(obj)->getOverlyingObject());

		if(obj_view)
		{
			dynamic_cast<BaseGraphicObject *>(obj)->setFadedOut(!fade_in);

			if(fade_in)
				obj_view->fadeIn();
			else
				obj_view->fadeOut(min_object_opacity);

			//If the minimum opacity is zero the object hidden
			obj_view->setVisible(scene->isLayersActive(obj_view->getLayers()) &&
													 (fade_in || (!fade_in && min_object_opacity > 0)));

			setModified(true);
		}
	}

	scene->clearSelection();
}

void ModelWidget::fadeObjects(QAction *action, bool fade_in)
{
	if(!action)
		return;

	std::vector<BaseObject *> list;

	//If the database object is selected or there is no object select
	if(selected_objects.empty() || (selected_objects.size() == 1 && selected_objects[0]->getObjectType() == ObjectType::Database))
	{
		ObjectType obj_type = static_cast<ObjectType>(action->data().toUInt());

		//If the action contains a data of type ObjectType::ObjBaseObject means that the user wants to fade all objects
		if(obj_type == ObjectType::BaseObject)
		{
			std::vector<ObjectType> types = { ObjectType::Schema, ObjectType::Table, ObjectType::View,
																	 ObjectType::Relationship, ObjectType::BaseRelationship, ObjectType::Textbox};

			for(ObjectType type : types)
			{
				list.insert(list.end(),
										db_model->getObjectList(type)->begin(),
										db_model->getObjectList(type)->end());
			}
		}
		else
		{
			//Fading objects of a certain type
			list = *db_model->getObjectList(obj_type);

			if(obj_type == ObjectType::Relationship)
			{
				list.insert(list.end(),
										db_model->getObjectList(ObjectType::BaseRelationship)->begin(),
										db_model->getObjectList(ObjectType::BaseRelationship)->end());
			}
		}
	}
	else
	{
		//For tag object the fade is applied in the tables/views related to it
		if(selected_objects.size() == 1 && selected_objects[0]->getObjectType() == ObjectType::Tag)
			list = selected_objects[0]->getReferences();
		else
		{
			bool fade_rels = action == action_fade_rels_in || action == action_fade_rels_out,
					fade_peer_tabs = action == action_fade_peer_tables_in || action == action_fade_peer_tables_out,
					fade_both_objs = action == action_fade_tabs_rels_in || action == action_fade_tabs_rels_out;

			if(fade_rels || fade_peer_tabs || fade_both_objs)
			{
				//Applying fade to the relationships linked to the selected table/view
				std::vector<BaseRelationship *> rel_list = db_model->getRelationships(dynamic_cast<BaseTable *>(selected_objects[0]));

				for(auto rel : rel_list)
				{
					if(fade_rels || fade_both_objs)
						list.push_back(rel);

					if(fade_peer_tabs || fade_both_objs)
					{
						list.push_back(rel->getTable(BaseRelationship::SrcTable));
						list.push_back(rel->getTable(BaseRelationship::DstTable));
					}
				}

				std::vector<BaseObject *>::iterator end;
				std::sort(list.begin(), list.end());
				end=std::unique(list.begin(), list.end());
				list.erase(end, list.end());
			}
			else
				//Applying fade to the selected objects
				list = selected_objects;
		}
	}

	fadeObjects(list, fade_in);
	scene->clearSelection();
}

void ModelWidget::fadeObjectsIn()
{
	fadeObjects(qobject_cast<QAction *>(sender()), true);
}

void ModelWidget::fadeObjectsOut()
{
	fadeObjects(qobject_cast<QAction *>(sender()), false);
}

void ModelWidget::setAllCollapseMode(BaseTable::CollapseMode mode)
{
	BaseTable *base_tab = nullptr;
	std::vector<BaseObject *> objects;

	this->scene->clearSelection();
	objects.assign(db_model->getObjectList(ObjectType::Table)->begin(), db_model->getObjectList(ObjectType::Table)->end());
	objects.insert(objects.end(), db_model->getObjectList(ObjectType::View)->begin(), db_model->getObjectList(ObjectType::View)->end());
	objects.insert(objects.end(), db_model->getObjectList(ObjectType::ForeignTable)->begin(), db_model->getObjectList(ObjectType::ForeignTable)->end());

	for(auto obj : objects)
	{
		base_tab = dynamic_cast<BaseTable *>(obj);

		if(base_tab)
			base_tab->setCollapseMode(mode);
	}

	this->setModified(true);
}

void ModelWidget::moveObjectsInZStack(int direction)
{
	BaseObjectView *obj_view = nullptr;
	BaseTableView *tab_view = nullptr;
	TextboxView *tbox_view = nullptr;
	QList<QGraphicsItem *> items;
	int zval = 0;
	bool changed = false;

	op_list->startOperationChain();

	for(auto &obj : selected_objects)
	{
		// Ignoring if the object is not a table or textbox
		if(!BaseTable::isBaseTable(obj->getObjectType()) &&
			 obj->getObjectType() != ObjectType::Textbox)
			continue;

		obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(obj)->getOverlyingObject());

		// We'll check the items around the one we selected to move in the z stack
		items = obj_view->collidingItems();

		if(!obj_view->collidingItems().isEmpty())
		{
			zval = obj_view->zValue();

			for(auto &item : items)
			{
				tab_view  = dynamic_cast<BaseTableView *>(item);
				tbox_view  = dynamic_cast<TextboxView *>(item);

				// Ignoring if the object is not a table or textbox
				if(!tab_view && !tbox_view)
					continue;

				if((direction == BringToFront && zval < item->zValue()) ||
					 (direction == SendToBack && zval > item->zValue()))
					zval = item->zValue();
			}

			op_list->registerObject(obj_view->getUnderlyingObject(), Operation::ObjModified);

			zval += (1 * direction);

			if(zval < BaseGraphicObject::MinZValue)
				zval = BaseGraphicObject::MinZValue;
			else if(zval > BaseGraphicObject::MaxZValue)
				zval = BaseGraphicObject::MaxZValue;

			obj_view->setZValue(zval);
			changed = true;
		}
	}

	op_list->finishOperationChain();
	setModified(changed);

	if(changed)
		emit s_objectModified();
}

void ModelWidget::setCollapseMode()
{
	BaseTable::CollapseMode mode = static_cast<BaseTable::CollapseMode>(dynamic_cast<QAction *>(sender())->data().toUInt());
	BaseTable *base_tab = nullptr;
	std::vector<BaseObject *> objects;

	if(selected_objects.empty() || (selected_objects.size() == 1 && selected_objects[0] == db_model))
	{
		objects.assign(db_model->getObjectList(ObjectType::Table)->begin(), db_model->getObjectList(ObjectType::Table)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::ForeignTable)->begin(), db_model->getObjectList(ObjectType::ForeignTable)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::View)->begin(), db_model->getObjectList(ObjectType::View)->end());
	}
	else
		objects = selected_objects;

	for(auto obj : objects)
	{
		base_tab = dynamic_cast<BaseTable *>(obj);

		if(base_tab && base_tab->getCollapseMode() != mode)
		{
			base_tab->setCollapseMode(mode);
			base_tab->setModified(true);
		}
	}

	db_model->setObjectsModified({ ObjectType::Schema });
	this->setModified(true);
}

void ModelWidget::togglePagination()
{
	bool enable = dynamic_cast<QAction *>(sender())->data().toBool();
	BaseTable *base_tab = nullptr;
	std::vector<BaseObject *> objects;

	if(selected_objects.empty() || (selected_objects.size() == 1 && selected_objects[0] == db_model))
	{
		objects.assign(db_model->getObjectList(ObjectType::Table)->begin(), db_model->getObjectList(ObjectType::Table)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::ForeignTable)->begin(), db_model->getObjectList(ObjectType::ForeignTable)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::View)->begin(), db_model->getObjectList(ObjectType::View)->end());
	}
	else
		objects = selected_objects;

	for(auto obj : objects)
	{
		base_tab = dynamic_cast<BaseTable *>(obj);

		if(base_tab && base_tab->isPaginationEnabled() != enable)
		{
			base_tab->setPaginationEnabled(enable);
			base_tab->setModified(true);
		}
	}

	db_model->setObjectsModified({ ObjectType::Schema });
	this->setModified(true);
}

void ModelWidget::toggleSchemasRectangles()
{
	bool visible = sender() == action_show_schemas_rects;
	Schema *schema = nullptr;

	for(auto obj : *db_model->getObjectList(ObjectType::Schema))
	{
		schema = dynamic_cast<Schema *>(obj);

		if(schema && schema->isRectVisible() != visible)
		{
			schema->setRectVisible(visible);
			schema->setModified(true);
		}
	}

	db_model->setShowSysSchemasRects(visible);

	db_model->setObjectsModified({ ObjectType::Table,
																 ObjectType::ForeignTable,
																 ObjectType::View });
	this->setModified(true);
}

void ModelWidget::updateObjectsOpacity()
{
	std::vector<ObjectType> types = { ObjectType::Schema, ObjectType::Table, ObjectType::View,
															 ObjectType::Relationship, ObjectType::BaseRelationship, ObjectType::Textbox};
	BaseObjectView *obj_view = nullptr;
	BaseGraphicObject *base_obj = nullptr;

	for(auto type : types)
	{
		for(auto object : *db_model->getObjectList(type))
		{
			base_obj = dynamic_cast<BaseGraphicObject *>(object);
			obj_view = dynamic_cast<BaseObjectView *>(base_obj->getOverlyingObject());

			if(obj_view &&
				 ((base_obj->isFadedOut() && obj_view->opacity() == 1) ||
					(obj_view->opacity() < 1.0 && obj_view->opacity() != min_object_opacity)))
			{
				obj_view->setOpacity(min_object_opacity);
				obj_view->setVisible(min_object_opacity > 0);
			}
		}
	}
}

void ModelWidget::configureConstraintsMenu(TableObject *tab_obj)
{
	if(!tab_obj)
		return;

	PhysicalTable *table=dynamic_cast<PhysicalTable *>(tab_obj->getParentTable());

	if(tab_obj->getObjectType()==ObjectType::Column)
	{
		unsigned count = table->getConstraintCount();
		Constraint *constr = nullptr;
		QString str_aux;
		QAction *action = nullptr;
		QMenu *submenu;
		QList<QMenu *> submenus;

		for(unsigned i=0; i < count; i++)
		{
			constr=table->getConstraint(i);
			if(constr->isColumnReferenced(dynamic_cast<Column *>(tab_obj), false))
			{
				switch(!constr->getConstraintType())
				{
					case ConstraintType::PrimaryKey: str_aux=QString("_%1").arg(TableObjectView::TextPrimaryKey); break;
					case ConstraintType::ForeignKey: str_aux=QString("_%1").arg(TableObjectView::TextForeignKey); break;
					case ConstraintType::Check: str_aux=QString("_%1").arg(TableObjectView::TextCheck); break;
					case ConstraintType::Unique: str_aux=QString("_%1").arg(TableObjectView::TextUnique); break;
					case ConstraintType::Exclude: str_aux=QString("_%1").arg(TableObjectView::TextExclude); break;
				}

				//For each constaint is created a menu with the edit, source code, protect/unprotect and delete actions
				submenu=new QMenu(&popup_menu);
				submenu->setIcon(QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(ObjectType::Constraint) + str_aux)));
				submenu->setTitle(constr->getName());

				action=new QAction(dynamic_cast<QObject *>(submenu));
				action->setIcon(QIcon(GuiUtilsNs::getIconPath("edit")));
				action->setText(tr("Properties"));
				action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
				connect(action, &QAction::triggered, this, &ModelWidget::editObject);
				submenu->addAction(action);

				action=new QAction(dynamic_cast<QObject *>(submenu));
				action->setIcon(QIcon(GuiUtilsNs::getIconPath("sourcecode")));
				action->setText(tr("Source code"));
				action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
				connect(action, &QAction::triggered, this, &ModelWidget::showSourceCode);
				submenu->addAction(action);

				if(!constr->isAddedByRelationship())
				{
					if(!constr->getParentTable()->isProtected())
					{
						action=new QAction(dynamic_cast<QObject *>(&popup_menu));
						action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
						connect(action, &QAction::triggered, this, &ModelWidget::protectObject);
						submenu->addAction(action);

						if(constr->isProtected())
						{
							action->setIcon(QIcon(GuiUtilsNs::getIconPath("unprotect")));
							action->setText(tr("Unprotect"));
						}
						else
						{
							action->setIcon(QIcon(GuiUtilsNs::getIconPath("protect")));
							action->setText(tr("Protect"));
						}
					}

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(QIcon(GuiUtilsNs::getIconPath("delete")));
					action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
					action->setText(tr("Delete"));
					submenu->addAction(action);

					connect(action, &QAction::triggered, this, [this](){
						removeObjects(false);
					});

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(QIcon(GuiUtilsNs::getIconPath("delcascade")));
					action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
					action->setText(tr("Del. cascade"));
					submenu->addAction(action);

					connect(action, &QAction::triggered, this, [this](){
						removeObjects(true);
					});
				}
				submenus.push_back(submenu);
			}
		}

		//Adding the constraint submenus to the main popup menu
		if(!submenus.empty())
		{
			submenu=new QMenu(&popup_menu);
			submenu->setTitle(tr("Constraints"));
			submenu->setIcon(QIcon(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(ObjectType::Constraint))));

			for(auto &menu : submenus)
				submenu->addMenu(menu);

			popup_menu.insertMenu(action_edit, submenu);
		}
	}

}

void ModelWidget::configureBasicActions(BaseObject *obj)
{
	if(!obj)
		return;

	BaseRelationship *rel=dynamic_cast<BaseRelationship *>(obj);
	ObjectType obj_type=obj->getObjectType();
	QAction *action = nullptr;

	configureQuickMenu(obj);
	popup_menu.addAction(action_edit);

	if((obj_type==ObjectType::Schema && obj->isSystemObject()) ||
			(!obj->isProtected() && (BaseTable::isBaseTable(obj_type) || obj_type==ObjectType::BaseRelationship ||
															 obj_type==ObjectType::Relationship || obj_type==ObjectType::Schema ||
															 obj_type == ObjectType::Tag)))
	{
		if(BaseTable::isBaseTable(obj_type))
		{
			for(auto type : BaseObject::getChildObjectTypes(obj_type))
				new_object_menu.addAction(actions_new_objects[type]);

			if(obj_type==ObjectType::Table)
				new_object_menu.addAction(actions_new_objects[ObjectType::Relationship]);

			popup_menu.insertAction(action_quick_actions, action_new_object);
			popup_menu.addAction(action_sel_table_rels);
			action_sel_table_rels->setData(QVariant::fromValue<void *>(obj));
		}
		else if(obj_type==ObjectType::Relationship || obj_type==ObjectType::BaseRelationship)
		{
			if(obj_type==ObjectType::Relationship)
			{
				new_object_menu.addAction(actions_new_objects[ObjectType::Column]);
				new_object_menu.addAction(actions_new_objects[ObjectType::Constraint]);
				popup_menu.insertAction(action_quick_actions, action_new_object);
			}

			if(rel->getRelationshipType()==Relationship::RelationshipNn)
			{
				action_convert_relnn->setData(QVariant::fromValue<void *>(rel));
				popup_menu.addAction(action_convert_relnn);
			}

			if(rel->getRelationshipType() == Relationship::Relationship11 ||
				 rel->getRelationshipType() == Relationship::Relationship1n)
			{
				action_convert_rel1n->setData(QVariant::fromValue<void *>(rel));
				popup_menu.addAction(action_convert_rel1n);
			}

			if(!rel->isSelfRelationship())
			{
				if(rel->getPoints().empty())
				{
					action_break_rel_line->setData(QVariant::fromValue<void *>(rel));
					popup_menu.addAction(action_break_rel_line);
				}
				else
				{
					action_remove_rel_points->setData(QVariant::fromValue<void *>(rel));
					popup_menu.addAction(action_remove_rel_points);
				}

				popup_menu.addAction(action_jump_to_table);
				jump_to_tab_menu.clear();

				action = jump_to_tab_menu.addAction(QIcon(GuiUtilsNs::getIconPath(rel->getTable(BaseRelationship::SrcTable)->getObjectType())),
																						rel->getTable(BaseRelationship::SrcTable)->getSignature(), this, &ModelWidget::jumpToTable);
				action->setData(QVariant::fromValue<void *>(reinterpret_cast<void *>(rel->getTable(BaseRelationship::SrcTable))));

				action = jump_to_tab_menu.addAction(QIcon(GuiUtilsNs::getIconPath(rel->getTable(BaseRelationship::DstTable)->getObjectType())),
																						rel->getTable(BaseRelationship::DstTable)->getSignature(), this, &ModelWidget::jumpToTable);
				action->setData(QVariant::fromValue<void *>(reinterpret_cast<void *>(rel->getTable(BaseRelationship::DstTable))));
			}
		}
		else if(obj_type == ObjectType::Schema)
		{
			for(auto type : BaseObject::getChildObjectTypes(ObjectType::Schema))
				new_object_menu.addAction(actions_new_objects[type]);

			popup_menu.insertAction(action_quick_actions, action_new_object);
			popup_menu.addAction(action_sel_sch_children);
			action_sel_sch_children->setData(QVariant::fromValue<void *>(obj));
		}
		else if(obj_type == ObjectType::Tag)
		{
			popup_menu.addAction(action_sel_tagged_tabs);
			action_sel_tagged_tabs->setData(QVariant::fromValue<void *>(obj));
		}
	}

	/* Adding the action to highlight the object only when the sender is not one of the
	the objects that calls this method from inside the ModelWidget instance. This action
	is mainly used when the user wants to find a graphical object from the ModelObjects dockwidget*/
	if((sender()!=this && sender()!=scene) && dynamic_cast<BaseGraphicObject *>(obj))
	{
		popup_menu.addAction(action_select_object);
		action_select_object->setData(QVariant::fromValue<void *>(obj));
	}

	action_edit->setData(QVariant::fromValue<void *>(obj));
	action_deps_refs->setData(QVariant::fromValue<void *>(obj));

	TableObject *tab_obj = dynamic_cast<TableObject *>(obj);

	if(tab_obj &&
			(tab_obj->getObjectType() == ObjectType::Column ||
			 tab_obj->getObjectType() == ObjectType::Constraint ||
			 tab_obj->getObjectType() == ObjectType::Index))
	{
		Column *col = dynamic_cast<Column *>(tab_obj);

		if(tab_obj->isAddedByRelationship())
		{
			action_parent_rel->setData(QVariant::fromValue<void *>(tab_obj->getParentRelationship()));
			popup_menu.addAction(action_parent_rel);
		}
		else if(col && col->getType().isSerialType())
		{
			action_create_seq_col->setData(QVariant::fromValue<void *>(col));
			popup_menu.addAction(action_create_seq_col);
		}
		else if(col && col->getType().isIntegerType())
		{
			action_conv_int_serial->setData(QVariant::fromValue<void *>(col));
			popup_menu.addAction(action_conv_int_serial);
		}
	}

	popup_menu.addSeparator();
	popup_menu.addAction(action_source_code);

	if(!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship()))
		popup_menu.addAction(action_deps_refs);
}

void ModelWidget::configureDatabaseActions()
{
	new_object_menu.addAction(action_database_category);
	new_object_menu.addAction(action_schema_category);
	new_object_menu.addAction(actions_new_objects[ObjectType::Relationship]);
	new_object_menu.addAction(actions_new_objects[ObjectType::GenericSql]);
	new_object_menu.addAction(actions_new_objects[ObjectType::Tag]);
	new_object_menu.addAction(actions_new_objects[ObjectType::Textbox]);

	popup_menu.addAction(action_new_object);
	configureQuickMenu(db_model);

	action_edit->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(db_model)));

	popup_menu.addAction(action_edit);

	popup_menu.addSeparator();
	popup_menu.addAction(action_source_code);

	if(db_model->isProtected())
		popup_menu.addAction(action_unprotect);
	else
		popup_menu.addAction(action_protect);

	if(scene->items().count() > 1)
		popup_menu.addAction(action_select_all);
}

void ModelWidget::configurePopupMenu(const std::vector<BaseObject *> &objects)
{
	new_object_menu.clear();
	quick_actions_menu.clear();
	popup_menu.clear();

	/* We do not configure the popup menu if the viewport
	 * has the intaraction disabled */
	if(!viewport->isInteractive())
		return;

	unsigned count = 0, i = 0;
	std::vector<QMenu *> submenus;
	TableObject *tab_obj=nullptr;
	bool protected_obj=false, model_protected=db_model->isProtected();

	enableModelActions(false);
	selected_objects=objects;
	new_object_menu.setEnabled(!this->db_model->isProtected());

	if(objects.size() <= 1)
	{
		//Case there is no selected object or the selected object is the database model
		if(objects.empty() || (objects.size()==1 && objects[0]==db_model))
			configureDatabaseActions();
		else if(objects.size() == 1)
		{
			tab_obj = dynamic_cast<TableObject *>(objects.front());
			configureBasicActions(objects.front());
		}		
	}
	else
	{
		configureQuickMenu(nullptr);
		popup_menu.addSeparator();
		popup_menu.addAction(action_source_code);
	}

	if(objects.size() > 1)
	{
		bool rem_points = true;

		for(auto &obj : objects)
		{
			rem_points = obj->getObjectType() == ObjectType::Relationship || obj->getObjectType() == ObjectType::BaseRelationship;
			if(!rem_points) break;
		}

		if(rem_points)
		{
			action_remove_rel_points->setData(QVariant());
			popup_menu.addAction(action_remove_rel_points);
		}
	}

	/* Adds the protect/unprotect action when the selected object was not included by relationship
	and if its a table object and the parent table is not protected. */
	if(!objects.empty() &&
			!this->db_model->isProtected() &&
			(!tab_obj || (tab_obj && !tab_obj->getParentTable()->isProtected() && !tab_obj->isAddedByRelationship())))
	{
		/* Special case for systema objects: The actions protect/unprotect will be displayed only for
		 system schemas. The rest of system objects those actions aren't available */
		if(!objects[0]->isSystemObject() ||
				(objects[0]->isSystemObject() && objects[0]->getObjectType()==ObjectType::Schema))
		{
			if(!objects[0]->isProtected())
				popup_menu.addAction(action_protect);
			else
				popup_menu.addAction(action_unprotect);
		}
	}

	//Adding the extended attributes action (only for table/view/database)
	if(objects.size() > 1 ||
		 (objects.empty() && (db_model->getObjectCount(ObjectType::Table) > 0 ||
													db_model->getObjectCount(ObjectType::ForeignTable) > 0 ||
													db_model->getObjectCount(ObjectType::View) > 0)) ||
		 (objects.size() == 1 && (BaseTable::isBaseTable(objects[0]->getObjectType()) ||
															objects[0]->getObjectType() == ObjectType::Database)))
	{
		bool tab_or_view = false;

		for(BaseObject *obj : objects)
		{
			if(!tab_or_view)
			{
				tab_or_view=(PhysicalTable::isPhysicalTable(obj->getObjectType()) || obj->getObjectType()==ObjectType::View);
				break;
			}
		}

		if(tab_or_view ||  objects.empty() || objects.size() == 1)
		{
			popup_menu.addAction(action_collapse_mode);
			popup_menu.addAction(action_pagination);
		}

		if(objects.empty() || (objects.size() == 1 && objects[0]->getObjectType() == ObjectType::Database))
			popup_menu.addAction(action_schemas_rects);
	}

	if((objects.size() == 1 &&
			(BaseTable::isBaseTable(objects[0]->getObjectType()) ||
			 objects[0]->getObjectType() == ObjectType::Textbox)) ||
		 (objects.size() > 1 && !scene->hasOnlyTableChildrenSelection()))
		popup_menu.addAction(action_stacking);

	if(!tab_obj &&
		 (objects.empty() || (objects.size() > 1 && !scene->hasOnlyTableChildrenSelection()) ||
			(objects.size() == 1 && (objects[0]->getObjectType() == ObjectType::Database ||
															 objects[0]->getObjectType() == ObjectType::Tag ||
															 BaseGraphicObject::isGraphicObject(objects[0]->getObjectType())))))
	{
		//Adding fade inout action only for graphical objects or when there is no objects selected or many objects seleted
		popup_menu.addAction(action_fade);
		popup_menu.addSeparator();
		configureFadeMenu();
	}

	//Adding the copy and paste if there is selected objects
	if(!model_protected &&
			!(objects.size()==1 && (objects[0]==db_model || objects[0]->getObjectType()==ObjectType::BaseRelationship)) &&
			!objects.empty())
	{
		popup_menu.addAction(action_copy);

		count=objects.size();
		i=0;
		while(i < count && !protected_obj)
			protected_obj=objects[i++]->isProtected();

		if(!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship()))
			popup_menu.addAction(action_cut);

		popup_menu.addAction(action_duplicate);
	}

	//If there is copied object adds the paste action
	if(!model_protected && !copied_objects.empty())
		popup_menu.addAction(action_paste);

	/* Adding the delete object action. This action will be unavailable on following conditions:
	1) The selected object is the database itself
	2) The object is protected
	3) The object is table child object and it was added by relationship
	4) The object is a base relationship (table-view) */
	if((tab_obj && !tab_obj->isAddedByRelationship() && !tab_obj->isProtected()) ||
			(objects.size()==1 && objects[0]->isProtected()) ||
			(!tab_obj && objects.size()==1 && objects[0]!=db_model && objects[0]->getObjectType()!=ObjectType::BaseRelationship) ||
			(objects.size()==1 && objects[0]->getObjectType()==ObjectType::BaseRelationship &&
			 dynamic_cast<BaseRelationship *>(objects[0])->getRelationshipType()==BaseRelationship::RelationshipFk) ||
			objects.size() > 1)
	{
		popup_menu.addAction(action_remove);
		popup_menu.addAction(action_cascade_del);
	}

	//If the table object is a column creates a special menu to acess the constraints that is applied to the column
	configureConstraintsMenu(tab_obj);

	//Enable the popup actions that are visible
	QList<QAction *> actions = popup_menu.actions();
	actions.append(quick_actions_menu.actions());

	for(auto &act : actions)
		act->setEnabled(true);

	if(objects.size() <= 2 && !scene->hasOnlyTableChildrenSelection())
	{
		popup_menu.addSeparator();
		popup_menu.addAction(action_edit_creation_order);
	}

	configurePluginsActions();
}

bool ModelWidget::isModified()
{
	return modified;
}

DatabaseModel *ModelWidget::getDatabaseModel()
{
	return db_model;
}

ObjectsScene *ModelWidget::getObjectsScene()
{
	return scene;
}

QGraphicsView *ModelWidget::getViewport()
{
	return viewport;
}

OperationList *ModelWidget::getOperationList()
{
	return op_list;
}

std::vector<BaseObject *> ModelWidget::getSelectedObjects()
{
	return selected_objects;
}

bool ModelWidget::hasSelectedObjects()
{
	return !selected_objects.empty();
}

void ModelWidget::setSaveLastCanvasPosition(bool value)
{
	ModelWidget::save_restore_pos=value;
}

void ModelWidget::setRenderSmoothnessDisabled(bool value)
{
	ModelWidget::disable_render_smooth=value;
}

void ModelWidget::setSimplifiedObjectCreation(bool value)
{
	ModelWidget::simple_obj_creation=value;
}

void ModelWidget::setMinimumObjectOpacity(unsigned min_opacity)
{
	if(min_opacity > 70)
		min_opacity = 70;

	ModelWidget::min_object_opacity = static_cast<double>(min_opacity)/100.0;
}

void ModelWidget::highlightObject()
{
	QAction *action=dynamic_cast<QAction *>(sender());

	if(action )
	{
		BaseObject *obj=reinterpret_cast<BaseObject *>(action->data().value<void *>());
		BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(obj);

		if(graph_obj)
		{
			BaseObjectView *obj_view=dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

			scene->clearSelection();
			obj_view->setSelected(true);
			viewport->centerOn(obj_view);
		}
	}
}

void ModelWidget::toggleNewObjectOverlay()
{
	if(new_obj_overlay_wgt->isHidden() &&
		(selected_objects.empty() ||
		(selected_objects[0]->getObjectType()!=ObjectType::BaseRelationship &&
		selected_objects[0]->getObjectType()!=ObjectType::Textbox)))
	{
		new_obj_overlay_wgt->raise();
		new_obj_overlay_wgt->show();
		new_obj_overlay_wgt->setSelectedObjects(selected_objects);
		this->adjustOverlayPosition();
	}
	else
			new_obj_overlay_wgt->hide();
}

void ModelWidget::adjustOverlayPosition()
{
	int px=0, py=0;

	px=(this->width()/2) - (new_obj_overlay_wgt->width()/2);
	py=(this->height()/2) - (new_obj_overlay_wgt->height()/2);
	new_obj_overlay_wgt->move(px, py);
}

void ModelWidget::toggleObjectSQL()
{
	QAction *action=dynamic_cast<QAction *>(sender());

	if(action)
	{
		BaseObject *object=reinterpret_cast<BaseObject *>(action->data().value<void *>());
		GuiUtilsNs::disableObjectSQL(object, !object->isSQLDisabled());
		setModified(true);
		emit s_objectModified();
	}
}

void ModelWidget::createSequenceFromColumn()
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		Column *col=reinterpret_cast<Column *>(action->data().value<void *>());
		Sequence *seq=nullptr;
		Table *tab=dynamic_cast<Table *>(col->getParentTable());

		op_list->startOperationChain();

		//Creates a sequence which name is like the ones auto generated by PostgreSQL
		seq=new Sequence;
		seq->setName(BaseObject::formatName(tab->getName() + "_" + col->getName() + "_seq"));
		seq->setName(CoreUtilsNs::generateUniqueName(seq, *db_model->getObjectList(ObjectType::Sequence), false));

		seq->setSchema(tab->getSchema());
		seq->setDefaultValues(col->getType());

		op_list->registerObject(seq, Operation::ObjCreated);
		db_model->addSequence(seq);

		BaseObject::swapObjectsIds(tab, seq, false);

		op_list->registerObject(col, Operation::ObjModified, -1, tab);
		//Changes the column type to the alias for serial type
		col->setType(col->getType().getAliasType());
		col->setSequence(seq);

		op_list->finishOperationChain();

		//Revalidate the relationships since the modified column can be a primary key
		if(tab->getPrimaryKey() && tab->getPrimaryKey()->isColumnReferenced(col))
			db_model->validateRelationships();

		tab->setModified(true);
		this->setModified(true);
		emit s_objectCreated();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::convertIntegerToSerial()
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		Column *col=reinterpret_cast<Column *>(action->data().value<void *>());
		Table *tab=dynamic_cast<Table *>(col->getParentTable());
		PgSqlType col_type=col->getType();
		QRegularExpression regexp("^nextval\\(.+\\:\\:regclass\\)");
		QString serial_tp;

		if(!col_type.isIntegerType() || (!col->getDefaultValue().contains(regexp) && !col->getSequence()))
			throw Exception(Exception::getErrorMessage(ErrorCode::InvConversionIntegerToSerial).arg(col->getName()),
											ErrorCode::InvConversionIntegerToSerial ,PGM_FUNC,PGM_FILE,PGM_LINE);

		op_list->registerObject(col, Operation::ObjModified, -1, tab);

		if(col_type=="integer" || col_type=="int4")
			serial_tp="serial";
		else if(col_type=="smallint" || col_type=="int2")
			serial_tp="smallserial";
		else
			serial_tp="bigserial";

		col->setType(PgSqlType(serial_tp));
		col->setDefaultValue("");

		//Revalidate the relationships since the modified column can be a primary key
		if(tab->getPrimaryKey()->isColumnReferenced(col))
			db_model->validateRelationships();

		tab->setModified(true);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::breakRelationshipLine()
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		BaseRelationship *rel=dynamic_cast<BaseRelationship *>(selected_objects[0]);

		op_list->registerObject(rel, Operation::ObjModified);
		breakRelationshipLine(rel, static_cast<RelBreakMode>(action->data().toInt()));
		rel->setModified(true);
		this->setModified(true);

		emit s_objectModified();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::breakRelationshipLine(BaseRelationship *rel, RelBreakMode break_type)
{
	if(!rel) return;

	try
	{
		RelationshipView *rel_view=dynamic_cast<RelationshipView *>(rel->getOverlyingObject());
		double dx, dy;
		QPointF src_pnt, dst_pnt;

		src_pnt=rel_view->getConnectionPoint(BaseRelationship::SrcTable);
		dst_pnt=rel_view->getConnectionPoint(BaseRelationship::DstTable);

		if(break_type==BreakVertNinetyDegrees)
			rel->setPoints({ QPointF(src_pnt.x(), dst_pnt.y()) });
		else if(break_type==BreakHorizNinetyDegrees)
			rel->setPoints({ QPointF(dst_pnt.x(), src_pnt.y()) });
		else if(break_type==BreakHoriz2NinetyDegrees)
		{
			//Calculates the midle vertical point between the tables centers
			dy=(src_pnt.y() + dst_pnt.y())/2;

			//Adds two points on the middle space between tables creating two 90° angles
			rel->setPoints({ QPointF(src_pnt.x(), dy), QPointF(dst_pnt.x(), dy) });
		}
		else
		{
			//Calculates the middle horizontal point between the tables centers
			dx=(src_pnt.x() + dst_pnt.x())/2;

			//Adds two points on the middle space between tables creating two 90° angles
			rel->setPoints({ QPointF(dx, src_pnt.y()), QPointF(dx, dst_pnt.y()) });
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelWidget::removeRelationshipPoints()
{
	try
	{
		QAction *action=dynamic_cast<QAction *>(sender());
		BaseRelationship *rel=reinterpret_cast<BaseRelationship *>(action->data().value<void *>());

		//Remove points from all selected relationships
		if(!rel && !selected_objects.empty())
		{
			std::vector<BaseObject *> rels;

			rels = *db_model->getObjectList(ObjectType::BaseRelationship);
			rels.insert(rels.end(), db_model->getObjectList(ObjectType::Relationship)->begin(),  db_model->getObjectList(ObjectType::Relationship)->end());

			op_list->startOperationChain();
			for(auto &obj : rels)
			{
				rel = dynamic_cast<BaseRelationship *>(obj);

				if(!rel->isProtected())
				{
					op_list->registerObject(rel, Operation::ObjModified);
					rel->setPoints({});
					rel->setModified(true);
				}
			}
			op_list->finishOperationChain();
		}
		else
		{
			op_list->registerObject(rel, Operation::ObjModified);
			rel->setPoints({});
			rel->setModified(true);
		}

		scene->clearSelection();
		this->setModified(true);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::rearrangeSchemasInGrid(unsigned tabs_per_row, unsigned sch_per_row, QPointF origin, double obj_spacing)
{
	std::vector<BaseObject *> *objects=nullptr;
	Schema *schema=nullptr;
	SchemaView *sch_view=nullptr;
	unsigned sch_id=0, min_cnt = 0;
	double x=origin.x(), y=origin.y(), max_y=-1, cy=0;

	objects=db_model->getObjectList(ObjectType::Schema);

	/* If schemas per row or tables per row isn't specified
	 * we calculate an minimal number of objects to be arranged per row */
	if(sch_per_row == 0)
	{
		min_cnt = objects->size() * 0.10;
		sch_per_row = min_cnt < 3 ? 3 : min_cnt;
	}

	if(tabs_per_row == 0)
	{
		min_cnt = (db_model->getObjectCount(ObjectType::Table) +
							 db_model->getObjectCount(ObjectType::View) +
							 db_model->getObjectCount(ObjectType::ForeignTable)) * 0.05;
		tabs_per_row = min_cnt < 5 ? 5 : min_cnt;
	}

	for(BaseObject *obj : *objects)
	{
		schema=dynamic_cast<Schema *>(obj);

		/* Forcing the schema rectangle to be visible in order to correctly positioin
			 schemas over the screen */
		schema->setRectVisible(true);

		sch_view=dynamic_cast<SchemaView *>(schema->getOverlyingObject());
		schema->setModified(true);

		//The schema is processed only there are tables inside of it
		if(sch_view && sch_view->getChildrenCount() > 0)
		{
			//Organizing the tables inside the schema
			rearrangeTablesInGrid(schema, tabs_per_row, QPointF(x,y), obj_spacing);
			schema->setModified(true);

			cy=sch_view->pos().y() + sch_view->boundingRect().height();

			//Defining the maximum y position to avoid schema boxes colliding vertically
			if(max_y < cy)
				max_y=cy;

			sch_id++;

			//It the current schema is the last of it`s row
			if(sch_id >= sch_per_row)
			{
				//Incrementing the row position
				sch_id=0;
				y=max_y + obj_spacing;
				x=origin.x();
				max_y=-1;
			}
			else
				//Configuring the x position for the next schema on the current row
				x=sch_view->pos().x() + sch_view->boundingRect().width() + obj_spacing;
		}
	}

	db_model->setObjectsModified({ ObjectType::Table, ObjectType::ForeignTable, ObjectType::View,
																 ObjectType::BaseRelationship, ObjectType::Relationship});

	//Adjust the whole scene size due to table/schema repositioning
	this->adjustSceneRect(false);
}

void ModelWidget::rearrangeTablesInGrid(Schema *schema, unsigned tabs_per_row,  QPointF origin, double obj_spacing)
{
	if(schema)
	{
		std::vector<BaseObject *> tables, views, ftables;
		std::vector<BaseObject *>::iterator itr;
		BaseTableView *tab_view=nullptr;
		BaseTable *base_tab=nullptr;
		unsigned tab_id=0;
		double max_y=-1, x=origin.x(), y=origin.y(), cy=0;

		//Get the tables and views for the specified schema
		tables=db_model->getObjects(ObjectType::Table, schema);
		ftables=db_model->getObjects(ObjectType::ForeignTable, schema);
		views=db_model->getObjects(ObjectType::View, schema);
		tables.insert(tables.end(), ftables.begin(), ftables.end());
		tables.insert(tables.end(), views.begin(), views.end());

		itr=tables.begin();
		while(itr!=tables.end())
		{
			base_tab=dynamic_cast<BaseTable *>(*itr);
			tab_view=dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
			tab_view->setPos(QPointF(x,y));

			//Defining the maximum y position to avoid table boxes colliding vertically
			cy=tab_view->pos().y()  + tab_view->boundingRect().bottomRight().y();
			if(max_y < cy)
				max_y=cy;

			//It the current table is the last of it's row
			tab_id++;
			if(tab_id >= tabs_per_row)
			{
				//Incrementing the row position
				tab_id=0;
				y=max_y + obj_spacing;
				x=origin.x();
				max_y=-1;
			}
			else
				//Configuring the x position for the next table on the current row
				x=tab_view->pos().x() + tab_view->boundingRect().width() + obj_spacing;

			itr++;
		}
	}
}

void ModelWidget::swapObjectsIds()
{
	BaseForm parent_form(this);
	SwapObjectsIdsWidget *swap_ids_wgt=new SwapObjectsIdsWidget;

	swap_ids_wgt->setModel(this->getDatabaseModel());

	if(!selected_objects.empty())
		swap_ids_wgt->setSelectedObjects(selected_objects[0], selected_objects.size() == 2 ? selected_objects[1] : nullptr);

	parent_form.setMainWidget(swap_ids_wgt, &SwapObjectsIdsWidget::swapObjectsIds);
	parent_form.setButtonConfiguration(Messagebox::OkCancelButtons);

	parent_form.apply_ok_btn->setEnabled(false);
	parent_form.apply_ok_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("swapobjs")));
	parent_form.apply_ok_btn->setText(tr("&Swap ids"));

	parent_form.cancel_btn->setIcon(QIcon(GuiUtilsNs::getIconPath("close1")));
	parent_form.cancel_btn->setText(tr("&Close"));

	connect(swap_ids_wgt, &SwapObjectsIdsWidget::s_objectsIdsSwapped, this, [this](){
		op_list->removeOperations();
		setModified(true);
		emit s_objectManipulated();
	});

	connect(swap_ids_wgt, &SwapObjectsIdsWidget::s_objectsIdsSwapReady, parent_form.apply_ok_btn, &QPushButton::setEnabled);

	GeneralConfigWidget::restoreWidgetGeometry(&parent_form, swap_ids_wgt->metaObject()->className());
	parent_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&parent_form, swap_ids_wgt->metaObject()->className());
}

void ModelWidget::jumpToTable()
{
	QAction *act = qobject_cast<QAction *>(sender());
	BaseTable *tab = nullptr;
	BaseTableView *tab_view = nullptr;

	if(!act)
		return;

	tab = reinterpret_cast<BaseTable *>(act->data().value<void *>());
	scene->clearSelection();
	tab_view = dynamic_cast<BaseTableView *>(tab->getOverlyingObject());
	tab_view->setSelected(true);
	viewport->centerOn(tab_view);
}

void ModelWidget::editTableData()
{
	TableDataWidget *tab_data_wgt=new TableDataWidget;

	tab_data_wgt->setAttributes(db_model, dynamic_cast<PhysicalTable *>(selected_objects.at(0)));
	openEditingForm(tab_data_wgt);
	this->setModified(true);
	emit s_objectManipulated();
}

void ModelWidget::updateModelLayersInfo()
{
	QStringList layers = scene->getLayers();

	db_model->setLayers(layers);
	db_model->setActiveLayers(scene->getActiveLayersIds());
	db_model->setLayerNameColors(scene->getLayerColorNames(ObjectsScene::LayerNameColor));
	db_model->setLayerRectColors(scene->getLayerColorNames(ObjectsScene::LayerRectColor));
	db_model->setLayerNamesVisible(scene->isLayerNamesVisible());
	db_model->setLayerRectsVisible(scene->isLayerRectsVisible());
	setModified(true);
}

void ModelWidget::rearrangeTablesHierarchically()
{
	std::vector<BaseObject *> objects;
	BaseGraphicObject *graph_obj = nullptr;
	BaseTableView *tab_view = nullptr, *root = nullptr;
	int num_rels = 0;

	scene->clearSelection();

	objects.assign(db_model->getObjectList(ObjectType::Table)->begin(), db_model->getObjectList(ObjectType::Table)->end());
	objects.insert(objects.end(), db_model->getObjectList(ObjectType::View)->begin(), db_model->getObjectList(ObjectType::View)->end());

	//We determine the root by searching the table/view which contains the more amount of relationships connected
	for(auto obj : objects)
	{
		graph_obj = dynamic_cast<BaseGraphicObject *>(obj);
		dynamic_cast<Schema *>(graph_obj->getSchema())->setRectVisible(false);

		tab_view = dynamic_cast<BaseTableView *>(graph_obj->getOverlyingObject());

		if(tab_view->getConnectRelsCount() > num_rels)
		{
			root = tab_view;
			num_rels = tab_view->getConnectRelsCount();
		}
	}

	if(root)
	{
		BaseObjectView *obj_view = nullptr;
		BaseRelationship *rel = nullptr;
		QRectF items_rect;
		std::vector<BaseObject *> evaluated_tabs, not_evaluated, not_linked_tabs;
		double px = 0, py = 0, max_h = 0, max_w = 0;

		//Positioning the root object at the top-left portion of canvas
		root->setPos(QPointF(50, 50));
		evaluated_tabs.push_back(root->getUnderlyingObject());
		items_rect = rearrangeTablesHierarchically(root, evaluated_tabs);
		max_w = items_rect.width();

		objects.clear();
		objects.assign(db_model->getObjectList(ObjectType::Table)->begin(), db_model->getObjectList(ObjectType::Table)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::View)->begin(), db_model->getObjectList(ObjectType::View)->end());

		//Retrieving the rest of tables/views that were not evaluated in the previous iteration
		std::sort(objects.begin(), objects.end());
		std::sort(evaluated_tabs.begin(), evaluated_tabs.end());
		std::set_difference(objects.begin(), objects.end(), evaluated_tabs.begin(), evaluated_tabs.end(),
												 std::inserter(not_evaluated, not_evaluated.begin()));

		/* While there is not evaluated objects (tables/views that are linked to each other but
		none of them are linked to the root (in)directly) we need to perform the same operation
		done for the root previously */
		while(!not_evaluated.empty())
		{
			num_rels = 0;
			root = nullptr;

			//Determining which table has the greater number of relationships attached
			for(auto &tab : not_evaluated)
			{
				tab_view = dynamic_cast<BaseTableView *>(dynamic_cast<BaseTable *>(tab)->getOverlyingObject());

				if(tab_view->getConnectRelsCount() > num_rels)
				{
					root = tab_view;
					num_rels = tab_view->getConnectRelsCount();
				}
			}

			//Once determined the new root we perform the positioning of its "children"
			if(root && std::find(evaluated_tabs.begin(), evaluated_tabs.end(), root->getUnderlyingObject()) == evaluated_tabs.end())
			{
				root->setPos(QPointF(50, items_rect.bottom() + 50));
				evaluated_tabs.push_back(root->getUnderlyingObject());
				items_rect = rearrangeTablesHierarchically(root, evaluated_tabs);
				not_evaluated.erase(std::find(not_evaluated.begin(), not_evaluated.end(),  root->getUnderlyingObject()));

				if(items_rect.width() > max_w)
					max_w = items_rect.width();
			}
			else
			{
				tab_view = dynamic_cast<BaseTableView *>(dynamic_cast<BaseTable *>(not_evaluated.front())->getOverlyingObject());

				//If the table/view has not relationships connected we separate it in a new list for further rearrangement
				if(tab_view->getConnectRelsCount() == 0)
					not_linked_tabs.push_back(not_evaluated.front());

				not_evaluated.erase(not_evaluated.begin());
			}
		}

		//Repositioning remaining tables (without relationships) and textboxes
		objects.clear();
		objects.assign(not_linked_tabs.begin(), not_linked_tabs.end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::Textbox)->begin(), db_model->getObjectList(ObjectType::Textbox)->end());

		px = 50;
		py = items_rect.bottom() + 100;
		max_h = 0;

		for(auto &obj : objects)
		{
			obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(obj)->getOverlyingObject());
			obj_view->setPos(px, py);
			px += obj_view->boundingRect().width() + 100;

			if(obj_view->boundingRect().height() > max_h)
				max_h = obj_view->boundingRect().height();

			if(px > max_w)
			{
				px = 50;
				py += max_h + 100;
			}
		}

		objects.clear();
		objects.assign(db_model->getObjectList(ObjectType::Relationship)->begin(), db_model->getObjectList(ObjectType::Relationship)->end());
		objects.insert(objects.end(), db_model->getObjectList(ObjectType::BaseRelationship)->begin(), db_model->getObjectList(ObjectType::BaseRelationship)->end());

		for(auto obj : objects)
		{
			rel = dynamic_cast<BaseRelationship *>(obj);
			rel->setPoints({});
			rel->resetLabelsDistance();

			try
			{
				if(!RelationshipView::isCurvedLines() &&
						round(rel->getTable(BaseRelationship::SrcTable)->getPosition().y()) !=
								round(rel->getTable(BaseRelationship::DstTable)->getPosition().y()))
				{
					breakRelationshipLine(dynamic_cast<BaseRelationship *>(obj), ModelWidget::BreakVert2NinetyDegrees);
				}
			}
			catch(Exception &e)
			{
				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}

		db_model->setObjectsModified({ ObjectType::Table, ObjectType::View, ObjectType::Schema, ObjectType::Relationship, ObjectType::BaseRelationship });
	}
	else
	{
		//This is a fallback arrangement when the model does not have relationships
		rearrangeSchemasInGrid();
	}

	adjustSceneRect(false);
	viewport->updateScene({ scene->sceneRect() });
}

QRectF ModelWidget::rearrangeTablesHierarchically(BaseTableView *root, std::vector<BaseObject *> &evaluated_tabs)
{
	BaseTable *base_tab = dynamic_cast<BaseTable *>(root->getUnderlyingObject()),
			*src_tab = nullptr, *dst_tab = nullptr, *curr_tab = nullptr;
	std::vector<BaseRelationship *> rels ;
	double px = 0, py = 0, px1 = 0, py1 = 0;
	BaseTableView *tab_view = nullptr;
	std::vector<BaseTable *> tabs = { base_tab }, next_tabs;
	bool is_protected = false;

	while(!tabs.empty())
	{
		base_tab = tabs.front();
		tabs.erase(tabs.begin());
		tab_view = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
		rels = db_model->getRelationships(base_tab);

		for(auto &rel : rels)
		{
			if(rel->isSelfRelationship())
				continue;

			src_tab = rel->getTable(BaseRelationship::SrcTable);
			dst_tab = rel->getTable(BaseRelationship::DstTable);

			if(src_tab != base_tab)
				curr_tab = src_tab;
			else if(dst_tab != base_tab)
				curr_tab = dst_tab;
			else
				curr_tab = nullptr;

			if(curr_tab && std::find(evaluated_tabs.begin(), evaluated_tabs.end(), dynamic_cast<BaseObject *>(curr_tab)) == evaluated_tabs.end())
			{
				next_tabs.push_back(curr_tab);
				evaluated_tabs.push_back(dynamic_cast<BaseObject *>(curr_tab));
			}
		}

		if(tabs.empty())
		{
			px = tab_view->pos().x() + (tab_view->boundingRect().width() * 1.50);
			py = root->pos().y() + 75;

			for(auto &next_tab : next_tabs)
			{
				tab_view = dynamic_cast<BaseTableView *>(next_tab->getOverlyingObject());

				//Temporarily unprotecting the table so it can be moved
				if(next_tab->isProtected())
				{
					next_tab->setProtected(false);
					is_protected = true;
				}
				else
					is_protected = false;

				tab_view->setPos(QPointF(px, py));
				next_tab->setProtected(is_protected);

				py += tab_view->boundingRect().height() + 75;
				px += 50;
			}

			if(px > px1) px1 = px;
			if(py > py1) py1 = py;

			tabs.assign(next_tabs.begin(), next_tabs.end());
			next_tabs.clear();
		}
	}

	return { root->pos(), QPointF(px1, py1) };
}

void ModelWidget::rearrangeTablesInSchema(Schema *schema, QPointF start)
{
	std::vector<BaseObject *> tables, views, ftables;

	if(!schema) return;

	tables = db_model->getObjects(ObjectType::Table, schema);
	ftables = db_model->getObjects(ObjectType::ForeignTable, schema);
	views = db_model->getObjects(ObjectType::View, schema);

	tables.insert(tables.end(), views.begin(), views.end());
	tables.insert(tables.end(), ftables.begin(), ftables.end());

	if(!tables.empty())
	{
		BaseTable *base_tab = nullptr;
		BaseTableView *tab_view = nullptr,  *comp_tab = nullptr, *curr_tab = nullptr;

		//If there two or less tables we put them side-by-side
		if(tables.size() <= 2)
		{
			base_tab = dynamic_cast<BaseTable *>(tables[0]);
			curr_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
			curr_tab->setPos(start);

			if(tables.size() > 1)
			{
				tab_view = curr_tab;
				base_tab = dynamic_cast<BaseTable *>(tables[1]);
				curr_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
				curr_tab->setPos(start + QPointF(tab_view->boundingRect().width() * 1.25, 0));
			}
		}
		else
		{
			double max_w = 0, max_h = 0;
			bool has_collision = false;
			QRectF curr_brect, comp_brect, irect;
			QPointF pos;
			std::random_device rand_seed;
			std::default_random_engine rand_num_engine;
			unsigned tries = 0;

			rand_num_engine.seed(rand_seed());

			/* Calculating the maximum width and height
			 * The new tables' positions are calculated using these dimensions */
			for(auto &tab : tables)
			{
				base_tab = dynamic_cast<BaseTable *>(tab);
				curr_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
				max_w += curr_tab->boundingRect().width() * 0.60;
				max_h += curr_tab->boundingRect().height() * 0.60;
			}

			if(tables.size() >= 4)
			{
				max_w *= 0.40;
				max_h *= 0.40;
			}
			else
			{
				max_w *= 1.05;
				max_h *= 1.05;
			}

			std::uniform_int_distribution<unsigned> dist_x(start.x(), start.x() + max_w),
					dist_y(start.y(), start.y() + max_h);

			//Doing the first random positioning on all tables
			for(auto &tab : tables)
			{
				base_tab = dynamic_cast<BaseTable *>(tab);
				curr_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
				pos.setX(dist_x(rand_num_engine));
				pos.setY(dist_y(rand_num_engine));
				curr_tab->setPos(pos);
			}

			/* Collision detection: If a table collides with other tables it'll then repositioned
			 * until no interception is detected */
			for(auto &tab : tables)
			{
				base_tab = dynamic_cast<BaseTable *>(tab);
				curr_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
				curr_brect = QRectF(curr_tab->pos(), curr_tab->boundingRect().size());
				tries = 0;

				do
				{
					has_collision = false;

					for(auto &tab1 : tables)
					{
						if(tab == tab1)
							continue;

						base_tab = dynamic_cast<BaseTable *>(tab1);
						comp_tab = dynamic_cast<BaseTableView *>(base_tab->getOverlyingObject());
						comp_brect = QRectF(comp_tab->pos(), comp_tab->boundingRect().size());
						irect = comp_brect.intersected(curr_brect);

						if(irect.isValid())
						{
							has_collision = true;
							pos.setX(dist_x(rand_num_engine));
							pos.setY(dist_y(rand_num_engine));
							curr_tab->setPos(pos);
							curr_brect = QRectF(curr_tab->pos(), curr_tab->boundingRect().size());
							break;
						}
					}

					tries++;
				}
				while(has_collision && tries < (tables.size() * 50));
			}
		}

		schema->setRectVisible(true);
		schema->setModified(true);
	}
}

void ModelWidget::rearrangeTablesInSchemas()
{
	BaseRelationship *base_rel = nullptr;
	Schema *schema = nullptr;
	SchemaView *sch_view = nullptr, *sch_view_aux = nullptr;
	QRectF curr_brect, comp_brect, irect;
	std::random_device rand_seed;
	std::default_random_engine rand_num_engine;
	double max_w = 1000, max_h = 1000;
	std::vector<BaseObject *> schemas = *db_model->getObjectList(ObjectType::Schema), rels;
	bool has_collision = false;
	std::uniform_int_distribution<unsigned> dist_x(0, max_w), dist_y(0, max_h);
	unsigned tries = 0,
			max_tries = (db_model->getObjectCount(ObjectType::Table) +
									 db_model->getObjectCount(ObjectType::View) +
									 db_model->getObjectCount(ObjectType::ForeignTable) +
									 db_model->getObjectCount(ObjectType::Schema)) * 50;


	rand_num_engine.seed(rand_seed());

	/* Rearraging tables inside schemas and determining the maximum width and height by summing
	 * all schemas widths and heights. These values will be serve as the maximum
	 * position limit for the schemas */
	for(auto &sch : schemas)
	{
		schema = dynamic_cast<Schema *>(sch);
		sch_view = dynamic_cast<SchemaView *>(schema->getOverlyingObject());

		if(!sch_view)	continue;

		rearrangeTablesInSchema(schema, QPointF(dist_x(rand_num_engine), dist_y(rand_num_engine)));

		max_w += sch_view->boundingRect().width();
		max_h += sch_view->boundingRect().height();
	}

	std::uniform_int_distribution<unsigned>::param_type new_dx(0, max_w * 0.40);
	dist_x.param(new_dx);

	std::uniform_int_distribution<unsigned>::param_type new_dy(0, max_h * 0.40);
	dist_y.param(new_dy);

	/* Collision detection: If a schema collides with other schemas it'll then repositioned
	 * until no interception is detected or the tries reached the max_tries value */
	for(auto &sch : schemas)
	{
		schema = dynamic_cast<Schema *>(sch);
		sch_view = dynamic_cast<SchemaView *>(schema->getOverlyingObject());
		tries = 0;

		if(!sch_view)	continue;

		curr_brect = QRectF(sch_view->pos(), sch_view->boundingRect().size());

		do
		{
			has_collision = false;

			for(auto &sch1 : schemas)
			{
				schema = dynamic_cast<Schema *>(sch1);
				sch_view_aux = dynamic_cast<SchemaView *>(schema->getOverlyingObject());

				if(sch == sch1 || !sch_view_aux)
					continue;

				comp_brect = QRectF(sch_view_aux->pos(), sch_view_aux->boundingRect().size());
				irect = comp_brect.intersected(curr_brect);

				if(irect.isValid())
				{
					has_collision = true;
					sch_view->moveTo(QPointF(dist_x(rand_num_engine), dist_y(rand_num_engine)));
					curr_brect = QRectF(sch_view->pos(), sch_view->boundingRect().size());
					break;
				}
			}

			tries++;
		}
		while(has_collision && tries < max_tries);
	}

	//Removing all custom points from relationships
	rels.assign(db_model->getObjectList(ObjectType::Relationship)->begin(), db_model->getObjectList(ObjectType::Relationship)->end());
	rels.insert(rels.end(), db_model->getObjectList(ObjectType::BaseRelationship)->begin(), db_model->getObjectList(ObjectType::BaseRelationship)->end());

	for(auto &rel : rels)
	{
		base_rel = dynamic_cast<BaseRelationship *>(rel);
		base_rel->setPoints({});
		base_rel->resetLabelsDistance();
	}

	db_model->setObjectsModified({ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable,
																 ObjectType::Schema, ObjectType::Relationship, ObjectType::BaseRelationship });
	adjustSceneRect(false);
	viewport->updateScene({ scene->sceneRect() });
}

void ModelWidget::updateMagnifierArea()
{
	QPoint pos = viewport->mapFromGlobal(QCursor::pos());
	QPointF scene_pos = viewport->mapToScene(pos);
	QSize size = magnifier_area_lbl->size() * qApp->devicePixelRatio();
	QPixmap pix = QPixmap(size);
	double cx = magnifier_area_lbl->width() / 2, cy =  magnifier_area_lbl->height() / 2;

	pix.setDevicePixelRatio(qApp->devicePixelRatio());
	pix.fill(ObjectsScene::getCanvasColor());

	magnifier_rect.setRect(0, 0,
												 magnifier_area_lbl->width() * current_zoom,
												 magnifier_area_lbl->height() * current_zoom);

	magnifier_rect.translate(pos - QPoint(magnifier_rect.width()/2, magnifier_rect.height()/2));

	if(magnifier_rect.left() <= magnifier_area_lbl->geometry().right())
		magnifier_area_lbl->move(viewport->width() - magnifier_area_lbl->width(), magnifier_area_lbl->geometry().top());

	if(magnifier_rect.right() >= magnifier_area_lbl->geometry().left())
		magnifier_area_lbl->move(5, magnifier_area_lbl->geometry().top());

	QPainter p(&pix);
	p.setRenderHints(viewport->renderHints());

	scene->blockSignals(true);
	scene->render(&p, QRectF(QPointF(0,0), size), QRectF(scene_pos - QPointF(cx, cy), size));

	p.setPen(QColor(80,0,0));
	p.drawLine(QPointF(cx, cy - 10), QPointF(cx, cy + 10));
	p.drawLine(QPointF(cx - 10, cy), QPointF(cx + 10, cy));

	magnifier_area_lbl->setPixmap(pix);
	scene->blockSignals(false);
}

void ModelWidget::showMagnifierArea(bool show)
{
	show = show && (current_zoom < 1);

	if(show)
	{
		updateMagnifierArea();
		viewport->setCursor(Qt::CrossCursor);
	}
	else
		viewport->setCursor(Qt::ArrowCursor);

	magnifier_area_lbl->setVisible(show);

	emit s_maginifierAreaVisible(show);
}
