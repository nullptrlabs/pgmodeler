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

#include <QtTest/QtTest>
#include "widgets/numberedtexteditor.h"
#include <QHBoxLayout>
#include <QDialog>

class LineNumbersTest: public QObject {
	Q_OBJECT

  private slots:
		void handleLineSelectionUsingMouse();
};

void LineNumbersTest::handleLineSelectionUsingMouse()
{
  QDialog *dlg=new QDialog;
	NumberedTextEditor *edt=new NumberedTextEditor(dlg, true);
#warning Replace explict layout instantiation by GuiUtilsNs::createLayout()
  QHBoxLayout *layout=new QHBoxLayout(dlg);

	LineNumbersWidget::setColors(Qt::darkGray, QColor(230,230,230));

  layout->addWidget(edt);
	dlg->setMinimumSize(800,600);

	QFont fnt = edt->font();
	edt->setFont(fnt);
	edt->setPlainText("--\
-- PostgreSQL database dump\
--\
-- Dumped from database version 9.4.4 \
-- Dumped by pg_dump version 9.4.4 \
-- Started on 2015-07-20 19:24:52 WEST \
 \
SET statement_timeout = 0; \
SET lock_timeout = 0; \
SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; \
SET check_function_bodies = false; \
SET client_min_messages = warning; \
-- \
-- PostgreSQL database dump \
-- \
-- Dumped from database version 9.4.4 \
-- Dumped by pg_dump version 9.4.4 \
-- Started on 2015-07-20 19:24:52 WEST \
SET statement_timeout = 0; \
SET lock_timeout = 0; \
SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; \
SET check_function_bodies = false; \
SET client_min_messages = warning; \
-- Dumped from database version 9.4.4 \
-- Dumped by pg_dump version 9.4.4 \
-- Started on 2015-07-20 19:24:52 WEST \
SET statement_timeout = 0; \
SET lock_timeout = 0; \
SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; \
SET check_function_bodies = false; \
SET client_min_messages = warning; \
-- Dumped from database version 9.4.4 \
-- Dumped by pg_dump version 9.4.4 \
-- Started on 2015-07-20 19:24:52 WEST \
SET statement_timeout = 0; \
SET lock_timeout = 0; \
SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; \
SET check_function_bodies = false; \
SET client_min_messages = warning; \n\
foobaar\n\
bla\n\
-- Dumped from database version 9.4.4 -- Dumped by pg_dump version 9.4.4\
 -- Started on 2015-07-20 19:24:52 WEST SET statement_timeout = 0; \
SET lock_timeout = 0; SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; SET check_function_bodies = false; \
SET client_min_messages = warning; -- Dumped from database version 9.4.4 \
-- Dumped by pg_dump version 9.4.4 -- Started on 2015-07-20 19:24:52 WEST \
SET statement_timeout = 0; SET lock_timeout = 0; SET client_encoding = 'UTF8'; \
SET standard_conforming_strings = on; SET check_function_bodies = false; \
SET client_min_messages = warning;\n\n\n\n\n\n\n");

	//edt->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
  dlg->exec();
}

QTEST_MAIN(LineNumbersTest)
#include "linenumberstest.moc"
