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

/**
\ingroup libgui
\class ModelWidget
\brief Implements the widget that permits the direct user interation over the database model.
*/

#ifndef MODEL_WIDGET_H
#define MODEL_WIDGET_H

#include <QMenu>
#include "databasemodel.h"
#include "operationlist.h"
#include "messagebox.h"
#include "objectsscene.h"
#include "newobjectoverlaywidget.h"
#include "layerswidget.h"

class PgModelerGuiPlugin;
class CodePreviewWidget;
class BaseObjectWidget;

class __libgui ModelWidget: public QWidget {
	Q_OBJECT

	private:
		static QList<const PgModelerGuiPlugin *> plugins;

		enum RelBreakMode {
			//Break vertically the line in one 90° angle
			BreakVertNinetyDegrees,

			//Break horizontally the line in one 90° angle
			BreakHorizNinetyDegrees,

			//Break vertically the line in two 90° angles
			BreakVert2NinetyDegrees,

			//Break horizontally the line in two 90° angles
			BreakHoriz2NinetyDegrees
		};

		//! \brief Constants used to control the object stacking method
		static constexpr int BringToFront = 1,

		SendToBack = -1;

		XmlParser *xmlparser;

		NewObjectOverlayWidget *new_obj_overlay_wgt;

		LayersWidget *layers_wgt;

		//! \brief Current zoom aplied to the scene
		double current_zoom;

		//! \brief Indicates if the model was modified by some operation
		bool modified,

		//! \brief Indicates if the panning mode was activated via event filter (see eventFilter())
		panning_mode,

		//! \brief Stores the current state of grid visibility prior a panning move
		curr_show_grid,

		//! \brief Stores the current state of page delimiters visibility prior a panning move
		curr_show_delim,

		//! \brief Indicates if the canvas panning move is being made via mouse wheel
		wheel_move,

		//! \brief Indicates if the scene is being moved by either panning move or via mouse wheel
		scene_moving,

		//! \brief Indicates if added graphical object must be blinked/highlighted (see handleObjectAddition())
		blink_new_objs;

		/*! \brief Indicates if the cut operation is currently activated. This flag modifies
		the way the methods copyObjects() and removeObject() works. */
		static bool cut_operation,

		//! \brief Indicates if the last position and zoom must be saved/restored
		save_restore_pos,

		//! \brief Indicates that graphical objects like table, view and textboxes can be created without click canvas (direclty from their editing form)
		simple_obj_creation,

		disable_render_smooth;

		//! \brief Indicates if the minimum object opacity used when appliyng fade out to objects
		static double min_object_opacity;

		/*! \brief Stores the model that generates the copy/cut operation. This model is updated
		from the destination model whenever a past/cut operation is done. */
		static ModelWidget *src_model;

		//! \brief Copied object on the source model
		static std::vector<BaseObject *> copied_objects,

		//! \brief Stores the cutted object on source model (only when executing cut command)
		cut_objects;

		//! \brief Frame that indicates if the model is protected
		QFrame *protected_model_frm;

		//! \brief Scenes managed by this widget, one per segmentation bucket
		QList<ObjectsScene *> scenes;

		//! \brief Index of the currently active scene within scenes (the one attached to viewport)
		int active_scene_idx;

		//! \brief Convenience pointer to the currently active scene (always equal to scenes[active_scene_idx])
		ObjectsScene *scene;

		//! \brief Maps each schema to the scene that holds it and its children, in schema-segmentation mode
		QHash<Schema *, ObjectsScene *> schema_scenes;

		/*! \brief Running count of Table/View/ForeignTable insertions in threshold-segmentation mode.
		 * Not itself persisted -- reseeded from the loaded objects' scene indexes on loadModel() */
		unsigned threshold_obj_count;

		/*! \brief Minimum fraction of a model's Table/View/ForeignTable count concentrated in a
		 * single schema for getAlternateSegMode() to suggest SegByThreshold over SegBySchema
		 * -- provisional value, to be calibrated later */
		static constexpr double ConcentrationSuggestThreshold = 0.8;

		//! \brief Minimum number of relationships in a model file for getAlternateSegMode() to suggest hiding them
		static constexpr unsigned HideRelsSuggestThreshold = 2000;

		//! \brief Result of a lightweight, pre-load scan of a model file for getAlternateSegMode()
		struct AlternateSegMode {
			//! \brief The suggested mode: SegBySchema or SegByThreshold, NoSegmentation when isValid() is false
			DatabaseModel::SegmentationMode mode = DatabaseModel::NoSegmentation;

			//! \brief Total Table/View/ForeignTable count found in the file
			unsigned total_objs = 0;

			//! \brief Table/View/ForeignTable count belonging to the single most populated schema
			unsigned max_schema_objs = 0;

			//! \brief Name of that most populated schema
			QString max_schema_name;

			//! \brief Total relationship count (relationships and basic relationships) found in the file
			unsigned total_rels = 0;

			//! \brief True when the user accepted hiding the relationships (see checkAlternateSegmentation())
			bool hide_rels = false;

			//! \brief True when the relationship count is large enough for a hide relationships suggestion
			bool suggestsHideRels() const
			{
				return total_rels >= HideRelsSuggestThreshold;
			}

			//! \brief True when this instance carries an actual, checked alternate segmentation mode
			bool isValid() const
			{
				return mode != DatabaseModel::NoSegmentation &&
							 total_objs > 0 && max_schema_objs > 0 && max_schema_objs <= total_objs;
			}
		};

		/*! \brief Scans filename's XML directly (no DTD validation, no object instantiation) tallying
		 * Table/View/ForeignTable counts per referenced schema, to detect upfront -- before the real,
		 * much more expensive loadModel() call -- whether SegBySchema or SegByThreshold is the better
		 * fit for this model. Only runs the detection at all when the file has no segmentation mode
		 * of its own saved yet (attribs[Attributes::SegMode] empty) -- a model that already picked a
		 * mode, explicitly or via a previous suggestion, is never second-guessed. Never modifies the
		 * file or any DatabaseModel instance */
		static AlternateSegMode getAlternateSegMode(const QString &filename);

		/*! \brief Runs getAlternateSegMode() against filename and prompts the user to confirm what it
		 * suggests: first the alternate segmentation mode (when valid), then hiding the relationships
		 * (when the file has many). Returns the confirmed ones: mode (invalid when there's nothing to
		 * suggest or the user declines) and hide_rels */
		AlternateSegMode checkAlternateSegmentation(const QString &filename);

		//! \brief Asks the user to confirm loading the model with the segmentation mode alt_seg_mode suggests
		bool confirmAlternateSegMode(const AlternateSegMode &alt_seg_mode);

		/*! \brief Lightweight, pre-load scan of filename's root attributes (no DTD validation, no
		 * object instantiation) to tell whether its saved scene distribution is still consistent
		 * with its own seg-threshold. Compares seg-obj-count (the Table/View/ForeignTable count that
		 * produced the <scenes> below when the file was saved) against seg-threshold: replaying
		 * seg-obj-count/seg-threshold+1 must reproduce the same scene count the file actually has, or
		 * the threshold was changed (setPendingSceneThreshold()) after that save and the per-object
		 * scene indices are stale. Only meaningful when the file's own SegMode is already
		 * SegByThreshold -- returns false for anything else, including files predating seg-obj-count
		 * (nothing to compare against, so nothing is second-guessed) */
		static bool needsThresholdRebucketing(const QString &filename);

		//! \brief Manages the objects scene
		QGraphicsView *viewport;

		//! \brief Model's general context menu
		QMenu popup_menu,

		//! \brief Stores the actions related to new objects creation
		new_object_menu,

		//! \brief Stores the quick actions
		quick_actions_menu,

		//! \brief Stores the schemas used by the "move to schema" operation
		schemas_menu,

		//! \brief Stores the scenes used by the "move to scene" operation (subtask 4.2)
		scenes_menu,

		//! \brief Stores the role names used by the "change owner" operation
		owners_menu,

		//! \brief Stores the tags used by the "set tag" operation
		tags_menu,

		break_rel_menu,

		//! \brief Stores the actions that change the custom color of the relationships
		rel_color_menu,

		fade_menu,

		fade_in_all_menu,

		fade_out_all_menu,

		fade_rels_menu,

		fade_peer_tables_menu,

		fade_both_objs_menu,

		toggle_attrs_menu,

		pagination_menu,

		select_all_menu,

		jump_to_tab_menu,

		toggle_sch_rects_menu,

		database_category_menu,

		schema_category_menu,

		stacking_menu;

		//! \brief Stores the selected object on the scene
		std::vector<BaseObject *> selected_objects;

		//! \brief Type of the object to be inserted on the model
		ObjectType new_obj_type;

		//! \brief Operation list that stores the modifications executed over the model
		OperationList *op_list;

		//! \brief Database model handle by the ModelWidget class. All operations are made over this attribute
		DatabaseModel *db_model;

		//! \brief Stores the loaded database model filename
		QString filename,

		//! \brief Stores the temporary database model filename
		tmp_filename;

		//! \brief This label shows the user the current applied zoom
		QLabel *zoom_info_lbl,

		//! \brief This label shows a small portion of the canvas in normal zoom at the current cursor position
		*magnifier_area_lbl;

		//! \brief This rect stores the current size of the magnifier area in viewport that is drawn in the magnifier area label
		QRect magnifier_rect;

		//! \brief This timer controls the interval the zoom label is visible
		QTimer zoom_info_timer,

		/*! \brief This timer controls the interval that the background of the scene is hidden while
		 *  using the mouse wheel to zoom or move the scene */
		wheel_timer;

		//! \brief Stores the installed plugins actions to be used in the model context menu
		QList<QAction *> plugins_actions;

		//! \brief Opens a editing form for objects at database level
		template<class Class, class WidgetClass>
		int openEditingForm(BaseObject *object);

		/*! \brief Opens a editing form for objects that can have a parent other than database, e.g., schema level objects,
				table child object or relationship attributes & constraints */
		template<class Class, class WidgetClass, class ParentClass>
		int openEditingForm(BaseObject *object, BaseObject *parent_obj);

		//! \brief Opens a editing form for objects that can have a position in the canvas area
		template<class Class, class WidgetClass, class ParentClass>
		int openEditingForm(BaseObject *object, BaseObject *parent_obj, const QPointF &pos);

		//! \brief Opens a editing form specific for tables and foreign tables
		int openTableEditingForm(ObjectType tab_type, PhysicalTable *object, Schema *schema, const QPointF &pos);

		//! \brief Configures the submenu related to the installed plugins actions
		void configurePluginsActions();

		//! \brief Configures the submenu related to the object
		void configureQuickMenu(BaseObject *object);

		//! \brief Configures the submenu related to fade in/out operations
		void configureFadeMenu();

		//! \brief Configures the constraints submenu related to a column
		void configureConstraintsMenu(TableObject *tab_obj);

		//! \brief Configures the basic actions in the popup menu for a single object
		void configureBasicActions(BaseObject *obj);

		//! \brief Configures the basic actions in the popup menu for the database object
		void configureDatabaseActions();

		/*! \brief Creates a Tag (as_layer=false) or a layer (as_layer=true) named after each schema in
		 * schemas, associating that schema's Table/View/ForeignTable children with it. An empty
		 * schemas means every schema in the model, and additionally asks for confirmation first
		 * (the batch actions in the canvas' popup menu call it this way). If any table already has a
		 * tag, asks once whether the new one should replace it -- layers have no such conflict, a
		 * table can freely belong to more than one. A schema whose children actually got the new
		 * tag/layer has its rectangle hidden afterwards (undoable, like the rest of this method),
		 * now redundant as a visual grouping cue */
		void createTagOrLayerFromSchemas(const std::vector<Schema *> &schemas, bool as_layer);

		//! \brief Fades in our out the object types held by the specified action
		void fadeObjects(QAction *action, bool fade_in);

		void breakRelationshipLine(BaseRelationship *rel, RelBreakMode break_type);

		//! \brief Per-scene worker for the public rearrangeTablesHierarchically(bool)
		void rearrangeTablesHierarchically(ObjectsScene *target_scene);

		/*! \brief Arrange tables starting from a specified root in a hierarchical way, considering only
		relationships whose other table is also in target_scene (a cross-scene/cut-off partner is treated
		as not connected) -- for a certain table its child (or related) tables are places aside from left
		to right and top to bottom. This method returns the bounding rect of the items after the rearrangement */
		QRectF rearrangeTablesHierarchically(ObjectsScene *target_scene, BaseTableView *root, std::vector<BaseObject *> &evaluated_tabs);

		/*! \brief Arrange tables inside the provided schema randomly (scattered). An start point should
		 * be provided. The method will avoid to put two or more tables in the same position causing
		 * overlaping. This method causes the schema rectangle to be enabled. */
		void rearrangeTablesInSchema(Schema *schema, QPointF start);

		/*! \brief Core of rearrangeTablesInSchema(), minus the schema-specific rectangle handling --
		 * scatters the given tables (already gathered by the caller) randomly around start, avoiding
		 * collisions between them. Reused directly for a flat, schema-less scene arrangement */
		void arrangeTablesScattered(std::vector<BaseObject *> tables, QPointF start);

		void updateMagnifierArea();

		/*! \brief Move the selected objects in the Z coordenate either to bottom or top.
		 * The direction is defined by the constants BringToTop or SendToBottom. */
		void moveObjectsInZStack(int direction);

		//! \brief Applies the layer settings from the internal database model to the scene object
		void updateSceneLayers();

		//! \brief Applies the layer settings from the internal database model to a single scene
		void applySceneLayers(ObjectsScene *scn);

		/*! \brief Creates a new scene, appends it to scenes and returns it. When auto_activate is true (the
		default) it activates the new scene if none is active yet */
		ObjectsScene *addScene(bool auto_activate = true);
		ObjectsScene *addScene(Schema *schema);

		/*! \brief Removes scn from scenes and destroys it. If scn is the active scene, activates
		 * another one first (the first remaining scene, or none if scn was the last one), so
		 * viewport never ends up pointing at a freed scene */
		void removeScene(ObjectsScene *scn);

		//! \brief Returns whichever of rel's two tables isn't tab (the other side of the relationship)
		BaseTable *getRelationshipPartner(BaseRelationship *rel, BaseTable *tab);

		//! \brief Builds the graphical item representing graph_obj (Table, ForeignTable, View or Textbox), or nullptr for any other type
		QGraphicsItem *createGraphicItem(BaseGraphicObject *graph_obj);

		/*! \brief Creates rel's RelationshipView if both its tables currently share a scene and the
		 * model doesn't hide its relationships, or registers it in the relationship marker of both
		 * tables otherwise -- same rule as handleObjectAddition()'s Relationship case, reading each
		 * table's scene fresh every time it's called. Used to re-route a relationship after one of
		 * its tables was relocated */
		void routeRelationship(BaseRelationship *rel);

		/*! \brief Asks the user to save and reload the model so a pending change takes effect, unless
		 * the user opted out of this question. Emits s_modelSaveReloadRequested() when accepted */
		void askSaveAndReload(const QString &message);

		/*! \brief Reacts to OperationList::s_operationExecuted, connected once in the constructor --
		 * covers undo/redo of Operation::ObjModified, which restores the object's previous schema or
		 * scene index in place (see OperationList::executeOperation()) without ever calling
		 * moveToSchema()/moveToScene()/relocateEditedObject(), so nothing else notices the object may
		 * now belong in a different scene. Two independent resync rules, since a Table/View/
		 * ForeignTable's scene can be driven by either attribute depending on segmentation mode: its
		 * schema (schema-segmentation, existing since before subtask 4.2) or its own scene index
		 * (threshold-segmentation, and Textbox in either segmented mode, both added by 4.2). A no-op
		 * whenever neither applies, or the view already agrees with the restored state */
		void handleObjectSchemaChanged(BaseObject *object, Operation::OperType op_type);

		/*! \brief Moves obj_wgt's edited object to its new schema's scene, if the schema selector in
		 * the object's editing form actually changed it (see BaseObjectWidget::prev_schema) --
		 * covers the same schema-change-implies-scene-change case as moveToSchema(), but for the
		 * schema selector embedded in each object's own editing form instead of the popup menu */
		void relocateEditedObject(BaseObjectWidget *obj_wgt);

	protected:
		QAction *action_source_code,
		*action_edit,
		*action_protect,
		*action_unprotect,
		*action_remove,
		*action_cascade_del,
		*action_select_all,
		*action_convert_relnn,
		*action_convert_rel1n,
		*action_copy,
		*action_paste,
		*action_cut,
		*action_associations,
		*action_new_object,
		*action_rename,
		*action_moveto_schema,
		*action_moveto_scene,
		*action_edit_perms,
		*action_change_owner,
		*action_quick_actions,
		*action_sel_sch_children,
		*action_sel_table_rels,
		*action_sel_tagged_tabs,
		*action_create_tag_from_schema,
		*action_create_layer_from_schema,
		*action_create_tags_all_schemas,
		*action_create_layers_all_schemas,
		*action_select_object,
		*action_parent_rel,
		*action_append_sql,
		*action_create_seq_col,
		*action_conv_int_serial,
		*action_break_rel_line,
		*action_remove_rel_points,
		*action_rel_color,
		*action_set_tag,
		*action_set_layer,
		*action_disable_sql,
		*action_enable_sql,
		*action_duplicate,
		*action_fade,
		*action_fade_in,
		*action_fade_out,
		*action_fade_rels,
		*action_fade_rels_in,
		*action_fade_rels_out,
		*action_fade_peer_tables,
		*action_fade_peer_tables_in,
		*action_fade_peer_tables_out,
		*action_fade_tabs_rels,
		*action_fade_tabs_rels_in,
		*action_fade_tabs_rels_out,
		*action_pagination,
		*action_collapse_mode,
		*action_collapse_ext_attribs,
		*action_collpase_all_attribs,
		*action_no_collapse_attribs,
		*action_edit_creation_order,
		*action_jump_to_table,
		*action_schemas_rects,
		*action_show_schemas_rects,
		*action_hide_schemas_rects,
		*action_edit_data,
		*action_database_category,
		*action_schema_category,
		*action_bring_to_front,
		*action_send_to_back,
		*action_stacking;

		//! \brief Actions used to create new objects on the model
		std::map<ObjectType, QAction *> actions_new_objects;

		//! \brief Stores the relationship types menu
		QMenu *rels_menu,

		copy_menu,

		cut_menu;

		void resizeEvent(QResizeEvent *) override;
		void mousePressEvent(QMouseEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;
		void hideEvent(QHideEvent *) override;

		//! \brief Captures and handles the QWeelEvent raised on the viewport scrollbars
		bool eventFilter(QObject *object, QEvent *event) override;

		//! \brief Cancel the creation of a new object (only for graphical objects)
		void cancelObjectAddition();

		//! \brief Disables the model actions when some new object action is active
		void enableModelActions(bool value);

		/*! \brief Reorganizes the given tables/views/foreign tables in a grid form. The parameters are:
		 the objects to arrange (already gathered by the caller -- a schema's children or a whole
		 scene's, flat), an origin point, number of tables per row and a object spacing */
		void rearrangeTablesInGrid(std::vector<BaseObject *> tables, unsigned tabs_per_row, QPointF origin, double obj_spacing);

		//! \brief Per-scene worker for the public rearrangeSchemasInGrid(bool, ...) -- see there for the segmented-mode fallback
		void rearrangeSchemasInGrid(ObjectsScene *target_scene, unsigned tabs_per_row, unsigned sch_per_row,
																QPointF origin, double obj_spacing);

		//! \brief Per-scene worker for the public rearrangeTablesInSchemas(bool) -- see there for the segmented-mode fallback
		void rearrangeTablesInSchemas(ObjectsScene *target_scene);

		/*! \brief Filters the given type's objects (all of them, or just parent_obj's children when
		 * provided) down to the ones whose graphical item currently lives in target_scene */
		std::vector<BaseObject *> getSceneObjects(ObjectType type, ObjectsScene *target_scene, BaseObject *parent_obj = nullptr);

		void fadeObjects(const std::vector<BaseObject *> &objects, bool fade_in);

		void setAllCollapseMode(BaseTable::CollapseMode mode);

	public:
		static constexpr double MinimumZoom = ObjectsScene::MinScaleFactor,
		MaximumZoom = ObjectsScene::MaxScaleFactor,
		ZoomIncrement = 0.050000;

		static constexpr char SaveDisabled[] { "save-disabled" },
		ModelImported[] { "model-imported" },
		AltFilename[] { "alt-filename" };

		ModelWidget(QWidget *parent = nullptr);
		~ModelWidget() override;

		//! \brief Creates a BaseForm instance and insert the widget into it. A custom configuration for dialog buttons can be passed
		template<class WidgetClass>
		int openEditingForm(WidgetClass *widget, Messagebox::ButtonsId button_conf = Messagebox::OkCancelButtons);

		//! \brief Aligns the objects of every scene of the model to the grid
		void alignObjectsToGrid();

		/*! \brief Configures the scene aligning the object to the grid and resizing the scene
		 * rect when some object is out of bound. The parameter use_model_rect, when true, uses
		 * the rect stored in the database model (retrieved from the dbm file loading) to resize
		 * the scene rect. The parameter expand_only, when true, indicates that the method must
		 * only expand the scene rect, If the recalculated rect is smaller the the current
		 * scene rect that no operation is done. */
		void adjustSceneRect(bool use_model_rect, bool expand_only = false);

		/*! \brief When use_model_rect is true (the default), applies adjustSceneRect(true) to the
		 * active scene (unchanged legacy behavior, pulling from DatabaseModel's single saved
		 * scene-rect) and, for every other scene, sets its rect straight from its own items' bounding
		 * rect -- never ObjectsScene::adjustSceneRect(), which unconditionally pulls the left/top edge
		 * back to the origin and would be wrong here, since these scenes aren't being resized
		 * interactively, just given a correct rect for the first time.
		 * When false, the active scene gets the exact same items'-bounding-rect treatment as every
		 * other one instead of the DatabaseModel-sourced shortcut -- used right after a threshold
		 * rebucketing (see loadModel()), where scene membership just changed and any rect sourced from
		 * before that -- the model's single legacy rect included -- no longer describes what's
		 * actually in each scene now */
		void adjustAllScenesRects(bool use_model_rect = true);

		/*! \brief Expand the scene rect to the specified direction.
		 * The size expanded is determined by the current page layout used by the scene. */
		void expandSceneRect(ObjectsScene::ExpandDirection exp_dir);

		//! \brief Set the model as modified forcing it to be redrawn
		void setModified(bool value);

		//! \brief Returns the loaded database model filename
		QString getFilename();

		//! \brief Returns the temporary (security copy) of the currently loaded model
		QString getTempFilename();

		//! \brief Shows the editing form according to the passed object type
		void showObjectForm(ObjectType obj_type, BaseObject *object=nullptr, BaseObject *parent_obj=nullptr, const QPointF &pos=QPointF(DNaN, DNaN));

		//! \brief Applies a zoom factor to the model
		void applyZoom(double zoom);

		//! \brief Returns the current zoom factor applied to the model
		double getCurrentZoom();

		//! \brief Returns if the model is modified or not
		bool isModified();

		//! \brief Returns the reference database model
		DatabaseModel *getDatabaseModel();

		//! \brief Returns the currently active object scene used by the model
		ObjectsScene *getScene();

		//! \brief Returns the scene at the given index in the scenes collection (nullptr if idx is out of bounds)
		ObjectsScene *getScene(int idx);

		//! \brief Returns the full collection of scenes currently managed by this widget
		QList<ObjectsScene *> getScenes();

		//! \brief Returns the number of scenes currently managed by this widget
		int getSceneCount();

		//! \brief Returns the index of the currently active scene within the scenes collection
		int getActiveSceneIndex();

		//! \brief Returns a display name for each scene in the collection, in order (schema name if schema-segmented, "Scene N" otherwise)
		QStringList getSceneNames();

		//! \brief Returns the scene that currently holds the graphical representation of the given object, or nullptr if the object has none
		ObjectsScene *getSceneForObject(BaseObject *object);

		/*! \brief Returns the scene assigned to the given schema, or nullptr if none is assigned yet.
		 * Unlike getSceneForObject(), this doesn't depend on the schema having a graphical item of its
		 * own -- schema-segmentation mode never draws one (see ModelWidget::handleObjectAddition()) --
		 * and only ever resolves to something outside of that mode, since it's the only one where a
		 * schema maps to exactly one scene */
		ObjectsScene *getSchemaScene(Schema *schema);

		/*! \brief Discards graph_obj's existing item and builds a fresh one directly in dst_scene,
		 * re-routing (see routeRelationship()) every relationship that was connected to it -- a
		 * no-op if graph_obj has no item yet, or is already in dst_scene. Used by moveToSchema()
		 * and relocateEditedObject() instead of ObjectsScene::relocateItem(): moving an existing
		 * item across scenes leaves some of its internal child items corrupted in a way that only
		 * crashes later, confirmed for RelationshipView and avoided here for every relocatable type */
		void relocateObjectView(BaseGraphicObject *graph_obj, ObjectsScene *dst_scene);

		/*! \brief Textbox counterpart to relocateObjectView() -- Textbox has no relationships, cut-off
		 * markers or schema, so it needs none of that method's table-specific bookkeeping (it casts
		 * the existing item to BaseTableView, which a Textbox's StyledTextboxView never is -- a no-op
		 * if called on one). Same discard+recreate technique, just the trivial case: remove the old
		 * item, build a fresh StyledTextboxView directly in dst_scene */
		void relocateTextboxView(Textbox *txtbox, ObjectsScene *dst_scene);

		//! \brief Returns the scene view used by the model
		QGraphicsView *getViewport();

		//! \brief Returns the operation list used by database model
		OperationList *getOperationList();

		//! \brief Returns the currently selected list of objects
		std::vector<BaseObject *> getSelectedObjects();

		//! \brief Returns the whether there are at least one object selected in the model
		bool hasSelectedObjects();

		//! \brief Defines if any instance of ModelWidget must restore the last saved editing position on canvas
		static void setSaveLastCanvasPosition(bool value);

		//! \brief Defines if any instance of the class must disable rendering smoothness improving performance
		static void setRenderSmoothnessDisabled(bool value);

		//! \brief Defines if any instance of the class must simiplify the graphical object's creation
		static void setSimplifiedObjectCreation(bool value);

		//! \brief Defines the minimum object opacity when using fade out feature
		static void setMinimumObjectOpacity(unsigned min_opacity);

		//! \brief Restore the last editing position on canvas as well the zoom factor
		void restoreLastCanvasPosition();

		/*! \brief Save the last editing position on canvas as well the zoom factor. This method return true when
		the current values was saved on the database model */
		bool saveLastCanvasPosition();

		void setUpdatesEnabled(bool value);

		void updateRenderHints();

		/*! \brief Updates the opacity factor of the objects faded in the model. This method should be called
		whenever the min_object_opacity changes */
		void updateObjectsOpacity();

		/*! \brief Rearrange table/view/textboxes in the canvas in such way to provide better visualization
		 * of the whole model. Currently only hierachical arrangement is possible. See rearrangeTablesHierarchically().
		 * current_scene_only restricts it to the active scene; otherwise every scene is rearranged, each
		 * independently (own root, own textbox/leftover-table layout, own bounding area) */
		void rearrangeTablesHierarchically(bool current_scene_only);

		/*! \brief Reorganizes the schemas over the scene in a grid form. The parameters are: whether to
		 * restrict to the active scene or arrange every scene independently, an origin point, number of
		 * tables per row, schemas per row and a object spacing. Outside of NoSegmentation there's no
		 * schema box to grid (see ModelWidget::handleObjectAddition()) -- each scene's tables are
		 * arranged directly in a flat grid instead, schema grouping ignored */
		void rearrangeSchemasInGrid(bool current_scene_only, unsigned tabs_per_row = 0, unsigned sch_per_row = 0,
																QPointF origin = QPointF(50, 50), double obj_spacing = 75);

		//! \brief Arrange all tables in their schemas randomly (scattered). See rearrangeSchemasInGrid() for current_scene_only
		void rearrangeTablesInSchemas(bool current_scene_only);

		/*! \brief Writes the union of every scene's rect into the model's legacy scene-rect attribute,
		 * used as a single-scene fallback if the model is later reopened/edited without segmentation.
		 * Called only right before saving, never reactively per-scene. Public so pgmodeler-cli's
		 * importDatabase() (which saves the DatabaseModel directly, bypassing ModelWidget::saveModel())
		 * can call it too */
		void updateModelSceneRectInfo();

		void emitSceneInteracted();

		//! \brief Toggles the protection over the model and display a message indicating the status
		void setProtected(bool protect);

		void setInteractive(bool interact);

		bool isInteractive();

	private slots:
		//! \brief Handles the signals that indicates the object creation on the reference database model
		void handleObjectAddition(BaseObject *object);

		//! \brief Handles the signals that indicates the object removal on the reference database model
		void handleObjectRemoval(BaseObject *object);

		//! \brief Handles the signals that indicates the object moviment on the scene
		void handleObjectsMovement(ObjectsScene *src_scene, bool end_moviment);

		//! \brief Handles the signals that indicates the object edition on the scene
		void handleObjectModification(BaseGraphicObject *object);

		//! \brief Handles the signals that indicates the object was double clicked on the scene
		void handleObjectDoubleClick(BaseGraphicObject *object);

		//! \brief Shows the relationships (hidden and cross-scene) of the table a double-clicked relationship marker belongs to
		void showTableRelationships(BaseTable *table);

		//! \brief Configures the popup menu specific for the passed object
		void configurePopupMenu(BaseObject *object);

		//! \brief Configures the popup menu according the the selected objects list
		void configurePopupMenu(const std::vector<BaseObject *> &objects=std::vector<BaseObject *>());

		//! \brief Shows the configured popup menu
		void showObjectMenu();

		//! \brief Shows the widget containing the dependencies and references to the object
		void showObjectAssociations();

		//! \brief Triggers the object editing
		void editObject();

		//! \brief Toggles the protection of the selected objects
		void protectObject();

		//! \brief Triggers the quick rename action
		void renameObjects();

		//! \brief Move the selected object to a schema (selectable via menu)
		void moveToSchema();

		/*! \brief Moves the selected objects to a scene (selectable via menu, subtask 4.2). In
		 * schema-segmentation mode, only Textbox can be freely reassigned this way -- Table/View/
		 * ForeignTable's scene follows their schema there, so moving just one of them without its
		 * schema would break that invariant. Any other type is silently skipped (an alert is shown
		 * afterwards if anything was), fully applied only in threshold-segmentation mode */
		void moveToScene();

		//! \brief Update objects visibility according to the layers visibility
		void updateObjectsLayers();

		//! \brief Quickly changes the object's owner via popup menu
		void changeOwner();

		//! \brief Quickly sets the table's tag via popup menu
		void setTag();

		//! \brief Triggers the permission editing form
		void editPermissions();

		//! \brief Selects all the graphical objects under the selected schema, regardless of which scene each one is in
		void selectSchemaChildren();

		//! \brief Selects all the relationships connected to the selected table
		void selectTableRelationships();

		//! \brief Selects all the tables and views that references the selected tag
		void selectTaggedTables();

		//! \brief Creates a tag named after the schema targeted by the popup menu, tagging its children
		void createTagFromSchema();

		//! \brief Creates a layer named after the schema targeted by the popup menu, adding its children to it
		void createLayerFromSchema();

		//! \brief Batch version of createTagFromSchema() for every schema in the model, asked for via the canvas' popup menu
		void createTagsForAllSchemas();

		//! \brief Batch version of createLayerFromSchema() for every schema in the model, asked for via the canvas' popup menu
		void createLayersForAllSchemas();

		//! \brief Removes the selected objects
		void removeObjects(bool cascade = false);

		//! \brief Selects all the graphical objects on the scene
		void selectAllObjects();

		//! \brief Copies all the selected objects
		void copyObjects(bool duplicate_mode, bool copy_deps);

		//! \brief Paste all the objects copied previously
		void pasteObjects(bool duplicate_mode = false);

		//! \brief Duplicate the selected table object in its parent table
		void duplicateObject();

		//! \brief Cuts the selected objects. The effective removal is made when the cutted objects are pasted.
		void cutObjects(bool copy_deps);

		//! \brief Converts a Many to Many relationship generating a table and two additional relationships.
		void convertRelationshipNN();

		/*! \brief Converts a one to (one|many) relationship by copying columns and constraints representing the link
		 * to the receiver table in order to allow manual editings by the user */
		void convertRelationship1N();

		//! \brief Loads the selected object source code on the source code widget.
		void showSourceCode();

		//! \brief Adds a new object onto the selected model/table/schema
		void addNewObject();

		//! \brief Triggers the sql insertion widget for the current database model
		void editCustomSQL();

		//! \brief Configures the selected object vector whenever the selection changes on the scene
		void configureObjectSelection();

		/*! \brief Creates a sequence based upon the selected column. This method changes the default value
		for the column as well the type. */
		void createSequenceFromColumn();

		/*! \brief Creates a serial data type based upon the selected column data type. The prerequisite to create a serial data type
		is that the column's type is an integer one (smallint, int, bigint) and the default value is a function call
		to nextval('myseq'::regclass) */
		void convertIntegerToSerial();

		//! \brief Break the relationship line in one or two straight angles (see BREAK_??? constants)
		void breakRelationshipLine();

		//! \brief Removes any user added point from relationship
		void removeRelationshipPoints();

		/*! \brief Sets a random custom color in the selected relationships or disables their custom color.
		 * The action that triggers the slot stores true in its data to set a random color */
		void changeRelationshipsColor();

		//! \brief Highlights the object stored on the action that triggers the slot
		void highlightObject();

		void toggleNewObjectOverlay();

		void adjustOverlayPosition();

		void toggleObjectSQL();

		void fadeObjectsIn();

		void fadeObjectsOut();

		void setCollapseMode();

		void togglePagination();

		void toggleSchemasRectangles();

		void swapObjectsIds();

		void jumpToTable();

		void editTableData();

		/*! \brief Update all the information about layer in the internal database model
		 * so the correct info is written into the xml code of the model file */
		void updateModelLayersInfo();

		void showMagnifierArea(bool show);

		//! \brief Prepares the viewport to a move by hiding grid/delimiters
		void startSceneMove();

		//! \brief Restores the previous grid/delimiter visibility state after finishing a scene move
		void finishSceneMove();

		/*! \brief Toggles the blinking of new graphical objects added to the model widget
		 *  This method first call QGraphicsItem::setSelected and then QGraphicsView::centerOn
		 *  to simulate a highlight in the new object */
		void setBlinkAddedObjects(bool value);

		void selectObjectsInLayers(const QList<unsigned int> &layer_ids);

	public slots:
		//! \brief Clears the selection on every scene, not just the active one
		void clearSelectionInAllScenes();

		//! \brief Activates the scene at the given index, reattaching it to the viewport
		void setActiveScene(int idx);

		/*! \brief Sets the segmentation mode to be persisted on the next save. Doesn't affect live
		 * routing this session (see DatabaseModel::setSavedSegmentationMode()) -- just marks the
		 * model modified */
		void setPendingSegmentationMode(DatabaseModel::SegmentationMode mode);

		/*! \brief Sets the segmentation threshold to be persisted on the next save. Doesn't affect
		 * live routing this session (see DatabaseModel::setSavedSegThreshold()) -- just marks the
		 * model modified */
		void setPendingSceneThreshold(unsigned value);

		/*! \brief Sets the hide relationships option to be persisted on the next save. Doesn't affect
		 * live routing this session (see DatabaseModel::setSavedHideRelationships()) -- just marks the
		 * model modified */
		void setPendingHideRelationships(bool value);

		/*! \brief Loads filename into this widget's model. When check_alt_seg_mode is true and the
		 * file has no segmentation mode of its own saved yet, checkAlternateSegmentation() is run
		 * against it first and, if it suggests one, the user is prompted to apply it -- taking effect
		 * on this very load (via DatabaseModel::loadModel()'s override params) instead of only after
		 * an extra save+reload round-trip. Defaults to no use of suggestion. */
		void loadModel(const QString &filename, bool check_alt_seg_mode = false);
		void saveModel(const QString &filename);
		void saveModel();
		void printModel(QPrinter *printer, bool print_grid, bool print_page_nums, bool resize_delims);
		void update();
		void bringToFront();
		void sendToBack();

	signals:
		void s_objectAdded(BaseGraphicObject *object);
		void s_itemAdded(QGraphicsItem *item);
		void s_objectModified();
		void s_objectsMoved();
		void s_objectCreated();
		void s_objectRemoved();
		void s_zoomModified(double);
		void s_maginifierAreaVisible(bool);
		void s_modelResized();
		void s_modelSaveReloadRequested();

		//! \brief Emitted after setActiveScene() switches the scene attached to the viewport
		void s_activeSceneChanged(int new_idx);

		//! \brief Emitted whenever a scene is added to or removed from the scenes collection
		void s_scenesListChanged();

		//! \brief Signal emitted whenever the interaction status is changes (see setInteractive)
		void s_interactiveChanged(bool);

		//! \brief Signal emitted whenever the modified status of the model changes
		void s_modelModified(bool);

		//! \brief Signal emitted whenever a object is created / edited using the form
		void s_objectManipulated();

		/*! \brief Signal emitted whenever the user open an object in its editing form but cancel the operation
		by closing the form */
		void s_manipulationCanceled();

		/*! \brief Signal emitted whenever the scene suffered an interaction.
		 * This version carries the interacted object in the scene */
		void s_sceneInteracted(BaseObjectView *sel_obj);

		/*! \brief Signal emitted whenever the scene suffered an interaction.
		 * This version carries the scene size after being adjusted */
		void s_sceneInteracted(const QSizeF &scene_size);

		/*! \brief Signal emitted whenever the scene suffered an interaction.
		 * This version carries the mouse position after the moving objects */
		void s_sceneInteracted(const QPointF &mouse_pos);

		/*! \brief Signal emitted whenever the scene suffered an interaction.
		 * This version carries the amount objects selected and their bounding rects */
		void s_sceneInteracted(int obj_count, const QRectF &objs_rect);

		/*! \brief Signal emitted whenever the scene suffered a successful drag & drop event.
		 * It passes the mime data when the event occurred */
		void s_sceneDragDropped(const QMimeData *mime_data);

		/*! \brief This signal is emitted whenever the user requests via context menu Quick > Set layer > New layer button
		 *  the creation of a new layer, or via Quick > Create layer from schema(s). This signal just redirects the
		 *  new layer name to the outside world do be handled properly (currently, in MainWindow) */
		void s_newLayerRequested(const QString &layer_name);

		//! \brief Signal emitted whenever the layers of the selected object is changed via "Set layers" menu item
		void s_objectsLayerChanged();

		friend class MainWindow;
		friend class ModelExportWidget;
		friend class OperationListWidget;
		friend class ModelObjectsWidget;
		friend class ModelOverviewWidget;
		friend class ModelValidationWidget;
		friend class ObjectSearchWidget;
		friend class NewObjectOverlayWidget;
		friend class LayersConfigWidget;
		friend class MetadataHandlingWidget;
		friend class DatabaseImportWidget;
};

#endif
