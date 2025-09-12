/**
 * \file tbaddressbook.h
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
 * Class definition for Thunderbird addressbook.
 */

#ifndef TBADDRESSBOOK_H_
#define TBADDRESSBOOK_H_

#include <QFileInfo>
#include <QHash>
#include <QObject>

class TbAddressbook : public QObject {
  Q_OBJECT

 public:
  static TbAddressbook *instance();
  auto getContacts() -> QHash<QString, QString>;

 private:
  explicit TbAddressbook(QObject *pParent = nullptr);

  void importVCards(const QFileInfo &fiDbFile);
  void extractNumber(const QString &sVCard, const QString &sLocalCountryCode);

  QHash<QString, QString> m_PhoneNumbers;
};

#endif  // TBADDRESSBOOK_H_
