/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Authors:
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

#include "QXmppAuthenticationManager.h"
#include "QXmppClient.h"
#include "QXmppConstants_p.h"
#include "QXmppSasl_p.h"
#include "QXmppNonSASLAuth.h"
#include "QXmppStreamFeatures.h"

#include <QDomElement>

QXmppAuthenticationManager::QXmppAuthenticationManager()
    : QXmppClientExtension(), m_saslClient(nullptr)
{
}

bool QXmppAuthenticationManager::handleStanza(const QDomElement &stanza)
{
    if (QXmppStreamFeatures::isStreamFeatures(stanza)) {
        QXmppStreamFeatures features;
        features.parse(stanza);

        const bool nonSaslAvailable = features.nonSaslAuthMode() != QXmppStreamFeatures::Disabled;
        const bool saslAvailable = !features.authMechanisms().isEmpty();
        if (saslAvailable && client()->configuration().useSASLAuthentication())
        {
            // supported and preferred SASL auth mechanisms
            const QString preferredMechanism = client()->configuration().saslAuthMechanism();
            QStringList supportedMechanisms = QXmppSaslClient::availableMechanisms();
            if (supportedMechanisms.contains(preferredMechanism)) {
                supportedMechanisms.removeAll(preferredMechanism);
                supportedMechanisms.prepend(preferredMechanism);
            }
            if (client()->configuration().facebookAppId().isEmpty() || client()->configuration().facebookAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-FACEBOOK-PLATFORM");
            if (client()->configuration().windowsLiveAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-MESSENGER-OAUTH2");
            if (client()->configuration().googleAccessToken().isEmpty())
                supportedMechanisms.removeAll("X-OAUTH2");

            // determine SASL Authentication mechanism to use
            QStringList commonMechanisms;
            QString usedMechanism;
            foreach (const QString &mechanism, supportedMechanisms) {
                if (features.authMechanisms().contains(mechanism))
                    commonMechanisms << mechanism;
            }
            if (commonMechanisms.isEmpty()) {
                warning("No supported SASL Authentication mechanism available");
                client()->disconnectFromServer();
                return true;
            } else {
                usedMechanism = commonMechanisms.first();
            }

            m_saslClient = QXmppSaslClient::create(usedMechanism, this);
            if (!m_saslClient) {
                warning("SASL mechanism negotiation failed");
                client()->disconnectFromServer();
                return true;
            }
            info(QString("SASL mechanism '%1' selected").arg(m_saslClient->mechanism()));
            m_saslClient->setHost(client()->configuration().domain());
            m_saslClient->setServiceType("xmpp");
            if (m_saslClient->mechanism() == "X-FACEBOOK-PLATFORM") {
                m_saslClient->setUsername(client()->configuration().facebookAppId());
                m_saslClient->setPassword(client()->configuration().facebookAccessToken());
            } else if (m_saslClient->mechanism() == "X-MESSENGER-OAUTH2") {
                m_saslClient->setPassword(client()->configuration().windowsLiveAccessToken());
            } else if (m_saslClient->mechanism() == "X-OAUTH2") {
                m_saslClient->setUsername(client()->configuration().user());
                m_saslClient->setPassword(client()->configuration().googleAccessToken());
            } else {
                m_saslClient->setUsername(client()->configuration().user());
                m_saslClient->setPassword(client()->configuration().password());
            }

            // send SASL auth request
            QByteArray response;
            if (!m_saslClient->respond(QByteArray(), response)) {
                warning("SASL initial response failed");
                client()->disconnectFromServer();
                return true;
            }
            client()->sendPacket(QXmppSaslAuth(m_saslClient->mechanism(), response));
            return true;
        } else if(nonSaslAvailable && client()->configuration().useNonSASLAuthentication()) {
            sendNonSASLAuthQuery();
            return true;
        }
    }

    if (stanza.namespaceURI() == ns_sasl) {
        if (!m_saslClient) {
            warning("SASL stanza received, but no mechanism selected");
            return true;
        }
        if (stanza.tagName() == "success") {
            debug("Authenticated");
            m_isAuthenticated = true;
            client()->sendStreamElement();
        } else if (stanza.tagName() == "challenge") {
            QXmppSaslChallenge challenge;
            challenge.parse(stanza);

            QByteArray response;
            if (m_saslClient->respond(challenge.value(), response)) {
                client()->sendPacket(QXmppSaslResponse(response));
            } else {
                warning("Could not respond to SASL challenge");
                client()->disconnectFromServer();
            }
        } else if (stanza.tagName() == "failure") {
            QXmppSaslFailure failure;
            failure.parse(stanza);

            // RFC3920 defines the error condition as "not-authorized", but
            // some broken servers use "bad-auth" instead. We tolerate this
            // by remapping the error to "not-authorized".
            if (failure.condition() == "not-authorized" || failure.condition() == "bad-auth")
                client()->setXmppStreamError(QXmppStanza::Error::NotAuthorized);
            else
                client()->setXmppStreamError(QXmppStanza::Error::UndefinedCondition);
            client()->handleConnectionError(QXmppClient::XmppStreamError);

            warning("Authentication failure");
            client()->disconnectFromServer();
        }
    } else if (stanza.namespaceURI() == ns_client && stanza.tagName() == "iq") {
        const QString type = stanza.attribute("type");

        // XEP-0078: Non-SASL Authentication
        if(stanza.attribute("id") == m_nonSASLAuthId && type == "result") {
            // successful Non-SASL Authentication
            debug("Authenticated (Non-SASL)");
            m_isAuthenticated = true;

            // xmpp connection made
            emit authenticated();
        } else if(QXmppNonSASLAuthIq::isNonSASLAuthIq(stanza)) {
            if (type == "result") {
                bool digest = !stanza.firstChildElement("query").
                     firstChildElement("digest").isNull();
                bool plain = !stanza.firstChildElement("query").
                     firstChildElement("password").isNull();
                bool plainText = false;

                if (plain && digest) {
                    if (client()->configuration().nonSASLAuthMechanism() ==
                       QXmppConfiguration::NonSASLDigest)
                        plainText = false;
                    else
                        plainText = true;
                } else if(plain) {
                    plainText = true;
                } else if(digest) {
                    plainText = false;
                } else {
                    warning("No supported Non-SASL Authentication mechanism available");
                    client()->disconnectFromServer();
                    return true;
                }
                sendNonSASLAuth(plainText);
            }
        }
    }
    return false;
}

void QXmppAuthenticationManager::handleStream(const QDomElement &element)
{
    if (m_streamId.isEmpty())
        m_streamId = element.attribute("id");

    // no version specified, signals XMPP Version < 1.0.
    // switch to old auth mechanism if enabled
    if (element.attribute("version").isEmpty() && client()->configuration().useNonSASLAuthentication()) {
        sendNonSASLAuthQuery();
    }
}

void QXmppAuthenticationManager::handleClientState(QXmppClient::State state)
{
    if (state == QXmppClient::State::ConnectingState) {
        // reset authentication step
        delete m_saslClient;
        m_saslClient = nullptr;
    }
}

void QXmppAuthenticationManager::setClient(QXmppClient *client)
{
    QXmppClientExtension::setClient(client);

    connect(client, &QXmppClient::streamReceived,
            this, &QXmppAuthenticationManager::handleStream);
    connect(client, &QXmppClient::stateChanged,
            this, &QXmppAuthenticationManager::handleClientState);
}

bool QXmppAuthenticationManager::isAuthenticated() const
{
    return m_isAuthenticated;
}

void QXmppAuthenticationManager::sendNonSASLAuth(bool plaintext)
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Set);
    authQuery.setUsername(client()->configuration().user());
    if (plaintext)
        authQuery.setPassword(client()->configuration().password());
    else
        authQuery.setDigest(m_streamId, client()->configuration().password());
    authQuery.setResource(client()->configuration().resource());
    m_nonSASLAuthId = authQuery.id();
    client()->sendPacket(authQuery);
}

void QXmppAuthenticationManager::sendNonSASLAuthQuery()
{
    QXmppNonSASLAuthIq authQuery;
    authQuery.setType(QXmppIq::Get);
    authQuery.setTo(client()->configuration().domain());
    // FIXME : why are we setting the username, XEP-0078 states we should
    // not attempt to guess the required fields?
    authQuery.setUsername(client()->configuration().user());
    client()->sendPacket(authQuery);
}
