/**
 * \file carddav.cpp
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
 * CardDAV addressbook extraction.
 */

#include "carddav.h"

#include <QDebug>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QXmlStreamReader>

#include "../settings.h"

CardDAV::CardDAV(QObject *pParent) : QObject{pParent} {}

CardDAV *CardDAV::instance() {
  static CardDAV _instance;
  return &_instance;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto CardDAV::getContacts() -> QHash<QString, QString> {
  m_PhoneNumbers.clear();

  for (const auto &addressbook : Settings().getCardDavAddressbooks()) {
    QString sUrl = addressbook.value(QStringLiteral("URL"));
    QString sUser = addressbook.value(QStringLiteral("User"));
    QString sPassword = addressbook.value(QStringLiteral("Password"));

    if (!sUrl.isEmpty() && !sUser.isEmpty() && !sPassword.isEmpty()) {
      qDebug() << "Reading CardDAV addressbook:" << sUrl;
      QByteArray data = this->sendReportRequest(QUrl(sUrl), sUser, sPassword);

      if (!data.isEmpty()) {
        this->extractNumber(data, Settings().getCountryCode());
      }
    }
  }

  return m_PhoneNumbers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto CardDAV::sendReportRequest(const QUrl &url, const QString &sUsername,
                                const QString &sAppPassword) -> QByteArray {
  QByteArray authHeader =
      "Basic " +
      QByteArray(QString("%1:%2").arg(sUsername, sAppPassword).toUtf8())
          .toBase64();

  QNetworkAccessManager nam;
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
  request.setRawHeader("Authorization", authHeader);
  request.setRawHeader("Depth", "1");  // Important!

  QString reportBody = R"(<?xml version="1.0" encoding="UTF-8"?>
<card:addressbook-query xmlns:d="DAV:" xmlns:card="urn:ietf:params:xml:ns:carddav">
  <d:prop>
    <d:getetag/>
    <card:address-data/>
  </d:prop>
</card:addressbook-query>)";

  QNetworkReply *reply =
      nam.sendCustomRequest(request, "REPORT", reportBody.toUtf8());

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    qWarning() << "Download failed:" << reply->error();
    qDebug()
        << "Status:"
        << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Error:" << reply->errorString();
    qDebug() << "Server answer:" << reply->readAll();

    reply->deleteLater();
    return QByteArray();
  }

  QByteArray data = reply->readAll();
  // qDebug() << "REPORT:";
  // qDebug() << data;
  reply->deleteLater();
  return data;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CardDAV::extractNumber(const QByteArray &xmlData,
                            const QString &sLocalCountryCode) {
  QXmlStreamReader xml(xmlData);

  while (!xml.atEnd() && !xml.hasError()) {
    xml.readNext();

    // Search for <card:address-data>
    if (xml.isStartElement() && xml.name() == QStringLiteral("address-data")) {
      QString vcardData =
          xml.readElementText(QXmlStreamReader::IncludeChildElements);
      const QStringList lines = vcardData.split("\n", Qt::SkipEmptyParts);

      QString sName;
      QStringList telLines;

      for (const QString &lineRaw : lines) {
        QString line = lineRaw.trimmed();
        if (line.startsWith(QStringLiteral("FN:"))) {
          sName = line.mid(3);  // everything after "FN:"
        } else if (line.startsWith(QStringLiteral("TEL"))) {
          telLines << line;
        }
      }

      QString sPhoneType;
      for (const QString &lineRaw : telLines) {
        // Example: TEL;TYPE=WORK,CELL:012345678
        QString line = lineRaw.trimmed();

        int sepIndex = line.indexOf(':');
        if (sepIndex > 0) {
          QString sType = line.left(sepIndex);  // "TEL;TYPE=WORK,CELL"
          QString sNumber = line.mid(sepIndex + 1).trimmed();  // Number

          if (!sNumber.isEmpty()) {
            // Type info
            int typePos = sType.indexOf(QStringLiteral("TYPE="));
            if (typePos != -1) {
              sType = sType.mid(typePos + 5);  // everything after "TYPE="
            }

            // Concat types as a number can have assigned multiple types
            if (sType.contains(QStringLiteral("home"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Home") + " ";
            }
            if (sType.contains(QStringLiteral("work"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Work") + " ";
            }
            if (sType.contains(QStringLiteral("cell"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Cell") + " ";
            }
            if (sType.contains(QStringLiteral("fax"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Fax") + " ";
            }
            if (sType.contains(QStringLiteral("pager"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Pager") + " ";
            }
            if (sType.contains(QStringLiteral("car"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Car") + " ";
            }
            if (sType.contains(QStringLiteral("voice"), Qt::CaseInsensitive)) {
              sPhoneType += tr("Voice") + " ";
            }

            if (!sPhoneType.trimmed().isEmpty()) {
              sPhoneType = " (" + sPhoneType.trimmed() + ")";
            }

            // Format country code
            sNumber =
                sNumber.replace(QStringLiteral("+"), QStringLiteral("00"));
            // Remove country code, if it is your country
            if (sNumber.startsWith(sLocalCountryCode)) {
              sNumber = sNumber.replace(0, sLocalCountryCode.length(),
                                        QStringLiteral("0"));
            }
            // Cleanup number (remove spaces, '-', '(', ')', '/')
            sNumber.remove(
                QRegularExpression(QStringLiteral("[\\s\\-\\(\\)/]")));

            m_PhoneNumbers[sNumber] = sName + sPhoneType;
            sPhoneType.clear();
          }
        }
      }
    }
  }

  if (xml.hasError()) {
    qWarning() << "XML parse error:" << xml.errorString();
  }
}
