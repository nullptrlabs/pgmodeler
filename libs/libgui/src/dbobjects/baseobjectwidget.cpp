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

#include "baseobjectwidget.h"
#include "customuistyle.h"
#include "messagebox.h"
#include "permissionwidget.h"
#include "widgets/customsqlwidget.h"
#include "baseform.h"
#include "settings/generalconfigwidget.h"
#include "utilsns.h"
#include "dbobjects/pgsqltypewidget.h"
#include "guiutilsns.h"

BaseObjectWidget::BaseObjectWidget(QWidget *parent, ObjectType obj_type): QWidget(parent)
{
	QSpacerItem *spacer=nullptr;

	setWindowTitle("");
	setupUi(this);

	handled_obj_type=obj_type;
	operation_count=0;
	new_object=false;
	model=nullptr;
	table=nullptr;
	relationship=nullptr;
	prev_schema=nullptr;
	op_list=nullptr;
	object=nullptr;
	object_px=DNaN;
	object_py=DNaN;
	schema_sel=nullptr;
	owner_sel=nullptr;
	tablespace_sel=nullptr;
	object_protected = false;

	connect(edt_perms_tb, &QPushButton::clicked, this, &BaseObjectWidget::editPermissions);
	connect(append_sql_tb, &QPushButton::clicked, this, &BaseObjectWidget::editCustomSQL);

	schema_sel=new ObjectSelectorWidget(ObjectType::Schema, this);
	collation_sel=new ObjectSelectorWidget(ObjectType::Collation, this);
	tablespace_sel=new ObjectSelectorWidget(ObjectType::Tablespace, this);
	owner_sel=new ObjectSelectorWidget(ObjectType::Role, this);

	baseobject_grid = new QGridLayout;
	baseobject_grid->setObjectName("objetobase_grid");
	baseobject_grid->addWidget(protected_obj_frm, 0, 0, 1, 0);
	baseobject_grid->addWidget(name_lbl, 1, 0, 1, 1);
	baseobject_grid->addWidget(name_edt, 1, 1, 1, 1);
	baseobject_grid->addWidget(id_ico_wgt, 1, 2, 1, 3);
	baseobject_grid->addWidget(logical_name_lbl, 2, 0, 1, 1);
	baseobject_grid->addWidget(alias_edt, 2, 1, 1, 1);
	baseobject_grid->addWidget(schema_lbl, 4, 0, 1, 1);
	baseobject_grid->addWidget(schema_sel, 4, 1, 1, 4);
	baseobject_grid->addWidget(collation_lbl, 5, 0, 1, 1);
	baseobject_grid->addWidget(collation_sel, 5, 1, 1, 4);
	baseobject_grid->addWidget(tablespace_lbl, 6, 0, 1, 1);
	baseobject_grid->addWidget(tablespace_sel, 6, 1, 1, 4);
	baseobject_grid->addWidget(owner_lbl, 7, 0, 1, 1);
	baseobject_grid->addWidget(owner_sel, 7, 1, 1, 4);
	baseobject_grid->addWidget(comment_lbl, 8, 0, 1, 1);
	baseobject_grid->addWidget(comment_edt, 8, 1, 1, 4);

	misc_btns_lt=new QHBoxLayout;
	spacer=new QSpacerItem(20,1,QSizePolicy::Expanding);

	misc_btns_lt->addItem(spacer);
	misc_btns_lt->addWidget(append_sql_tb);
	misc_btns_lt->addWidget(edt_perms_tb);
	misc_btns_lt->addWidget(disable_sql_chk);

	baseobject_grid->addLayout(misc_btns_lt,9,0,1,5);
}

bool BaseObjectWidget::eventFilter(QObject *object, QEvent *event)
{
	//Filters the ENTER/RETURN pressing forcing the parent form activate the "Apply" button
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent *kevent=dynamic_cast<QKeyEvent *>(event);

		// If the object is protected we avoid accepting the enter hit
		if(!protected_obj_frm->isVisible() &&
			 (kevent->key()==Qt::Key_Return || kevent->key()==Qt::Key_Enter))
		{
			applyConfiguration();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

ObjectType BaseObjectWidget::getHandledObjectType()
{
	return handled_obj_type;
}

bool BaseObjectWidget::isHandledObjectProtected()
{
	return object_protected;
}

void BaseObjectWidget::showEvent(QShowEvent *)
{
	name_edt->setFocus();
}

void BaseObjectWidget::setRequiredField(QWidget *widget)
{
	if(widget)
	{
		QLabel *lbl=qobject_cast<QLabel *>(widget);
		QLineEdit *edt=qobject_cast<QLineEdit *>(widget);
		QTextEdit *txt=qobject_cast<QTextEdit *>(widget);
		QGroupBox *grp=qobject_cast<QGroupBox *>(widget);
		ObjectSelectorWidget *sel=dynamic_cast<ObjectSelectorWidget *>(widget);
		PgSQLTypeWidget *pgtype=dynamic_cast<PgSQLTypeWidget *>(widget);
		QString str_aux = " <span style='color: #ff0000;'>*</span> ";
		QColor border_color = CustomTableWidget::getTableItemColor(CustomTableWidget::RemovedItemBgColor);

		if(lbl || pgtype || grp)
		{
			if(lbl)
				lbl->setText(str_aux + lbl->text());

			if(pgtype || grp)
				widget->setStyleSheet("QGroupBox {	font-weight: bold; }");
			else if(lbl)
				widget->setStyleSheet("QWidget {	font-weight: bold; }");
		}
		else if(edt || txt || sel)
		{
			if(sel)
			{
				widget = sel->obj_name_edt;
				widget->setStyleSheet(QString("ObjectSelectorWidget > QLineEdit { border: 2px solid %2; padding-top: 2px; padding-bottom: 2px; border-radius: 4px; }").arg(border_color.name()));
			}
			else
				widget->setStyleSheet(QString("%1 { border: 2px solid %2; padding-top: 2px; padding-bottom: 2px; border-radius: 4px; }").arg(widget->metaObject()->className()).arg(border_color.name()));
		}

		str_aux=(!widget->toolTip().isEmpty() ? "\n" : "");
		widget->setToolTip(widget->toolTip() + str_aux + tr("Required field. Leaving this empty will raise errors!"));
	}
}

void BaseObjectWidget::setAttributes(DatabaseModel *model, BaseObject *object, BaseObject *parent_obj)
{
	setAttributes(model, nullptr, object, parent_obj, DNaN, DNaN, false);
}

void BaseObjectWidget::disableReferencesSQL(BaseObject *object)
{
	TableObject *tab_obj = nullptr;

	for(auto &obj : object->getReferences())
	{
		tab_obj = dynamic_cast<TableObject *>(obj);

		//If the object is a relationship added does not do anything since the relationship itself will be disabled
		if(!tab_obj || (tab_obj && !tab_obj->isAddedByRelationship()))
		{
			obj->setSQLDisabled(disable_sql_chk->isChecked());

			//Update the parent table graphical representation to show the disabled child object
			if(tab_obj)
				tab_obj->getParentTable()->setModified(true);

			//Disable the references of the current object too
			disableReferencesSQL(obj);
		}
	}
}

void BaseObjectWidget::configureTabOrder(std::vector<QWidget *> widgets)
{
	ObjectSelectorWidget *obj_sel=nullptr;
	PgSQLTypeWidget *type_wgt=nullptr;
	std::vector<QWidget *> children, tab_order;
	int idx=0, cnt=0;

	widgets.insert(widgets.begin(),
	{ name_edt, alias_edt, schema_sel , collation_sel, owner_sel, tablespace_sel,
	  comment_edt, append_sql_tb, edt_perms_tb, disable_sql_chk });

	for(auto &wgt : widgets)
	{
		wgt->setFocusPolicy(Qt::StrongFocus);

		obj_sel=dynamic_cast<ObjectSelectorWidget *>(wgt);
		type_wgt=dynamic_cast<PgSQLTypeWidget *>(wgt);

		if(obj_sel)
			children={ obj_sel->rem_object_tb, obj_sel->sel_object_tb };
		else if(type_wgt)
		{
			children={ type_wgt->type_cmb, type_wgt->length_sb, type_wgt->precision_sb,
					   type_wgt->dimension_sb, type_wgt->interval_cmb, type_wgt->spatial_cmb,
					   type_wgt->srid_spb, type_wgt->var_z_chk, type_wgt->var_m_chk,
					   type_wgt->timezone_chk };
		}

		tab_order.push_back(wgt);

		for(auto &child : children)
		{
			child->setFocusPolicy(Qt::StrongFocus);
			tab_order.push_back(child);
		}
	}

	cnt=tab_order.size()-1;

	for(idx=0; idx < cnt; idx++)
		QWidget::setTabOrder(tab_order[idx], tab_order[idx+1]);
}

BaseObject *BaseObjectWidget::getHandledObject()
{
	return object;
}

void BaseObjectWidget::cancelChainedOperation()
{
	bool op_list_changed=false;

	if(op_list->isOperationChainStarted())
		op_list->finishOperationChain();

	if(operation_count < op_list->getCurrentSize())
	{
		op_list_changed=true;
		BaseObjectWidget::cancelConfiguration();
	}

	if(new_object && this->object)
	{
		if(!op_list_changed)
			delete this->object;

		this->object=nullptr;
	}
}

void BaseObjectWidget::setAttributes(DatabaseModel *model, OperationList *op_list, BaseObject *object, BaseObject *parent_obj, double obj_px, double obj_py, bool uses_op_list)
{
	ObjectType obj_type, parent_type=ObjectType::BaseObject;

	/* Reseting the objects attributes in order to force them to be redefined
	every time this method is called */
	this->object=nullptr;
	this->model=nullptr;
	this->op_list=nullptr;
	this->relationship=nullptr;
	this->table=nullptr;

	if(!model || (uses_op_list && !op_list))
		throw Exception(ErrorCode::AsgNotAllocattedObject,PGM_FUNC,PGM_FILE,PGM_LINE);

	if(op_list)
	  operation_count = op_list->getCurrentSize();

	this->model=model;

	if(parent_obj)
	{
		parent_type=parent_obj->getObjectType();

		if(BaseTable::isBaseTable(parent_type))
			this->table=dynamic_cast<BaseTable *>(parent_obj);
		else if(parent_type==ObjectType::Relationship)
			this->relationship=dynamic_cast<Relationship *>(parent_obj);
		else if(parent_type!=ObjectType::Database && parent_type!=ObjectType::Schema)
			throw Exception(ErrorCode::AsgObjectInvalidType,PGM_FUNC,PGM_FILE,PGM_LINE);
	}
	else
	{
		TableObject *tab_obj=dynamic_cast<TableObject *>(object);

		if(object && object->getSchema())
			parent_obj=object->getSchema();
		else if(tab_obj && tab_obj->getParentTable())
			parent_obj=tab_obj->getParentTable();
		else
			parent_obj=model;
	}

	if(dynamic_cast<BaseGraphicObject *>(object))
		dynamic_cast<BaseGraphicObject *>(object)->setModified(false);

	this->op_list=op_list;
	this->object=object;

	if(this->table)
	{
		this->object_px=this->table->getPosition().x();
		this->object_py=this->table->getPosition().y();
	}
	else
	{
		this->object_px=obj_px;
		this->object_py=obj_py;
	}

	name_edt->setFocus();
	edt_perms_tb->setEnabled(object!=nullptr && !new_object);
	append_sql_tb->setEnabled(object!=nullptr && !new_object);

	owner_sel->setModel(model);
	owner_sel->setSelectedObject(model->getDefaultObject(ObjectType::Role));

	schema_sel->setModel(model);
	schema_sel->setSelectedObject(model->getDefaultObject(ObjectType::Schema));

	tablespace_sel->setModel(model);
	tablespace_sel->setSelectedObject(model->getDefaultObject(ObjectType::Tablespace));

	collation_sel->setModel(model);
	collation_sel->setSelectedObject(model->getDefaultObject(ObjectType::Collation));

	if(object)
	{
		obj_id_lbl->setVisible(true);
		obj_id_lbl->setText(QString("ID: %1").arg(object->getObjectId()));

		if(handled_obj_type != ObjectType::BaseObject)
			name_edt->setText(object->getName());
		else
			name_edt->setText(object->getSignature());

		comment_edt->setPlainText(object->getComment());
		alias_edt->setText(object->getAlias());

		/* When creating a new table or relationship the object is pre allocated and the flag new_object is set.
		 * In order to avoid the selectors to have empty values, we check if the flag is false which means
		 * that the object is not new at all */
		if(!new_object)
		{
			schema_sel->setSelectedObject(object->getSchema());
			tablespace_sel->setSelectedObject(object->getTablespace());
			owner_sel->setSelectedObject(object->getOwner());
			collation_sel->setSelectedObject(object->getCollation());
		}
		/* Some objects like tables and views came with a pre selected schema even when
	   they are new objects. So we need to show this selected schema instead of the default one */
		else if(new_object && object->getSchema())
			schema_sel->setSelectedObject(object->getSchema());

		obj_type=object->getObjectType();
		object_protected=(parent_type!=ObjectType::Relationship &&
																	 (object->isProtected() ||
																		(TableObject::isTableObject(obj_type) &&
																		 dynamic_cast<TableObject *>(object)->isAddedByRelationship())));
		protected_obj_frm->setVisible(object_protected);
		disable_sql_chk->setChecked(object->isSQLDisabled());
	}
	else
	{
		object_protected = false;
		obj_id_lbl->setVisible(false);
		protected_obj_frm->setVisible(false);

		if(parent_obj && parent_obj->getObjectType()==ObjectType::Schema)
			schema_sel->setSelectedObject(parent_obj);
	}
}

void BaseObjectWidget::configureFormLayout(QGridLayout *grid, ObjectType obj_type)
{
	if(!grid)
		setLayout(baseobject_grid);
	else
	{
		QLayoutItem *item = nullptr;
		int lin = 0, col = 0, col_span = 0,
				row_span = 0, item_id = 0, item_count = 0;

		/* Move all the widgets of the passed grid layout one row down,
		 permiting the insertion of the 'baseobject_grid' at the top
		 of the items */
		item_count = grid->count();
		for(item_id = item_count-1; item_id >= 0; item_id--)
		{
			item = grid->itemAt(item_id);
			grid->getItemPosition(item_id, &lin, &col, &row_span, &col_span);
			grid->removeItem(item);
			grid->addItem(item, lin+1, col, row_span, col_span);

			/* Configuring QTextEdit to accept tabs as focus changes. This code
			only applies to widgets directly linked to the layout. If there is some
			QTextEdit nested in some child widget this is not applied */
			if(dynamic_cast<QTextEdit *>(item->widget()))
				dynamic_cast<QTextEdit *>(item->widget())->setTabChangesFocus(true);
		}

		//Adding the base layout on the top
		grid->addLayout(baseobject_grid, 0,0,1,0);
		baseobject_grid = grid;
	}

	baseobject_grid->setContentsMargins(GuiUtilsNs::LtMargins);
	configureFormFields(obj_type, obj_type != ObjectType::BaseObject);
}

void BaseObjectWidget::configureFormFields(ObjectType obj_type, bool inst_ev_filter)
{
	QObjectList chld_list;
	QWidget *wgt = nullptr;

	disable_sql_chk->setVisible(obj_type!=ObjectType::BaseObject && obj_type!=ObjectType::Permission &&
															obj_type!=ObjectType::Textbox && obj_type!=ObjectType::Tag &&
															obj_type!=ObjectType::Parameter);

	alias_edt->setVisible(BaseObject::acceptsAlias(obj_type));
	logical_name_lbl->setVisible(BaseObject::acceptsAlias(obj_type));

	edt_perms_tb->setVisible(Permission::acceptsPermission(obj_type));
	append_sql_tb->setVisible(BaseObject::acceptsCustomSQL(obj_type));

	schema_lbl->setVisible(BaseObject::acceptsSchema(obj_type));
	schema_sel->setVisible(BaseObject::acceptsSchema(obj_type));

	owner_lbl->setVisible(BaseObject::acceptsOwner(obj_type));
	owner_sel->setVisible(BaseObject::acceptsOwner(obj_type));

	tablespace_lbl->setVisible(BaseObject::acceptsTablespace(obj_type));
	tablespace_sel->setVisible(BaseObject::acceptsTablespace(obj_type));

	collation_lbl->setVisible(BaseObject::acceptsCollation(obj_type));
	collation_sel->setVisible(BaseObject::acceptsCollation(obj_type));

	comment_lbl->setVisible(BaseObject::acceptsComment(obj_type));
	comment_edt->setVisible(BaseObject::acceptsComment(obj_type));

	if(obj_type!=ObjectType::BaseObject)
	{
		obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath(obj_type)));
		obj_icon_lbl->setToolTip(BaseObject::getTypeName(obj_type));

		if(obj_type != ObjectType::Permission && obj_type != ObjectType::Cast &&
				obj_type != ObjectType::UserMapping && obj_type != ObjectType::Transform)
		{
			setRequiredField(name_lbl);
			setRequiredField(name_edt);
		}
		else
		{
			QFont font=name_edt->font();
			name_edt->setReadOnly(true);
			font.setItalic(true);
			name_edt->setFont(font);
		}

		if(obj_type!=ObjectType::Extension)
		{
			setRequiredField(schema_lbl);
			setRequiredField(schema_sel);
		}
	}

	if(inst_ev_filter)
	{
		//Install the event filter into all children object in order to capture key press
		chld_list = this->children();

		while(!chld_list.isEmpty())
		{
			wgt=dynamic_cast<QWidget *>(chld_list.front());

			//Avoids install event filters in objects that are inteneded to edit multiple lines
			if(wgt &&
					wgt->metaObject()->className()!=QString("QPlainTextEdit") &&
					wgt->metaObject()->className()!=QString("NumberedTextEditor"))
				wgt->installEventFilter(this);

			chld_list.pop_front();
		}
	}
}

QString BaseObjectWidget::generateVersionsInterval(unsigned ver_interv_id, const QString &ini_ver, const QString &end_ver)
{
	if(ver_interv_id==UntilVersion && !ini_ver.isEmpty())
		return (UtilsNs::EntityLt + "= " + ini_ver);

	if(ver_interv_id==VersionsInterval && !ini_ver.isEmpty() && !end_ver.isEmpty())
		return (UtilsNs::EntityGt + "= " + ini_ver + UtilsNs::EntityAmp + UtilsNs::EntityLt + "= " + end_ver);

	if(ver_interv_id==AfterVersion &&  !ini_ver.isEmpty())
		return (UtilsNs::EntityGt + "= " + ini_ver);

	return "";
}

QFrame *BaseObjectWidget::generateInformationFrame(const QString &msg)
{
	QFrame *info_frm=nullptr;
	QGridLayout *grid=nullptr;
	QLabel *ico_lbl=nullptr, *msg_lbl=nullptr;
	QFont font;

	info_frm = new QFrame;

	font.setItalic(false);
	font.setBold(false);
	info_frm->setFont(font);

	info_frm->setObjectName("info_frm");
	info_frm->setFrameShape(QFrame::StyledPanel);
	info_frm->setFrameShadow(QFrame::Raised);
	info_frm->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	CustomUiStyle::setStyleHint(CustomUiStyle::InfoFrmHint, info_frm);

	grid = new QGridLayout(info_frm);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);
	grid->setObjectName("grid");

	ico_lbl = new QLabel(info_frm);
	ico_lbl->setObjectName("icon_lbl");
	ico_lbl->setMinimumSize(QSize(25, 25));
	ico_lbl->setMaximumSize(QSize(25, 25));
	ico_lbl->setScaledContents(true);
	ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("info")));
	ico_lbl->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	grid->addWidget(ico_lbl, 0, 0, 1, 1);

	msg_lbl = new QLabel(info_frm);
	msg_lbl->setFont(font);
	msg_lbl->setObjectName("message_lbl");
	msg_lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	msg_lbl->setWordWrap(true);

	msg_lbl->setText(msg);

	grid->addWidget(msg_lbl, 0, 1, 1, 1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);

	return info_frm;
}

void BaseObjectWidget::highlightVersionSpecificFields(std::map<QString, std::vector<QWidget *> > &fields,
																											std::map< QWidget *, std::vector<QString> > *values)
{
	QString field_name;

	for(auto itr : fields)
	{
		for(auto wgt : itr.second)
		{
			if(values && values->count(wgt) > 0)
			{
				field_name+="<br/>" + tr("Value(s)") + ": (";
				for(auto value : values->at(wgt))
				{
					field_name += value;
					field_name+=", ";
				}

				field_name.remove(field_name.length() - 2, 2);
				field_name+=")";
			}

			wgt->setStyleSheet("QWidget {	font-weight: bold; font-style: italic; text-decoration: underline; }");
			wgt->setToolTip("<p>PostgreSQL" + itr.first + QString(" %1</p>").arg(field_name));
		}
	}
}

QFrame *BaseObjectWidget::generateVersionWarningFrame(std::map<QString, std::vector<QWidget *> > &fields,
																											std::map< QWidget *, std::vector<QString> > *values)
{
	QFrame *alert_frm=nullptr;
	QGridLayout *grid=nullptr;
	QLabel *ico_lbl=nullptr, *msg_lbl=nullptr;
	QFont font;

	highlightVersionSpecificFields(fields, values);

	alert_frm = new QFrame;
	font.setItalic(false);
	font.setBold(false);

	alert_frm->setObjectName("alert_frm");
	alert_frm->setFrameShape(QFrame::StyledPanel);
	alert_frm->setFrameShadow(QFrame::Raised);
	alert_frm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	CustomUiStyle::setStyleHint(CustomUiStyle::AlertFrmHint, alert_frm);

	grid = new QGridLayout(alert_frm);
	grid->setObjectName("grid");

	ico_lbl = new QLabel(alert_frm);
	ico_lbl->setObjectName("icon_lbl");
	ico_lbl->setMinimumSize(QSize(25, 25));
	ico_lbl->setMaximumSize(QSize(25, 25));
	ico_lbl->setScaledContents(true);
	ico_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("alert")));
	ico_lbl->setAlignment(Qt::AlignLeft|Qt::AlignTop);

	grid->addWidget(ico_lbl, 0, 0, 1, 1);

	msg_lbl = new QLabel(alert_frm);
	msg_lbl->setFont(font);
	msg_lbl->setObjectName("mensagelm_lb");
	msg_lbl->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	msg_lbl->setWordWrap(true);

	msg_lbl->setText(tr("The <em><u><strong>highlighted</strong></u></em> fields in the form or one of their values are available only on specific PostgreSQL versions. \
							Generating SQL code for versions other than those specified in the fields' tooltips may create incompatible code."));

	grid->addWidget(msg_lbl, 0, 1, 1, 1);
	grid->setContentsMargins(GuiUtilsNs::LtMargins);

	alert_frm->adjustSize();
	return alert_frm;
}

void BaseObjectWidget::editPermissions()
{
	BaseObject *parent_obj=nullptr;
	BaseForm parent_form(this);
	PermissionWidget *permission_wgt=new PermissionWidget;

	if(this->relationship)
		parent_obj=this->relationship;

	permission_wgt->setAttributes(this->model, parent_obj, this->object);
	parent_form.setMainWidget(permission_wgt);
	parent_form.setButtonConfiguration(Messagebox::CloseButton);

	GeneralConfigWidget::restoreWidgetGeometry(&parent_form, permission_wgt->metaObject()->className());
	parent_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&parent_form, permission_wgt->metaObject()->className());
}

void BaseObjectWidget::editCustomSQL()
{
	BaseForm parent_form(this);
	CustomSQLWidget *customsql_wgt=new CustomSQLWidget;

	customsql_wgt->setAttributes(this->model, this->object);
	parent_form.setMainWidget(customsql_wgt);

	GeneralConfigWidget::restoreWidgetGeometry(&parent_form, customsql_wgt->metaObject()->className());
	parent_form.exec();
	GeneralConfigWidget::saveWidgetGeometry(&parent_form, customsql_wgt->metaObject()->className());
}

void BaseObjectWidget::applyConfiguration()
{
	if(object)
	{
		try
		{
			BaseObject *aux_obj=nullptr, *aux_obj1=nullptr, *parent_obj=nullptr;
			bool new_obj;
			ObjectType obj_type=object->getObjectType();
			QString obj_name;

			qApp->setOverrideCursor(Qt::WaitCursor);
			obj_name=BaseObject::formatName(name_edt->text().toUtf8(), obj_type==ObjectType::Operator);

			if(this->object->acceptsSchema() &&  schema_sel->getSelectedObject())
				obj_name=schema_sel->getSelectedObject()->getName(true) + "." + obj_name;

			//Checking the object duplicity
			if(obj_type!=ObjectType::Database && obj_type!=ObjectType::Permission && obj_type!=ObjectType::Parameter)
			{
				if(table)
				{
					//Validationg the object against the siblings on parent table
					parent_obj=table;
					aux_obj=table->getObject(obj_name,obj_type);
					aux_obj1=table->getObject(object->getName(),obj_type);
					new_obj=(!aux_obj && !aux_obj1);
				}
				else if(relationship)
				{
					//Validationg the object against the siblings on parent relationship
					parent_obj=relationship;
					aux_obj=relationship->getObject(obj_name,obj_type);
					aux_obj1=relationship->getObject(object->getName(),obj_type);
					new_obj=(!aux_obj && !aux_obj1);
				}
				//Validationg the object against the other objects on model
				else
				{
					parent_obj=model;
					aux_obj=model->getObject(obj_name,obj_type);

					/* Special case for tables and views. Its necessary to make an additional
					checking on table list when the configured object is a view or a checking
					on view list when the configured object is a table, this because PostgreSQL
					does not accepts tables and views have the same name on the same schema */
					if(BaseTable::isBaseTable(obj_type))
						aux_obj = model->getObject(obj_name, { ObjectType::Table, ObjectType::ForeignTable, ObjectType::View });

					new_obj = (aux_obj == nullptr);
				}

				//Raises an error if another object is found with the same name as the editing object
				if(!new_obj && aux_obj && aux_obj!=object)
				{
					throw Exception(Exception::getErrorMessage(ErrorCode::AsgDuplicatedObject)
									.arg(obj_name)
									.arg(BaseObject::getTypeName(obj_type))
									.arg(parent_obj->getName(true))
									.arg(parent_obj->getTypeName()),
									ErrorCode::AsgDuplicatedObject,PGM_FUNC,PGM_FILE,PGM_LINE);
				}
			}

			//Renames the object (only cast object aren't renamed)
			if(obj_type!=ObjectType::Cast)
			{
				prev_name=object->getName();
				object->setName(name_edt->text().trimmed().toUtf8());
			}

			if(object->acceptsAlias())
				object->setAlias(alias_edt->text().trimmed());

			//Sets the object's comment
			if(object->acceptsComment())
				object->setComment(comment_edt->toPlainText().toUtf8());

			//Sets the object's tablespace
			if(object->acceptsTablespace())
				object->setTablespace(tablespace_sel->getSelectedObject());

			//Sets the object's owner
			if(object->acceptsOwner())
				object->setOwner(owner_sel->getSelectedObject());

			//Sets the object's collation
			if(object->acceptsCollation())
				object->setCollation(collation_sel->getSelectedObject());

			//Sets the object's schema			
			if(object->acceptsSchema())
			{
				Schema *esquema=dynamic_cast<Schema *>(schema_sel->getSelectedObject());
				this->prev_schema=dynamic_cast<Schema *>(object->getSchema());
				object->setSchema(esquema);
			}

			if(!object->isProtected() && !object->isSystemObject())
				GuiUtilsNs::disableObjectSQL(object, disable_sql_chk->isChecked());
		}
		catch(Exception &e)
		{
			//qApp->restoreOverrideCursor();
			throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
		}
	}
}

void BaseObjectWidget::finishConfiguration()
{
	try
	{
		if(this->object)
		{
			ObjectType obj_type=this->object->getObjectType();
			BaseGraphicObject *graph_obj=dynamic_cast<BaseGraphicObject *>(this->object);
			TableObject *tab_obj=dynamic_cast<TableObject *>(this->object);
			std::vector<BaseObject *> ref_objs;

			if(graph_obj && !std::isnan(object_px) && !std::isnan(object_py))
				graph_obj->setPosition(QPointF(object_px, object_py));

			if(new_object)
			{
				//If the object is a table object and the parent table is specified, adds it to table
				if(table && TableObject::isTableObject(obj_type))
					table->addObject(this->object);
				//Adding the object on the relationship, if specified
				else if(relationship && (obj_type==ObjectType::Column || obj_type==ObjectType::Constraint))
					relationship->addObject(dynamic_cast<TableObject *>(this->object));
				//Adding the object on the model
				else if(obj_type!=ObjectType::Parameter)
					model->addObject(this->object);

				registerNewObject();
				new_object=false;
			}
			else
			{
				//If the object is being updated, validates its SQL definition
				if(obj_type==ObjectType::BaseRelationship || obj_type==ObjectType::Textbox || obj_type==ObjectType::Tag)
					this->object->getSourceCode(SchemaParser::XmlCode);
				else
					this->object->getSourceCode(SchemaParser::SqlCode);
			}

			for(auto &obj : object->getReferences())
			{
				obj->setCodeInvalidated(true);

				if(obj->getObjectType() == ObjectType::Column)
					dynamic_cast<Column *>(obj)->getParentTable()->setModified(true);
			}

			object->setCodeInvalidated(true);

			//If the object is graphical (or a table object), updates it (or its parent) on the scene
			if(graph_obj || tab_obj)
			{
				if(!graph_obj && tab_obj && tab_obj->getObjectType()!=ObjectType::Parameter)
				{
					if(this->table)
						graph_obj=dynamic_cast<BaseGraphicObject *>(this->table);
					else
						graph_obj=dynamic_cast<BaseGraphicObject *>(this->relationship);

					graph_obj->setModified(true);
					graph_obj->setCodeInvalidated(true);
				}
				// If the new object is a graphica one, we assign the postion prior to its addtion to the parent
				else if(graph_obj)
					graph_obj->setModified(true);

				/* Updates the visual schemas when the objects is moved to another or a
					table object is added to a table */
				if(object->getSchema())
					dynamic_cast<Schema *>(object->getSchema())->setModified(true);
				else if(tab_obj && tab_obj->getParentTable() &&
						tab_obj->getParentTable()->getSchema())
					dynamic_cast<Schema *>(tab_obj->getParentTable() ->getSchema())->setModified(true);

				if(prev_schema && object->getSchema()!=prev_schema)
					prev_schema->setModified(true);
			}

			object->clearDependencies();
			object->updateDependencies();

			emit s_objectManipulated();
			emit s_closeRequested();
		}

		qApp->restoreOverrideCursor();
	}
	catch(Exception &e)
	{
		if(e.getErrorCode()==ErrorCode::AsgObjectInvalidDefinition)
		{
			throw Exception(Exception::getErrorMessage(ErrorCode::RequiredFieldsNotFilled)
											.arg(this->object->getName()).arg(this->object->getTypeName()),
											ErrorCode::RequiredFieldsNotFilled,PGM_FUNC,PGM_FILE,PGM_LINE,&e);
		}

		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}

void BaseObjectWidget::cancelConfiguration()
{
	if(!object)
		return;

	ObjectType obj_type;
	obj_type=this->object->getObjectType();

	if(new_object)
	{
		TableObject *tab_obj=dynamic_cast<TableObject *>(this->object);

		//Removes the object from its parent
		if(!table && !tab_obj && model->getObjectIndex(this->object) >= 0)
			model->removeObject(this->object);
		else if(table && table->getObjectIndex(tab_obj) >= 0)
			table->removeObject(tab_obj);
		else if(relationship && relationship->getObjectIndex(tab_obj) >= 0)
			relationship->removeObject(tab_obj);

		if(!BaseTable::isBaseTable(obj_type) && obj_type != ObjectType::Relationship)
		{
			if(!op_list->isObjectRegistered(this->object, Operation::ObjCreated))
				delete this->object;

			this->object=nullptr;
		}
	}

	//If the object is not a new one, restore its previous state
	if(op_list &&
	  ((!new_object && obj_type!=ObjectType::Database && obj_type!=ObjectType::Permission && operation_count != op_list->getCurrentSize()) ||
		 (new_object && (BaseTable::isBaseTable(obj_type) || obj_type==ObjectType::Relationship))))
	{
		try
		{
			op_list->undoOperation();
			op_list->removeLastOperation();
		}
		catch(Exception &){}
	}

	qApp->restoreOverrideCursor();
	emit s_objectManipulated();
}

void BaseObjectWidget::registerNewObject()
{
	try
	{
		if(this->new_object && op_list && !op_list->isObjectRegistered(this->object, Operation::ObjCreated))
		{
			//If the object is a new one is necessary register it on the operation list
			if(this->table)
				op_list->registerObject(this->object, Operation::ObjCreated, -1, this->table);
			else if(this->relationship)
				op_list->registerObject(this->object, Operation::ObjCreated, -1, this->relationship);
			else
				op_list->registerObject(this->object, Operation::ObjCreated);
		}
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE,&e);
	}
}
