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

#include "fritzsoap.h"

using namespace Qt::StringLiterals;

FritzPhonebook::FritzPhonebook(QObject *parent) : QObject(parent) {}

void FritzPhonebook::setSavepath(const QString &savepath) {
  m_savepath = savepath;
}
const QDir FritzPhonebook::getSavepath() { return QDir(m_savepath); }

QHash<QString, QHash<QString, QString> > FritzPhonebook::getPhonebookList() {
  const QString body =
      u"<u:GetPhonebookList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>"_s;

  const QString response = FritzSOAP::instance()->sendRequest(
      u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebookList"_s,
      body, u"/upnp/control/x_contact"_s);

  QStringList phonebookIDs;
  QXmlStreamReader xml(response);
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == u"NewPhonebookList"_s) {
      phonebookIDs =
          xml.readElementText().split(QStringLiteral(","), Qt::SkipEmptyParts);
    }
  }

  if (xml.hasError()) {
    qWarning() << "XML Parse Error:" << xml.errorString();
  }

  QHash<QString, QHash<QString, QString> > phonebooks;

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
      QHash<QString, QString> info;
      info.insert(QStringLiteral("Name"), sListInfo.at(0));
      info.insert(QStringLiteral("URL"), sListInfo.at(1));
      phonebooks.insert(idStr, info);
    } else {
      qWarning() << "No Name and/or URL for phonebook ID" << id;
    }
  }

  return phonebooks;
}

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

  QString baseDir = m_savepath;
  // QString baseDir =
  // QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) +
  // u"/phonebooks"_s;
  QDir().mkpath(baseDir);
  QString filePath =
      baseDir + u"/phonebook_"_s + QString::number(id) + u".xml"_s;

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

QStringList FritzPhonebook::getPhonebookUrlAndName(int phonebookId) {
  const QString body =
      u"<u:GetPhonebook xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\">"
      u"<NewPhonebookID>"_s +
      QString::number(phonebookId) +
      u"</NewPhonebookID>"
      u"</u:GetPhonebook>"_s;

  const QString response = FritzSOAP::instance()->sendRequest(
      u"urn:dslforum-org:service:X_AVM-DE_OnTel:1"_s, u"GetPhonebook"_s, body,
      u"/upnp/control/x_contact"_s);

  if (response.isEmpty()) {
    qWarning() << "Empty SOAP response for GetPhonebook";
    return {};
  }

  QXmlStreamReader xml(response);
  QStringList sListInfo;
  QString sUrl;
  QString sName;
  qDebug() << response;
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() && xml.name() == u"NewPhonebookURL") {
      sUrl = xml.readElementText().trimmed();
    }
    if (xml.isStartElement() && xml.name() == u"NewPhonebookName") {
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

  QDomNodeList contacts = doc.elementsByTagName(u"contact"_s);
  for (int i = 0; i < contacts.count(); ++i) {
    QDomElement contact = contacts.at(i).toElement();
    QString name;
    QStringList numbers;

    QDomElement person = contact.firstChildElement(u"person"_s);
    if (!person.isNull()) {
      QDomElement realNameElement = person.firstChildElement(u"realName"_s);
      if (!realNameElement.isNull()) {
        name = realNameElement.text().trimmed();
      }
    }

    QDomNodeList telList = contact.elementsByTagName(u"number"_s);
    for (int j = 0; j < telList.count(); ++j) {
      QDomElement numberElement = telList.at(j).toElement();
      QString raw = numberElement.text().trimmed();
      QString normalized = normalizeNumber(raw, countryCode);
      QString type = numberElement.attribute(u"type"_s, "").trimmed().toLower();

      if (name.startsWith(u"SPAM"_s)) {
        name = tr("SPAM CALL") + " (" + normalized + ")";
        type.clear();
      } else {
        if (type == u"home"_s) {
          type = " (" + tr("Home") + ")";
        }
        if (type == u"work"_s) {
          type = " (" + tr("Work") + ")";
        }
        if (type == u"mobile"_s) {
          type = " (" + tr("Cell") + ")";
        }
        if (type == u"fax"_s) {
          type = " (" + tr("Fax") + ")";
        }
        if (type == u"fax_work"_s) {
          type = " (" + tr("Fax") + ")";
        }
        if (type == u"other"_s) {
          type = " (" + tr("Other") + ")";
        }
        if (type == u"intern"_s) {
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

QString FritzPhonebook::normalizeNumber(QString number,
                                        const QString &countryCode) const {
  // Format country code
  number = number.replace(QStringLiteral("+"), QStringLiteral("00"));
  // Remove country code, if it is your country
  if (number.startsWith(countryCode)) {
    number = number.replace(0, countryCode.length(), QStringLiteral("0"));
  }

  // Cleanup number (remove spaces, '-', '(', ')', '/')
  number.remove(QRegularExpression(u"[\\s\\-\\(\\)/]"_s));

  return number;
}
