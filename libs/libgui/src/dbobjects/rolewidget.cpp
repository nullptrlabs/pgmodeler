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

#include "rolewidget.h"
#include "widgets/modelobjectswidget.h"
#include "guiutilsns.h"

RoleWidget::RoleWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Role)
{
	CustomTableWidget *obj_tab = nullptr;
	QGridLayout *grid = nullptr;
	QFrame *frame = nullptr;
	std::map<QString, std::vector<QWidget *> > fields_map;
	unsigned i;

	Ui_RoleWidget::setupUi(this);

	object_selection_wgt = new ModelObjectsWidget(true);

	frame = generateInformationFrame(tr("Assigning <strong><em>-1</em></strong> to <strong><em>Connections</em></strong> creates a role without connection limit.<br/>\
											Unchecking <strong><em>Validity</em></strong> creates an role that never expires."));

	attributes_lt->addWidget(frame);
	frame->setParent(attributes_tab);

	connect(validity_chk, &QCheckBox::toggled, validity_dte, &QDateTimeEdit::setEnabled);
	connect(members_twg, &QTabWidget::currentChanged, this, &RoleWidget::configureRoleSelection);

	//Alocation of the member role tables
	for(auto &tab : { members_tab, adm_members_tab, members_of_tab })
	{
		obj_tab = GuiUtilsNs::createWidgetInParent<CustomTableWidget>(
								GuiUtilsNs::LtMargin,
								CustomTableWidget::AllButtons ^
								(CustomTableWidget::UpdateButton | CustomTableWidget::DuplicateButton),
								true, tab);

		member_roles_tabs[tab] = obj_tab;
		obj_tab->setColumnCount(4);

		obj_tab->setHeaderLabel(tr("Role"), 0);
		obj_tab->setHeaderIcon(GuiUtilsNs::getIcon("role"), 0);

		obj_tab->setHeaderLabel(tr("Validity"), 1);
		obj_tab->setHeaderIcon(GuiUtilsNs::getIcon("validity"), 1);

		obj_tab->setHeaderLabel(tr("Members"), 2);
		obj_tab->setHeaderIcon(GuiUtilsNs::getIcon("role"), 2);

		obj_tab->setHeaderLabel(tr("Admin option"), 3);
		obj_tab->setHeaderIcon(GuiUtilsNs::getIcon("role"), 3);
	}

	configureTabbedLayout(members_twg);

	connect(object_selection_wgt, qOverload<BaseObject *, bool>(&ModelObjectsWidget::s_visibilityChanged), this, &RoleWidget::showSelectedRoleData);

	setMinimumSize(600, 320);
}

RoleWidget::~RoleWidget()
{
	delete object_selection_wgt;
}

void RoleWidget::configureRoleSelection()
{
	QWidget *curr_tab = members_twg->currentWidget();

	if(!member_roles_tabs.count(curr_tab))
		return;

	//Disconnects all signals from the member role tables
	for(auto &[owner_tab, table_wgt] : member_roles_tabs)
		disconnect(table_wgt, nullptr, this, nullptr);

	//Connects the signal/slots only on the current table
	connect(member_roles_tabs[curr_tab], &CustomTableWidget::s_rowAdded, this, &RoleWidget::selectMemberRole);
	connect(member_roles_tabs[curr_tab], &CustomTableWidget::s_rowEdited, this, &RoleWidget::selectMemberRole);
}

void RoleWidget::selectMemberRole()
{
	object_selection_wgt->setObjectVisible(ObjectType::Role, true);
	object_selection_wgt->setModel(this->model);
	object_selection_wgt->show();
}

void RoleWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Role *role)
{
	if(role)
	{
		conn_limit_sb->setValue(role->getConnectionLimit());
		password_edt->setText(role->getPassword());

		validity_chk->setChecked(!role->getValidity().isEmpty());
		validity_dte->setDateTime(QDateTime::fromString(role->getValidity(), "yyyy-MM-dd hh:mm:ss"));

		superusr_chk->setChecked(role->getOption(Role::OpSuperuser));
		create_db_chk->setChecked(role->getOption(Role::OpCreateDb));
		create_role_chk->setChecked(role->getOption(Role::OpCreateRole));
		inh_perm_chk->setChecked(role->getOption(Role::OpInherit));
		can_login_chk->setChecked(role->getOption(Role::OpLogin));
		can_replicate_chk->setChecked(role->getOption(Role::OpReplication));
		bypass_rls_chk->setChecked(role->getOption(Role::OpBypassRls));
	}

	BaseObjectWidget::setAttributes(model, op_list, role);
	op_list->startOperationChain();
	fillMembersTable();
	configureRoleSelection();
}

void RoleWidget::showRoleData(Role *role, CustomTableWidget *role_tab, unsigned row)
{
	if(!role || !role_tab)
		throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);

	QStringList rl_names;
	Role *aux_role = nullptr;

	role_tab->setRowData(QVariant::fromValue(reinterpret_cast<void *>(role)), row);
	role_tab->setCellText(role->getName(), row, 0);
	role_tab->setCellText(role->getValidity(), row, 1);

	for(auto &type_id : { Role::MemberRole, Role::AdminRole })
	{
		for(unsigned rl_id = 0; rl_id < role->getRoleCount(type_id); rl_id++)
		{
			aux_role = role->getRole(type_id, rl_id);
			rl_names.append(aux_role->getName());
		}

		role_tab->setCellText(rl_names.join(", "), row, 2 + type_id);
		rl_names.clear();
	}
}

CustomTableWidget *RoleWidget::getRolesTable(Role::RoleType rl_type)
{
	QWidget *tab = (rl_type == Role::MemberRole ? members_tab : adm_members_tab);

	if(!member_roles_tabs.count(tab))
		return nullptr;

	return member_roles_tabs[tab];
}

void RoleWidget::fillMembersTable()
{
	if(!this->object)
		return;

	Role *aux_role = nullptr, *role = dynamic_cast<Role *>(this->object);
	CustomTableWidget *role_tab = nullptr;

	for(auto &rl_type : { Role::MemberRole, Role::AdminRole })
	{
		role_tab = getRolesTable(rl_type);
		role_tab->blockSignals(true);

		for(unsigned rl_id = 0; rl_id < role->getRoleCount(rl_type); rl_id++)
		{
			aux_role = role->getRole(rl_type, rl_id);
			role_tab->addRow();
			showRoleData(aux_role, role_tab, rl_id);
		}

		role_tab->blockSignals(false);
		role_tab->clearSelection();
	}
}

void RoleWidget::showSelectedRoleData()
{
	try
	{
		unsigned idx_tab = 0;
		int row = 0, idx_row = -1;
		BaseObject *obj_sel = nullptr;
		CustomTableWidget *role_tab = member_roles_tabs[members_twg->currentWidget()];
		Role::RoleType rl_type = members_twg->currentWidget() == members_tab ? Role::MemberRole : Role::AdminRole;

		//Get the selected role
		obj_sel = object_selection_wgt->getSelectedObject();

		//Gets the index of the table where the role data is displayed
		idx_tab = members_twg->currentIndex();
		row = role_tab->getSelectedRow();

		if(obj_sel)
			idx_row = role_tab->getRowIndex(QVariant::fromValue<void *>(dynamic_cast<void *>(obj_sel)));

		if(obj_sel && idx_row < 0)
			showRoleData(dynamic_cast<Role *>(obj_sel), role_tab, row);
		else
		{
			/* If the current row does not has a value indicates that it is recently added and does not have
				 data, in this case it will be removed */
			if(!role_tab->getRowData(row).value<void *>())
				role_tab->removeRow(row);

			//Raises an error if the role already is in the table
			if(obj_sel && idx_row >= 0)
			{
				Messagebox::error(Exception::getErrorMessage(ErrorCode::InsDuplicatedRole).arg(obj_sel->getName(), name_edt->text()),
													ErrorCode::InsDuplicatedRole, PGM_FUNC, PGM_FILE, PGM_LINE);
			}
		}
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

void RoleWidget::applyConfiguration()
{
	Role *role=nullptr, *aux_role=nullptr;

	try
	{
		startConfiguration<Role>();

		role=dynamic_cast<Role *>(this->object);
		role->setConnectionLimit(conn_limit_sb->value());
		role->setPassword(password_edt->text());

		if(validity_chk->isChecked())
			role->setValidity(validity_dte->dateTime().toString("yyyy-MM-dd hh:mm"));
		else
			role->setValidity("");

		role->setOption(Role::OpSuperuser, superusr_chk->isChecked());
		role->setOption(Role::OpCreateDb, create_db_chk->isChecked());
		role->setOption(Role::OpCreateRole, create_role_chk->isChecked());
		role->setOption(Role::OpInherit, inh_perm_chk->isChecked());
		role->setOption(Role::OpLogin, can_login_chk->isChecked());
		role->setOption(Role::OpReplication, can_replicate_chk->isChecked());
		role->setOption(Role::OpBypassRls, bypass_rls_chk->isChecked());

		CustomTableWidget *role_tab = nullptr;

		for(auto rl_type : { Role::MemberRole, Role::AdminRole })
		{
			role_tab = getRolesTable(rl_type);
			role->removeRoles(rl_type);

			for(unsigned rl_id = 0; rl_id < role_tab->getRowCount(); rl_id++)
			{
				aux_role = reinterpret_cast<Role *>(role_tab->getRowData(rl_id).value<void *>());
				role->addRole(rl_type, aux_role);
			}
		}

		/* Special case for Member Of tab, here we try to add the role being edited
		 * as a member of the the roles in the table */
		role_tab = member_roles_tabs[members_of_tab];

		for(unsigned rl_id = 0; rl_id < role_tab->getRowCount(); rl_id++)
		{
			aux_role = reinterpret_cast<Role *>(role_tab->getRowData(rl_id).value<void *>());

			/* Raises an error if the role to be added is the postgres one
			 * For now, there is no way to assign roles direct to the postgres role due to
			 * its permanet protection status. May be changed in future releases */
			if(aux_role->isSystemObject())
			{
				throw Exception(Exception::getErrorMessage(ErrorCode::OprReservedObject)
												.arg(aux_role->getName(), aux_role->getTypeName()),
												ErrorCode::OprReservedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
			}

			op_list->registerObject(aux_role, Operation::ObjModified);
			aux_role->addRole(Role::MemberRole, role);
		}

		BaseObjectWidget::applyConfiguration();
		op_list->finishOperationChain();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
