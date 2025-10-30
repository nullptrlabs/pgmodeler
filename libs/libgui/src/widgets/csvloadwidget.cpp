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

#include "csvloadwidget.h"
#include <QFileDialog>
#include "exception.h"
#include <QTextStream>
#include "csvparser.h"
#include "messagebox.h"

CsvLoadWidget::CsvLoadWidget(QWidget * parent, bool cols_in_first_row) : QWidget(parent)
{
	setupUi(this);

	file_sel = new FileSelectorWidget(this);
	file_sel->setFileMustExist(true);
	file_sel->setFileDialogTitle(tr("Load CSV file"));
	file_sel->setMimeTypeFilters({"text/csv", "application/octet-stream"});
	load_csv_grid->addWidget(file_sel, 0, 1, 1, 8);

	separator_edt->setVisible(false);

	if(cols_in_first_row)
	{
		col_names_chk->setVisible(false);
		col_names_chk->setChecked(true);
	}

	connect(txt_delim_chk, &QCheckBox::toggled, txt_delim_edt, &QLineEdit::setEnabled);
	connect(load_btn, &QPushButton::clicked, this, &CsvLoadWidget::loadCsvFile);

	connect(separator_cmb, &QComboBox::currentTextChanged, this, [this](){
			separator_edt->setVisible(separator_cmb->currentIndex() == separator_cmb->count()-1);
	});

	connect(file_sel, &FileSelectorWidget::s_selectorChanged, load_btn, &QPushButton::setEnabled);
}

CsvDocument CsvLoadWidget::getCsvDocument()
{
	return csv_document;
}

CsvDocument CsvLoadWidget::loadCsvFromBuffer(const QString &csv_buffer, const QChar &separator, const QChar &text_delim, bool cols_in_first_row)
{
	try
	{
		CsvDocument csv_doc;
		CsvParser csv_parser;

		csv_parser.setSpecialChars(separator, text_delim, CsvDocument::LineBreak);
		csv_parser.setColumnInFirstRow(cols_in_first_row);
		return csv_parser.parseBuffer(csv_buffer);
	}
	catch(Exception &e)
	{
		throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
	}
}

void CsvLoadWidget::loadCsvFile()
{
	try
	{
		CsvParser csv_parser;

		csv_parser.setSpecialChars(getSeparator(),
															 txt_delim_chk->isChecked() ? txt_delim_edt->text().at(0) : CsvDocument::TextDelimiter,
															 CsvDocument::LineBreak);
		csv_parser.setColumnInFirstRow(col_names_chk->isChecked());
		csv_document = csv_parser.parseFile(file_sel->getSelectedFile());
		file_sel->clearSelector();

		emit s_csvFileLoaded();
	}
	catch(Exception &e)
	{
		Messagebox::error(e, PGM_FUNC, PGM_FILE, PGM_LINE);
	}
}

QChar CsvLoadWidget::getSeparator()
{
	QString separators =";, \t";
	separators += separator_edt->text().isEmpty() ? CsvDocument::Separator : separator_edt->text();
	return separators[separator_cmb->currentIndex()];
}

bool CsvLoadWidget::isColumnsInFirstRow()
{
	return col_names_chk->isChecked();
}
