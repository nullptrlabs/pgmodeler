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

#include "rolewidget.h"
#include "widgets/modelobjectswidget.h"
#include "guiutilsns.h"

RoleWidget::RoleWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Role)
{
	CustomTableWidget *obj_tab=nullptr;
	QGridLayout *grid=nullptr;
	QFrame *frame=nullptr;
	std::map<QString, std::vector<QWidget *> > fields_map;
	unsigned i;

	Ui_RoleWidget::setupUi(this);
	configureFormLayout(role_grid, ObjectType::Role);

	object_selection_wgt=new ModelObjectsWidget(true);

	frame=generateInformationFrame(tr("Assigning <strong><em>-1</em></strong> to <strong><em>Connections</em></strong> creates a role without connection limit.<br/>\
										  Unchecking <strong><em>Validity</em></strong> creates an role that never expires."));

	role_grid->addWidget(frame, role_grid->count()+1, 0, 1, 4);
	frame->setParent(this);

	connect(validity_chk, &QCheckBox::toggled, validity_dte, &QDateTimeEdit::setEnabled);
	connect(members_twg, &QTabWidget::currentChanged, this, &RoleWidget::configureRoleSelection);

	//Alocation of the member role tables
	for(i=0; i < 3; i++)
	{
		obj_tab=new CustomTableWidget(CustomTableWidget::AllButtons ^
																	(CustomTableWidget::UpdateButton | CustomTableWidget::DuplicateButton), true, this);
		members_tab[i]=obj_tab;

		obj_tab->setColumnCount(4);

		obj_tab->setHeaderLabel(tr("Role"),0);
		obj_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("role")),0);

		obj_tab->setHeaderLabel(tr("Validity"),1);
		obj_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("validity")),1);

		obj_tab->setHeaderLabel(tr("Members"),2);
		obj_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("role")),2);

		obj_tab->setHeaderLabel(tr("Admin option"),3);
		obj_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("role")),3);

		grid=new QGridLayout;
		grid->addWidget(obj_tab,0,0,1,1);
		grid->setContentsMargins(GuiUtilsNs::LtMargins);
		members_twg->widget(i)->setLayout(grid);
	}

	connect(object_selection_wgt, qOverload<BaseObject *, bool>(&ModelObjectsWidget::s_visibilityChanged), this, &RoleWidget::showSelectedRoleData);

	setMinimumSize(580, 550);
}

RoleWidget::~RoleWidget()
{
	delete object_selection_wgt;
}

void RoleWidget::configureRoleSelection()
{
	//Disconnects all signals from the member role tables
	for(auto & memb_tb : members_tab)
		disconnect(memb_tb, nullptr,this, nullptr);

	//Connects the signal/slots only on the current table
	connect(members_tab[members_twg->currentIndex()], &CustomTableWidget::s_rowAdded, this, &RoleWidget::selectMemberRole);
	connect(members_tab[members_twg->currentIndex()], &CustomTableWidget::s_rowEdited, this, &RoleWidget::selectMemberRole);
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
		passwd_edt->setText(role->getPassword());

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

void RoleWidget::showRoleData(Role *role, unsigned table_id, unsigned row)
{
	if(role)
	{
		QStringList rl_names;
		Role *aux_role=nullptr;

		if(table_id > 3)
			throw Exception(ErrorCode::RefObjectInvalidIndex,PGM_FUNC,PGM_FILE,PGM_LINE);

		members_tab[table_id]->setRowData(QVariant::fromValue(reinterpret_cast<void *>(role)), row);
		members_tab[table_id]->setCellText(role->getName(), row, 0);
		members_tab[table_id]->setCellText(role->getValidity(), row, 1);

		for(auto type_id : { Role::MemberRole, Role::AdminRole })
		{
			for(unsigned i=0; i < role->getRoleCount(type_id); i++)
			{
				aux_role = role->getRole(type_id, i);
				rl_names.append(aux_role->getName());
			}

			members_tab[table_id]->setCellText(rl_names.join(", "), row, 2 + type_id);
			rl_names.clear();
		}
	}
}

void RoleWidget::fillMembersTable()
{
	if(this->object)
	{
		Role *aux_role=nullptr, *role=nullptr;
		role=dynamic_cast<Role *>(this->object);

		for(auto type_id : { Role::MemberRole, Role::AdminRole })
		{
			members_tab[type_id]->blockSignals(true);

			for(unsigned i=0; i < role->getRoleCount(type_id); i++)
			{
				aux_role=role->getRole(type_id, i);
				members_tab[type_id]->addRow();
				showRoleData(aux_role, type_id, i);
			}

			members_tab[type_id]->blockSignals(false);
			members_tab[type_id]->clearSelection();
		}
	}
}

void RoleWidget::showSelectedRoleData()
{
	try
	{
		unsigned idx_tab = 0;
		int lin = 0, idx_lin = -1;
		BaseObject *obj_sel=nullptr;

					 //Get the selected role
		obj_sel=object_selection_wgt->getSelectedObject();

					 //Gets the index of the table where the role data is displayed
		idx_tab=members_twg->currentIndex();
		lin=members_tab[idx_tab]->getSelectedRow();

		if(obj_sel)
			idx_lin=members_tab[idx_tab]->getRowIndex(QVariant::fromValue<void *>(dynamic_cast<void *>(obj_sel)));

		if(obj_sel && idx_lin < 0)
			showRoleData(dynamic_cast<Role *>(obj_sel), idx_tab, lin);
		else
		{
			/* If the current row does not has a value indicates that it is recently added and does not have
				 data, in this case it will be removed */
			if(!members_tab[idx_tab]->getRowData(lin).value<void *>())
				members_tab[idx_tab]->removeRow(lin);

			//Raises an error if the role already is in the table
			if(obj_sel && idx_lin >= 0)
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
		role->setPassword(passwd_edt->text());

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

		for(auto rl_type : { Role::MemberRole, Role::AdminRole })
		{
			role->removeRoles(rl_type);

			for(unsigned i = 0; i < members_tab[rl_type]->getRowCount(); i++)
			{
				aux_role=reinterpret_cast<Role *>(members_tab[rl_type]->getRowData(i).value<void *>());
				role->addRole(rl_type, aux_role);
			}
		}

		/* Special case for Member Of tab, here we try to add the role being edited
		 * as a member of the the roles in the table */
		for(unsigned i = 0; i < members_tab[2]->getRowCount(); i++)
		{
			aux_role = reinterpret_cast<Role *>(members_tab[2]->getRowData(i).value<void *>());

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
