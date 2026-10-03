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

#include "baseform.h"
#include "modelwidget.h"
#include "customuistyle.h"
#include "codepreviewwidget.h"
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
#include "objectrenamewidget.h"
#include "dbobjects/permissionwidget.h"
#include "dbobjects/collationwidget.h"
#include "dbobjects/extensionwidget.h"
#include "customsqlwidget.h"
#include "dbobjects/tagwidget.h"
#include "dbobjects/eventtriggerwidget.h"
#include "guiutilsns.h"
#include "swapobjectsidswidget.h"
#include "crossscenerelswidget.h"
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
#include "cutoffitem.h"
#include "graphicalview.h"
#include "relationshipview.h"
#include "schemaview.h"
#include "styledtextboxview.h"
#include "tableview.h"
#include "pgmodelerguiplugin.h"
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
	QFont font;
	QLabel *label=nullptr;
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
	scene = nullptr;
	active_scene_idx = -1;
	threshold_obj_count = 0;
	modified = panning_mode = wheel_move = false;
	scene_moving = blink_new_objs = false;
	curr_show_grid = curr_show_delim = true;
	new_obj_type = ObjectType::BaseObject;

	//Generating a temporary file name for the model
	tmp_filename = UtilsNs::getTemporaryFilePath(
									 GlobalAttributes::getTemporaryFilePath("model_XXXXXX" + GlobalAttributes::DbModelExt));

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
	label->setPixmap(GuiUtilsNs::getPixmap("alert"));
	label->setObjectName("icon_lbl");

	QHBoxLayout *hbox = GuiUtilsNs::createHBoxLayout(GuiUtilsNs::LtMargin,
																									 GuiUtilsNs::LtSpacing,
																									 protected_model_frm);
	hbox->addWidget(label);
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

	hbox->addWidget(label);
	protected_model_frm->adjustSize();

	db_model = new DatabaseModel(this);
	xmlparser = db_model->getXMLParser();
	op_list = new OperationList(db_model, this);

	viewport = new QGraphicsView(this);

	/* Since we constantly work with objects (re)configuration which leads to bounding
	 * rectangle changes, the viewport area update is done by using bounding rects
	 * as reference in order to perform updates on the areas covered by the modified
	 * objects bounding rects. */
	viewport->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	viewport->setCacheMode(QGraphicsView::CacheBackground);

	updateRenderHints();

	//Force the scene to be drawn from the left to right and from top to bottom
	viewport->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	viewport->centerOn(0,0);
	viewport->setMouseTracking(true);
	viewport->installEventFilter(this);
	viewport->setAcceptDrops(true);
	viewport->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout *vbox = GuiUtilsNs::createVBoxLayout(20, GuiUtilsNs::LtSpacing, this);
	vbox->addWidget(protected_model_frm);
	vbox->addWidget(viewport);

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

	action_edit_data=new QAction(GuiUtilsNs::getIcon("editrows"), tr("Edit data"), this);

	action_source_code=new QAction(GuiUtilsNs::getIcon("sourcecode"), tr("Source"), this);
	action_source_code->setShortcut(QKeySequence(tr("Alt+S")));
	action_source_code->setToolTip(tr("Show object source code"));
	addAction(action_source_code);

	action_edit=new QAction(GuiUtilsNs::getIcon("edit"), tr("Properties"), this);
	action_edit->setShortcut(QKeySequence(tr("Space")));
	action_edit->setToolTip(tr("Edit the object properties"));
	addAction(action_edit);

	action_protect=new QAction(GuiUtilsNs::getIcon("protect"), tr("Protect"), this);
	action_unprotect=new QAction(GuiUtilsNs::getIcon("unprotect"), tr("Unprotect"), this);
	action_protect->setToolTip(tr("Protects object(s) from modifications"));

	action_remove=new QAction(GuiUtilsNs::getIcon("delete"), tr("Delete"), this);
	action_remove->setShortcut(QKeySequence(tr("Del")));
	action_remove->setMenuRole(QAction::NoRole);

	action_cascade_del=new QAction(GuiUtilsNs::getIcon("delcascade"), tr("Del. cascade"), this);
	action_cascade_del->setShortcut(QKeySequence(tr("Shift+Del")));
	action_cascade_del->setMenuRole(QAction::NoRole);

	action_select_all = select_all_menu.menuAction();
	action_select_all->setIcon(GuiUtilsNs::getIcon("selectmove"));
	action_select_all->setText(tr("Select all"));
	action_select_all->setToolTip(tr("Selects all the graphical objects in the model"));

	action_convert_relnn=new QAction(GuiUtilsNs::getIcon("convrelnn"), tr("Convert"), this);
	action_convert_rel1n=new QAction(GuiUtilsNs::getIcon("convrel1n"), tr("Convert"), this);

	action_copy = copy_menu.menuAction();
	action_copy->setText(tr("Copy"));
	action_copy->setMenuRole(QAction::NoRole);
	action_copy->setIcon(GuiUtilsNs::getIcon("copy"));

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

	action_paste=new QAction(GuiUtilsNs::getIcon("paste"), tr("Paste"), this);
	action_paste->setShortcut(QKeySequence(tr("Ctrl+V")));
	action_paste->setMenuRole(QAction::NoRole);

	action_cut = cut_menu.menuAction();
	action_cut->setText(tr("Cut"));
	action_cut->setMenuRole(QAction::NoRole);
	action_cut->setIcon(GuiUtilsNs::getIcon("cut"));

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

	action_associations=new QAction(GuiUtilsNs::getIcon("associations"), tr("Associations"), this);

	action_new_object = new_object_menu.menuAction();
	action_new_object->setIcon(GuiUtilsNs::getIcon("newobject"));
	action_new_object->setText(tr("New"));
	action_new_object->setToolTip(tr("Add a new object in the model"));
	addAction(action_new_object);

	action_quick_actions = quick_actions_menu.menuAction();
	action_quick_actions->setIcon(GuiUtilsNs::getIcon("quickactions"));
	action_quick_actions->setText(tr("Quick"));
	action_quick_actions->setToolTip(tr("Quick action for the selected object"));
	addAction(action_quick_actions);

	action_rename=new QAction(GuiUtilsNs::getIcon("rename"), tr("Rename"), this);
	action_rename->setShortcut(QKeySequence(tr("F2")));
	action_rename->setToolTip(tr("Quick rename the object"));
	addAction(action_rename);

	action_moveto_schema = schemas_menu.menuAction();
	action_moveto_schema->setIcon(GuiUtilsNs::getIcon("movetoschema"));
	action_moveto_schema->setText(tr("Move to schema"));
	addAction(action_moveto_schema);

	action_moveto_scene = scenes_menu.menuAction();
	action_moveto_scene->setIcon(GuiUtilsNs::getIcon("movetoscene"));
	action_moveto_scene->setText(tr("Move to scene"));
	addAction(action_moveto_scene);

	action_set_layer = new QAction(GuiUtilsNs::getIcon("layers"), tr("Set layers"), this);
	action_set_layer->setShortcut(QKeySequence("F3"));
	addAction(action_set_layer);
	layers_wgt = new LayersWidget(this);

	action_set_tag = tags_menu.menuAction();
	action_set_tag->setIcon(GuiUtilsNs::getIcon("tag"));
	action_set_tag->setText(tr("Set tag"));
	addAction(action_set_tag);

	action_edit_perms=new QAction(GuiUtilsNs::getIcon("permission"), tr("Edit permissions"), this);
	action_edit_perms->setShortcut(QKeySequence(tr("Ctrl+E")));
	addAction(action_edit_perms);

	action_change_owner = owners_menu.menuAction();
	action_change_owner->setIcon(GuiUtilsNs::getIcon("changeowner"));
	action_change_owner->setText(tr("Change owner"));
	addAction(action_change_owner);

	action_sel_sch_children=new QAction(GuiUtilsNs::getIcon("selectmove"), tr("Select children"), this);
	action_sel_tagged_tabs=new QAction(GuiUtilsNs::getIcon("selectmove"), tr("Select tagged"), this);
	action_sel_table_rels=new QAction(GuiUtilsNs::getIcon("selectmove"), tr("Select relationships"), this);
	addAction(action_sel_sch_children);
	addAction(action_sel_tagged_tabs);
	addAction(action_sel_table_rels);

	action_create_tag_from_schema=new QAction(GuiUtilsNs::getIcon("tag"), tr("Add tag from schema"), this);
	action_create_layer_from_schema=new QAction(GuiUtilsNs::getIcon("layers"), tr("Add layer from schema"), this);
	action_create_tags_all_schemas=new QAction(GuiUtilsNs::getIcon("tag"), tr("Add tags from schemas"), this);
	action_create_layers_all_schemas=new QAction(GuiUtilsNs::getIcon("layers"), tr("Add layers from schemas"), this);
	addAction(action_create_tag_from_schema);
	addAction(action_create_layer_from_schema);
	addAction(action_create_tags_all_schemas);
	addAction(action_create_layers_all_schemas);

	action_select_object=new QAction(GuiUtilsNs::getIcon("moved"), tr("Select"), this);
	action_parent_rel=new QAction(GuiUtilsNs::getIcon("relationship"), tr("Open relationship"), this);
	addAction(action_select_object);
	addAction(action_parent_rel);

	action_append_sql=new QAction(GuiUtilsNs::getIcon("sqlappend"), tr("Custom SQL"), this);
	action_append_sql->setShortcut(QKeySequence(tr("Alt+Q")));
	addAction(action_append_sql);

	action_create_seq_col=new QAction(GuiUtilsNs::getIcon("sequence"), tr("Convert to sequence"), this);
	action_conv_int_serial=new QAction(GuiUtilsNs::getIcon("sequence"), tr("Convert to serial"), this);
	addAction(action_create_seq_col);
	addAction(action_conv_int_serial);

	action_remove_rel_points=new QAction(GuiUtilsNs::getIcon("removepoints"), tr("Remove points"), this);
	addAction(action_remove_rel_points);

	action_enable_sql=new QAction(GuiUtilsNs::getIcon("enablesql"), tr("Enable SQL"), this);
	action_disable_sql=new QAction(GuiUtilsNs::getIcon("disablesql"), tr("Disable SQL"), this);
	addAction(action_enable_sql);
	addAction(action_disable_sql);

	action_duplicate=new QAction(GuiUtilsNs::getIcon("duplicate"), tr("Duplicate"), this);
	action_duplicate->setShortcut(QKeySequence(tr("Ctrl+D")));
	action_duplicate->setMenuRole(QAction::NoRole);
	addAction(action_duplicate);

	action_pagination = pagination_menu.menuAction();
	action_pagination->setIcon(GuiUtilsNs::getIcon("pagination"));
	action_pagination->setText(tr("Pagination"));
	addAction(action_pagination);

	action = pagination_menu.addAction(tr("Enable"));
	action->setData(true);
	connect(action, &QAction::triggered, this, &ModelWidget::togglePagination);

	action = pagination_menu.addAction(tr("Disable"));
	action->setData(false);
	connect(action, &QAction::triggered, this, &ModelWidget::togglePagination);

	action_jump_to_table = jump_to_tab_menu.menuAction();
	action_jump_to_table->setIcon(GuiUtilsNs::getIcon("jumptotable"));
	action_jump_to_table->setText(tr("Jump to table"));
	addAction(action_jump_to_table);

	action_collapse_mode = toggle_attrs_menu.menuAction();
	action_collapse_mode->setIcon(GuiUtilsNs::getIcon("collapse"));
	action_collapse_mode->setText(tr("Collapse"));
	addAction(action_collapse_mode);

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
	action_schemas_rects->setIcon(GuiUtilsNs::getIcon("schemarect"));
	action_schemas_rects->setText(tr("Schemas rectangles"));
	action_show_schemas_rects=new QAction(tr("Show"), this);
	action_hide_schemas_rects=new QAction(tr("Hide"), this);
	toggle_sch_rects_menu.addAction(action_show_schemas_rects);
	toggle_sch_rects_menu.addAction(action_hide_schemas_rects);

	action_fade = fade_menu.menuAction();
	action_fade->setIcon(GuiUtilsNs::getIcon("fade"));
	action_fade->setText(tr("Fade in/out"));

	action_fade_in = new QAction(GuiUtilsNs::getIcon("fadein"), tr("Fade in"), this);
	action_fade_out = new QAction(GuiUtilsNs::getIcon("fadeout"), tr("Fade out"), this);
	addAction(action_fade_in);
	addAction(action_fade_out);

	action_fade_rels = fade_rels_menu.menuAction();
	action_fade_rels->setIcon(GuiUtilsNs::getIcon("relationship"));
	action_fade_rels->setText(tr("Relationships"));
	action_fade_rels_in=new QAction(GuiUtilsNs::getIcon("fadein"), tr("Fade in"), this);
	action_fade_rels_out=new QAction(GuiUtilsNs::getIcon("fadeout"), tr("Fade out"), this);
	addAction(action_fade_rels_in);
	addAction(action_fade_rels_out);

	action_fade_peer_tables = fade_peer_tables_menu.menuAction();
	action_fade_peer_tables->setIcon(GuiUtilsNs::getIcon("table"));
	action_fade_peer_tables->setText(tr("Peer tables"));
	action_fade_peer_tables_in = new QAction(GuiUtilsNs::getIcon("fadein"), tr("Fade in"), this);
	action_fade_peer_tables_out = new QAction(GuiUtilsNs::getIcon("fadeout"), tr("Fade out"), this);
	addAction(action_fade_peer_tables_in);
	addAction(action_fade_peer_tables_out);

	action_fade_tabs_rels = fade_both_objs_menu.menuAction();
	action_fade_tabs_rels->setText(tr("Tables && Relationships"));
	action_fade_tabs_rels_in = new QAction(GuiUtilsNs::getIcon("fadein"), tr("Fade in"), this);
	action_fade_tabs_rels_out = new QAction(GuiUtilsNs::getIcon("fadeout"), tr("Fade out"), this);
	addAction(action_fade_tabs_rels_in);
	addAction(action_fade_tabs_rels_out);

	fade_rels_menu.addAction(action_fade_rels_in);
	fade_rels_menu.addAction(action_fade_rels_out);

	fade_peer_tables_menu.addAction(action_fade_peer_tables_in);
	fade_peer_tables_menu.addAction(action_fade_peer_tables_out);

	fade_both_objs_menu.addAction(action_fade_tabs_rels_in);
	fade_both_objs_menu.addAction(action_fade_tabs_rels_out);

	action_edit_creation_order=new QAction(GuiUtilsNs::getIcon("swapobjs"), tr("Swap ids"), this);
	action_edit_creation_order->setToolTip(tr("Edit the objects creation order by swapping their ids"));
	addAction(action_edit_creation_order);
	connect(action_edit_creation_order, &QAction::triggered, this, &ModelWidget::swapObjectsIds);

	action_break_rel_line = break_rel_menu.menuAction();
	action_break_rel_line->setIcon(GuiUtilsNs::getIcon("breakrelline"));
	action_break_rel_line->setText(tr("Break line"));

	action=new QAction(GuiUtilsNs::getIcon("breakline_90dv"), tr("90° (vertical)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakVertNinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(GuiUtilsNs::getIcon("breakline_90dh"), tr("90° (horizontal)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakHorizNinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(GuiUtilsNs::getIcon("breakline_290dv"), tr("90° + 90° (vertical)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakVert2NinetyDegrees);
	break_rel_menu.addAction(action);

	action=new QAction(GuiUtilsNs::getIcon("breakline_290dh"), tr("90° + 90° (horizontal)"), this);
	connect(action, &QAction::triggered, this, qOverload<>(&ModelWidget::breakRelationshipLine));
	action->setData(BreakHoriz2NinetyDegrees);
	break_rel_menu.addAction(action);

	//Alocatting the object creation actions
	for(auto &type : types_vect)
	{
		actions_new_objects[type]=new QAction(GuiUtilsNs::getIcon(type), BaseObject::getTypeName(type), this);
		addAction(actions_new_objects[type]);
		actions_new_objects[type]->setData(QVariant(enum_t(type)));
		connect(actions_new_objects[type], &QAction::triggered, this, &ModelWidget::addNewObject);
	}

	// Configuring the submenu of database level objects
	action_database_category = database_category_menu.menuAction();
	action_database_category->setIcon(GuiUtilsNs::getIcon(ObjectType::Database));
	action_database_category->setText(tr("Database object"));
	types_vect = BaseObject::getChildObjectTypes(ObjectType::Database);

	for(auto &type : types_vect)
		database_category_menu.addAction(actions_new_objects[type]);

	// Configuring the submenu of schema level objects
	action_schema_category = schema_category_menu.menuAction();
	action_schema_category->setIcon(GuiUtilsNs::getIcon(ObjectType::Schema));
	action_schema_category->setText(tr("Schema object"));
	types_vect = BaseObject::getChildObjectTypes(ObjectType::Schema);

	for(auto &type : types_vect)
		schema_category_menu.addAction(actions_new_objects[type]);

	//Creating the relationship submenu
	rels_menu=new QMenu(this);
	actions_new_objects[ObjectType::Relationship] = rels_menu->menuAction();
	actions_new_objects[ObjectType::Relationship]->setIcon(GuiUtilsNs::getIcon(ObjectType::Relationship));
	actions_new_objects[ObjectType::Relationship]->setText(BaseObject::getTypeName(ObjectType::Relationship));
	actions_new_objects[ObjectType::Relationship]->setData(QVariant(enum_t(ObjectType::Relationship)));

	for(int i=0; i < rel_types_cod.size(); i++)
	{
		str_ico=BaseObject::getSchemaName(ObjectType::Relationship) + rel_types_cod[i];

		action=new QAction(GuiUtilsNs::getIcon(str_ico),
							 BaseRelationship::getRelationshipTypeName(rel_types_id[i], false), this);

		//Storing a unique identifier for the relationship type
		action->setData(QVariant(enum_t(ObjectType::Relationship) + rel_types_id[i]));

		connect(action, &QAction::triggered, this, &ModelWidget::addNewObject);
		rels_menu->addAction(action);
		addAction(action);
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
			action=new QAction(GuiUtilsNs::getIcon(BaseObject::getSchemaName(obj_type)), labels[i++], this);
			select_all_menu.addAction(action);
		}

		action->setData(QVariant(enum_t(obj_type)));
		addAction(action);
		connect(action, &QAction::triggered, this, &ModelWidget::selectAllObjects);
	}

	action_stacking = stacking_menu.menuAction();
	action_stacking->setIcon(GuiUtilsNs::getIcon("stacking"));
	action_stacking->setText(tr("Stacking"));

	action_send_to_back=new QAction(GuiUtilsNs::getIcon("sendtoback"), tr("Send to back"), this);
	action_send_to_back->setShortcut(QKeySequence("Shift+Down"));
	addAction(action_send_to_back);

	action_bring_to_front=new QAction(GuiUtilsNs::getIcon("bringtofront"), tr("Bring to front"), this);
	action_bring_to_front->setShortcut(QKeySequence("Shift+Up"));
	addAction(action_bring_to_front);

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
		act = fade_in_all_menu.addAction(GuiUtilsNs::getIcon(BaseObject::getSchemaName(act_type)),	act_labels[id]);
		act->setData(enum_t(act_type));
		addAction(act);
		connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);

		act = fade_out_all_menu.addAction(GuiUtilsNs::getIcon(BaseObject::getSchemaName(act_type)),	act_labels[id]);
		act->setData(enum_t(act_type));
		addAction(act);
		connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);

		id++;
	}

	act = fade_in_all_menu.addAction(tr("All objects"));
	act->setData(enum_t(ObjectType::BaseObject));
	addAction(act);
	fade_in_all_menu.insertSeparator(act);
	connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsIn);
	fade_in_all_menu.menuAction()->setText("Fade in");
	fade_in_all_menu.menuAction()->setIcon(GuiUtilsNs::getIcon("fadein"));

	act = fade_out_all_menu.addAction(tr("All objects"));
	act->setData(enum_t(ObjectType::BaseObject));
	addAction(act);
	fade_out_all_menu.insertSeparator(act);
	connect(act, &QAction::triggered, this, &ModelWidget::fadeObjectsOut);
	fade_out_all_menu.menuAction()->setText("Fade out");
	fade_out_all_menu.menuAction()->setIcon(GuiUtilsNs::getIcon("fadeout"));

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
	connect(action_associations, &QAction::triggered, this, &ModelWidget::showObjectAssociations);

	connect(action_paste, &QAction::triggered, this, __slot(this, ModelWidget::pasteObjects));
	connect(action_duplicate, &QAction::triggered, this, &ModelWidget::duplicateObject);

	connect(action_rename, &QAction::triggered, this, &ModelWidget::renameObjects);
	connect(action_edit_perms, &QAction::triggered, this, &ModelWidget::editPermissions);
	connect(action_sel_sch_children, &QAction::triggered, this, &ModelWidget::selectSchemaChildren);
	connect(action_sel_table_rels, &QAction::triggered, this, &ModelWidget::selectTableRelationships);
	connect(action_sel_tagged_tabs, &QAction::triggered, this, &ModelWidget::selectTaggedTables);
	connect(action_create_tag_from_schema, &QAction::triggered, this, &ModelWidget::createTagFromSchema);
	connect(action_create_layer_from_schema, &QAction::triggered, this, &ModelWidget::createLayerFromSchema);
	connect(action_create_tags_all_schemas, &QAction::triggered, this, &ModelWidget::createTagsForAllSchemas);
	connect(action_create_layers_all_schemas, &QAction::triggered, this, &ModelWidget::createLayersForAllSchemas);
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
	connect(op_list, &OperationList::s_operationExecuted, this, &ModelWidget::handleObjectSchemaChanged);

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

	/* Nothing may react to the teardown, and the order matters: the views point to the model's
	 * objects, the operation list still queries the model while being destroyed, and only then
	 * the model itself (and every object in it) can go */
	for(auto &scn : scenes)
		scn->blockSignals(true);

	op_list->blockSignals(true);
	db_model->blockSignals(true);

	while(!scenes.isEmpty())
		delete scenes.takeLast();

	scene = nullptr;

	delete op_list;
	delete db_model;

	popup_menu.clear();
	new_object_menu.clear();
	quick_actions_menu.clear();
	schemas_menu.clear();
	scenes_menu.clear();
	owners_menu.clear();
	tags_menu.clear();
	break_rel_menu.clear();
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
	ObjectsScene *evt_scene = qobject_cast<ObjectsScene *>(object);
	bool is_scene_event = evt_scene && scenes.contains(evt_scene);

	/* Emitting a signal when the scene successfully made a drag & drop event
	 * This signal is handled in main window so the mime data containing the url
	 * to dbm files can be correctly loaded */
	if(is_scene_event && (event->type() == QEvent::GraphicsSceneDragEnter ||
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
	else if(is_scene_event)
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
				{
					//Selection now spans every scene -- unite each one's own selected-items rect to reflect the whole thing
					QRectF sel_rect;

					for(auto &scn : scenes)
						sel_rect = sel_rect.united(scn->itemsBoundingRect(true, true));

					emit s_sceneInteracted(static_cast<int>(selected_objects.size()), sel_rect);
				}
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

		//Save the zoom, position or active scene only if one of these attributes has changed
		if(db_model->getLastZoomFactor()!=current_zoom ||
				pos.x()!=hscroll->value() || pos.y()!=vscroll->value() ||
				db_model->getLastScene()!=active_scene_idx)
		{
			db_model->setLastPosition(QPoint(viewport->horizontalScrollBar()->value(),
																			 viewport->verticalScrollBar()->value()));
			db_model->setLastZoomFactor(this->current_zoom);
			db_model->setLastScene(active_scene_idx);
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
		int last_scene = db_model->getLastScene();

		/* An out-of-bounds index (e.g. the file was hand-edited, or the scene it pointed to was
		 * removed after the save that wrote it) falls back to the first available scene instead of
		 * just leaving whichever scene load happened to leave active */
		setActiveScene((last_scene >= 0 && last_scene < scenes.size()) ? last_scene : 0);

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

ObjectsScene *ModelWidget::getSchemaScene(Schema *schema)
{
	return schema_scenes.value(schema, nullptr);
}

QStringList ModelWidget::getSceneNames()
{
	QStringList names;
	DatabaseModel::SegmentationMode seg_mode = db_model->getSegmentationMode();

	/* schema_scenes can hold a pg_catalog entry even outside of schema-segmentation -- it's always
	 * the first schema created (see createSystemObjects()), before any scene exists and before the
	 * file's own seg-mode is known, so it falls through handleObjectAddition()'s safety net and gets
	 * registered there regardless of what the mode ends up being. Only trust that map here in the
	 * mode where it's actually meaningful; every other mode uses its own generic naming */
	for(int i = 0; i < scenes.size(); i++)
	{
		if(seg_mode == DatabaseModel::SegBySchema)
		{
			Schema *schema = schema_scenes.key(scenes.at(i), nullptr);
			names.append(schema ? schema->getName() : tr("Scene %1").arg(i + 1));
		}
		else if(seg_mode == DatabaseModel::NoSegmentation)
			names.append(tr("Default scene"));
		else
			names.append(tr("Scene %1").arg(i + 1));
	}

	return names;
}

int ModelWidget::getActiveSceneIndex()
{
	return active_scene_idx;
}

ObjectsScene *ModelWidget::addScene(Schema *schema)
{
	if(!schema || schema_scenes.contains(schema))
		return nullptr;

	/* pg_catalog is a system schema rarely touched directly by the user (usually
	 * populated via reverse engineering) -- never let it claim the default active scene,
	 * even when it's the first schema processed (createSystemObjects() always creates it
	 * before the file's own schemas are parsed on load) */
	ObjectsScene *new_scene = addScene(schema->getName() != "pg_catalog");
	schema_scenes[schema] = new_scene;

	return new_scene;
}

ObjectsScene *ModelWidget::addScene(bool auto_activate)
{
	ObjectsScene *new_scene = new ObjectsScene(this);
	new_scene->installEventFilter(this);

	connect(new_scene, &ObjectsScene::s_objectsMoved, this, [this, new_scene](bool end_moviment){
		handleObjectsMovement(new_scene, end_moviment);
	});
	connect(new_scene, &ObjectsScene::s_objectModified, this,  &ModelWidget::handleObjectModification);
	connect(new_scene, &ObjectsScene::s_objectDoubleClicked, this,  &ModelWidget::handleObjectDoubleClick);
	connect(new_scene, &ObjectsScene::s_cutoffDoubleClicked, this,  &ModelWidget::showCutoffRelationships);

	connect(new_scene, &ObjectsScene::s_objectSelected, this,  &ModelWidget::configureObjectSelection, Qt::QueuedConnection);
	connect(new_scene, qOverload<BaseObject *>(&ObjectsScene::s_popupMenuRequested), this, qOverload<BaseObject *>(&ModelWidget::configurePopupMenu), Qt::QueuedConnection);
	connect(new_scene, qOverload<>(&ObjectsScene::s_popupMenuRequested), this,  &ModelWidget::showObjectMenu, Qt::QueuedConnection);
	connect(new_scene, &ObjectsScene::s_childrenSelectionChanged, this, &ModelWidget::configureObjectSelection);
	connect(new_scene, &ObjectsScene::s_objectsSelectedInRange, this, &ModelWidget::configureObjectSelection);

	connect(new_scene, &ObjectsScene::s_selectionResetTriggered, this, [this, new_scene](){
		for(auto &scn : scenes)
		{
			if(scn != new_scene)
				scn->clearSelection();
		}
	});

	connect(new_scene, &ObjectsScene::s_collapseModeChanged, this, [this](){
		setModified(true);
	});

	connect(new_scene, &ObjectsScene::s_paginationToggled, this, [this](){
		setModified(true);
	});

	connect(new_scene, &ObjectsScene::s_currentPageChanged, this, [this](){
		setModified(true);
	});

	connect(new_scene, &ObjectsScene::s_objectsMovedLayer, this, [this](){
		setModified(true);
	});

	connect(new_scene, &ObjectsScene::s_ensureVisibleRequested, this, [this](const QRectF &rect){
		viewport->ensureVisible(rect);
	});

	connect(new_scene, &ObjectsScene::s_sceneRectChanged, this, [this](const QRectF &){
		viewport->resetCachedContent();
		setModified(true);
	});

	connect(new_scene, &ObjectsScene::s_layersChanged, this, &ModelWidget::updateModelLayersInfo);
	connect(new_scene, &ObjectsScene::s_activeLayersChanged, this, &ModelWidget::updateModelLayersInfo);
	connect(new_scene, qOverload<BaseObject *>(&ObjectsScene::s_popupMenuRequested), new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(new_scene, qOverload<>(&ObjectsScene::s_popupMenuRequested), new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(new_scene, &ObjectsScene::s_objectSelected, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(new_scene, &ObjectsScene::s_childrenSelectionChanged, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);
	connect(new_scene, &ObjectsScene::s_scenePressed, new_obj_overlay_wgt, &NewObjectOverlayWidget::hide);

	scenes.append(new_scene);

	if(auto_activate && !scene)
	{
		active_scene_idx = scenes.size() - 1;
		scene = new_scene;
		viewport->setScene(new_scene);
	}

	emit s_scenesListChanged();

	return new_scene;
}

void ModelWidget::removeScene(ObjectsScene *scn)
{
	if(!scn || !scenes.contains(scn))
		return;

	#ifdef PGMODELER_DEBUG
		/* A scene's items() always includes auxiliary/infrastructure items (the relationship
		 * creation tool's rubber-band line, the selection rect, layer paths, etc.) that ~ObjectsScene()
		 * already cleans up on its own -- only a leftover BaseObjectView (the common base of every
		 * database object's graphical representation: table, view, schema, relationship, textbox)
		 * indicates an object that should've been relocated/removed before the scene is destroyed */
		for(auto &itm : scn->items())
		{
			if(dynamic_cast<BaseObjectView *>(itm) && !itm->parentItem())
			{
				qWarning("ModelWidget::removeScene: destroying a scene that still has database objects in it");
				break;
			}
		}
	#endif

	bool was_active = (scn == scene);

	scenes.removeOne(scn);

	if(was_active)
	{
		active_scene_idx = -1;
		scene = nullptr;

		if(scenes.isEmpty())
			viewport->setScene(nullptr);
		else
			setActiveScene(0);
	}
	else
		active_scene_idx = scenes.indexOf(scene);

	delete scn;
	emit s_scenesListChanged();
}

void ModelWidget::setActiveScene(int idx)
{
	if(idx < 0 || idx >= scenes.size() || idx == active_scene_idx)
		return;

	/* The interactive relationship-line tool (click table A, drag/click table B) is scene-local --
	 * rel_line lives in the old ObjectsScene and only that scene ever receives mouse events while
	 * attached to the viewport, so it can't follow the user to another scene. Cross-scene
	 * relationships are still possible (select both tables first, Ctrl+click across scenes, then
	 * "New > Relationship"), just not through this click-and-drag gesture -- so switching scenes
	 * mid-gesture cancels it outright instead of leaving it stuck pointing at a table that's no
	 * longer reachable */
	if(new_obj_type > ObjectType::BaseTable)
		cancelObjectAddition();

	/* Selection now spans every scene at once -- switching the active scene must not touch it.
	 * Only an explicit reset (empty-area click, or a plain click replacing a selection) clears
	 * anything, and that already reaches every scene on its own (s_selectionResetTriggered) */
	active_scene_idx = idx;
	scene = scenes.at(active_scene_idx);

	viewport->setScene(scene);
	applyZoom(current_zoom);

	emit s_activeSceneChanged(active_scene_idx);
}

void ModelWidget::setPendingSegmentationMode(DatabaseModel::SegmentationMode mode)
{
	db_model->setSavedSegmentationMode(mode);
	setModified(true);

	if(GeneralConfigWidget::getConfigurationParam(Attributes::Configuration,
																								Attributes::AlertSaveReloadModel) == Attributes::True)
	{
		Messagebox msgbox;

		msgbox.setCustomOptionText(tr("Don't ask to save and reload again."));
		msgbox.show(tr("Changing the segmentation mode takes effect only after saving and reloading the model. Do you want to proceed?"),
								Messagebox::Confirm, Messagebox::YesNoButtons);

		if(msgbox.isAccepted())
			emit s_modelSaveReloadRequested();

		GeneralConfigWidget::appendConfigurationSection(Attributes::Configuration,
																										{{ Attributes::AlertSaveReloadModel,
																											 msgbox.isCustomOptionChecked() ?
																											 Attributes::False : Attributes::True }});
	}
}

void ModelWidget::setPendingSceneThreshold(unsigned value)
{
	db_model->setSavedSegThreshold(value);
	setModified(true);
}

void ModelWidget::handleObjectAddition(BaseObject *object)
{
	BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(object);

	if(!graph_obj)
		return;

	ObjectType obj_type = graph_obj->getObjectType();
	QGraphicsItem *item = nullptr;
	ObjectsScene *target_scene = scene;
	DatabaseModel::SegmentationMode seg_mode = db_model->getSegmentationMode();

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
		{
			BaseRelationship *rel = dynamic_cast<BaseRelationship *>(graph_obj);
			BaseTableView *src_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::SrcTable)->getOverlyingObject()),
										 *dst_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::DstTable)->getOverlyingObject());
			ObjectsScene *src_scene = src_view ? dynamic_cast<ObjectsScene *>(src_view->scene()) : nullptr,
									 *dst_scene = dst_view ? dynamic_cast<ObjectsScene *>(dst_view->scene()) : nullptr;

			/* Tables in the same scene get a regular RelationshipView. Tables in different scenes
			get no RelationshipView at all -- the relationship is registered as an external
			connection on each table instead, so BaseTableView can draw its own cut-off marker */
			if(src_scene && src_scene == dst_scene)
			{
				item = new RelationshipView(rel);
				target_scene = src_scene;
			}
			else
			{
				if(src_view)
					src_view->addExternalRelationship(rel);

				if(dst_view)
					dst_view->addExternalRelationship(rel);
			}
		}
		break;

		case ObjectType::Schema:
		{
			Schema *schema = dynamic_cast<Schema *>(graph_obj);

			//Dedicated schema/scene 1:1 mapping only applies in schema-segmentation mode
			if(seg_mode == DatabaseModel::SegBySchema)
			{
				target_scene = getSchemaScene(schema);

				if(!target_scene)
				{
					target_scene = addScene(schema);
					applySceneLayers(target_scene);

					/* public is the conventional default schema -- while the model is being built/loaded,
					make sure its scene ends up active even if pg_catalog (or some other schema) was
					processed first */
					if(db_model->isModelLoading() && schema->getName() == "public")
						setActiveScene(scenes.indexOf(target_scene));

					/* addScene() above already emitted s_scenesListChanged(), but schema_scenes didn't
					 * have this schema's entry yet at that point -- getSceneNames() would've fallen back
					 * to "Scene N" for it. Re-emit now that the mapping is complete, and keep the combo
					 * in sync with future renames too (scenes_cmb has no view of its own to react to) */
					connect(schema, &Schema::s_objectModified, this, &ModelWidget::s_scenesListChanged);
					emit s_scenesListChanged();
				}
			}

			/* The schema rectangle groups children that share a single view and recomputes its
			 * bounding box on every child edit -- in any segmented mode that either runs for
			 * nothing (schema == scene, so the box would just mirror the scene itself) or can't
			 * be drawn correctly at all (children split across scenes), so it's skipped outright
			 * rather than built and hidden */
			if(seg_mode == DatabaseModel::NoSegmentation)
				item = new SchemaView(schema);
		}
		break;

		default:
			item = new StyledTextboxView(dynamic_cast<Textbox *>(graph_obj));
		break;
	}

	if(seg_mode == DatabaseModel::SegBySchema && graph_obj->getSchema())
		target_scene = getSchemaScene(dynamic_cast<Schema *>(graph_obj->getSchema()));
	else if(seg_mode == DatabaseModel::SegByThreshold && BaseTable::isBaseTable(obj_type))
	{
		BaseTable *base_tab = dynamic_cast<BaseTable *>(graph_obj);
		int scn_idx = base_tab->getSceneIndex();

		if(scn_idx >= 0)
		{
			/* Object already carries a scene index -- from a file being loaded, or an undo
			 * restoring a previously-removed object. Widen the scenes collection as needed and
			 * route it there directly, bypassing the live counter entirely */
			while(scenes.size() <= scn_idx)
			{
				ObjectsScene *new_scene = addScene(scenes.isEmpty());
				applySceneLayers(new_scene);
			}

			target_scene = scenes.at(scn_idx);
		}
		else
		{
			/* Naive counter: every getSegThreshold() Table/View/ForeignTable insertions opens a
			 * new scene, and every insertion after that goes to it -- no clustering by
			 * connectivity, strictly insertion order */
			if(scenes.isEmpty())
			{
				target_scene = addScene();
				applySceneLayers(target_scene);
			}
			else
				target_scene = scenes.last();

			base_tab->setSceneIndex(scenes.indexOf(target_scene));

			if(++threshold_obj_count % db_model->getSegThreshold() == 0)
			{
				ObjectsScene *new_scene = addScene(false);
				applySceneLayers(new_scene);
			}
		}
	}
	/* Textbox never enters the threshold counter (same rule as the rest of Fase 1/2: it has no
	 * schema to route by, and doesn't count towards the insertion threshold either) -- but a
	 * manually reassigned Textbox (subtask 4.2, moveToScene()) or one loaded from a file that
	 * already carries a saved index still needs to land in that exact scene, in either segmented
	 * mode. NoSegmentation is excluded on purpose: there's only ever one scene there, and a stale
	 * index left over from a previous segmented-mode session must not widen the scenes collection */
	else if(obj_type == ObjectType::Textbox && seg_mode != DatabaseModel::NoSegmentation)
	{
		int scn_idx = graph_obj->getSceneIndex();

		if(scn_idx >= 0)
		{
			while(scenes.size() <= scn_idx)
			{
				ObjectsScene *new_scene = addScene(scenes.isEmpty());
				applySceneLayers(new_scene);
			}

			target_scene = scenes.at(scn_idx);
		}
	}

	//Safety net for the rare case an object arrives before any scene exists yet (e.g. no schema created so far)
	if(!target_scene)
	{
		target_scene = addScene();
		applySceneLayers(target_scene);

		/* A schema can land here when its segmentation-mode routing wasn't available yet at the
		 * time it was added (e.g. pg_catalog, always created by createSystemObjects() before the
		 * file's own seg-mode attribute is read from the XML in loadModel()). Register it in
		 * schema_scenes right away, regardless of the current mode -- otherwise, in schema
		 * segmentation, any of its own children parsed later in the same load (a reverse-engineered
		 * table living directly in pg_catalog, for instance) would find getSchemaScene() still
		 * returning nullptr and each land in their own separate safety-net scene instead of this one */
		Schema *schema = dynamic_cast<Schema *>(graph_obj);

		if(schema && !schema_scenes.contains(schema))
			schema_scenes[schema] = target_scene;
	}

	/* A Textbox routed here without a scene index of its own (fresh object, landed on whatever
	 * scene was active) still needs that index recorded on the model -- otherwise any later
	 * operation that snapshots/restores it (moveToScene()'s undo, for instance) captures -1
	 * instead of where it actually is, and can no longer resolve it back to a real scene */
	if(obj_type == ObjectType::Textbox && seg_mode != DatabaseModel::NoSegmentation &&
			graph_obj->getSceneIndex() < 0)
		graph_obj->setSceneIndex(scenes.indexOf(target_scene));

	if(item)
		target_scene->addItem(item, blink_new_objs);

	if(!db_model->isModelLoading())
		setModified(true);

	emit s_objectAdded(graph_obj);

	if(item && blink_new_objs)
		viewport->centerOn(item);
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
							cursor_ico = GuiUtilsNs::getPixmap("cursortmpl").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation);
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
		QGraphicsItem *item = dynamic_cast<QGraphicsItem *>(graph_obj->getOverlyingObject());
		ObjectsScene *item_scene = item ? dynamic_cast<ObjectsScene *>(item->scene()) : nullptr;

		if(item_scene)
			item_scene->removeItem(item);
		else if(graph_obj->getObjectType() == ObjectType::Relationship ||
						graph_obj->getObjectType() == ObjectType::BaseRelationship)
		{
			// Cross-scene relationship: no RelationshipView, just clear the cut-off markers
			BaseRelationship *rel = dynamic_cast<BaseRelationship *>(graph_obj);
			BaseTableView *src_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::SrcTable)->getOverlyingObject()),
										 *dst_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::DstTable)->getOverlyingObject());

			if(src_view)
				src_view->removeExternalRelationship(rel);

			if(dst_view)
				dst_view->removeExternalRelationship(rel);
		}

		if(graph_obj->getObjectType() == ObjectType::Schema)
		{
			Schema *schema = dynamic_cast<Schema *>(graph_obj);
			ObjectsScene *schema_scene = getSchemaScene(schema);

			if(schema_scene)
			{
				schema_scenes.remove(schema);
				removeScene(schema_scene);
			}
		}

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

void ModelWidget::showCutoffRelationships(BaseTable *table)
{
	if(!table)
		return;

	BaseForm base_form(this);
	CrossSceneRelsWidget *cross_scene_rels_wgt = new CrossSceneRelsWidget;

	cross_scene_rels_wgt->setAttributes(db_model, table);
	base_form.setMainWidget(cross_scene_rels_wgt);
	base_form.setButtonConfiguration(Messagebox::CloseButton);

	connect(cross_scene_rels_wgt, &CrossSceneRelsWidget::s_relationshipActivated, this, [this, cross_scene_rels_wgt](BaseRelationship *rel){
		showObjectForm(rel->getObjectType(), rel);
		cross_scene_rels_wgt->updateRelationships();
	});

	GeneralConfigWidget::restoreWidgetGeometry(&base_form, cross_scene_rels_wgt->metaObject()->className());
	base_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&base_form, cross_scene_rels_wgt->metaObject()->className());
}

void ModelWidget::handleObjectsMovement(ObjectsScene *src_scene, bool end_moviment)
{
	std::vector<BaseObject *>::iterator itr, itr_end;
	std::vector<BaseObject *> reg_tables;
	QList<BaseObjectView *> tables;
	BaseGraphicObject *obj=nullptr;
	Schema *schema=nullptr;

	/* Dragging is a purely visual, mouse-driven gesture that only ever happens in the scene
	 * currently attached to the viewport -- objects selected in any other scene never receive a
	 * mouseMoveEvent and don't actually move, so only src_scene's own subset of the (now
	 * cross-scene) selection belongs in this operation */
	std::vector<BaseObject *> moved_objs;

	for(auto &sel_obj : selected_objects)
	{
		BaseGraphicObject *sel_graph_obj = dynamic_cast<BaseGraphicObject *>(sel_obj);
		QGraphicsItem *sel_item = sel_graph_obj ? dynamic_cast<QGraphicsItem *>(sel_graph_obj->getOverlyingObject()) : nullptr;

		if(sel_item && sel_item->scene() == src_scene)
			moved_objs.push_back(sel_obj);
	}

	itr=moved_objs.begin();
	itr_end=moved_objs.end();

	// See the comment in startSceneMove() about dropping antialiasing while the view is actively changing
	viewport->setRenderHint(QPainter::Antialiasing, end_moviment ? !disable_render_smooth : false);
	viewport->setRenderHint(QPainter::TextAntialiasing, end_moviment ? !disable_render_smooth : false);

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
	{
		//Selection now spans every scene -- unite each one's own selected-items rect to reflect the whole thing
		QRectF sel_rect;

		for(auto &scn : scenes)
			sel_rect = sel_rect.united(scn->itemsBoundingRect(true, true));

		emit s_sceneInteracted(static_cast<int>(selected_objects.size()), sel_rect);
	}
}

void ModelWidget::setProtected(bool protect)
{
	QLabel *msg_lbl = protected_model_frm->findChild<QLabel *>("message_lbl");
	msg_lbl->setText(tr("The database model is protected. Modifications are disabled!"));

	protected_model_frm->setVisible(protect);
	db_model->setProtected(protect);
}

void ModelWidget::setInteractive(bool interact)
{
	QLabel *msg_lbl = protected_model_frm->findChild<QLabel *>("message_lbl");
	msg_lbl->setText(tr("The interaction over the database model is blocked. Modifications are temporarily disabled until the blocking operation finishes!"));

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

	/* Antialiasing is one of the most expensive parts of painting relationship
	 * lines/curves, which tend to have large bounding rects and thus get
	 * repainted as a side effect of any nearby change during panning/zooming.
	 * Dropping it for the duration of the move trades a bit of visual quality
	 * for a much cheaper repaint while the view is actively moving; the real
	 * preference is restored once the move settles (see finishSceneMove()). */
	viewport->setRenderHint(QPainter::Antialiasing, false);
	viewport->setRenderHint(QPainter::TextAntialiasing, false);

	viewport->resetCachedContent();
	scene->setShowSceneLimits(false);
}

void ModelWidget::finishSceneMove()
{
	scene_moving = false;
	ObjectsScene::setShowGrid(curr_show_grid);
	ObjectsScene::setShowPageDelimiters(curr_show_delim);
	updateRenderHints();
	viewport->resetCachedContent();
	scene->setShowSceneLimits(true);
	scene->invalidate(viewport->sceneRect());
}

void ModelWidget::setBlinkAddedObjects(bool value)
{
	blink_new_objs = value;
}

void ModelWidget::selectObjectsInLayers(const QList<unsigned int> &layer_ids)
{
	scene->blockSignals(true);
	scene->selectObjectsInLayers(layer_ids);
	scene->blockSignals(false);
	configureObjectSelection();
	emitSceneInteracted();
}

void ModelWidget::configureObjectSelection()
{
	//Selection now spans every scene, not just the active one -- aggregate them all here
	QList<QGraphicsItem *> items;

	for(auto &scn : scenes)
		items.append(scn->selectedItems());

	BaseObjectView *item = nullptr;
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

	/* Cutoff marker selection is exclusive with regular object selection (see CutoffItem::
	 * itemChange(), which handles the opposite direction). A cutoff can only ever be selected in
	 * whichever scene was active at the time of the click, and it can outlive that scene losing
	 * focus (selection now spans every scene, unchanged by setActiveScene()) -- so a table/
	 * relationship/etc. picked up here, in any scene, still needs this sweep across every scene to
	 * catch a cutoff left selected elsewhere */
	if(!selected_objects.empty())
	{
		for(auto &scn : scenes)
		{
			for(auto &sel_item : scn->selectedItems())
			{
				CutoffItem *cutoff = dynamic_cast<CutoffItem *>(sel_item);

				if(cutoff)
					cutoff->setSelected(false);
			}
		}
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

				/* Cancels the operation after showing the relationship editing form. The scenes were already
				 * silently cleared above (blockSignals), so this rarely has any item left to deselect and the
				 * usual selection-changed signal chain won't fire to resync selected_objects -- clear it here
				 * directly instead of relying on that chain */
				clearSelectionInAllScenes();
				selected_objects.clear();
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
		//Selects everything in every scene -- setSelectionArea() replaces each scene's own selection, so nothing else to clear first
		for(auto &scn : scenes)
		{
			QPainterPath pth;
			pth.addRect(scn->sceneRect());

			scn->blockItemsSignals(true);
			scn->setSelectionArea(pth);
			scn->blockItemsSignals(false);
		}
	}
	else
	{
		BaseObjectView *obj_view = nullptr;
		std::vector<BaseObject *> objs = *db_model->getObjectList(obj_type);

		if(obj_type == ObjectType::Relationship)
		{
			objs.insert(objs.end(),
									db_model->getObjectList(ObjectType::BaseRelationship)->begin(),
									db_model->getObjectList(ObjectType::BaseRelationship)->end());
		}

		clearSelectionInAllScenes();

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

						for(auto &pk_col : pk_cols)
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

ModelWidget::AlternateSegMode ModelWidget::getAlternateSegMode(const QString &filename)
{
	AlternateSegMode suggestion;

	if(filename.isEmpty())
		return suggestion;

	try
	{
		XmlParser xmlparser;
		QHash<QString, unsigned> objs_per_schema;
		QString elem_name;
		attribs_map attribs;

		xmlparser.loadXMLFile(filename);
		xmlparser.getElementAttributes(attribs);

		// A model that already picked a mode -- explicitly or via a previous suggestion -- is never second-guessed
		if(!attribs[Attributes::SegMode].isEmpty())
			return suggestion;

		if(!xmlparser.accessElement(XmlParser::ChildElement))
			return suggestion;

		do
		{
			if(xmlparser.getElementType() != XML_ELEMENT_NODE)
				continue;

			elem_name = xmlparser.getElementName();

			if(!BaseTable::isBaseTable(BaseObject::getObjectType(elem_name)))
				continue;

			// Table/view/foreigntable elements reference their schema via a <schema name=".."/> child
			xmlparser.savePosition();

			if(xmlparser.accessElement(XmlParser::ChildElement))
			{
				do
				{
					if(xmlparser.getElementType() == XML_ELEMENT_NODE &&
						 xmlparser.getElementName() == Attributes::Schema)
					{
						xmlparser.getElementAttributes(attribs);
						objs_per_schema[attribs[Attributes::Name]]++;
						break;
					}
				}
				while(xmlparser.accessElement(XmlParser::NextElement));
			}

			xmlparser.restorePosition();
		}
		while(xmlparser.accessElement(XmlParser::NextElement));

		for(auto itr = objs_per_schema.begin(); itr != objs_per_schema.end(); itr++)
		{
			suggestion.total_objs += itr.value();

			if(itr.value() > suggestion.max_schema_objs)
			{
				suggestion.max_schema_objs = itr.value();
				suggestion.max_schema_name = itr.key();
			}
		}

		if(suggestion.total_objs >= DatabaseModel::DefaultSegThreshold)
		{
			bool concentrated =
					(static_cast<double>(suggestion.max_schema_objs) / suggestion.total_objs) >= ConcentrationSuggestThreshold;

			suggestion.mode = concentrated ? DatabaseModel::SegByThreshold : DatabaseModel::SegBySchema;
		}
	}
	/* A malformed/unreadable file here just means no suggestion -- the real loadModel() call right
	 * after this one will surface the actual error to the user */
	catch(Exception &)
	{
		return {};
	}

	return suggestion;
}

ModelWidget::AlternateSegMode ModelWidget::checkAlternateSegmentation(const QString &filename)
{
	AlternateSegMode alt_seg_mode = getAlternateSegMode(filename);

	if(!alt_seg_mode.isValid())
		return {};

	double pct = (100.0 * alt_seg_mode.max_schema_objs) / alt_seg_mode.total_objs;
	QString msg;

	if(alt_seg_mode.mode == DatabaseModel::SegByThreshold)
	{
		msg = tr("This model concentrates <strong>%1%</strong> of its <strong>%2</strong> tables/views/foreign tables in a "
						 "single schema <strong>%3</strong>. Segmenting the canvas by schema wouldn't distribute "
						 "much of that load into separate scenes. Load it with threshold-based segmentation instead?")
					.arg(QString::number(pct, 'f', 0))
					.arg(alt_seg_mode.total_objs)
					.arg(alt_seg_mode.max_schema_name);
	}
	else
	{
		msg = tr("This model spreads its <strong>%1</strong> tables/views/foreign tables across several "
						 "schemas, none concentrating more than <strong>%2%</strong> of them (the largest, <strong>%3</strong>, "
						 "holds <strong>%4%</strong>). Segmenting the canvas by schema instead of by threshold would "
						 "distribute that load into separate scenes. Load it with schema-based segmentation instead?")
					.arg(alt_seg_mode.total_objs)
					.arg(QString::number(100 * ConcentrationSuggestThreshold, 'f', 0),
							 alt_seg_mode.max_schema_name,
							 QString::number(pct, 'f', 0));
	}

	if(Messagebox::isAccepted(Messagebox::confirm(msg)))
		return alt_seg_mode;

	return {};
}

bool ModelWidget::needsThresholdRebucketing(const QString &filename)
{
	if(filename.isEmpty())
		return false;

	try
	{
		XmlParser xmlparser;
		attribs_map attribs;

		xmlparser.loadXMLFile(filename);
		xmlparser.getElementAttributes(attribs);

		if(attribs[Attributes::SegMode].isEmpty() ||
			 static_cast<DatabaseModel::SegmentationMode>(attribs[Attributes::SegMode].toUInt()) != DatabaseModel::SegByThreshold ||
			 attribs[Attributes::SegObjCount].isEmpty())
			return false;

		unsigned total_objs = attribs[Attributes::SegObjCount].toUInt();
		unsigned threshold = attribs[Attributes::SegThreshold].toUInt();

		if(threshold == 0)
			threshold = DatabaseModel::DefaultSegThreshold;

		// Same formula handleObjectAddition()'s naive counter reproduces, including its trailing empty scene when total_objs is an exact multiple of threshold
		unsigned expected_scenes = (total_objs / threshold) + 1;
		unsigned actual_scenes = 1;

		if(xmlparser.accessElement(XmlParser::ChildElement))
		{
			do
			{
				if(xmlparser.getElementType() != XML_ELEMENT_NODE || xmlparser.getElementName() != Attributes::Scenes)
					continue;

				actual_scenes = 0;

				if(xmlparser.accessElement(XmlParser::ChildElement))
				{
					do
					{
						if(xmlparser.getElementType() == XML_ELEMENT_NODE)
							actual_scenes++;
					}
					while(xmlparser.accessElement(XmlParser::NextElement));
				}

				break;
			}
			while(xmlparser.accessElement(XmlParser::NextElement));
		}

		return expected_scenes != actual_scenes;
	}
	/* A malformed/unreadable file here just means no verdict -- the real loadModel() call right
	 * after this one will surface the actual error to the user */
	catch(Exception &)
	{
		return false;
	}
}

void ModelWidget::loadModel(const QString &filename, bool check_alt_seg_mode)
{
	TaskProgressWidget task_prog_wgt(this);
	bool curr_blink_objs = blink_new_objs;
	AlternateSegMode alt_seg_mode;

	try
	{
		if(check_alt_seg_mode)
			alt_seg_mode = checkAlternateSegmentation(filename);

		#ifdef PGMODELER_DEBUG
			quint64 start = QDateTime::currentMSecsSinceEpoch();
		#endif

		connect(db_model, &DatabaseModel::s_objectLoaded, &task_prog_wgt, qOverload<int, QString, unsigned>(&TaskProgressWidget::updateProgress));
		task_prog_wgt.addIcon(enum_t(ObjectType::BaseObject), GuiUtilsNs::getIcon("design"));
		task_prog_wgt.setWindowTitle(tr("Loading database model"));
		task_prog_wgt.setCancelable(true);
		task_prog_wgt.show();

		// Disable objects blinking during model loading
		blink_new_objs = false;

		bool rebucketed = needsThresholdRebucketing(filename);

		db_model->loadModel(filename,
												alt_seg_mode.isValid(), alt_seg_mode.mode,
												DatabaseModel::DefaultSegThreshold,
												rebucketed,
												[&task_prog_wgt](){ return task_prog_wgt.wasCanceled(); });

		blink_new_objs = curr_blink_objs;

		/* Any cut-off marker attached while its table's box was still stuck at its pre-configuration
		 * 0x0 default (updates are globally disabled for the whole duration of a bulk load, so this
		 * is a real possibility, not just a theoretical one) never gets a chance to reposition itself
		 * against the table's real geometry -- DatabaseModel::loadModel()'s final setObjectsModified()
		 * pass does give every table its final size, but deliberately suppresses the very signals
		 * CutoffItem listens for while doing so (see BaseTable::isGeomChangesIgnored()). Refresh them
		 * all explicitly, now that every table's box is settled.
		 * Note: getObjectList() (every object of the type, unfiltered), not getObjects() (which
		 * defaults to filtering by schema==nullptr, i.e. only schema-less objects -- never what's
		 * wanted here) */
		for(auto &type : { ObjectType::Table, ObjectType::View, ObjectType::ForeignTable })
		{
			for(auto &obj : *db_model->getObjectList(type))
			{
				BaseTableView *tab_view =
						dynamic_cast<BaseTableView *>(dynamic_cast<BaseGraphicObject *>(obj)->getOverlyingObject());

				if(tab_view && tab_view->getCutoffItem())
					tab_view->getCutoffItem()->updatePosition();
			}
		}

		/* In threshold-segmentation mode, every loaded Table/View/ForeignTable already routed itself
		 * via its own persisted scene index (see handleObjectAddition()), without touching
		 * threshold_obj_count -- reseed it now from how many of them landed in the last scene, so
		 * further interactive insertions resume filling it instead of restarting the count at zero */
		if(db_model->getSegmentationMode() == DatabaseModel::SegByThreshold && !scenes.isEmpty())
		{
			int last_idx = scenes.size() - 1;
			std::vector<ObjectType> types { ObjectType::Table, ObjectType::View, ObjectType::ForeignTable };

			threshold_obj_count = 0;

			for(auto &type : types)
			{
				for(auto &obj : db_model->getObjects(type))
				{
					if(dynamic_cast<BaseTable *>(obj)->getSceneIndex() == last_idx)
						threshold_obj_count++;
				}
			}
		}

		this->filename = filename;

		/* The persisted per-scene rects below are keyed by scene index and were fitted to whichever
		 * objects occupied that index under the OLD threshold -- rebucketing just changed which
		 * objects occupy each index, so trusting them here would clip scenes to a bounding box that
		 * no longer matches what they actually contain. Clearing them makes adjustAllScenesRects()
		 * fall back to recomputing every scene's rect from its real, current items instead */
		if(rebucketed)
			db_model->setSceneRects({});

		updateSceneLayers();
		updateObjectsOpacity();
		adjustAllScenesRects(!rebucketed);

		task_prog_wgt.close();
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
		blink_new_objs = curr_blink_objs;
		task_prog_wgt.close();
		setModified(false);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelWidget::updateSceneLayers()
{
	for(auto &scn : scenes)
		applySceneLayers(scn);

	db_model->setObjectsModified({ ObjectType::Schema });
}

void ModelWidget::applySceneLayers(ObjectsScene *scn)
{
	scn->blockSignals(true);
	scn->addLayers(db_model->getLayers(), false);
	scn->setLayerColors(ObjectsScene::LayerNameColor, db_model->getLayerNameColors());
	scn->setLayerColors(ObjectsScene::LayerRectColor, db_model->getLayerRectColors());
	scn->setLayerNamesVisible(db_model->isLayerNamesVisible());
	scn->setLayerRectsVisible(db_model->isLayerRectsVisible());
	scn->setActiveLayers(db_model->getActiveLayers());
	scn->blockSignals(false);
}

void ModelWidget::alignObjectsToGrid()
{
	for(auto &scn : scenes)
		scn->alignObjectsToGrid();
}

void ModelWidget::adjustSceneRect(bool use_model_rect, bool expand_only)
{
	if(ObjectsScene::isAlignObjectsToGrid())
		alignObjectsToGrid();

	QRectF rect = db_model->getSceneRect();

	if(use_model_rect && rect.isValid())
		scene->setSceneRect(rect);
	else
		rect = scene->adjustSceneRect(expand_only);

	setModified(true);
	emit s_sceneInteracted(rect.size());
}

void ModelWidget::adjustAllScenesRects(bool use_model_rect)
{
	if(use_model_rect)
		adjustSceneRect(true);

	QMap<int, QRectF> persisted_rects = db_model->getSceneRects();

	for(int idx = 0; idx < scenes.size(); idx++)
	{
		ObjectsScene *scn = scenes.at(idx);

		if(use_model_rect && scn == scene)
			continue;

		/* A persisted rect takes precedence -- falls back to the items' own bounding rect (no side
		 * effects, items stay exactly where they are) for scenes with no persisted entry: an old
		 * file predating this attribute, a scene created since the last save, or -- when this whole
		 * method was called with use_model_rect false -- every scene, since the caller already knows
		 * none of the persisted rects (cleared beforehand) describe current scene membership */
		if(persisted_rects.contains(idx))
			scn->setSceneRect(persisted_rects.value(idx));
		else
			scn->setSceneRect(scn->itemsBoundingRect(true, false));
	}
}

void ModelWidget::expandSceneRect(ObjectsScene::ExpandDirection exp_dir)
{
	scene->expandSceneRect(exp_dir);
}

void ModelWidget::printModel(QPrinter *printer, bool print_grid, bool print_page_nums, bool resize_delims)
{
	if(!printer)
		return;

	bool show_grid = false, show_delims = false, first_page = true;
	unsigned page = 0, h_page_cnt = 0, v_page_cnt = 0, h_pg_id = 0, v_pg_id = 0, global_page = 0;
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

	for(auto &scn : scenes)
	{
		scn->setShowSceneLimits(false);
		scn->update();
		scn->clearSelection();
	}

	//Creates a painter to draw the model directly on the printer
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);
	font.setPointSizeF(BaseObjectView::DefaultFontSize * 0.75);
	font.setBold(true);
	painter.setFont(font);

	margins = printer->pageLayout().marginsPoints();

	//Print every scene's pages in sequence onto the same printer job, one page after another
	for(auto &scn : scenes)
	{
		//Get the pages rect for printing
		pages = scn->getPagesForPrinting(printer->pageLayout(), h_page_cnt, v_page_cnt,
																			resize_delims && current_zoom < 1 ? current_zoom : 1);

		for(page = 0, h_pg_id = 1, v_pg_id = 1; page < static_cast<unsigned>(pages.size()); page++, global_page++)
		{
			if(!first_page)
				printer->newPage();

			first_page = false;

			//Render the current page on the printer
			scn->render(&painter, QRect(), pages[page]);

			//Print the current page number if this option is marked
			if(print_page_nums)
			{
				page_info = tr("Page #%1 / X:%2 Y:%3").arg(QString::number(global_page + 1)).arg(h_pg_id).arg(v_pg_id);
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
		}
	}

	//Restore the grid option backup
	ObjectsScene::setCanvasColor(bg_color);
	ObjectsScene::setShowGrid(show_grid);
	ObjectsScene::setShowPageDelimiters(show_delims);

	for(auto &scn : scenes)
	{
		scn->setShowSceneLimits(true);
		scn->update();
	}
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
			bkpfile = UtilsNs::getTemporaryFilePath(fi.absolutePath() +
																							GlobalAttributes::DirSeparator +
																							QString("%1_XXXXXX%2").arg(db_model->getName(), GlobalAttributes::DbModelBkpExt));

			/* Trying to rename the original model to the backup filename so
			 * we can write the new one in its place */
			if(!QFile::rename(filename, bkpfile))
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::FileDirectoryNotWritten).arg(bkpfile),
												ErrorCode::FileDirectoryNotWritten,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			has_bkp_file = true;
		}

		saveLastCanvasPosition();
		updateModelSceneRectInfo();
		db_model->saveModel(filename, SchemaParser::XmlCode);
		this->filename=filename;

		task_prog_wgt.close();
		disconnect(db_model, nullptr, &task_prog_wgt, nullptr);
		setModified(false);

		/* Doing a final check to the file regarding its size.
		 * If we have a empty file something went wrong during the saving process (disk failure, thread errors, etc)
		 * so we raise an error to the user and restore the backup file to its original path */
		if(fi.size() == 0)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::ModelFileInvalidSize).arg(filename),
											ErrorCode::ModelFileInvalidSize,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

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

			throw Exception(Exception::getErrorMessage(ErrorCode::ModelFileSaveFailure).arg(filename, bkpfile),
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
	if constexpr (std::is_base_of_v<BaseObjectWidget, WidgetClass>)
	{
		BaseRelationship *rel = dynamic_cast<BaseRelationship *>(widget->getHandledObject());
		editing_form.setMainWidget(widget);

		if(rel)
			class_name.prepend(rel->getRelationshipTypeName().replace(QRegularExpression("( )+|(\\-)+"), ""));
	}
	else
		// Otherwise we use the QWidget version of BaseForm::setMainWidget
		editing_form.setMainWidget(widget);

	editing_form.setButtonConfiguration(button_conf);
	editing_form.enableTrackChanges(true, { "inc_indirect_links_chk", "unique_results_chk",
																					"version_cmb", "code_options_cmb" });

	GeneralConfigWidget::restoreWidgetGeometry(&editing_form, class_name);
	res = editing_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&editing_form, class_name);

	if constexpr (std::is_base_of_v<BaseObjectWidget, WidgetClass>)
	{
		if(res == QDialog::Accepted)
			relocateEditedObject(widget);
	}

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
			clearSelectionInAllScenes();
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

void ModelWidget::showObjectAssociations()
{
	QAction *obj_sender=dynamic_cast<QAction *>(sender());

	if(obj_sender)
	{
		BaseObject *object=reinterpret_cast<BaseObject *>(obj_sender->data().value<void *>());

		if(object)
		{
			ObjectAssociationsWidget *obj_assoc_wgt = new ObjectAssociationsWidget;
			obj_assoc_wgt->setAttributes(this, object, true);
			openEditingForm(obj_assoc_wgt, Messagebox::CloseButton);
		}
	}
}

void ModelWidget::showSourceCode()
{
	CodePreviewWidget *code_preview_wgt = new CodePreviewWidget;
	code_preview_wgt->setAttributes(db_model, selected_objects);
	openEditingForm(code_preview_wgt, Messagebox::CloseButton);
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

BaseTable *ModelWidget::getRelationshipPartner(BaseRelationship *rel, BaseTable *tab)
{
	BaseTable *src_tab = rel->getTable(BaseRelationship::SrcTable);
	return src_tab == tab ? rel->getTable(BaseRelationship::DstTable) : src_tab;
}

QGraphicsItem *ModelWidget::createGraphicItem(BaseGraphicObject *graph_obj)
{
	switch(graph_obj->getObjectType())
	{
		case ObjectType::ForeignTable:
		case ObjectType::Table:
			return new TableView(dynamic_cast<PhysicalTable *>(graph_obj));

		case ObjectType::View:
			return new GraphicalView(dynamic_cast<View *>(graph_obj));

		case ObjectType::Textbox:
			return new StyledTextboxView(dynamic_cast<Textbox *>(graph_obj));

		default:
			return nullptr;
	}
}

void ModelWidget::routeRelationship(BaseRelationship *rel)
{
	BaseTableView *src_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::SrcTable)->getOverlyingObject()),
								 *dst_view = dynamic_cast<BaseTableView *>(rel->getTable(BaseRelationship::DstTable)->getOverlyingObject());
	ObjectsScene *src_scene = src_view ? dynamic_cast<ObjectsScene *>(src_view->scene()) : nullptr,
							 *dst_scene = dst_view ? dynamic_cast<ObjectsScene *>(dst_view->scene()) : nullptr;

	/* Same routing rule as handleObjectAddition()'s Relationship case (kept as a separate, small
	 * duplication instead of extracting a shared method -- that choke point already carries a lot
	 * of surrounding side effects, e.g. blink_new_objs/setModified/emit s_objectAdded, that a mere
	 * re-route shouldn't trigger): same scene now -> real RelationshipView; different scenes -> no
	 * view, register as an external (cut-off) connection on both tables instead */
	if(src_scene && src_scene == dst_scene)
		src_scene->addItem(new RelationshipView(rel));
	else
	{
		if(src_view)
			src_view->addExternalRelationship(rel);

		if(dst_view)
			dst_view->addExternalRelationship(rel);
	}
}

void ModelWidget::relocateObjectView(BaseGraphicObject *graph_obj, ObjectsScene *dst_scene)
{
	BaseTableView *old_view = dynamic_cast<BaseTableView *>(graph_obj->getOverlyingObject());
	ObjectsScene *old_scene = old_view ? dynamic_cast<ObjectsScene *>(old_view->scene()) : nullptr;

	if(!old_view || !old_scene || !dst_scene || old_scene == dst_scene)
		return;

	/* connected_rels/external_rels live on the view instance, not the model -- read them now,
	 * before old_view is discarded, or this table's relationships would be silently lost instead
	 * of re-routed */
	std::vector<BaseRelationship *> rels = old_view->connected_rels;
	QList<BaseRelationship *> ext_rels = old_view->getExternalRelationships();

	/* Detach every relationship from this table BEFORE its view is destroyed:
	 * - A same-scene relationship's RelationshipView must be removed here, not after -- removing
	 *   it calls disconnectTables(), which dereferences both tables[] pointers. Doing this once
	 *   old_view is already gone would leave a dangling pointer the instant the view is touched.
	 * - A cross-scene relationship has no view -- drop it from both tables' external list. Doing
	 *   this on old_view's own side too (not just the partner's) matters: its cut-off marker is a
	 *   standalone QGraphicsItem added directly to old_scene, not a Qt child of old_view, so
	 *   removing/discarding old_view below never touches it on its own -- left alone, it stays
	 *   orphaned, still rendered in old_scene, for as long as that scene lives */
	BaseTable *tab = dynamic_cast<BaseTable *>(graph_obj);

	for(auto &rel : rels)
	{
		QGraphicsItem *rel_item = dynamic_cast<QGraphicsItem *>(rel->getOverlyingObject());
		ObjectsScene *rel_scene = rel_item ? dynamic_cast<ObjectsScene *>(rel_item->scene()) : nullptr;

		if(rel_scene)
			rel_scene->removeItem(rel_item);
	}

	for(auto &rel : ext_rels)
	{
		BaseTableView *partner_view = dynamic_cast<BaseTableView *>(getRelationshipPartner(rel, tab)->getOverlyingObject());

		if(partner_view && partner_view != old_view)
			partner_view->removeExternalRelationship(rel);

		old_view->removeExternalRelationship(rel);
	}

	old_scene->removeItem(old_view);
	dst_scene->addItem(createGraphicItem(graph_obj));

	/* The item may sit outside dst_scene's current bounds (it keeps its own (x, y), and the
	 * destination scene is an independent canvas with its own unrelated bounds) -- grow the rect
	 * to include it, or it renders invisible until something else happens to touch the rect */
	dst_scene->adjustSceneRect(true);

	/* Re-route every relationship now that the table has a new view in its new scene --
	 * routeRelationship() reads each table's *current* scene fresh, so this alone also self-heals
	 * the case where two related tables are relocated together in the same operation: the first
	 * one to move routes its relationship as external (partner hasn't moved yet), and re-routing
	 * it again once the second table follows finds them sharing a scene again and creates a real
	 * RelationshipView */
	for(auto &rel : rels)
		routeRelationship(rel);

	for(auto &rel : ext_rels)
		routeRelationship(rel);
}

void ModelWidget::relocateTextboxView(Textbox *txtbox, ObjectsScene *dst_scene)
{
	QGraphicsItem *old_item = dynamic_cast<QGraphicsItem *>(txtbox->getOverlyingObject());
	ObjectsScene *old_scene = old_item ? dynamic_cast<ObjectsScene *>(old_item->scene()) : nullptr;

	if(!old_item || !old_scene || !dst_scene || old_scene == dst_scene)
		return;

	old_scene->removeItem(old_item);
	dst_scene->addItem(createGraphicItem(txtbox));
	dst_scene->adjustSceneRect(true);
}

void ModelWidget::handleObjectSchemaChanged(BaseObject *object, Operation::OperType op_type)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(object);

	if(!graph_obj || op_type != Operation::ObjModified)
		return;

	ObjectType obj_type = graph_obj->getObjectType();
	DatabaseModel::SegmentationMode seg_mode = db_model->getSegmentationMode();
	ObjectsScene *dst_scene = nullptr;

	//Schema-segmentation: a Table/View/ForeignTable's scene follows its schema
	if(BaseTable::isBaseTable(obj_type))
	{
		Schema *schema = dynamic_cast<Schema *>(graph_obj->getSchema());
		dst_scene = schema ? getSchemaScene(schema) : nullptr;
	}

	/* Threshold-segmentation (Table/View/ForeignTable), or Textbox in either segmented mode
	 * both driven by the object's own scene index instead of its schema */
	if(!dst_scene &&
			((BaseTable::isBaseTable(obj_type) && seg_mode == DatabaseModel::SegByThreshold) ||
			 (obj_type == ObjectType::Textbox && seg_mode != DatabaseModel::NoSegmentation)))
	{
		int scn_idx = graph_obj->getSceneIndex();
		dst_scene = (scn_idx >= 0 && scn_idx < scenes.size()) ? scenes.at(scn_idx) : nullptr;
	}

	if(!dst_scene)
		return;

	if(obj_type == ObjectType::Textbox)
		relocateTextboxView(dynamic_cast<Textbox *>(graph_obj), dst_scene);
	else
		relocateObjectView(graph_obj, dst_scene);
}

void ModelWidget::relocateEditedObject(BaseObjectWidget *obj_wgt)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(obj_wgt->object);

	if(!graph_obj || !obj_wgt->prev_schema || graph_obj->getSchema() == obj_wgt->prev_schema)
		return;

	Schema *schema = dynamic_cast<Schema *>(graph_obj->getSchema());
	ObjectsScene *dst_scene = schema ? getSchemaScene(schema) : nullptr;

	if(dst_scene)
		relocateObjectView(graph_obj, dst_scene);
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

				//If the object is a graphical one, relocate its item to the destination schema's scene
				if(obj_graph)
				{
					ObjectsScene *dst_scene=getSchemaScene(schema);

					/* Each schema owns its own scene -- the item follows its object to the destination
					 * schema's scene, keeping its own (x, y): the destination scene is an independent
					 * canvas, so there's no longer a reason to snap it near the new schema's box the
					 * way single-scene mode used to -- doing that here would stack every object moved
					 * into the same schema at the same spot, destroying whatever layout the user had */
					if(dst_scene)
						relocateObjectView(obj_graph, dst_scene);
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
		if(op_id >=0 && op_id > op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void ModelWidget::moveToScene()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	int dst_idx = act->data().toInt();
	ObjectsScene *dst_scene = (dst_idx >= 0 && dst_idx < scenes.size()) ? scenes.at(dst_idx) : nullptr;
	DatabaseModel::SegmentationMode seg_mode = db_model->getSegmentationMode();
	BaseGraphicObject *obj_graph = nullptr;
	ObjectType obj_type;
	bool some_skipped = false;
	int op_id = -1, op_curr_idx = op_list->getCurrentIndex();

	if(!dst_scene)
		return;

	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);

		op_list->startOperationChain();

		for(BaseObject *obj : selected_objects)
		{
			obj_graph=dynamic_cast<BaseGraphicObject *>(obj);
			obj_type=obj->getObjectType();

			/* Only Table/View/ForeignTable and Textbox have an independent position/scene at all --
			 * anything else (Schema, Relationship...) is silently skipped, same as moveToSchema() does
			 * for objects that don't accept a schema	*/
			if(!obj_graph || !(BaseTable::isBaseTable(obj_type) || obj_type == ObjectType::Textbox))
				continue;

			/* Scope restriction in schema-segmentation mode, a Table/View/ForeignTable's scene is 
			 * strictly tied to its schema (handleObjectAddition()), moving just one of them here 
			 * without its schema would break that invariant. Only Textbox, which has no schema to 
 			 * begin with, can be freely reassigned in this mode.
			 * Threshold-segmentation has no such restriction: every eligible type can move freely */
			if(seg_mode == DatabaseModel::SegBySchema && obj_type != ObjectType::Textbox)
			{
				some_skipped = true;
				continue;
			}

			if(obj_graph->getSceneIndex() == dst_idx)
				continue;

			op_id = op_list->registerObject(obj, Operation::ObjModified, -1);
			obj_graph->setSceneIndex(dst_idx);

			if(obj_type == ObjectType::Textbox)
				relocateTextboxView(dynamic_cast<Textbox *>(obj_graph), dst_scene);
			else
				relocateObjectView(obj_graph, dst_scene);
		}

		op_list->finishOperationChain();
		db_model->setObjectsModified();
		this->setModified(true);

		emit s_objectModified();
		qApp->restoreOverrideCursor();

		if(some_skipped)
		{
			Messagebox::alert(tr("Some of the selected objects were not moved due to the segmentation by schema mode."
													 "Table-like objects cannot be moved to another scene other than the one representing their parent schemas."
													 "Only text boxes can be freely reassigned to another scene in this mode."));
		}
	}
	catch(Exception &e)
	{
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
									.arg(obj->getName(), obj->getTypeName()),
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

void ModelWidget::clearSelectionInAllScenes()
{
	for(auto &scn : scenes)
		scn->clearSelection();
}

void ModelWidget::selectSchemaChildren()
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	Schema *schema=nullptr;
	std::vector<ObjectType> types = { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View };
	std::vector<BaseObject *> children;
	BaseObjectView *obj_view = nullptr;

	schema=dynamic_cast<Schema *>(
				 reinterpret_cast<BaseObject *>(
					 dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	clearSelectionInAllScenes();

	/* Reads the children straight from the model instead of going through SchemaView::selectChildren()
	 * -- that method stays in place for its own, unrelated job (the Shift+click gesture on the
	 * schema's own box, NoSegmentation-only, where schema and scene are the same thing by
	 * construction), but it has no notion of scenes at all, so it can't be reused here: a schema's
	 * children can be scattered across several scenes in threshold segmentation */
	for(auto &type : types)
	{
		auto objs = db_model->getObjects(type, schema);
		children.insert(children.end(), objs.begin(), objs.end());
	}

	for(auto &child : children)
	{
		obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(child)->getOverlyingObject());

		if(obj_view)
		{
			obj_view->blockSignals(true);
			obj_view->setSelected(true);
			obj_view->blockSignals(false);
		}
	}

	//Also selects the schema's own box, when it has one (NoSegmentation mode)
	obj_view = dynamic_cast<BaseObjectView *>(schema->getOverlyingObject());

	if(obj_view)
	{
		obj_view->blockSignals(true);
		obj_view->setSelected(true);
		obj_view->blockSignals(false);
	}

	configureObjectSelection();
	emitSceneInteracted();
}

void ModelWidget::selectTableRelationships()
{
	QObject *obj_sender=dynamic_cast<QAction *>(sender());
	BaseTable *table=nullptr;

	table=dynamic_cast<BaseTable *>(
				 reinterpret_cast<BaseObject *>(
					 dynamic_cast<QAction *>(obj_sender)->data().value<void *>()));

	clearSelectionInAllScenes();

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

	clearSelectionInAllScenes();

	for(auto &object : tag->getReferences())
	{
		obj_view = dynamic_cast<BaseObjectView *>(dynamic_cast<BaseGraphicObject *>(object)->getOverlyingObject());
		obj_view->blockSignals(true);
		obj_view->setSelected(true);
		obj_view->blockSignals(false);
	}

	configureObjectSelection();
	emit emitSceneInteracted();
}

void ModelWidget::createTagFromSchema()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	Schema *schema = dynamic_cast<Schema *>(reinterpret_cast<BaseObject *>(act->data().value<void *>()));

	createTagOrLayerFromSchemas({ schema }, false);
}

void ModelWidget::createLayerFromSchema()
{
	QAction *act = dynamic_cast<QAction *>(sender());
	Schema *schema = dynamic_cast<Schema *>(reinterpret_cast<BaseObject *>(act->data().value<void *>()));

	createTagOrLayerFromSchemas({ schema }, true);
}

void ModelWidget::createTagsForAllSchemas()
{
	createTagOrLayerFromSchemas({}, false);
}

void ModelWidget::createLayersForAllSchemas()
{
	createTagOrLayerFromSchemas({}, true);
}

void ModelWidget::createTagOrLayerFromSchemas(const std::vector<Schema *> &schemas, bool as_layer)
{
	std::vector<Schema *> all_schemas;

	/* Empty schemas means "batch for every schema in the model" (see createTagsForAllSchemas()/
	 * createLayersForAllSchemas()) -- that's the only case asking for the extra "this will
	 * create N ..." confirmation, a single targeted schema doesn't need it */
	if(schemas.empty())
	{
		for(auto &obj : *db_model->getObjectList(ObjectType::Schema))
			all_schemas.push_back(dynamic_cast<Schema *>(obj));

		QString msg = as_layer ?
					tr("This will create <strong>%1</strong> layer(s), one for each schema in the model. Continue?") :
					tr("This will create <strong>%1</strong> tag(s), one for each schema in the model. Continue?");

		if(!Messagebox::isAccepted(Messagebox::confirm(msg.arg(all_schemas.size()))))
			return;
	}

	const std::vector<Schema *> &target_schemas = schemas.empty() ? all_schemas : schemas;
	std::vector<ObjectType> child_types = { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View };
	std::vector<std::pair<Schema *, std::vector<BaseTable *>>> schema_children;
	int conflicting_tags = 0;

	for(auto &schema : target_schemas)
	{
		std::vector<BaseTable *> children;

		for(auto &type : child_types)
		{
			for(auto &obj : db_model->getObjects(type, schema))
				children.push_back(dynamic_cast<BaseTable *>(obj));
		}

		/* Layers are cumulative (a table can belong to several at once), so they're always just
		 * added alongside whatever the table already has -- nothing to conflict with. Tags are
		 * exclusive (a table has at most one), so an existing one is a real conflict that needs
		 * a decision upfront */
		if(!as_layer)
		{
			for(auto &tab : children)
			{
				if(tab->getTag())
					conflicting_tags++;
			}
		}

		schema_children.push_back({ schema, children });
	}

	bool replace_tag = false;

	if(conflicting_tags > 0)
	{
		replace_tag = Messagebox::isAccepted(Messagebox::confirm(
					tr("Found <strong>%1</strong> table(s) that already have tags assigned. "
						 "Should the new tag replace it on those tables?").arg(conflicting_tags)));
	}

	int op_id = -1, op_curr_idx = op_list->getCurrentIndex();
	std::vector<BaseObject *> modified_objs;

	try
	{
		op_list->startOperationChain();

		for(auto &[schema, children] : schema_children)
		{
			if(children.empty())
				continue;

			if(as_layer)
			{
				/* Goes through the same path LayersWidget's "new layer" field uses (s_newLayerRequested
				 * -> MainWindow::addNewLayer() -> LayersConfigWidget::addLayer()) instead of touching the
				 * scenes directly, so LayersConfigWidget's own row list stays in sync -- it has no other
				 * way of finding out a layer was added. Direct connection, so by the time emit returns
				 * the layer is already appended to every scene, disambiguated and active in all of them */
				emit s_newLayerRequested(schema->getName());
				unsigned layer_id = static_cast<unsigned>(scene->getLayers().size() - 1);

				for(auto &tab : children)
				{
					op_id = op_list->registerObject(tab, Operation::ObjModified, -1);
					tab->addToLayer(layer_id);
					modified_objs.push_back(tab);
				}

				/* Every child got added to the layer (layers have no conflict to skip on), so the
				 * schema's own rectangle -- now redundant as a visual grouping cue -- can go */
				if(schema->isRectVisible())
				{
					op_id = op_list->registerObject(schema, Operation::ObjModified, -1);
					schema->setRectVisible(false);
					schema->setModified(true);
					modified_objs.push_back(schema);
				}
			}
			else
			{
				Tag *tag = new Tag;
				tag->setName(schema->getName());

				if(db_model->getObjectIndex(tag->getSignature(), ObjectType::Tag) >= 0)
					tag->setName(CoreUtilsNs::generateUniqueName(tag, *db_model->getObjectList(ObjectType::Tag)));

				/* Seeds every element with the same default colors a plain (untagged) table
				 * already has -- a freshly built Tag starts with every color zeroed out (black),
				 * so without this every element other than TableTitle would render solid black */
				for(auto &attr : Tag::getColorAttributes())
				{
					tag->setElementColor(attr, BaseObjectView::getElementColor(attr, ColorId::FillColor1), ColorId::FillColor1);

					if(attr != Attributes::TableName && attr != Attributes::TableSchemaName)
					{
						tag->setElementColor(attr, BaseObjectView::getElementColor(attr, ColorId::FillColor2), ColorId::FillColor2);
						tag->setElementColor(attr, BaseObjectView::getElementColor(attr, ColorId::BorderColor), ColorId::BorderColor);
					}
				}

				tag->setElementColor(Attributes::TableTitle, schema->getFillColor(), ColorId::FillColor1);
				tag->setElementColor(Attributes::TableTitle, schema->getFillColor(), ColorId::FillColor2);
				tag->setElementColor(Attributes::TableTitle,
														 CustomUiStyle::getAdjustedColor(schema->getFillColor(),
																														 CustomUiStyle::MinFactor,
																														 CustomUiStyle::MinFactor),
														 ColorId::BorderColor);

				db_model->addTag(tag);
				op_id = op_list->registerObject(tag, Operation::ObjCreated);
				modified_objs.push_back(tag);

				bool any_tagged = false;

				for(auto &tab : children)
				{
					if(tab->getTag() && !replace_tag)
						continue;

					op_id = op_list->registerObject(tab, Operation::ObjModified, -1);
					tab->clearDependencies();
					tab->setTag(tag);
					tab->updateDependencies();
					modified_objs.push_back(tab);
					any_tagged = true;
				}

				/* Only hide the schema's rectangle if at least one of its children actually got the
				 * new tag -- if every table was skipped due to an existing tag, the rectangle is
				 * still the only visual grouping cue left for this schema */
				if(any_tagged && schema->isRectVisible())
				{
					op_id = op_list->registerObject(schema, Operation::ObjModified, -1);
					schema->setRectVisible(false);
					schema->setModified(true);
					modified_objs.push_back(schema);
				}
			}
		}

		op_list->finishOperationChain();
		db_model->setObjectsModified(modified_objs);
		setModified(true);
		emit s_objectModified();
	}
	catch(Exception &e)
	{
		if(op_id >= 0 && op_id > op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
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
										.arg(object->getName(), object->getTypeName()),
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
						msgbox.show(tr("Do you want to %1 the children of the schema <strong>%2</strong> too?")
												.arg(protect ? tr("protect") : tr("unprotect")).arg(object->getName()),
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
	//Set the flag indicating that a cut operation started
	ModelWidget::cut_operation=true;
	copyObjects(false, copy_deps);
}

void ModelWidget::copyObjects(bool duplicate_mode, bool copy_deps)
{
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
											.arg(selected_objects[0]->getName(), selected_objects[0]->getTypeName()),
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
									 tr("Validating object: `%1' (%2)").arg(object->getName(), object->getTypeName()),
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
									 tr("Generating XML for: `%1' (%2)")
									 .arg(object->getName(), object->getTypeName()),
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
											 tr("Pasting object: `%1' (%2)")
											 .arg(object->getName(), object->getTypeName()),
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
					bool obj_added = false;

					if(sel_table && tab_obj->getObjectType() == ObjectType::Column)
					{
						sel_table->addObject(tab_obj);
						sel_table->setModified(true);
						obj_added = true;
					}
					else if(constr && duplicate_mode &&
							constr->getConstraintType() == ConstraintType::PrimaryKey &&
							constr->getParentTable()->getObjectIndex(constr) < 0)
					{
						constr->getParentTable()->addObject(constr);
						constr->getParentTable()->setModified(true);
						obj_added = true;
					}

					//Updates the fk relationships if the constraint is a foreign-key
					if(constr && constr->getConstraintType() == ConstraintType::ForeignKey)
						db_model->updateTableFKRelationships(dynamic_cast<Table *>(tab_obj->getParentTable()));

					if(obj_added)
						op_list->registerObject(tab_obj, Operation::ObjCreated, -1, tab_obj->getParentTable());
					else
						/* In some cases the object may not be added to the parent table
						 * either by any error from xml parsing or other that prevented the
						 * table object to be inserted in the table, so, to avoid leak, we
						 * delete the object that was created from xml */
						delete tab_obj;
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
						for(BaseObject *ref_obj : sel_obj->getReferences(true))
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
														.arg(object->getName(), object->getTypeName()),
														ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

					//Raises an error if the user try to remove a protected object
					if(object->isProtected())
					{
						throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
														.arg(object->getName(true), object->getTypeName()),
														ErrorCode::RemProtectedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
					}

					tab_obj=dynamic_cast<TableObject *>(object);

					if(tab_obj)
					{
						if(tab_obj->isAddedByRelationship())
						{
							throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
															.arg(tab_obj->getName(true), tab_obj->getTypeName()),
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
	action_associations->setEnabled(value);
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
	bool tab_or_view=false, is_graph_obj = false, accepts_owner=false, accepts_schema=false, movable_to_scene=false;

	if(object)
		sel_objs.push_back(object);
	else
		sel_objs = selected_objects;

	/* Determining if one or more selected objects accepts schema, owner or are table/views,
	 * this is done to correctly show the actions to the user */
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

		/* Table/View/ForeignTable and Textbox are the only types with an independently movable
		 * scene -- the exact restriction by segmentation mode is enforced in moveToScene() itself */
		if(!movable_to_scene)
			movable_to_scene = (tab_or_view || obj_type == ObjectType::Textbox);

		if(tab_or_view && accepts_owner && accepts_schema && movable_to_scene)
			break;
	}

	//Meaningless with a single scene (NoSegmentation always has exactly one)
	movable_to_scene = movable_to_scene && scenes.size() > 1;

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
							act = new QAction(obj->getName(), menus[i]);
							act->setIcon(GuiUtilsNs::getIcon(types[i]));

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

		/* Configuring the "Move to scene" submenu. Entries are scene indexes, not
		 * BaseObject instances, so it's built separately from the schemas/owners/tags loop above */
		if(movable_to_scene)
		{
			scenes_menu.clear();

			QStringList scene_names = getSceneNames();
			BaseGraphicObject *single_sel_graph = sel_objs.size() == 1 ?
																						 dynamic_cast<BaseGraphicObject *>(sel_objs[0]) : nullptr;

			for(int i = 0; i < scene_names.size(); i++)
			{
				act = new QAction(scene_names.at(i), &scenes_menu);

				act->setCheckable(sel_objs.size() == 1);
				act->setChecked(single_sel_graph && getSceneForObject(single_sel_graph) == scenes.at(i));
				act->setEnabled(!act->isChecked());
				act->setIcon(GuiUtilsNs::getIcon("design"));
				act->setData(i);

				connect(act, &QAction::triggered, this, &ModelWidget::moveToScene);
				scenes_menu.addAction(act);
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

		if(movable_to_scene)
			quick_actions_menu.addAction(action_moveto_scene);

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

		//Last items: create tag/layer from schema(s) -- singular for one schema, batch for the database itself
		if(object && obj_type == ObjectType::Schema)
		{
			quick_actions_menu.addSeparator();
			quick_actions_menu.addAction(action_create_tag_from_schema);
			quick_actions_menu.addAction(action_create_layer_from_schema);
			action_create_tag_from_schema->setData(QVariant::fromValue<void *>(object));
			action_create_layer_from_schema->setData(QVariant::fromValue<void *>(object));
		}
		else if(object == dynamic_cast<BaseObject *>(db_model) &&
						!db_model->getObjectList(ObjectType::Schema)->empty())
		{
			quick_actions_menu.addSeparator();
			quick_actions_menu.addAction(action_create_tags_all_schemas);
			quick_actions_menu.addAction(action_create_layers_all_schemas);
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
	int op_id = -1, op_curr_idx = op_list->getCurrentIndex();

	try
	{
		op_list->startOperationChain();

		for(auto &obj : *db_model->getObjectList(ObjectType::Schema))
		{
			schema = dynamic_cast<Schema *>(obj);

			if(schema && schema->isRectVisible() != visible)
			{
				op_id = op_list->registerObject(schema, Operation::ObjModified, -1);
				schema->setRectVisible(visible);
				schema->setModified(true);
			}
		}

		op_list->finishOperationChain();
	}
	catch(Exception &e)
	{
		if(op_id >= 0 && op_id > op_curr_idx)
			op_list->removeLastOperation();

		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
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
				submenu->setIcon(GuiUtilsNs::getIcon(BaseObject::getSchemaName(ObjectType::Constraint) + str_aux));
				submenu->setTitle(constr->getName());

				action=new QAction(dynamic_cast<QObject *>(submenu));
				action->setIcon(GuiUtilsNs::getIcon("edit"));
				action->setText(tr("Properties"));
				action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
				connect(action, &QAction::triggered, this, &ModelWidget::editObject);
				submenu->addAction(action);

				action=new QAction(dynamic_cast<QObject *>(submenu));
				action->setIcon(GuiUtilsNs::getIcon("sourcecode"));
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
							action->setIcon(GuiUtilsNs::getIcon("unprotect"));
							action->setText(tr("Unprotect"));
						}
						else
						{
							action->setIcon(GuiUtilsNs::getIcon("protect"));
							action->setText(tr("Protect"));
						}
					}

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(GuiUtilsNs::getIcon("delete"));
					action->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(constr)));
					action->setText(tr("Delete"));
					submenu->addAction(action);

					connect(action, &QAction::triggered, this, [this](){
						removeObjects(false);
					});

					action=new QAction(dynamic_cast<QObject *>(submenu));
					action->setIcon(GuiUtilsNs::getIcon("delcascade"));
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
			submenu->setIcon(GuiUtilsNs::getIcon(BaseObject::getSchemaName(ObjectType::Constraint)));

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

				action = jump_to_tab_menu.addAction(GuiUtilsNs::getIcon(rel->getTable(BaseRelationship::SrcTable)->getObjectType()),
																						rel->getTable(BaseRelationship::SrcTable)->getSignature(), this, &ModelWidget::jumpToTable);
				action->setData(QVariant::fromValue<void *>(reinterpret_cast<void *>(rel->getTable(BaseRelationship::SrcTable))));

				action = jump_to_tab_menu.addAction(GuiUtilsNs::getIcon(rel->getTable(BaseRelationship::DstTable)->getObjectType()),
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
	action_associations->setData(QVariant::fromValue<void *>(obj));

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
		popup_menu.addAction(action_associations);
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
	action_associations->setData(QVariant::fromValue<void *>(dynamic_cast<BaseObject *>(db_model)));

	popup_menu.addAction(action_edit);

	popup_menu.addSeparator();
	popup_menu.addAction(action_source_code);
	popup_menu.addAction(action_associations);

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

		/* Mirrors the pre-selected-tables shortcut addNewObject() already accepts
		 * (modelwidget.cpp:1488-1494): exactly two tables selected together, from any scene(s),
		 * is also a valid trigger for "New > Relationship" -- same as selecting a single table
		 * already shows the relationship types in its own "New" menu */
		if(objects.size() == 2 &&
				objects[0]->getObjectType() == ObjectType::Table &&
				objects[1]->getObjectType() == ObjectType::Table &&
				!objects[0]->isProtected() && !objects[1]->isProtected())
		{
			new_object_menu.addAction(actions_new_objects[ObjectType::Relationship]);
			popup_menu.insertAction(action_quick_actions, action_new_object);
		}

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

		/* Toggling schema rectangles is available only when the model is not
		 * segmented in multiple scenes */
		if(db_model->getSegmentationMode() == DatabaseModel::NoSegmentation &&
			 (objects.empty() ||
			 (objects.size() == 1 && objects[0]->getObjectType() == ObjectType::Database)))
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

ObjectsScene *ModelWidget::getScene()
{
	return scene;
}

ObjectsScene *ModelWidget::getScene(int idx)
{
	if(idx < 0 || idx >= scenes.size())
		return nullptr;

	return scenes.at(idx);
}

QList<ObjectsScene *> ModelWidget::getScenes()
{
	return scenes;
}

int ModelWidget::getSceneCount()
{
	return static_cast<int>(scenes.size());
}

ObjectsScene *ModelWidget::getSceneForObject(BaseObject *object)
{
	BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(object);

	if(!graph_obj)
		return nullptr;

	BaseObjectView *obj_view = dynamic_cast<BaseObjectView *>(graph_obj->getOverlyingObject());

	if(!obj_view)
		return nullptr;

	return dynamic_cast<ObjectsScene *>(obj_view->scene());
}

std::vector<BaseObject *> ModelWidget::getSceneObjects(ObjectType type, ObjectsScene *target_scene, BaseObject *parent_obj)
{
	std::vector<BaseObject *> src_objs = parent_obj ? db_model->getObjects(type, parent_obj) : *db_model->getObjectList(type),
														 result;

	for(auto &obj : src_objs)
	{
		if(getSceneForObject(obj) == target_scene)
			result.push_back(obj);
	}

	return result;
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

			// Cross-scene relationships have no graphical item to highlight
			if(!obj_view)
				return;

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

		// Cross-scene relationships have no line to break
		if(!rel_view) return;

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

void ModelWidget::rearrangeSchemasInGrid(bool current_scene_only, unsigned tabs_per_row, unsigned sch_per_row, QPointF origin, double obj_spacing)
{
	if(current_scene_only)
		rearrangeSchemasInGrid(scene, tabs_per_row, sch_per_row, origin, obj_spacing);
	else
	{
		for(auto &scn : scenes)
			rearrangeSchemasInGrid(scn, tabs_per_row, sch_per_row, origin, obj_spacing);
	}

	db_model->setObjectsModified({ ObjectType::Table, ObjectType::ForeignTable, ObjectType::View,
																 ObjectType::BaseRelationship, ObjectType::Relationship});

	if(current_scene_only)
		adjustSceneRect(false);
	else
		adjustAllScenesRects(false);
}

void ModelWidget::rearrangeSchemasInGrid(ObjectsScene *target_scene, unsigned tabs_per_row, unsigned sch_per_row, QPointF origin, double obj_spacing)
{
	/* Outside of NoSegmentation there's no SchemaView to grid in the first place (see
	 * ModelWidget::handleObjectAddition()) -- SegBySchema's scene already *is* the schema (a box
	 * around it would just mirror the scene itself) and SegByThreshold mixes tables from several
	 * schemas with no schema grouping at all. Either way, fall back to a flat grid of the scene's
	 * own tables, schema boundaries ignored entirely */
	if(db_model->getSegmentationMode() != DatabaseModel::NoSegmentation)
	{
		std::vector<BaseObject *> tables = getSceneObjects(ObjectType::Table, target_scene);
		std::vector<BaseObject *> ftables = getSceneObjects(ObjectType::ForeignTable, target_scene);
		std::vector<BaseObject *> views = getSceneObjects(ObjectType::View, target_scene);
		tables.insert(tables.end(), ftables.begin(), ftables.end());
		tables.insert(tables.end(), views.begin(), views.end());

		rearrangeTablesInGrid(tables, tabs_per_row == 0 ? 5 : tabs_per_row, origin, obj_spacing);
		return;
	}

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
			std::vector<BaseObject *> tables = db_model->getObjects(ObjectType::Table, schema);
			std::vector<BaseObject *> ftables = db_model->getObjects(ObjectType::ForeignTable, schema);
			std::vector<BaseObject *> views = db_model->getObjects(ObjectType::View, schema);
			tables.insert(tables.end(), ftables.begin(), ftables.end());
			tables.insert(tables.end(), views.begin(), views.end());

			rearrangeTablesInGrid(tables, tabs_per_row, QPointF(x,y), obj_spacing);
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
}

void ModelWidget::rearrangeTablesInGrid(std::vector<BaseObject *> tables, unsigned tabs_per_row, QPointF origin, double obj_spacing)
{
	std::vector<BaseObject *>::iterator itr;
	BaseTableView *tab_view=nullptr;
	BaseTable *base_tab=nullptr;
	unsigned tab_id=0;
	double max_y=-1, x=origin.x(), y=origin.y(), cy=0;

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

void ModelWidget::swapObjectsIds()
{
	BaseForm parent_form(this);
	SwapObjectsIdsWidget *swap_ids_wgt=new SwapObjectsIdsWidget;

	swap_ids_wgt->setModel(this->getDatabaseModel());

	if(!selected_objects.empty())
		swap_ids_wgt->setSelectedObjects(selected_objects[0], selected_objects.size() == 2 ? selected_objects[1] : nullptr);

	parent_form.setMainWidget(swap_ids_wgt, &SwapObjectsIdsWidget::swapObjectsIds);
	parent_form.setButtonConfiguration(Messagebox::OkCancelButtons);

	parent_form.accept_btn->setEnabled(false);
	parent_form.accept_btn->setIcon(GuiUtilsNs::getIcon("swapobjs"));
	parent_form.accept_btn->setText(tr("&Swap ids"));

	parent_form.reject_btn->setIcon(GuiUtilsNs::getIcon("close1"));
	parent_form.reject_btn->setText(tr("&Close"));

	connect(swap_ids_wgt, &SwapObjectsIdsWidget::s_objectsIdsSwapped, this, [this](){
		op_list->removeOperations();
		setModified(true);
		emit s_objectManipulated();
	});

	connect(swap_ids_wgt, &SwapObjectsIdsWidget::s_objectsIdsSwapReady,
					parent_form.accept_btn, &QPushButton::setEnabled);

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

void ModelWidget::updateModelSceneRectInfo()
{
	QRectF united_rect;
	QMap<int, QRectF> per_scene_rects;

	for(int idx = 0; idx < scenes.size(); idx++)
	{
		united_rect = united_rect.united(scenes.at(idx)->sceneRect());
		per_scene_rects[idx] = scenes.at(idx)->sceneRect();
	}

	db_model->setSceneRect(united_rect);
	db_model->setSceneRects(per_scene_rects);
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

void ModelWidget::rearrangeTablesHierarchically(bool current_scene_only)
{
	if(current_scene_only)
		rearrangeTablesHierarchically(scene);
	else
	{
		for(auto &scn : scenes)
			rearrangeTablesHierarchically(scn);
	}

	if(current_scene_only)
		adjustSceneRect(false);
	else
		adjustAllScenesRects(false);

	viewport->updateScene({ scene->sceneRect() });
}

void ModelWidget::rearrangeTablesHierarchically(ObjectsScene *target_scene)
{
	std::vector<BaseObject *> objects = getSceneObjects(ObjectType::Table, target_scene);
	std::vector<BaseObject *> scene_views = getSceneObjects(ObjectType::View, target_scene);
	objects.insert(objects.end(), scene_views.begin(), scene_views.end());

	BaseGraphicObject *graph_obj = nullptr;
	BaseTableView *tab_view = nullptr, *root = nullptr;
	int num_rels = 0;

	target_scene->clearSelection();

	//We determine the root by searching the table/view which contains the more amount of relationships connected
	for(auto &obj : objects)
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

	if(!root)
	{
		//This is a fallback arrangement when the scene does not have relationships
		rearrangeSchemasInGrid(target_scene, 0, 0, QPointF(50, 50), 75);
		return;
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
		items_rect = rearrangeTablesHierarchically(target_scene, root, evaluated_tabs);
		max_w = items_rect.width();

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
				items_rect = rearrangeTablesHierarchically(target_scene, root, evaluated_tabs);
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

		std::vector<BaseObject *> scene_textboxes = getSceneObjects(ObjectType::Textbox, target_scene);
		objects.insert(objects.end(), scene_textboxes.begin(), scene_textboxes.end());

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

		std::vector<BaseObject *> scene_rels = getSceneObjects(ObjectType::Relationship, target_scene);
		std::vector<BaseObject *> scene_base_rels = getSceneObjects(ObjectType::BaseRelationship, target_scene);
		objects.insert(objects.end(), scene_rels.begin(), scene_rels.end());
		objects.insert(objects.end(), scene_base_rels.begin(), scene_base_rels.end());

		for(auto &obj : objects)
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
}

QRectF ModelWidget::rearrangeTablesHierarchically(ObjectsScene *target_scene, BaseTableView *root, std::vector<BaseObject *> &evaluated_tabs)
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

			/* A cross-scene partner has no RelationshipView here (cut-off marker instead) -- treat it
			 * as not connected, this scene's hierarchy shouldn't reach into another scene's tables */
			if(curr_tab && getSceneForObject(curr_tab) != target_scene)
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
	if(!schema) return;

	std::vector<BaseObject *> tables = db_model->getObjects(ObjectType::Table, schema);
	std::vector<BaseObject *> ftables = db_model->getObjects(ObjectType::ForeignTable, schema);
	std::vector<BaseObject *> views = db_model->getObjects(ObjectType::View, schema);

	tables.insert(tables.end(), views.begin(), views.end());
	tables.insert(tables.end(), ftables.begin(), ftables.end());

	if(!tables.empty())
	{
		arrangeTablesScattered(tables, start);
		schema->setRectVisible(true);
		schema->setModified(true);
	}
}

void ModelWidget::arrangeTablesScattered(std::vector<BaseObject *> tables, QPointF start)
{
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
	}
}

void ModelWidget::rearrangeTablesInSchemas(bool current_scene_only)
{
	if(current_scene_only)
		rearrangeTablesInSchemas(scene);
	else
	{
		for(auto &scn : scenes)
			rearrangeTablesInSchemas(scn);
	}

	db_model->setObjectsModified({ ObjectType::Table, ObjectType::View, ObjectType::ForeignTable,
																 ObjectType::Schema, ObjectType::Relationship, ObjectType::BaseRelationship });

	if(current_scene_only)
		adjustSceneRect(false);
	else
		adjustAllScenesRects(false);

	viewport->updateScene({ scene->sceneRect() });
}

void ModelWidget::rearrangeTablesInSchemas(ObjectsScene *target_scene)
{
	/* Outside of NoSegmentation there's no SchemaView to treat as its own collidable region (see
	 * ModelWidget::handleObjectAddition()) -- scatter the scene's tables directly instead, as one
	 * flat group, schema boundaries ignored entirely */
	if(db_model->getSegmentationMode() != DatabaseModel::NoSegmentation)
	{
		std::vector<BaseObject *> tables = getSceneObjects(ObjectType::Table, target_scene);
		std::vector<BaseObject *> ftables = getSceneObjects(ObjectType::ForeignTable, target_scene);
		std::vector<BaseObject *> views = getSceneObjects(ObjectType::View, target_scene);
		tables.insert(tables.end(), ftables.begin(), ftables.end());
		tables.insert(tables.end(), views.begin(), views.end());

		arrangeTablesScattered(tables, QPointF(0, 0));

		std::vector<BaseObject *> rels = getSceneObjects(ObjectType::Relationship, target_scene);
		std::vector<BaseObject *> base_rels = getSceneObjects(ObjectType::BaseRelationship, target_scene);
		rels.insert(rels.end(), base_rels.begin(), base_rels.end());

		for(auto &obj : rels)
		{
			BaseRelationship *base_rel = dynamic_cast<BaseRelationship *>(obj);
			base_rel->setPoints({});
			base_rel->resetLabelsDistance();
		}

		return;
	}

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
