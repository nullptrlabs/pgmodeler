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
#include "utilsns.h"
#include "exception.h"

class XmlParserTest: public QObject {
	Q_OBJECT

	private slots:
		void correctlyConvertJsonValsToXmlEntites();
};

void XmlParserTest::correctlyConvertJsonValsToXmlEntites()
{
	QString value = "value=\"'{\"attr\": { \"\" }}'::json\"		value-abc=\"true\"     value-cde=\"'{\"sign_aspect\": { \"message_no\": 0, \"message_multi\": \"\" }}'::json\"\n",
			expected = "value=\"'{&quot;attr&quot;: { &quot;&quot; }}'::json\"		value-abc=\"true\"     value-cde=\"'{&quot;sign_aspect&quot;: { &quot;message_no&quot;: 0, &quot;message_multi&quot;: &quot;&quot; }}'::json\"\n",
			result;

	try
	{
		result = UtilsNs::convertToXmlEntities(value);
		QCOMPARE(result, expected);
	}
	catch(Exception &e)
	{
		QFAIL(e.getExceptionsText().toStdString().c_str());
	}
}

QTEST_MAIN(XmlParserTest)
#include "xmlparsertest.moc"
