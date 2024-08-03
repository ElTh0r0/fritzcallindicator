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
 * Settings dialog.
 */

#include "settings.h"

#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "ui_settings.h"

const QString Settings::DEFAULT_HOST_NAME = QStringLiteral("fritz.box");
const uint Settings::DEFAULT_CALL_MONITOR_PORT = 1012;
const uint Settings::DEFAULT_RETRY_INTERVAL_SEC = 20;
const uint Settings::DEFAULT_POPUP_TIMEOUT_SEC = 10;
const QString Settings::DEFAULT_COUNTRY_CODE = QStringLiteral("0049");
const uint Settings::DEFAULT_MAX_OWN_NUMBERS = 3;

Settings::Settings(QObject *pParent) : m_pUi(new Ui::SettingsDialog()) {
  Q_UNUSED(pParent)
  qDebug() << Q_FUNC_INFO;
  m_pUi->setupUi(this);

#if defined _WIN32
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#else
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#endif

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);

  m_pUi->tableOwnNumbers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  this->readSettings();
}

Settings::~Settings() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readSettings() {
  qDebug() << Q_FUNC_INFO;

  // General
  m_nPopupTimeout =
      m_pSettings
          ->value(QStringLiteral("PopupTimeout"), DEFAULT_POPUP_TIMEOUT_SEC)
          .toUInt();
  m_pUi->spinBoxTimeout->setValue(m_nPopupTimeout);

  m_sCountryCode =
      m_pSettings->value(QStringLiteral("CountryCode"), DEFAULT_COUNTRY_CODE)
          .toString();
  m_pUi->lineEditCountryCode->setText(m_sCountryCode);

  m_sListTbAddressbooks =
      m_pSettings->value(QStringLiteral("TbAddressbooks"), QStringList())
          .toStringList();

  m_pSettings->beginGroup(QStringLiteral("Connection"));
  m_sHostName =
      m_pSettings->value(QStringLiteral("HostName"), DEFAULT_HOST_NAME)
          .toString();
  m_pUi->lineEditHost->setText(m_sHostName);
  m_nPortNumber =
      m_pSettings->value(QStringLiteral("Port"), DEFAULT_CALL_MONITOR_PORT)
          .toUInt();
  m_pUi->spinBoxPort->setValue(m_nPortNumber);
  m_nRetryInterval =
      m_pSettings
          ->value(QStringLiteral("RetryInterval"), DEFAULT_RETRY_INTERVAL_SEC)
          .toUInt();
  m_pSettings->endGroup();

  m_pSettings->beginGroup(QStringLiteral("PhoneNumbers"));
  m_nMaxOwnNumbers =
      m_pSettings->value(QStringLiteral("MaxNumbers"), DEFAULT_MAX_OWN_NUMBERS)
          .toUInt();
  QString sTmpNum;
  QString sTmpDesc;
  int row;
  m_OwnNumbers.clear();
  m_pUi->tableOwnNumbers->clearContents();
  m_pUi->tableOwnNumbers->model()->removeRows(
      0, m_pUi->tableOwnNumbers->rowCount());
  for (uint i = 1; i < m_nMaxOwnNumbers + 1; i++) {
    sTmpNum =
        m_pSettings
            ->value(QStringLiteral("Phone%1_Number").arg(QString::number(i)),
                    "")
            .toString()
            .trimmed();
    if (!sTmpNum.isEmpty()) {
      sTmpDesc =
          m_pSettings
              ->value(
                  QStringLiteral("Phone%1_Description").arg(QString::number(i)),
                  "")
              .toString()
              .trimmed();
      m_OwnNumbers[sTmpNum] = sTmpDesc;

      row = m_pUi->tableOwnNumbers->rowCount();
      m_pUi->tableOwnNumbers->insertRow(row);
      QTableWidgetItem *itemNum = new QTableWidgetItem(sTmpNum);
      QTableWidgetItem *itemDesc = new QTableWidgetItem(sTmpDesc);
      m_pUi->tableOwnNumbers->setItem(row, 0, itemNum);
      m_pUi->tableOwnNumbers->setItem(row, 1, itemDesc);
    }
  }
  m_pSettings->endGroup();

  // Fill table with empty rows
  for (uint i = 0; i < m_nMaxOwnNumbers - m_OwnNumbers.size(); i++) {
    row = m_pUi->tableOwnNumbers->rowCount();
    m_pUi->tableOwnNumbers->insertRow(row);
    QTableWidgetItem *itemNum = new QTableWidgetItem();
    QTableWidgetItem *itemDesc = new QTableWidgetItem();
    m_pUi->tableOwnNumbers->setItem(row, 0, itemNum);
    m_pUi->tableOwnNumbers->setItem(row, 1, itemDesc);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::accept() {
  qDebug() << Q_FUNC_INFO;

  // General
  m_nPopupTimeout = m_pUi->spinBoxTimeout->value();
  m_pSettings->setValue(QStringLiteral("PopupTimeout"), m_nPopupTimeout);

  m_sCountryCode = m_pUi->lineEditCountryCode->text();
  if (m_sCountryCode.startsWith('+')) {
    m_sCountryCode = "00" + m_sCountryCode.remove('+');
  }
  if (!m_sCountryCode.startsWith(QStringLiteral("00"))) {
    if (m_sCountryCode.startsWith('0')) {
      m_sCountryCode = "0" + m_sCountryCode;
    } else {
      m_sCountryCode = "00" + m_sCountryCode;
    }
  }
  m_pSettings->setValue(QStringLiteral("CountryCode"), m_sCountryCode);

  m_pSettings->setValue(QStringLiteral("TbAddressbooks"),
                        m_sListTbAddressbooks);

  m_pSettings->beginGroup(QStringLiteral("Connection"));
  m_sHostName = m_pUi->lineEditHost->text();
  m_pSettings->setValue(QStringLiteral("HostName"), m_sHostName);
  m_nPortNumber = m_pUi->spinBoxPort->value();
  m_pSettings->setValue(QStringLiteral("Port"), m_nPortNumber);
  m_pSettings->setValue(QStringLiteral("RetryInterval"), m_nRetryInterval);
  m_pSettings->endGroup();

  m_OwnNumbers.clear();
  m_pSettings->beginGroup(QStringLiteral("PhoneNumbers"));
  m_pSettings->setValue(QStringLiteral("MaxNumbers"), m_nMaxOwnNumbers);

  QString sTmpNum;
  for (int row = 0; row < m_pUi->tableOwnNumbers->rowCount(); ++row) {
    sTmpNum = m_pUi->tableOwnNumbers->item(row, 0)->text().trimmed();
    if (!sTmpNum.isEmpty()) {
      m_OwnNumbers[sTmpNum] =
          m_pUi->tableOwnNumbers->item(row, 1)->text().trimmed();
    }
  }

  int i = 1;
  for (auto entry = m_OwnNumbers.begin(), end = m_OwnNumbers.end();
       entry != end; ++entry) {
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Number").arg(QString::number(i)), entry.key());
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Description").arg(QString::number(i)),
        entry.value());
    i++;
  }
  // Delete conf entries if row table was cleared
  for (uint i = m_OwnNumbers.size() + 1; i < m_nMaxOwnNumbers + 1; i++) {
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Number").arg(QString::number(i)), "");
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Description").arg(QString::number(i)), "");
  }
  m_pSettings->endGroup();

  emit changedSettings(m_sHostName, m_nPortNumber, m_nRetryInterval);

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLanguage() -> const QString {
#ifdef Q_OS_UNIX
  QByteArray lang = qgetenv("LANG");
  if (!lang.isEmpty()) {
    return QLocale(QString::fromLatin1(lang)).name();
  }
#endif
  return QLocale::system().name();
}

void Settings::translateUi() { m_pUi->retranslateUi(this); }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

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
    if (this->window()->palette().window().color().lightnessF() <
        this->window()->palette().windowText().color().lightnessF()) {
      sIconTheme = QStringLiteral("dark");
    } else {
      sIconTheme = QStringLiteral("light");
    }
  }

  return sIconTheme;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::closeEvent(QCloseEvent *pEvent) {
  QMessageBox::information(this, QStringLiteral(APP_NAME),
                           tr("The program will keep running in the "
                              "system tray. To terminate the program, "
                              "choose <b>Quit</b> in the context menu "
                              "of the system tray entry."));
  hide();
  pEvent->ignore();
}
