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
#include "coreutilsns.h"
#include "table.h"

class BaseObjectTest: public QObject {
	Q_OBJECT

  private slots:
    void quoteNameIfKeyword();
    void nameIsInvalidIfStartsWithNumber();
		void dontFormatNameIfAlreadyQuoted();
};

void BaseObjectTest::quoteNameIfKeyword()
{
  QString name = "objectname", kw_name="table";
  QCOMPARE(BaseObject::formatName(name), name);
  QCOMPARE(BaseObject::formatName(kw_name), QString("\"%1\"").arg(kw_name));
}

void BaseObjectTest::nameIsInvalidIfStartsWithNumber()
{
  QCOMPARE(BaseObject::isValidName("name"), true);
  QCOMPARE(BaseObject::isValidName("nameA"), true);
}

void BaseObjectTest::dontFormatNameIfAlreadyQuoted()
{
	QString name = "\"SomeObjName\"";
	QCOMPARE(BaseObject::formatName(name), name);
}

QTEST_MAIN(BaseObjectTest)
#include "baseobjecttest.moc"
