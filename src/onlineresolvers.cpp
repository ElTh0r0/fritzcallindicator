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
#include <QStandardPaths>

OnlineResolvers::OnlineResolvers(QDir sharePath, QObject *pParent)
    : QObject{pParent} {
  qDebug() << Q_FUNC_INFO;
  QList<QDir> resolverPaths;

  if (sharePath.cd(QStringLiteral("online_resolvers"))) {
    resolverPaths << sharePath;
  } else {
    qWarning() << "Subfolder 'online_resolvers' not found in share folder:"
               << sharePath;
  }

  // User config directory
  QStringList sListPaths =
      QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation);
  if (sListPaths.isEmpty()) {
    qWarning() << "Error while getting QStandardPaths::AppConfigLocation.";
  } else {
    QDir userConfigPath(sListPaths[0].toLower());
    if (userConfigPath.cd(QStringLiteral("online_resolvers"))) {
      resolverPaths << userConfigPath;
    } else {
      qDebug() << "No custom 'online_resolvers' in" << userConfigPath;
    }
  }

  QStringList sListServices;
  const QStringList requiredKeys = {
      QStringLiteral("Service"), QStringLiteral("CountryCode"),
      QStringLiteral("URL"), QStringLiteral("NameRegExp"),
      QStringLiteral("CityRegExp")};

  for (const auto &path : resolverPaths) {
    const QStringList resolverFiles =
        path.entryList(QStringList() << QStringLiteral("*.conf"),
                       QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

    for (const auto &confFile : resolverFiles) {
      qDebug() << path.absoluteFilePath(confFile);
      QSettings resolver(path.absoluteFilePath(confFile), QSettings::IniFormat);
      QHash<QString, QString> tmpHash;
      bool bIsValid = true;
      QString sService;
      QString sCountryCode;

      for (const QString &sKey : requiredKeys) {
        QString sValue = resolver.value(sKey, "").toString().trimmed();
        if (sValue.isEmpty()) {
          qWarning() << "Invalid online resolver (" << sKey << ") - skipping"
                     << confFile;
          bIsValid = false;
          break;
        }
        tmpHash[sKey] = sValue;
        if (sKey == "Service") sService = sValue;
        if (sKey == "CountryCode") sCountryCode = sValue;
      }

      if (!bIsValid) continue;
      if (sListServices.contains(sService)) {
        qWarning() << "Skipping duplicate (Service)" << confFile;
        continue;
      }

      QString sKeyBase = confFile.chopped(5);
      if (!m_Resolvers.contains(sKeyBase)) {
        m_Resolvers[sKeyBase] = tmpHash;
        m_ResolverList[sKeyBase] = sService + "||" + sCountryCode;
        sListServices << sService;
      } else {
        qWarning() << "Skipping duplicate:" << confFile;
      }
    }
  }
  // qDebug() << m_Resolvers;

  m_pNwManager = new QNetworkAccessManager(this);
  m_pNwManager->setTransferTimeout(500);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto OnlineResolvers::getAvailableResolvers() const -> QHash<QString, QString> {
  return m_ResolverList;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto OnlineResolvers::searchOnline(const QString &sNumber,
                                   const QString &sCountryCode,
                                   const QStringList &sListEnabledResolvers)
    -> QString {
  QHashIterator<QString, QHash<QString, QString>> resolver(m_Resolvers);
  while (resolver.hasNext()) {
    resolver.next();
    if (sCountryCode ==
            resolver.value().value(QStringLiteral("CountryCode"), "") &&
        sListEnabledResolvers.contains(resolver.key())) {
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
        if (QNetworkReply::ContentGoneError != pReply->error() &&
            QNetworkReply::ContentNotFoundError != pReply->error()) {
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

auto OnlineResolvers::parseReply(const QString &sReply,
                                 const QHash<QString, QString> &resolver)
    -> QString {
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
