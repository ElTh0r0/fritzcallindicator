// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

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
