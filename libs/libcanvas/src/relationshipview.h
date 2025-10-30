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

	private:
		//! \brief Graphical point radius
		static constexpr double GraphicPointRadius=6.0;

		/*! \brief The width of the path stroke used to select relationship line
		 *  as well as add points to it */
		static constexpr double LineSelStrokeWidth=15;

		//! \brief Length of the lines linked to fk/pk columns
		static constexpr double ConnLineLength=20.0;

		//! \brief Indicates that the relationship labels must be hidden
		static bool hide_name_label,

		//! \brief Indicates that the relationship lines should be curved
		use_curved_lines,

		//! \brief Indicates that the relationship should be drawn in Crow's foot notation
		use_crows_foot;

		/*! \brief Specify the type of connection used by the lines. The first (classical)
		is to connect the line to tables through their central points. The second (better semantics)
		makes the line start from the fk columns on receiver table and connecting to the pk columns on reference table */
		static LineConnectionMode line_conn_mode;

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

		//! \brief Enables the usage of curved lines for all relationships
		static void setCurvedLines(bool value);

		//! \brief Returns the current state of curved lines usage
		static bool isCurvedLines();

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
