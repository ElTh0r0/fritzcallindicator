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

#include <QDialog>
#include <QDir>
#include <QHash>
#include <QObject>

class QSettings;
class QStringListModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class Settings : public QDialog {
  Q_OBJECT

 public:
  explicit Settings(const QDir sharePath, QObject *pParent = nullptr);
  virtual ~Settings();

  QString getHostName() const noexcept { return m_sHostName; }
  uint getCallMonitorPort() const noexcept { return m_nCallMonitorPort; }
  uint getTR064Port() const noexcept { return m_nTR064Port; }
  QString getFritzUser() const noexcept { return m_sFritzUser; }
  QString getFritzPassword() const noexcept { return m_sFritzPassword; }
  uint getRetryInterval() const noexcept { return m_nRetryInterval; }
  uint getPopupTimeout() const noexcept { return m_nPopupTimeout; }
  QString getCountryCode() const noexcept { return m_sCountryCode; }
  QStringList getDisabledOnlineResolvers() const noexcept {
    return m_sListDisabledOnlineResolvers;
  }
  QString resolveOwnNumber(const QString &sNumber) const noexcept {
    return m_OwnNumbers.value(sNumber, "");
  }
  auto getTbAddressbooks() -> const QStringList;
  auto getFritzPhonebooks() -> const QHash<QString, QString>;
  auto getLanguage() -> const QString;
  auto getIconTheme() -> const QString;
  void translateUi();

 public slots:
  void accept() override;

 signals:
  void changedConnectionSettings(const QString &sHostName,
                                 const uint nMonitorPort,
                                 const uint RetryInterval);
  void changedPhonebooks(const QStringList &sListTbAddressbooks,
                         const QHash<QString, QString> &sListFritzPhonebooks);

 protected:
  void showEvent(QShowEvent *pEvent) override;
  void closeEvent(QCloseEvent *event) override;

 private:
  void initOnlineResolvers(QDir sharePath);
  void initFritzPhonebooks(QDir savePath);
  void readSettings();
  auto getThunderbirdProfilePath() -> const QString;

  Ui::SettingsDialog *m_pUi;
  QSettings *m_pSettings;
  QString m_sHostName;
  uint m_nCallMonitorPort;
  uint m_nTR064Port;
  QString m_sFritzUser;
  QString m_sFritzPassword;
  uint m_nRetryInterval;
  uint m_nPopupTimeout;
  QString m_sCountryCode;
  QStringListModel *m_sListModel_TbAddressbooks;
  QHash<QString, QString> m_FritzPhoneBooks;
  QHash<QString, QString> m_OnlineResolvers;
  QStringList m_sListDisabledOnlineResolvers;
  uint m_nMaxOwnNumbers;
  QHash<QString, QString> m_OwnNumbers;
  static const QString DEFAULT_HOST_NAME;
  static const uint DEFAULT_CALL_MONITOR_PORT;
  static const uint DEFAULT_TR064_PORT;
  static const uint DEFAULT_RETRY_INTERVAL_SEC;
  static const uint DEFAULT_POPUP_TIMEOUT_SEC;
  static const QString DEFAULT_COUNTRY_CODE;
  static const uint DEFAULT_MAX_OWN_NUMBERS;
};

#endif  // SETTINGS_H_
