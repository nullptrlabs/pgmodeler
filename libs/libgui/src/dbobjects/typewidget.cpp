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
	QGridLayout *grid=nullptr;
	QFrame *frame=nullptr;
	unsigned i,i1;

	Ui_TypeWidget::setupUi(this);
	configureFormLayout(type_grid, ObjectType::Type);

	like_type=new PgSQLTypeWidget(this, tr("Like Type"));
	element_type=new PgSQLTypeWidget(this, tr("Element Type"));
	range_subtype=new PgSQLTypeWidget(this, tr("Subtype"));

	grid=dynamic_cast<QGridLayout *>(base_attribs_twg->widget(1)->layout());
	grid->addWidget(like_type,6,0,1,0);
	grid->addWidget(element_type,7,0,1,0);
	grid->addItem(new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding),8,0);

	grid=dynamic_cast<QGridLayout *>(base_attribs_twg->widget(0)->layout());
	for(i=Type::InputFunc; i <= Type::AnalyzeFunc; i++)
	{
		functions_sel[i]=nullptr;
		functions_sel[i]=new ObjectSelectorWidget(ObjectType::Function, this);
		grid->addWidget(functions_sel[i],i,1,1,1);
	}

	enumerations_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
											 (CustomTableWidget::EditButton | CustomTableWidget::DuplicateButton), true, this);
	grid=dynamic_cast<QGridLayout *>(enumerations_gb->layout());
	grid->addWidget(enumerations_tab,1,0,1,2);
	enumerations_gb->setVisible(false);

	attributes_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^ CustomTableWidget::DuplicateButton, true, this);
	attributes_tab->setColumnCount(3);
	attributes_tab->setHeaderLabel(tr("Name"),0);
	attributes_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	attributes_tab->setHeaderLabel(tr("Type"),1);
	attributes_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("usertype")),1);
	attributes_tab->setHeaderLabel(tr("Collation"),2);
	attributes_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("collation")),2);

	grid=dynamic_cast<QGridLayout *>(attributes_gb->layout());

	attrib_collation_sel=new ObjectSelectorWidget(ObjectType::Collation, this);
	grid->addWidget(attrib_collation_sel, 1,1,1,2);

	attrib_type_wgt=new PgSQLTypeWidget(this);
	grid->addWidget(attrib_type_wgt,2,0,1,2);

	grid->addWidget(attributes_tab,3,0,1,2);
	attributes_gb->setVisible(false);

	grid=dynamic_cast<QGridLayout *>(base_attribs_twg->widget(0)->layout());
	frame=generateInformationFrame(tr("The functions to be assigned to a type should be written in C language and possess, respectively, the following signatures:<br/>  <table>   <tr>    <td><strong>INPUT:</strong> <em>any function(cstring, oid, integer)</em></td>    <td><strong>OUTPUT:</strong> <em>cstring function(any)</em></td>   </tr>   <tr>    <td><strong>SEND:</strong> <em>byta function(any)</em></td>    <td><strong>RECV:</strong> <em>any function(internal, oid, integer)</em></td>   </tr>   <tr>    <td><strong>TPMOD_IN:</strong> <em>integer function(cstring[])</em></td>    <td><strong>TPMOD_OUT:</strong> <em>cstring function(integer)</em></td>   </tr>   <tr>    <td><strong>ANALYZE:</strong> <em>boolean function(internal)</em></td>    <tr>  </table>"));

	grid->addItem(new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding), grid->count()+1,0);
	grid->addWidget(frame, grid->count()+1, 0, 1, 0);
	frame->setParent(base_attribs_twg->widget(0));

	grid=dynamic_cast<QGridLayout *>(range_attribs_gb->layout());
	opclass_sel=new ObjectSelectorWidget(ObjectType::OpClass, this);
	grid->addWidget(opclass_sel,0,1,1,1);

	for(i1=1, i=Type::CanonicalFunc; i <= Type::SubtypeDiffFunc; i++,i1++)
	{
		functions_sel[i]=nullptr;
		functions_sel[i]=new ObjectSelectorWidget(ObjectType::Function, this);
		grid->addWidget(functions_sel[i],i1,1,1,1);
	}

	grid->addWidget(range_subtype,3,0,1,2);
	frame=generateInformationFrame(tr("The functions to be assigned to a range type should have the following signatures:<br/><br/><strong>Canonical:</strong> <em>any function(any)</em> <br/><strong>Subtype Diff:</strong> <em>double precision function(subtype, subtype)</em>"));
	grid->addItem(new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::Expanding),4,0);
	grid->addWidget(frame, 5, 0, 1, 0);

	range_attribs_gb->setVisible(false);

	connect(base_type_rb, &QRadioButton::toggled, this, &TypeWidget::selectTypeConfiguration);
	connect(composite_rb, &QRadioButton::toggled, this, &TypeWidget::selectTypeConfiguration);
	connect(enumeration_rb, &QRadioButton::toggled, this, &TypeWidget::selectTypeConfiguration);
	connect(enumerations_tab, &CustomTableWidget::s_rowAdded, this, &TypeWidget::handleEnumeration);
	connect(enumerations_tab, &CustomTableWidget::s_rowUpdated, this, &TypeWidget::handleEnumeration);
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
	setRequiredField(enumerations_gb);
	setRequiredField(attributes_gb);

	configureTabOrder({base_type_rb, enumeration_rb, composite_rb, range_rb,
						 enum_name_edt, attrib_name_edt, attrib_collation_sel, attrib_type_wgt,
						 opclass_sel, functions_sel[Type::CanonicalFunc], functions_sel[Type::SubtypeDiffFunc],
						 base_attribs_twg});

	setMinimumSize(620, 750);
}

void TypeWidget::selectTypeConfiguration()
{
	base_attribs_twg->setVisible(base_type_rb->isChecked());
	enumerations_gb->setVisible(enumeration_rb->isChecked());
	attributes_gb->setVisible(composite_rb->isChecked());
	range_attribs_gb->setVisible(range_rb->isChecked());
	collation_sel->setEnabled(range_rb->isChecked());
	collation_lbl->setEnabled(range_rb->isChecked());

	if(!range_rb->isChecked())
		collation_sel->clearSelector();
}

void TypeWidget::handleEnumeration(int row)
{
	if(!enum_name_edt->text().isEmpty())
	{
		enumerations_tab->setCellText(enum_name_edt->text(), row, 0);
		enum_name_edt->clear();
	}
	else if(enumerations_tab->getCellText(row, 0).isEmpty())
		enumerations_tab->removeRow(row);
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
	unsigned type_conf, i, count;

	BaseObjectWidget::setAttributes(model, op_list, type, schema);

	attrib_collation_sel->setModel(model);
	attrib_type_wgt->setAttributes(PgSqlType(), model);
	range_subtype->setAttributes(PgSqlType(), model);

	opclass_sel->setModel(model);

	for(i=Type::InputFunc; i <= Type::SubtypeDiffFunc; i++)
		functions_sel[i]->setModel(model);

	if(type)
	{
		type_conf=type->getConfiguration();

		if(type_conf==Type::CompositeType)
		{
			composite_rb->setChecked(true);
			attributes_tab->blockSignals(true);
			count=type->getAttributeCount();

			for(i=0; i < count; i++)
			{
				attributes_tab->addRow();
				showAttributeData(type->getAttribute(i),i);
			}

			attributes_tab->blockSignals(false);
			attributes_tab->clearSelection();
		}
		else if(type_conf==Type::EnumerationType)
		{
			enumeration_rb->setChecked(true);
			enumerations_tab->blockSignals(true);
			count=type->getEnumerationCount();

			for(i=0; i < count; i++)
			{
				enumerations_tab->addRow();
				enumerations_tab->setCellText(type->getEnumeration(i), i, 0);
			}

			enumerations_tab->blockSignals(false);
			enumerations_tab->clearSelection();
		}
		else if(type_conf==Type::RangeType)
		{
			range_rb->setChecked(true);
			opclass_sel->setSelectedObject(type->getSubtypeOpClass());
			range_subtype->setAttributes(type->getSubtype(), model);
			functions_sel[Type::CanonicalFunc]->setSelectedObject(type->getFunction(Type::CanonicalFunc));
			functions_sel[Type::SubtypeDiffFunc]->setSelectedObject(type->getFunction(Type::SubtypeDiffFunc));
		}
		else
		{
			base_type_rb->setChecked(true);

			like_tp=type->getLikeType();
			elem_tp=type->getElement();

			internal_len_sb->setValue(type->getInternalLength());
			by_value_chk->setChecked(type->isByValue());
			preferred_chk->setChecked(type->isPreferred());
			collatable_chk->setChecked(type->isCollatable());
			delimiter_edt->setText(QString(type->getDelimiter()));
			default_value_edt->setText(type->getDefaultValue());
			category_cmb->setCurrentIndex(category_cmb->findText(~type->getCategory()));
			storage_cmb->setCurrentIndex(storage_cmb->findText(~type->getStorage()));
			alignment_cmb->setCurrentIndex(alignment_cmb->findText(~type->getAlignment()));

			for(i=Type::InputFunc; i <= Type::AnalyzeFunc; i++)
				functions_sel[i]->setSelectedObject(type->getFunction(static_cast<Type::FunctionId>(i)));
		}
	}
	else
		selectTypeConfiguration();

	like_type->setAttributes(like_tp, model);
	element_type->setAttributes(elem_tp, model);
}

void TypeWidget::applyConfiguration()
{
	try
	{
		Type *type=nullptr;
		unsigned i, count;

		startConfiguration<Type>();
		type=dynamic_cast<Type *>(this->object);

		BaseObjectWidget::applyConfiguration();

		//Configuring an enumaration type
		if(enumeration_rb->isChecked())
		{
			type->setConfiguration(Type::EnumerationType);

			count=enumerations_tab->getRowCount();
			for(i=0; i < count; i++)
				type->addEnumeration(enumerations_tab->getCellText(i,0).toUtf8());
		}
		//Configuration a composite type
		else if(composite_rb->isChecked())
		{
			type->setConfiguration(Type::CompositeType);

			count=attributes_tab->getRowCount();
			for(i=0; i < count; i++)
				type->addAttribute(attributes_tab->getRowData(i).value<TypeAttribute>());
		}
		//Configuration a range type
		else if(range_rb->isChecked())
		{
			type->setConfiguration(Type::RangeType);
			type->setCollation(collation_sel->getSelectedObject());
			type->setSubtype(range_subtype->getPgSQLType());
			type->setSubtypeOpClass(dynamic_cast<OperatorClass *>(opclass_sel->getSelectedObject()));
			type->setFunction(Type::CanonicalFunc, dynamic_cast<Function *>(functions_sel[Type::CanonicalFunc]->getSelectedObject()));
			type->setFunction(Type::SubtypeDiffFunc, dynamic_cast<Function *>(functions_sel[Type::SubtypeDiffFunc]->getSelectedObject()));
		}
		//Configuring a base type
		else
		{
			type->setConfiguration(Type::BaseType);
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

			for(i=Type::InputFunc; i <= Type::AnalyzeFunc; i++)
				type->setFunction(static_cast<Type::FunctionId>(i),
													dynamic_cast<Function *>(functions_sel[i]->getSelectedObject()));
		}

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

