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

#include "transformwidget.h"

TransformWidget::TransformWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Transform)
{
	Ui_TransformWidget::setupUi(this);
	QGridLayout *transform_grid = new QGridLayout(this);

	type_wgt = new PgSQLTypeWidget(this);
	transform_grid->addWidget(type_wgt, 0, 0, 1, 2);

	language_sel = new ObjectSelectorWidget(ObjectType::Language, this);
	transform_grid->addWidget(language_lbl, 1, 0);
	transform_grid->addWidget(language_sel, 1, 1);

	from_sql_func_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	transform_grid->addWidget(from_sql_func_lbl, 2, 0);
	transform_grid->addWidget(from_sql_func_sel, 2, 1);

	to_sql_func_sel = new ObjectSelectorWidget(ObjectType::Function, this);
	transform_grid->addWidget(to_sql_func_lbl, 3, 0);
	transform_grid->addWidget(to_sql_func_sel, 3, 1);

	transform_grid->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding), 4, 0);

	configureFormLayout(transform_grid, ObjectType::Transform);
	setRequiredField(type_wgt);
	setRequiredField(language_sel);
	setRequiredField(language_lbl);
	setMinimumSize(640, 480);
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
