/*
 * Copyright (C) 2008-2019 The QXmpp developers
 *
 * Authors:
 *  Manjeet Dahiya
 *  Jeremy Lainé
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

#include <QDateTime>
#include <QDomElement>
#include <QTextStream>
#include <QXmlStreamWriter>

#include "QXmppConstants_p.h"
#include "QXmppMessage.h"
#include "QXmppUtils.h"

static const QStringList CHAT_STATES = {
    QString(),
    QStringLiteral("active"),
    QStringLiteral("inactive"),
    QStringLiteral("gone"),
    QStringLiteral("composing"),
    QStringLiteral("paused")
};

static const QStringList MESSAGE_TYPES = {
    QStringLiteral("error"),
    QStringLiteral("normal"),
    QStringLiteral("chat"),
    QStringLiteral("groupchat"),
    QStringLiteral("headline")
};

static const QStringList MARKER_TYPES = {
    QString(),
    QStringLiteral("received"),
    QStringLiteral("displayed"),
    QStringLiteral("acknowledged")
};

static const QStringList HINT_TYPES = {
    QStringLiteral("no-permanent-store"),
    QStringLiteral("no-store"),
    QStringLiteral("no-copy"),
    QStringLiteral("store")
};

static const QStringList ENCRYPTION_NAMESPACES = {
    QString(),
    QString(),
    ns_otr,
    ns_legacy_openpgp,
    ns_ox,
    ns_omemo
};

static const QStringList ENCRYPTION_NAMES = {
    QString(),
    QString(),
    QStringLiteral("OTR"),
    QStringLiteral("Legacy OpenPGP"),
    QStringLiteral("OpenPGP for XMPP (OX)"),
    QStringLiteral("OMEMO")
};

enum StampType
{
    LegacyDelayedDelivery,  // XEP-0091: Legacy Delayed Delivery
    DelayedDelivery         // XEP-0203: Delayed Delivery
};

class QXmppMessagePrivate : public QSharedData
{
public:
    QXmppMessage::Type type = QXmppMessage::Normal;
    QDateTime stamp;
    StampType stampType = DelayedDelivery;
    QXmppMessage::State state = QXmppMessage::None;

    bool attentionRequested = false;
    QString body;
    QString subject;
    QString thread;

    // XEP-0071: XHTML-IM
    QString xhtml;

    // Request message receipt as per XEP-0184.
    QString receiptId;
    bool receiptRequested = false;

    // XEP-0249: Direct MUC Invitations
    QString mucInvitationJid;
    QString mucInvitationPassword;
    QString mucInvitationReason;

    // XEP-0333: Chat Markers
    bool markable = false;
    QXmppMessage::Marker marker = QXmppMessage::NoMarker;
    QString markedId;
    QString markedThread;

    // XEP-0280: Message Carbons
    bool privatemsg = false;

    // XEP-0066: Out of Band Data
    QString outOfBandUrl;

    // XEP-0308: Last Message Correction
    QString replaceId;

    // XEP-0334: Message Processing Hints
    quint8 hints;

    // XEP-0367: Message Attaching
    QString attachId;

    // XEP-0369: Mediated Information eXchange (MIX)
    QString mixUserJid;
    QString mixUserNick;

    // XEP-0380: Explicit Message Encryption
    QString encryptionMethod;
    QString encryptionName;

    // XEP-0382: Spoiler messages
    bool isSpoiler = false;
    QString spoilerHint;
};

/// Constructs a QXmppMessage.
///
/// \param from
/// \param to
/// \param body
/// \param thread

QXmppMessage::QXmppMessage(const QString& from, const QString& to, const
                           QString& body, const QString& thread)
    : QXmppStanza(from, to)
    , d(new QXmppMessagePrivate)
{
    d->type = Chat;
    d->stampType = DelayedDelivery;
    d->state = None;
    d->attentionRequested = false;
    d->body = body;
    d->thread = thread;
    d->receiptRequested = false;

    d->markable = false;
    d->marker = NoMarker;

    d->privatemsg = false;

    d->hints = 0;
}

/// Constructs a copy of \a other.

QXmppMessage::QXmppMessage(const QXmppMessage &other) = default;

QXmppMessage::~QXmppMessage() = default;

/// Assigns \a other to this message.

QXmppMessage& QXmppMessage::operator=(const QXmppMessage &other) = default;

/// Returns the message's body.

QString QXmppMessage::body() const
{
    return d->body;
}

/// Sets the message's body.
///
/// \param body

void QXmppMessage::setBody(const QString& body)
{
    d->body = body;
}

/// Returns true if the user's attention is requested, as defined
/// by XEP-0224: Attention.

bool QXmppMessage::isAttentionRequested() const
{
    return d->attentionRequested;
}

/// Sets whether the user's attention is requested, as defined
/// by XEP-0224: Attention.
///
/// \a param requested

void QXmppMessage::setAttentionRequested(bool requested)
{
    d->attentionRequested = requested;
}

/// Returns true if a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.

bool QXmppMessage::isReceiptRequested() const
{
    return d->receiptRequested;
}

/// Sets whether a delivery receipt is requested, as defined
/// by XEP-0184: Message Delivery Receipts.
///
/// \a param requested

void QXmppMessage::setReceiptRequested(bool requested)
{
    d->receiptRequested = requested;
    if (requested && id().isEmpty())
        generateAndSetNextId();
}

/// If this message is a delivery receipt, returns the ID of the
/// original message.

QString QXmppMessage::receiptId() const
{
    return d->receiptId;
}

/// Make this message a delivery receipt for the message with
/// the given \a id.

void QXmppMessage::setReceiptId(const QString &id)
{
    d->receiptId = id;
}

/// Returns the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationJid() const
{
    return d->mucInvitationJid;
}

/// Sets the JID for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationJid(const QString &jid)
{
    d->mucInvitationJid = jid;
}

/// Returns the password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationPassword() const
{
    return d->mucInvitationPassword;
}

/// Sets the \a password for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationPassword(const QString &password)
{
    d->mucInvitationPassword = password;
}

/// Returns the reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

QString QXmppMessage::mucInvitationReason() const
{
    return d->mucInvitationReason;
}

/// Sets the \a reason for a multi-user chat direct invitation as defined
/// by XEP-0249: Direct MUC Invitations.

void QXmppMessage::setMucInvitationReason(const QString &reason)
{
    d->mucInvitationReason = reason;
}

/// Returns the message's type.
///

QXmppMessage::Type QXmppMessage::type() const
{
    return d->type;
}

/// Sets the message's type.
///
/// \param type

void QXmppMessage::setType(QXmppMessage::Type type)
{
    d->type = type;
}

/// Returns the message's timestamp (if any).

QDateTime QXmppMessage::stamp() const
{
    return d->stamp;
}

/// Sets the message's timestamp.
///
/// \param stamp

void QXmppMessage::setStamp(const QDateTime &stamp)
{
    d->stamp = stamp;
}

/// Returns the message's chat state.
///

QXmppMessage::State QXmppMessage::state() const
{
    return d->state;
}

/// Sets the message's chat state.
///
/// \param state

void QXmppMessage::setState(QXmppMessage::State state)
{
    d->state = state;
}

/// Returns the message's subject.
///

QString QXmppMessage::subject() const
{
    return d->subject;
}

/// Sets the message's subject.
///
/// \param subject

void QXmppMessage::setSubject(const QString& subject)
{
    d->subject = subject;
}

/// Returns the message's thread.

QString QXmppMessage::thread() const
{
    return d->thread;
}

/// Sets the message's thread.
///
/// \param thread

void QXmppMessage::setThread(const QString& thread)
{
    d->thread = thread;
}

/// Returns the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

QString QXmppMessage::xhtml() const
{
    return d->xhtml;
}

/// Sets the message's XHTML body as defined by
/// XEP-0071: XHTML-IM.

void QXmppMessage::setXhtml(const QString &xhtml)
{
    d->xhtml = xhtml;
}

/// Returns true if a message is markable, as defined
/// XEP-0333: Chat Markers.

bool QXmppMessage::isMarkable() const
{
    return d->markable;
}

/// Sets if the message is markable, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkable(const bool markable)
{
    d->markable = markable;
}

/// Returns the message's marker id, as defined
/// XEP-0333: Chat Markers.

QString QXmppMessage::markedId() const
{
    return d->markedId;
}

/// Sets the message's marker id, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkerId(const QString &markerId)
{
    d->markedId = markerId;
}

/// Returns the message's marker thread, as defined
/// XEP-0333: Chat Markers.

QString QXmppMessage::markedThread() const
{
    return d->markedThread;
}

/// Sets the message's marked thread, as defined
/// XEP-0333: Chat Markers.

void QXmppMessage::setMarkedThread(const QString &markedThread)
{
    d->markedThread = markedThread;
}

/// Returns the message's marker, as defined
/// XEP-0333: Chat Markers.

QXmppMessage::Marker QXmppMessage::marker() const
{
    return d->marker;
}

/// Sets the message's marker, as defined
/// XEP-0333: Chat Markers

void QXmppMessage::setMarker(const Marker marker)
{
    d->marker = marker;
}

/// Returns if the message is marked with a <private> tag,
/// in which case it will not be forwarded to other resources
/// according to XEP-0280: Message Carbons.

bool QXmppMessage::isPrivate() const
{
    return d->privatemsg;
}

/// If true is passed, the message is marked with a <private> tag,
/// in which case it will not be forwarded to other resources
/// according to XEP-0280: Message Carbons.

void QXmppMessage::setPrivate(const bool priv)
{
    d->privatemsg = priv;
}

/// Indicates if the QXmppStanza is a stanza in the XMPP sense (i. e. a message,
/// iq or presence)

bool QXmppMessage::isXmppStanza() const
{
    return true;
}

/// Returns a possibly attached URL from XEP-0066: Out of Band Data

QString QXmppMessage::outOfBandUrl() const
{
    return d->outOfBandUrl;
}

/// Sets the attached URL for XEP-0066: Out of Band Data

void QXmppMessage::setOutOfBandUrl(const QString &url)
{
    d->outOfBandUrl = url;
}

/// Returns the message id to replace with this message as used in XEP-0308:
/// Last Message Correction. If the returned string is empty, this message is
/// not replacing another.

QString QXmppMessage::replaceId() const
{
    return d->replaceId;
}

/// Sets the message id to replace with this message as in XEP-0308: Last
/// Message Correction.

void QXmppMessage::setReplaceId(const QString &replaceId)
{
    d->replaceId = replaceId;
}

/// Returns true if the message contains the hint passed, as defined in
/// XEP-0334: Message Processing Hints

bool QXmppMessage::hasHint(const Hint hint) const
{
    return d->hints & hint;
}

/// Adds a hint to the message, as defined in XEP-0334: Message Processing
/// Hints

void QXmppMessage::addHint(const Hint hint)
{
    d->hints |= hint;
}

/// Removes a hint from the message, as defined in XEP-0334: Message Processing
/// Hints

void QXmppMessage::removeHint(const Hint hint)
{
    d->hints &= ~hint;
}

/// Removes all hints from the message, as defined in XEP-0334: Message
/// Processing Hints

void QXmppMessage::removeAllHints()
{
    d->hints = 0;
}

/// Returns the message id this message is linked/attached to. See XEP-0367:
/// Message Attaching for details.

QString QXmppMessage::attachId() const
{
    return d->attachId;
}

/// Sets the id of the attached message as in XEP-0367: Message Attaching. This
/// can be used for a "reply to" or "reaction" function.
///
/// The used message id depends on the message context, see the Business rules
/// section of the XEP for details about when to use which id.

void QXmppMessage::setAttachId(const QString &attachId)
{
    d->attachId = attachId;
}

/// Returns the actual JID of a MIX channel participant.

QString QXmppMessage::mixUserJid() const
{
    return d->mixUserJid;
}

/// Sets the actual JID of a MIX channel participant.

void QXmppMessage::setMixUserJid(const QString& mixUserJid)
{
    d->mixUserJid = mixUserJid;
}

/// Returns the MIX participant's nickname.

QString QXmppMessage::mixUserNick() const
{
    return d->mixUserNick;
}

/// Sets the MIX participant's nickname.

void QXmppMessage::setMixUserNick(const QString& mixUserNick)
{
    d->mixUserNick = mixUserNick;
}

/// Returns the encryption method this message is advertised to be encrypted
/// with.
///
/// \note QXmppMessage::NoEncryption does not necesserily mean that the message
/// is not encrypted; it may also be that the author of the message does not
/// support XEP-0380: Explicit Message Encryption.
///
/// \note If this returns QXmppMessage::UnknownEncryption, you can still get
/// the namespace of the encryption with \c encryptionMethodNs() and possibly
/// also a name with \c encryptionName().

QXmppMessage::EncryptionMethod QXmppMessage::encryptionMethod() const
{
    if (d->encryptionMethod.isEmpty())
        return QXmppMessage::NoEncryption;

    int index = ENCRYPTION_NAMESPACES.indexOf(d->encryptionMethod);
    if (index < 0)
        return QXmppMessage::UnknownEncryption;
    return static_cast<QXmppMessage::EncryptionMethod>(index);
}

/// Advertises that this message is encrypted with the given encryption method.
/// See XEP-0380: Explicit Message Encryption for details.

void QXmppMessage::setEncryptionMethod(QXmppMessage::EncryptionMethod method)
{
    d->encryptionMethod = ENCRYPTION_NAMESPACES.at(int(method));
}

/// Returns the namespace of the advertised encryption method via. XEP-0380:
/// Explicit Message Encryption.

QString QXmppMessage::encryptionMethodNs() const
{
    return d->encryptionMethod;
}

/// Sets the namespace of the encryption method this message advertises to be
/// encrypted with. See XEP-0380: Explicit Message Encryption for details.

void QXmppMessage::setEncryptionMethodNs(const QString &encryptionMethod)
{
    d->encryptionMethod = encryptionMethod;
}

/// Returns the associated name of the encryption method this message
/// advertises to be encrypted with. See XEP-0380: Explicit Message Encryption
/// for details.

QString QXmppMessage::encryptionName() const
{
    if (!d->encryptionName.isEmpty())
        return d->encryptionName;
    return ENCRYPTION_NAMES.at(int(encryptionMethod()));
}

/// Sets the name of the encryption method for XEP-0380: Explicit Message
/// Encryption.
///
/// \note This should only be used, if the encryption method is custom and is
/// not one of the methods listed in the XEP.

void QXmppMessage::setEncryptionName(const QString &encryptionName)
{
    d->encryptionName = encryptionName;
}

/// Returns true, if this is a spoiler message according to XEP-0382: Spoiler
/// messages. The spoiler hint however can still be empty.
///
/// A spoiler message's content should not be visible to the user by default.

bool QXmppMessage::isSpoiler() const
{
    return d->isSpoiler;
}

/// Sets whether this is a spoiler message as specified in XEP-0382: Spoiler
/// messages.
///
/// The content of spoiler messages will not be displayed by default to the
/// user. However, clients not supporting spoiler messages will still display
/// the content as usual.

void QXmppMessage::setIsSpoiler(bool isSpoiler)
{
    d->isSpoiler = isSpoiler;
}

/// Returns the spoiler hint as specified in XEP-0382: Spoiler messages.
///
/// The hint may be empty, even if isSpoiler is true.

QString QXmppMessage::spoilerHint() const
{
    return d->spoilerHint;
}

/// Sets a spoiler hint for XEP-0382: Spoiler messages. If the spoiler hint
/// is not empty, isSpoiler will be set to true.
///
/// A spoiler hint is optional for spoiler messages.
///
/// Keep in mind that the spoiler hint is not displayed at all by clients not
/// supporting spoiler messages.

void QXmppMessage::setSpoilerHint(const QString &spoilerHint)
{
    d->spoilerHint = spoilerHint;
    if (!spoilerHint.isEmpty())
        d->isSpoiler = true;
}

/// \cond
void QXmppMessage::parse(const QDomElement &element)
{
    QXmppStanza::parse(element);

    // message type
    int i = MESSAGE_TYPES.indexOf(element.attribute("type"));
    if (i > -1)
        d->type = static_cast<Type>(i);

    QXmppElementList extensions;
    QDomElement xElement = element.firstChildElement();
    while (!xElement.isNull()) {
        if (xElement.tagName() == "body") {
            d->body = xElement.text();
        } else if (xElement.tagName() == "subject") {
            d->subject = xElement.text();
        } else if (xElement.tagName() == "thread") {
            d->thread = xElement.text();
        } else if (xElement.tagName() == "x") {
            if (xElement.namespaceURI() == ns_legacy_delayed_delivery) {
                // if XEP-0203 exists, XEP-0091 has no need to parse because
                // XEP-0091 is no more standard protocol)
                if (d->stamp.isNull()) {
                    // XEP-0091: Legacy Delayed Delivery
                    d->stamp = QDateTime::fromString(xElement.attribute("stamp"),
                                                     "yyyyMMddThh:mm:ss");
                    d->stamp.setTimeSpec(Qt::UTC);
                    d->stampType = LegacyDelayedDelivery;
                }
            } else if (xElement.namespaceURI() == ns_conference) {
                // XEP-0249: Direct MUC Invitations
                d->mucInvitationJid = xElement.attribute("jid");
                d->mucInvitationPassword = xElement.attribute("password");
                d->mucInvitationReason = xElement.attribute("reason");
            } else if (xElement.namespaceURI() == ns_oob) {
                // XEP-0066: Out of Band Data
                d->outOfBandUrl = xElement.firstChildElement("url").text();
            } else {
                extensions << QXmppElement(xElement);
            }
        // XEP-0071: XHTML-IM
        } else if (xElement.tagName() == "html" && xElement.namespaceURI() == ns_xhtml_im) {
            QDomElement bodyElement = xElement.firstChildElement("body");
            if (!bodyElement.isNull() && bodyElement.namespaceURI() == ns_xhtml) {
                QTextStream stream(&d->xhtml, QIODevice::WriteOnly);
                bodyElement.save(stream, 0);

                d->xhtml = d->xhtml.mid(d->xhtml.indexOf('>') + 1);
                d->xhtml.replace(" xmlns=\"http://www.w3.org/1999/xhtml\"", "");
                d->xhtml.replace("</body>", "");
                d->xhtml = d->xhtml.trimmed();
            }
        // XEP-0085: Chat State Notifications
        } else if (xElement.namespaceURI() == ns_chat_states) {
            int i = CHAT_STATES.indexOf(xElement.tagName());
            if (i > 0)
                d->state = static_cast<QXmppMessage::State>(i);
        // XEP-0184: Message Delivery Receipts
        } else if (xElement.tagName() == "received" && xElement.namespaceURI() == ns_message_receipts) {
            d->receiptId = xElement.attribute("id");

            // compatibility with old-style XEP
            if (d->receiptId.isEmpty())
                d->receiptId = id();
        } else if (xElement.tagName() == "request" && xElement.namespaceURI() == ns_message_receipts) {
            d->receiptRequested = true;
        // XEP-0203: Delayed Delivery
        } else if (xElement.tagName() == "delay" && xElement.namespaceURI() == ns_delayed_delivery) {
            d->stamp = QXmppUtils::datetimeFromString(xElement.attribute("stamp"));
            d->stampType = DelayedDelivery;
        // XEP-0224: Attention
        } else if (xElement.tagName() == "attention" && xElement.namespaceURI() == ns_attention) {
            d->attentionRequested = true;
        // XEP-0280: Message Carbons
        } else if (xElement.tagName() == "private" && xElement.namespaceURI() == ns_carbons) {
            d->privatemsg = true;
        // XEP-0308: Last Message Correction
        } else if (xElement.tagName() == "replace" && xElement.namespaceURI() == ns_message_correct) {
            d->replaceId = xElement.attribute("id");
        // XEP-0333: Chat Markers
        } else if (xElement.namespaceURI() == ns_chat_markers) {
            if (xElement.tagName() == "markable") {
                d->markable = true;
            } else {
                int marker = MARKER_TYPES.indexOf(xElement.tagName());
                if (marker > 0) {
                    d->marker = static_cast<QXmppMessage::Marker>(marker);
                    d->markedId = xElement.attribute("id");
                    d->markedThread = xElement.attribute("thread");
                }
            }
        // XEP-0334: Message Processing Hints
        } else if (xElement.namespaceURI() == ns_message_processing_hints && HINT_TYPES.contains(xElement.tagName())) {
            addHint(Hint(1u << uint(HINT_TYPES.indexOf(xElement.tagName()))));
        // XEP-0367: Message Attaching
        } else if (xElement.tagName() == "attach-to" && xElement.namespaceURI() == ns_message_attaching) {
            d->attachId = xElement.attribute("id");
        // XEP-0369: Mediated Information eXchange (MIX)
        } else if (xElement.tagName() == "mix" && xElement.namespaceURI() == ns_mix) {
            d->mixUserJid = xElement.firstChildElement("jid").text();
            d->mixUserNick = xElement.firstChildElement("nick").text();
        // XEP-0380: Explicit Message Encryption
        } else if (xElement.tagName() == "encryption" && xElement.namespaceURI() == ns_eme) {
            d->encryptionMethod = xElement.attribute("namespace");
            d->encryptionName = xElement.attribute("name");
        // XEP-0382: Spoiler messages
        } else if (xElement.tagName() == "spoiler" && xElement.namespaceURI() == ns_spoiler) {
            d->isSpoiler = true;
            d->spoilerHint = xElement.text();
        } else if (xElement.tagName() != "addresses" && xElement.tagName() != "error") {
            // other extensions
            extensions << QXmppElement(xElement);
        }
        xElement = xElement.nextSiblingElement();
    }
    setExtensions(extensions);
}

void QXmppMessage::toXml(QXmlStreamWriter *xmlWriter) const
{
    xmlWriter->writeStartElement("message");
    helperToXmlAddAttribute(xmlWriter, "xml:lang", lang());
    helperToXmlAddAttribute(xmlWriter, "id", id());
    helperToXmlAddAttribute(xmlWriter, "to", to());
    helperToXmlAddAttribute(xmlWriter, "from", from());
    helperToXmlAddAttribute(xmlWriter, "type", MESSAGE_TYPES.at(d->type));
    if (!d->subject.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "subject", d->subject);
    if (!d->body.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "body", d->body);
    if (!d->thread.isEmpty())
        helperToXmlAddTextElement(xmlWriter, "thread", d->thread);
    error().toXml(xmlWriter);

    // chat states
    if (d->state > None && d->state <= Paused) {
        xmlWriter->writeStartElement(CHAT_STATES.at(d->state));
        xmlWriter->writeAttribute("xmlns", ns_chat_states);
        xmlWriter->writeEndElement();
    }

    // XEP-0071: XHTML-IM
    if (!d->xhtml.isEmpty()) {
        xmlWriter->writeStartElement("html");
        xmlWriter->writeAttribute("xmlns", ns_xhtml_im);
        xmlWriter->writeStartElement("body");
        xmlWriter->writeAttribute("xmlns", ns_xhtml);
        xmlWriter->writeCharacters("");
        xmlWriter->device()->write(d->xhtml.toUtf8());
        xmlWriter->writeEndElement();
        xmlWriter->writeEndElement();
    }

    // time stamp
    if (d->stamp.isValid()) {
        QDateTime utcStamp = d->stamp.toUTC();
        if (d->stampType == DelayedDelivery)
        {
            // XEP-0203: Delayed Delivery
            xmlWriter->writeStartElement("delay");
            xmlWriter->writeAttribute("xmlns", ns_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", QXmppUtils::datetimeToString(utcStamp));
            xmlWriter->writeEndElement();
        } else {
            // XEP-0091: Legacy Delayed Delivery
            xmlWriter->writeStartElement("x");
            xmlWriter->writeAttribute("xmlns", ns_legacy_delayed_delivery);
            helperToXmlAddAttribute(xmlWriter, "stamp", utcStamp.toString("yyyyMMddThh:mm:ss"));
            xmlWriter->writeEndElement();
        }
    }

    // XEP-0184: Message Delivery Receipts
    if (!d->receiptId.isEmpty()) {
        xmlWriter->writeStartElement("received");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeAttribute("id", d->receiptId);
        xmlWriter->writeEndElement();
    }
    if (d->receiptRequested) {
        xmlWriter->writeStartElement("request");
        xmlWriter->writeAttribute("xmlns", ns_message_receipts);
        xmlWriter->writeEndElement();
    }

    // XEP-0224: Attention
    if (d->attentionRequested) {
        xmlWriter->writeStartElement("attention");
        xmlWriter->writeAttribute("xmlns", ns_attention);
        xmlWriter->writeEndElement();
    }

    // XEP-0249: Direct MUC Invitations
    if (!d->mucInvitationJid.isEmpty()) {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_conference);
        xmlWriter->writeAttribute("jid", d->mucInvitationJid);
        if (!d->mucInvitationPassword.isEmpty())
            xmlWriter->writeAttribute("password", d->mucInvitationPassword);
        if (!d->mucInvitationReason.isEmpty())
            xmlWriter->writeAttribute("reason", d->mucInvitationReason);
        xmlWriter->writeEndElement();
    }

    // XEP-0333: Chat Markers
    if (d->markable) {
        xmlWriter->writeStartElement("markable");
        xmlWriter->writeAttribute("xmlns", ns_chat_markers);
        xmlWriter->writeEndElement();
    }
    if (d->marker != NoMarker) {
        xmlWriter->writeStartElement(MARKER_TYPES.at(d->marker));
        xmlWriter->writeAttribute("xmlns", ns_chat_markers);
        xmlWriter->writeAttribute("id", d->markedId);
        if (!d->markedThread.isNull() && !d->markedThread.isEmpty()) {
            xmlWriter->writeAttribute("thread", d->markedThread);
        }
        xmlWriter->writeEndElement();
    }

    // XEP-0280: Message Carbons
    if (d->privatemsg) {
        xmlWriter->writeStartElement("private");
        xmlWriter->writeAttribute("xmlns", ns_carbons);
        xmlWriter->writeEndElement();
    }

    // XEP-0066: Out of Band Data
    if (!d->outOfBandUrl.isEmpty()) {
        xmlWriter->writeStartElement("x");
        xmlWriter->writeAttribute("xmlns", ns_oob);
        xmlWriter->writeTextElement("url", d->outOfBandUrl);
        xmlWriter->writeEndElement();
    }

    // XEP-0308: Last Message Correction
    if (!d->replaceId.isEmpty()) {
        xmlWriter->writeStartElement("replace");
        xmlWriter->writeAttribute("xmlns", ns_message_correct);
        xmlWriter->writeAttribute("id", d->replaceId);
        xmlWriter->writeEndElement();
    }

    // XEP-0334: Message Processing Hints
    for (quint8 i = 0; i < HINT_TYPES.size(); i++) {
        if (hasHint(Hint(1 << i))) {
            xmlWriter->writeStartElement(HINT_TYPES.at(i));
            xmlWriter->writeAttribute("xmlns", ns_message_processing_hints);
            xmlWriter->writeEndElement();
        }
    }

    // XEP-0367: Message Attaching
    if (!d->attachId.isEmpty()) {
        xmlWriter->writeStartElement("attach-to");
        xmlWriter->writeAttribute("xmlns", ns_message_attaching);
        xmlWriter->writeAttribute("id", d->attachId);
        xmlWriter->writeEndElement();
    }

    // XEP-0369: Mediated Information eXchange (MIX)
    if (!d->mixUserJid.isEmpty() || !d->mixUserNick.isEmpty()) {
        xmlWriter->writeStartElement("mix");
        xmlWriter->writeAttribute("xmlns", ns_mix);
        helperToXmlAddTextElement(xmlWriter, "jid", d->mixUserJid);
        helperToXmlAddTextElement(xmlWriter, "nick", d->mixUserNick);
        xmlWriter->writeEndElement();
    }

    // XEP-0380: Explicit Message Encryption
    if (!d->encryptionMethod.isEmpty()) {
        xmlWriter->writeStartElement("encryption");
        xmlWriter->writeAttribute("xmlns", ns_eme);
        xmlWriter->writeAttribute("namespace", d->encryptionMethod);
        helperToXmlAddAttribute(xmlWriter, "name", d->encryptionName);
        xmlWriter->writeEndElement();
    }

    // XEP-0382: Spoiler messages
    if (d->isSpoiler) {
        xmlWriter->writeStartElement("spoiler");
        xmlWriter->writeAttribute("xmlns", ns_spoiler);
        xmlWriter->writeCharacters(d->spoilerHint);
        xmlWriter->writeEndElement();
    }

    // other extensions
    QXmppStanza::extensionsToXml(xmlWriter);

    xmlWriter->writeEndElement();
}
/// \endcond
