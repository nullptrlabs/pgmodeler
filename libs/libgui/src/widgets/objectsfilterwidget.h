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
\class ObjectsFilterWidget
\brief Implements the operations to generated filter patterns for reverse engineering process.
*/

#ifndef OBJECTS_FILTER_WIDGET_H
#define OBJECTS_FILTER_WIDGET_H

#include "guiglobal.h"
#include "ui_objectsfilterwidget.h"
#include <QComboBox>
#include <QMenu>
#include <QWidgetAction>
#include <QListWidget>
#include "baseobject.h"

class __libgui ObjectsFilterWidget : public QWidget, Ui::ObjectsFilterWidget {
	Q_OBJECT

	private:
		std::vector<ObjectType> extra_obj_types;

		QMenu tab_objs_menu, options_menu;

		QAction *action_only_matching, *action_match_signature, *action_forced_filter;

		QWidgetAction *wgt_act_forced_filter;

		QListWidget *tab_objs_lst;

		QFrame *frame;

		//! \brief Creates a combobox of the objects' types accepted in the filter
		QComboBox *createObjectsCombo();

	public:
		explicit ObjectsFilterWidget(QWidget *parent = nullptr);

		//! \brief Returns a list of filters in the format accepted by the Catalog class (object_type:pattern:mode)
		QStringList getObjectFilters();

		//! \brief Returns a list of table children objects to be filtered forcebly
		QStringList getForceObjectsFilter();

		/*! \brief Set a list of table children objects to be filtered forcebly
		 *  Invalid types are ignored by this method */
		void setForceObjectsFilter(const QStringList &tab_obj_types);

		void setMatchBySignature(bool value);
		void setOnlyMatching(bool value);

		bool isOnlyMatching();
		bool isMatchBySignature();
		bool hasFiltersConfigured();

		/*! \brief When value is true the widget disable some features that aren't used
		 * by the object filtering in database models. Additionally, when the model filtering is enabled
		 * extra object types can be provided so they can be enabled in the filters construction */
		void setModelFilteringMode(bool value, const std::vector<ObjectType> &extra_types = {});

		//! \brief Populates the widget using a list of preconfigured filters
		void addFilters(const QStringList &filters);

	private slots:
		void addFilter();
		void removeFilter();

	public slots:
		void clearFilters();

	signals:
		void s_filterApplyingRequested();
		void s_filtersRemoved();
};

#endif
