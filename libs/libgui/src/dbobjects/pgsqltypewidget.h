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
\class PgSQLTypeWidget
\brief Implements the operations to configure PostgreSQL types via form.
*/

#ifndef PGSQL_TYPE_WIDGET_H
#define PGSQL_TYPE_WIDGET_H

#include "ui_pgsqltypewidget.h"
#include "utils/syntaxhighlighter.h"
#include "databasemodel.h"

class __libgui PgSQLTypeWidget: public QWidget, public Ui::PgSQLTypeWidget {
	Q_OBJECT

	private:
		//! \brief Stores the PostgreSQL type configured on the form
		PgSqlType type;

		//! \brief Syntax highlighter used on the format field
		SyntaxHighlighter *format_hl;

		bool allow_qualifiers;

		bool eventFilter(QObject *object, QEvent *event) override;

		static const QString InvalidType;

	public:
		PgSQLTypeWidget(QWidget * parent = nullptr, const QString &label="");

		/*! \brief Lists the PostgreSQL types on the specified combo. The user can configure which types
		must be shown using the last tree parameters. The DatabaseModel parameter is used to
		gather the user-defined types of the specified model. */
		static void listPgSQLTypes(QComboBox *combo, DatabaseModel *model,
									 UserTypeConfig::TypeConf user_type_conf=UserTypeConfig::AllUserTypes,
								   bool oid_types=true, bool pseudo_types=true);

	private slots:
		void updateTypeFormat();

	public slots:
		/*! \brief Configures the widget with the provided data type
		 *  The parameter allow_qualifiers indicates if length, preciosion, interval and other details of the type can be handled
		 *  The usr_type_conf indicates which data types must be handled by the widget
		 *  The oid_types indicates if oid data types are allowed
		 *  The pseudo_types indicates if pseudo data types are allowed */
		void setAttributes(PgSqlType type, DatabaseModel *model,
											 bool allow_qualifiers = true,
											 UserTypeConfig::TypeConf usr_type_conf=UserTypeConfig::AllUserTypes,
											 bool oid_types=true, bool pseudo_types=true);

		//! \brief Returns the PostgreSQL type configured via form
		PgSqlType getPgSQLType();
};

#endif
