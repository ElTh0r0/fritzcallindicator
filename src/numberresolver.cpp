/**
 * \file numberresolver.cpp
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
 * Resolve phone numbers; either by phone book or by country / area code
 */

#include "numberresolver.h"

#include <QDebug>
#include <QDirIterator>
#include <QMessageBox>

#include "phonebooks/fritzphonebook.h"
#include "phonebooks/thunderbird.h"

NumberResolver::NumberResolver(const QDir &sharePath,
                               const QString &sLocalCountryCode,
                               QObject *pParent)
    : QObject(pParent),
      m_sLocalCountryCode(sLocalCountryCode),
      CSV_SEPARATOR(';') {
  qDebug() << Q_FUNC_INFO;
  this->initCountryCodes(sharePath);
  this->initAreaCodes(sharePath);

  m_pOnlineResolvers = new OnlineResolvers(sharePath);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void NumberResolver::initCountryCodes(const QDir &sharePath) {
  qDebug() << Q_FUNC_INFO;

  QFile fCountryCsv(sharePath.absolutePath() + "/country_codes.csv");
  if (!fCountryCsv.exists()) {
    qWarning() << "Country codes file not found:"
               << sharePath.absolutePath() + "/country_codes.csv";
    QMessageBox::warning(nullptr, tr("Missing country codes"),
                         tr("Country codes file not found!"));
    return;
  }

  if (fCountryCsv.open(QIODevice::ReadOnly)) {
    QString sLine;
    QStringList tmpSplit;

    while (!fCountryCsv.atEnd()) {
      sLine = fCountryCsv.readLine();
      tmpSplit = sLine.split(CSV_SEPARATOR, Qt::SkipEmptyParts);
      if (tmpSplit.size() == 2) {
        m_CountryCodes[tmpSplit[0].trimmed()] = tmpSplit[1].trimmed();
      }
    }

    fCountryCsv.close();
  } else {
    qWarning() << "Failed to open country code file:"
               << sharePath.absolutePath() + "/country_codes.csv";
  }

  // qDebug() << "Found country codes:" << m_CountryCodes.keys();
  // qDebug() << "Found:" << m_CountryCodes["1658"];
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void NumberResolver::initAreaCodes(QDir sharePath) {
  qDebug() << Q_FUNC_INFO;

  if (!sharePath.cd(QStringLiteral("area_codes"))) {
    qWarning() << "Area codes folder not found:"
               << sharePath.absolutePath() + "/area_codes";
    QMessageBox::warning(nullptr, tr("Missing area codes"),
                         tr("Area codes folder not found!"));
    return;
  }

  QDirIterator it(
      sharePath.absolutePath(), QStringList() << QStringLiteral("*.csv"),
      QDir::NoDotAndDotDot | QDir::Files, QDirIterator::NoIteratorFlags);
  while (it.hasNext()) {
    it.next();

    QString sACode = it.fileName().remove(QStringLiteral(".csv"));
    if (sACode.contains('_')) {
      sACode.truncate(sACode.indexOf('_'));
    }

    QFile csv(it.filePath());
    if (csv.open(QIODevice::ReadOnly)) {
      QString sLine;
      QStringList tmpSplit;
      QHash<QString, QString> tmpCodes;

      while (!csv.atEnd()) {
        sLine = csv.readLine();
        tmpSplit = sLine.split(CSV_SEPARATOR, Qt::SkipEmptyParts);
        if (tmpSplit.size() == 2) {
          tmpCodes[tmpSplit[0].trimmed()] = tmpSplit[1].trimmed();
        }
      }

      csv.close();
      m_AreaCodes[sACode] = tmpCodes;
    } else {
      qWarning() << "Failed to open area code file:" << it.filePath();
    }
  }

  // qDebug() << "Found area codes:" << m_AreaCodes.keys();
  // qDebug() << "Found:" << m_AreaCodes["0049"]["32"];
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void NumberResolver::readPhonebooks() {
  m_KnownContacts.clear();

  // Merge external addressbooks into contacts list
  m_KnownContacts.insert(Thunderbird::instance()->getContacts());
  m_KnownContacts.insert(FritzPhonebook::instance()->getContacts());
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto NumberResolver::resolveNumber(
    const QString &sNumber, const QStringList &sListEnabledResolvers) const
    -> QString {
  bool bLocalCall(false);
  QString sCountryCode;
  QString sCountryName(tr("Unknown country"));
  QString sCityCode;
  QString sCityName(tr("Unknown city"));
  QString sPhoneNumber(sNumber);
  QString sKnownCaller;

  if (sPhoneNumber.isEmpty()) {
    return tr("Number suppressed");
  }

  // Caller registered in my country calls from abroad
  if (sPhoneNumber.startsWith(m_sLocalCountryCode)) {
    sPhoneNumber = "0" + sPhoneNumber.remove(0, m_sLocalCountryCode.length());
  }

  // Search in contacts
  // Local country code is automatically removed from address book!
  sKnownCaller = m_KnownContacts.value(sPhoneNumber, QString());
  if (!sKnownCaller.isEmpty()) {
    return sKnownCaller;
  }

  // Resolve country code
  if (sPhoneNumber.startsWith(QStringLiteral("00"))) {
    for (auto i = m_CountryCodes.cbegin(), end = m_CountryCodes.cend();
         i != end; ++i) {
      if (sPhoneNumber.startsWith("00" + i.key())) {
        sCountryCode = "00" + i.key();
        sPhoneNumber.remove(0, sCountryCode.length());
        sCountryName = m_CountryCodes.value(i.key(), tr("Unknown country"));
        break;
      }
    }
  } else if (sPhoneNumber.startsWith('0')) {
    sPhoneNumber.remove(0, 1);
  }

  if (sCountryCode.isEmpty()) {
    sCountryCode = m_sLocalCountryCode;
    bLocalCall = true;
  }

  // Search online
  sKnownCaller = m_pOnlineResolvers->searchOnline(
      "0" + sPhoneNumber, sCountryCode, sListEnabledResolvers);
  if (!sKnownCaller.isEmpty()) {
    return sKnownCaller;
  }

  // Resolve city code
  for (const auto &sCode : m_AreaCodes.value(sCountryCode).keys()) {
    if (sPhoneNumber.startsWith(sCode)) {
      sCityCode = sCode;
      sCityName =
          m_AreaCodes.value(sCountryCode).value(sCityCode, tr("Unknown city"));
      sPhoneNumber.remove(0, sCityCode.length());
      break;
    }
  }

  if (bLocalCall) {
    return "0" + sCityCode + " " + sPhoneNumber + " (" + sCityName + ")";
  }
  sCountryCode.remove(0, 2);
  return "+" + sCountryCode + " " + sCityCode + " " + sPhoneNumber + " (" +
         sCountryName + ": " + sCityName + ")";
}
