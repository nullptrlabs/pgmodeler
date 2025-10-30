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
\ingroup libcanvas
\class ObjectsScene
\brief Implements the basic operations to visualize and manipulate the graphical objects.
*/

#ifndef OBJECTS_SCENE_H
#define OBJECTS_SCENE_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPrinter>
#include <QKeyEvent>
#include "layeritem.h"
#include "baseobjectview.h"
#include "basetableview.h"
#include "doublenan.h"

class __libcanvas ObjectsScene: public QGraphicsScene {
	Q_OBJECT

	public:
		enum GridPattern: unsigned {
			SquarePattern,
			DotPattern
		};

		//! \brief Stores the default grid line color
		static const QColor DefaultGridColor,

		//! \brief Stores the default grid line color
		DefaultCanvasColor,

		//! \brief Stores the default page delimiter lines color
		DefaultDelimitersColor;

	private:
		static GridPattern grid_pattern;

		//! \brief Stores the grid line color
		static QColor grid_color,

		//! \brief Stores the canvas background color
		canvas_color,

		//! \brief Stores the page delimiter lines color
		delimiters_color;

		//! \brief Holds the names of the layers on the scene used to separate in the objects on the canvas
		QStringList layers,

		//! \brief Holds the names of the layers currently active (visible) in the canvas
		active_layers;

		//! \brief Stores the items used to represent layers around objects
		QList<LayerItem *> layers_paths;

		std::vector<BaseObjectView *> removed_objs;

		//! \brief Holds the tables/views which have selected children objects
		QList<BaseTableView *> tabs_sel_children;

		//! \brief Indicates if the scene need to be moved
		bool move_scene,

		show_scene_limits;

		static constexpr int SceneMoveStep=20,
		SceneMoveTimeout=20,
		SceneMoveThreshold=100;

		//! \brief Timer responsible to move the scene
		QTimer scene_move_timer,

		/*! \brief Timer responsible to check if the user puts cursor at corners for a certain amount of time.
		When this timeout the scene_move_timer will be triggered and the scene will be moved */
		corner_hover_timer,

		/*! brief Timer responsible to control the interval between key presses/releases when moving objects with the keyboard.
		This timer is started whenever the user releases the arrow keys. If the timer emits timeout() signal
		indicates that the user take more than 500ms to press and release the arrow keys again, this indicates to the scene
		to emit the signal s_objectsMove(true) to alert the end of the objects moving. If the user presses/releases the keys when
		the timer is still running the same will be restarted, until its timeout is reached. This trick avoids 'spamming' the
		operation list in ModelWidget creating registries only when the user starts and ends the objects movement. Intermediate
		key presses/releases aren't not registered in the operation history */
		object_move_timer;

		//! \brief Attributes used to control the direction of scene movement when user puts cursor at corners
		int scene_move_dx, scene_move_dy;

		//! \brief Object alignemnt, grid showing, page delimiter showing options
		static bool align_objs_grid,

		show_grid,

		show_page_delim,

		//! \brief Indicates if the corner move is enabled for the scene
		corner_move,

		/*! \brief Indicates that panning mode and range selection model are activate in inverse mode.
		By default panning model is activated with a single left-click and range selection with SHIFT + left-click */
		invert_rangesel_trigger,

		lock_delim_scale;

		//! \brief Scene grid size
		static unsigned grid_size,

		//! \brief The number of pages in which the scene rect is expanded
		expansion_factor;

		//! \brief Used to store the canvas/printer page layout (size, orientation, margins)
		static QPageLayout page_layout;

		static double delimiter_scale,

		//! \brief The minimum scene width is defined to be width of the current page layout * 2
		min_scene_width,

		//! \brief The minimum scene height is defined to be height of the current page layout * 2
		min_scene_height;

		//! \brief Indicates that there are objects being moved and the signal s_objectsMoved must be emitted
		bool moving_objs,

		//! \brief Indicates if the range selection (selection using a rectangle drawn on the canvas)
		enable_range_sel,

		//! \brief Indicates if the layers rects around the object must be displayed
		is_layer_rects_visible,

		//! \brief Indicates if the layers names in the rects around the object must be displayed
		is_layer_names_visible;

		//! \brief Initial point of selection rectangle
		QPointF sel_ini_pnt;

		//! \brief Rectangle used to select several objects on the scene
		QGraphicsPolygonItem *selection_rect;

		//! \brief Line used as a guide when inserting new relationship
		QGraphicsLineItem *rel_line;

		/*! \brief Indicates if the mouse cursor is under a move spot portion of scene.
		Additionally this method configures the direction of movement when returning true */
		bool mouseIsAtCorner();

		QGraphicsView *getActiveViewport();

		//! \brief Performs the final steps when moving the objects like adjusting position to grid, moving children object, etc
		void finishObjectsMove(const QPointF &pnt_end);

		/*! \brief Performs the scene position adjustment when pressing/releasing the arrow keys to avoid objects to be hidden in the corners.
		This method will adjust the scene size automatically when moving objects right or down if the selected items bounding rects
		exceeds the scene's size limits */
		void adjustScenePositionOnKeyEvent(int key);

		//! \brief Formats the name of the layer removing any invalid chars and doing the desambiguation in case the name already exists
		QString formatLayerName(const QString &name);

		void clearTablesChildrenSelection();

	protected:
		void drawBackground(QPainter *painter, const QRectF &rect) override;

		void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
		void keyPressEvent(QKeyEvent *event) override;
		void keyReleaseEvent(QKeyEvent *event) override;

		//! \brief Draws a line from the point 'p_start' to the cursor position and simulates the relationship creation
		void showRelationshipLine(bool value, const QPointF &p_start=QPointF(DNaN,DNaN));

		void blockItemsSignals(bool block);

		/*! \brief Move the objects from a deleted layer to the default one (0).
		 * This method automatically shifts the objects layers in case the layer being removed is
		 * greater than 0 or less than the maximum of layers minus 1, avoiding references to invalid layer ids. */
		void validateLayerRemoval(unsigned old_layer);

		/*! \brief Add a several layers at once. This method will resolve name conflicts automatically.
		 * This method removes all the current layers (except the first) and add the new ones.
		 * The parameter reset_objs_layers is used to define if the objects should be moved to the default
		 * layer or not. */
		void addLayers(QStringList names, bool reset_objs_layers);

		/*! \brief Destroy all layers (except the default one).
		 * The parameter reset_objs_layers is used to define if the objects should be moved to the default layer or not. */
		void removeLayers(bool reset_obj_layers);

		/*! \brief Adjusts the current scene rectangle based upon the items bounding rect.
		 *  The method returns the new scene rectangle. */
		QRectF adjustSceneRect(bool expand_only);

	public:
		//! \brief This enum controls the direction to where the scene must be expanded.
		enum ExpandDirection {
			ExpandTop,
			ExpandLeft,
			ExpandRight,
			ExpandBottom
		};

		enum LayerAttrColor: unsigned {
			LayerNameColor,
			LayerRectColor
		};

		static constexpr double MinScaleFactor = 0.100000,
		MaxScaleFactor = 5.000001;

		static constexpr unsigned DefaultLayer = 0;

		ObjectsScene();

		~ObjectsScene() override;

		/*! \brief Add a new layer to the scene. In case of duplicated name this method
		 * automatically does the desambiguation. The name of the new layer is returned. */
		QString addLayer(const QString &name);

		/*! \brief Rename the layer of the provided index. In case of duplicated name this method
		 * 	automatically does the desambiguation. */
		QString renameLayer(unsigned idx, const QString &name);

		//! \brief Remove the named layer. Objects in the destroyed layer are automatically moved to the default one
		void removeLayer(const QString &name);

		//! \brief Destroy all layers (except the default one) moving all objects from the destroyed layers to the default one
		void removeLayers();

		//! \brief Set the named layers as active. Activating a layer causes objects attached to it to be visible
		void setActiveLayers(QStringList act_layers);

		//! \brief Set the layers with the provided indexes as active. Activating a layer causes objects attached to it to be visible
		void setActiveLayers(QList<unsigned> layer_ids);

		//! \brief Returns true when the named layer is currenctly activated
		bool isLayerActive(const QString &name);

		//! \brief Returns true when the layer with the provided id is currenctly activated
		bool isLayerActive(unsigned layer_id);

		//! \brief Returns true when at least one of the layer ids in the list is currenctly activated
		bool isLayersActive(const QList<unsigned> &list);

		//! \brief Returns a list containing the names of the active layers
		QStringList getActiveLayers();

		//! \brief Returns a list containing the ids of the active layers
		QList<unsigned> getActiveLayersIds();

		//! \brief Returns a list containing the names of all layers in the scene
		QStringList getLayers();

		//! \brief This method causes objects in the active layers to have their visibility state updated.
		void updateActiveLayers();

		//! \brief Retuns a list of the layers colors names. The color ids must be LayerNameColor or LayerRectColor
		QStringList getLayerColorNames(LayerAttrColor color_id);

		/*! \brief This method sets up the text and background color of the layer referenced by the id.
		 * This method adjust the alpha channel for the background color to a make it semi transparent */
		void setLayerColors(int layer_id, QColor txt_color, QColor bg_color);

		/*! \brief This method sets up the layers name/rect colors. The layer_attr_id is either LayerNameColor or LayerRectColor.
		 * This method adjust the alpha channel for the background color to a make it semi transparent */
		void setLayerColors(LayerAttrColor layer_attr_id, const QStringList &colors);

		//! \brief Toggles the display of the red lines that denotes the scene boundaries
		void setShowSceneLimits(bool show);

		static void setEnableCornerMove(bool enable);
		static void setInvertRangeSelectionTrigger(bool invert);
		static bool isCornerMoveEnabled();

		static void setGridPattern(GridPattern pattern);
		static void setGridSize(unsigned size);
		static unsigned getGridSize();

		static void setAlignObjectsToGrid(bool value);
		static bool isAlignObjectsToGrid();

		static void setShowGrid(bool value);
		static bool isShowGrid();

		static void setShowPageDelimiters(bool value);
		static bool isShowPageDelimiters();

		/*! \brief Determines if the delimiter lines must have their scale locked when the
		 * curr_scale factor is less than 1.0. This allows the user to fit a greater amount
		 * of objects on a single page. This feature works only for printing. */
		static void setLockDelimiterScale(bool lock, double curr_scale);
		static bool isDelimiterScaleLocked();

		/*! \brief Configures the canvas page layout. This method invalidates the grid to force it to be recreated
		 * taking into account the new page settings */
		static void setPageLayout(const QPageLayout &page_lt);

		static QPageLayout getPageLayout();

		void addItem(QGraphicsItem *item, bool blink_new = false);
		void removeItem(QGraphicsItem *item);

		/*! \brief Define the geometry of the scene. If the rectangle is invalid or
		 *  the size is below the minimum specificed by min_scene_width/min_scene_height
		 *  it will be adjusted to the minimum acceptable size. */
		void setSceneRect(const QRectF &rect);

		//! \brief Aligns the specified point in relation to the grid
		static QPointF alignPointToGrid(const QPointF &pnt);

		/*! \brief Returns the items bounding rect. By default the method returns the same as QGraphicsScene::itemsBoundingRect.
		If the parameter seek_only_db_objs is true the returned rect will have the origin point calculated based upon the
		visible objects that inherits BaseObjectView and are database model objects (tables, views, textboxes, schemas and relationships).

		If the paramenter selected_only is true only selected objects will have the bounding rect calculated.
		Currently this parameter is ignored when using seek_only_db_objs = false

		If the parameter incl_layer_rects is true the layer rects (when visible) will be considered when calculating the items bounding rects.
		This is useful only when determining the brect during exporting to graphics file avoiding cutting layer rects at the edges of the
		resulting	image.

		Note: using this method with seek_only_db_objs=true can be time expensive depending on the size of the model so use it wisely. */
		QRectF itemsBoundingRect(bool seek_only_db_objs=false, bool selected_only = false, bool incl_layer_rects = false);

		/*! \brief Returns a vector containing all the page rects considering the provided page layout settings
		 * A scale factor can be provided so the method returns the amount of pages in a certain zoom factor (scale) */
		QList<QRectF> getPagesForPrinting(const QPageLayout &page_lt, unsigned &h_page_cnt, unsigned &v_page_cnt, double scale);

		/*! \brief Returns a vector containing all the page rects considering the current scene's page layout settings
		 * This version always uses scale = 1. If the user need to apply a scale in the pages, it must be done in the viewport
		 * attached to the scene */
		QList<QRectF> getPagesForPrinting(unsigned &h_page_cnt, unsigned &v_page_cnt);

		bool isRangeSelectionEnabled();
		bool isRangeSelectionTriggerInverted();
		bool isRelationshipLineVisible();
		bool isMovingObjects();

		QList<QGraphicsItem *> selectedItems(void) const;
		bool hasOnlyTableChildrenSelection(void) const;

		static void setGridColor(const QColor &value);
		static QColor getGridColor();

		static void setCanvasColor(const QColor &value);
		static QColor getCanvasColor();

		static void setPageDelimitersColor(const QColor &value);
		static QColor getPageDelimitersColor();

		static void setExpansionFactor(unsigned factor);
		static unsigned getExpansionFactor();

		bool isLayerRectsVisible();
		bool isLayerNamesVisible();

		/*! \brief Expand the scene rect to the specified direction.
		 * The size expanded is determined by the current page layout used by the scene. */
		void expandSceneRect(ExpandDirection exp_dir);

	public slots:
		//! \brief Force the update of all layer rectangles
		void updateLayerRects();

		void setLayerRectsVisible(bool value);
		void setLayerNamesVisible(bool value);
		void alignObjectsToGrid();
		void update();
		void clearSelection();

		//! \brief Toggles the object range selection
		void enableRangeSelection(bool value);

	private slots:
		/*! \brief Start/stop the timer responsible to move the scene. This method is called with true param
		whenever the user stay with the cursor at corner in a certain amount of time */
		void enableSceneMove(bool value=true);

		//! \brief Moves the scene when the user puts the mouse cursor on one of scene's edges
		void moveObjectScene();

		//! \brief Handles and redirects the signal emitted by the selected child object
		void handlePopupMenuRequested(TableObject *child_obj);

		//! \brief Handles and redirects the signal emitted by the selected object
		void handleObjectSelection(BaseGraphicObject *object, bool selected);

		//! \brief Handles the tables children objects selection changes
		void handleChildrenSelectionChanged();

	signals:
		//! \brief Signal emitted when the user start or finalizes a object movement.
		void s_objectsMoved(bool end_moviment);

		//! \brief Signal emitted when a object is modified on scene
		void s_objectModified(BaseGraphicObject *object);

		//! \brief Signal emitted when the user toggles a table's collapse mode in the scene
		void s_collapseModeChanged();

		//! \brief Signal emitted when the user toggles a table's attributes pagination
		void s_paginationToggled();

		//! \brief Signal emitted when the user changes a table's attributes page
		void s_currentPageChanged();

		//! \brief Signal emitted when the user right-click a specific object on the scene requesting the popup menu
		void s_popupMenuRequested(BaseObject *);

		//! \brief Signal emitted when the user right-click one or more objects on the scene requesting the popup menu
		void s_popupMenuRequested();

		//! \brief Signal emitted when the user double-click a object
		void s_objectDoubleClicked(BaseGraphicObject *objeto);

		//! \brief Signal emitted when a object is selected
		void s_objectSelected(BaseGraphicObject *objeto, bool selecionado);

		//! \brief Signal emitted when objects are selected via range selection
		void s_objectsSelectedInRange();

		//! \brief Signal emitted when a blank area of the canvas is pressed
		void s_scenePressed(Qt::MouseButtons);

		//! \brief Signal emitted when the active layers change
		void s_activeLayersChanged();

		//! \brief Signal emitted when the layers change (add, remove, rename)
		void s_layersChanged();

		//! \brief Signal emitted when objects are moved from a layer to another
		void s_objectsMovedLayer();

		//! \brief Signal emitted when tables children objects have their selection statuses changed
		void s_childrenSelectionChanged();

		/*! \brief Signal emitted when one of the dimensions of the scene rect changes after moving one or more object
		 * The signal argument is the selected object rectangles united in one single bounding rect.
		 * This signal is used to indicated that the viewport attached to the scene must center/ensure visible the
		 * passed rectagle causing the object recently move to be visible */
		void s_ensureVisibleRequested(QRectF);

		//! \brief Signal emitted when the scene rect changes
		void s_sceneRectChanged(QRectF);

		friend class ModelWidget;
		friend class PgModelerCliApp;
		friend class DatabaseImportWidget;
};

#endif
