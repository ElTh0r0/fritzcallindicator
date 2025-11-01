// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NUMBERRESOLVER_H_
#define NUMBERRESOLVER_H_

#include <QDir>
#include <QHash>
#include <QMultiHash>
#include <QObject>

#ifdef FRITZ_USE_ONLINE_RESOLVERS
#include "onlineresolvers.h"
#endif

class NumberResolver : public QObject {
  Q_OBJECT
 public:
  explicit NumberResolver(const QDir &sharePath,
                          const QString &sLocalCountryCode,
                          QObject *pParent = nullptr);
  auto resolveNumber(const QString &sNumber,
                     const QStringList &sListEnabledResolvers) const -> QString;
  auto getAvailableResolvers() const -> QHash<QString, QString>;

 public slots:
  void readPhonebooks();

 private:
  void initCountryCodes(const QDir &sharePath);
  void initAreaCodes(QDir sharePath);

  QString m_sLocalCountryCode;
  QHash<QString, QString> m_CountryCodes;
  QHash<QString, QHash<QString, QString>> m_AreaCodes;
  QHash<QString, QString> m_KnownContacts;
  const QChar CSV_SEPARATOR;

#ifdef FRITZ_USE_ONLINE_RESOLVERS
  OnlineResolvers *m_pOnlineResolvers;
#endif
};

#endif  // NUMBERRESOLVER_H_
