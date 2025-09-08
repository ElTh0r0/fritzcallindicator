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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

const QString Settings::DEFAULT_HOST_NAME = QStringLiteral("fritz.box");
const uint Settings::DEFAULT_CALL_MONITOR_PORT = 1012;
const uint Settings::DEFAULT_TR064_PORT = 49000;
const uint Settings::DEFAULT_RETRY_INTERVAL_SEC = 60;
const uint Settings::DEFAULT_POPUP_TIMEOUT_SEC = 10;
const QString Settings::DEFAULT_COUNTRY_CODE = QStringLiteral("0049");
const uint Settings::DEFAULT_MAX_OWN_NUMBERS = 3;
const uint Settings::DEFAULT_MAX_DAYS_OLD_CALLS = 7;
const uint Settings::DEFAULT_MAX_CALL_HISTORY = 10;

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

auto Settings::getPopupTimeout() const -> uint {
  return m_settings
      .value(QStringLiteral("PopupTimeout"), DEFAULT_POPUP_TIMEOUT_SEC)
      .toUInt();
}

auto Settings::getMaxDaysOfOldCalls() const -> uint {
  return m_settings
      .value(QStringLiteral("MaxDaysOfOldCalls"), DEFAULT_MAX_DAYS_OLD_CALLS)
      .toUInt();
}

auto Settings::getMaxCallHistory() const -> uint {
  return m_settings
      .value(QStringLiteral("MaxEntriesCallHistory"), DEFAULT_MAX_CALL_HISTORY)
      .toUInt();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// Connection

auto Settings::getHostName() const -> QString {
  return m_settings
      .value(QStringLiteral("Connection/HostName"), DEFAULT_HOST_NAME)
      .toString();
}

auto Settings::getCallMonitorPort() const -> uint {
  return m_settings
      .value(QStringLiteral("Connection/CallMonitorPort"),
             DEFAULT_CALL_MONITOR_PORT)
      .toUInt();
}

auto Settings::getTR064Port() const -> uint {
  return m_settings
      .value(QStringLiteral("Connection/TR064Port"), DEFAULT_TR064_PORT)
      .toUInt();
}

auto Settings::getFritzUser() const -> QString {
  return m_settings.value(QStringLiteral("Connection/FritzUser"), "")
      .toString();
}

auto Settings::getFritzPassword() const -> QString {
  return m_settings.value(QStringLiteral("Connection/FritzPassword"), "")
      .toString();
}

auto Settings::getRetryInterval() const -> uint {
  return m_settings
      .value(QStringLiteral("Connection/RetryInterval"),
             DEFAULT_RETRY_INTERVAL_SEC)
      .toUInt();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// NumberResolvers

auto Settings::getTbAddressbooks() -> const QStringList {
  return m_settings
      .value(QStringLiteral("NumberResolvers/TbAddressbooks"), QStringList())
      .toStringList();
}

auto Settings::getEnabledOnlineResolvers() const -> QStringList {
  return m_settings
      .value(QStringLiteral("NumberResolvers/OnlineResolvers"), QStringList())
      .toStringList();
}

auto Settings::getEnabledFritzPhonebooks() -> const QStringList {
  return m_settings
      .value(QStringLiteral("NumberResolvers/FritzPhoneBooks"), QStringList())
      .toStringList();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
// PhoneNumbers

auto Settings::getMaxOwnNumbers() const -> uint {
  return m_settings
      .value(QStringLiteral("PhoneNumbers/MaxNumbers"), DEFAULT_MAX_OWN_NUMBERS)
      .toUInt();
}

auto Settings::getOwnNumbers() -> QMap<QString, QString> {
  m_OwnNumbers.clear();

  for (uint i = 1; i < this->getMaxOwnNumbers() + 1; i++) {
    QString sTmpNum = m_settings
                          .value(QStringLiteral("PhoneNumbers/Phone%1_Number")
                                     .arg(QString::number(i)),
                                 "")
                          .toString()
                          .trimmed();
    if (!sTmpNum.isEmpty()) {
      QString sTmpDesc =
          m_settings
              .value(QStringLiteral("PhoneNumbers/Phone%1_Description")
                         .arg(QString::number(i)),
                     "")
              .toString()
              .trimmed();
      m_OwnNumbers[sTmpNum] = sTmpDesc;
    }
  }

  return m_OwnNumbers;
}

auto Settings::setOwnNumbers(const QMap<QString, QString> &ownNumbers) -> void {
  m_OwnNumbers.clear();
  m_OwnNumbers = ownNumbers;

  int i = 1;
  for (auto entry = m_OwnNumbers.begin(), end = m_OwnNumbers.end();
       entry != end; ++entry) {
    m_settings.setValue(
        QStringLiteral("PhoneNumbers/Phone%1_Number").arg(QString::number(i)),
        entry.key());
    m_settings.setValue(QStringLiteral("PhoneNumbers/Phone%1_Description")
                            .arg(QString::number(i)),
                        entry.value());
    i++;
  }

  // Delete ini entry if a number was removed
  for (uint j = m_OwnNumbers.size() + 1; j < this->getMaxOwnNumbers() + 1;
       j++) {
    m_settings.setValue(
        QStringLiteral("PhoneNumbers/Phone%1_Number").arg(QString::number(j)),
        "");
    m_settings.setValue(QStringLiteral("PhoneNumbers/Phone%1_Description")
                            .arg(QString::number(j)),
                        "");
  }
}

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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::setValue(const QString &sKey, const QVariant &vValue) -> void {
  m_settings.setValue(sKey, vValue);
}
