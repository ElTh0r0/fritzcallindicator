// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUNDERBIRD_H_
#define THUNDERBIRD_H_

#include <QFileInfo>
#include <QHash>
#include <QObject>

class Thunderbird : public QObject {
  Q_OBJECT

 public:
  static Thunderbird *instance();
  auto getContacts() -> const QHash<QString, QString>;

 private:
  explicit Thunderbird(QObject *pParent = nullptr);

  void importVCards(const QFileInfo &fiDbFile);
  void extractNumber(const QString &sVCard, const QString &sLocalCountryCode);

  QHash<QString, QString> m_PhoneNumbers;
};

#endif  // THUNDERBIRD_H_
