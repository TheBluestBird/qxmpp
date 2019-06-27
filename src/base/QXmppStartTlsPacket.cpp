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

#include "QXmppStartTlsPacket.h"

#include "QXmppConstants_p.h"

#include <QDomElement>
#include <QXmlStreamWriter>

QXmppStartTlsPacket::QXmppStartTlsPacket()
{
}

void QXmppStartTlsPacket::toXml(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("starttls");
    writer->writeAttribute("xmlns", ns_tls);
    writer->writeEndElement();
}

bool QXmppStartTlsPacket::isStartTlsPacket(const QDomElement &element)
{
    return element.tagName() == "starttls" && element.namespaceURI() == ns_tls;
}
