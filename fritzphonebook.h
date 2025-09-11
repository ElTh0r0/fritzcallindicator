/**
 * \file fritzphonebook.h
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
 * Class definition for FritzBox phonebook.
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright:
 * Copyright (c) 2025, Agundur <info@agundur.de>
 * Released under the GPL-2.0-only OR GPL-3.0-only OR
 * LicenseRef-KDE-Accepted-GPL Original code form:
 * https://github.com/Agundur-KDE/kfritz
 */

#ifndef FRITZPHONEBOOK_H_
#define FRITZPHONEBOOK_H_

#include <QDir>
#include <QObject>
#include <QStringList>

class FritzPhonebook : public QObject {
  Q_OBJECT

 public:
  static FritzPhonebook *instance();

  auto getContacts() -> QHash<QString, QString>;
  QStringList getPhonebookList();

 private:
  explicit FritzPhonebook(QObject *pParent = nullptr);

  QStringList getPhonebookUrlAndName(int id);
  bool downloadPhonebook(int id, const QUrl &url);
  QHash<QString, QString> loadFromFile(const QString &xmlFilePath,
                                       const QString &countryCode);
  QString normalizeNumber(QString number, const QString &countryCode) const;

  QHash<QString, QString> m_Phonebooks;
  static QString m_sSavepath;
};

#endif  // FRITZPHONEBOOK_H_
