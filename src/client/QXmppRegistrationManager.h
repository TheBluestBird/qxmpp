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

#ifndef QXMPPREGISTRATIONMANAGER_H
#define QXMPPREGISTRATIONMANAGER_H

#include "QXmppClientExtension.h"
class QXmppRegisterIq;

class QXmppRegistrationManager : public QXmppClientExtension
{
    Q_OBJECT
    Q_PROPERTY(bool supported READ registrationSupported
                              NOTIFY registrationSupportedChanged)

public:
    QXmppRegistrationManager() = default;

    QStringList discoveryFeatures() const override;

    void changePassword(const QString &newPassword);

    bool registrationSupported() const;

	void requestRegistrationForm(const QString &service = "");
	void setRegistrationFormToSend(const QXmppDataForm &dataForm);

    bool registrationEnabled() const;
    void setRegistrationEnabled(bool enabled);

signals:
    void registrationSupportedChanged();

    void passwordChanged(const QString &newPassword);
    void passwordChangeFailed(const QXmppStanza::Error &error);

    void registrationFormReceived(const QXmppRegisterIq &iq);
    void registrationSucceeded();

    /// Emitted when the registration failed
    ///
    /// \note The errors depend on the server.
    ///       Known errors are:
    ///       \li type=Cancel and condition=Conflict:
    ///           username already exists
    ///       \li type=Modify and condition=NotAcceptable:
    ///           required information was missing
    ///           or password too weak
    ///       \li type=Cancel and condition=NotAllowed:
    ///           CAPTCHA verification failed
    ///
    /// \param error error of the failed registration.
    ///
    void registrationFailed(const QXmppStanza::Error &error);

protected:
    void setClient(QXmppClient *client) override;

private slots:
    void handleDiscoInfo(const QXmppDiscoveryIq &iq);

private:
    bool handleStanza(const QDomElement &stanza) override;
    void setRegistrationSupported(bool registrationSupported);
	void sendRegistrationForm();

    bool m_registrationEnabled = false;
    bool m_registrationSupported = false;

    // caching
    QString m_changePasswordIqId;
    QString m_newPassword;

    QString m_registrationId;

	QXmppDataForm m_registrationFormToSend;
};

#endif // QXMPPREGISTRATIONMANAGER_H
