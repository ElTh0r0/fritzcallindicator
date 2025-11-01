// SPDX-FileCopyrightText: 2025 Thorsten Roth
// SPDX-FileCopyrightText: 2025 Agundur <info@agundur.de>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR
// LicenseRef-KDE-Accepted-GPL

#ifndef FRITZSOAP_H_
#define FRITZSOAP_H_

#include <QObject>

class FritzSOAP : public QObject {
  Q_OBJECT

 public:
  static FritzSOAP *instance();

  QString sendRequest(const QString &service, const QString &action,
                      const QString &body, const QString &controlUrl);

 private:
  explicit FritzSOAP(QObject *pParent = nullptr);
};

#endif  // FRITZSOAP_H_
