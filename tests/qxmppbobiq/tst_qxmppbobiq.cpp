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

#include <QMimeType>
#include <QObject>

#include "QXmppBobContentId.h"
#include "QXmppBobIq.h"
#include "util.h"

class tst_QXmppBobIq : public QObject
{
    Q_OBJECT

private slots:
    void testBasic();
};

void tst_QXmppBobIq::testBasic()
{
    const QByteArray xml(
        "<iq id=\"get-data-1\" "
                "to=\"ladymacbeth@shakespeare.lit/castle\" "
                "from=\"doctor@shakespeare.lit/pda\" "
                "type=\"get\">"
            "<data xmlns=\"urn:xmpp:bob\" cid=\"sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org\"></data>"
        "</iq>"
    );

    QXmppBobIq iq;
    parsePacket(iq, xml);
    QCOMPARE(iq.from(), QStringLiteral("doctor@shakespeare.lit/pda"));
    QCOMPARE(iq.id(), QStringLiteral("get-data-1"));
    QCOMPARE(iq.to(), QStringLiteral("ladymacbeth@shakespeare.lit/castle"));
    QCOMPARE(iq.type(), QXmppIq::Get);
    QCOMPARE(iq.cid().toContentId(), QStringLiteral("sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org"));
    QCOMPARE(iq.contentType(), QMimeType());
    QCOMPARE(iq.data(), QByteArray());
    QCOMPARE(iq.maxAge(), 0);
    serializePacket(iq, xml);

    iq = QXmppBobIq();
    iq.setFrom(QStringLiteral("doctor@shakespeare.lit/pda"));
    iq.setId(QStringLiteral("get-data-1"));
    iq.setTo(QStringLiteral("ladymacbeth@shakespeare.lit/castle"));
    iq.setType(QXmppIq::Get);
    iq.setCid(QXmppBobContentId::fromContentId(QStringLiteral("sha1+8f35fef110ffc5df08d579a50083ff9308fb6242@bob.xmpp.org")));
    serializePacket(iq, xml);
}

QTEST_MAIN(tst_QXmppBobIq)
#include "tst_qxmppbobiq.moc"
