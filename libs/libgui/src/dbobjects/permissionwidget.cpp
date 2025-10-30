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

#include "permissionwidget.h"
#include "guiutilsns.h"

PermissionWidget::PermissionWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Permission)
{
	QGridLayout *grid=nullptr;
	QFrame *frame=nullptr;
	QCheckBox *check=nullptr;
	unsigned i;
	QString privs[]={ Attributes::SelectPriv, Attributes::InsertPriv,
					  Attributes::UpdatePriv, Attributes::DeletePriv,
					  Attributes::TruncatePriv, Attributes::ReferencesPriv,
					  Attributes::TriggerPriv, Attributes::CreatePriv,
					  Attributes::ConnectPriv, Attributes::TemporaryPriv,
					  Attributes::ExecutPriv, Attributes::UsagePriv };

	Ui_PermissionWidget::setupUi(this);

	code_hl=new SyntaxHighlighter(code_txt);
	code_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	object_selection_wgt=new ModelObjectsWidget(true);
	permission=nullptr;

	configureFormLayout(permission_grid, ObjectType::Permission);

	roles_tab=new CustomTableWidget(CustomTableWidget::AddButton |
									CustomTableWidget::RemoveButton |
									CustomTableWidget::EditButton, false, this);
	roles_tab->setColumnCount(1);
	roles_tab->setHeaderLabel(tr("Name"),0);
	roles_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);

	grid=new QGridLayout;
	grid->addWidget(roles_tab,0,0,1,1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	roles_gb->setLayout(grid);

	permissions_tab=new CustomTableWidget(CustomTableWidget::RemoveButton |
										  CustomTableWidget::EditButton |
										  CustomTableWidget::RemoveAllButton, true, this);
	permissions_tab->setColumnCount(3);
	permissions_tab->setHeaderLabel(tr("Id"),0);
	permissions_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("uid")),0);
	permissions_tab->setHeaderLabel(tr("Roles"),1);
	permissions_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("role")),1);
	permissions_tab->setHeaderLabel(tr("Privileges"),2);
	permissions_tab->setHeaderIcon(QPixmap(GuiUtilsNs::getIconPath("grant")),2);

	grid=new QGridLayout;
	grid->addWidget(permissions_tab,0,0,1,1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	permissions_gb->setLayout(grid);

	for(i=Permission::PrivSelect; i<=Permission::PrivUsage; i++)
	{
		check=new QCheckBox;
		check->setText(privs[i].toUpper());
		privileges_tbw->insertRow(i);
		privileges_tbw->setCellWidget(i,0,check);
		connect(check, &QCheckBox::clicked, this, &PermissionWidget::checkPrivilege);

		check=new QCheckBox;
		check->setText("GRANT OPTION");
		check->setEnabled(false);
		privileges_tbw->setCellWidget(i,1,check);
		connect(check, &QCheckBox::clicked, this, &PermissionWidget::checkPrivilege);
	}

	frame=generateInformationFrame(tr("Leave the <em><strong>Roles</strong></em> grid empty in order to create a %1 applicable to <strong><em>PUBLIC</em></strong>.")
																 .arg(BaseObject::getTypeName(ObjectType::Permission).toLower()));
	permission_grid->addWidget(frame, permission_grid->count()+1, 0, 1, 0);
	frame->setParent(this);

	connect(roles_tab, &CustomTableWidget::s_rowAdded, roles_tab, &CustomTableWidget::selectRow);
	connect(roles_tab, &CustomTableWidget::s_rowEdited, this, &PermissionWidget::selectRole);
	connect(roles_tab, &CustomTableWidget::s_rowRemoved, this, &PermissionWidget::enableEditButtons);
	connect(roles_tab, &CustomTableWidget::s_rowAdded, this, &PermissionWidget::enableEditButtons);
	connect(roles_tab, &CustomTableWidget::s_rowRemoved, this, &PermissionWidget::disableGrantOptions);
	connect(roles_tab, &CustomTableWidget::s_rowAdded, this, &PermissionWidget::disableGrantOptions);

	connect(permissions_tab, &CustomTableWidget::s_rowRemoved, this, &PermissionWidget::removePermission);
	connect(permissions_tab, &CustomTableWidget::s_rowEdited, this, &PermissionWidget::editPermission);
	connect(permissions_tab, &CustomTableWidget::s_rowSelected, this, &PermissionWidget::selectPermission);

	connect(cancel_tb, &QToolButton::clicked, this, &PermissionWidget::cancelOperation);
	connect(add_perm_tb, &QToolButton::clicked, this, __slot(this, PermissionWidget::addPermission));
	connect(upd_perm_tb, &QToolButton::clicked, this, __slot(this, PermissionWidget::updatePermission));

	connect(revoke_rb, &QRadioButton::toggled, cascade_chk, &QCheckBox::setEnabled);
	connect(revoke_rb, &QRadioButton::toggled, this, &PermissionWidget::disableGrantOptions);
	connect(grant_rb, &QRadioButton::toggled, this, &PermissionWidget::disableGrantOptions);

	setMinimumSize(670,600);

	configureTabOrder({ perm_id_edt, perm_disable_sql_chk, roles_tab, grant_rb, revoke_rb, cascade_chk,
											privileges_tbw, add_perm_tb, upd_perm_tb, cancel_tb, permissions_tab });
}

PermissionWidget::~PermissionWidget()
{
	delete object_selection_wgt;
}

void PermissionWidget::setAttributes(DatabaseModel *model, BaseObject *parent_obj, BaseObject *object)
{
	BaseObjectWidget::setAttributes(model,object,parent_obj);

	perms_changed=false;
	protected_obj_frm->setVisible(false);
	obj_id_lbl->setVisible(false);

	if(object)
	{
		unsigned priv;
		QCheckBox *chk=nullptr, *chk1=nullptr;

		connect(object_selection_wgt, qOverload<BaseObject *, bool>(&ModelObjectsWidget::s_visibilityChanged), this, [this](){
			__trycatch( showSelectedRoleData(); )
		});

		connect(roles_tab, &CustomTableWidget::s_rowAdded, this, &PermissionWidget::selectRole);
		connect(permissions_tab, &CustomTableWidget::s_rowsRemoved, this, __slot(this, PermissionWidget::removePermissions));

		name_edt->setText(QString("%1 (%2)").arg(object->getSignature()).arg(object->getTypeName()));

		for(priv=Permission::PrivSelect; priv<=Permission::PrivUsage; priv++)
		{
			//Gets the checkboxes that represents the privilege and the GRANT OPTION
			chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
			chk1=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));

			chk->setChecked(false);
			chk1->setChecked(false);

			//Enabling the checkboxes using a validation of privilege type against the curret object type.
			privileges_tbw->setRowHidden(priv, !Permission::acceptsPermission(object->getObjectType(), priv));
		}

		privileges_tbw->resizeColumnsToContents();

		listPermissions();
		permissions_tab->blockSignals(true);
		permissions_tab->clearSelection();
		permissions_tab->blockSignals(false);
		updateCodePreview();
	}
}

void PermissionWidget::selectRole()
{
	object_selection_wgt->setObjectVisible(ObjectType::Role, true);
	object_selection_wgt->setModel(this->model);
	object_selection_wgt->show();
}

void PermissionWidget::selectPermission(int perm_id)
{
	if(perm_id >= 0)
		permission=reinterpret_cast<Permission *>(permissions_tab->getRowData(perm_id).value<void *>());
	else
		permission=nullptr;
}

void PermissionWidget::disableGrantOptions()
{
	QCheckBox *check=nullptr;

	for(unsigned i=Permission::PrivSelect; i<=Permission::PrivUsage; i++)
	{
		check=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(i,1));
		check->setEnabled(roles_tab->getRowCount() > 0);

		if(!check->isEnabled())
			check->setChecked(false);
	}

	cascade_chk->setEnabled(revoke_rb->isChecked() && roles_tab->getRowCount() > 0);

	if(!cascade_chk->isEnabled())
		cascade_chk->setChecked(false);
}

void PermissionWidget::listPermissions()
{
	if(model)
	{
		std::vector<Permission *> permissions;
		Permission *perm=nullptr;
		unsigned i, count, i1, count1;
		QString str_aux;

		model->getPermissions(this->object, permissions);
		count=permissions.size();

		permissions_tab->blockSignals(true);
		permissions_tab->removeRows();
		permissions_tab->blockSignals(false);

		for(i=0; i < count; i++)
		{
			perm=permissions[i];

			permissions_tab->blockSignals(true);
			permissions_tab->addRow();
			permissions_tab->setRowData(QVariant::fromValue<void *>(reinterpret_cast<void *>(perm)),i);
			permissions_tab->setCellText(perm->getName(),i,0);
			permissions_tab->setCellText(perm->getPermissionString(),i,2);
			permissions_tab->blockSignals(false);

			count1=perm->getRoleCount();
			for(i1=0; i1 < count1; i1++)
			{
				str_aux+=perm->getRole(i1)->getName();
				str_aux+=",";
			}
			str_aux.remove(str_aux.size()-1,1);
			permissions_tab->setCellText(str_aux,i,1);
			str_aux.clear();
		}

		permission=nullptr;
	}
}

void PermissionWidget::showSelectedRoleData()
{
	int row, row_idx=-1;
	Role *role=nullptr;

	role=dynamic_cast<Role *>(object_selection_wgt->getSelectedObject());
	row=roles_tab->getSelectedRow();

	if(role)
		row_idx=roles_tab->getRowIndex(QVariant::fromValue<void *>(dynamic_cast<void *>(role)));

	if(role && row_idx < 0)
	{
		roles_tab->setCellText(role->getName(), row, 0);
		roles_tab->setRowData(QVariant::fromValue<void *>(dynamic_cast<void *>(role)), row);
	}
	else
	{
		if(!roles_tab->getRowData(row).value<void *>())
			roles_tab->removeRow(row);

		//Raise an error if the role already exists on selected role table
		if(role && row_idx >= 0)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedObjectContainer)
											.arg(role->getName())
											.arg(role->getTypeName())
											.arg(roles_gb->title().remove('&')),
											ErrorCode::InsDuplicatedRole,PGM_FUNC,PGM_FILE,PGM_LINE);
		}
	}
}

void PermissionWidget::addPermission()
{
	Permission *perm=nullptr;

	try
	{
		perm=new Permission(this->object);
		configurePermission(perm);
		model->addPermission(perm);
		model->addChangelogEntry(perm, Operation::ObjCreated);
		listPermissions();
		cancelOperation();
		perms_changed=true;
		updateCodePreview();
	}
	catch(Exception &e)
	{
		if(perm)
		{
			model->removePermission(perm);
			delete perm;
		}

		cancelOperation();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PermissionWidget::updatePermission()
{
	Permission *perm=nullptr,*perm_bkp=nullptr;
	int perm_idx=-1;

	try
	{
		perm=new Permission(this->object);

		/* Creates a backup permission. This will receive the current values of the
		editing permission, in case of errors these values can be restored */
		perm_bkp=new Permission(this->object);
		(*perm_bkp)=(*permission);

		configurePermission(perm);

		//Checking if the permission already exists on model
		perm_idx=model->getPermissionIndex(perm, false);

		if(perm_idx < 0 || (perm_idx >=0 && model->getObject(perm_idx,ObjectType::Permission)==permission))
		{
			(*permission)=(*perm);
			listPermissions();
			cancelOperation();
		}
		else
		{
			//Raises an error if the configured permission already exists
			throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedPermission)
							.arg(permission->getObject()->getName())
							.arg(permission->getObject()->getTypeName()),
							ErrorCode::AsgDuplicatedPermission,PGM_FUNC,PGM_FILE,PGM_LINE);
		}

		delete perm_bkp;
		perms_changed=true;
		updateCodePreview();
	}
	catch(Exception &e)
	{
		(*permission)=(*perm_bkp);

		delete perm;
		delete perm_bkp;

		cancelOperation();
		throw Exception(e.getErrorMessage(), e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

void PermissionWidget::editPermission()
{
	if(permission)
	{
		unsigned priv, i, count;
		QCheckBox *chk=nullptr, *chk1=nullptr;
		Role *role=nullptr;

		roles_tab->blockSignals(true);
		roles_tab->removeRows();

		perm_disable_sql_chk->setChecked(permission->isSQLDisabled());
		perm_id_edt->setText(permission->getName());
		revoke_rb->setChecked(permission->isRevoke());
		cascade_chk->setChecked(permission->isCascade());

		count=permission->getRoleCount();
		for(i=0; i < count; i++)
		{
			roles_tab->addRow();
			role=permission->getRole(i);
			roles_tab->setRowData(QVariant::fromValue<void *>(reinterpret_cast<void *>(role)), i);
			roles_tab->setCellText(role->getName(),i,0);
		}

		roles_tab->blockSignals(false);

		for(priv=Permission::PrivSelect; priv<=Permission::PrivUsage; priv++)
		{
			chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
			chk1=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));

			chk->setChecked(permission->getPrivilege(static_cast<Permission::PrivilegeId>(priv)));
			chk1->setChecked(permission->getGrantOption(static_cast<Permission::PrivilegeId>(priv)));
		}

		enableEditButtons();
	}
}

void PermissionWidget::removePermission(int)
{ 
	model->removePermission(permission);
	model->addChangelogEntry(permission, Operation::ObjRemoved);
	cancelOperation();
	permission=nullptr;
	permissions_tab->clearSelection();
	perms_changed=true;
	updateCodePreview();
}

void PermissionWidget::removePermissions()
{
	std::vector<Permission *> perms;

	model->getPermissions(object, perms);
	model->removePermissions(object);
	cancelOperation();
	perms_changed=true;
	updateCodePreview();

	for(auto &perm : perms)
		model->addChangelogEntry(perm, Operation::ObjRemoved);
}

void PermissionWidget::configurePermission(Permission *perm)
{
	if(perm)
	{
		unsigned count, i, priv;
		QCheckBox *chk=nullptr, *chk1=nullptr;

		perm->setSQLDisabled(perm_disable_sql_chk->isChecked());
		perm->setCascade(cascade_chk->isChecked());
		perm->setRevoke(revoke_rb->isChecked());

		perm->removeRoles();
		count=roles_tab->getRowCount();

		for(i=0; i < count; i++)
			perm->addRole(reinterpret_cast<Role *>(roles_tab->getRowData(i).value<void *>()));

		for(priv=Permission::PrivSelect; priv <= Permission::PrivUsage; priv++)
		{
			if(!privileges_tbw->isRowHidden(priv))
			{
				chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
				chk1=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));
				perm->setPrivilege(static_cast<Permission::PrivilegeId>(priv), chk->isChecked(), chk1->isChecked());
			}
		}
	}
}

void PermissionWidget::cancelOperation()
{
	unsigned priv;
	QCheckBox *chk=nullptr;

	permission=nullptr;

	for(priv=Permission::PrivSelect; priv<=Permission::PrivUsage; priv++)
	{
		chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
		chk->setChecked(false);
		chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));
		chk->setChecked(false);
	}

	roles_tab->removeRows();
	perm_id_edt->clear();
	enableEditButtons();
	cancel_tb->setEnabled(false);
	permissions_tab->clearSelection();
	perm_disable_sql_chk->setChecked(false);
}

void PermissionWidget::checkPrivilege()
{
	QObject *obj_sender=sender();

	if(obj_sender && obj_sender->metaObject()->className()==QString("QCheckBox"))
	{
		QCheckBox *chk=nullptr, *chk_priv=nullptr, *chk_gop=nullptr;
		unsigned priv;

		chk=dynamic_cast<QCheckBox *>(obj_sender);

		for(priv=Permission::PrivSelect; priv<=Permission::PrivUsage; priv++)
		{
			chk_priv=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
			chk_gop=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));

			if(chk==chk_gop)
			{
				chk_priv->setChecked(chk_gop->isChecked());
				break;
			}

			if(chk==chk_priv && !chk->isChecked())
			{
				chk_gop->setChecked(false);
				break;
			}
		}

		enableEditButtons();
	}
}

void PermissionWidget::enableEditButtons()
{
	bool checked_privs=false;
	unsigned priv;
	QCheckBox *chk=nullptr, *chk1=nullptr;

	for(priv=Permission::PrivSelect; priv<=Permission::PrivUsage && !checked_privs; priv++)
	{
		chk=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,0));
		chk1=dynamic_cast<QCheckBox *>(privileges_tbw->cellWidget(priv,1));
		checked_privs=(chk->isChecked() || chk1->isChecked());
	}

	upd_perm_tb->setEnabled(checked_privs && permission!=nullptr);
	add_perm_tb->setEnabled(checked_privs);
	cancel_tb->setEnabled(add_perm_tb->isEnabled() || upd_perm_tb->isEnabled() || permissions_tab->getRowCount() > 0);
}

void PermissionWidget::updateCodePreview()
{
	try
	{
		QString code;
		std::vector<Permission *> perms;
		unsigned i=0, cnt=0;

		model->getPermissions(this->object, perms);
		cnt=perms.size();

		for(i=0; i < cnt; i++)
			code+=perms[i]->getSourceCode(SchemaParser::SqlCode);

		if(code.isEmpty())
			code=tr("-- No permissions defined for the specified object!");

		code_txt->setPlainText(code);
	}
	catch(Exception &e)
	{
		QString str_aux;
		//In case of error no code is outputed, showing a error message in the code preview widget
		str_aux=tr("/* Could not generate the SQL code preview for permissions!");
		str_aux+=QString("\n\n>> Returned error(s): \n\n%1*/").arg(e.getExceptionsText());
		code_txt->setPlainText(str_aux);
	}
}

void PermissionWidget::applyConfiguration()
{
	if(perms_changed)
		emit s_objectManipulated();

	emit s_closeRequested();
}
