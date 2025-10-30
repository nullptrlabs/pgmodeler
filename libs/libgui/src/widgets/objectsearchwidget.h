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

/**
\ingroup libgui
\class ObjectSearchWidget
\brief Implements the operations to find objects over the model
*/

#ifndef OBJECT_SEARCH_WIDGET_H
#define OBJECT_SEARCH_WIDGET_H

#include "ui_objectsearchwidget.h"
#include "modelwidget.h"
#include "objecttypeslistwidget.h"
#include <QWidgetAction>
#include <QComboBox>

class __libgui ObjectSearchWidget : public QWidget, public Ui::ObjectSearchWidget {
	Q_OBJECT

	private:
		std::vector<BaseObject *> found_objs,

		selected_objs;

		QMenu select_menu, fade_menu, filter_menu;

		//! \brief Reference model widget
		ModelWidget *model_wgt;

		//! \brief Stores the selected object on the result list
		BaseObject *selected_obj;

		QWidget *filter_wgt;

		QCheckBox *regexp_chk, *exact_match_chk, *case_sensitive_chk;

		ObjectTypesListWidget *obj_types_lst;

		QComboBox *_search_attrs_cmb;

		QLabel *_search_attrs_lbl;

		//! \brief Captures the ENTER press to execute search
		bool eventFilter(QObject *object, QEvent *event) override;

		void showEvent(QShowEvent *) override;

		void resizeEvent(QResizeEvent *event) override;

	public:
		ObjectSearchWidget(QWidget *parent = nullptr);

		//! \brief Sets the database model to work on
		void setModel(ModelWidget *model_wgt);

	signals:
		void s_hideRequested();

	public slots:
		void hide();
		void clearResult();
		void findObjects();

	private slots:
		//! \brief Opens the editing form of the selected object
		void editObject();

		//! \brief Selects a object on result list
		void selectObject();

		void fadeObjects();
		void selectObjects();
		void showObjectMenu();

		friend class MainWindow;
};

#endif
