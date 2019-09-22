/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
 *  Melvin Keskin
 *  Linus Jahn
 *
 * Source:
 *  https://github.com/qxmpp-project/qxmpp
 *
 * This file is a part of QXmpp library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 */

#include "QXmppRegistrationManager.h"

#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppDiscoveryManager.h"
#include "QXmppRegisterIq.h"
#include "QXmppStreamFeatures.h"
#include "QXmppUtils.h"

#include <QDomElement>

QStringList QXmppRegistrationManager::discoveryFeatures() const
{
	return QStringList() << ns_register;
}

/// Changes the user's password
///
/// \param newPassword The requested new password

void QXmppRegistrationManager::changePassword(const QString &newPassword)
{
	m_changePasswordIqId = QXmppUtils::generateStanzaHash(24);
	m_newPassword = newPassword;

	QXmppRegisterIq iq;
    iq.setType(QXmppIq::Set);
	iq.setTo(client()->configuration().domain());
	iq.setFrom(client()->configuration().jid());
	iq.setUsername(QXmppUtils::jidToUser(client()->configuration().jid()));
	iq.setPassword(newPassword);
	iq.setId(m_changePasswordIqId);

    client()->sendPacket(iq);
}

/// Cancels an existing registration on the server.

QString QXmppRegistrationManager::deleteAccount()
{
    // there cannot be two parallel (un-)registration requests
//    Q_ASSERT(m_registrationId.isEmpty());
//    m_registrationId = QXmppUtils::generateStanzaHash(24);

    QXmppRegisterIq iq;
    iq.setType(QXmppIq::Set);
    iq.setFrom(client()->configuration().jid());
    iq.setRegisterType(QXmppRegisterIq::Remove);

    if (client()->sendPacket(iq))
        return iq.id();
    return {};
}

void QXmppRegistrationManager::setClient(QXmppClient *client)
{
	QXmppClientExtension::setClient(client);
	// get service discovery manager
	auto *disco = client->findExtension<QXmppDiscoveryManager>();
    if (disco != nullptr) {
		connect(disco, &QXmppDiscoveryManager::infoReceived,
		        this, &QXmppRegistrationManager::handleDiscoInfo);

		connect(client, &QXmppClient::disconnected, [this] () {
			setRegistrationSupported(false);
		});
	}
}

void QXmppRegistrationManager::handleDiscoInfo(const QXmppDiscoveryIq &iq)
{
	// check features of own server
	if (iq.from().isEmpty() || iq.from() == client()->configuration().domain()) {
		if (iq.features().contains(ns_register))
			setRegistrationSupported(true);
	}
}

bool QXmppRegistrationManager::handleStanza(const QDomElement &stanza)
{
    if (m_registrationEnabled && QXmppStreamFeatures::isStreamFeatures(stanza)) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        if (features.registerMode() == QXmppStreamFeatures::Disabled) {
            client()->handleConnectionError(QXmppClient::RegistrationUnsupportedError);
            warning("Disconnecting because registration was requested, but the"
                    " server does not support it.");
            client()->disconnectFromServer();
            return true;
        }

		if (!m_registrationFormToSend.isNull()) {
			sendRegistrationForm();
			return true;
		}

		// send request to register
		requestRegistrationForm();
		return true;
    }

    if (stanza.tagName() == "iq") {
		if (stanza.attribute("id") == m_registrationId) {
            QXmppIq iq;
            iq.parse(stanza);
            if (iq.type() == QXmppIq::Result) {
                emit registrationSucceeded();
                return true;
            }
			if (iq.type() == QXmppIq::Error) {
				emit registrationFailed(iq.error());
				return true;
			}
        } else if (stanza.attribute("id") == m_changePasswordIqId) {
            QXmppIq iq;
            iq.parse(stanza);

            if (iq.type() == QXmppIq::Result) {
                // Success
                client()->configuration().setPassword(m_newPassword);
                emit passwordChanged(m_newPassword);
                m_changePasswordIqId = QString();
                m_newPassword = QString();
                return true;
            }

            if (iq.type() == QXmppIq::Error) {
                // Error
                emit passwordChangeFailed(iq.error());
                warning(QString("Failed to change password: %1").arg(iq.error().text()));
                m_changePasswordIqId = QString();
                m_newPassword = QString();
                return true;
            }
		} else if (QXmppRegisterIq::isRegisterIq(stanza)) {
			QXmppRegisterIq iq;
			iq.parse(stanza);

			emit registrationFormReceived(iq);
		}
    }
    return false;
}

bool QXmppRegistrationManager::registrationSupported() const
{
	return m_registrationSupported;
}

void QXmppRegistrationManager::requestRegistrationForm(const QString &service)
{
	QXmppRegisterIq iq;
    iq.setType(QXmppIq::Get);
	iq.setTo(service);
	client()->sendPacket(iq);
}

void QXmppRegistrationManager::setRegistrationFormToSend(const QXmppDataForm &dataForm)
{
	m_registrationFormToSend = dataForm;
}

void QXmppRegistrationManager::setRegistrationSupported(bool registrationSupported)
{
	if (m_registrationSupported == registrationSupported) {
		m_registrationSupported = registrationSupported;
		emit registrationSupportedChanged();
	}
}

void QXmppRegistrationManager::sendRegistrationForm()
{
	m_registrationFormToSend.setType(QXmppDataForm::Submit);

	QXmppRegisterIq iq;
    iq.setType(QXmppIq::Set);
	iq.setTo(client()->configuration().domain());
	iq.setForm(m_registrationFormToSend);

	client()->sendPacket(iq);
	m_registrationId = iq.id();

	m_registrationFormToSend = QXmppDataForm();
}

/// Returns whether to only request the registration form and not to connect
/// with username/password.

bool QXmppRegistrationManager::registrationEnabled() const
{
    return m_registrationEnabled;
}

/// Sets whether to only request the registration form and not to connect with
/// username/password.
///
/// \param registrationEnabled true to register, false to connect normally.

void QXmppRegistrationManager::setRegistrationEnabled(bool enabled)
{
	m_registrationEnabled = enabled;
}
