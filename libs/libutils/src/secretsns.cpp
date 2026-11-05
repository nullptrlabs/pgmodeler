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

#include "secretsns.h"
#include <QByteArray>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QStandardPaths>
#include <QSysInfo>

namespace SecretsNs {
	namespace {
		constexpr int NonceLen = 16;

		QByteArray hmac(const QByteArray &data, const QByteArray &key)
		{
			return QMessageAuthenticationCode::hash(data, key, QCryptographicHash::Sha256);
		}

		/* The master key is bound to the machine id and to the home directory of the
		 * current user. Both are computed once per process. */
		const QByteArray &masterKey()
		{
			static const QByteArray key = hmac(QSysInfo::machineUniqueId() + '\0' +
																				 QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toUtf8(),
																				 "pgmodeler-secrets-key-v1");
			return key;
		}

		const QByteArray &encKey()
		{
			static const QByteArray key = hmac("enc", masterKey());
			return key;
		}

		const QByteArray &macKey()
		{
			static const QByteArray key = hmac("mac", masterKey());
			return key;
		}

		//! \brief The nonce is a MAC of the plain data (synthetic IV), so it also authenticates the result
		QByteArray makeNonce(const QByteArray &plain)
		{
			return hmac(plain, macKey()).left(NonceLen);
		}

		//! \brief XORs the data with a keystream made of HMAC(enc_key, nonce | block counter)
		QByteArray applyKeystream(const QByteArray &data, const QByteArray &nonce)
		{
			QByteArray result = data, block;
			qsizetype pos = 0;
			quint32 counter = 0;

			while(pos < result.size())
			{
				QByteArray ctr_bytes;

				ctr_bytes.append(static_cast<char>((counter >> 24) & 0xFF));
				ctr_bytes.append(static_cast<char>((counter >> 16) & 0xFF));
				ctr_bytes.append(static_cast<char>((counter >> 8) & 0xFF));
				ctr_bytes.append(static_cast<char>(counter & 0xFF));

				block = hmac(nonce + ctr_bytes, encKey());

				for(qsizetype i = 0; i < block.size() && pos < result.size(); i++, pos++)
					result[pos] = static_cast<char>(result[pos] ^ block[i]);

				counter++;
			}

			return result;
		}

		bool equalBytes(const QByteArray &a, const QByteArray &b)
		{
			if(a.size() != b.size())
				return false;

			unsigned char diff = 0;

			for(qsizetype i = 0; i < a.size(); i++)
				diff |= static_cast<unsigned char>(a[i] ^ b[i]);

			return diff == 0;
		}
	}

	bool isEncrypted(const QString &value)
	{
		return value.startsWith(EncryptedPrefix);
	}

	QString encrypt(const QString &secret)
	{
		if(secret.isEmpty())
			return secret;

		QByteArray plain = secret.toUtf8(),
				nonce = makeNonce(plain);

		return EncryptedPrefix + QString::fromLatin1((nonce + applyKeystream(plain, nonce)).toBase64());
	}

	std::pair<QString, bool> decrypt(const QString &value)
	{
		if(!isEncrypted(value))
			return { value, true };

		auto decoded = QByteArray::fromBase64Encoding(value.mid(EncryptedPrefix.size()).toLatin1(),
																									QByteArray::AbortOnBase64DecodingErrors);

		if(decoded && decoded.decoded.size() > NonceLen)
		{
			QByteArray nonce = decoded.decoded.left(NonceLen),
					plain = applyKeystream(decoded.decoded.mid(NonceLen), nonce);

			if(equalBytes(makeNonce(plain), nonce))
				return { QString::fromUtf8(plain), true };
		}

		return { "", false };
	}
}
