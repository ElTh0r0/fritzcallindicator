/**
 * \file onlineresolvers.h
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
 * Class definition for online number resolvers.
 */

#ifndef ONLINERESOLVERS_H_
#define ONLINERESOLVERS_H_

#include <QFileInfo>
#include <QHash>
#include <QNetworkAccessManager>
#include <QObject>

class OnlineResolvers : public QObject {
  Q_OBJECT
 public:
  explicit OnlineResolvers(QDir sharePath, QObject *pParent = nullptr);
  auto searchOnline(const QString &sNumber, const QString &sCountryCode,
                    const QStringList &sListDisabledResolvers) -> QString;

 private:
  auto parseReply(const QString &sReply,
                  const QHash<QString, QString> &resolver) -> QString;

  QHash<QString, QHash<QString, QString>> m_Resolvers;
  QNetworkAccessManager *m_pNwManager;
};

#endif  // ONLINERESOLVERS_H_
