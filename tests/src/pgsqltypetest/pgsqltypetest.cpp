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
#include "pgsqltypes/pgsqltype.h"
#include "pgsqlversions.h"
#include "pgmodelerunittest.h"

/*! \brief Tests the registration of the built-in types in PgSqlType (categories and the boundaries between them)
and the supported PostgreSQL versions. */
class PgSqlTypeTest: public QObject, public PgModelerUnitTest {
	Q_OBJECT

	public:
		PgSqlTypeTest() : PgModelerUnitTest(SCHEMASDIR){}

	private slots:
		void oidTypesAreRegistered_data();
		void oidTypesAreRegistered();
		void categoriesKeepTheirBoundaries_data();
		void categoriesKeepTheirBoundaries();
		void getTypesFiltersOidsAndPseudos();
		void typeIdsRoundTripThroughNames();
		void postgreSql19IsTheDefaultVersion();
};

void PgSqlTypeTest::oidTypesAreRegistered_data()
{
	QTest::addColumn<QString>("name");

	for(auto &name : QStringList { "oid", "oid8", "regproc", "regprocedure", "regoper", "regoperator",
																 "regclass", "regrole", "regnamespace", "regdatabase", "regtype",
																 "regconfig", "regdictionary", "xid", "cid", "tid", "oidvector" })
		QTest::newRow(qPrintable(name)) << name;
}

void PgSqlTypeTest::oidTypesAreRegistered()
{
	QFETCH(QString, name);

	QVERIFY(PgSqlType::isRegistered(name));

	PgSqlType type(name);

	QVERIFY(type.isOidType());
	QVERIFY(!type.isPseudoType());
	QVERIFY(!type.isUserType());
	QCOMPARE(type.getCategory(), PgSqlType::OidType);
	QCOMPARE(~type, name);
	QCOMPARE(type.getTypeSql(), name);
}

void PgSqlTypeTest::categoriesKeepTheirBoundaries_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<int>("category");

	// First/last types of each range, they move whenever a type is added before them
	QTest::newRow("smallint") << "smallint" << static_cast<int>(PgSqlType::IntegerType);
	QTest::newRow("pg_lsn") << "pg_lsn" << static_cast<int>(PgSqlType::OtherType);
	QTest::newRow("box2d") << "box2d" << static_cast<int>(PgSqlType::PostGiSType);
	QTest::newRow("valid_detail") << "valid_detail" << static_cast<int>(PgSqlType::PostGiSType);
	QTest::newRow("int4range") << "int4range" << static_cast<int>(PgSqlType::RangeType);
	QTest::newRow("tstzmultirange") << "tstzmultirange" << static_cast<int>(PgSqlType::RangeType);
	QTest::newRow("oid") << "oid" << static_cast<int>(PgSqlType::OidType);
	QTest::newRow("oidvector") << "oidvector" << static_cast<int>(PgSqlType::OidType);
	QTest::newRow("\"any\"") << "\"any\"" << static_cast<int>(PgSqlType::PolymorphicType);
	QTest::newRow("cstring") << "cstring" << static_cast<int>(PgSqlType::PseudoType);
	QTest::newRow("anyelement") << "anyelement" << static_cast<int>(PgSqlType::PolymorphicType);
	QTest::newRow("unknown") << "unknown" << static_cast<int>(PgSqlType::PseudoType);
}

void PgSqlTypeTest::categoriesKeepTheirBoundaries()
{
	QFETCH(QString, name);
	QFETCH(int, category);

	PgSqlType type(name);

	QCOMPARE(static_cast<int>(type.getCategory()), category);
	QVERIFY(!type.isUserType());
}

void PgSqlTypeTest::getTypesFiltersOidsAndPseudos()
{
	QStringList all = PgSqlType::getTypes(true, true),
			no_oids = PgSqlType::getTypes(false, true),
			no_pseudos = PgSqlType::getTypes(true, false),
			base = PgSqlType::getTypes(false, false);

	for(auto &name : QStringList { "oid8", "regdatabase" })
	{
		QVERIFY(all.contains(name));
		QVERIFY(no_pseudos.contains(name));
		QVERIFY(!no_oids.contains(name));
		QVERIFY(!base.contains(name));
	}

	QVERIFY(all.contains("unknown"));
	QVERIFY(no_oids.contains("unknown"));
	QVERIFY(!no_pseudos.contains("unknown"));

	// 17 oid types and 25 pseudo types
	QCOMPARE(all.size() - no_oids.size(), 17);
	QCOMPARE(all.size() - no_pseudos.size(), 25);
}

void PgSqlTypeTest::typeIdsRoundTripThroughNames()
{
	for(auto &name : PgSqlType::getTypes(true, true))
	{
		PgSqlType type(name);
		PgSqlType from_id(type.getTypeId(), 0);

		QCOMPARE(~from_id, name);
		QCOMPARE(PgSqlType::parseString(name).getTypeId(), type.getTypeId());
	}

	// Array and parsed forms of the new types
	QCOMPARE(PgSqlType::parseString("oid8[]").getDimension(), 1u);
	QCOMPARE(~PgSqlType::parseString("regdatabase"), QString("regdatabase"));
}

void PgSqlTypeTest::postgreSql19IsTheDefaultVersion()
{
	QCOMPARE(PgSqlVersions::DefaulVersion, QString("19.0"));
	QCOMPARE(PgSqlVersions::AllVersions.first(), PgSqlVersions::PgSqlVersion190);
	QCOMPARE(PgSqlVersions::AllVersions.last(), PgSqlVersions::MinimumVersion);
	QCOMPARE(PgSqlVersions::AllVersions.size(), 10);

	QCOMPARE(PgSqlVersions::parseString("19.0", false), QString("19.0"));
	QCOMPARE(PgSqlVersions::parseString("18.0", false), QString("18.0"));

	// A major version unknown to the tool falls back to the default one
	QCOMPARE(PgSqlVersions::parseString("20.0", false), PgSqlVersions::DefaulVersion);
}

QTEST_MAIN(PgSqlTypeTest)
#include "pgsqltypetest.moc"
