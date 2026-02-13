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

#include "viewwidget.h"
#include "customuistyle.h"
#include "rulewidget.h"
#include "triggerwidget.h"
#include "indexwidget.h"
#include "baseform.h"
#include "coreutilsns.h"
#include "guiutilsns.h"

ViewWidget::ViewWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::View)
{
	QGridLayout *grid = nullptr;
	QVBoxLayout *vbox = nullptr;

	Ui_ViewWidget::setupUi(this);

	alert_frm->setVisible(false);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	check_option_cmb->addItem(tr("No check"));
	check_option_cmb->addItems(CheckOptionType::getTypes());
	check_option_cmb->setCurrentIndex(0);

	sql_definition_txt = GuiUtilsNs::createWidgetInParent<NumberedTextEditor>(GuiUtilsNs::LtMargin, sql_definition_tab, true);
	sql_definition_hl = new SyntaxHighlighter(sql_definition_txt);
	sql_definition_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	std::vector ref_types {
		ObjectType::Schema, ObjectType::Column,
		ObjectType::Table, ObjectType::ForeignTable,
		ObjectType::View, ObjectType::Function,
		ObjectType::Procedure
	};

	obj_refs_wgt = GuiUtilsNs::createWidgetInParent<ReferencesWidget>(GuiUtilsNs::LtMargin,
																																		ref_types,
																																		true, view_refs_tab);

	tag_sel = new ObjectSelectorWidget(ObjectType::Tag, this);
	tag_lt->insertWidget(1, tag_sel);

	custom_cols_wgt = GuiUtilsNs::createWidgetInParent< SimpleColumnsWidget>(GuiUtilsNs::LtMargin,
																																					 columns_tab);

	//Configuring the table objects that stores the triggers and rules
	QWidgetList parent_tab { triggers_tab, rules_tab, indexes_tab };
	CustomTableWidget *tab = nullptr;
	int p_idx = 0;

	for(auto &type : { ObjectType::Trigger, ObjectType::Rule, ObjectType::Index })
	{
		tab = GuiUtilsNs::createWidgetInParent<CustomTableWidget>(GuiUtilsNs::LtMargin,
																															CustomTableWidget::AllButtons ^
																															(CustomTableWidget::UpdateButton  | CustomTableWidget::MoveButtons),
																															true, parent_tab[p_idx++]);
		objects_tab_map[type] = tab;

		connect(tab, &CustomTableWidget::s_rowsRemoved, this, __slot(this, ViewWidget::removeObjects));
		connect(tab, &CustomTableWidget::s_rowRemoved, this, __slot_n(this, ViewWidget::removeObject));
		connect(tab, &CustomTableWidget::s_rowAdded, this, __slot(this, ViewWidget::handleObject));
		connect(tab, &CustomTableWidget::s_rowEdited, this, __slot(this, ViewWidget::handleObject));
		connect(tab, &CustomTableWidget::s_rowDuplicated, this, __slot_n(this, ViewWidget::duplicateObject));
	}

	objects_tab_map[ObjectType::Trigger]->setColumnCount(6);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(GuiUtilsNs::getIcon("uid"),0);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Refer. Table"), 1);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(GuiUtilsNs::getIcon("table"),1);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Firing"), 2);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(GuiUtilsNs::getIcon("trigger"),2);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Events"), 3);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Alias"), 4);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Comment"), 5);

	objects_tab_map[ObjectType::Index]->setColumnCount(4);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Index]->setHeaderIcon(GuiUtilsNs::getIcon("uid"),0);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Indexing"), 1);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Alias"), 2);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Comment"), 3);

	objects_tab_map[ObjectType::Rule]->setColumnCount(5);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Rule]->setHeaderIcon(GuiUtilsNs::getIcon("uid"),0);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Execution"), 1);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Event"), 2);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Alias"), 3);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Comment"), 4);

	tablespace_sel->setEnabled(false);
	tablespace_lbl->setEnabled(false);

	connect(view_type_cmb, &QComboBox::activated, this, [this](int idx){
		with_no_data_chk->setEnabled(idx == Materialized);
		tablespace_sel->setEnabled(idx == Materialized);
		tablespace_lbl->setEnabled(idx == Materialized);
		check_option_cmb->setEnabled(idx == Ordinary);
	});

	connect(sql_definition_txt, &NumberedTextEditor::textChanged, this, [this]() {
		alert_frm->setVisible(sql_definition_txt->toPlainText().contains(QRegularExpression(View::ExtraSCRegExp)));
	});

	layout()->removeItem(type_sec_tag_lt);
	extra_wgts_lt->addLayout(type_sec_tag_lt);
	configureTabbedLayout(attributes_tbw);

	//configureTabOrder({ tag_sel, view_type_cmb, with_no_data_chk, attributes_tbw });
	setMinimumSize(700, 650);
}

CustomTableWidget *ViewWidget::getObjectTable(ObjectType obj_type)
{
	if(objects_tab_map.count(obj_type) > 0)
		return objects_tab_map[obj_type];

	return nullptr;
}

template<class Class, class WidgetClass>
int ViewWidget::openEditingForm(TableObject *object)
{
	BaseForm editing_form(this);
	WidgetClass *object_wgt=new WidgetClass;
	object_wgt->setAttributes(this->model, this->op_list,
														dynamic_cast<BaseTable *>(this->object),
														dynamic_cast<Class *>(object));
	editing_form.setMainWidget(object_wgt);

	return editing_form.exec();
}

void ViewWidget::handleObject()
{
	ObjectType obj_type=ObjectType::BaseObject;
	TableObject *object=nullptr;
	CustomTableWidget *obj_table=nullptr;

	try
	{
		obj_type=getObjectType(sender());
		obj_table=getObjectTable(obj_type);

		if(obj_table->getSelectedRow()>=0)
			object=reinterpret_cast<TableObject *>(obj_table->getRowData(obj_table->getSelectedRow()).value<void *>());

		if(obj_type==ObjectType::Trigger)
			openEditingForm<Trigger,TriggerWidget>(object);
		else if(obj_type==ObjectType::Index)
			openEditingForm<Index,IndexWidget>(object);
		else
			openEditingForm<Rule,RuleWidget>(object);

		listObjects(obj_type);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ViewWidget::duplicateObject(int curr_row, int new_row)
{
	ObjectType obj_type=ObjectType::BaseObject;
	BaseObject *object=nullptr, *dup_object=nullptr;
	CustomTableWidget *obj_table=nullptr;
	View *view = dynamic_cast<View *>(this->object);
	int op_id = -1;

	try
	{
		obj_type=getObjectType(sender());

		//Selects the object table based upon the passed object type
		obj_table=getObjectTable(obj_type);

		//Gets the object reference if there is an item select on table
		if(curr_row >= 0)
			object = reinterpret_cast<BaseObject *>(obj_table->getRowData(curr_row).value<void *>());

		CoreUtilsNs::copyObject(&dup_object, object, obj_type);
		dup_object->setName(CoreUtilsNs::generateUniqueName(dup_object, *view->getObjectList(obj_type), false, "_cp"));

		op_id=op_list->registerObject(dup_object, Operation::ObjCreated, new_row, this->object);

		view->addObject(dup_object);
		view->setModified(true);
		listObjects(obj_type);
	}
	catch(Exception &e)
	{
		//If operation was registered
		if(op_id >= 0)
		{
			op_list->ignoreOperationChain(true);
			op_list->removeLastOperation();
			op_list->ignoreOperationChain(false);
		}

		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ViewWidget::removeObjects()
{
	View *view=nullptr;
	unsigned count, op_count=0, i;
	BaseObject *object=nullptr;
	ObjectType obj_type=getObjectType(sender());

	try
	{
		view=dynamic_cast<View *>(this->object);
		op_count=op_list->getCurrentSize();

		while(view->getObjectCount(obj_type) > 0)
		{
			object=view->getObject(0, obj_type);
			view->removeObject(object);
			op_list->registerObject(object, Operation::ObjRemoved, 0, this->object);
		}
	}
	catch(Exception &e)
	{
		if(op_count < op_list->getCurrentSize())
		{
			count=op_list->getCurrentSize()-op_count;
			op_list->ignoreOperationChain(true);

			for(i=0; i < count; i++)
			{
				op_list->undoOperation();
				op_list->removeLastOperation();
			}

			op_list->ignoreOperationChain(false);
		}

		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ViewWidget::removeObject(int row)
{
	View *view=nullptr;
	BaseObject *object=nullptr;
	ObjectType obj_type=getObjectType(sender());

	try
	{
		view=dynamic_cast<View *>(this->object);
		object=view->getObject(row, obj_type);
		view->removeObject(object);
		op_list->registerObject(object, Operation::ObjRemoved, row, this->object);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

ObjectType ViewWidget::getObjectType(QObject *sender)
{
	ObjectType obj_type=ObjectType::BaseObject;

	if(sender)
	{
		std::map<ObjectType, CustomTableWidget *>::iterator itr, itr_end;

		itr=objects_tab_map.begin();
		itr_end=objects_tab_map.end();

		while(itr!=itr_end && obj_type==ObjectType::BaseObject)
		{
			if(itr->second==sender)
				obj_type=itr->first;

			itr++;
		}
	}

	return obj_type;
}

void ViewWidget::showObjectData(TableObject *object, int row)
{
	CustomTableWidget *tab=nullptr;
	Trigger *trigger=nullptr;
	Rule *rule=nullptr;
	Index *index=nullptr;
	ObjectType obj_type;
	QString str_aux;
	unsigned i;
	EventType events[]={ EventType::OnInsert, EventType::OnDelete,
						 EventType::OnTruncate,	EventType::OnUpdate };

	obj_type=object->getObjectType();
	tab=objects_tab_map[obj_type];

	//Column 0: Object name
	tab->setCellText(object->getName(),row,0);

	if(obj_type==ObjectType::Trigger)
	{
		trigger=dynamic_cast<Trigger *>(object);

		//Column 1: Table referenced by the trigger (constraint trigger)
		tab->clearCellText(row,1);
		if(trigger->getReferencedTable())
			tab->setCellText(trigger->getReferencedTable()->getName(true),row,1);

		//Column 2: Trigger firing type
		tab->setCellText(~trigger->getFiringType(),row,2);

		//Column 3: Events that fires the trigger
		for(i=0; i < sizeof(events)/sizeof(EventType); i++)
		{
			if(trigger->isExecuteOnEvent(events[i]))
				str_aux+=~events[i] + ", ";
		}

		str_aux.remove(str_aux.size()-2, 2);
		tab->setCellText(str_aux ,row,3);
		tab->setCellText(trigger->getAlias(), row, 4);
	}
	else if(obj_type==ObjectType::Rule)
	{
		rule=dynamic_cast<Rule *>(object);

		//Column 1: Rule execution type
		tab->setCellText(~rule->getExecutionType(),row,1);

		//Column 2: Rule event type
		tab->setCellText(~rule->getEventType(),row,2);

		tab->setCellText(rule->getAlias(), row, 3);
	}
	else
	{
		index=dynamic_cast<Index *>(object);

		//Column 1: Indexing type
		tab->setCellText(~index->getIndexingType(),row,1);
		tab->setCellText(index->getAlias(), row, 2);
	}

	tab->setCellText(object->getComment(), row, tab->getColumnCount() - 1);
	tab->setRowData(QVariant::fromValue<void *>(object), row);
}

void ViewWidget::listObjects(ObjectType obj_type)
{
	CustomTableWidget *tab = nullptr;
	unsigned count = 0, i = 0;
	View *view = nullptr;

	try
	{
		//Gets the object table related to the object type
		tab = objects_tab_map[obj_type];
		view = dynamic_cast<View *>(this->object);

		tab->blockSignals(true);
		tab->removeRows();

		count = view->getObjectCount(obj_type);
		for(i = 0; i < count; i++)
		{
			tab->addRow();
			showObjectData(view->getObject(i, obj_type), i);
		}
		tab->clearSelection();
		tab->blockSignals(false);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

QString ViewWidget::getSQLCodePreview()
{
	if(attributes_tbw->currentIndex() != attributes_tbw->count() - 1)
		return "";

	try
	{
		View aux_view;

		aux_view.BaseObject::setName(name_edt->text().toUtf8());
		aux_view.BaseObject::setSchema(schema_sel->getSelectedObject());
		aux_view.setTablespace(tablespace_sel->getSelectedObject());

		aux_view.setSqlDefinition(sql_definition_txt->toPlainText());
		aux_view.setReferences(obj_refs_wgt->getObjectReferences());

		aux_view.setMaterialized(view_type_cmb->currentIndex() == Materialized);
		aux_view.setRecursive(view_type_cmb->currentIndex() == Recursive);
		aux_view.setWithNoData(with_no_data_chk->isChecked());

		return aux_view.getSourceCode(SchemaParser::SqlCode);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void ViewWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, View *view, double px, double py)
{
	if(!view)
	{
		view = new View;

		if(schema)
			view->setSchema(schema);

		/* Sets the 'new_object' flag as true indicating that the alocated table must be treated
			 as a recently created object */
		this->new_object=true;
	}

	BaseObjectWidget::setAttributes(model,op_list, view, schema, px, py);

	sql_definition_txt->setPlainText(view->getSqlDefinition());
	obj_refs_wgt->setAttributes(this->model, view->getObjectReferences());

	if(view->isMaterialized())
		view_type_cmb->setCurrentIndex(Materialized);

	if(view->isRecursive())
		view_type_cmb->setCurrentIndex(Recursive);

	with_no_data_chk->setChecked(view->isWithNoData());

	op_list->startOperationChain();
	operation_count = op_list->getCurrentSize();

	tag_sel->setModel(this->model);
	tag_sel->setSelectedObject(view->getTag());

	custom_cols_wgt->setAttributes(this->model, view->getCustomColumns());
	check_option_cmb->setCurrentText(~view->getCheckOption());
	security_barrier_chk->setChecked(view->isSecurityBarrier());
	security_invoker_chk->setChecked(view->isSecurityInvoker());

	listObjects(ObjectType::Trigger);
	listObjects(ObjectType::Rule);
	listObjects(ObjectType::Index);
}

void ViewWidget::applyConfiguration()
{
	try
	{
		View *view=nullptr;
		ObjectType types[]={ ObjectType::Trigger, ObjectType::Rule, ObjectType::Index };

		if(!this->new_object)
			op_list->registerObject(this->object, Operation::ObjModified);
		else
			registerNewObject();

		BaseObjectWidget::applyConfiguration();

		view=dynamic_cast<View *>(this->object);
		view->removeObjects();
		view->setSecurityBarrier(security_barrier_chk->isChecked());
		view->setSecurityInvoker(security_invoker_chk->isChecked());
		view->setCheckOption(check_option_cmb->currentIndex() > 0 ? check_option_cmb->currentText() : "");
		view->setMaterialized(view_type_cmb->currentIndex() == Materialized);
		view->setRecursive(view_type_cmb->currentIndex() == Recursive);
		view->setWithNoData(with_no_data_chk->isChecked());
		view->setTag(dynamic_cast<Tag *>(tag_sel->getSelectedObject()));

		view->setSqlDefinition(sql_definition_txt->toPlainText());
		view->setReferences(obj_refs_wgt->getObjectReferences());
		view->setCustomColumns(custom_cols_wgt->getColumns());

		//Adds the auxiliary view objects into configured view
		for(auto &type : types)
		{
			for(unsigned i=0; i < objects_tab_map[type]->getRowCount(); i++)
				view->addObject(reinterpret_cast<TableObject *>(objects_tab_map[type]->getRowData(i).value<void *>()));
		}

		op_list->finishOperationChain();
		finishConfiguration();

		this->model->updateViewRelationships(view);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void ViewWidget::cancelConfiguration()
{
	BaseObjectWidget::cancelChainedOperation();
}
