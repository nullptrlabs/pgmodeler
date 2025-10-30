#include "swapobjectsidswidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"

const QString SwapObjectsIdsWidget::IdLabel {"ID: <strong>%1</strong>"};

SwapObjectsIdsWidget::SwapObjectsIdsWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
	std::vector<ObjectType> types=BaseObject::getObjectTypes(true, { ObjectType::Permission,
																																	 ObjectType::Textbox,
																																	 ObjectType::Column,
																																	 ObjectType::Constraint });
	setupUi(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	sort_column = 0;
	sort_order = Qt::AscendingOrder;

	selector_idx = 0;
	src_object_sel=nullptr;
	dst_object_sel=nullptr;

	filter_lt->setAlignment(filter_btn, Qt::AlignLeft);

	QHBoxLayout *hbox = new QHBoxLayout(src_sel_parent);
	hbox->setContentsMargins(0,0,0,0);
	src_object_sel=new ObjectSelectorWidget(types, src_sel_parent);
	src_object_sel->enableObjectCreation(false);
	hbox->addWidget(src_object_sel);

	hbox = new QHBoxLayout(dst_sel_parent);
	hbox->setContentsMargins(0,0,0,0);
	dst_object_sel=new ObjectSelectorWidget(types, dst_sel_parent);
	dst_object_sel->enableObjectCreation(false);
	hbox->addWidget(dst_object_sel);

	setModel(nullptr);
	filter_wgt->setVisible(false);

	connect(filter_btn, &QPushButton::toggled, filter_wgt, &QWidget::setVisible);

	connect(src_object_sel, &ObjectSelectorWidget::s_objectSelected, this, &SwapObjectsIdsWidget::showObjectId);
	connect(dst_object_sel, &ObjectSelectorWidget::s_objectSelected, this, &SwapObjectsIdsWidget::showObjectId);
	connect(src_object_sel, &ObjectSelectorWidget::s_selectorCleared, this, &SwapObjectsIdsWidget::showObjectId);
	connect(dst_object_sel, &ObjectSelectorWidget::s_selectorCleared, this, &SwapObjectsIdsWidget::showObjectId);

	connect(swap_values_tb, &QToolButton::clicked, this, [this](){
		BaseObject *obj = src_object_sel->getSelectedObject();
		src_object_sel->setSelectedObject(dst_object_sel->getSelectedObject());
		dst_object_sel->setSelectedObject(obj);
	});

	connect(objects_view, &QTableView::doubleClicked, this, [this](const QModelIndex &index){
		if(QApplication::mouseButtons() == Qt::LeftButton)
			selectItem(index);
	});

	connect(objects_view->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [this](int index, Qt::SortOrder order){
		sort_column = index;
		sort_order = order;
	});

	connect(filter_edt, &QLineEdit::textChanged, this, &SwapObjectsIdsWidget::filterObjects);
	connect(hide_rels_chk, &QCheckBox::toggled, this, &SwapObjectsIdsWidget::filterObjects);
	connect(hide_sys_objs_chk, &QCheckBox::toggled, this, &SwapObjectsIdsWidget::filterObjects);

	objects_view->installEventFilter(this);
	setMinimumSize(640,480);
}

void SwapObjectsIdsWidget::setModel(DatabaseModel *model)
{
	this->model=model;

	src_object_sel->setModel(model);
	dst_object_sel->setModel(model);

	src_object_sel->clearSelector();
	dst_object_sel->clearSelector();

	fillCreationOrderGrid();
}

void SwapObjectsIdsWidget::setSelectedObjects(BaseObject *src_object, BaseObject *dst_objct)
{
	src_object_sel->setSelectedObject(src_object);
	dst_object_sel->setSelectedObject(dst_objct);
	selector_idx = (src_object && !dst_objct ? 1 : 0);
}

void SwapObjectsIdsWidget::fillCreationOrderGrid()
{
	if(!model)
		return;

	std::map<unsigned, BaseObject *> creation_order = model->getCreationOrder(SchemaParser::SqlCode);
	std::vector<BaseObject *> objects;

	//Using an stl function to extract all the values (objects) from the map and put them into a list
	std::for_each(creation_order.begin(), creation_order.end(), [&objects](const std::pair<unsigned, BaseObject *> &itr) {
		if(itr.second->getObjectType() != ObjectType::Constraint) {
			objects.push_back(itr.second);
		}
	});

	/* Blocking the signals of the horizontal header to avoid calling the slot
	 * that stores the current sort column/order. The slot must be called only
	 * the user clicks the header, not when sortByColumn() is called. */
	objects_view->horizontalHeader()->blockSignals(true);
	GuiUtilsNs::populateObjectsTable(objects_view, objects, "");

	if(!filter_edt->text().isEmpty() || hide_rels_chk->isChecked() || hide_sys_objs_chk->isChecked())
		filterObjects();

	objects_view->sortByColumn(sort_column, sort_order);
	objects_view->horizontalHeader()->blockSignals(false);
}

bool SwapObjectsIdsWidget::eventFilter(QObject *object, QEvent *event)
{
	if(object == objects_view && event->type() == QEvent::KeyPress)
	{
		QKeyEvent *k_event = dynamic_cast<QKeyEvent *>(event);
		QModelIndex index = objects_view->currentIndex();
		int row = index.row();

		if(k_event->key() == Qt::Key_Space)
			selectItem(index);
		else if((k_event->modifiers() == Qt::ControlModifier ||

						/* Workaround for macOS: On some keyboards, Qt seems to detect that modifiers
						 * ControlModifier and KeypadModifier are combined even only Command/Control key is held,
						 * thus, we have to do this extra check */
						k_event->modifiers() == (Qt::ControlModifier | Qt::KeypadModifier)) &&

						(k_event->key() == Qt::Key_Down || k_event->key() == Qt::Key_Up))
		{
			QAbstractItemModel *model = objects_view->model();
			QModelIndex aux_index;
			int key_code = k_event->key();

			clearSelectors();
			selectItem(index);

			while(!aux_index.isValid())
			{
				// If user presses down we get the next item below
				if(key_code == Qt::Key_Down && row < model->rowCount() - 1)
					aux_index = model->index(row + 1, 0);
				// If user presses up we get the next item above
				else if(key_code == Qt::Key_Up && row > 1)
					aux_index = model->index(row - 1, 0);

				// If we reach a hidden row we need to jump to the next one that is not hidden
				if(aux_index.isValid() && objects_view->isRowHidden(aux_index.row()))
				{
					aux_index = QModelIndex();
					row += (key_code == Qt::Key_Down ? 1 : -1);
				}

				// Breaking if we've reached the top or down limits (avoiding swap ids with null objects)
				if((key_code == Qt::Key_Down && row >= model->rowCount() - 1) ||
					 (key_code == Qt::Key_Up && row == 1))
					break;
			}

			if(aux_index.isValid())
			{
				selectItem(aux_index);
				swapObjectsIds();
				clearSelectors();
				objects_view->setCurrentIndex(objects_view->model()->index(row , 0));
			}
		}
	}

	return QWidget::eventFilter(object, event);
}

void SwapObjectsIdsWidget::showObjectId()
{
	QLabel *ico_lbl=nullptr, *id_lbl=nullptr;
	BaseObject *sel_obj=nullptr;

	if(sender()==src_object_sel)
	{
		ico_lbl=src_ico_lbl;
		id_lbl=src_id_lbl;
		sel_obj=src_object_sel->getSelectedObject();
	}
	else
	{
		ico_lbl=dst_ico_lbl;
		id_lbl=dst_id_lbl;
		sel_obj=dst_object_sel->getSelectedObject();
	}

	id_lbl->clear();
	if(sel_obj)
	{
		id_lbl->setText(IdLabel.arg(sel_obj->getObjectId()));
		ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(sel_obj->getObjectType())));
		ico_lbl->setToolTip(sel_obj->getTypeName());

		id_lbl->setVisible(true);
		ico_lbl->setVisible(true);
	}
	else
	{
		id_lbl->setVisible(false);
		ico_lbl->setVisible(false);
	}

	swap_values_tb->setEnabled(src_object_sel->getSelectedObject() &&
														 dst_object_sel->getSelectedObject());

	emit s_objectsIdsSwapReady(src_object_sel->getSelectedObject() &&
														 dst_object_sel->getSelectedObject());
}

void SwapObjectsIdsWidget::swapObjectsIds()
{
	BaseObject *src_obj=src_object_sel->getSelectedObject(),
			*dst_obj=dst_object_sel->getSelectedObject();
	BaseGraphicObject *graph_src_obj=dynamic_cast<BaseGraphicObject *>(src_obj),
			*graph_dst_obj=dynamic_cast<BaseGraphicObject *>(dst_obj);

	if(!src_obj || !dst_obj)
		return;

	//Raise an exception if the user try to swap an id of relationship by other object of different kind
	if((src_obj->getObjectType()==ObjectType::Relationship ||
			dst_obj->getObjectType()==ObjectType::Relationship) &&
		 (src_obj->getObjectType() != dst_obj->getObjectType()))
	{
		Messagebox::error(Exception::getErrorMessage(ErrorCode::InvRelationshipIdSwap),
											ErrorCode::InvRelationshipIdSwap, PGM_FUNC, PGM_FILE, PGM_LINE);
		return;
	}

	try
	{
		qApp->setOverrideCursor(Qt::WaitCursor);
		BaseObject::swapObjectsIds(src_obj, dst_obj, false);

		//Special id swap for relationship
		if(src_obj->getObjectType()==ObjectType::Relationship)
		{
			std::vector<BaseObject *>::iterator itr, itr1;
			std::vector<BaseObject *> *list=model->getObjectList(ObjectType::Relationship);

			//Find the relationships in the list and swap the memory position too
			itr=std::find(list->begin(), list->end(), src_obj);
			itr1=std::find(list->begin(), list->end(), dst_obj);
			(*itr)=dst_obj;
			(*itr1)=src_obj;

			model->validateRelationships();
		}
		else
		{
			if(graph_src_obj)
				graph_src_obj->setModified(true);

			if(graph_dst_obj)
				graph_dst_obj->setModified(true);
		}

		model->setInvalidated(true);
		fillCreationOrderGrid();

		src_id_lbl->setText(IdLabel.arg(src_object_sel->getSelectedObject()->getObjectId()));
		dst_id_lbl->setText(IdLabel.arg(dst_object_sel->getSelectedObject()->getObjectId()));

		qApp->restoreOverrideCursor();
		emit s_objectsIdsSwapped();
	}
	catch(Exception &e)
	{
		//qApp->restoreOverrideCursor();
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void SwapObjectsIdsWidget::filterObjects()
{
	BaseObject *object = nullptr;
	bool is_rel = false, is_sys_obj = false;

	QAbstractItemModel *model = objects_view->model();
	QList<QModelIndex> indexes = model->match(model->index(0,0), Qt::DisplayRole, filter_edt->text(),	-1);
	QModelIndex index;

	for(int row = 0; row < objects_view->model()->rowCount(); row++)
		objects_view->setRowHidden(row, true);

	while(!indexes.isEmpty())
	{
		index = indexes.front();
		object = reinterpret_cast<BaseObject *>(index.data(Qt::UserRole).value<void *>());

		if(!object)
		{
			indexes.pop_front();
			continue;
		}

		is_rel = (object->getObjectType() == ObjectType::BaseRelationship || object->getObjectType() == ObjectType::Relationship);
		is_sys_obj = object->isSystemObject();

		if((!is_rel && !is_sys_obj) ||
			 (!hide_rels_chk->isChecked() && is_rel) ||
			 (!hide_sys_objs_chk->isChecked() && is_sys_obj))
			objects_view->setRowHidden(indexes.front().row(), false);

		indexes.pop_front();
	}
}

void SwapObjectsIdsWidget::selectItem(const QModelIndex &index)
{
	QModelIndex aux_index = index.column() == 0 ? index : objects_view->model()->index(index.row(), 0);
	BaseObject *obj = reinterpret_cast<BaseObject *>(aux_index.data(Qt::UserRole).value<void *>());

	if(selector_idx == 0)
	{
		src_object_sel->setSelectedObject(obj);
		selector_idx = 1;
	}
	else
	{
		dst_object_sel->setSelectedObject(obj);
		selector_idx = 0;
	}
}

void SwapObjectsIdsWidget::clearSelectors()
{
	selector_idx = 0;
	src_object_sel->clearSelector();
	dst_object_sel->clearSelector();
}
