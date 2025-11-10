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

#include "referenceswidget.h"
#include "guiutilsns.h"

const QRegularExpression ReferencesWidget::AttrDelimRegexp {
	QString("(\\%1)+|(\\%2)+")
		.arg(SchemaParser::CharStartAttribute)
		.arg(SchemaParser::CharEndAttribute)
};

ReferencesWidget::ReferencesWidget(const std::vector<ObjectType> &types, bool conf_view_refs, QWidget *parent): QWidget(parent)
{
	Ui_ReferencesWidget::setupUi(this);

	object_sel = new ObjectSelectorWidget(types, this);
	references_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																								 CustomTableWidget::DuplicateButton, true, this);

	this->conf_view_refs = conf_view_refs;

	references_grid->addWidget(object_sel, 0, 1, 1, 1);
	references_grid->addWidget(references_tab, 3, 0, 1, 2);

	references_tab->setColumnCount(conf_view_refs ? 7 : 6);

	references_tab->setHeaderLabel(tr("Ref. name"), 0);
	references_tab->setHeaderIcon(GuiUtilsNs::getIcon("uid"), 0);

	references_tab->setHeaderLabel(tr("Ref. alias"), 1);

	references_tab->setHeaderLabel(tr("Object"), 2);
	references_tab->setHeaderIcon(GuiUtilsNs::getIcon("objects"), 2);

	references_tab->setHeaderLabel(tr("Type"), 3);
	references_tab->setHeaderIcon(GuiUtilsNs::getIcon(BaseObject::getSchemaName(ObjectType::Type)), 3);

	references_tab->setHeaderLabel(tr("Use signature"), 4);
	references_tab->setHeaderLabel(tr("Format name"), 5);

	if(conf_view_refs)
		references_tab->setHeaderLabel(tr("Use column(s)"), 6);

	use_columns_chk->setVisible(conf_view_refs);

	connect(object_sel, &ObjectSelectorWidget::s_selectorChanged, this, [this](bool selected){
		sel_obj_icon_lbl->setPixmap(selected ? GuiUtilsNs::getIconPath(object_sel->getSelectedObject()->getSchemaName()) : QPixmap());
		sel_obj_icon_lbl->setToolTip(selected ? object_sel->getSelectedObject()->getTypeName() : "");
	});

	connect(references_tab, &CustomTableWidget::s_rowAdded, this, &ReferencesWidget::handleReference);
	connect(references_tab, &CustomTableWidget::s_rowEdited, this, &ReferencesWidget::editReference);
	connect(references_tab, &CustomTableWidget::s_rowUpdated, this, &ReferencesWidget::handleReference);

	connect(ref_name_edt, &QLineEdit::textChanged, this, [this](const QString &txt){
		references_tab->setButtonsEnabled(CustomTableWidget::AddButton, !txt.isEmpty() && object_sel->getSelectedObject());
	});

	connect(object_sel, &ObjectSelectorWidget::s_selectorChanged, this, [this](bool obj_selected){
		references_tab->setButtonsEnabled(CustomTableWidget::AddButton, !ref_name_edt->text().isEmpty() && obj_selected);
		use_columns_chk->setEnabled(obj_selected &&
																(BaseTable::isBaseTable(object_sel->getSelectedObject()->getObjectType()) ||
																 object_sel->getSelectedObject()->getObjectType() == ObjectType::Column));

		if(!use_columns_chk->isEnabled())
			use_columns_chk->setChecked(false);
	});

	connect(format_name_chk, &QCheckBox::toggled, use_signature_chk, &QCheckBox::setEnabled);
}

void ReferencesWidget::setAttributes(DatabaseModel *model, const std::vector<Reference> &refs)
{
	references_tab->blockSignals(true);

	for(auto &ref : refs)
	{
		references_tab->addRow();
		showReferenceData(references_tab->getRowCount() - 1,
														 ref.getObject(), ref.getRefName(), ref.getRefAlias(),
														 ref.isUseSignature(), ref.isFormatName(), ref.isUseColumns());
	}

	references_tab->clearSelection();
	references_tab->blockSignals(false);
	object_sel->setModel(model);
	references_tab->setButtonsEnabled(CustomTableWidget::AddButton, false);
}

std::vector<Reference> ReferencesWidget::getObjectReferences()
{
	std::vector<Reference> refs;

	for(unsigned row = 0; row < references_tab->getRowCount(); row++)
		refs.push_back(references_tab->getRowData(row).value<Reference>());

	return refs;
}

void ReferencesWidget::handleReference(int row)
{
	showReferenceData(row,
										object_sel->getSelectedObject(),
										ref_name_edt->text().remove(AttrDelimRegexp),
										ref_alias_edt->text().remove(AttrDelimRegexp),
										use_signature_chk->isChecked(),
										format_name_chk->isChecked(),
										use_columns_chk->isChecked());
	clearReferenceForm();
}

void ReferencesWidget::editReference(int row)
{
	Reference ref = references_tab->getRowData(row).value<Reference>();

	ref_name_edt->setText(ref.getRefName());
	ref_alias_edt->setText(ref.getRefAlias());
	use_signature_chk->setChecked(ref.isUseSignature());
	format_name_chk->setChecked(ref.isFormatName());
	use_columns_chk->setChecked(ref.isUseColumns() && conf_view_refs);
	object_sel->setSelectedObject(ref.getObject());
}

void ReferencesWidget::clearReferenceForm()
{
	object_sel->clearSelector();
	ref_name_edt->clear();
	ref_alias_edt->clear();
	use_signature_chk->setChecked(false);
	format_name_chk->setChecked(false);
	references_tab->clearSelection();
	references_tab->setButtonsEnabled(CustomTableWidget::AddButton, false);
}

void ReferencesWidget::showReferenceData(int row, BaseObject *object, const QString &ref_name, const QString &ref_alias,
																										 bool use_signature, bool format_name, bool use_columns)
{
	Reference ref = Reference(object, ref_name, ref_alias, use_signature, format_name, use_columns);

	references_tab->setCellText(ref_name, row, 0);
	references_tab->setCellText(ref_alias, row, 1);
	references_tab->setCellText(object->getSignature(), row, 2);
	references_tab->setCellText(object->getTypeName(), row, 3);
	references_tab->setCellText(use_signature ? tr("Yes") : tr("No"), row, 4);
	references_tab->setCellText(format_name ? tr("Yes") : tr("No"), row, 5);

	if(conf_view_refs)
		references_tab->setCellText(use_columns ? tr("Yes") : tr("No"), row, 6);

	references_tab->setRowData(QVariant::fromValue<Reference>(ref), row);
}
