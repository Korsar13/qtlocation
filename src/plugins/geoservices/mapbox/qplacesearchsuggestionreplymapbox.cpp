/****************************************************************************
**
** Copyright (C) 2017 Mapbox, Inc.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtFoo module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qplacesearchsuggestionreplymapbox.h"
#include "qplacemanagerenginemapbox.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtLocation/QPlaceResult>
#include <QtLocation/QPlaceSearchRequest>

QT_BEGIN_NAMESPACE

QPlaceSearchSuggestionReplyMapbox::QPlaceSearchSuggestionReplyMapbox(QNetworkReply *reply, QPlaceManagerEngineMapbox *parent)
:   QPlaceSearchSuggestionReply(parent)
{
    Q_ASSERT(parent);
    if (!reply) {
        setError(UnknownError, QStringLiteral("Null reply"));
        return;
    }

    connect(reply, &QNetworkReply::finished, this, &QPlaceSearchSuggestionReplyMapbox::onReplyFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &QPlaceSearchSuggestionReplyMapbox::onNetworkError);

    connect(this, &QPlaceReply::aborted, reply, &QNetworkReply::abort);
    connect(this, &QObject::destroyed, reply, &QObject::deleteLater);
}

QPlaceSearchSuggestionReplyMapbox::~QPlaceSearchSuggestionReplyMapbox()
{
}

void QPlaceSearchSuggestionReplyMapbox::setError(QPlaceReply::Error errorCode, const QString &errorString)
{
    QPlaceReply::setError(errorCode, errorString);
    emit error(errorCode, errorString);

    setFinished(true);
    emit finished();
}

void QPlaceSearchSuggestionReplyMapbox::onReplyFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
        return;

    QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
    if (!document.isObject()) {
        setError(ParseError, tr("Response parse error"));
        return;
    }

    const QJsonArray features = document.object().value(QStringLiteral("features")).toArray();

    QStringList suggestions;
    for (const QJsonValueConstRef feature : features) {
        if (feature.isObject())
            suggestions.append(feature.toObject().value(QStringLiteral("text")).toString());
    }

    setSuggestions(suggestions);

    setFinished(true);
    emit finished();
}

void QPlaceSearchSuggestionReplyMapbox::onNetworkError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    QNetworkReply *reply = static_cast<QNetworkReply *>(sender());
    reply->deleteLater();
    setError(CommunicationError, reply->errorString());
}

QT_END_NAMESPACE
