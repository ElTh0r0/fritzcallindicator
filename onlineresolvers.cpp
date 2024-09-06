/**
 * \file onlineresolvers.cpp
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
 * Online numbers resolvers.
 */

#include "onlineresolvers.h"

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QSettings>

OnlineResolvers::OnlineResolvers(QDir sharePath, QObject *pParent)
    : QObject{pParent} {
  qDebug() << Q_FUNC_INFO;

  sharePath.cd(QStringLiteral("online_resolvers"));
  const QStringList resolverFiles =
      sharePath.entryList(QStringList() << QStringLiteral("*.conf"),
                          QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  qDebug() << "Online resolvers:";
  for (const auto &confFile : resolverFiles) {
    qDebug() << sharePath.absoluteFilePath(confFile);
    QSettings resolver(sharePath.absoluteFilePath(confFile),
                       QSettings::IniFormat);
    // TODO: Add found resolvers to settings dialog and give possibility to
    // enable/disable its usage

    QHash<QString, QString> tmpHash;
    tmpHash[QStringLiteral("Service")] =
        resolver.value(QStringLiteral("Service"), "").toString().trimmed();
    tmpHash[QStringLiteral("CountryCode")] =
        resolver.value(QStringLiteral("CountryCode"), "").toString().trimmed();
    tmpHash[QStringLiteral("URL")] =
        resolver.value(QStringLiteral("URL"), "").toString().trimmed();
    tmpHash[QStringLiteral("NameRegExp")] =
        resolver.value(QStringLiteral("NameRegExp"), "").toString().trimmed();
    tmpHash[QStringLiteral("CityRegExp")] =
        resolver.value(QStringLiteral("CityRegExp"), "").toString().trimmed();

    // chopped(5) = "remove" last 5 characters (.conf) and return the result
    m_Resolvers[confFile.chopped(5)] = tmpHash;
    // qDebug() << m_Resolvers;
  }

  m_pNwManager = new QNetworkAccessManager(this);
  m_pNwManager->setTransferTimeout(500);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto OnlineResolvers::searchOnline(
    const QString &sNumber, const QString &sCountryCode,
    const QStringList &sListDisabledResolvers) -> QString {
  QHashIterator<QString, QHash<QString, QString>> resolver(m_Resolvers);
  while (resolver.hasNext()) {
    resolver.next();
    if (sCountryCode ==
            resolver.value().value(QStringLiteral("CountryCode"), "") &&
        !sListDisabledResolvers.contains(resolver.key())) {
      qDebug() << "Searching online:"
               << resolver.value().value(QStringLiteral("Service"), "");
      QString sUrl = resolver.value().value(QStringLiteral("URL"), "");
      if (sUrl.isEmpty()) {
        continue;
      }
      sUrl = sUrl.replace(QStringLiteral("%NUMBER%"), sNumber);

      QEventLoop loop;
      QNetworkReply *pReply = m_pNwManager->get(QNetworkRequest(QUrl(sUrl)));
      connect(pReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
      loop.exec();

      QIODevice *pData(pReply);
      if (QNetworkReply::NoError != pReply->error()) {
        if (QNetworkReply::ContentGoneError != pReply->error()) {
          qWarning() << "Error (#" << pReply->error()
                     << ") while NW reply:" << pData->errorString();
          // qDebug() << "Reply content:" << pReply->readAll();
        }
        continue;
      }

      QString sReplyData = QString::fromUtf8(pData->readAll());
      // qDebug() << parseResult(sReplyData, m_Resolvers["das_oertliche"]);
      pReply->deleteLater();
      QString sResult = parseReply(sReplyData, resolver.value());
      if (!sResult.isEmpty()) {
        return sResult;
      }
    }
  }
  return QString();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto OnlineResolvers::parseReply(
    const QString &sReply, const QHash<QString, QString> &resolver) -> QString {
  QRegularExpressionMatch match;
  // qDebug() << resolver.value(QStringLiteral("NameRegExp"), "");
  QRegularExpression regexp(resolver.value(QStringLiteral("NameRegExp"), ""));
  QString sReturn;

  if ((match = regexp.match(sReply)).hasMatch()) {
    sReturn = match.captured(1).trimmed();
  }

  if (!sReply.isEmpty()) {
    // qDebug() << resolver.value(QStringLiteral("CityRegExp"), "");
    regexp.setPattern(resolver.value(QStringLiteral("CityRegExp"), ""));
    if ((match = regexp.match(sReply)).hasMatch()) {
      sReturn += " (" + match.captured(1).trimmed() + ")";
    }
  }

  return sReturn;
}
