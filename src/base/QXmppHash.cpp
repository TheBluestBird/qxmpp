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

#include "QXmppHash.h"

#include <QSharedData>

class QXmppHashPrivate : public QSharedData
{
public:
    QXmppHashPrivate() = default;
    QXmppHashPrivate(const QXmppHashPrivate &other) = default;
    ~QXmppHashPrivate() = default;

    QByteArray hash;
};

QXmppHash::QXmppHash()
{
    d = new QXmppHashPrivate;
}

QXmppHash::QXmppHash(const QXmppHash &other) = default;

QXmppHash::~QXmppHash() = default;
