// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

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
                    const QStringList &sListEnabledResolvers) -> QString;
  auto getAvailableResolvers() const -> QHash<QString, QString>;

 private:
  auto parseReply(const QString &sReply,
                  const QHash<QString, QString> &resolver) -> QString;

  QHash<QString, QHash<QString, QString>> m_Resolvers;
  QHash<QString, QString> m_ResolverList;
  QNetworkAccessManager *m_pNwManager;
};

#endif  // ONLINERESOLVERS_H_
