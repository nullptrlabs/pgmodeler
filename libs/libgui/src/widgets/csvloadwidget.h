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
\class CsvLoadWidget
\brief Implements basic features to load csv files and return the loaded buffer to the application
*/

#ifndef CSV_LOAD_WIDGET_H
#define CSV_LOAD_WIDGET_H

#include "ui_csvloadwidget.h"
#include "fileselectorwidget.h"
#include "csvdocument.h"

class __libgui CsvLoadWidget : public QWidget, Ui::CsvLoadWidget {
	Q_OBJECT

	private:
		//! \brief Holds the names of columns extracted from the csv file
		QStringList csv_columns;

		//! \brief Holds the rows extracted from the csv file
		QList<QStringList> csv_rows;

		FileSelectorWidget *file_sel;

		CsvDocument csv_document;

	public:
		CsvLoadWidget(QWidget * parent = nullptr, bool cols_in_first_row = true);

		CsvDocument getCsvDocument();

		bool isColumnsInFirstRow();

		QChar getSeparator();

		/*! \brief Loads a csv document from a buffer. The user can specify the value separator, text delimiter and an object
		 *  which will store the column names.In that case, the column names are only extracted from the first
		 *  row if the cols_in_first_row is true */
		static CsvDocument loadCsvFromBuffer(const QString &csv_buffer, const QChar &separator, const QChar &text_delim, bool cols_in_first_row);

	private slots:
		void loadCsvFile();

	signals:
		void s_csvFileLoaded();
};

#endif
