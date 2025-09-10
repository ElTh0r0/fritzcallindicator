/**
 * \file fritzsoap.h
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
 * Class definition for FritzBox SOAP request
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright:
 * Copyright (c) 2025, Agundur <info@agundur.de>
 * Released under the GPL-2.0-only OR GPL-3.0-only OR
 * LicenseRef-KDE-Accepted-GPL Original code form:
 * https://github.com/Agundur-KDE/kfritz
 */

#ifndef FRITZSOAP_H_
#define FRITZSOAP_H_

#include <QDir>
#include <QObject>
#include <QStringList>

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
