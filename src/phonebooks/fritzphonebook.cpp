/**
 * \file fritzphonebook.cpp
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
 * FritzBox phonebook extraction.
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright:
 * Copyright (c) 2025, Agundur <info@agundur.de>
 * Released under the GPL-2.0-only OR GPL-3.0-only OR
 * LicenseRef-KDE-Accepted-GPL Original code form:
 * https://github.com/Agundur-KDE/kfritz
 */

#include "fritzphonebook.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <QXmlStreamReader>

#include "../fritzsoap.h"
#include "../settings.h"

using namespace Qt::StringLiterals;

QString FritzPhonebook::m_sSavepath;

FritzPhonebook::FritzPhonebook(QObject *pParent) : QObject(pParent) {}

FritzPhonebook *FritzPhonebook::instance() {
  static FritzPhonebook _instance;
  m_sSavepath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" +
      qApp->applicationName().toLower() +
      QStringLiteral("/fritzbox_phonebooks");
  return &_instance;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto FritzPhonebook::getContacts() -> QHash<QString, QString> {
  QHash<QString, QString> tmpContacts;
  QHash<QString, QString> PhoneNumbers;

  QHashIterator<QString, QString> i(m_Phonebooks);
  while (i.hasNext()) {
    i.next();
    tmpContacts.clear();

    if (Settings().getEnabledFritzPhonebooks().contains(i.value())) {
      qDebug() << "Reading FritzBox phonebook:" << i.key() << i.value();
      QString path = m_sSavepath + QStringLiteral("/phonebook_") + i.key() +
                     QStringLiteral(".xml");
      tmpContacts = this->loadFromFile(path, Settings().getCountryCode());

      // Merge into contacts list
      PhoneNumbers.insert(tmpContacts);
    }
  }

  return PhoneNumbers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QStringList FritzPhonebook::getPhonebookList() {
  m_Phonebooks.clear();

  const QString body = QStringLiteral(
      "<u:GetPhonebookList "
      "xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>");

  const QString response = FritzSOAP::instance()->sendRequest(
      QStringLiteral("urn:dslforum-org:service:X_AVM-DE_OnTel:1"),
      QStringLiteral("GetPhonebookList"), body,
      QStringLiteral("/upnp/control/x_contact"));

  QStringList phonebookIDs;
  QXmlStreamReader xml(response);
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() &&
        xml.name() == QStringLiteral("NewPhonebookList")) {
      phonebookIDs =
          xml.readElementText().split(QStringLiteral(","), Qt::SkipEmptyParts);
      break;
    }
  }

  if (xml.hasError()) {
    qWarning() << "XML Parse Error:" << xml.errorString();
  }

  for (const QString &idStr : phonebookIDs) {
    bool ok = false;
    int id = idStr.toInt(&ok);
    if (!ok) {
      qWarning() << "Invalid phonebook ID:" << idStr;
      continue;
    }

    QStringList sListInfo = getPhonebookUrlAndName(id);
    if (sListInfo.size() == 2) {
      qDebug() << "Phonebook ID" << id << "→ Name:" << sListInfo.at(0)
               << "→ URL:" << sListInfo.at(1);
      m_Phonebooks.insert(idStr, sListInfo.at(0));
      downloadPhonebook(id, QUrl(sListInfo.at(1)));
    } else {
      qWarning() << "No Name and/or URL for phonebook ID" << id;
    }
  }

  return m_Phonebooks.values();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

bool FritzPhonebook::downloadPhonebook(int id, const QUrl &url) {
  qDebug() << "Downloading phonebook ID" << id << "from" << url;

  QNetworkAccessManager nam;
  QNetworkRequest request(url);
  QNetworkReply *reply = nam.get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    qWarning() << "Download failed:" << reply->errorString();
    reply->deleteLater();
    return false;
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();

  /*
  QString baseDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
      QStringLiteral("/phonebooks");
  QDir().mkpath(baseDir);
  QString filePath =
      baseDir + QStringLiteral("/phonebook_") + QString::number(id) +
  QStringLiteral(".xml");
  */
  QDir().mkpath(m_sSavepath);
  QString filePath = m_sSavepath + QStringLiteral("/phonebook_") +
                     QString::number(id) + QStringLiteral(".xml");

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qWarning() << "Failed to write file:" << filePath;
    return false;
  }

  file.write(data);
  file.close();
  qDebug() << "Saved phonebook ID" << id << "to" << filePath;

  return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QStringList FritzPhonebook::getPhonebookUrlAndName(int phonebookId) {
  const QString body =
      QStringLiteral(
          "<u:GetPhonebook "
          "xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\">"
          "<NewPhonebookID>") +
      QString::number(phonebookId) +
      QStringLiteral(
          "</NewPhonebookID>"
          "</u:GetPhonebook>");

  const QString response = FritzSOAP::instance()->sendRequest(
      QStringLiteral("urn:dslforum-org:service:X_AVM-DE_OnTel:1"),
      QStringLiteral("GetPhonebook"), body,
      QStringLiteral("/upnp/control/x_contact"));

  if (response.isEmpty()) {
    qWarning() << "Empty SOAP response for GetPhonebook";
    return {};
  }

  QXmlStreamReader xml(response);
  QStringList sListInfo;
  QString sUrl;
  QString sName;
  // qDebug() << response;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() &&
        xml.name() == QStringLiteral("NewPhonebookURL")) {
      sUrl = xml.readElementText().trimmed();
    }
    if (xml.isStartElement() &&
        xml.name() == QStringLiteral("NewPhonebookName")) {
      sName = xml.readElementText().trimmed();
    }
  }

  if (xml.hasError()) {
    qWarning() << "XML Parse Error:" << xml.errorString();
    sListInfo.clear();
  }
  if (!sUrl.isEmpty() && !sName.isEmpty()) {
    sListInfo << sName << sUrl;
  }

  return sListInfo;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QHash<QString, QString> FritzPhonebook::loadFromFile(
    const QString &xmlFilePath, const QString &countryCode) {
  QHash<QString, QString> phoneNumbers;

  QFile file(xmlFilePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "❌ Could not open phonebook XML:" << xmlFilePath;
    return QHash<QString, QString>();
  }

  QDomDocument doc;
  if (!doc.setContent(&file)) {
    qWarning() << "❌ Invalid XML in phonebook file:" << xmlFilePath;
    return QHash<QString, QString>();
  }

  file.close();

  QDomNodeList contacts = doc.elementsByTagName(QStringLiteral("contact"));
  for (int i = 0; i < contacts.count(); ++i) {
    QDomElement contact = contacts.at(i).toElement();
    QString name;

    QDomElement person = contact.firstChildElement(QStringLiteral("person"));
    if (!person.isNull()) {
      QDomElement realNameElement =
          person.firstChildElement(QStringLiteral("realName"));
      if (!realNameElement.isNull()) {
        name = realNameElement.text().trimmed();
      }
    }

    QDomNodeList telList = contact.elementsByTagName(QStringLiteral("number"));
    for (int j = 0; j < telList.count(); ++j) {
      QDomElement numberElement = telList.at(j).toElement();
      QString raw = numberElement.text().trimmed();
      QString normalized = normalizeNumber(raw, countryCode);
      QString type = numberElement.attribute(QStringLiteral("type"), "")
                         .trimmed()
                         .toLower();

      if (name.startsWith(QStringLiteral("SPAM"))) {
        name = tr("SPAM CALL") + " (" + normalized + ")";
        type.clear();
      } else {
        if (type == QStringLiteral("home")) {
          type = " (" + tr("Home") + ")";
        }
        if (type == QStringLiteral("work")) {
          type = " (" + tr("Work") + ")";
        }
        if (type == QStringLiteral("mobile")) {
          type = " (" + tr("Cell") + ")";
        }
        if (type == QStringLiteral("fax")) {
          type = " (" + tr("Fax") + ")";
        }
        if (type == QStringLiteral("fax_work")) {
          type = " (" + tr("Fax") + ")";
        }
        if (type == QStringLiteral("other")) {
          type = " (" + tr("Other") + ")";
        }
        if (type == QStringLiteral("intern")) {
          type = " (" + tr("Intern") + ")";
        }
      }

      if (!normalized.isEmpty() && !name.isEmpty()) {
        phoneNumbers[normalized] = name + type;
      }
    }
  }

  qDebug() << "✅ Phonebook loaded:" << phoneNumbers.size() << "entries.";
  return phoneNumbers;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString FritzPhonebook::normalizeNumber(QString number,
                                        const QString &countryCode) const {
  // Format country code
  number = number.replace(QStringLiteral("+"), QStringLiteral("00"));
  // Remove country code, if it is your country
  if (number.startsWith(countryCode)) {
    number = number.replace(0, countryCode.length(), QStringLiteral("0"));
  }

  // Cleanup number (remove spaces, '-', '(', ')', '/')
  number.remove(QRegularExpression(QStringLiteral("[\\s\\-\\(\\)/]")));

  return number;
}
