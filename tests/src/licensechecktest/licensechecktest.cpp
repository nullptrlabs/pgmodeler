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
#include <memory>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include "licensecheck/licensecheckns.h"
#include "licensecheck/licensekeydata.h"

/*! \brief Tests the license signature verification and the masked embedded public key.
 *  The signing side (server) is simulated with a throwaway RSA key pair, so no real
 *  license, credential or private key is needed. */
class LicenseCheckTest: public QObject {
	Q_OBJECT

	private:
		using KeyPtr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

		KeyPtr genKey() { return KeyPtr(EVP_RSA_gen(2048), EVP_PKEY_free); }

		//! \brief Returns the DER (SubjectPublicKeyInfo) of the public part of the key
		QByteArray publicDer(EVP_PKEY *key)
		{
			unsigned char *buf = nullptr;
			int len = i2d_PUBKEY(key, &buf);
			QByteArray der(reinterpret_cast<const char *>(buf), len);
			OPENSSL_free(buf);
			return der;
		}

		//! \brief Signs the payload the same way the server does: RSA-SHA256, signature in base64
		QString sign(EVP_PKEY *key, const QString &payload)
		{
			QByteArray data = payload.toUtf8();
			size_t sig_len = 0;
			std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);

			EVP_DigestSignInit(ctx.get(), nullptr, EVP_sha256(), nullptr, key);
			EVP_DigestSign(ctx.get(), nullptr, &sig_len, reinterpret_cast<const unsigned char *>(data.constData()), data.size());

			QByteArray sig(sig_len, 0);
			EVP_DigestSign(ctx.get(), reinterpret_cast<unsigned char *>(sig.data()), &sig_len,
										 reinterpret_cast<const unsigned char *>(data.constData()), data.size());
			sig.resize(sig_len);
			return QString::fromLatin1(sig.toBase64());
		}

	private slots:
		void embeddedKeyIsARsa2048PublicKey();
		void validSignatureIsAccepted();
		void alteredPayloadIsRejected();
		void alteredSignatureIsRejected();
		void signatureFromAnotherKeyIsRejected();
		void emptyOrInvalidInputsAreRejected();
		void foreignSignatureIsRejectedByTheEmbeddedKey();
		void noncelessOrEmptyNonceNeverValidates();
};

void LicenseCheckTest::embeddedKeyIsARsa2048PublicKey()
{
	unsigned char key[LicenseKeyData::KeySize];
	LicenseKeyData::unmask(key);

	const unsigned char *ptr = key;
	KeyPtr pub(d2i_PUBKEY(nullptr, &ptr, LicenseKeyData::KeySize), EVP_PKEY_free);

	QVERIFY(pub != nullptr);
	QCOMPARE(EVP_PKEY_bits(pub.get()), 2048);

	// The DER must be re-encoded identically, i.e., it is well formed with no trailing bytes
	QCOMPARE(publicDer(pub.get()), QByteArray(reinterpret_cast<const char *>(key), LicenseKeyData::KeySize));

	// The masked array must not be the plain key
	QVERIFY(memcmp(key, LicenseKeyData::Masked, LicenseKeyData::KeySize) != 0);
}

void LicenseCheckTest::validSignatureIsAccepted()
{
	KeyPtr key = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==";

	QVERIFY(LicenseCheckNs::validateSignatureWithKey(payload, sign(key.get(), payload), publicDer(key.get())));
}

void LicenseCheckTest::alteredPayloadIsRejected()
{
	KeyPtr key = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==",
			signature = sign(key.get(), payload);

	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload + "A", signature, publicDer(key.get())));
}

void LicenseCheckTest::alteredSignatureIsRejected()
{
	KeyPtr key = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==";
	QByteArray sig = QByteArray::fromBase64(sign(key.get(), payload).toLatin1());

	sig[10] = sig[10] ^ 0x01;
	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, QString::fromLatin1(sig.toBase64()), publicDer(key.get())));
}

void LicenseCheckTest::signatureFromAnotherKeyIsRejected()
{
	KeyPtr key = genKey(), other = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==";

	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, sign(other.get(), payload), publicDer(key.get())));
}

void LicenseCheckTest::emptyOrInvalidInputsAreRejected()
{
	KeyPtr key = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==",
			signature = sign(key.get(), payload);
	QByteArray der = publicDer(key.get());

	QVERIFY(!LicenseCheckNs::validateSignatureWithKey("", signature, der));
	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, "", der));
	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, signature, QByteArray()));
	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, signature, QByteArray("not a der key")));

	// A truncated key must not crash nor validate
	QVERIFY(!LicenseCheckNs::validateSignatureWithKey(payload, signature, der.left(der.size() / 2)));
}

void LicenseCheckTest::foreignSignatureIsRejectedByTheEmbeddedKey()
{
	KeyPtr key = genKey();
	QString payload = "eyJlbWFpbCI6InRlc3RAZXhhbXBsZS5jb20ifQ==";

	/* Only the real server holds the private key of the embedded one, so anything
	 * signed by another key (or garbage) must fail through the unmasking path too */
	QVERIFY(!LicenseCheckNs::validateSignature(payload, sign(key.get(), payload)));
	QVERIFY(!LicenseCheckNs::validateSignature(payload, "AAAA"));
	QVERIFY(!LicenseCheckNs::validateSignature("", ""));
}

void LicenseCheckTest::noncelessOrEmptyNonceNeverValidates()
{
	KeyPtr key = genKey();
	QString payload = QString::fromLatin1(QByteArray("{\"nonce\":\"abc\"}").toBase64()),
			response = QString("{\"payload\":\"%1\",\"signature\":\"%2\"}").arg(payload, sign(key.get(), payload));

	// An empty expected nonce is never accepted, whatever the response is
	QVERIFY(!LicenseCheckNs::validateServerNonce(response.toUtf8(), ""));

	// A response not signed by the embedded key is rejected even if the nonce matches
	QVERIFY(!LicenseCheckNs::validateServerNonce(response.toUtf8(), "abc"));
}

QTEST_MAIN(LicenseCheckTest)
#include "licensechecktest.moc"
