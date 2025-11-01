// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <QDir>
#include <QSettings>

class Settings : public QObject {
  Q_OBJECT

 public:
  static Settings* instance();

  // General
  auto getCountryCode() const -> QString;
  void setCountryCode(QString sCountryCode);
  auto getPopupTimeout() const -> uint;
  void setPopupTimeout(const uint nPopupTimeout);
  auto getMaxDaysOfOldCalls() const -> uint;
  void setMaxDaysOfOldCalls(const uint nMaxDays);
  auto getMaxEntriesCallHistory() const -> uint;
  void setMaxEntriesCallHistory(const uint nMaxEntries);
  auto getAutostart() const -> bool;
  void setAutostart(const bool bAutostart);
  auto isAutostartEnabled() const -> bool;
  auto getNotificationSound() const -> QString;
  void setNotificationSound(const QString& sNotificationSound);

  // FritzBox
  auto getHostName() const -> QString;
  void setHostName(const QString& sHostName);
  auto getCallMonitorPort() const -> uint;
  void setCallMonitorPort(const uint nCallMonitorPort);
  auto getTR064Port() const -> uint;
  void setTR064Port(const uint nTR064Port);
  auto getFritzUser() const -> QString;
  void setFritzUser(const QString& sUser);
  auto getFritzPassword() const -> QString;
  void setFritzPassword(const QString& sPassword);
  auto getRetryInterval() const -> uint;
  void setRetryInterval(const uint nRetryInterval);
  auto getEnabledFritzPhonebooks() -> const QStringList;
  void setEnabledFritzPhonebooks(const QStringList& sListFritzPhonebooks);

  // NumberResolvers
  auto getTbAddressbooks() -> const QStringList;
  void setTbAddressbooks(const QStringList& sListTbAddressbooks);
  auto getCardDavAddressbooks() -> const QList<QHash<QString, QString>>;
  void setCardDavAddressbooks(
      const QList<QHash<QString, QString>>& addressbooks);
  auto getEnabledOnlineResolvers() const -> QStringList;
  void setEnabledOnlineResolvers(const QStringList& sListOnlineResolvers);

  // PhoneNumbers
  auto getMaxOwnNumbers() const -> uint;
  void setMaxOwnNumbers(const uint nMaxOwnNumbers);
  auto getOwnNumbers() -> const QMap<QString, QString>;
  void setOwnNumbers(const QMap<QString, QString>& ownNumbers);
  auto resolveOwnNumber(const QString& sNumber) const -> QString;

  static auto getLanguage() -> const QString;
  static auto getIconTheme() -> const QString;

 private:
  explicit Settings(QObject* pParent = nullptr);
  QSettings m_settings;
  QMap<QString, QString> m_OwnNumbers;

  // Ini groups
  static const QString GROUP_FRITZBOX;
  static const QString GROUP_NUMBER_RESOLVERS;
  static const QString GROUP_PHONE_NUMBERS;
  // General
  static const QString DEFAULT_COUNTRY_CODE;
  static const uint DEFAULT_POPUP_TIMEOUT_SEC;
  static const uint DEFAULT_MAX_DAYS_OLD_CALLS;
  static const uint DEFAULT_MAX_CALL_HISTORY;
  static const bool DEFAULT_AUTOSTART;
  // FritzBox
  static const QString DEFAULT_HOST_NAME;
  static const uint DEFAULT_CALL_MONITOR_PORT;
  static const uint DEFAULT_TR064_PORT;
  static const uint DEFAULT_RETRY_INTERVAL_SEC;
  // PhoneNumbers
  static const uint DEFAULT_MAX_OWN_NUMBERS;
};

#endif  // SETTINGS_H_
