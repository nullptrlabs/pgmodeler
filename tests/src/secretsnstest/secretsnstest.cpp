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
#include "secretsns.h"
#include "utilsns.h"

class SecretsNsTest: public QObject {
	Q_OBJECT

	private slots:
		void testEncryptedValueHasPrefixAndHidesSecret();
		void testRoundTrip();
		void testRoundTripWithSpecialAndMultiByteChars();
		void testRoundTripWithLongSecret();
		void testEmptySecretIsKeptEmpty();
		void testEncryptionIsDeterministic();
		void testDifferentSecretsGenerateDifferentValues();
		void testEncryptedValueIsSafeForXmlAndFieldSeparator();
		void testLegacyPlainTextIsReturnedAsIs();
		void testTamperedValueFailsToDecrypt();
		void testMalformedValueFailsToDecrypt();
		void testSetOwnerOnlyAccess();
};

void SecretsNsTest::testEncryptedValueHasPrefixAndHidesSecret()
{
	QString enc = SecretsNs::encrypt("my-secret-pwd");

	QVERIFY(SecretsNs::isEncrypted(enc));
	QVERIFY(enc.startsWith(SecretsNs::EncryptedPrefix));
	QVERIFY(!enc.contains("my-secret-pwd"));
	QVERIFY(!SecretsNs::isEncrypted("my-secret-pwd"));
}

void SecretsNsTest::testRoundTrip()
{
	auto [dec, ok] = SecretsNs::decrypt(SecretsNs::encrypt("postgres"));

	QVERIFY(ok);
	QCOMPARE(dec, "postgres");
}

void SecretsNsTest::testRoundTripWithSpecialAndMultiByteChars()
{
	for(const QString &pwd : { QString("p@ss w0rd/:&<>\"'"), QString("senha-çãé·•€"), QString("a"), QString(" ") })
	{
		auto [dec, ok] = SecretsNs::decrypt(SecretsNs::encrypt(pwd));

		QVERIFY2(ok, qPrintable(pwd));
		QCOMPARE(dec, pwd);
	}
}

void SecretsNsTest::testRoundTripWithLongSecret()
{
	// Longer than one keystream block (32 bytes) to cover the block counter
	QString pwd = QString("0123456789abcdef").repeated(20);
	auto [dec, ok] = SecretsNs::decrypt(SecretsNs::encrypt(pwd));

	QCOMPARE(dec, pwd);
	QVERIFY(ok);
}

void SecretsNsTest::testEmptySecretIsKeptEmpty()
{
	auto [dec, ok] = SecretsNs::decrypt("");

	QVERIFY(SecretsNs::encrypt("").isEmpty());
	QVERIFY(dec.isEmpty());
	QVERIFY(ok);
}

void SecretsNsTest::testEncryptionIsDeterministic()
{
	QCOMPARE(SecretsNs::encrypt("postgres"), SecretsNs::encrypt("postgres"));
}

void SecretsNsTest::testDifferentSecretsGenerateDifferentValues()
{
	QVERIFY(SecretsNs::encrypt("postgres") != SecretsNs::encrypt("postgres1"));
	QVERIFY(SecretsNs::encrypt("postgres") != SecretsNs::encrypt("Postgres"));
}

void SecretsNsTest::testEncryptedValueIsSafeForXmlAndFieldSeparator()
{
	QString enc = SecretsNs::encrypt(QString("p@ss/word?&<>\"").repeated(5));

	for(const QChar &chr : QString("&<>\"'") + UtilsNs::DataSeparator)
		QVERIFY2(!enc.contains(chr), qPrintable(QString(chr)));
}

void SecretsNsTest::testLegacyPlainTextIsReturnedAsIs()
{
	auto [dec, ok] = SecretsNs::decrypt("postgres");

	QCOMPARE(dec, "postgres");
	QVERIFY(ok);
}

void SecretsNsTest::testTamperedValueFailsToDecrypt()
{
	QString enc = SecretsNs::encrypt("postgres");

	// Changing a char in the encoded payload
	int idx = SecretsNs::EncryptedPrefix.size() + 30;
	enc[idx] = enc[idx] == 'A' ? 'B' : 'A';

	auto [dec, ok] = SecretsNs::decrypt(enc);

	QVERIFY(dec.isEmpty());
	QVERIFY(!ok);
}

void SecretsNsTest::testMalformedValueFailsToDecrypt()
{
	for(const QString &value : { SecretsNs::EncryptedPrefix,
															 SecretsNs::EncryptedPrefix + "###",
															 SecretsNs::EncryptedPrefix + "QUJD" })
	{
		auto [dec, ok] = SecretsNs::decrypt(value);

		QVERIFY2(dec.isEmpty(), qPrintable(value));
		QVERIFY2(!ok, qPrintable(value));
	}
}

void SecretsNsTest::testSetOwnerOnlyAccess()
{
	QTemporaryFile file;

	QVERIFY(file.open());
	QVERIFY(UtilsNs::setOwnerOnlyAccess(file.fileName()));

	// A non-existent file isn't an error
	QVERIFY(UtilsNs::setOwnerOnlyAccess(file.fileName() + ".missing"));

	#ifndef Q_OS_WIN
		QFileDevice::Permissions perms = QFile::permissions(file.fileName());

		QVERIFY(perms.testFlags(QFileDevice::ReadOwner | QFileDevice::WriteOwner));
		QVERIFY(!(perms & (QFileDevice::ReadGroup | QFileDevice::WriteGroup |
											 QFileDevice::ReadOther | QFileDevice::WriteOther)));
	#endif
}

QTEST_MAIN(SecretsNsTest)
#include "secretsnstest.moc"
