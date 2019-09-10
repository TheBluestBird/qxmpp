/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Authors:
 *  Jeremy Lainé
 *  Manjeet Dahiya
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

#include <QObject>

#include "QXmppBobContentId.h"
#include "util.h"

class tst_QXmppBobContentId : public QObject
{
    Q_OBJECT

private slots:
    void testBasic();
};

void tst_QXmppBobContentId::testBasic()
{
    qDebug() << "Moinsen";
    QXmppBobContentId cid = QXmppBobContentId::fromCidUrl(
        QStringLiteral("cid:sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"));

    qDebug() << "Schmoinsen";
    QCOMPARE(cid.algorithm(), QCryptographicHash::Sha1);
    QCOMPARE(cid.hash().toHex(), QByteArrayLiteral("8f35fef110ffc5df08d579a50083ff9308fb6242"));
    qDebug() << "feddig";
}

QTEST_MAIN(tst_QXmppBobContentId)
#include "tst_qxmppbobcontentid.moc"
