/**
 * \file numberresolver.h
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
 * Class definition phone number resolver
 */

#ifndef NUMBERRESOLVER_H_
#define NUMBERRESOLVER_H_

#include <QDir>
#include <QHash>
#include <QMultiHash>
#include <QObject>

class NumberResolver : public QObject {
  Q_OBJECT
 public:
  explicit NumberResolver(const QDir &sharePath,
                          const QString &sLocalCountryCode,
                          QObject *pParent = nullptr);
  auto resolveNumber(const QString &sNumber) const -> QString;

 public slots:
  void readTbPhonebooks(const QStringList &sListTbAddressbooks);

 private:
  void initCountryCodes(const QDir &sharePath);
  void initAreaCodes(QDir sharePath);

  QString m_sLocalCountryCode;
  QHash<QString, QString> m_CountryCodes;
  QHash<QString, QHash<QString, QString>> m_AreaCodes;
  QHash<QString, QString> m_KnownContacts;
  const QChar CSV_SEPARATOR;
};

#endif  // NUMBERRESOLVER_H_
