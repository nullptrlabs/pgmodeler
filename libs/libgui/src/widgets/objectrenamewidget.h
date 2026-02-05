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
\class ObjectRenameWidget
\brief Widget used to quickly rename objects without open the editing form
*/

#ifndef OBJECT_RENAME_WIDGET_H
#define OBJECT_RENAME_WIDGET_H

#include "ui_objectrenamewidget.h"
#include "databasemodel.h"
#include "operationlist.h"
#include "guiglobal.h"

class __libgui ObjectRenameWidget: public QDialog, public Ui::ObjectRenameWidget {
	Q_OBJECT

	private:
		//! \brief Objects to be renamed
		std::vector<BaseObject *> objects;

		//! \brief Operation list used to register the modification over object
		OperationList *op_list;

		//! \brief Model used to made duplicity name validations
		DatabaseModel *model;

		bool paste_mode;

		void hideEvent(QHideEvent *) override;

		void updateLabelsButtons();

		void showEvent(QShowEvent *event) override;

	protected:
		bool eventFilter(QObject *object, QEvent *event) override;

	public:
		ObjectRenameWidget(QWidget *parent);

		/*! \brief This version toggles the "normal mode" where the provided list of object are renamed.
		 * This method performs the all the needed operations pre and post rename like validate relationships,
		 * handle operation history, etc */
		void setAttributes(std::vector<BaseObject *> objs, DatabaseModel *model, OperationList *op_list);

		/*! \brief This version toggles the "paste mode" of the rename widget.
		 *  In this mode, when the user click "Apply" the type name is validated and
		 *  the widget closed. The typed name can be retrieved via getNewName() */
		void setAttributes(BaseObject *object);

		QString getNewName();

	public slots:
		int exec() override;

	private slots:
		void applyRenaming();
		void validateName();
};

#endif
