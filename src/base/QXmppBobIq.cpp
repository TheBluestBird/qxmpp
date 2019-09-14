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

#include "QXmppBobIq.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QMimeDatabase>
#include <QSharedData>

class QXmppBobIqPrivate : public QSharedData
{
public:
    QXmppBobContentId cid;
    unsigned int maxAge = 0;
    QMimeType contentType;
    QByteArray data;
};

QXmppBobIq::QXmppBobIq()
    : d(new QXmppBobIqPrivate)
{
}

QXmppBobIq::QXmppBobIq(const QXmppBobIq &other) = default;

QXmppBobIq::~QXmppBobIq() = default;

QXmppBobIq &QXmppBobIq::operator=(const QXmppBobIq &other) = default;

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

bool QXmppBobIq::isBobIq(const QDomElement &element)
{
    return element.firstChildElement("data").namespaceURI() == ns_bob;
}

void QXmppBobIq::parseElementFromChild(const QDomElement &element)
{
    QDomElement data = element.firstChildElement("data");
    if (data.isNull() || data.namespaceURI() != ns_bob)
        return;

    d->cid = QXmppBobContentId::fromContentId(data.attribute("cid"));
    d->maxAge = data.attribute("max-age").toUInt();
    d->contentType = QMimeDatabase().mimeTypeForName(data.attribute("type"));
    d->data = QByteArray::fromBase64(data.text().toUtf8());
}

void QXmppBobIq::toXmlElementFromChild(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("data");
    writer->writeAttribute("xmlns", ns_bob);
    helperToXmlAddAttribute(writer, "cid", d->cid.toContentId());
    if (d->maxAge)
        helperToXmlAddAttribute(writer, "max-age", QString::number(d->maxAge));
    helperToXmlAddAttribute(writer, "type", d->contentType.name());
    writer->writeCharacters(d->data.toBase64());
    writer->writeEndElement();
}
