/**
 * \file settings.cpp
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
 * Settings class.
 */

#include "settings.h"

#include <QGuiApplication>
#include <QPalette>
#include <QStandardPaths>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

// Ini groups
const QString Settings::GROUP_FRITZBOX = QStringLiteral("FritzBox");
const QString Settings::GROUP_NUMBER_RESOLVERS =
    QStringLiteral("NumberResolvers");
const QString Settings::GROUP_PHONE_NUMBERS = QStringLiteral("PhoneNumbers");
// General
const QString Settings::DEFAULT_COUNTRY_CODE = QStringLiteral("0049");
const uint Settings::DEFAULT_POPUP_TIMEOUT_SEC = 10;
const uint Settings::DEFAULT_MAX_DAYS_OLD_CALLS = 7;
const uint Settings::DEFAULT_MAX_CALL_HISTORY = 10;
const bool Settings::DEFAULT_AUTOSTART = false;
// FritzBox
const QString Settings::DEFAULT_HOST_NAME = QStringLiteral("fritz.box");
const uint Settings::DEFAULT_CALL_MONITOR_PORT = 1012;
const uint Settings::DEFAULT_TR064_PORT = 49000;
const uint Settings::DEFAULT_RETRY_INTERVAL_SEC = 60;
// PhoneNumbers
const uint Settings::DEFAULT_MAX_OWN_NUMBERS = 3;

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

Settings *Settings::instance() {
  static Settings _instance;
  return &_instance;
}

Settings::Settings(QObject *pParent)
    : QObject(pParent),
      m_settings(QSettings::IniFormat, QSettings::UserScope,
                 qApp->applicationName().toLower(),
                 qApp->applicationName().toLower()) {}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// General

auto Settings::getCountryCode() const -> QString {
  return m_settings.value(QStringLiteral("CountryCode"), DEFAULT_COUNTRY_CODE)
      .toString();
}

void Settings::setCountryCode(QString sCountryCode) {
  if (sCountryCode.startsWith('+')) {
    sCountryCode = "00" + sCountryCode.remove('+');
  }
  if (!sCountryCode.startsWith(QStringLiteral("00"))) {
    if (sCountryCode.startsWith('0')) {
      sCountryCode = "0" + sCountryCode;
    } else {
      sCountryCode = "00" + sCountryCode;
    }
  }
  m_settings.setValue(QStringLiteral("CountryCode"), sCountryCode);
}

// ----------------------------------------------------------------------------

auto Settings::getPopupTimeout() const -> uint {
  return m_settings
      .value(QStringLiteral("PopupTimeout"), DEFAULT_POPUP_TIMEOUT_SEC)
      .toUInt();
}

void Settings::setPopupTimeout(const uint nPopupTimeout) {
  m_settings.setValue(QStringLiteral("PopupTimeout"), nPopupTimeout);
}

// ----------------------------------------------------------------------------

auto Settings::getMaxDaysOfOldCalls() const -> uint {
  return m_settings
      .value(QStringLiteral("MaxDaysOfOldCalls"), DEFAULT_MAX_DAYS_OLD_CALLS)
      .toUInt();
}

void Settings::setMaxDaysOfOldCalls(const uint nMaxDays) {
  m_settings.setValue(QStringLiteral("MaxDaysOfOldCalls"), nMaxDays);
}

// ----------------------------------------------------------------------------

auto Settings::getMaxEntriesCallHistory() const -> uint {
  return m_settings
      .value(QStringLiteral("MaxEntriesCallHistory"), DEFAULT_MAX_CALL_HISTORY)
      .toUInt();
}

void Settings::setMaxEntriesCallHistory(const uint nMaxEntries) {
  m_settings.setValue(QStringLiteral("MaxEntriesCallHistory"), nMaxEntries);
}

// ----------------------------------------------------------------------------

auto Settings::getAutostart() const -> bool {
  return m_settings.value(QStringLiteral("Autostart"), DEFAULT_AUTOSTART)
      .toBool();
}

void Settings::setAutostart(const bool bAutostart) {
  m_settings.setValue(QStringLiteral("Autostart"), bAutostart);

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
  QString sAutostartPath =
      QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
      "/autostart/";
  QDir dAutostart(sAutostartPath);
  if (!dAutostart.exists()) {
    dAutostart.mkpath(".");
  }

  QFile fDesktop(sAutostartPath + QString(APP_NAME).toLower() +
                 QStringLiteral(".desktop"));
  if (bAutostart) {
    if (fDesktop.open(QIODevice::WriteOnly)) {
      QByteArray content(
          "[Desktop Entry]\nName=" + QByteArray(APP_NAME).toLower() +
          "\nIcon=" + QByteArray(APP_NAME).toLower() +
          "\nExec=" + QCoreApplication::applicationFilePath().toLatin1() +
          "\nTerminal=false\nType=Application"
          "\nX-GNOME-Autostart-enabled=true\n");
      fDesktop.write(content);
    }
  } else {
    fDesktop.remove();
  }
#elif defined(Q_OS_WIN)
  QSettings regAutostart(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  QSettings regWorkDir(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App "
      "Paths",
      QSettings::NativeFormat);
  if (bAutostart) {
    QString sAppPath =
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    regAutostart.setValue(APP_NAME, sAppPath);
    regWorkDir.beginGroup(QString(APP_NAME) + ".exe");
    regWorkDir.setValue("Path", QCoreApplication::applicationDirPath());
    regWorkDir.endGroup();

  } else {
    regAutostart.remove(APP_NAME);
    regWorkDir.beginGroup(QString(APP_NAME) + ".exe");
    regWorkDir.remove("");
    regWorkDir.endGroup();
  }
#endif
}

auto Settings::isAutostartEnabled() const -> bool {
  bool bEnabled = false;

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
  QString sAutostartPath =
      QStandardPaths::locate(QStandardPaths::GenericConfigLocation,
                             "autostart/", QStandardPaths::LocateDirectory) +
      QString(APP_NAME).toLower() + QStringLiteral(".desktop");
  bEnabled = QFile(sAutostartPath).exists();
#elif defined(Q_OS_WIN)
  QSettings regAutostart(
      "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  bEnabled = regAutostart.value(APP_NAME).toString() ==
             QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
#endif

  return bEnabled;
}

// ----------------------------------------------------------------------------

auto Settings::getNotificationSound() const -> QString {
  QString sFile(
      m_settings.value(QStringLiteral("NotificationSound"), "").toString());
  if (!QFile::exists(sFile)) {
    sFile.clear();
  }
  return sFile;
}

void Settings::setNotificationSound(const QString &sNotificationSound) {
  if (QFile::exists(sNotificationSound)) {
    m_settings.setValue(QStringLiteral("NotificationSound"),
                        sNotificationSound.trimmed());
  } else {
    m_settings.setValue(QStringLiteral("NotificationSound"), "");
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// FritzBox

auto Settings::getHostName() const -> QString {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("HostName"),
             DEFAULT_HOST_NAME)
      .toString();
}

void Settings::setHostName(const QString &sHostName) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("HostName"),
                      sHostName.trimmed());
}

// ----------------------------------------------------------------------------

auto Settings::getCallMonitorPort() const -> uint {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("CallMonitorPort"),
             DEFAULT_CALL_MONITOR_PORT)
      .toUInt();
}

void Settings::setCallMonitorPort(const uint nCallMonitorPort) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("CallMonitorPort"),
                      nCallMonitorPort);
}

// ----------------------------------------------------------------------------

auto Settings::getTR064Port() const -> uint {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("TR064Port"),
             DEFAULT_TR064_PORT)
      .toUInt();
}

void Settings::setTR064Port(const uint nTR064Port) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("TR064Port"),
                      nTR064Port);
}

// ----------------------------------------------------------------------------

auto Settings::getFritzUser() const -> QString {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("FritzUser"), "")
      .toString();
}

void Settings::setFritzUser(const QString &sUser) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("FritzUser"),
                      sUser.trimmed());
}

// ----------------------------------------------------------------------------

auto Settings::getFritzPassword() const -> QString {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("FritzPassword"), "")
      .toString();
}

void Settings::setFritzPassword(const QString &sPassword) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("FritzPassword"),
                      sPassword.trimmed());
}

// ----------------------------------------------------------------------------

auto Settings::getRetryInterval() const -> uint {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("RetryInterval"),
             DEFAULT_RETRY_INTERVAL_SEC)
      .toUInt();
}

void Settings::setRetryInterval(const uint nRetryInterval) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("RetryInterval"),
                      nRetryInterval);
}

// ----------------------------------------------------------------------------

auto Settings::getEnabledFritzPhonebooks() -> const QStringList {
  return m_settings
      .value(GROUP_FRITZBOX + "/" + QStringLiteral("PhoneBooks"), QStringList())
      .toStringList();
}

void Settings::setEnabledFritzPhonebooks(
    const QStringList &sListFritzPhonebooks) {
  m_settings.setValue(GROUP_FRITZBOX + "/" + QStringLiteral("PhoneBooks"),
                      sListFritzPhonebooks);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// NumberResolvers

auto Settings::getTbAddressbooks() -> const QStringList {
  return m_settings
      .value(GROUP_NUMBER_RESOLVERS + "/" + QStringLiteral("TbAddressbooks"),
             QStringList())
      .toStringList();
}

void Settings::setTbAddressbooks(const QStringList &sListTbAddressbooks) {
  m_settings.setValue(
      GROUP_NUMBER_RESOLVERS + "/" + QStringLiteral("TbAddressbooks"),
      sListTbAddressbooks);
}

// ----------------------------------------------------------------------------

auto Settings::getCardDavAddressbooks()
    -> const QList<QHash<QString, QString>> {
  QList<QHash<QString, QString>> addressbooks;
  int size = m_settings.beginReadArray(QStringLiteral("CardDAV"));
  for (int row = 0; row < size; ++row) {
    m_settings.setArrayIndex(row);
    QHash<QString, QString> entry;
    entry[QStringLiteral("URL")] =
        m_settings.value(QStringLiteral("URL"), "").toString();
    entry[QStringLiteral("User")] =
        m_settings.value(QStringLiteral("User"), "").toString();
    entry[QStringLiteral("Password")] =
        m_settings.value(QStringLiteral("Password"), "").toString();
    addressbooks.append(entry);
  }
  m_settings.endArray();
  return addressbooks;
}

void Settings::setCardDavAddressbooks(
    const QList<QHash<QString, QString>> &addressbooks) {
  m_settings.beginGroup(QStringLiteral("CardDAV"));
  m_settings.remove("");  // Delete existing entries
  m_settings.endGroup();

  m_settings.beginWriteArray(QStringLiteral("CardDAV"));
  for (int row = 0; row < addressbooks.size(); ++row) {
    m_settings.setArrayIndex(row);
    const QHash<QString, QString> &entry = addressbooks[row];
    m_settings.setValue(QStringLiteral("URL"),
                        entry.value(QStringLiteral("URL"), ""));
    m_settings.setValue(QStringLiteral("User"),
                        entry.value(QStringLiteral("User"), ""));
    m_settings.setValue(QStringLiteral("Password"),
                        entry.value(QStringLiteral("Password"), ""));
  }
  m_settings.endArray();
}

// ----------------------------------------------------------------------------

auto Settings::getEnabledOnlineResolvers() const -> QStringList {
  return m_settings
      .value(GROUP_NUMBER_RESOLVERS + "/" + QStringLiteral("OnlineResolvers"),
             QStringList())
      .toStringList();
}

void Settings::setEnabledOnlineResolvers(
    const QStringList &sListOnlineResolvers) {
  m_settings.setValue(
      GROUP_NUMBER_RESOLVERS + "/" + QStringLiteral("OnlineResolvers"),
      sListOnlineResolvers);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// PhoneNumbers

auto Settings::getMaxOwnNumbers() const -> uint {
  return m_settings
      .value(GROUP_PHONE_NUMBERS + "/" + QStringLiteral("MaxNumbers"),
             DEFAULT_MAX_OWN_NUMBERS)
      .toUInt();
}

void Settings::setMaxOwnNumbers(const uint nMaxOwnNumbers) {
  m_settings.setValue(GROUP_PHONE_NUMBERS + "/" + QStringLiteral("MaxNumbers"),
                      nMaxOwnNumbers);
}

// ----------------------------------------------------------------------------

auto Settings::getOwnNumbers() -> const QMap<QString, QString> {
  m_OwnNumbers.clear();

  for (uint i = 1; i < this->getMaxOwnNumbers() + 1; i++) {
    QString sTmpNum =
        m_settings
            .value(GROUP_PHONE_NUMBERS + "/" +
                       QStringLiteral("Phone%1_Number").arg(QString::number(i)),
                   "")
            .toString()
            .trimmed();
    if (!sTmpNum.isEmpty()) {
      QString sTmpDesc = m_settings
                             .value(GROUP_PHONE_NUMBERS + "/" +
                                        QStringLiteral("Phone%1_Description")
                                            .arg(QString::number(i)),
                                    "")
                             .toString()
                             .trimmed();
      m_OwnNumbers[sTmpNum] = sTmpDesc;
    }
  }

  return m_OwnNumbers;
}

void Settings::setOwnNumbers(const QMap<QString, QString> &ownNumbers) {
  m_OwnNumbers.clear();
  m_OwnNumbers = ownNumbers;

  int i = 1;
  for (auto entry = m_OwnNumbers.begin(), end = m_OwnNumbers.end();
       entry != end; ++entry) {
    m_settings.setValue(
        GROUP_PHONE_NUMBERS + "/" +
            QStringLiteral("Phone%1_Number").arg(QString::number(i)),
        entry.key());
    m_settings.setValue(
        GROUP_PHONE_NUMBERS + "/" +
            QStringLiteral("Phone%1_Description").arg(QString::number(i)),
        entry.value());
    i++;
  }

  // Delete ini entry if a number was removed
  for (uint j = m_OwnNumbers.size() + 1; j < this->getMaxOwnNumbers() + 1;
       j++) {
    m_settings.setValue(
        GROUP_PHONE_NUMBERS + "/" +
            QStringLiteral("Phone%1_Number").arg(QString::number(j)),
        "");
    m_settings.setValue(
        GROUP_PHONE_NUMBERS + "/" +
            QStringLiteral("Phone%1_Description").arg(QString::number(j)),
        "");
  }
}

// ----------------------------------------------------------------------------

auto Settings::resolveOwnNumber(const QString &sNumber) const -> QString {
  return m_OwnNumbers.value(sNumber, "");
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Misc

auto Settings::getLanguage() -> const QString {
#ifdef Q_OS_UNIX
  QByteArray lang = qgetenv("LANG");
  if (!lang.isEmpty()) {
    return QLocale(QString::fromLatin1(lang)).name();
  }
#endif
  return QLocale::system().name();
}

auto Settings::getIconTheme() -> const QString {
  QString sIconTheme;

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  qDebug() << "Detected color scheme:"
           << QGuiApplication::styleHints()->colorScheme();
  if (Qt::ColorScheme::Dark == QGuiApplication::styleHints()->colorScheme()) {
    sIconTheme = QStringLiteral("dark");
  } else if (Qt::ColorScheme::Light ==
             QGuiApplication::styleHints()->colorScheme()) {
    sIconTheme = QStringLiteral("light");
  }
#endif
  // If < Qt 6.5 or if Qt::ColorScheme::Unknown was returned
  if (sIconTheme.isEmpty()) {
    // If window is darker than text
    QPalette palette;
    if (palette.window().color().lightnessF() <
        palette.windowText().color().lightnessF()) {
      sIconTheme = QStringLiteral("dark");
    } else {
      sIconTheme = QStringLiteral("light");
    }
  }

  return sIconTheme;
}
