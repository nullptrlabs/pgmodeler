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

class __libgui ModelWidget: public QWidget {
	Q_OBJECT

	private:
		#ifdef DEMO_VERSION
			//Maximum number of objects' operations (create, move, delete, etc)
			static constexpr unsigned MaxObjActions = 200;
			unsigned obj_actions_cnt;

			bool updateObjActionCounter() {
				obj_actions_cnt++;

				// If maximum actions is reached
				if(obj_actions_cnt > MaxObjActions)
				{
					Messagebox::alert("You have reached the maximum number of actions that can be taken over the database object in the demo version!");
					return true;
				}

				return false;
			}
		#endif

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

		//! \brief Graphical objects scene
		ObjectsScene *scene;

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

		//! \brief Stores the role names used by the "change owner" operation
		owners_menu,

		//! \brief Stores the tags used by the "set tag" operation
		tags_menu,

		break_rel_menu,

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

		//! \brief Fades in our out the object types held by the specified action
		void fadeObjects(QAction *action, bool fade_in);

		void breakRelationshipLine(BaseRelationship *rel, RelBreakMode break_type);

		/*! \brief Arrange tables starting from a specified root in a hierarchical way
		where for a certain table its child (or related) tables are places aside from left to right and top to bottom.
		This method returns the bounding rect of the items after the rearrangement */
		QRectF rearrangeTablesHierarchically(BaseTableView *root, std::vector<BaseObject *> &evaluated_tabs);

		/*! \brief Arrange tables inside the provided schema randomly (scattered). An start point should
		 * be provided. The method will avoid to put two or more tables in the same position causing
		 * overlaping. This method causes the schema rectangle to be enabled. */
		void rearrangeTablesInSchema(Schema *schema, QPointF start);

		void updateMagnifierArea();

		/*! \brief Move the selected objects in the Z coordenate either to bottom or top.
		 * The direction is defined by the constants BringToTop or SendToBottom. */
		void moveObjectsInZStack(int direction);

		//! \brief Applies the layer settings from the internal database model to the scene object
		void updateSceneLayers();

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
		*action_deps_refs,
		*action_new_object,
		*action_rename,
		*action_moveto_schema,
		*action_edit_perms,
		*action_change_owner,
		*action_quick_actions,
		*action_sel_sch_children,
		*action_sel_table_rels,
		*action_sel_tagged_tabs,
		*action_select_object,
		*action_parent_rel,
		*action_append_sql,
		*action_create_seq_col,
		*action_conv_int_serial,
		*action_break_rel_line,
		*action_remove_rel_points,
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

		/*! \brief Reorganizes the tables of a specific schema over the scene. The parameter are:
		 the schema in which the tables will be rearranged, an origin point, number of tables per row
		 a object spacing */
		void rearrangeTablesInGrid(Schema *schema, unsigned tabs_per_row, QPointF origin, double obj_spacing);

		void fadeObjects(const std::vector<BaseObject *> &objects, bool fade_in);

		void setAllCollapseMode(BaseTable::CollapseMode mode);

	public:
		static constexpr double MinimumZoom = ObjectsScene::MinScaleFactor,
		MaximumZoom = ObjectsScene::MaxScaleFactor,
		ZoomIncrement = 0.050000;

		ModelWidget(QWidget *parent = nullptr);
		~ModelWidget() override;

		//! \brief Creates a BaseForm instance and insert the widget into it. A custom configuration for dialog buttons can be passed
		template<class WidgetClass>
		int openEditingForm(WidgetClass *widget, Messagebox::ButtonsId button_conf = Messagebox::OkCancelButtons);

		/*! \brief Configures the scene aligning the object to the grid and resizing the scene
		 * rect when some object is out of bound. The parameter use_model_rect, when true, uses
		 * the rect stored in the database model (retrieved from the dbm file loading) to resize
		 * the scene rect. The parameter expand_only, when true, indicates that the method must
		 * only expand the scene rect, If the recalculated rect is smaller the the current
		 * scene rect that no operation is done. */
		void adjustSceneRect(bool use_model_rect, bool expand_only = false);

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

		//! \brief Returns the object scene used by the model
		ObjectsScene *getObjectsScene();

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
		 * of the whole model. Currently only hierachical arrangement is possible. See rearrangeTablesHierarchically() */
		void rearrangeTablesHierarchically();

		/*! \brief Reorganizes the schemas over the scene in a grid form. The parameters are: an origin point,
		number of tables per row, schemas per row and a object spacing */
		void rearrangeSchemasInGrid(unsigned tabs_per_row = 0, unsigned sch_per_row = 0, QPointF origin = QPointF(50, 50), double obj_spacing = 75);

		//! \brief Arrange all tables it their schemas randomly (scattered)
		void rearrangeTablesInSchemas();

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
		void handleObjectsMovement(bool end_moviment);

		//! \brief Handles the signals that indicates the object edition on the scene
		void handleObjectModification(BaseGraphicObject *object);

		//! \brief Handles the signals that indicates the object was double clicked on the scene
		void handleObjectDoubleClick(BaseGraphicObject *object);

		//! \brief Configures the popup menu specific for the passed object
		void configurePopupMenu(BaseObject *object);

		//! \brief Configures the popup menu according the the selected objects list
		void configurePopupMenu(const std::vector<BaseObject *> &objects=std::vector<BaseObject *>());

		//! \brief Shows the configured popup menu
		void showObjectMenu();

		//! \brief Shows the widget containing the dependencies and references to the object
		void showDependenciesReferences();

		//! \brief Triggers the object editing
		void editObject();

		//! \brief Toggles the protection of the selected objects
		void protectObject();

		//! \brief Triggers the quick rename action
		void renameObjects();

		//! \brief Move the selected object to a schema (selectable via menu)
		void moveToSchema();

		//! \brief Update objects visibility according to the layers visibility
		void updateObjectsLayers();

		//! \brief Quickly changes the object's owner via popup menu
		void changeOwner();

		//! \brief Quickly sets the table's tag via popup menu
		void setTag();

		//! \brief Triggers the permission editing form
		void editPermissions();

		//! \brief Selects all the graphical objects under the selected schema
		void selectSchemaChildren();

		//! \brief Selects all the relationships connected to the selected table
		void selectTableRelationships();

		//! \brief Selects all the tables and views that references the selected tag
		void selectTaggedTables();

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

	public slots:
		void loadModel(const QString &filename);
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
		 *  the creation of a new layer. This signal just redirects the new layer name to the outside world do be
		 *  handled properly (currently, in MainWindow) */
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
