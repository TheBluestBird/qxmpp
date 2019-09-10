/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Author:
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

#include "QXmppBobContentId.h"

#include <QChar>
#include <QCryptographicHash>
#include <QString>

//class QXmppBobContentIdPrivate : QSharedData
class QXmppBobContentIdPrivate
{
public:
    QCryptographicHash::Algorithm hashAlgo;
    QByteArray hash;
};

QXmppBobContentId QXmppBobContentId::fromCidUrl(const QString &input)
{
    if (input.startsWith(QStringLiteral("cid:")))
        return fromContenId(input.mid(4));
    return {};
}

QXmppBobContentId QXmppBobContentId::fromContenId(const QString &input)
{
    if (!input.endsWith(QStringLiteral("@bob.xmpp.org")))
        return {};

    // remove '@bob.xmpp.org'
    QString hashAndAlgoStr = input.left(input.size() - 13);
    // get size of hash algo id
    int hashAlgoLeft = 0;
    for (int i = 0; i < hashAndAlgoStr.size() && hashAndAlgoStr.at(i) != "+"; i++)
        hashAlgoLeft++;

    QXmppBobContentId cid;

    QString hashAlgo = hashAndAlgoStr.left(hashAlgoLeft);
    if (hashAlgo == QStringLiteral("sha1"))
        cid.setAlgorithm(QCryptographicHash::Sha1);
    else if (hashAlgo == QStringLiteral("sha256"))
        cid.setAlgorithm(QCryptographicHash::Sha256);
    else if (hashAlgo == QStringLiteral("sha3-256"))
        cid.setAlgorithm(QCryptographicHash::Sha3_256);
    else
        return {};

    cid.setHash(QByteArray::fromHex(hashAndAlgoStr.mid(hashAlgoLeft).toUtf8()));
    return cid;
}

QXmppBobContentId::QXmppBobContentId()
    : d(new QXmppBobContentIdPrivate)
{
}

QXmppBobContentId::~QXmppBobContentId()
{
    delete d;
}

QString QXmppBobContentId::toContentId() const
{
    QString output;
    switch (d->hashAlgo) {
    case QCryptographicHash::Sha1:
        output += QStringLiteral("sha1");
        break;
    case QCryptographicHash::Sha256:
        output += QStringLiteral("sha256");
        break;
    case QCryptographicHash::Sha3_256:
        output += QStringLiteral("sha3-256");
        break;
    default:
        return output;
    };

    output += QStringLiteral("+");



    return output;
}

QString QXmppBobContentId::toCidUrl() const
{
    return QStringLiteral("cid:") + toContentId();
}

/*
QXmppBobContentId::QXmppBobContentId(const QXmppBobContentId &cid)
    : d(cid.d)
{
}

QXmppBobContentId &QXmppBobContentId::operator=(const QXmppBobContentId &other)
{
    d = other.d;
    return *this;
}
*/

QByteArray QXmppBobContentId::hash() const
{
    return d->hash;
}

void QXmppBobContentId::setHash(const QByteArray &hash)
{
    d->hash = hash;
}

QCryptographicHash::Algorithm QXmppBobContentId::algorithm() const
{
    return d->hashAlgo;
}

void QXmppBobContentId::setAlgorithm(QCryptographicHash::Algorithm algo)
{
    d->hashAlgo = algo;
}
