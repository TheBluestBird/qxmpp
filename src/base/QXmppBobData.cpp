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

#include "QXmppBobData.h"

#include <QSharedData>
#include <QMimeType>

class QXmppBobDataPrivate : public QSharedData
{
public:
    QXmppBobContentId cid;
    unsigned int maxAge = 0;
    QMimeType contentType;
    QByteArray data;
};

QXmppBobData::QXmppBobData()
{

}


QXmppBobContentId QXmppBobIq::cid() const
{
    return d->cid;
}

void QXmppBobIq::setCid(const QXmppBobContentId &cid)
{
    d->cid = cid;
}

unsigned int QXmppBobIq::maxAge() const
{
    return d->maxAge;
}

void QXmppBobIq::setMaxAge(unsigned int maxAge)
{
    d->maxAge = maxAge;
}

QMimeType QXmppBobIq::contentType() const
{
    return d->contentType;
}

void QXmppBobIq::setContentType(const QMimeType &contentType)
{
    d->contentType = contentType;
}

QByteArray QXmppBobIq::data() const
{
    return d->data;
}

void QXmppBobIq::setData(const QByteArray &data)
{
    d->data = data;
}

