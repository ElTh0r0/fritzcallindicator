// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR
// LicenseRef-KDE-Accepted-GPL

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
  Settings *pSettings = Settings::instance();
  const QUrl url =
      QUrl(QStringLiteral("http://") + pSettings->getHostName() + ":" +
           QString::number(pSettings->getTR064Port()) + controlUrl);

  QNetworkRequest request(url);

  // Authorization
  QString credentials =
      pSettings->getFritzUser() + ":" + pSettings->getFritzPassword();
  QByteArray auth = "Basic " + credentials.toUtf8().toBase64();

  request.setRawHeader("Authorization", auth);

  // Content & SOAP
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

  // Optional: Auth-Fallback über QAuthenticator (nur wenn RawHeader nicht
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
