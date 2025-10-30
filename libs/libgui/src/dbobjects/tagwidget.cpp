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

#include "tagwidget.h"

TagWidget::TagWidget(QWidget *parent): BaseObjectWidget(parent, ObjectType::Tag)
{
	Ui_TagWidget::setupUi(this);
	configureFormLayout(tag_grid, ObjectType::Tag);

	unsigned color_count=1;
	int row=0;

	for(auto &attr : Tag::getColorAttributes())
	{
		if(color_count==1 && attr!=Attributes::TableName && attr!=Attributes::TableSchemaName)
			color_count=3;

		color_pickers[attr]=new ColorPickerWidget(color_count, this);
		colors_grid->addWidget(color_pickers[attr], row, 1);
		colors_grid->addItem(new QSpacerItem(10,10, QSizePolicy::Expanding, QSizePolicy::Fixed), row, 2);
		row++;
	}

	setMinimumSize(450, 280);
}

void TagWidget::setAttributes(DatabaseModel *model, OperationList *op_list, Tag *tag)
{
	unsigned color_count=1, i;

	BaseObjectWidget::setAttributes(model, op_list, tag);

	for(auto &attr : Tag::getColorAttributes())
	{
		if(color_count==1 && attr!=Attributes::TableName && attr!=Attributes::TableSchemaName)
			color_count=3;

		for(i=0; i < color_count; i++)
		{
			if(tag)
				color_pickers[attr]->setColor(i, tag->getElementColor(attr, static_cast<ColorId>(i)));
			else
				color_pickers[attr]->setColor(i, BaseObjectView::getElementColor(attr, static_cast<ColorId>(i)));
		}
	}
}

void TagWidget::applyConfiguration()
{
	try
	{
		Tag *tag=nullptr;
		std::vector<BaseObject *> tagged_tabs;

		startConfiguration<Tag>();
		tag=dynamic_cast<Tag *>(this->object);

		BaseObjectWidget::applyConfiguration();

		tag->setElementColor(Attributes::TableName, color_pickers[Attributes::TableName]->getColor(0), ColorId::FillColor1);
		tag->setElementColor(Attributes::TableSchemaName, color_pickers[Attributes::TableSchemaName]->getColor(0), ColorId::FillColor1);

		for(auto &attr : Tag::getColorAttributes())
		{
			if(attr == Attributes::TableName || attr == Attributes::TableSchemaName)
				continue;

			tag->setElementColors(attr,
								  QString("%1,%2,%3")
									.arg(color_pickers[attr]->getColor(enum_t(ColorId::FillColor1)).name())
									.arg(color_pickers[attr]->getColor(enum_t(ColorId::FillColor2)).name())
									.arg(color_pickers[attr]->getColor(enum_t(ColorId::BorderColor)).name()));
		}

		for(auto &obj : tag->getReferences())
			dynamic_cast<BaseTable *>(obj)->setModified(true);

		finishConfiguration();
	}
	catch(Exception &e)
	{
		cancelConfiguration();
		throw Exception(e.getErrorMessage(),e.getErrorCode(),PGM_FUNC,PGM_FILE,PGM_LINE, &e);
	}
}
