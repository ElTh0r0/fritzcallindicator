// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR
// LicenseRef-KDE-Accepted-GPL

#ifndef FRITZPHONEBOOK_H_
#define FRITZPHONEBOOK_H_

#include <QDir>
#include <QObject>
#include <QStringList>

class FritzPhonebook : public QObject {
  Q_OBJECT

 public:
  static FritzPhonebook *instance();

  auto getContacts() -> const QHash<QString, QString>;
  const QStringList getPhonebookList();

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
