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

#include <QSharedData>
#include <QString>

class QXmppBobContentIdPrivate : public QSharedData
{
public:
    QCryptographicHash::Algorithm hashAlgo;
    QByteArray hash;
};

QXmppBobContentId QXmppBobContentId::fromCidUrl(const QString &input)
{
    if (input.startsWith(QStringLiteral("cid:")))
        return fromContentId(input.mid(4));
    return {};
}

QXmppBobContentId QXmppBobContentId::fromContentId(const QString &input)
{
    if (!input.endsWith(QStringLiteral("@bob.xmpp.org")))
        return {};

    // remove '@bob.xmpp.org'
    QString hashAndAlgoStr = input.left(input.size() - 13);
    // get size of hash algo id
    QStringList algoAndHash = hashAndAlgoStr.split(QStringLiteral("+"));
    if (algoAndHash.size() != 2)
        return {};

    QXmppBobContentId cid;

    if (algoAndHash.first() == QStringLiteral("sha1"))
        cid.setAlgorithm(QCryptographicHash::Sha1);
    else if (algoAndHash.first() == QStringLiteral("sha256"))
        cid.setAlgorithm(QCryptographicHash::Sha256);
    else if (algoAndHash.first() == QStringLiteral("sha3-256"))
        cid.setAlgorithm(QCryptographicHash::Sha3_256);
    else
        return {};

    cid.setHash(QByteArray::fromHex(algoAndHash.last().toUtf8()));
    return cid;
}

QXmppBobContentId::QXmppBobContentId()
    : d(new QXmppBobContentIdPrivate)
{
}

bool QXmppBobContentId::operator==(const QXmppBobContentId &other) const
{
    return d == other.d;
}

QXmppBobContentId::~QXmppBobContentId() = default;

QXmppBobContentId::QXmppBobContentId(const QXmppBobContentId &cid) = default;

QXmppBobContentId &QXmppBobContentId::operator=(const QXmppBobContentId &other) = default;

QString QXmppBobContentId::toContentId() const
{
    if (d->hash.isEmpty())
        return {};

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
        return {};
    };

    output += QStringLiteral("+");
    output += d->hash.toHex();
    output += QStringLiteral("@bob.xmpp.org");
    return output;
}

QString QXmppBobContentId::toCidUrl() const
{
    if (d->hash.isEmpty())
        return {};
    return QStringLiteral("cid:") + toContentId();
}

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
