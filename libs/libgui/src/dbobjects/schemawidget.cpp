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

#include "schemawidget.h"

SchemaWidget::SchemaWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Schema)
{
	Ui_SchemaWidget::setupUi(this);
	QHBoxLayout *hbox = dynamic_cast<QHBoxLayout *>(attributes_gb->layout());

	configureFormLayout(schema_grid, ObjectType::Schema);

	fill_color_picker = new ColorPickerWidget(1, this);
	hbox->insertWidget(1, fill_color_picker);

	name_color_picker = new ColorPickerWidget(1, this);
	hbox->insertWidget(3, name_color_picker);

	configureTabOrder({ fill_color_picker, name_color_picker, show_rect_chk });

	setMinimumSize(480, 140);
}

void SchemaWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Schema *schema)
{
	bool enable=false;

	BaseObjectWidget::setAttributes(model, op_list, schema);

	enable=!(schema && schema->isSystemObject());
	edt_perms_tb->setEnabled(enable);
	name_edt->setEnabled(enable);
	comment_edt->setEnabled(enable);
	owner_sel->setEnabled(enable);
	disable_sql_chk->setEnabled(enable);
	append_sql_tb->setEnabled(enable);

	if(schema)
	{
		if(schema->isSystemObject())
			protected_obj_frm->setVisible(false);

		fill_color_picker->setColor(0, schema->getFillColor());
		name_color_picker->setColor(0, schema->getNameColor());
		show_rect_chk->setChecked(schema && schema->isRectVisible());
	}
	else
		fill_color_picker->setColor(0, QColor(225,225,225));
}

void SchemaWidget::applyConfiguration()
{
	try
	{
		Schema *schema=nullptr;

		startConfiguration<Schema>();
		schema=dynamic_cast<Schema *>(this->object);
		BaseObjectWidget::applyConfiguration();

		schema->setRectVisible(show_rect_chk->isChecked());
		schema->setFillColor(fill_color_picker->getColor(0));
		schema->setNameColor(name_color_picker->getColor(0));
		model->validateSchemaRenaming(dynamic_cast<Schema *>(this->object), this->prev_name);

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}

