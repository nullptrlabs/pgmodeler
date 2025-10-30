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
\class ModelObjectsWidget
\brief Implements a widget that permits an tree and list view of all model objects.
*/

#ifndef MODEL_OBJECTS_WIDGET_H
#define MODEL_OBJECTS_WIDGET_H

#include "ui_modelobjectswidget.h"
#include "modelwidget.h"
#include "objecttypeslistwidget.h"

class __libgui ModelObjectsWidget: public QWidget, public Ui::ModelObjectsWidget {
	Q_OBJECT

	private:
		/*! \brief This event loop is used to make the widget enter in a loop (like QDialogs) when
		 *  the show() is called and the widget is configured as a simplified view */
		QEventLoop event_loop;

		/*! \brief Indicates if the widget must be used as a simplified view (without the most interactions).
		The purpose to use it as simplified view is to be serve as a object pick commonly used on the
		object selectors. See ObjectSelectorWidget for details. */
		bool simplified_view,

		/*! \brief Indicates if the object tree state must be saved, this means, that the current item
					expansion is memorized and can be restored at any moment via restoreTreeState() method */
		save_tree_state,

		/*! \brief Allow the object creation in simplified mode by using the "New [object type]" popup menu.
		This flag is ignored if the model object widget is used in the complete mode since the main purpose
		of the widget is to allow the object management */
		enable_obj_creation;

		//! \brief Stores the objects currently selected on the tree/list
		std::vector<BaseObject *> selected_objs;

		//! \brief Reference model widget. This is the object used to populate the tree and list
		ModelWidget *model_wgt;

		//! \brief Reference database model. This is the object used to populate the tree and list
		DatabaseModel *db_model;

		//! \brief Stores which object types are visible on the view
		std::map<ObjectType, bool> visible_objs_map;

		ObjectTypesListWidget *obj_types_wgt;

		//! \brief Updates only a schema tree starting from the 'root' item
		void updateSchemaTree(QTreeWidgetItem *root);

		/*! \brief Updates only a table / foreign table tree starting from the 'root' item.
		 * table_type must be ObjectType::Table or ObjectType::ForeignTable */
		void updateTableTree(QTreeWidgetItem *root, BaseObject *schema, ObjectType table_type);

		//! \brief Updates only a view tree starting from the 'root' item
		void updateViewTree(QTreeWidgetItem *root, BaseObject *schema);

		//! \brief Updates only the permission tree related to the specified object
		void updatePermissionTree(QTreeWidgetItem *root, BaseObject *object);

		//! \brief Updates the whole database object tree
		void updateDatabaseTree();

		//! \brief Updates the whole object list
		void updateObjectsTable();

		//! \brief Returns an item from the tree related to the specified object reference
		QTreeWidgetItem *getTreeItem(BaseObject *object);

		//! \brief Returns an item from the tree related to the specified item id
		QTreeWidgetItem *getTreeItem(const QString &item_id);

		//! \brief Generates a QVariant containing the passed object reference as data
		QVariant generateItemData(BaseObject *object);

		BaseObject *getTreeItemData(QTreeWidgetItem *item);

		QTreeWidgetItem *createItemForObject(BaseObject *object, QTreeWidgetItem *root=nullptr, bool update_perms=true);

		void mouseMoveEvent(QMouseEvent *) override;
		void resizeEvent(QResizeEvent *) override;
		void closeEvent(QCloseEvent *) override;
		void showEvent(QShowEvent *) override;
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		ModelObjectsWidget(bool simplified_view=false, QWidget * parent = nullptr);

		BaseObject *getSelectedObject();

		//! \brief Enables the object creation in simplified view by exposing the popup menu "New [object]"
		void enableObjectCreation(bool value);

	protected:
		/*! \brief Saves the currently expanded items on the specified vector.
		 *  The vertical scrollbar position is stored in v_scroll_pos */
		void saveTreeState(QStringList &exp_items_ids, int &v_scroll_pos);

		/*! \brief Restores the tree at a previous state when the specified items were expanded
		 *  as well as to a vertical scrollbar position */
		void restoreTreeState(const QStringList &exp_items_ids, int v_scroll_pos);

		//! \brief Defines if the widget must save/restore the tree state automaticaly
		void saveTreeState(bool value);

		void clearSelectedObject();

	public slots:
		void setModel(ModelWidget *model_wgt);
		void setModel(DatabaseModel *db_model);
		void changeObjectsView();
		void updateObjectsView();
		void setObjectVisible(ObjectType obj_type, bool visible);
		void close();
		void hide();
		void show();

	private slots:
		void setAllObjectsVisible(bool value);
		void selectObject();
		void showObjectMenu();
		void editObject();
		void collapseAll();
		void filterObjects();
		void selectCreatedObject(BaseObject *obj);

	signals:
		void s_visibilityChanged(BaseObject *,bool);
		void s_visibilityChanged(bool);

		friend class MainWindow;
};

#endif
