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

#include "typewidget.h"
#include "guiutilsns.h"

TypeWidget::TypeWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Type)
{
	Ui_TypeWidget::setupUi(this);

	like_type = new PgSQLTypeWidget(this, tr("Like type"));
	element_type = new PgSQLTypeWidget(this, tr("Element type"));

	base_attribs_grid->addWidget(like_type, base_attribs_grid->count(), 0, 1, 0);
	base_attribs_grid->addWidget(element_type, base_attribs_grid->count(), 0, 1, 0);
	base_attribs_grid->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding),
														 base_attribs_grid->count(), 0, 1, 0);

	QList<QVBoxLayout *> func_lts {
		input_lt, output_lt, recv_lt, send_lt,
		tpmod_in_lt, tpmod_out_lt, analyze_lt
	};

	int lt_idx = 0;

	for(auto &func_sel : functions_sel)
	{
		func_sel = new ObjectSelectorWidget(ObjectType::Function, functions_pg);
		func_lts[lt_idx++]->addWidget(func_sel);

		if(lt_idx >= func_lts.size())
			break;
	}

	enumerations_tab = GuiUtilsNs::createWidgetInParent<CustomTableWidget>(0, CustomTableWidget::AllButtons ^
																																				 (CustomTableWidget::EditButton | CustomTableWidget::DuplicateButton),
																																				 true, enumerations_pg);
	enumerations_tab->setCellsEditable(true);

	attributes_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																				 CustomTableWidget::DuplicateButton,
																				 true, comp_attribs_wgt);
	attributes_tab->setColumnCount(3);
	attributes_tab->setHeaderLabel(tr("Name"),0);
	attributes_tab->setHeaderIcon(GuiUtilsNs::getIcon("uid"),0);
	attributes_tab->setHeaderLabel(tr("Type"),1);
	attributes_tab->setHeaderIcon(GuiUtilsNs::getIcon("usertype"),1);
	attributes_tab->setHeaderLabel(tr("Collation"),2);
	attributes_tab->setHeaderIcon(GuiUtilsNs::getIcon("collation"),2);

	attrib_collation_sel = new ObjectSelectorWidget(ObjectType::Collation, comp_attribs_wgt);
	attrib_collation_lt->addWidget(attrib_collation_sel);
	attrib_collation_sel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	attrib_type_wgt = new PgSQLTypeWidget(comp_attribs_wgt);
	comp_attribs_grid->addWidget(attrib_type_wgt, comp_attribs_grid->count(), 0, 1, 0);
	comp_attribs_grid->addWidget(attributes_tab, comp_attribs_grid->count(), 0, 1, 0);

	opclass_sel = new ObjectSelectorWidget(ObjectType::OpClass, range_attribs_wgt);
	range_opclass_lt->addWidget(opclass_sel);

	functions_sel[Type::CanonicalFunc] = new ObjectSelectorWidget(ObjectType::Function, this);
	canonical_func_lt->addWidget(functions_sel[Type::CanonicalFunc]);

	functions_sel[Type::SubtypeDiffFunc] = new ObjectSelectorWidget(ObjectType::Function, this);
	diff_func_lt->addWidget(functions_sel[Type::SubtypeDiffFunc]);

	range_subtype = new PgSQLTypeWidget(this, tr("Subtype"));
	range_attribs_lt->addWidget(range_subtype);

	connect(configuration_cmb, &QComboBox::activated, this, &TypeWidget::selectTypeConfiguration);
	connect(attributes_tab, &CustomTableWidget::s_rowEdited, this, &TypeWidget::editAttribute);

	connect(attributes_tab, &CustomTableWidget::s_rowAdded, this, __slot_n(this, TypeWidget::handleAttribute));
	connect(attributes_tab, &CustomTableWidget::s_rowUpdated, this, __slot_n(this, TypeWidget::handleAttribute));

	storage_cmb->addItems(StorageType::getTypes());
	category_cmb->addItems(CategoryType::getTypes());

	setRequiredField(range_subtype);
	setRequiredField(input_lbl);
	setRequiredField(output_lbl);
	setRequiredField(functions_sel[Type::InputFunc]);
	setRequiredField(functions_sel[Type::OutputFunc]);

	layout()->removeItem(configuration_lt);
	extra_wgts_lt->addLayout(configuration_lt);
	configureTabbedLayout(type_attribs_twg);

	configureTabOrder({ attrib_name_edt, attrib_collation_sel, attrib_type_wgt,
										 opclass_sel, functions_sel[Type::CanonicalFunc], functions_sel[Type::SubtypeDiffFunc],
										 type_attribs_twg});

	setMinimumSize(600, 480);
}

void TypeWidget::selectTypeConfiguration(int type_idx)
{
	Type::TypeConfig type_cfg = static_cast<Type::TypeConfig>(type_idx);
	bool is_base = (type_cfg == Type::BaseType),
			is_enum = (type_cfg == Type::EnumerationType),
			is_comp = (type_cfg == Type::CompositeType),
			is_range = (type_cfg == Type::RangeType);

	type_attribs_twg->setTabVisible(type_attribs_twg->indexOf(functions_pg), is_base);
	type_attribs_twg->setTabVisible(type_attribs_twg->indexOf(attributes_pg), !is_enum);
	type_attribs_twg->setTabVisible(type_attribs_twg->indexOf(enumerations_pg), is_enum);

	base_attribs_wgt->setVisible(is_base);
	comp_attribs_wgt->setVisible(is_comp);
	range_attribs_wgt->setVisible(is_range);

	collation_sel->setEnabled(is_range);
	collation_lbl->setEnabled(is_range);

	if(!is_range)
		collation_sel->clearSelector();
}

void TypeWidget::showAttributeData(TypeAttribute attrib, int row)
{
	attributes_tab->setCellText(attrib.getName(), row, 0);
	attributes_tab->setCellText(*attrib.getType(), row, 1);

	if(attrib.getCollation())
		attributes_tab->setCellText(attrib.getCollation()->getName(true), row, 2);
	else
		attributes_tab->clearCellText(row,2);

	attributes_tab->setRowData(QVariant::fromValue<TypeAttribute>(attrib), row);
}

void TypeWidget::editAttribute(int row)
{
	TypeAttribute attrib=attributes_tab->getRowData(row).value<TypeAttribute>();

	attrib_name_edt->setText(attrib.getName());
	attrib_collation_sel->setSelectedObject(attrib.getCollation());
	attrib_type_wgt->setAttributes(attrib.getType(), this->model);
}

void TypeWidget::handleAttribute(int row)
{
	try
	{
		TypeAttribute attrib;

		attrib.setName(attrib_name_edt->text().toUtf8());
		attrib.setType(attrib_type_wgt->getPgSQLType());
		attrib.setCollation(attrib_collation_sel->getSelectedObject());
		showAttributeData(attrib, row);

		attrib_name_edt->clear();
		attrib_collation_sel->clearSelector();
	}
	catch(Exception &e)
	{
		if(attributes_tab->getCellText(row,0).isEmpty())
			attributes_tab->removeRow(row);

		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void TypeWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema, Type *type)
{
	PgSqlType like_tp, elem_tp;
	unsigned i, count;
	Type::TypeConfig type_conf;

	BaseObjectWidget::setAttributes(model, op_list, type, schema);

	attrib_collation_sel->setModel(model);
	attrib_type_wgt->setAttributes(PgSqlType(), model);
	range_subtype->setAttributes(PgSqlType(), model);

	opclass_sel->setModel(model);

	for(int func_idx = Type::InputFunc; func_idx <= Type::SubtypeDiffFunc; func_idx++)
		functions_sel[func_idx]->setModel(model);

	if(type)
	{
		type_conf = type->getConfiguration();
		configuration_cmb->setCurrentIndex(enum_t(type_conf));

		if(type_conf == Type::CompositeType)
		{
			attributes_tab->blockSignals(true);
			count = type->getAttributeCount();

			for(unsigned i = 0; i < count; i++)
			{
				attributes_tab->addRow();
				showAttributeData(type->getAttribute(i),i);
			}

			attributes_tab->blockSignals(false);
			attributes_tab->clearSelection();
		}
		else if(type_conf == Type::EnumerationType)
		{
			enumerations_tab->blockSignals(true);
			count = type->getEnumerationCount();

			for(unsigned i = 0; i < count; i++)
			{
				enumerations_tab->addRow();
				enumerations_tab->setCellText(type->getEnumeration(i), i, 0);
			}

			enumerations_tab->blockSignals(false);
			enumerations_tab->clearSelection();
		}
		else if(type_conf == Type::RangeType)
		{
			opclass_sel->setSelectedObject(type->getSubtypeOpClass());
			range_subtype->setAttributes(type->getSubtype(), model);
			functions_sel[Type::CanonicalFunc]->setSelectedObject(type->getFunction(Type::CanonicalFunc));
			functions_sel[Type::SubtypeDiffFunc]->setSelectedObject(type->getFunction(Type::SubtypeDiffFunc));
		}
		else
		{
			like_tp = type->getLikeType();
			elem_tp = type->getElement();

			internal_len_sb->setValue(type->getInternalLength());
			by_value_chk->setChecked(type->isByValue());
			preferred_chk->setChecked(type->isPreferred());
			collatable_chk->setChecked(type->isCollatable());
			delimiter_edt->setText(QString(type->getDelimiter()));
			default_value_edt->setText(type->getDefaultValue());
			category_cmb->setCurrentIndex(category_cmb->findText(~type->getCategory()));
			storage_cmb->setCurrentIndex(storage_cmb->findText(~type->getStorage()));
			alignment_cmb->setCurrentIndex(alignment_cmb->findText(~type->getAlignment()));

			for(unsigned i = Type::InputFunc; i <= Type::AnalyzeFunc; i++)
				functions_sel[i]->setSelectedObject(type->getFunction(static_cast<Type::FunctionId>(i)));
		}
	}
	else
		selectTypeConfiguration(Type::BaseType);

	like_type->setAttributes(like_tp, model);
	element_type->setAttributes(elem_tp, model);
}

void TypeWidget::applyConfiguration()
{
	try
	{
		Type *type = nullptr;
		unsigned i = 0, count = 0;
		Type::TypeConfig type_conf = static_cast<Type::TypeConfig>(configuration_cmb->currentIndex());

		startConfiguration<Type>();
		type = dynamic_cast<Type *>(this->object);

		BaseObjectWidget::applyConfiguration();
		type->setConfiguration(type_conf);

		//Configuring an enumaration type
		if(type_conf == Type::EnumerationType)
		{
			count = enumerations_tab->getRowCount();
			for(i = 0; i < count; i++)
				type->addEnumeration(enumerations_tab->getCellText(i,0).toUtf8());
		}
		//Configuration a composite type
		else if(type_conf == Type::CompositeType)
		{
			count = attributes_tab->getRowCount();
			for(i = 0; i < count; i++)
				type->addAttribute(attributes_tab->getRowData(i).value<TypeAttribute>());
		}
		//Configuration a range type
		else if(type_conf == Type::RangeType)
		{
			type->setCollation(collation_sel->getSelectedObject());
			type->setSubtype(range_subtype->getPgSQLType());
			type->setSubtypeOpClass(dynamic_cast<OperatorClass *>(opclass_sel->getSelectedObject()));

			type->setFunction(Type::CanonicalFunc,
												dynamic_cast<Function *>(functions_sel[Type::CanonicalFunc]->getSelectedObject()));

			type->setFunction(Type::SubtypeDiffFunc,
												dynamic_cast<Function *>(functions_sel[Type::SubtypeDiffFunc]->getSelectedObject()));
		}
		//Configuring a base type
		else
		{
			type->setLikeType(like_type->getPgSQLType());
			type->setElement(element_type->getPgSQLType());
			type->setInternalLength(internal_len_sb->value());
			type->setByValue(by_value_chk->isChecked());
			type->setPreferred(preferred_chk->isChecked());
			type->setCollatable(collatable_chk->isChecked());

			if(!delimiter_edt->text().isEmpty())
				type->setDelimiter(delimiter_edt->text().at(0).toLatin1());

			type->setDefaultValue(default_value_edt->text());
			type->setCategory(CategoryType(category_cmb->currentText()));
			type->setAlignment(PgSqlType(alignment_cmb->currentText()));
			type->setStorage(StorageType(storage_cmb->currentText()));

			for(i = Type::InputFunc; i <= Type::AnalyzeFunc; i++)
			{
				type->setFunction(static_cast<Type::FunctionId>(i),
													dynamic_cast<Function *>(functions_sel[i]->getSelectedObject()));
			}
		}

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

