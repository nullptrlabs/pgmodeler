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
#include "widgets/fileselectorwidget.h"

class FileSelectorTest: public QObject {
	Q_OBJECT

  private slots:
		void testFileSelection();
};

void FileSelectorTest::testFileSelection()
{
  QDialog *dlg = new QDialog;
  QHBoxLayout *layout = new QHBoxLayout(dlg);
  FileSelectorWidget *selector = new FileSelectorWidget;

	//selector->setFileMode(QFileDialog::ExistingFile);
	selector->setFileMustExist(true);
	selector->setAcceptMode(QFileDialog::AcceptOpen);
	//selector->setNameFilters({"Database model file (*.dbm)", "All files (*.*)"});
	selector->setFileDialogTitle("Select *.dbm file...");
	selector->setAllowFilenameInput(true);
	selector->setSelectedFile("/home/raphael/pgmodeler");
	//selector->setReadOnly(true);

	layout->addWidget(selector);
	dlg->setMinimumWidth(400);
	dlg->exec();
}

QTEST_MAIN(FileSelectorTest)
#include "fileselectortest.moc"
