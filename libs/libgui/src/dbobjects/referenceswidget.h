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
\class ReferencesWidget
\brief Implements the operations to create/edit generic sql object references via form.
*/

#ifndef REFERENCES_WIDGET_H
#define REFERENCES_WIDGET_H

#include "ui_referenceswidget.h"
#include "widgets/customtablewidget.h"
#include "widgets/objectselectorwidget.h"

class __libgui ReferencesWidget: public QWidget, Ui::ReferencesWidget {
	Q_OBJECT

	private:
		CustomTableWidget *references_tab;

		ObjectSelectorWidget *object_sel;

		bool conf_view_refs;

		//! \brief A regular expression used to remove attribute/reference delimiters {} from the names of configured references
		static const QRegularExpression AttrDelimRegexp;

		void showReferenceData(int row, BaseObject *object, const QString &ref_name, const QString &ref_alias,
													bool use_signature, bool format_name, bool use_columns);

	public:
		ReferencesWidget(const std::vector<ObjectType> &types, bool conf_view_refs, QWidget * parent = nullptr);

		void setAttributes(DatabaseModel *model, const std::vector<Reference> &refs);

		std::vector<Reference> getObjectReferences();

	private slots:
		void handleReference(int row);
		void editReference(int row);
		void clearReferenceForm();
};

#endif
