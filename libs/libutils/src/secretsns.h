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

/**
\ingroup libutils
\namespace SecretsNs
\brief Obfuscates secrets (e.g., passwords) stored in the user's configuration files.

The purpose is to keep the secrets from being read by someone who just opens the file, e.g. another
user of the same machine or a copy of the file outside of it. The key is bound to the machine and the
user's home directory, so it's NOT a protection against someone that already controls the user's account.

An encrypted secret has the form `pgm-enc1:<base64>`. Values without that prefix are considered legacy
plain text and are returned untouched by decrypt(), which keeps old configuration files readable.
*/

#ifndef SECRETS_NS_H
#define SECRETS_NS_H

#include "utilsglobal.h"
#include <QString>
#include <utility>

namespace SecretsNs {
	//! \brief Prefix that identifies (and versions) an encrypted secret
	inline const QString EncryptedPrefix("pgm-enc1:");

	//! \brief Returns true when the value has the form of an encrypted secret
	__libutils bool isEncrypted(const QString &value);

	/*! \brief Returns the encrypted form of the secret. Empty secrets are returned as they are.
	 * The result is deterministic: the same secret always generates the same output on a given
	 * machine/user, so encrypted values can be compared to detect changes. */
	__libutils QString encrypt(const QString &secret);

	/*! \brief Returns a pair with the secret contained in the value and the result of the operation.
	 * Values that aren't encrypted (legacy plain text) are returned as they are with a true result.
	 * If an encrypted value can't be decrypted (e.g., it was created on another machine or was tampered)
	 * the pair is an empty string and false. */
	__libutils std::pair<QString, bool> decrypt(const QString &value);
}

#endif
