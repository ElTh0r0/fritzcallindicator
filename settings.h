/**
 * \file settings.h
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
 * Class definition for settings.
 */

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDir>
#include <QSettings>

class Settings : public QObject {
  Q_OBJECT

 public:
  explicit Settings(QObject* pParent = nullptr);
  static Settings* instance();

  // General
  auto getCountryCode() const -> QString;
  auto getPopupTimeout() const -> uint;
  auto getMaxDaysOfOldCalls() const -> uint;
  auto getMaxCallHistory() const -> uint;
  // Connection
  auto getHostName() const -> QString;
  auto getCallMonitorPort() const -> uint;
  auto getTR064Port() const -> uint;
  auto getFritzUser() const -> QString;
  auto getFritzPassword() const -> QString;
  auto getRetryInterval() const -> uint;
  // NumberResolvers
  auto getTbAddressbooks() -> const QStringList;
  auto getEnabledOnlineResolvers() const -> QStringList;
  auto getEnabledFritzPhonebooks() -> const QStringList;
  // PhoneNumbers
  auto getMaxOwnNumbers() const -> uint;
  auto getOwnNumbers() -> QMap<QString, QString>;
  auto setOwnNumbers(const QMap<QString, QString> &ownNumbers) -> void;
  auto resolveOwnNumber(const QString& sNumber) const -> QString;

  static auto getLanguage() -> const QString;
  static auto getIconTheme() -> const QString;

  auto setValue(const QString& sKey, const QVariant& vValue) -> void;

 private:
  QSettings m_settings;
  QMap<QString, QString> m_OwnNumbers;

  static const QString DEFAULT_HOST_NAME;
  static const uint DEFAULT_CALL_MONITOR_PORT;
  static const uint DEFAULT_TR064_PORT;
  static const uint DEFAULT_RETRY_INTERVAL_SEC;
  static const uint DEFAULT_POPUP_TIMEOUT_SEC;
  static const QString DEFAULT_COUNTRY_CODE;
  static const uint DEFAULT_MAX_OWN_NUMBERS;
  static const uint DEFAULT_MAX_DAYS_OLD_CALLS;
  static const uint DEFAULT_MAX_CALL_HISTORY;
};

#endif  // SETTINGS_H_
