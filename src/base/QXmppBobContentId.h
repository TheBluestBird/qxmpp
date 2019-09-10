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

#ifndef QXMPPBOBCONTENTID_H
#define QXMPPBOBCONTENTID_H

//#include <QSharedDataPointer>
#include <QCryptographicHash>

class QXmppBobContentIdPrivate;

class QXmppBobContentId
{
public:
    static QXmppBobContentId fromCidUrl(const QString &input);
    static QXmppBobContentId fromContenId(const QString &input);

    QXmppBobContentId();
//    QXmppBobContentId(const QXmppBobContentId &cid);
    ~QXmppBobContentId();

//    QXmppBobContentId& operator=(const QXmppBobContentId &other);

    QString toContentId() const;
    QString toCidUrl() const;

    QByteArray hash() const;
    void setHash(const QByteArray &hash);

    QCryptographicHash::Algorithm algorithm() const;
    void setAlgorithm(QCryptographicHash::Algorithm algo);

private:
//    QSharedDataPointer<QXmppBobContentIdPrivate> d;
    QXmppBobContentIdPrivate *d;
};

#endif // QXMPPBOBCONTENTID_H
