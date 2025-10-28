/**
 * \file fritzsoap.cpp
 *
 * \section LICENSE
 *
 * Copyright (C) 2024-present Thorsten Roth
 *
 * This file is part of FritzCallIndicator.
 *
 * FritzCallIndicator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FritzCallIndicator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FritzCallIndicator.  If not, see <https://www.gnu.org/licenses/>.
 *
 * \section DESCRIPTION
 * FritzBox SOAP connection.
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright:
 * Copyright (c) 2025, Agundur <info@agundur.de>
 * Released under the GPL-2.0-only OR GPL-3.0-only OR
 * LicenseRef-KDE-Accepted-GPL Original code form:
 * https://github.com/Agundur-KDE/kfritz
 */

#include "fritzsoap.h"

#include <QAuthenticator>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "settings.h"

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

FritzSOAP::FritzSOAP(QObject *pParent) : QObject(pParent) {}

FritzSOAP *FritzSOAP::instance() {
  static FritzSOAP _instance;
  return &_instance;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString FritzSOAP::sendRequest(const QString &service, const QString &action,
                               const QString &body, const QString &controlUrl) {
  Settings settings;
  const QUrl url =
      QUrl(QStringLiteral("http://") + settings.getHostName() + ":" +
           QString::number(settings.getTR064Port()) + controlUrl);

  QNetworkRequest request(url);

  // 🔐 Authorization
  QString credentials =
      settings.getFritzUser() + ":" + settings.getFritzPassword();
  QByteArray auth = "Basic " + credentials.toUtf8().toBase64();

  request.setRawHeader("Authorization", auth);

  // 📄 Content & SOAP
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("text/xml; charset=\"utf-8\""));

  request.setRawHeader("SOAPACTION",
                       "\"" + service.toUtf8() + "#" + action.toUtf8() + "\"");

  const QString envelope = QStringLiteral(
                               R"(<?xml version="1.0" encoding="utf-8"?>
<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
            s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
  <s:Body>)") + body + QStringLiteral("</s:Body></s:Envelope>");

  QNetworkAccessManager nam;

  // 🔐 Optional: Auth-Fallback über QAuthenticator (nur wenn RawHeader nicht
  // greift)
  QObject::connect(
      &nam, &QNetworkAccessManager::authenticationRequired,
      [](QNetworkReply * /*reply*/, QAuthenticator *authenticator) {
        qDebug() << "authenticationRequired() triggered!";
        authenticator->setUser(Settings::instance()->getFritzUser());
        authenticator->setPassword(Settings::instance()->getFritzPassword());
      });

  QNetworkReply *reply = nam.post(request, envelope.toUtf8());

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QString result;
  if (reply->error() != QNetworkReply::NoError) {
    qWarning() << "SOAP request failed:" << reply->errorString();
  } else {
    result = QString::fromUtf8(reply->readAll());
  }

  reply->deleteLater();
  return result;
}
