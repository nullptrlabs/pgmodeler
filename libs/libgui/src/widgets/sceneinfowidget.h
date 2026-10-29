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

/**
\ingroup libgui
\class SceneInfoWidget
\brief Implements an widget to show info about canvas (scene) and selected objects
*/

#ifndef SCENE_INFO_WIDGET_H
#define SCENE_INFO_WIDGET_H

#include "guiglobal.h"
#include "ui_sceneinfowidget.h"
#include "baseobjectview.h"
#include "databasemodel.h"

class __libgui SceneInfoWidget: public QWidget, public Ui::SceneInfoWidget {
	Q_OBJECT

	public:
		SceneInfoWidget(QWidget * parent = nullptr);

	public slots:
		void updateSelectedObject(BaseObjectView *object);
		void updateSelectedObjects(int obj_count, const QRectF &objs_rect);
		void updateMousePosition(const QPointF &mouse_pos);
		void updateSceneZoom(double zoom);

		//! \brief Repopulates scenes_cmb from names, selecting active_idx without emitting s_activeSceneSelected
		void updateScenesCombo(const QStringList &names, int active_idx);

		//! \brief Reflects mode/threshold on seg_modes_cmb/threshold_spb without emitting the selection signals
		void updateSegmentationInfo(DatabaseModel::SegmentationMode mode, unsigned threshold);

		//! \brief Reflects the hide relationships option on toggle_rels_tb (checked when the relationships are visible) without emitting s_hideRelationshipsToggled
		void updateHideRelationships(bool value);

	signals:
		//! \brief Emitted when the user picks a different scene in scenes_cmb
		void s_activeSceneSelected(int idx);

		//! \brief Emitted when the user picks a different segmentation mode in seg_modes_cmb
		void s_segmentationModeSelected(DatabaseModel::SegmentationMode mode);

		//! \brief Emitted when the user commits a new value in threshold_spb
		void s_thresholdSelected(unsigned value);

		//! \brief Emitted when the user toggles toggle_rels_tb, carrying the new value of the hide relationships option
		void s_hideRelationshipsToggled(bool value);

	private slots:
		void enableNavigationButtons();
};

#endif
