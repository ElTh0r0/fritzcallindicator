/**
 * \file carddav.h
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
 * Class definition for CardDAV addressbook.
 */

#ifndef CARDDAV_H_
#define CARDDAV_H_

#include <QHash>
#include <QObject>

class CardDAV : public QObject {
  Q_OBJECT

 public:
  static CardDAV *instance();
  auto getContacts() -> const QHash<QString, QString>;

 private:
  explicit CardDAV(QObject *pParent = nullptr);

  auto sendReportRequest(const QUrl &url, const QString &sUsername,
                         const QString &sAppPassword) -> QByteArray;
  void extractNumber(const QByteArray &xmlData,
                     const QString &sLocalCountryCode);

  QHash<QString, QString> m_PhoneNumbers;
};

#endif  // CARDDAV_H_
