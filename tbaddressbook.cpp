/**
 * \file tbaddressbook.cpp
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
 * Thunderbird addressbook extraction.
 */

#include "tbaddressbook.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStandardPaths>

TbAddressbook::TbAddressbook(QObject *pParent) : QObject{pParent} {}

auto TbAddressbook::importVCards(const QFileInfo &fiDbFile,
                                 const QString &sLocalCountryCode)
    -> QHash<QString, QString> {
  m_PhoneNumbers.clear();

  if (fiDbFile.exists()) {
    QString sTmpDb(
        QStandardPaths::writableLocation(QStandardPaths::TempLocation) + +"/" +
        fiDbFile.fileName());

    if (QFile::exists(sTmpDb)) {
      if (!QFile::remove(sTmpDb)) {
        qWarning() << "Couldn't delete old temp DB:" << sTmpDb;
      }
      QFile::remove(sTmpDb + "-shm");
      QFile::remove(sTmpDb + "-wal");
    }
    if (!QFile::copy(fiDbFile.absoluteFilePath(), sTmpDb)) {
      qWarning() << "Couldn't copy DB to temp - source:"
                 << fiDbFile.absoluteFilePath() << " - destination:" << sTmpDb;
      return QHash<QString, QString>();
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(sTmpDb);
    db.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));

    if (!db.open()) {
      qWarning() << "ERROR: Connection to database failed!";
    } else {
      QSqlQuery query;
      if (query.exec(QStringLiteral("SELECT value FROM properties WHERE name = "
                                    "'_vCard' and value LIKE '%TEL;%'"))) {
        while (query.next()) {
          // qDebug() << query.value(QStringLiteral("value")).toString() + "\n";
          this->extractNumber(query.value(QStringLiteral("value")).toString(),
                              sLocalCountryCode);
        }
      } else {
        qDebug() << "ERROR while reading database:" << query.lastError();
      }

      db.close();
    }

    QFile::remove(sTmpDb);
    QFile::remove(sTmpDb + "-shm");
    QFile::remove(sTmpDb + "-wal");
  } else {
    qWarning() << "ERROR: DB file not found!";
  }

  return m_PhoneNumbers;
}

void TbAddressbook::extractNumber(const QString &sVCard,
                                  const QString &sLocalCountryCode) {
  static QRegularExpression rSplit(QStringLiteral("\n|\r\n|\r"));
  QStringList splitVCard;
  QStringList findName;
  QString sName;
  QString sNumber;
  QString sPhoneType;

  splitVCard << sVCard.split(rSplit, Qt::SkipEmptyParts);

  // Search for the name first, because the vcard
  // entries may not necessarily start with "FN:"
  static QRegularExpression rFN(QStringLiteral("^FN:"));
  findName << splitVCard.filter(rFN);
  if (findName.isEmpty()) {
    sName = QStringLiteral("ANONYMUS");
  } else {
    sName = findName.first().mid(3).trimmed();
  }

  for (const auto &s : std::as_const(splitVCard)) {
    if (s.startsWith(QStringLiteral("TEL;"), Qt::CaseInsensitive) ||
        s.startsWith(QStringLiteral("ITEM1.TEL"), Qt::CaseInsensitive) ||
        s.startsWith(QStringLiteral("ITEM2.TEL"), Qt::CaseInsensitive) ||
        s.startsWith(QStringLiteral("ITEM3.TEL"), Qt::CaseInsensitive)) {
      // qDebug() << s;

      // Concat types as a number can have assigned multiple for one number
      if (s.contains(QStringLiteral("home"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Home") + " ";
      }
      if (s.contains(QStringLiteral("work"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Work") + " ";
      }
      if (s.contains(QStringLiteral("cell"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Cell") + " ";
      }
      if (s.contains(QStringLiteral("fax"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Fax") + " ";
      }
      if (s.contains(QStringLiteral("pager"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Pager") + " ";
      }
      if (s.contains(QStringLiteral("video"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Video") + " ";
      }
      if (s.contains(QStringLiteral("voice"), Qt::CaseInsensitive)) {
        sPhoneType += tr("Voice") + " ";
      }

      if (!sPhoneType.trimmed().isEmpty()) {
        sPhoneType = " (" + sPhoneType.trimmed() + ")";
      }

      // Extract number - just consider everything after last ':' as number
      sNumber = s.mid(s.lastIndexOf(':') + 1).trimmed();
      // Format country code
      sNumber = sNumber.replace(QStringLiteral("+"), QStringLiteral("00"));
      // Remove country code, if it is your country
      if (sNumber.startsWith(sLocalCountryCode)) {
        sNumber =
            sNumber.replace(0, sLocalCountryCode.length(), QStringLiteral("0"));
      }
      // Cleanup number
      sNumber.remove('/').remove('-').remove('(').remove(')').remove(' ');

      m_PhoneNumbers[sNumber] = sName + sPhoneType;
      sNumber.clear();     // Reset after adding to list!
      sPhoneType.clear();  // Reset after adding to list!
    }
  }
}
