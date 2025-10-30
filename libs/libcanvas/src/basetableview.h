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
\class BaseTableView
\brief Base class shared by the classes TableView and GraphicalView
*/

#ifndef BASE_TABLE_VIEW_H
#define BASE_TABLE_VIEW_H

#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include "baseobjectview.h"
#include "basetable.h"
#include "tabletitleview.h"
#include "tableobjectview.h"
#include "roundedrectitem.h"
#include "baserelationship.h"
#include "textpolygonitem.h"
#include "attributestoggleritem.h"

class __libcanvas BaseTableView: public BaseObjectView {
	Q_OBJECT

	private:
		/*! \brief This timer is used to control the selection enabling of the object
		 * in mouse presse event in order to avoid select it instead of a child.
		 * See mousePressEvent() for details */
		QTimer sel_enabler_timer;

		/*! \brief Stores the references to the relationships connected to this table. */
		std::vector<BaseRelationship *> connected_rels;

	protected:		
		//! \brief Stores the selected child objects in order to retrieve them in ObjectScene/ModelWidget
		QList<TableObjectView *> sel_child_objs;

		/*! \brief This attributes indicates that the object's geometry update is pending demanding a
		 * call to configureObject(). This attribute is set to true only when the objects is invisible
		 * the the configureObject is called. Once the object gets visible again this attribute is set
		 * to false and the geometry updated immediately (see BaseTableView::itemChange()) */
		bool pending_geom_update;

		//! \brief Item groups that stores columns and extended attributes, respectively
		QGraphicsItemGroup *columns,

		*ext_attribs;

		//! brief Indicates if the extended attributes body should be hidden
		static bool hide_ext_attribs,

		//! brief Indicates if the tag object should be hidden
		hide_tags;

		//! brief Controls the maximum amount of attributes visible per page (columns/references + extended attributes)
		static unsigned attribs_per_page[2];

		//! \brief Polygonal object that defines the table body
		RoundedRectItem *body,

		//! \brief Extended table attributes (indexes, rules, triggers) section body
		*ext_attribs_body;

		AttributesTogglerItem *attribs_toggler;

		TextPolygonItem *tag_item;

		//! \brief Stores the reference to the child object currently selected on table
		TableObjectView *sel_child_obj_view;

		//! \brief Table title
		TableTitleView *title;

		//! \brief Stores the original table's tool tip
		QString table_tooltip;

		QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

		void addConnectedRelationship(BaseRelationship *base_rel);

		void removeConnectedRelationship(BaseRelationship *base_rel);

		/*! \brief Returns the index of the relationship in the list of the connected relationships
		 * If 'only_self_rels' is true then only self relationship are searched */
		int getConnectedRelationshipIndex(BaseRelationship *base_rel, bool only_self_rels = false);

		//! \brief Configures the tag object when the source object has one.
		void configureTag();

		/*! \brief Configures basic attributes of the table. A width need to be provided so
		the extended attributes section can follow the same width as the body and title */
		void __configureObject(double width);

		//! \brief Determines the table width based upon its subsection (title, body and extended attribs)
		double calculateWidth();

		/*! \brief This as an auxiliary method called before the object changes its dimensions and it causes
		 * the object to not being selectable. This method is called whenever one of the signals are captured
		 * from the attributes toggler: s_paginationToggled s_currentPageChanged s_collapseModeChanged */
		void startGeometryUpdate();

		/*! \brief This auxiliary method is called after any geometry change finishes forcing the update of the object
		 * and in some cases the schema box which contains it */
		void finishGeometryUpdate();

		/*! \brief Determines the pagination paramenters for a section of the table. The input parameters are
		 * the section (BaseTable::AttribsSection | ExtAttribsSection) and the total amount of attributes in the section.
		 * The other paramenters start_attr and end_attr are reference parameters that will hold the indexes of items
		 * to be displayed in the current page. See configureObject() on TableView and GraphicalView */
		bool configurePaginationParams(BaseTable::TableSection section_id, unsigned total_attrs, unsigned &start_attr, unsigned &end_attr);

	public:
		enum ConnectionPoint: unsigned {
			LeftConnPoint,
			RightConnPoint
		};

		BaseTableView(BaseTable *base_tab);
		~BaseTableView() override;

		void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
		void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
		void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

		//! brief Defines the amount of attributes per page to be displayed
		static void setAttributesPerPage(BaseTable::TableSection section_id, unsigned value);

		//! brief Returns the current amount of attributes per page to be displayed
		static unsigned getAttributesPerPage(BaseTable::TableSection section_id);

		//! \brief Hides the table's extended attributes (rules, triggers, indexes). This applies to all table/view instances
		static void setHideExtAttributes(bool value);

		//! \brief Hides the table tags. This applies to all table instances
		static void setHideTags(bool value);

		//! \brief Returns the current visibility state of extended attributes
		static bool isExtAttributesHidden();

		//! \brief Returns the current visibility state of tags
		static bool isTagsHidden();

		//! \brief Returns the current count of connected relationships
		int getConnectRelsCount();

		//! \brief This method just emits the signal to indicate that the relationships attached must be updated
		void requestRelationshipsUpdate();

		//! \brief Toggles the placeholder object when there is at least one relationship connected to the object
		void togglePlaceholder(bool value) override;

		unsigned getConnectedRelsCount(BaseTable *src_tab, BaseTable *dst_tab);

		//! \brief Configures the shadow for the table
		void configureObjectShadow();

		//! \brief Returns a list of selected children objects
		QList<TableObjectView *> getSelectedChidren();

		//! \brief Clear the selection over all selected children
		void clearChildrenSelection();

		//! \brief Selects all the relationships connected to the table
		void selectRelationships();

		//! \brief Selects the child item view in the table view that is related to tab_obj
		void setChildSelected(TableObject *tab_obj);

	private slots:
		/*! \brief This slot reconfigures the table when the attributes toggler emits the signal s_collapseModeChanged
		 * hiding or exposing the sections related to the current collapse mode */
		void configureCollapsedSections(BaseTable::CollapseMode coll_mode);

		//! \brief This slot reconfigures the table when the attributes toggler emits the signal s_paginationToggled
		void togglePagination(bool enabled);

		/*! \brief This slot reconfigures the table when the attributes toggler emits the signal s_currentPageChanged
		 * causing the the attributes of the current page to be displayed */
		void configureCurrentPage(BaseTable::TableSection section_id, unsigned page);

	signals:
		//! \brief Signal emitted when a table is moved over the scene
		void s_objectMoved();

		//! \brief Signal emitted to indicate that the relationships attached to the table need to be updated
		void s_relUpdateRequest();

		//! \brief Signal emitted when the user right-click a focused table child object requesting a popup menu
		void s_popupMenuRequested(TableObject *);

		//! \brief Signal emitted when the user clicks a focused table child object and holding Control+Shift
		void s_childrenSelectionChanged();

		//! \brief Signal emitted when the user toggles the table's collapse mode
		void s_collapseModeChanged();

		//! \brief Signal emitted when the user toggles the table's attributes pagination
		void s_paginationToggled();

		//! \brief Signal emitted when the user changes the current table's attributes page
		void s_currentPageChanged();

		//! \brief Signal emitted when the object need the scene to clear its selection
		void s_sceneClearRequested();

		friend class RelationshipView;
};

#endif
