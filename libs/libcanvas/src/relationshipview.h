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
\ingroup libcanvas
\class RelationshipView
\brief Implements the graphical representation for relationships
*/

#ifndef RELATIONSHIP_VIEW_H
#define RELATIONSHIP_VIEW_H

#include "textboxview.h"
#include "baserelationship.h"
#include "basetableview.h"
#include "beziercurveitem.h"

class __libcanvas RelationshipView: public BaseObjectView {
	Q_OBJECT

	public:
		enum LineConnectionMode: unsigned {
			ConnectCenterPoints,
			ConnectFkToPk,
			ConnectTableEdges
		};

		/*! \brief The style of the lines used to render the relationships. The orthogonal style draws
		 * only horizontal/vertical segments with rounded corners and ignores the points added by the user. */
		enum LineStyle: unsigned {
			StraightLines,
			CurvedLines,
			OrthogonalLines
		};

	private:
		//! \brief Graphical point radius
		static constexpr double GraphicPointRadius=6.0;

		/*! \brief The width of the path stroke used to select relationship line
		 *  as well as add points to it */
		static constexpr double LineSelStrokeWidth=15;

		//! \brief Length of the lines linked to fk/pk columns
		static constexpr double ConnLineLength=20.0;

		/*! \brief Ideal radius, along each segment, used to round off the corners of a
		 * self-relationship's loop and of orthogonal lines */
		static constexpr double RoundedCornerRadius=10.0;

		//! \brief Radius of the arcs the orthogonal lines make to hop over the lines that cross them
		static constexpr double BumpRadius=4.0;

		//! \brief Clearance kept between the lines and the end zones (crow's foot area) of other relationships
		static constexpr double ZoneClearance=3.0;

		//! \brief Maximum number of times the scene recalculates a line that goes through an end zone
		static constexpr unsigned MaxZoneRetries=2;

		//! \brief Indicates that the relationship labels must be hidden
		static bool hide_name_label,

		//! \brief Indicates that the relationship should be drawn in Crow's foot notation
		use_crows_foot;

		/*! \brief Specify the type of connection used by the lines. The first (classical)
		is to connect the line to tables through their central points. The second (better semantics)
		makes the line start from the fk columns on receiver table and connecting to the pk columns on reference table */
		static LineConnectionMode line_conn_mode;

		//! \brief Specifies how the lines of all relationships are rendered
		static LineStyle line_style;

		/*! \brief Indicate that the line is being configured/updated. This flag is used to evict
		 that the configureLine() method is exceedingly called during the table moving. */
		bool configuring_line,

		//! \brief Indicates if the instance is configured to use placeholders
		using_placeholders;

		//! \brief Stores the graphical representation for labels
		TextboxView *labels[3];

		//! \brief Stores the graphical representation for the participant tables
		BaseTableView *tables[2];

		QString tab_hashes[2];

		/*! \brief Stores the points on tables where the relationship line is connected.
		This attribute is updated every time the configureLine() method is called.
		When the relationship uses the classical link mode (center points) this attribute
		contains the table's center points. Now, when the new line mode is used this
		attribute stores the middle point between the connection points of each table */
		QPointF conn_points[2];

		//! \brief Graphical representation for the user added points
		std::vector<QGraphicsPolygonItem *> graph_points;

		//! \brief Lines that represent the relationship
		std::vector<QGraphicsLineItem *> lines,

		//! \brief Lines that are connected to the reference table (only on CONNECT_FK_TO_PK mode)
		pk_lines,

		//! \brief Lines that are connected to the receiver table (only on CONNECT_FK_TO_PK mode)
		fk_lines;

		//! \brief Stores the graphical representation for relationship attributes
		std::vector<QGraphicsItemGroup *> attributes;

		//! \brief Relationship descriptor (lozenge -> (1,n)-(1,n) relationship, triangle -> inheritance)
		QGraphicsPolygonItem *descriptor;

		//! \brief Stores the current hovered child object
		QGraphicsItem *sel_object;

		//! \brief Stores the initial (default) labels position
		QPointF labels_ini_pos[3];

		QGraphicsEllipseItem *line_circles[2];

		//! \brief Stores the curved lines representing the relationship
		std::vector<BezierCurveItem *> curves;

		//! \brief The length of the straight parts at the start and end of the last curve configured
		double curve_end_lens[2];

		/*! \brief The areas, at the ends of the orthogonal line, occupied by the crow's foot descriptors.
		 * Other relationships' lines are routed around them */
		std::vector<QRectF> end_zones;

		/*! \brief Counts how many times the route of the line was recalculated by the scene because it
		 * went through the end zone of another relationship. It limits the attempts to avoid endless updates */
		unsigned zone_retries;

		//! \brief The points of the last orthogonal line configured. They are used to calculate the bumps
		std::vector<QPointF> route_points,

		/*! \brief Positions, over the horizontal segments of the orthogonal line, where it hops
		 * over the vertical lines of other relationships that cross it */
		bumps;

		/*! \brief The route of the last orthogonal line configured before the displacements of its segments
		 * (made by the user) were applied, and the minimum length of its first and last segments */
		std::vector<QPointF> base_route;

		double route_stubs[2];

		/*! \brief The areas (position and size) of the tables when the line was last configured. The displacements
		 * of the segments are discarded when one of the tables moves or changes its size */
		QRectF shift_tab_rects[2];

		//! \brief Index of the segment of the orthogonal line being dragged by the user (-1 when none)
		int drag_seg_idx;

		//! \brief Indicates that the dragged segment is horizontal (it moves vertically) and that it was effectively moved
		bool drag_horiz, drag_moved;

		//! \brief The point where the drag started and the displacement of the segment at that time and now
		QPointF drag_ini_pos;

		double drag_ini_shift, drag_shift;

		//! \brief Stores the crow's foot notation descriptors
		QGraphicsItemGroup * cf_descriptors[2];

		std::vector<QGraphicsLineItem *> src_cf_lines,	dst_cf_lines;

		QGraphicsEllipseItem *round_cf_descriptors[2];

		//! \brief Stores the selected child object index
		int sel_object_idx;

		//! \brief Configures the labels positioning
		void configureLabels();

		//! \brief Configures the relationship tooltip
		void configureToolTip();

		//! \brief Configures the descriptor form and positioning
		void configureDescriptor();

		//! \brief Configures the crow's feet descriptors form and positioning
		void configureCrowsFootDescriptors();

		//! \brief Configures the attributes positioning
		void configureAttributes();

		//! \brief Configures the position info object
		void configurePositionInfo();

		/*! \brief Geometry of the relationship line computed on each update. It carries the data
		 * shared between the stages that compute the connection points and the ones that draw the line */
		struct LineGeometry {
			//! \brief The points where the line is connected to the tables
			QPointF p_central[2],

			//! \brief Points where the fk/pk connector lines meet (fk-to-pk mode only)
			fk_pnt, pk_pnt;

			//! \brief The points the line goes through, including the ones at its ends
			std::vector<QPointF> points,

			//! \brief The fk/pk columns connection points (fk-to-pk mode only)
			fk_points, pk_points;

			//! \brief The directions in which the line leaves the tables at p_central (null vector means any direction)
			QPointF conn_dirs[2];

			//! \brief The length of the line at each end that must remain straight (area of the crow's foot descriptors)
			double end_lens[2] { 0, 0 };

			//! \brief The line is connected at the same side on both tables (fk-to-pk mode only)
			bool conn_same_sides { false },

			//! \brief Which tables have the line connected to their horizontal/vertical edges (table edges mode only)
			conn_horiz_sides[2] { false, false }, conn_vert_sides[2] { false, false };
		};

		//! \brief Removes from the group and destroys the items beyond the first "count" ones of the vector
		template<class T>
		static void trimGroupItems(QGraphicsItemGroup *group, std::vector<T *> &items, size_t count);

		/*! \brief Determines the table sides where the fk/pk connection lines are attached. When the tables
		 * overlap horizontally both are connected at the same side, otherwise at the opposite ones. */
		static void getFkPkConnSides(const QRectF &ref_rect, const QRectF &rec_rect,
																		 BaseTableView::ConnectionPoint &pk_type, BaseTableView::ConnectionPoint &fk_type,
																		 bool &same_sides);

		/*! \brief Returns whether the line must be (re)configured. It's not needed when the line is already
		 * being configured, the tables aren't rendered yet or their hash codes didn't change since the last update */
		bool isLineUpdateNeeded();

		//! \brief Stores the current hash codes of the tables to detect their changes in the next update
		void updateTablesHashes();

		//! \brief Reconnects the tables' signals if the placeholder usage changed
		void syncPlaceholderConnection();

		//! \brief Computes the geometry of the line according to the relationship type and the line connection mode
		LineGeometry computeLineGeometry();

		/*! \brief Swaps the tables of identifier 1-1 relationships so the line is drawn
		 * from the reference to the receiver table */
		void swapIdentifierTables();

		//! \brief Computes the geometry of self-relationships (the loop at the table's top right corner)
		void configureSelfRelGeometry(LineGeometry &geom);

		//! \brief Computes the connection points at the tables' center points
		void configureCenterGeometry(LineGeometry &geom);

		//! \brief Computes the connection points at the fk/pk columns (falls back to the center points)
		void configureFkToPkGeometry(LineGeometry &geom);

		/*! \brief Removes the duplicated points of the route as well as the intermediate
		 * points of collinear segments */
		static std::vector<QPointF> cleanRoute(const std::vector<QPointF> &route);

		//! \brief Returns the smallest rect that contains all the points of the route
		static QRectF getRouteRect(const std::vector<QPointF> &route);

		/*! \brief Returns whether the horizontal/vertical segment crosses the inside of one of the obstacles */
		static bool isSegmentBlocked(const QPointF &p1, const QPointF &p2, const std::vector<QRectF> &obstacles);

		//! \brief Returns whether any segment of the route crosses the inside of one of the obstacles
		static bool isRouteBlocked(const std::vector<QPointF> &route, const std::vector<QRectF> &obstacles);

		/*! \brief Returns whether the lines can be routed around the tables placed in their way. It's disabled
		 * when the scene is not available or while the tables are being moved without placeholders */
		bool isObstacleAvoidanceEnabled();

		//! \brief Returns the areas of the visible tables inside the region, excluding the ones linked by the relationship
		std::vector<QRectF> getObstacles(const QRectF &region);

		//! \brief Returns the end zones (crow's foot areas) of the other relationships inside the region, including the clearance
		std::vector<QRectF> getEndZones(const QRectF &region);

		/*! \brief Searches the best orthogonal path from start to goal that doesn't cross the obstacles.
		 * The start_dir/goal_dir (0=right, 1=down, 2=left, 3=up, -1=any) are the directions in which the path
		 * leaves the start and arrives at the goal. Returns an empty path if there's none. */
		static std::vector<QPointF> findOrthogonalPath(const QPointF &start, const QPointF &goal, int start_dir, int goal_dir,
																									 const std::vector<QRectF> &obstacles);

		/*! \brief Returns the route modified to go around the tables that stand in its way. The dirs and stubs
		 * are the same used to create the route. The route is returned unchanged if it has no obstacles or
		 * if a way around them can't be found */
		std::vector<QPointF> avoidObstacles(const std::vector<QPointF> &route, const QPointF *dirs, const double *stubs);

		/*! \brief Discards the displacements of the segments if any of the tables moved or changed its size
		 * since the line was last configured */
		void discardOutdatedSegmentShifts();

		/*! \brief Returns the area the segments of the orthogonal line can't leave when displaced: the area of
		 * the base route and of the tables linked by the relationship, plus a margin */
		QRectF getSegmentShiftLimits();

		/*! \brief Returns the displacement limited so the segment idx of the route (a horizontal or vertical one) stays
		 * inside the limits area and its neighbor segments keep the length they need (stubs are the minimum
		 * length of the first and last segments). The route must be the one the displacement is applied to */
		double clampSegmentShift(const std::vector<QPointF> &route, size_t idx, double shift, const double *stubs, const QRectF &limits);

		/*! \brief Returns the route with the segments displaced according to the shifts stored in the relationship.
		 * Shifts that don't match the route (index out of the inner segments or a different orientation) are ignored */
		std::vector<QPointF> applySegmentShifts(const std::vector<QPointF> &route, const double *stubs,
																						const std::map<unsigned, BaseRelationship::SegmentShift> *custom_shifts = nullptr);

		/*! \brief Returns the index of the inner segment of the orthogonal line closest to the point, or -1 if none is
		 * near enough. The horiz parameter receives the orientation of the segment */
		int pickSegment(const QPointF &pnt, bool &horiz);

		/*! \brief Starts the drag of the inner segment of the orthogonal line closest to the point (if any is near enough).
		 * Returns true if a segment was picked */
		bool startSegmentDrag(const QPointF &pnt);

		//! \brief Moves the dragged segment following the point, previewing the line without reconfiguring it
		void dragSegment(const QPointF &pnt);

		//! \brief Stores the displacement of the dragged segment in the relationship and reconfigures the line
		void finishSegmentDrag();

		//! \brief Alerts the scene that the orthogonal line changed so the bumps of the lines can be recalculated
		void notifyRouteChanged();

		//! \brief Defines the positions where the orthogonal line hops over other lines and redraws it
		void setBumps(const std::vector<QPointF> &new_bumps);

		/*! \brief Computes the orthogonal line (horizontal/vertical segments only) between the tables
		 * according to the line connection mode */
		void configureOrthogonalGeometry(LineGeometry &geom);

		/*! \brief Determines the side of the rect (horizontal or vertical, towards the sign direction) where
		 * the orthogonal line leaves it to reach other_rect. Returns false if the rects overlap */
		static bool getEdgeSide(const QRectF &rect, const QRectF &other_rect, bool &horiz, int &sign);

		/*! \brief Returns the position, along the side of rect, where the orthogonal line to other_rect
		 * would be connected if the side had no other lines */
		static double getEdgePreferredPos(const QRectF &rect, const QRectF &other_rect, bool horiz, double border);

		/*! \brief Returns the position, along the side of the table tab_idx, where this relationship is connected.
		 * The lines of all relationships connected to the same side are spread along it to avoid sharing the same point */
		double getEdgeAnchorPos(unsigned tab_idx, bool horiz, int sign);

		/*! \brief Computes the connection points and directions of the orthogonal line at the tables' edges.
		 * The stubs parameter receives the minimum length of the first and last segments of the line (crow's foot room) */
		void configureOrthogonalEdges(LineGeometry &geom, double stubs[2], double &mid_shift);

		/*! \brief Returns the points of the orthogonal line that links p0 to p1 leaving them in the directions dir0 and dir1.
		 * The stub0 and stub1 parameters are the minimum length of the segments at the ends of the line,
		 * and mid_shift displaces the middle segment. Null directions are chosen automatically. */
		static std::vector<QPointF> getOrthogonalRoute(QPointF p0, QPointF dir0, double stub0,
																									 QPointF p1, QPointF dir1, double stub1, double mid_shift);

		//! \brief Adds the connection points at the ends of the line points
		void addEndPoints(LineGeometry &geom);

		//! \brief Computes the connection points at the tables' edges and adds them to the line points
		void configureTableEdgesGeometry(LineGeometry &geom);

		/*! \brief Computes the connection point at the edge of the table tab_idx where the line crosses it.
		 * Returns the point where the line leaves the edge towards the other table */
		QPointF configureEdgeConnection(unsigned tab_idx, const QLineF &line, LineGeometry &geom);

		//! \brief Returns whether the lines of the relationship have descriptors (e.g. crow's foot) at their ends
		bool hasEndDescriptors();

		//! \brief Returns the length factor of the line segment that leaves the table's edge
		double getEdgeSizeFactor(unsigned tab_idx);

		//! \brief Returns the distance from the table's corners where the line can't be connected
		double getEdgeBorderFactor(unsigned tab_idx);

		//! \brief Destroys the fk/pk connector lines (used in modes other than fk-to-pk)
		void removeConnectorLines();

		//! \brief Configures the graphical representation of the points added by the user
		void configureGraphicPoints(const std::vector<QPointF> &points);

		//! \brief Returns the pen used to draw the relationship lines
		QPen getLinePen();

		//! \brief Configures the lines that link the fk/pk columns to their connection points (fk-to-pk mode only)
		void configureConnectorLines(const LineGeometry &geom, const QPen &pen);

		//! \brief Configures the straight segments that go through the provided points
		void configureLineSegments(const std::vector<QPointF> &points, const QPen &pen);

		//! \brief Returns whether the circles at the line ends must be displayed
		bool isLineEndCirclesVisible();

		//! \brief Configures the circles at the line ends (p_central holds the two connection points)
		void configureLineCircles(const QPointF *p_central, const QPen &pen);

		//! \brief Configures the curves (or restores the straight lines when curves are disabled)
		void configureCurves(const LineGeometry &geom);

		//! \brief Configures one curve per segment of an ordinary relationship line
		void configureCurvedLines(const LineGeometry &geom);

		/*! \brief Configures the single curve that traces the whole line with its corners rounded.
		 * It's used by self-relationships and by the orthogonal lines */
		void configurePolylineCurve(const std::vector<QPointF> &points, double start_len = 0, double end_len = 0);

		//! \brief Destroys all curves and makes the straight lines visible again
		void removeCurves();

		//! \brief Configures the specified label's position based as well some styles for it
		void configureLabelPosition(BaseRelationship::LabelId label_id, double x, double y);

		/*! \brief Returns the default pen width size taking into account the screen dpi and the
		 * whether the relationship is identifier or not */
		double getDefaultPenWidth();

		QPainterPath rel_shape;

	protected:
		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
		void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
		void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
		void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
		void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

		/*! \brief (Re)connects the tables to the relationship changing the signals captured.
		This method is called whenever the placeholder usage is toggled. If the placeholders are on
		the the table's signal s_relUpdateRequested() is used otherwise the s_objectMoved() is used */
		void connectTables();

		//! \brief Disconnects the signal handled by the relationship which senders are the tables
		void disconnectTables();

	public:
		RelationshipView(BaseRelationship *rel);

		~RelationshipView() override;

		QPainterPath shape() const override;

		//! \brief Calculates the relationship bounding rect considering all the children objects dimension
		void configureBoundingRect();

		//! \brief Returns the relationship that generates the graphical representation
		BaseRelationship *getUnderlyingObject();

		//! \brief Hides the relationship's name label. This applies to all relationship instances
		static void setHideNameLabel(bool value);

		//! \brief Returns the current visibility state of name label
		static bool isNameLabelHidden();

		//! \brief Defines the style of the lines used by all relationships
		static void setLineStyle(LineStyle style);

		//! \brief Returns the style of the lines used by the relationships
		static LineStyle getLineStyle();

		//! \brief Enables the usage of Crow's foot notation for all relationships
		static void setCrowsFoot(bool value);

		//! \brief Returns the current state of Crow's foot notation usage
		static bool isCrowsFoot();

		/*! \brief Configures the mode in which the lines are connected on tables.
		The first one is the CONNECT_CENTER_PNTS (the classical one) which connects the
		two tables through the center points. The CONNECT_FK_TO_PK is the one with a better
		semantics	and connects the fk columns of receiver table to pk columns on reference table */
		static void setLineConnectionMode(LineConnectionMode mode);

		//! \brief Returns the line connection mode used for the relationships
		static unsigned getLineConnectinMode();

		/*! \brief Returns the connection point for the specified table. The connection point is
		 where the relationship is connected on envolved tables. The point returned deffers depending on the
		 line connection mode used.	*/
		QPointF getConnectionPoint(BaseRelationship::TableId table_idx);

		/*! \brief Forces the line to be completely reconfigured, even if the tables didn't change. It's used
		 * to update the orthogonal lines when the tables standing in their way are moved */
		void invalidateRoute(bool reset_retries = true);

		//! \brief Returns the label through its index
		TextboxView *getLabel(BaseRelationship::LabelId lab_idx);

		//! \brief Returns the current visibility state of the tables in that are linked by the relationship
		bool isTableVisible(BaseRelationship::TableId table_idx);

		void configureObjectShadow(void) = delete;
		void configureObjectSelection(void) = delete;

	public slots:
		//! \brief Configures the relationship line
		void configureLine();

	private slots:
		//! \brief Makes the comple relationship configuration
		void configureObject() override;

	signals:
		void s_relationshipModified(BaseGraphicObject *rel);

	friend class ObjectsScene;
};

#endif
