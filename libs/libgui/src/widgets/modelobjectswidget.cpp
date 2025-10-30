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

#include "modelobjectswidget.h"
#include "tools/databaseimportwidget.h"
#include "guiutilsns.h"
#include "settings/generalconfigwidget.h"
#include "customtablewidget.h"
#include <QScrollBar>

ModelObjectsWidget::ModelObjectsWidget(bool simplified_view, QWidget *parent) : QWidget(parent)
{
	setupUi(this);
	obj_types_wgt = nullptr;
	model_wgt=nullptr;
	db_model=nullptr;
	setModel(db_model);

	QFont fnt = title_lbl->font();
	fnt.setPointSizeF(fnt.pointSizeF() * 0.85);
	title_lbl->setFont(fnt);

	title_wgt->setVisible(!simplified_view);
	this->simplified_view = simplified_view;
	this->save_tree_state = !simplified_view;
	enable_obj_creation=simplified_view;

	select_tb->setVisible(simplified_view);
	cancel_tb->setVisible(simplified_view);
	options_tb->setVisible(!simplified_view);
	visibleobjects_grp->setVisible(false);
	filter_wgt->setVisible(simplified_view);

	if(simplified_view)
		model_objs_grid->setContentsMargins(GuiUtilsNs::LtMargins);

	connect(objectstree_tw, &QTreeWidget::itemPressed, this, &ModelObjectsWidget::selectObject);
	connect(objectstree_tw, &QTreeWidget::itemPressed, this, &ModelObjectsWidget::showObjectMenu);

	connect(objectstree_tw, &QTreeWidget::itemCollapsed, this, [this](){
		objectstree_tw->resizeColumnToContents(0);
	});

	connect(objectstree_tw, &QTreeWidget::itemExpanded, this, [this](){
		objectstree_tw->resizeColumnToContents(0);
	});

	connect(expand_all_tb, &QToolButton::clicked,  this, [this](){
		objectstree_tw->blockSignals(true);
		objectstree_tw->expandAll();
		objectstree_tw->blockSignals(false);
		objectstree_tw->resizeColumnToContents(0);
	});

	connect(collapse_all_tb, &QToolButton::clicked,  this, [this](){
		objectstree_tw->blockSignals(true);
		objectstree_tw->collapseAll();
		objectstree_tw->blockSignals(false);
		objectstree_tw->resizeColumnToContents(0);
	});

	if(!simplified_view)
	{
		obj_types_wgt = new ObjectTypesListWidget(this);
		visibleobjects_grp->layout()->addWidget(obj_types_wgt);

		connect(options_tb, &QToolButton::clicked,this, &ModelObjectsWidget::changeObjectsView);

		connect(obj_types_wgt, &ObjectTypesListWidget::s_typeCheckStateChanged, this, [this](ObjectType obj_type, Qt::CheckState state) {
			setObjectVisible(obj_type, state == Qt::Checked);
			updateObjectsView();
		});

		connect(obj_types_wgt, &ObjectTypesListWidget::s_typesCheckStateChanged, this, [this](Qt::CheckState state) {
			setAllObjectsVisible(state == Qt::Checked);
		});

		connect(objectstree_tw, &QTreeWidget::itemDoubleClicked, this, &ModelObjectsWidget::editObject);
		connect(hide_tb, &QToolButton::clicked, this, &ModelObjectsWidget::hide);

		setAllObjectsVisible(true);	
		objectstree_tw->installEventFilter(this);
		objectstree_tw->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}
	else
	{
		setMinimumSize(250, 300);
		setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowTitleHint);
		setWindowModality(Qt::ApplicationModal);
		connect(objectstree_tw, &QTreeWidget::itemDoubleClicked, this, &ModelObjectsWidget::close);
		connect(select_tb, &QToolButton::clicked, this, &ModelObjectsWidget::close);
		connect(cancel_tb, &QToolButton::clicked, this, &ModelObjectsWidget::close);
	}

	connect(filter_edt, &QLineEdit::textChanged, this, &ModelObjectsWidget::filterObjects);
	connect(by_id_chk, &QCheckBox::toggled, this, &ModelObjectsWidget::filterObjects);
}

bool ModelObjectsWidget::eventFilter(QObject *object, QEvent *event)
{
	if(event->type() == QEvent::FocusOut && object == objectstree_tw)
	{
		QFocusEvent *evnt = dynamic_cast<QFocusEvent *>(event);

		if(evnt->reason() == Qt::MouseFocusReason)
		{
			clearSelectedObject();

			if(model_wgt)
				model_wgt->configurePopupMenu(nullptr);

			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

void ModelObjectsWidget::hide()
{
	QWidget::hide();
	emit s_visibilityChanged(false);
}

void ModelObjectsWidget::show()
{
	QWidget::show();

	/* When in simplified view we use an event loop to block the execution in
	 * the show event until the user takes an action in the widget that
	 * causes its closing */
	if(simplified_view)
	{
		connect(this, qOverload<BaseObject *, bool>(&ModelObjectsWidget::s_visibilityChanged), this, [this](BaseObject *, bool visible) {
			if(!visible)
				event_loop.quit();
		});

		event_loop.exec();
	}
}

void ModelObjectsWidget::showObjectMenu()
{
	if(!selected_objs.empty() &&
		 QApplication::mouseButtons()==Qt::RightButton &&
		 model_wgt && !simplified_view)
	{
		model_wgt->showObjectMenu();
	}
}

void ModelObjectsWidget::editObject()
{
	if(selected_objs.size() == 1 && model_wgt && !simplified_view)
	{
		//If the user double-clicked the item "Permission (n)" on tree view
		if(sender()==objectstree_tw && objectstree_tw->currentItem() &&
			 objectstree_tw->currentItem()->data(1, Qt::UserRole).toUInt() == enum_t(ObjectType::Permission))
			model_wgt->showObjectForm(ObjectType::Permission, getTreeItemData(objectstree_tw->currentItem()));
		//If the user double-clicked a permission on  list view
		else
			model_wgt->editObject();

		clearSelectedObject();
	}
}

void ModelObjectsWidget::selectObject()
{
	BaseObject *selected_obj = nullptr;
	ObjectType obj_type = ObjectType::BaseObject;
	ModelWidget *model_wgt = nullptr;
	QList<QTreeWidgetItem *> sel_items = objectstree_tw->selectedItems();

	if(!simplified_view && this->model_wgt)
		model_wgt = this->model_wgt;
	else if(simplified_view)
		model_wgt = db_model->getModelWidget();

	selected_objs.clear();
	QTreeWidgetItem *tree_item = objectstree_tw->currentItem();

	/* Avoiding selecting the object if the model has
	 * interaction disabled */
	if(!model_wgt ||
		 (model_wgt && !model_wgt->isInteractive()))
		return;

	if(tree_item)
	{
		obj_type = static_cast<ObjectType>(tree_item->data(1, Qt::UserRole).toUInt());

		for(auto &item : sel_items)
		{
			selected_obj = getTreeItemData(item);

			if(selected_obj)
				selected_objs.push_back(selected_obj);
		}
	}

	//If user select a group item popups a "New [OBJECT]" menu
	if((!simplified_view || (simplified_view && enable_obj_creation)) &&
			selected_objs.empty() && QApplication::mouseButtons() == Qt::RightButton &&
			obj_type != ObjectType::Column && obj_type != ObjectType::Constraint && obj_type != ObjectType::Rule &&
			obj_type != ObjectType::Index && obj_type != ObjectType::Trigger && obj_type != ObjectType::Permission)
	{
		QAction act, *p_act = nullptr;
		QMenu popup;

		//If not a relationship, connect the action to the addNewObject method of the model wiget
		if(obj_type != ObjectType::Relationship)
		{
			act.setData(QVariant(enum_t(obj_type)));
			p_act = &act;
			connect(p_act, &QAction::triggered, model_wgt, &ModelWidget::addNewObject);
		}
		//Case is a relationship, insert the relationship menu of the model wiget into the action
		else
			p_act = model_wgt->rels_menu->menuAction();

		if(simplified_view && enable_obj_creation)
			connect(model_wgt->getDatabaseModel(), &DatabaseModel::s_objectAdded, this, &ModelObjectsWidget::selectCreatedObject, Qt::QueuedConnection);

		p_act->setIcon(QIcon(GuiUtilsNs::getIconPath(obj_type)));
		p_act->setText(tr("New") + " " + BaseObject::getTypeName(obj_type));
		popup.addAction(p_act);
		popup.exec(QCursor::pos());
		disconnect(p_act, nullptr, model_wgt, nullptr);
		disconnect(model_wgt->getDatabaseModel(), nullptr, this, nullptr);
	}

	if(obj_type != ObjectType::Permission && !selected_objs.empty() && !simplified_view)
	{
		model_wgt->scene->blockSignals(true);
		model_wgt->scene->clearSelection();
		model_wgt->scene->blockSignals(false);

		/* If the user has selected only one graphical object in the tree and is holding
		 * Alt key then the object will be highlighted in the canvas */
		if(selected_objs.size() == 1 && qApp->keyboardModifiers() == Qt::AltModifier)
		{
			BaseGraphicObject *graph_obj = dynamic_cast<BaseGraphicObject *>(selected_objs.at(0));

			if(!graph_obj)
				return;

			QGraphicsItem *scn_item = dynamic_cast<QGraphicsItem *>(graph_obj->getOverlyingObject());

			scn_item->setSelected(true);
			model_wgt->scene->views().at(0)->centerOn(scn_item);
		}

		model_wgt->configurePopupMenu(selected_objs);
		model_wgt->emitSceneInteracted();
	}
}

BaseObject *ModelObjectsWidget::getTreeItemData(QTreeWidgetItem *item)
{
 if(!item)
	 return nullptr;

 return reinterpret_cast<BaseObject *>(item->data(0,Qt::UserRole).value<void *>());
}

QVariant ModelObjectsWidget::generateItemData(BaseObject *object)
{
	return QVariant::fromValue(reinterpret_cast<void *>(object));
}

QTreeWidgetItem *ModelObjectsWidget::createItemForObject(BaseObject *object, QTreeWidgetItem *root, bool update_perms)
{
	QTreeWidgetItem *item=nullptr;
	QFont font;
	ObjectType obj_type;
	TableObject *tab_obj=nullptr;
	QString obj_name;

	if(!object)
		throw Exception(ErrorCode::OprNotAllocatedObject ,PGM_FUNC,PGM_FILE,PGM_LINE);

	obj_type=object->getObjectType();
	tab_obj=dynamic_cast<TableObject *>(object);
	item=new QTreeWidgetItem(root);

	if(BaseFunction::isBaseFunction(obj_type))
	{
		BaseFunction *func = dynamic_cast<BaseFunction *>(object);
		func->createSignature(false);
		item->setText(0,func->getSignature());
		obj_name = func->getSignature();
		func->createSignature(true);
	}
	else if(obj_type==ObjectType::Operator)
	{
		Operator *oper=dynamic_cast<Operator *>(object);
		item->setText(0, oper->getSignature(false));
		obj_name=oper->getSignature(false);
	}
	else if(obj_type==ObjectType::OpClass || obj_type == ObjectType::OpFamily)
	{
		obj_name=object->getSignature(false);
		obj_name.replace(QRegularExpression("( )+(USING)( )+"), " [");
		obj_name+=QChar(']');
		item->setText(0,obj_name);
	}
	else
	{
		item->setText(0,object->getName());
		obj_name=object->getName();
	}

	item->setToolTip(0, QString("%1 (id: %2)").arg(obj_name).arg(object->getObjectId()));
	item->setData(0, Qt::UserRole, generateItemData(object));
	item->setData(2, Qt::UserRole, QString("%1_%2").arg(object->getObjectId()).arg(object->getSchemaName()));

	if(update_perms)
		updatePermissionTree(item, object);

	font=item->font(0);
	font.setStrikeOut(object->isSQLDisabled() && !object->isSystemObject());

	if(tab_obj && tab_obj->isAddedByRelationship())
	{
		font.setItalic(true);
		item->setForeground(0, CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemAltFgColor));
	}
	else if(object->isProtected() || object->isSystemObject())
	{
		font.setItalic(true);
		item->setForeground(0, CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemAltFgColor));
	}

	item->setFont(0,font);

	int sub_type = -1;

	if(obj_type == ObjectType::BaseRelationship || obj_type == ObjectType::Relationship)
		sub_type = dynamic_cast<BaseRelationship *>(object)->getRelationshipType();
	else if(obj_type == ObjectType::Constraint)
		sub_type = dynamic_cast<Constraint *>(object)->getConstraintType().getTypeId();

	item->setIcon(0, QIcon(GuiUtilsNs::getIconPath(obj_type, sub_type)));
	item->setData(2, Qt::UserRole, QString("%1_%2").arg(object->getObjectId()).arg(object->getSchemaName()));

	return item;
}

void ModelObjectsWidget::setObjectVisible(ObjectType obj_type, bool visible)
{
	if(obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::BaseTable)
		visible_objs_map[obj_type]=visible;

	if(visible && simplified_view)
	{
		if(obj_type!=ObjectType::Database)
			visible_objs_map[ObjectType::Database]=true;

		if(TableObject::isTableObject(obj_type))
		{
			visible_objs_map[ObjectType::Table]=visible_objs_map[ObjectType::Schema]=true;

			if(obj_type == ObjectType::Column ||
				 obj_type == ObjectType::Constraint ||
				 obj_type == ObjectType::Trigger)
				visible_objs_map[ObjectType::ForeignTable]=true;
		}

		if(BaseObject::acceptsSchema(obj_type))
			visible_objs_map[ObjectType::Schema]=true;
	}
}

void ModelObjectsWidget::setAllObjectsVisible(bool value)
{
	for(auto &type : BaseObject::getObjectTypes(true, { ObjectType::BaseRelationship }))
		visible_objs_map[type] = value;

	updateObjectsView();
}

void ModelObjectsWidget::changeObjectsView()
{
	if(sender()==options_tb)
	{
		filter_wgt->setVisible(options_tb->isChecked());
		visibleobjects_grp->setVisible(options_tb->isChecked());
	}

	expand_all_tb->setEnabled(true);
	collapse_all_tb->setEnabled(true);
}

void ModelObjectsWidget::collapseAll()
{
	QTreeWidgetItem *root=objectstree_tw->topLevelItem(0);
	objectstree_tw->collapseAll();

	if(root)
		root->setExpanded(true);
}

void ModelObjectsWidget::filterObjects()
{
	DatabaseImportWidget::filterObjects(objectstree_tw, filter_edt->text(), (by_id_chk->isChecked() ? 1 : 0), simplified_view);
}

void ModelObjectsWidget::updateObjectsView()
{
	selected_objs.clear();
	updateDatabaseTree();

	if(!filter_edt->text().isEmpty())
		filterObjects();
}

void ModelObjectsWidget::updateSchemaTree(QTreeWidgetItem *root)
{
	if(db_model && visible_objs_map[ObjectType::Schema])
	{
		BaseObject *schema=nullptr;
		std::vector<BaseObject *> obj_list;
		QFont font;
		QTreeWidgetItem *item = nullptr, *item1=nullptr, *item2=nullptr;
		std::vector<ObjectType> types = BaseObject::getChildObjectTypes(ObjectType::Schema);
		int count = 0;
		QPixmap group_icon=QPixmap(GuiUtilsNs::getIconPath(QString(BaseObject::getSchemaName(ObjectType::Schema))));

		//Removing the ObjectType::Table and ObjectType::View types since they are handled separetedly
		types.erase(std::find(types.begin(), types.end(), ObjectType::Table));
		types.erase(std::find(types.begin(), types.end(), ObjectType::ForeignTable));
		types.erase(std::find(types.begin(), types.end(), ObjectType::View));

		//Get the current schema count on database
		count = (db_model->getObjectCount(ObjectType::Schema));
		item = new QTreeWidgetItem(root);
		item->setIcon(0,group_icon);
		item->setData(1, Qt::UserRole, QVariant(enum_t(ObjectType::Schema)));

		//Create the schema group item
		item->setText(0, QString("%1 (%2)").arg(BaseObject::getTypeName(ObjectType::Schema)).arg(count));
		item->setData(2, Qt::UserRole, QString("%1_grp").arg(BaseObject::getSchemaName(ObjectType::Schema)));

		font = item->font(0);
		font.setItalic(true);
		item->setFont(0, font);

		try
		{
			for(auto &sch_obj : *db_model->getObjectList(ObjectType::Schema))
			{
				schema = dynamic_cast<Schema *>(sch_obj);
				item1 = createItemForObject(schema, item);

				//Updates the table subtree for the current schema
				updateTableTree(item1, schema, ObjectType::Table);

				//Updates the foreign table subtree for the current schema
				updateTableTree(item1, schema, ObjectType::ForeignTable);

				//Updates the view subtree for the current schema
				updateViewTree(item1, schema);

				//Creates the object group at schema level (function, domain, sequences, etc)
				for(auto &type : types)
				{
					if(visible_objs_map[type])
					{
						item2 = new QTreeWidgetItem(item1);
						item2->setIcon(0,QPixmap(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(type))));

						//Get the objects that belongs to the current schema
						obj_list = db_model->getObjects(type, schema);
						item2->setText(0, QString("%1 (%2)").arg(BaseObject::getTypeName(type)).arg(obj_list.size()));
						item2->setData(1, Qt::UserRole, QVariant(enum_t(type)));
						item2->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(schema->getObjectId()).arg(BaseObject::getSchemaName(type)));

						font = item2->font(0);
						font.setItalic(true);
						item2->setFont(0, font);

						for(auto &obj : obj_list)
							createItemForObject(obj, item2);
					}
				}
			}
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}
}

void ModelObjectsWidget::updateTableTree(QTreeWidgetItem *root, BaseObject *schema, ObjectType table_type)
{
	if(db_model && PhysicalTable::isPhysicalTable(table_type) && visible_objs_map[table_type])
	{
		std::vector<BaseObject *> obj_list;
		PhysicalTable *table = nullptr;
		QTreeWidgetItem *item = nullptr, *item1 = nullptr, *item2 = nullptr;
		QFont font;
		std::vector<ObjectType> types = BaseObject::getChildObjectTypes(table_type);
		QPixmap group_icon = QPixmap(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(table_type)));

		try
		{
			//Get all tables that belongs to the specified schema
			obj_list=db_model->getObjects(table_type, schema);

			//Create a table group item
			item=new QTreeWidgetItem(root);
			item->setIcon(0,group_icon);
			item->setText(0, BaseObject::getTypeName(table_type) + QString(" (%1)").arg(obj_list.size()));
			item->setData(1, Qt::UserRole, QVariant(enum_t(table_type)));
			item->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(schema->getObjectId()).arg(BaseObject::getSchemaName(table_type)));

			font=item->font(0);
			font.setItalic(true);
			item->setFont(0, font);

			for(auto &obj : obj_list)
			{
				table = dynamic_cast<PhysicalTable *>(obj);
				item1 = createItemForObject(table, item);

				//Creating the group for the child objects (column, rules, triggers, indexes and constraints)
				for(auto &type : types)
				{
					if(visible_objs_map[type])
					{
						item2 = new QTreeWidgetItem(item1);
						item2->setIcon(0,QPixmap(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(type))));
						font = item2->font(0);
						font.setItalic(true);
						item2->setFont(0, font);

						item2->setText(0, QString("%1 (%2)").arg(BaseObject::getTypeName(type)).arg(table->getObjectCount(type)));
						item2->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(table->getObjectId()).arg(BaseObject::getSchemaName(type)));

						for(auto tab_obj : *table->getObjectList(type))
							createItemForObject(tab_obj, item2);
					}
				}
			}
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}
}

void ModelObjectsWidget::updateViewTree(QTreeWidgetItem *root, BaseObject *schema)
{
	if(db_model && visible_objs_map[ObjectType::View])
	{
		std::vector<BaseObject *> obj_list;
		View *view = nullptr;
		QTreeWidgetItem *item = nullptr, *item1 = nullptr, *item2 = nullptr;
		QFont font;
		std::vector<ObjectType> types = BaseObject::getChildObjectTypes(ObjectType::View);
		QPixmap group_icon = QPixmap(GuiUtilsNs::getIconPath(QString(BaseObject::getSchemaName(ObjectType::View))));

		try
		{
			//Get all views that belongs to the specified schema
			obj_list = db_model->getObjects(ObjectType::View, schema);

			//Create a table group item
			item=new QTreeWidgetItem(root);
			item->setIcon(0, group_icon);
			item->setText(0, BaseObject::getTypeName(ObjectType::View) + QString(" (%1)").arg(obj_list.size()));
			item->setData(1, Qt::UserRole, QVariant(enum_t(ObjectType::View)));
			item->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(schema->getObjectId()).arg(BaseObject::getSchemaName(ObjectType::View)));

			font = item->font(0);
			font.setItalic(true);
			item->setFont(0, font);

			for(auto &obj : obj_list)
			{
				view = dynamic_cast<View *>(obj);
				item1 = createItemForObject(view, item);

				//Creating the group for the child objects (rules, triggers)
				for(auto &type : types)
				{
					if(visible_objs_map[type])
					{
						item2 = new QTreeWidgetItem(item1);
						item2->setIcon(0,QPixmap(GuiUtilsNs::getIconPath(BaseObject::getSchemaName(type))));
						font = item2->font(0);
						font.setItalic(true);
						item2->setFont(0, font);

						item2->setText(0, BaseObject::getTypeName(type) + QString(" (%1)").arg(view->getObjectCount(type)));
						item2->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(view->getObjectId()).arg(BaseObject::getSchemaName(type)));

						for(auto &object : view->getObjects())
							createItemForObject(object, item2);
					}
				}
			}
		}
		catch(Exception &e)
		{
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}
}

void ModelObjectsWidget::updatePermissionTree(QTreeWidgetItem *root, BaseObject *object)
{
	try
	{
		if(db_model && visible_objs_map[ObjectType::Permission] &&
				Permission::acceptsPermission(object->getObjectType()))
		{
			std::vector<Permission *> perms;
			QTreeWidgetItem *item = new QTreeWidgetItem(root);
			QFont font=item->font(0);

			db_model->getPermissions(object, perms);
			item->setIcon(0,QPixmap(GuiUtilsNs::getIconPath("permission")));

			font.setItalic(true);
			item->setFont(0, font);
			item->setText(0, QString("%1 (%2)")
											.arg(BaseObject::getTypeName(ObjectType::Permission))
											.arg(perms.size()));

			item->setData(0, Qt::UserRole, generateItemData(object));
			item->setData(1, Qt::UserRole, static_cast<unsigned>(ObjectType::Permission));
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ModelObjectsWidget::updateDatabaseTree()
{
	if(!db_model)
		objectstree_tw->clear();
	else
	{
		QString str_aux;
		QTreeWidgetItem *root=nullptr,*item1=nullptr, *item2=nullptr;
		QFont font;
		std::vector<BaseObject *> obj_list;
		std::vector<ObjectType> types = BaseObject::getChildObjectTypes(ObjectType::Database);
		int tree_v_pos = 0;
		QStringList tree_state;

		objectstree_tw->setUpdatesEnabled(false);

		types.push_back(ObjectType::Tag);
		types.push_back(ObjectType::GenericSql);
		types.push_back(ObjectType::Textbox);
		types.push_back(ObjectType::Relationship);
		types.erase(std::find(types.begin(), types.end(), ObjectType::Schema));

		if(save_tree_state)
			saveTreeState(tree_state, tree_v_pos);

		try
		{
			objectstree_tw->clear();

			if(visible_objs_map[ObjectType::Database])
			{
				root = createItemForObject(db_model);
				objectstree_tw->insertTopLevelItem(0, root);

				updateSchemaTree(root);

				for(auto &type : types)
				{
					if(visible_objs_map[type])
					{
						item1 = new QTreeWidgetItem(root);
						str_aux = QString(BaseObject::getSchemaName(type));

						item1->setIcon(0, QPixmap(GuiUtilsNs::getIconPath(str_aux)));
						item1->setData(1, Qt::UserRole, QVariant(enum_t(type)));

						obj_list=(*db_model->getObjectList(type));

						//Special case for relationship, merging the base relationship list to the relationship list
						if(type == ObjectType::Relationship)
						{
							std::vector<BaseObject *> obj_list_aux;
							obj_list_aux=(*db_model->getObjectList(ObjectType::BaseRelationship));
							obj_list.insert(obj_list.end(), obj_list_aux.begin(), obj_list_aux.end());
						}

						item1->setText(0, BaseObject::getTypeName(type) + QString(" (%1)").arg(obj_list.size()));
						item1->setData(2, Qt::UserRole, QString("%1_%2_grp").arg(db_model->getObjectId()).arg(BaseObject::getSchemaName(type)));
						font = item1->font(0);
						font.setItalic(true);
						item1->setFont(0, font);

						for(auto &object : obj_list)
						{
							item2 = createItemForObject(object, item1);

							if(type == ObjectType::Tag)
							{
								for(auto &ref : object->getReferences())
									createItemForObject(ref, item2, false);
							}
						}
					}
				}

				objectstree_tw->expandItem(root);
			}
		}
		catch(Exception &e)
		{
			objectstree_tw->setUpdatesEnabled(true);
			Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
		}

		objectstree_tw->sortByColumn(0, Qt::AscendingOrder);
		objectstree_tw->setUpdatesEnabled(true);

		if(save_tree_state)
			restoreTreeState(tree_state, tree_v_pos);
	}
}

BaseObject *ModelObjectsWidget::getSelectedObject()
{
	if(!simplified_view || selected_objs.size() != 1)
		return nullptr;

	return selected_objs[0];
}

void ModelObjectsWidget::enableObjectCreation(bool value)
{
  enable_obj_creation=value;
}

void ModelObjectsWidget::close()
{
	QObject *obj_sender=sender();

	if(obj_sender == cancel_tb)
		selected_objs.clear();
	else
	{
		BaseObject *selected_obj = nullptr;

		selected_obj = getTreeItemData(objectstree_tw->currentItem());

		if(selected_obj && std::find(selected_objs.begin(), selected_objs.end(), selected_obj) == selected_objs.end())
			selected_objs.push_back(selected_obj);
	}

	QWidget::close();
}

void ModelObjectsWidget::setModel(ModelWidget *model_wgt)
{
	this->model_wgt=model_wgt;

	if(model_wgt)
		setModel(model_wgt->db_model);
	else
		setModel(static_cast<DatabaseModel *>(nullptr));
}

void ModelObjectsWidget::setModel(DatabaseModel *db_model)
{
	bool enable = (db_model!=nullptr);

	this->db_model=db_model;
	content_frm->setEnabled(enable);
	updateObjectsView();	
	expand_all_tb->setEnabled(enable);
	collapse_all_tb->setEnabled(enable);
	options_tb->setEnabled(enable);
	filter_lbl->setEnabled(enable);
	filter_edt->setEnabled(enable);
	by_id_chk->setEnabled(enable);
}

void ModelObjectsWidget::showEvent(QShowEvent *)
{
	if(simplified_view)
	{
		QWidget *wgt = QApplication::activeWindow();

		filter_edt->setFocus();
		filter_edt->blockSignals(true);
		by_id_chk->blockSignals(true);
		filter_edt->clear();
		by_id_chk->setChecked(false);
		filter_edt->blockSignals(false);
		by_id_chk->blockSignals(false);

		GeneralConfigWidget::restoreWidgetGeometry(this, this->metaObject()->className());

		if(wgt)
		{
			int x = wgt->pos().x() + abs((wgt->width() - this->width()) / 2),
					y = wgt->pos().y() + abs((wgt->height() - this->height()) / 2);
			this->move(x, y);
		}
	}
}

void ModelObjectsWidget::closeEvent(QCloseEvent *)
{
	if(simplified_view)
	{
		std::map<ObjectType, bool>::iterator itr, itr_end;

		itr=visible_objs_map.begin();
		itr_end=visible_objs_map.end();

		while(itr!=itr_end)
		{
			itr->second=false;
			itr++;
		}

		GeneralConfigWidget::saveWidgetGeometry(this, this->metaObject()->className());
	}

	emit s_visibilityChanged(getSelectedObject(), !this->isVisible());
}

void ModelObjectsWidget::mouseMoveEvent(QMouseEvent *)
{
	static QPoint pos=QCursor::pos(), pos1=QCursor::pos();

	pos=pos1;
	pos1=QCursor::pos();

	if(simplified_view && QApplication::mouseButtons()==Qt::LeftButton)
	{
		QPoint pos_dif;
		QRect ret = this->screen()->geometry();
		int px, py;

		pos_dif=pos1-pos;
		px=this->pos().x() + pos_dif.x();
		py=this->pos().y() + pos_dif.y();

		if(px<0)
			px=0;
		else if((px + this->width()) > ret.right())
			px=ret.right() - this->width();

		if(py<0)
			py=0;
		else if((py + this->height()) > ret.bottom())
			py=ret.bottom() - this->height();

		this->move(px,py);
	}
}

void ModelObjectsWidget::resizeEvent(QResizeEvent *)
{
	objectstree_tw->header()->setMinimumSectionSize(objectstree_tw->width());
	objectstree_tw->header()->setDefaultSectionSize(objectstree_tw->width());
}

void ModelObjectsWidget::saveTreeState(bool value)
{
	save_tree_state=(!simplified_view && value);
}

void ModelObjectsWidget::clearSelectedObject()
{
	objectstree_tw->blockSignals(true);
	objectstree_tw->clearSelection();
	objectstree_tw->blockSignals(false);
	selected_objs.clear();
	model_wgt->configurePopupMenu(nullptr);
	model_wgt->emitSceneInteracted();
}

void ModelObjectsWidget::saveTreeState(QStringList &exp_items_ids, int &v_scroll_pos)
{
	QTreeWidgetItemIterator itr(objectstree_tw);
	QTreeWidgetItem *item=nullptr;

	while(*itr)
	{
		item = *itr;

		if(item->isExpanded())
			exp_items_ids.push_back(item->data(2, Qt::UserRole).toString());

		++itr;
	}

	v_scroll_pos = objectstree_tw->verticalScrollBar()->value();
}

void ModelObjectsWidget::restoreTreeState(const QStringList &exp_items_ids, int v_scroll_pos)
{
	QTreeWidgetItem *item = nullptr;

	for(auto &item_id : exp_items_ids)
	{
		item = getTreeItem(item_id);

		if(!item)
			continue;

		objectstree_tw->expandItem(item);
	}

	objectstree_tw->verticalScrollBar()->setValue(v_scroll_pos);
}

QTreeWidgetItem *ModelObjectsWidget::getTreeItem(BaseObject *object)
{
	if(!object)
		return nullptr;

	QTreeWidgetItemIterator itr(objectstree_tw);

	while(*itr)
	{
		if(getTreeItemData(*itr) == object)
			return *itr;

		++itr;
	}

	return nullptr;
}

QTreeWidgetItem *ModelObjectsWidget::getTreeItem(const QString &item_id)
{
	QTreeWidgetItemIterator itr(objectstree_tw);
	QTreeWidgetItem *item = nullptr;

	while(*itr)
	{
		if((*itr)->data(2, Qt::UserRole).toString() == item_id)
		{
			item = *itr;
			break;
		}

		++itr;
	}

	return item;
}

void ModelObjectsWidget::selectCreatedObject(BaseObject *obj)
{
	updateObjectsView();
	QTreeWidgetItem *item = getTreeItem(obj);

	if(item)
	{
		objectstree_tw->blockSignals(true);
		item->setSelected(true);
		objectstree_tw->setCurrentItem(item);
		objectstree_tw->scrollToItem(item);
		select_tb->setFocus();
		objectstree_tw->blockSignals(false);
	}
}
