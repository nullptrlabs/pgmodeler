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

#include "policywidget.h"
#include "guiutilsns.h"

PolicyWidget::PolicyWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Policy)
{
	Ui_PolicyWidget::setupUi(this);

	model_objs_wgt = new ModelObjectsWidget(true, this);
	model_objs_wgt->setObjectVisible(ObjectType::Role, true);

	using_edt = GuiUtilsNs::createNumberedTextEditor(using_wgt, false, font().pointSizeF());
	using_edt->setTabChangesFocus(true);
	using_hl = new SyntaxHighlighter(using_edt, false, false, font().pointSizeF());
	using_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	check_edt = GuiUtilsNs::createNumberedTextEditor(check_wgt, false, font().pointSizeF());
	check_edt->setTabChangesFocus(true);
	check_hl = new SyntaxHighlighter(check_edt, false, false, font().pointSizeF());
	check_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	roles_tab = new CustomTableWidget(CustomTableWidget::AllButtons ^
																		 (CustomTableWidget::DuplicateButton |
																			CustomTableWidget::UpdateButton |
																			CustomTableWidget::EditButton), true, this);
	roles_tab->setColumnCount(1);
	roles_tab->setHeaderLabel(tr("Name"), 0);
	roles_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")), 0);

	QVBoxLayout *vbox = new QVBoxLayout;
	vbox->addWidget(roles_tab);

	QFrame *frame=generateInformationFrame(tr("Leave the <em><strong>Roles</strong></em> grid empty in order to create a %1 applicable to <strong><em>PUBLIC</em></strong>.")
																				 .arg(BaseObject::getTypeName(ObjectType::Policy).toLower()));
	vbox->addWidget(frame);
	frame->setParent(this);
	vbox->setContentsMargins(GuiUtilsNs::LtMargins);
	attribs_tbw->widget(0)->setLayout(vbox);

	command_cmb->addItems(PolicyCmdType::getTypes());

	configureFormLayout(policy_grid, ObjectType::Policy);
	configureTabOrder({ command_cmb, permissive_chk, attribs_tbw });

	connect(roles_tab, &CustomTableWidget::s_rowAdded, model_objs_wgt, &ModelObjectsWidget::show);
	connect(model_objs_wgt, qOverload<BaseObject *, bool>(&ModelObjectsWidget::s_visibilityChanged), this, &PolicyWidget::selectRole);
}

void PolicyWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseObject *parent_obj, Policy *policy)
{
	if(!parent_obj)
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	BaseObjectWidget::setAttributes(model, op_list, policy, parent_obj);
	model_objs_wgt->setModel(model);

	if(policy)
	{
		command_cmb->setCurrentText(~policy->getPolicyCommand());
		permissive_chk->setChecked(policy->isPermissive());
		check_edt->setPlainText(policy->getCheckExpression());
		using_edt->setPlainText(policy->getUsingExpression());

		roles_tab->blockSignals(true);

		for(auto role : policy->getRoles())
		{
			roles_tab->addRow();
			roles_tab->setCellText(role->getName(), roles_tab->getRowCount() - 1, 0);
			roles_tab->setRowData(QVariant::fromValue<void *>(reinterpret_cast<void*>(role)), roles_tab->getRowCount() - 1);
		}

		roles_tab->blockSignals(false);
	}
}

void PolicyWidget::selectRole(BaseObject *role, bool show_wgt)
{
	if(!show_wgt)
	{
		if(!role)
			roles_tab->removeRow(roles_tab->getRowCount() - 1);
		else
		{
			roles_tab->setCellText(role->getName(), roles_tab->getRowCount() - 1, 0);
			roles_tab->setRowData(QVariant::fromValue<void *>(reinterpret_cast<void*>(role)), roles_tab->getRowCount() - 1);
		}
	}
}

void PolicyWidget::applyConfiguration()
{
	try
	{
		Policy *policy = nullptr;
		unsigned count, i;

		startConfiguration<Policy>();

		policy =dynamic_cast<Policy *>(this->object);
		policy->removeRoles();
		policy->setUsingExpression(using_edt->toPlainText());
		policy->setCheckExpression(check_edt->toPlainText());
		policy->setPermissive(permissive_chk->isChecked());
		policy->setPolicyCommand(command_cmb->currentText());

		count=roles_tab->getRowCount();

		for(i=0; i < count; i++)
			policy->addRole(reinterpret_cast<Role *>(roles_tab->getRowData(i).value<void *>()));

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
