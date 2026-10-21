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
#include "relationshipconfigwidget.h"
#include "relationshipview.h"
#include "xmlparser.h"
#include "schemaparser.h"
#include "globalattributes.h"
#include "attributes.h"
#include "utilsns.h"
#include "pgmodelerunittest.h"

/*! \brief Tests the settings of the relationship lines: the line style (straight, curved or orthogonal) in the
configuration files, their DTDs and schemas (including the files of previous versions that have the old option
to use curved lines) and the widget that edits it. Nothing is written to the user's configuration. */
class RelationshipSettingsTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		RelationshipSettingsTest() : PgModelerUnitTest(SCHEMASDIR){}

	private:
		RelationshipView::LineStyle saved_style;
		bool saved_crows_foot;
		unsigned saved_mode;

		//! \brief Returns whether the XML text is valid against the DTD of the configuration file
		bool isValid(const QString &dtd_name, const QString &xml, QString *error = nullptr);

		QString confText(const QString &name);

	private slots:
		void init();
		void cleanup();

		void theDefaultRelationshipsConfIsValidAndOrthogonal();
		void everyLineStyleIsAcceptedByTheDtd();
		void anUnknownLineStyleIsRejectedByTheDtd();
		void aFileWithoutTheLineStyleIsValid();
		void aFileWithTheOldOptionOfTheGeneralConfIsStillValid();
		void theDefaultGeneralConfHasNoOldOption();
		void theSchemaGeneratesAValidFileForEveryStyleAndMode();
		void theSchemaSeparatesTheAttributesOfTheRootElement();

		void theWidgetOffersTheThreeStylesInTheViewsOrder();
		void applyingTheWidgetChangesTheStyleOfTheViews();
		void theWidgetKeepsTheLinkModeAndTheStyleIndependent();
		void changingTheStyleMarksTheConfigurationAsChanged();
		void theStyleOfTheWidgetFollowsTheLoadedFile();
};

void RelationshipSettingsTest::init()
{
	saved_style = RelationshipView::getLineStyle();
	saved_crows_foot = RelationshipView::isCrowsFoot();
	saved_mode = RelationshipView::getLineConnectinMode();
}

void RelationshipSettingsTest::cleanup()
{
	RelationshipView::setLineStyle(saved_style);
	RelationshipView::setCrowsFoot(saved_crows_foot);
	RelationshipView::setLineConnectionMode(static_cast<RelationshipView::LineConnectionMode>(saved_mode));
}

QString RelationshipSettingsTest::confText(const QString &name)
{
	return QString::fromUtf8(UtilsNs::loadFile(GlobalAttributes::getTmplConfigurationFilePath(name)));
}

bool RelationshipSettingsTest::isValid(const QString &dtd_name, const QString &xml, QString *error)
{
	XmlParser parser;

	try
	{
		parser.setDTDFile(GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::ObjectDTDDir,
																																		 dtd_name + GlobalAttributes::ObjectDTDExt), dtd_name);
		parser.loadXMLBuffer(xml);
	}
	catch(Exception &e)
	{
		if(error)
			*error = e.getExceptionsText();

		return false;
	}

	return true;
}

void RelationshipSettingsTest::theDefaultRelationshipsConfIsValidAndOrthogonal()
{
	QString xml = confText("relationships.conf"), err;

	QVERIFY2(isValid("relationships", xml, &err), qPrintable(err));
	QVERIFY(xml.contains("line-style=\"orthogonal\""));
}

void RelationshipSettingsTest::everyLineStyleIsAcceptedByTheDtd()
{
	QString xml = confText("relationships.conf"), err;

	for(auto style : { "straight", "curved", "orthogonal" })
	{
		QString changed = xml;

		changed.replace("line-style=\"orthogonal\"", QString("line-style=\"%1\"").arg(style));
		QVERIFY2(isValid("relationships", changed, &err), qPrintable(QString("%1: %2").arg(style, err)));
	}
}

void RelationshipSettingsTest::anUnknownLineStyleIsRejectedByTheDtd()
{
	QString xml = confText("relationships.conf");

	for(auto style : { "zigzag", "", "Orthogonal", "true" })
	{
		QString changed = xml;

		changed.replace("line-style=\"orthogonal\"", QString("line-style=\"%1\"").arg(style));
		QVERIFY2(!isValid("relationships", changed), style);
	}
}

void RelationshipSettingsTest::aFileWithoutTheLineStyleIsValid()
{
	QString xml = confText("relationships.conf"), err;

	// Files saved by previous versions have no line style: the DTD supplies the default
	xml.replace(" line-style=\"orthogonal\"", "");
	QVERIFY(!xml.contains("line-style"));
	QVERIFY2(isValid("relationships", xml, &err), qPrintable(err));
}

void RelationshipSettingsTest::aFileWithTheOldOptionOfTheGeneralConfIsStillValid()
{
	QString xml = confText("general.conf"), err;

	// The old option (curved lines) lived in the general configuration
	QVERIFY(xml.contains("<configuration "));

	for(auto value : { "true", "false" })
	{
		QString changed = xml;

		changed.replace("<configuration ", QString("<configuration use-curved-lines=\"%1\" ").arg(value));
		QVERIFY2(isValid("general", changed, &err), qPrintable(QString("%1: %2").arg(value, err)));
	}
}

void RelationshipSettingsTest::theDefaultGeneralConfHasNoOldOption()
{
	QString xml = confText("general.conf"), err;

	QVERIFY2(isValid("general", xml, &err), qPrintable(err));
	QVERIFY(!xml.contains("use-curved-lines"));
}

void RelationshipSettingsTest::theSchemaGeneratesAValidFileForEveryStyleAndMode()
{
	SchemaParser parser;
	QString sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "relationships" + GlobalAttributes::SchemaExt), err;

	for(auto style : { "straight", "curved", "orthogonal" })
	{
		for(auto mode : { "fk-to-pk", "center-pnts", "table-edges", "crows-foot" })
		{
			attribs_map attribs = {
				{ "link-mode", mode }, { "line-style", style }, { "deferrable", "false" }, { "defer-type", "" },
				{ "upd-action", "" }, { "del-action", "" }, { "fk-idx-type", "" },
				{ "patterns", "\t\t<patterns type=\"relgen\" pk-pattern=\"{dt}_pk\"/>\n" }
			};

			parser.ignoreEmptyAttributes(true);

			QString xml = parser.getSourceCode(sch, attribs);

			QVERIFY2(xml.contains(QString("line-style=\"%1\"").arg(style)), qPrintable(xml));
			QVERIFY2(xml.contains(QString("link-mode=\"%1\"").arg(mode)), qPrintable(xml));
			QVERIFY2(isValid("relationships", xml, &err), qPrintable(QString("%1/%2: %3").arg(style, mode, err)));
		}
	}
}

void RelationshipSettingsTest::theSchemaSeparatesTheAttributesOfTheRootElement()
{
	SchemaParser parser;
	QString sch = GlobalAttributes::getTmplConfigurationFilePath(GlobalAttributes::SchemasDir, "relationships" + GlobalAttributes::SchemaExt);
	attribs_map attribs = {
		{ "link-mode", "crows-foot" }, { "line-style", "curved" }, { "deferrable", "false" }, { "defer-type", "" },
		{ "upd-action", "" }, { "del-action", "" }, { "fk-idx-type", "" }, { "patterns", "" }
	};

	parser.ignoreEmptyAttributes(true);

	// The attributes must have a space between them, otherwise the generated file is not a valid XML
	QString xml = parser.getSourceCode(sch, attribs);

	QVERIFY2(xml.contains("<relationships link-mode=\"crows-foot\" line-style=\"curved\">") ||
					 xml.contains("<relationships link-mode=\"crows-foot\" line-style=\"curved\" >"), qPrintable(xml));
	QVERIFY(!xml.contains("\"line-style"));
}

void RelationshipSettingsTest::theWidgetOffersTheThreeStylesInTheViewsOrder()
{
	RelationshipConfigWidget wgt;

	QCOMPARE(wgt.rel_line_style_cmb->count(), 3);

	// The position of each item is the value of the style in the views
	QCOMPARE(wgt.rel_line_style_cmb->itemData(RelationshipView::StraightLines).toString(), QString(Attributes::Straight));
	QCOMPARE(wgt.rel_line_style_cmb->itemData(RelationshipView::CurvedLines).toString(), QString(Attributes::Curved));
	QCOMPARE(wgt.rel_line_style_cmb->itemData(RelationshipView::OrthogonalLines).toString(), QString(Attributes::Orthogonal));
	QCOMPARE(wgt.rel_line_style_cmb->findData("zigzag"), -1);
}

void RelationshipSettingsTest::applyingTheWidgetChangesTheStyleOfTheViews()
{
	RelationshipConfigWidget wgt;

	for(auto style : { RelationshipView::CurvedLines, RelationshipView::StraightLines, RelationshipView::OrthogonalLines, RelationshipView::StraightLines })
	{
		wgt.rel_line_style_cmb->setCurrentIndex(style);
		wgt.applyConfiguration();
		QCOMPARE(RelationshipView::getLineStyle(), style);
	}
}

void RelationshipSettingsTest::theWidgetKeepsTheLinkModeAndTheStyleIndependent()
{
	RelationshipConfigWidget wgt;

	for(auto style : { RelationshipView::CurvedLines, RelationshipView::OrthogonalLines })
	{
		wgt.rel_line_style_cmb->setCurrentIndex(style);

		wgt.crows_foot_tb->setChecked(false);
		wgt.center_pnts_tb->setChecked(true);
		wgt.applyConfiguration();
		QCOMPARE(RelationshipView::getLineStyle(), style);
		QVERIFY(!RelationshipView::isCrowsFoot());
		QCOMPARE(RelationshipView::getLineConnectinMode(), unsigned(RelationshipView::ConnectCenterPoints));

		wgt.crows_foot_tb->setChecked(true);
		wgt.applyConfiguration();
		QCOMPARE(RelationshipView::getLineStyle(), style);
		QVERIFY(RelationshipView::isCrowsFoot());
		QCOMPARE(RelationshipView::getLineConnectinMode(), unsigned(RelationshipView::ConnectTableEdges));
	}
}

void RelationshipSettingsTest::changingTheStyleMarksTheConfigurationAsChanged()
{
	RelationshipConfigWidget wgt;

	wgt.setConfigurationChanged(false);
	QVERIFY(!wgt.isConfigurationChanged());

	wgt.rel_line_style_cmb->setCurrentIndex((wgt.rel_line_style_cmb->currentIndex() + 1) % 3);
	QVERIFY(wgt.isConfigurationChanged());

	wgt.applyConfiguration();
	QVERIFY(!wgt.isConfigurationChanged());
}

void RelationshipSettingsTest::theStyleOfTheWidgetFollowsTheLoadedFile()
{
	RelationshipConfigWidget wgt;

	/* The loading reads the user's configuration (or the default one): whatever it has, the widget ends up
	 * with one of the styles, and with the orthogonal one when the file doesn't say anything */
	wgt.loadConfiguration();
	QVERIFY(wgt.rel_line_style_cmb->currentIndex() >= 0 && wgt.rel_line_style_cmb->currentIndex() < 3);
	QVERIFY(wgt.rel_line_style_cmb->currentData().toString() == Attributes::Straight ||
					wgt.rel_line_style_cmb->currentData().toString() == Attributes::Curved ||
					wgt.rel_line_style_cmb->currentData().toString() == Attributes::Orthogonal);
}

QTEST_MAIN(RelationshipSettingsTest)
#include "relationshipsettingstest.moc"
