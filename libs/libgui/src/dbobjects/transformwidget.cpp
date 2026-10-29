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

#include "transformwidget.h"

TransformWidget::TransformWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Transform)
{
	Ui_TransformWidget::setupUi(this);

	type_wgt = new PgSQLTypeWidget(this);
	attributes_lt->insertWidget(0, type_wgt);

	language_sel = new ObjectSelectorWidget(ObjectType::Language, this);
	language_lt->addWidget(language_sel);

	from_sql_func_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	from_sql_func_lt->addWidget(from_sql_func_sel);

	to_sql_func_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	to_sql_func_lt->addWidget(to_sql_func_sel);

	attributes_lt->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));

	setRequiredField(type_wgt);
	setRequiredField(language_sel);
	setRequiredField(language_lbl);
	configureTabbedLayout(attributes_tbw);

	setMinimumSize(550, 400);
}

void TransformWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Transform *transform)
{
	PgSqlType type;

	BaseObjectWidget::setAttributes(model, op_list, transform);

	if(transform)
	{
		type = transform->getType();
		language_sel->setSelectedObject(transform->getLanguage());
		from_sql_func_sel->setSelectedObject(transform->getFunction(Transform::FromSqlFunc));
		to_sql_func_sel->setSelectedObject(transform->getFunction(Transform::ToSqlFunc));
	}

	language_sel->setModel(model);
	from_sql_func_sel->setModel(model);
	to_sql_func_sel->setModel(model);
	type_wgt->setAttributes(type, model, false, UserTypeConfig::AllUserTypes, false, false);
}

void TransformWidget::applyConfiguration()
{
	try
	{
		Transform *transform = nullptr;
		startConfiguration<Transform>();

		transform = dynamic_cast<Transform *>(this->object);
		transform->setType(type_wgt->getPgSQLType());
		transform->setLanguage(dynamic_cast<Language *>(language_sel->getSelectedObject()));
		transform->setFunction(dynamic_cast<Function *>(from_sql_func_sel->getSelectedObject()), Transform::FromSqlFunc);
		transform->setFunction(dynamic_cast<Function *>(to_sql_func_sel->getSelectedObject()), Transform::ToSqlFunc);

		BaseObjectWidget::applyConfiguration();
		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
