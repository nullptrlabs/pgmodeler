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

#include "sceneinfowidget.h"
#include "customuistyle.h"
#include "guiutilsns.h"
#include "relationshipview.h"

SceneInfoWidget::SceneInfoWidget(QWidget *parent): QWidget(parent)
{
	setupUi(this);

	CustomUiStyle::setStyleHint(CustomUiStyle::TabBarFrmHint,
															{ cursor_pos_info_frm, separator_ln,
																obj_sel_info_frm, separator_ln2,
																separator_ln3, separator_ln4,
																seg_mode_frm });

	GuiUtilsNs::configureWidgetsFont({ scenes_cmb, seg_modes_cmb, threshold_spb,
																		 mouse_pos_info_lbl, zoom_info_lbl, sel_obj_count_lbl,
																		 obj_pos_info_lbl, threshold_spb->findChild<QLineEdit *>() },
																	 GuiUtilsNs::MediumFontFactor);

	threshold_spb->setValue(DatabaseModel::DefaultSegThreshold);
	threshold_spb->setSingleStep(DatabaseModel::DefaultSegThreshold / 2);
	threshold_spb->setMinimum(DatabaseModel::DefaultSegThreshold);
	threshold_spb->setMaximum(DatabaseModel::DefaultSegThreshold * 5);
	threshold_spb->setVisible(false);

	connect(scenes_cmb, &QComboBox::activated, this, &SceneInfoWidget::s_activeSceneSelected);

	connect(seg_modes_cmb, &QComboBox::currentIndexChanged, this, [this](int idx) {
		DatabaseModel::SegmentationMode mode = static_cast<DatabaseModel::SegmentationMode>(idx);
		threshold_spb->setVisible(mode == DatabaseModel::SegByThreshold);
		emit s_segmentationModeSelected(mode);
	});

	connect(threshold_spb, &QSpinBox::editingFinished, this, [this](){
		emit s_thresholdSelected(static_cast<unsigned>(threshold_spb->value()));
	});

	// The button is checked while the relationships are visible, the opposite of the model's option
	connect(toggle_rels_tb, &QToolButton::toggled, this, [this](bool visible) {
		emit s_hideRelationshipsToggled(!visible);
	});

	connect(next_tb, &QToolButton::clicked, this, [this](){
		scenes_cmb->setCurrentIndex(scenes_cmb->currentIndex() + 1);
		emit s_activeSceneSelected(scenes_cmb->currentIndex());
		enableNavigationButtons();
	});

	connect(previous_tb, &QToolButton::clicked, this, [this](){
		scenes_cmb->setCurrentIndex(scenes_cmb->currentIndex() - 1);
		emit s_activeSceneSelected(scenes_cmb->currentIndex());
		enableNavigationButtons();
	});
}

void SceneInfoWidget::updateScenesCombo(const QStringList &names, int active_idx)
{
	scenes_cmb->blockSignals(true);
	scenes_cmb->clear();
	scenes_cmb->addItems(names);
	scenes_cmb->setCurrentIndex(active_idx);
	scenes_cmb->blockSignals(false);
	enableNavigationButtons();
}

void SceneInfoWidget::updateHideRelationships(bool hide)
{
	toggle_rels_tb->blockSignals(true);
	toggle_rels_tb->setChecked(!hide);
	toggle_rels_tb->blockSignals(false);
}

void SceneInfoWidget::updateSegmentationInfo(DatabaseModel::SegmentationMode mode, unsigned threshold)
{
	seg_modes_cmb->blockSignals(true);
	seg_modes_cmb->setCurrentIndex(static_cast<int>(mode));
	seg_modes_cmb->blockSignals(false);

	threshold_spb->blockSignals(true);

	//Widens the ceiling instead of silently clamping a legitimately larger persisted/loaded value
	if(static_cast<int>(threshold) > threshold_spb->maximum())
		threshold_spb->setMaximum(static_cast<int>(threshold));

	threshold_spb->setValue(static_cast<int>(threshold));
	threshold_spb->setVisible(mode == DatabaseModel::SegByThreshold);
	threshold_spb->blockSignals(false);

	scene_nav_wgt->setVisible(mode != DatabaseModel::NoSegmentation);
}

void SceneInfoWidget::updateSelectedObject(BaseObjectView *object)
{
	if(!object)
	{
		sel_obj_icon_lbl->setPixmap(GuiUtilsNs::getPixmap("close1"));
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

	sel_obj_icon_lbl->setPixmap(GuiUtilsNs::getPixmap("selectmove"));
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

void SceneInfoWidget::enableNavigationButtons()
{
	previous_tb->setEnabled(scenes_cmb->currentIndex() > 0 && scenes_cmb->count() > 1);
	next_tb->setEnabled(scenes_cmb->currentIndex() >= 0 && scenes_cmb->currentIndex() != (scenes_cmb->count() - 1));
}
