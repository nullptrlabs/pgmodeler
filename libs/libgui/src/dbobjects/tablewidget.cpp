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

#include "tablewidget.h"
#include "columnwidget.h"
#include "constraintwidget.h"
#include "rulewidget.h"
#include "indexwidget.h"
#include "triggerwidget.h"
#include "baseform.h"
#include "widgets/tabledatawidget.h"
#include "policywidget.h"
#include "settings/generalconfigwidget.h"
#include "coreutilsns.h"
#include "relationshipview.h"
#include "guiutilsns.h"
#include "pgsqlversions.h"

TableWidget::TableWidget(QWidget *parent, ObjectType tab_type): BaseObjectWidget(parent, tab_type)
{
	QGridLayout *grid=nullptr;
	QVBoxLayout *vbox=nullptr;
	CustomTableWidget *tab=nullptr;
	ObjectType types[]={ ObjectType::Column, ObjectType::Constraint, ObjectType::Trigger,
											 ObjectType::Rule, ObjectType::Index, ObjectType::Policy };
	std::map<QString, std::vector<QWidget *> > fields_map;	
	QPushButton *edt_data_tb=nullptr;
	QStringList part_types;

	Ui_TableWidget::setupUi(this);

	edt_data_tb=new QPushButton(this);
	QPixmap icon=QPixmap(GuiUtilsNs::getIconPath("editrows"));
	edt_data_tb->setMinimumSize(edt_perms_tb->minimumSize());
	edt_data_tb->setText(tr("Edit data"));
	edt_data_tb->setToolTip(tr("Define initial data for the table"));
	edt_data_tb->setIcon(icon);
	edt_data_tb->setIconSize(edt_perms_tb->iconSize());

	connect(edt_data_tb, &QPushButton::clicked, this, __slot(this, TableWidget::editData));
	misc_btns_lt->insertWidget(1, edt_data_tb);

	fields_map[generateVersionsInterval(UntilVersion, PgSqlVersions::PgSqlVersion110)].push_back(with_oids_chk);
	warn_frame=generateVersionWarningFrame(fields_map);
	table_grid->addWidget(warn_frame, table_grid->count()+1, 0, 1, 2);
	warn_frame->setParent(this);

	parent_tables = new CustomTableWidget(CustomTableWidget::NoButtons, true, this);
	parent_tables->setColumnCount(3);
	parent_tables->setHeaderLabel(tr("Name"), 0);
	parent_tables->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	parent_tables->setHeaderLabel(tr("Schema"), 1);
	parent_tables->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("schema")),1);
	parent_tables->setHeaderLabel(tr("Type"), 2);
	parent_tables->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),2);

	server_sel=nullptr;
	server_sel=new ObjectSelectorWidget(ObjectType::ForeignServer, this);

	vbox = new QVBoxLayout;
	vbox->setContentsMargins(0,0,0,0);
	vbox->addWidget(server_sel);
	server_wgt->setLayout(vbox);

	options_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																			 (CustomTableWidget::EditButton | CustomTableWidget::UpdateButton), true, this);
	options_tab->setCellsEditable(true);
	options_tab->setColumnCount(2);
	options_tab->setHeaderLabel(tr("Option"), 0);
	options_tab->setHeaderLabel(tr("Value"), 1);

	vbox = new QVBoxLayout;
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	vbox->addWidget(options_tab);
	attributes_tbw->widget(9)->setLayout(vbox);

	tag_sel = new ObjectSelectorWidget(ObjectType::Tag, this);
	vbox = new QVBoxLayout(tag_sel_parent);
	vbox->addWidget(tag_sel);
	vbox->setContentsMargins(0,0,0,0);

	grid=new QGridLayout;
	grid->addWidget(parent_tables, 0,0,1,1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	attributes_tbw->widget(8)->setLayout(grid);

	//Configuring the table objects that stores the columns, triggers, constraints, rules and indexes
	for(unsigned i=0; i <= 5; i++)
	{
		tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
								CustomTableWidget::UpdateButton, true, this);

		objects_tab_map[types[i]]=tab;

		grid=new QGridLayout;
		grid->addWidget(tab, 0,0,1,1);
		grid->setContentsMargins(GuiUtilsNs::LtMargins);
		attributes_tbw->widget(i + 1)->setLayout(grid);

		connect(tab, &CustomTableWidget::s_rowsRemoved, this, __slot(this, TableWidget::removeObjects));
		connect(tab, &CustomTableWidget::s_rowRemoved, this, __slot_n(this, TableWidget::removeObject));
		connect(tab, &CustomTableWidget::s_rowAdded, this, __slot(this, TableWidget::handleObject));
		connect(tab, &CustomTableWidget::s_rowEdited, this, __slot(this, TableWidget::handleObject));
		connect(tab, &CustomTableWidget::s_rowDuplicated, this, __slot_n(this, TableWidget::duplicateObject));
		connect(tab, &CustomTableWidget::s_rowsMoved, this, __slot_n(this, TableWidget::swapObjects));
	}

	objects_tab_map[ObjectType::Column]->setColumnCount(7);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("PK"), 0);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Name"), 1);
	objects_tab_map[ObjectType::Column]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),1);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Type"), 2);
	objects_tab_map[ObjectType::Column]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),2);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Default Value"), 3);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Attribute(s)"), 4);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Alias"), 5);
	objects_tab_map[ObjectType::Column]->setHeaderLabel(tr("Comment"), 6);
	objects_tab_map[ObjectType::Column]->adjustColumnToContents(0);

	connect(objects_tab_map[ObjectType::Column], &CustomTableWidget::s_cellClicked, this, [this](int row, int col) {
		if(col == 0 && objects_tab_map[ObjectType::Column]->isCellDisabled(static_cast<unsigned>(row), static_cast<unsigned>(col)))
		{
			PhysicalTable *table = dynamic_cast<Table *>(this->object);
			Constraint *pk = table->getPrimaryKey();

			if(pk && pk->isAddedByRelationship())
				Messagebox::alert(tr("It is not possible to mark a column as primary key when the table already has a primary key which was created by a relationship! This action should be done in the section <strong>Primary key</strong> of the relationship's editing form."));
			else
				Messagebox::alert(tr("It is not possible to mark a column created by a relationship as primary key! This action should be done in the section <strong>Primary key</strong> of the relationship's editing form."));
		}
	});

	objects_tab_map[ObjectType::Constraint]->setColumnCount(6);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Constraint]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("Type"), 1);
	objects_tab_map[ObjectType::Constraint]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("ON DELETE"), 2);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("ON UPDATE"), 3);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("Alias"), 4);
	objects_tab_map[ObjectType::Constraint]->setHeaderLabel(tr("Comment"), 5);

	objects_tab_map[ObjectType::Trigger]->setColumnCount(6);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Refer. Table"), 1);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("table")),1);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Firing"), 2);
	objects_tab_map[ObjectType::Trigger]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("trigger")),2);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Events"), 3);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Alias"), 4);
	objects_tab_map[ObjectType::Trigger]->setHeaderLabel(tr("Comment"), 5);

	objects_tab_map[ObjectType::Rule]->setColumnCount(5);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Rule]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Execution"), 1);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Event"), 2);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Alias"), 3);
	objects_tab_map[ObjectType::Rule]->setHeaderLabel(tr("Comment"), 4);

	objects_tab_map[ObjectType::Index]->setColumnCount(4);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Index]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Indexing"), 1);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Alias"), 2);
	objects_tab_map[ObjectType::Index]->setHeaderLabel(tr("Comment"), 3);

	objects_tab_map[ObjectType::Policy]->setColumnCount(8);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Name"), 0);
	objects_tab_map[ObjectType::Policy]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Command"), 1);
	objects_tab_map[ObjectType::Policy]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("keyword")),1);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Permissive"), 2);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("USING expression"), 3);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("CHECK expression"), 4);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Roles"), 5);
	objects_tab_map[ObjectType::Policy]->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("role")),5);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Alias"), 6);
	objects_tab_map[ObjectType::Policy]->setHeaderLabel(tr("Comment"), 7);

	partition_keys_tab = new ElementsTableWidget;
	partition_keys_tab->setEnabled(false);
	grid = dynamic_cast<QGridLayout *>(attributes_tbw->widget(7)->layout());
	grid->addWidget(partition_keys_tab, 1, 0, 1, 2);

	part_types = PartitioningType::getTypes();
	part_types.push_front(tr("None"));
	partitioning_type_cmb->addItems(part_types);

	connect(partitioning_type_cmb, &QComboBox::currentTextChanged, this, [this](){
	  partition_keys_tab->setEnabled(partitioning_type_cmb->currentIndex() != 0);
	});

	setRequiredField(server_lbl);
	setRequiredField(server_sel);

	configureFormFields(tab_type);
	baseobject_grid->setContentsMargins(0, 0, 0, 0);
	dynamic_cast<QGridLayout*>(attributes_tbw->widget(0)->layout())->addLayout(baseobject_grid, 0, 0, 1, 3);

	configureTabOrder({ tag_sel });
	setMinimumSize(700, 650);
}

template<class Class, class WidgetClass>
int TableWidget::openEditingForm(TableObject *object)
{
	BaseForm editing_form(this);
	WidgetClass *object_wgt=new WidgetClass;
	int res = 0;

	object_wgt->setAttributes(this->model, this->op_list, dynamic_cast<PhysicalTable *>(this->object), dynamic_cast<Class *>(object));
	editing_form.setMainWidget(object_wgt);

	GeneralConfigWidget::restoreWidgetGeometry(&editing_form, object_wgt->metaObject()->className());
	res = editing_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&editing_form, object_wgt->metaObject()->className());

	return res;
}

CustomTableWidget *TableWidget::getObjectTable(ObjectType obj_type)
{
	if(objects_tab_map.count(obj_type) > 0)
		return objects_tab_map[obj_type];

	return nullptr;
}

ObjectType TableWidget::getObjectType(QObject *sender)
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

void TableWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Table *table, double pos_x, double pos_y)
{
	if(!table)
	{
		table=new Table;

		if(schema)
			table->setSchema(schema);

		/* Sets the 'new_object' flag as true indicating that the alocated table must be treated
			 as a recently created object */
		this->new_object=true;
	}

	__setAttributes(model, op_list, schema, table, pos_x, pos_y);
	server_lbl->setVisible(false);
	server_wgt->setVisible(false);
	attributes_tbw->removeTab(9); // Removing the options tab
}

void TableWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, ForeignTable *ftable, double pos_x, double pos_y)
{
	if(!ftable)
	{
		ftable = new ForeignTable;

		if(schema)
			ftable->setSchema(schema);

		/* Sets the 'new_object' flag as true indicating that the alocated table must be treated
			 as a recently created object */
		this->new_object=true;
	}

	__setAttributes(model, op_list, schema, ftable, pos_x, pos_y);

	warn_frame->setVisible(false);
	with_oids_chk->setVisible(false);
	unlogged_chk->setVisible(false);
	enable_rls_chk->setVisible(false);
	force_rls_chk->setVisible(false);
	attributes_tbw->removeTab(4); //Removing the Index tab
	attributes_tbw->removeTab(4); //Removing the Rule tab
	attributes_tbw->removeTab(4); //Removing the Policies tab
	attributes_tbw->removeTab(4); //Removing the Partition keys tab
	objects_tab_map[ObjectType::Column]->setHeaderVisible(0, false); //Hiding the "PK" checkbox on columns grid
	server_sel->setModel(this->model);
	server_sel->setSelectedObject(ftable->getForeignServer());
}

void TableWidget::__setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, PhysicalTable *table, double pos_x, double pos_y)
{
	try
	{
		unsigned i, count;
		PhysicalTable *aux_tab=nullptr;
		std::vector<ObjectType> types=BaseObject::getChildObjectTypes(ObjectType::Table);
		std::vector<PartitionKey> part_keys;

		BaseObjectWidget::setAttributes(model, op_list, table, schema, pos_x, pos_y);

		op_list->startOperationChain();
		operation_count=op_list->getCurrentSize();

		/* Listing all objects (column, constraint, trigger, index, rule) on the
		 * respective table objects */
		for(auto &type : types)
			listObjects(type);

		//Listing the ancestor tables
		count=table->getAncestorTableCount();
		for(i=0; i < count; i++)
		{
			aux_tab=table->getAncestorTable(i);
			parent_tables->addRow();
			parent_tables->setCellText(aux_tab->getName(), i, 0);
			parent_tables->setCellText(aux_tab->getSchema()->getName(), i, 1);
			parent_tables->setCellText(tr("Parent"), i, 2);
		}

		aux_tab=table->getCopyTable();
		if(aux_tab)
		{
			i = parent_tables->getRowCount();
			parent_tables->addRow();
			parent_tables->setCellText(aux_tab->getName(), i, 0);
			parent_tables->setCellText(aux_tab->getSchema()->getName(), i, 1);
			parent_tables->setCellText(tr("Copy"), i, 2);
		}

		aux_tab=table->getPartitionedTable();
		if(aux_tab)
		{
			i = parent_tables->getRowCount();
			parent_tables->addRow();
			parent_tables->setCellText(aux_tab->getName(), i, 0);
			parent_tables->setCellText(aux_tab->getSchema()->getName(), i, 1);
			parent_tables->setCellText(tr("Partitioned"), i, 2);
		}

		i = parent_tables->getRowCount();
		for(auto &tab : table->getPartionTables())
		{
			parent_tables->addRow();
			parent_tables->setCellText(tab->getName(), i, 0);
			parent_tables->setCellText(tab->getSchema()->getName(), i, 1);
			parent_tables->setCellText(tr("Partition"), i, 2);
			i++;
		}

		if(table->getObjectType() == ObjectType::Table)
		{
			Table *tab = dynamic_cast<Table *>(table);
			unlogged_chk->setChecked(tab->isUnlogged());
			enable_rls_chk->setChecked(tab->isRLSEnabled());
			force_rls_chk->setChecked(tab->isRLSForced());
			with_oids_chk->setChecked(tab->isWithOIDs());
		}
		else
		{
			ForeignTable *ftab = dynamic_cast<ForeignTable *>(table);
			options_tab->blockSignals(true);

			for(auto &itr : ftab->getOptions())
			{
				options_tab->addRow();
				options_tab->setCellText(itr.first, options_tab->getRowCount() - 1, 0);
				options_tab->setCellText(itr.second, options_tab->getRowCount() - 1, 1);
			}

			options_tab->clearSelection();
			options_tab->blockSignals(false);
		}

		parent_tables->clearSelection();
		gen_alter_cmds_chk->setChecked(table->isGenerateAlterCmds() && !table->isPartition() && !table->isPartitioned());
		gen_alter_cmds_chk->setEnabled(!table->isPartition() && !table->isPartitioned());

		tag_sel->setModel(this->model);
		tag_sel->setSelectedObject(table->getTag());

		int idx = partitioning_type_cmb->findText(~table->getPartitioningType());
		partitioning_type_cmb->setCurrentIndex(idx < 0 ? 0 : idx);
		partition_keys_tab->setAttributes<PartitionKey>(model, table);

		part_keys = table->getPartitionKeys();
		partition_keys_tab->setAttributes<PartitionKey>(this->model, table);
		partition_keys_tab->setElements<PartitionKey>(part_keys);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::listObjects(ObjectType obj_type)
{
	CustomTableWidget *tab = nullptr;
	PhysicalTable *table = nullptr;
	std::vector<unsigned> pk_cols;

	try
	{
		//Gets the object table related to the object type
		tab = objects_tab_map[obj_type];
		table = dynamic_cast<PhysicalTable *>(this->object);

		tab->blockSignals(true);

		/* In case of displaying columns we need to store
		 * the row related to table columns that are marked as primary key so
		 * we restore the check state after (re)populate the grid */
		if(obj_type == ObjectType::Column)
		{
			for(unsigned row = 0; row < tab->getRowCount(); row++)
			{
				if(tab->getCellCheckState(row, 0) == Qt::Checked)
					pk_cols.push_back(row);
			}
		}

		tab->removeRows();
		std::vector<TableObject *> *list = table->getObjectList(obj_type);

		if(list)
		{
			for(auto &obj : *list)
			{
				tab->addRow();
				showObjectData(obj, tab->getRowCount() - 1);
			}
		}

		// Restoring the check state of columns marked as primary key
		for(auto &pk_col : pk_cols)
		{
			if(pk_col >= tab->getRowCount())
				continue;

			tab->setCellCheckState(Qt::Checked, pk_col, 0);
		}

		tab->resizeContents();
		tab->clearSelection();
		tab->blockSignals(false);

		//Enables the add button on the constraints, triggers and index tab only when there is columns created
		if(obj_type==ObjectType::Column)
		{
			objects_tab_map[ObjectType::Constraint]->setButtonsEnabled(CustomTableWidget::AddButton,
																																 objects_tab_map[ObjectType::Column]->getRowCount() > 0);
			objects_tab_map[ObjectType::Trigger]->setButtonsEnabled(CustomTableWidget::AddButton,
																															objects_tab_map[ObjectType::Column]->getRowCount() > 0);
			objects_tab_map[ObjectType::Index]->setButtonsEnabled(CustomTableWidget::AddButton,
																														objects_tab_map[ObjectType::Column]->getRowCount() > 0);
		}

		/* Disabling the table widget if the table object is protected
		 * avoiding the addition to new object to the table, preserving
		 * the protection status */
		tab->setEnabled(!table->isProtected());
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::handleObject()
{
	ObjectType obj_type=ObjectType::BaseObject;
	TableObject *object=nullptr;
	CustomTableWidget *obj_table=nullptr;

	try
	{
		obj_type=getObjectType(sender());

		//Selects the object table based upon the passed object type
		obj_table=getObjectTable(obj_type);

		//Gets the object reference if there is an item select on table
		if(obj_table->getSelectedRow()>=0)
			object=reinterpret_cast<TableObject *>(obj_table->getRowData(obj_table->getSelectedRow()).value<void *>());

		if(obj_type==ObjectType::Column)
			openEditingForm<Column, ColumnWidget>(object);
		else if(obj_type==ObjectType::Constraint)
			openEditingForm<Constraint, ConstraintWidget>(object);
		else if(obj_type==ObjectType::Trigger)
			openEditingForm<Trigger, TriggerWidget>(object);
		else if(obj_type==ObjectType::Index)
			openEditingForm<Index, IndexWidget>(object);
		else if(obj_type==ObjectType::Rule)
			openEditingForm<Rule, RuleWidget>(object);
		else
			openEditingForm<Policy, PolicyWidget>(object);

		listObjects(obj_type);

		if(obj_type == ObjectType::Constraint)
			listObjects(ObjectType::Column);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::showObjectData(TableObject *object, int row)
{
	CustomTableWidget *tab=nullptr;
	Column *column=nullptr;
	Constraint *constr=nullptr;
	Trigger *trigger=nullptr;
	Rule *rule=nullptr;
	Index *index=nullptr;
	Policy *policy=nullptr;
	ObjectType obj_type;
	QString str_aux, str_aux1;

	QStringList contr_types={ ~ConstraintType(ConstraintType::PrimaryKey), ~ConstraintType(ConstraintType::ForeignKey),
														~ConstraintType(ConstraintType::Check), ~ConstraintType(ConstraintType::Unique),
														"NOT NULL" },
			constr_codes={ TableObjectView::TextPrimaryKey,
										 TableObjectView::TextForeignKey,
										 TableObjectView::TextCheck,
										 TableObjectView::TextUnique,
										 TableObjectView::TextNotNull};

	QFont font;
	unsigned i;
	EventType events[]={ EventType::OnInsert, EventType::OnDelete,
						 EventType::OnTruncate,	EventType::OnUpdate };

	obj_type=object->getObjectType();
	tab=objects_tab_map[obj_type];


	if(obj_type==ObjectType::Column)
		tab->setCellText(object->getName(),row,1);
	else
		tab->setCellText(object->getName(),row,0);

	//For each object type there is a use for the columns from 1 to 3
	if(obj_type==ObjectType::Column)
	{
		PhysicalTable *table = dynamic_cast<PhysicalTable *>(this->object);
		Constraint *pk = table->getPrimaryKey();
		column=dynamic_cast<Column *>(object);

		//Column 2: Column data type
		tab->setCellText(*column->getType(),row,2);

		//Column 3: Column defaul value
		if(column->getSequence())
			str_aux=QString("nextval('%1'::regclass)").arg(column->getSequence()->getName(true).remove('"'));
		else if(column->getIdentityType() != IdentityType::Null)
			str_aux=QString("GENERATED %1 AS IDENTITY").arg(~column->getIdentityType());
		else
			str_aux=column->getDefaultValue();

		if(str_aux.isEmpty()) str_aux="-";
		tab->setCellText(str_aux,row,3);

		//Column 4: Column attributes (constraints which belongs)
		str_aux=TableObjectView::getConstraintString(column);
		for(int i=0; i < constr_codes.size(); i++)
		{
			if(str_aux.indexOf(constr_codes[i]) >= 0)
				str_aux1+=contr_types[i] + ", ";
		}

		if(str_aux1.isEmpty())
			str_aux1="-";
		else
			str_aux1.remove(str_aux1.size()-2, 2);

		tab->setCellText(str_aux1,row,4);

		if(str_aux.indexOf(TableObjectView::TextPrimaryKey) >= 0)
			tab->setCellCheckState(Qt::Checked, row, 0);
		else
			tab->setCellCheckState(Qt::Unchecked, row, 0);

		if(column->isAddedByRelationship() || (pk && pk->isAddedByRelationship()))
			tab->setCellDisabled(true, row, 0);

		tab->setCellText(column->getAlias(), row, 5);
		tab->adjustColumnToContents(0);
	}
	else if(obj_type==ObjectType::Constraint)
	{
		constr=dynamic_cast<Constraint *>(object);

		//Column 1: Constraint type
		tab->setCellText(~constr->getConstraintType(),row,1);

		if(constr->getConstraintType()==ConstraintType::ForeignKey)
		{
			//Column 2: ON DELETE action
			tab->setCellText(~constr->getActionType(Constraint::DeleteAction),row,2);

			//Column 3: ON UPDATE action
			tab->setCellText(~constr->getActionType(Constraint::UpdateAction),row,3);
		}
		else
		{
			tab->setCellText("-",row,2);
			tab->setCellText("-",row,3);
		}

		tab->setCellText(constr->getAlias(), row, 4);
	}
	else if(obj_type==ObjectType::Trigger)
	{
		trigger=dynamic_cast<Trigger *>(object);

		//Column 1: Table referenced by the trigger (constraint trigger)
		tab->clearCellText(row,1);
		if(trigger->getReferencedTable())
			tab->setCellText(trigger->getReferencedTable()->getName(true),row,1);

		//Column 2: Trigger firing type
		tab->setCellText(~trigger->getFiringType(),row,2);

		//Column 3: Events that fires the trigger
		for(i=0; i < 4; i++)
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
	else if(obj_type==ObjectType::Index)
	{
		index=dynamic_cast<Index *>(object);

		//Coluna 1: Indexing type
		tab->setCellText(~index->getIndexingType(),row,1);

		tab->setCellText(index->getAlias(), row, 2);
	}
	else if(obj_type==ObjectType::Policy)
	{
		QStringList rol_names;

		policy = dynamic_cast<Policy *>(object);

		//Column 1: Command
		tab->setCellText(~policy->getPolicyCommand(), row, 1);

		//Column 2: Permissive
		tab->setCellText(QString("%1").arg(policy->isPermissive() ? tr("Yes") : tr("No")), row, 2);

		//Column 3: USING expression
		tab->setCellText(policy->getUsingExpression(), row, 3);

		//Column 4: CHECK expression
		tab->setCellText(policy->getCheckExpression(), row, 4);

		for(auto role : policy->getRoles())
			rol_names.append(role->getName());

		//Column 5: Roles
		tab->setCellText(!rol_names.isEmpty() ? rol_names.join(", ") : "PUBLIC", row, 5);

		tab->setCellText(policy->getAlias(), row, 6);
	}

	//Changes the foreground/background color of the table row if the object is protected or added by relationship
	if(object->isAddedByRelationship() || object->isProtected())
	{
		font=tab->font();
		font.setItalic(true);

		if(object->isAddedByRelationship())
		{
			tab->setRowFont(row, font);
			tab->setRowColors(row,
												CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemFgColor),
												CustomTableWidget::getTableItemColor(CustomTableWidget::RelAddedItemBgColor));
		}
		else
		{
			tab->setRowFont(row, font);
			tab->setRowColors(row,
												CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemFgColor),
												CustomTableWidget::getTableItemColor(CustomTableWidget::ProtItemBgColor));
		}
	}

	tab->setCellText(object->getComment(), row, tab->getColumnCount() - 1);
	tab->setRowData(QVariant::fromValue<void *>(object), row);
}

void TableWidget::removeObjects()
{
	PhysicalTable *table=nullptr;
	unsigned count, op_count=0, i;
	BaseObject *object=nullptr;
	ObjectType obj_type=ObjectType::BaseObject;
	bool has_pk = false;

	try
	{
		table=dynamic_cast<PhysicalTable *>(this->object);
		obj_type=getObjectType(sender());
		count=table->getObjectCount(obj_type);
		op_count=op_list->getCurrentSize();

		for(i=0; i < count; i++)
		{
			object=table->getObject(0, obj_type);

			if(!object->isProtected() &&
					!dynamic_cast<TableObject *>(object)->isAddedByRelationship())
			{
				op_list->registerObject(object, Operation::ObjRemoved, 0, this->object);
				table->removeObject(object);

				if(obj_type == ObjectType::Constraint && !has_pk &&
					 dynamic_cast<Constraint *>(object)->getConstraintType() == ConstraintType::PrimaryKey)
					has_pk = true;
			}
			else
				throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
								.arg(object->getName())
								.arg(object->getTypeName()),
								ErrorCode::RemProtectedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		if(obj_type == ObjectType::Constraint)
			updatePkColumnsCheckState(has_pk);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);

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

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::removeObject(int row)
{
	PhysicalTable *table = nullptr;
	BaseObject *object = nullptr;
	ObjectType obj_type = ObjectType::BaseObject;
	int op_id = -1;

	try
	{
		table=dynamic_cast<PhysicalTable *>(this->object);
		obj_type=getObjectType(sender());
		object=table->getObject(row, obj_type);

		if(!object->isProtected() &&
				!dynamic_cast<TableObject *>(object)->isAddedByRelationship())
		{
			op_id=op_list->registerObject(object, Operation::ObjRemoved, row, this->object);
			table->removeObject(object);
			table->setModified(true);
		}
		else
			throw Exception(Exception::getErrorMessage(ErrorCode::RemProtectedObject)
							.arg(object->getName())
							.arg(object->getTypeName()),
							ErrorCode::RemProtectedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

		if(obj_type == ObjectType::Constraint)
			updatePkColumnsCheckState(dynamic_cast<Constraint *>(object)->getConstraintType() == ConstraintType::PrimaryKey);
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

void TableWidget::updatePkColumnsCheckState(bool has_pk)
{
	int res = Messagebox::Rejected;
	QList<unsigned> pk_col_rows;
	CustomTableWidget *tab = objects_tab_map[ObjectType::Column];

	if(has_pk)
	{
		for(unsigned row = 0; row < tab->getRowCount(); row++)
		{
			if(tab->getCellCheckState(row, 0) == Qt::Checked)
				pk_col_rows.push_back(row);
		}

		if(!pk_col_rows.isEmpty())
			res = Messagebox::confirm(tr("The primary key of the table was removed, do you want to uncheck the columns marked as <strong>PK</strong> in the <strong>Columns</strong> tab in order to avoid the primary key being created again?"));
	}

	listObjects(ObjectType::Column);

	if(has_pk && !pk_col_rows.isEmpty())
	{
		for(auto &row : pk_col_rows)
			tab->setCellCheckState(Messagebox::isAccepted(res) ?
														 Qt::Unchecked : Qt::Checked, row, 0);
	}
}

void TableWidget::duplicateObject(int sel_row, int new_row)
{
	ObjectType obj_type=ObjectType::BaseObject;
	BaseObject *object=nullptr, *dup_object=nullptr;
	CustomTableWidget *obj_table=nullptr;
	PhysicalTable *table = dynamic_cast<PhysicalTable *>(this->object);
	int op_id = -1;

	try
	{
		obj_type=getObjectType(sender());

		//Selects the object table based upon the passed object type
		obj_table=getObjectTable(obj_type);

		//Gets the object reference if there is an item select on table
		if(sel_row >= 0)
			object = reinterpret_cast<BaseObject *>(obj_table->getRowData(sel_row).value<void *>());

		CoreUtilsNs::copyObject(&dup_object, object, obj_type);
		dup_object->setName(CoreUtilsNs::generateUniqueName(dup_object, *table->getObjectList(obj_type), false, "_cp"));

		op_id=op_list->registerObject(dup_object, Operation::ObjCreated, new_row, this->object);

		table->addObject(dup_object);
		table->setModified(true);
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

void TableWidget::swapObjects(int idx1, int idx2)
{
	ObjectType obj_type=ObjectType::BaseObject;
	PhysicalTable *table=nullptr;
	int count;

	try
	{
		obj_type=getObjectType(sender());
		table=dynamic_cast<PhysicalTable *>(this->object);
		count=table->getObjectCount(obj_type);

		if(idx1 >= count)
			//Special case 1: the object was moved to the first row, its index is swapped with index 0
			op_list->updateObjectIndex(table->getObject(idx2, obj_type), 0);
		else if(idx2 >= count)
			//Special case 2: the object was moved to the last row, its index is swapped with index count-1
			op_list->updateObjectIndex(table->getObject(idx1, obj_type), count-1);
		else
		{
			op_list->updateObjectIndex(table->getObject(idx1, obj_type), idx2);
			op_list->updateObjectIndex(table->getObject(idx2, obj_type), idx1);
		}

		table->swapObjectsIndexes(obj_type, idx1, idx2);
	}
	catch(Exception &e)
	{
		listObjects(obj_type);
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::editData()
{
	BaseForm base_form(this);
	TableDataWidget *tab_data_wgt=new TableDataWidget(this);

	tab_data_wgt->setAttributes(this->model, dynamic_cast<PhysicalTable *>(this->object));
	base_form.setMainWidget(tab_data_wgt);
	base_form.setButtonConfiguration(Messagebox::OkCancelButtons);

	GeneralConfigWidget::restoreWidgetGeometry(&base_form, tab_data_wgt->metaObject()->className());
	base_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&base_form, tab_data_wgt->metaObject()->className());
}

void TableWidget::applyConfiguration()
{
	try
	{
		PhysicalTable *table=nullptr;
		Table *aux_tab = nullptr;
		Constraint *pk = nullptr;
		std::vector<BaseRelationship *> rels;
		std::vector<Column *> pk_cols;
		std::vector<PartitionKey> part_keys;
		CustomTableWidget *col_tab = objects_tab_map[ObjectType::Column];
		PartitioningType part_type;

		if(!this->new_object)
			op_list->registerObject(this->object, Operation::ObjModified);
		else
			registerNewObject();

		table=dynamic_cast<PhysicalTable *>(this->object);
		aux_tab = dynamic_cast<Table *>(table);

		table->setGenerateAlterCmds(gen_alter_cmds_chk->isChecked());
		table->setTag(dynamic_cast<Tag *>(tag_sel->getSelectedObject()));

		// Applying settings specific to table
		if(aux_tab)
		{
			aux_tab->setWithOIDs(with_oids_chk->isChecked());
			aux_tab->setRLSEnabled(enable_rls_chk->isChecked());
			aux_tab->setRLSForced(force_rls_chk->isChecked());
			aux_tab->setUnlogged(unlogged_chk->isChecked());
		}
		// Applying settings specific to foreign table
		else if(server_sel->isVisible())
		{
			ForeignTable *ftable = dynamic_cast<ForeignTable *>(table);

			ftable->setForeignServer(dynamic_cast<ForeignServer *>(server_sel->getSelectedObject()));
			ftable->removeOptions();
			for(unsigned row = 0; row < options_tab->getRowCount(); row++)
				ftable->setOption(options_tab->getCellText(row, 0), options_tab->getCellText(row, 1));
		}

		part_type = partitioning_type_cmb->currentIndex() == 0 ? PartitioningType::Null : PartitioningType(partitioning_type_cmb->currentText());
		table->setPartitioningType(part_type);

		if(part_type != PartitioningType::Null)
		{
			partition_keys_tab->getElements<PartitionKey>(part_keys);
			table->addPartitionKeys(part_keys);

			if(part_keys.empty())
				part_type = PartitioningType::Null;
		}

		BaseObjectWidget::applyConfiguration();

		//Retrieving all columns marked as primary key
		for(unsigned row = 0; row < col_tab->getRowCount(); row++)
		{
			if(col_tab->getCellCheckState(row, 0) == Qt::Checked)
				pk_cols.push_back(reinterpret_cast<Column *>(col_tab->getRowData(row).value<void *>()));
		}

		pk = table->getPrimaryKey();

		//If there is at least one column marked as pk
		if(!pk_cols.empty())
		{
			if(!pk)
			{
				//Create the primary key if the table does not own one
				QString pk_name = QString("%1_pk").arg(table->getName());

				if(pk_name.size() > BaseObject::ObjectNameMaxLength)
					pk_name.remove(BaseObject::ObjectNameMaxLength, pk_name.size());

				pk = new Constraint;
				pk->setName(pk_name);
				pk->setName(CoreUtilsNs::generateUniqueName(pk, *table->getObjectList(ObjectType::Constraint)));

				for(Column *col : pk_cols)
					pk->addColumn(col, Constraint::SourceCols);

				table->addConstraint(pk);
				op_list->registerObject(pk, Operation::ObjCreated, -1, table);
			}
			else if(!pk->isAddedByRelationship())
			{
			  std::vector<Column *> orig_pk_cols = pk->getColumns(Constraint::SourceCols);

				//If the table owns a pk we only update the columns
				op_list->registerObject(pk, Operation::ObjModified, -1, table);
				pk->removeColumns();

				/* Adding the original primary key columns if they also exists in the
				 * list of columns generated by the selection in the column list */
				for(auto col : orig_pk_cols)
				{
				  if(std::find(pk_cols.begin(), pk_cols.end(), col) != pk_cols.end())
					pk->addColumn(col, Constraint::SourceCols);
				}

				/* Adding the other columns selected in the grid as pk columns.
				 * Duplicated columns are discarded by the method Constraint::addColumn */
				for(Column *col : pk_cols)
					pk->addColumn(col, Constraint::SourceCols);

				pk->updateDependencies();
			}
		}
		else if(pk_cols.empty() && pk && !pk->isAddedByRelationship())
		{
			//Removing the primary key from the table when no column is checked as pk
			op_list->registerObject(pk, Operation::ObjRemoved, -1, table);
			table->removeObject(pk);
		}

		try
		{
			table->saveRelObjectsIndexes();

			if(model->getRelationship(table, nullptr))
				model->validateRelationships();

			if(aux_tab)
				model->updateTableFKRelationships(aux_tab);

			model->updateViewsReferencingTable(table);
		}
		catch(Exception &e)
		{
			if(e.getErrorCode()==ErrorCode::RemInvalidatedObjects)
				Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
			else
				throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}

		op_list->finishOperationChain();
		finishConfiguration();

		if(RelationshipView::getLineConnectinMode()==RelationshipView::ConnectFkToPk)
		{
			/* Forcing the update of relationships connected to the table in order to reconfigure the line
			 in case of the relationship is using the CONNECT_FK_TO_PK line mode */
			rels=model->getRelationships(table);
			for(auto &rel : rels)
			{
				if(rel->getRelationshipType()==Relationship::Relationship11 ||
						rel->getRelationshipType()==Relationship::Relationship1n ||
						rel->getRelationshipType()==Relationship::RelationshipFk)
					rel->setModified(true);
			}
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TableWidget::cancelConfiguration()
{
	BaseObjectWidget::cancelChainedOperation();
}

