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
#include "utils/syntaxhighlighter.h"
#include "pgmodelerunittest.h"
#include <QDialog>
#include <QHBoxLayout>

class SyntaxHighlighterTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		SyntaxHighlighterTest() : PgModelerUnitTest(SCHEMASDIR) {}

	private slots:
		void handleMultiLineComment();
};

void SyntaxHighlighterTest::handleMultiLineComment()
{
	QDialog *dlg=new QDialog;
	QPlainTextEdit *edt=new QPlainTextEdit(dlg);
#warning Replace explict layout instantiation by GuiUtilsNs::createLayout()
	QHBoxLayout *layout=new QHBoxLayout(dlg);
	SyntaxHighlighter *sql_hl=nullptr;

	sql_hl=new SyntaxHighlighter(edt, false);
	sql_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	layout->addWidget(edt);
	dlg->exec();
}

QTEST_MAIN(SyntaxHighlighterTest)
#include "syntaxhighlightertest.moc"
