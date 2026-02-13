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
\class BugReportForm
\brief Implements operations to permit user generate bug reports without use the crash handler application.
*/

#ifndef BUG_REPORT_FORM_H
#define BUG_REPORT_FORM_H

#include <QDialog>
#include "ui_bugreportform.h"
#include "utils/syntaxhighlighter.h"
#include "widgets/fileselectorwidget.h"

class __libgui BugReportForm : public QDialog, public Ui::BugReportForm {
	Q_OBJECT

	private:	
		//! \brief Syntax highlight for model text widget
		SyntaxHighlighter *hl_model_txt;

		FileSelectorWidget *output_sel;
		
	protected:
		//! \brief Delimiter character which separates the sections of the compressed file
		static constexpr char CharDelimiter = static_cast<char>(3);
		
		//! \brief Generates an uncompressed buffer based upon the data in fields
		virtual QByteArray generateReportBuffer();
		
		//! \brief Generates the bug report file from uncompressed buffer
		void generateReport(const QByteArray &buf);
		
	public:
		BugReportForm(QWidget * parent = nullptr, Qt::WindowFlags f = Qt::Widget);
		
		//! \brief Attach the provided file in the bug report
		void attachModel(const QString &filename);

	public slots:
		void generateReport();
		
	private slots:
		void enableGeneration();
		void attachModel();
};

#endif
