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

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QStringListModel>

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

Settings::Settings(const QDir sharePath, QObject *pParent)
    : m_pUi(new Ui::SettingsDialog()) {
  Q_UNUSED(pParent)
  qDebug() << Q_FUNC_INFO;
  m_pUi->setupUi(this);
  m_pUi->tabWidget->setCurrentIndex(0);

#if defined _WIN32
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#else
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#endif

  m_sListModel_TbAddressbooks = new QStringListModel(this);
  m_pUi->listView_TbAddressbooks->setModel(m_sListModel_TbAddressbooks);

  m_pUi->tableOwnNumbers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);

  connect(m_pUi->toolButton_AddTbAddressbook, &QToolButton::clicked, [=]() {
    QString sFile =
        QFileDialog::getOpenFileName(this, tr("Select Thunderbird addressbook"),
                                     this->getThunderbirdProfilePath(),
                                     tr("SQLite abook files (abook*.sqlite)"));
    if (!sFile.isEmpty()) {
      if (m_sListModel_TbAddressbooks->insertRow(
              m_sListModel_TbAddressbooks->rowCount())) {
        QModelIndex index = m_sListModel_TbAddressbooks->index(
            m_sListModel_TbAddressbooks->rowCount() - 1, 0);
        m_sListModel_TbAddressbooks->setData(index, sFile);
      }
    }
  });

  connect(m_pUi->toolButton_RemoveTbAddressbook, &QToolButton::clicked, [=]() {
    QModelIndex index = m_pUi->listView_TbAddressbooks->currentIndex();
    if (index.isValid() &&
        index.row() < m_sListModel_TbAddressbooks->stringList().size()) {
      m_sListModel_TbAddressbooks->removeRows(index.row(), 1);
    }
  });

  this->readSettings();
  this->initOnlineResolvers(sharePath);  // After readSettings!
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

void Settings::initOnlineResolvers(QDir sharePath) {
  qDebug() << Q_FUNC_INFO;

  if (!sharePath.cd(QStringLiteral("online_resolvers"))) {
    qWarning() << "Subfolder 'online_resolvers' not found!";
  }
  const QStringList resolverFiles =
      sharePath.entryList(QStringList() << QStringLiteral("*.conf"),
                          QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
  for (const auto &confFile : resolverFiles) {
    QSettings resolver(sharePath.absoluteFilePath(confFile),
                       QSettings::IniFormat);

    int row = m_pUi->tableOnlineResolvers->rowCount();
    m_pUi->tableOnlineResolvers->insertRow(row);

    QTableWidgetItem *itemEnabled = new QTableWidgetItem();
    itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    if (m_sListDisabledOnlineResolvers.contains(confFile.chopped(5))) {
      itemEnabled->setCheckState(Qt::Unchecked);
    } else {
      itemEnabled->setCheckState(Qt::Checked);
    }

    QTableWidgetItem *itemService = new QTableWidgetItem(
        resolver.value(QStringLiteral("Service"), "").toString().trimmed());
    QTableWidgetItem *itemCountry = new QTableWidgetItem(
        resolver.value(QStringLiteral("CountryCode"), "").toString().trimmed());
    m_pUi->tableOnlineResolvers->setItem(row, 0, itemEnabled);
    m_pUi->tableOnlineResolvers->setItem(row, 1, itemService);
    m_pUi->tableOnlineResolvers->setItem(row, 2, itemCountry);
    // chopped(5) = "remove" last 5 characters (.conf) and return the result
    m_OnlineResolvers
        [resolver.value(QStringLiteral("Service"), "").toString().trimmed()] =
            confFile.chopped(5);
  }
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

  QStringList sListTbAddressbooks =
      m_pSettings->value(QStringLiteral("TbAddressbooks"), QStringList())
          .toStringList();
  m_sListModel_TbAddressbooks->setStringList(sListTbAddressbooks);

  m_sListDisabledOnlineResolvers =
      m_pSettings
          ->value(QStringLiteral("DisabledOnlineResolvers"), QStringList())
          .toStringList();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (m_sListDisabledOnlineResolvers.contains(m_OnlineResolvers.value(
            m_pUi->tableOnlineResolvers->item(row, 1)->text()))) {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Unchecked);
    } else {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Checked);
    }
  }

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

  int row;
  m_OwnNumbers.clear();
  m_pUi->tableOwnNumbers->clearContents();
  m_pUi->tableOwnNumbers->model()->removeRows(
      0, m_pUi->tableOwnNumbers->rowCount());
  for (uint i = 1; i < m_nMaxOwnNumbers + 1; i++) {
    QString sTmpNum =
        m_pSettings
            ->value(QStringLiteral("Phone%1_Number").arg(QString::number(i)),
                    "")
            .toString()
            .trimmed();
    if (!sTmpNum.isEmpty()) {
      QString sTmpDesc =
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
                        m_sListModel_TbAddressbooks->stringList());
  m_sListDisabledOnlineResolvers.clear();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (Qt::Checked !=
        m_pUi->tableOnlineResolvers->item(row, 0)->checkState()) {
      m_sListDisabledOnlineResolvers << m_OnlineResolvers.value(
          m_pUi->tableOnlineResolvers->item(row, 1)->text());
    }
  }
  m_pSettings->setValue(QStringLiteral("DisabledOnlineResolvers"),
                        m_sListDisabledOnlineResolvers);

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

  for (int row = 0; row < m_pUi->tableOwnNumbers->rowCount(); ++row) {
    QString sTmpNum = m_pUi->tableOwnNumbers->item(row, 0)->text().trimmed();
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
  for (uint j = m_OwnNumbers.size() + 1; j < m_nMaxOwnNumbers + 1; j++) {
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Number").arg(QString::number(j)), "");
    m_pSettings->setValue(
        QStringLiteral("Phone%1_Description").arg(QString::number(j)), "");
  }
  m_pSettings->endGroup();

  emit changedConnectionSettings(m_sHostName, m_nPortNumber, m_nRetryInterval);
  emit changedTbAddressbooks(m_sListModel_TbAddressbooks->stringList());

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getThunderbirdProfilePath() -> const QString {
  QString sTbPath =
      QStandardPaths::locate(QStandardPaths::HomeLocation, ".thunderbird",
                             QStandardPaths::LocateDirectory);
#ifndef Q_OS_UNIX
  QDir dRoaming(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
  // AppDataLocation returns ".../AppData/Roaming/FritzCallIndicator"
  if (dRoaming.cd("../Thunderbird")) {
    sTbPath = dRoaming.absolutePath();
  }
#endif

  if (!sTbPath.isEmpty()) {
    // Find current default profile sub folder
    QFileInfo fiProfilesIni(sTbPath + "/profiles.ini");

    if (fiProfilesIni.exists()) {
      QSettings profiles(fiProfilesIni.absoluteFilePath(),
                         QSettings::IniFormat);
      const QStringList sListGroups = profiles.childGroups();

      for (const auto &sGroup : sListGroups) {
        profiles.beginGroup(sGroup);
        if (profiles.contains("Default")) {
          QString sSubfolder = profiles.value("Path", "").toString();
          if (!sSubfolder.isEmpty()) {
            sTbPath += "/" + sSubfolder;
            break;
          }
        }
        profiles.endGroup();
      }
    }
  }

  return sTbPath;
}

auto Settings::getTbAddressbooks() -> const QStringList {
  return m_sListModel_TbAddressbooks->stringList();
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
