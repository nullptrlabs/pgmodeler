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

#include "sceneinfowidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "relationshipview.h"

SceneInfoWidget::SceneInfoWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);
	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, cursor_pos_info_frm);
	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, separator_ln);
 	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, obj_sel_info_frm);
	CustomUiStyle::setStyleHint(CustomUiStyle::DefaultFrmHint, separator_ln2);
}

void SceneInfoWidget::updateSelectedObject(BaseObjectView *object)
{
	if(!object)
	{
		sel_obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("close1")));
		sel_obj_count_lbl->setText(tr("None"));
		obj_pos_info_lbl->setText(tr("None"));
	}
	else
	{
		RelationshipView *rel_view = dynamic_cast<RelationshipView *>(object);
		QRectF rect = rel_view ? rel_view->boundingRect() :
											QRectF(object->pos().x(),
														 object->pos().y(),
														 object->boundingRect().width(),
														 object->boundingRect().height());

		sel_obj_icon_lbl->setPixmap(GuiUtilsNs::getIconPath(object->getUnderlyingObject()->getSchemaName()));
		sel_obj_count_lbl->setText(tr("Selected: <strong>%1</strong>").arg(1));
		obj_pos_info_lbl->setText(QString("(%1, %2) [w: %3, h: %4]")
															.arg(round(rect.left()))
															.arg(round(rect.top()))
															.arg(round(rect.width()))
															.arg(round(rect.height())));
	}
}

void SceneInfoWidget::updateSelectedObjects(int obj_count, const QRectF &objs_rect)
{
	QRect rect = objs_rect.toRect();

	if(!rect.isValid())
		rect = QRect(0,0,0,0);

	sel_obj_icon_lbl->setPixmap(QPixmap(GuiUtilsNs::getIconPath("selectmove")));
	sel_obj_count_lbl->setText(tr("Selected: <strong>%1</strong>").arg(obj_count));
	obj_pos_info_lbl->setText(QString("(%1, %2) [w: %3, h: %4]")
														.arg(rect.left())
														.arg(rect.top())
														.arg(rect.size().width())
														.arg(rect.size().height()));
}

void SceneInfoWidget::updateMousePosition(const QPointF &mouse_pos)
{
	mouse_pos_info_lbl->setText(QString("%1, %2").arg(round(mouse_pos.x())).arg(round(mouse_pos.y())));
}

void SceneInfoWidget::updateSceneZoom(double zoom)
{
	zoom_info_lbl->setText(QString("%1%").arg(zoom * 100));
}
