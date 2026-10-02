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
	QHBoxLayout *layout=new QHBoxLayout(dlg);
	SyntaxHighlighter *sql_hl=nullptr;

	sql_hl=new SyntaxHighlighter(edt, false);
	sql_hl->loadConfiguration(GlobalAttributes::getSQLHighlightConfPath());

	layout->addWidget(edt);
	dlg->exec();
}

QTEST_MAIN(SyntaxHighlighterTest)
#include "syntaxhighlightertest.moc"
