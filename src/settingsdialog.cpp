/**
 * \file settingsdialog.cpp
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

#include "settingsdialog.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDomDocument>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QStringListModel>
#ifdef FRITZ_USE_NOTIFICATION_SOUND
#include <QMediaFormat>
#endif

#include "phonebooks/fritzphonebook.h"
#include "settings.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(const QDir sharePath, QObject *pParent)
    : m_pUi(new Ui::SettingsDialog()) {
  Q_UNUSED(pParent)
  qDebug() << Q_FUNC_INFO;
  m_pUi->setupUi(this);
  m_pUi->tabWidget->setCurrentIndex(0);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &SettingsDialog::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);

  m_pUi->tableOwnNumbers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  m_pUi->tableFritzPhonebooks->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);

#ifdef FRITZ_USE_NOTIFICATION_SOUND
#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
  connect(m_pUi->cbNotificationSound, &QCheckBox::stateChanged, [=](int state) {
    Q_UNUSED(state);
#else
  connect(m_pUi->cbNotificationSound, &QCheckBox::checkStateChanged, [=]() {
#endif
    if (m_pUi->cbNotificationSound->isChecked()) {
      m_pUi->lineEditNotification->setEnabled(true);
    } else {
      m_pUi->lineEditNotification->clear();
      m_pUi->lineEditNotification->setEnabled(false);
    }
  });
  m_pUi->lineEditNotification->installEventFilter(this);

  // Supported audio formats
  QMediaFormat format;
  const QList<QMediaFormat::FileFormat> supportedFormats =
      format.supportedFileFormats(QMediaFormat::Decode);
  for (const auto fileFormat : supportedFormats) {
    if (fileFormat == QMediaFormat::FileFormat::MP3)
      m_sListSupportedAudioFormats << QStringLiteral("*.mp3");
    else if (fileFormat == QMediaFormat::FileFormat::Wave)
      m_sListSupportedAudioFormats << QStringLiteral("*.wav");
    else if (fileFormat == QMediaFormat::FileFormat::Ogg)
      m_sListSupportedAudioFormats << QStringLiteral("*.ogg")
                                   << QStringLiteral("*.oga");
    else if (fileFormat == QMediaFormat::FileFormat::AAC)
      m_sListSupportedAudioFormats << QStringLiteral("*.aac");
    else if (fileFormat == QMediaFormat::FileFormat::FLAC)
      m_sListSupportedAudioFormats << QStringLiteral("*.flac");
    else if (fileFormat == QMediaFormat::FileFormat::Mpeg4Audio)
      m_sListSupportedAudioFormats << QStringLiteral("*.m4a");
    else if (fileFormat == QMediaFormat::FileFormat::WMA)
      m_sListSupportedAudioFormats << QStringLiteral("*.wma");
  }

#else
  m_pUi->lblNotificationSound->hide();
  m_pUi->cbNotificationSound->hide();
  m_pUi->lineEditNotification->hide();
#endif

#ifdef FRITZ_USE_THUNDERBIRD_ADDRESSBOOK
  m_sListModel_TbAddressbooks = new QStringListModel(this);
  m_pUi->listView_TbAddressbooks->setModel(m_sListModel_TbAddressbooks);

  connect(m_pUi->toolButton_AddTbAddressbook, &QToolButton::clicked, [=]() {
    QString sFile =
        QFileDialog::getOpenFileName(this, tr("Select Thunderbird addressbook"),
                                     this->getThunderbirdProfilePath(),
                                     tr("SQLite abook files (abook*.sqlite)"));
    if (!sFile.isEmpty() &&
        !m_sListModel_TbAddressbooks->stringList().contains(sFile)) {
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
#else
  m_pUi->lblTbAddressbooks->hide();
  m_pUi->listView_TbAddressbooks->hide();
  m_pUi->toolButton_AddTbAddressbook->hide();
  m_pUi->toolButton_RemoveTbAddressbook->hide();
  delete m_pUi->horizontalLayoutThunderbird;
  m_pUi->lineCarddav->hide();
#endif

#ifdef FRITZ_USE_CARDDAV_ADDRESSBOOK
  m_pUi->tableCardDav->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  connect(m_pUi->toolButton_AddCardDavAddressbook, &QToolButton::clicked,
          [=]() {
            int row = m_pUi->tableCardDav->rowCount();
            m_pUi->tableCardDav->insertRow(row);
            for (int col = 0; col < m_pUi->tableCardDav->columnCount(); ++col)
              m_pUi->tableCardDav->setItem(row, col, new QTableWidgetItem(""));
            m_pUi->tableCardDav->setCurrentCell(
                row, 0);  // Focus first cell of new row
          });
  connect(
      m_pUi->toolButton_RemoveCardDavAddressbook, &QToolButton::clicked, [=]() {
        QModelIndex index = m_pUi->tableCardDav->currentIndex();
        if (index.isValid() && index.row() < m_pUi->tableCardDav->rowCount()) {
          m_pUi->tableCardDav->removeRow(index.row());
        }
      });
#else
  m_pUi->lineCarddav->hide();
  m_pUi->lblCarddav->hide();
  m_pUi->tableCardDav->hide();
  m_pUi->toolButton_AddCardDavAddressbook->hide();
  m_pUi->toolButton_RemoveCardDavAddressbook->hide();
  delete m_pUi->horizontalLayoutCardDav;
  m_pUi->lineOnlineResolvers->hide();
#endif

  this->readSettings();
  this->initFritzPhonebooks();  // After readSettings!
#ifdef FRITZ_USE_ONLINE_RESOLVERS
  m_pUi->tableOnlineResolvers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  this->initOnlineResolvers(sharePath);  // After readSettings!
#else
  m_pUi->lineOnlineResolvers->hide();
  m_pUi->lblOnlineResolvers->hide();
  m_pUi->tableOnlineResolvers->hide();
#endif

#if !defined(FRITZ_USE_ONLINE_RESOLVERS) &&        \
    !defined(FRITZ_USE_THUNDERBIRD_ADDRESSBOOK) && \
    !defined(FRITZ_USE_CARDDAV_ADDRESSBOOK)
  m_pUi->tabWidget->removeTab(m_pUi->tabWidget->indexOf(m_pUi->tab_resolver));
#endif

  bool bAutostart = Settings().getAutostart();
  if (bAutostart != Settings().isAutostartEnabled()) {
    Settings().setAutostart(bAutostart);
  }
}

SettingsDialog::~SettingsDialog() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef FRITZ_USE_NOTIFICATION_SOUND
bool SettingsDialog::eventFilter(QObject *pObj, QEvent *pEvent) {
  if (m_pUi->cbNotificationSound->isChecked() &&
      pObj == m_pUi->lineEditNotification &&
      pEvent->type() == QEvent::MouseButtonPress) {
    QString sFilepath = m_pUi->lineEditNotification->text().trimmed();
    if (!sFilepath.isEmpty() && QFileInfo::exists(sFilepath)) {
      sFilepath = QFileInfo(sFilepath).absolutePath();
    }
    sFilepath = QFileDialog::getOpenFileName(
        this, tr("Select notification sound for incoming calls"), sFilepath,
        tr("Supported audio formats") + QStringLiteral(" (") +
            m_sListSupportedAudioFormats.join(' ') + QStringLiteral(");;") +
            tr("All files") + QStringLiteral(" (*.*)"));
    if (!sFilepath.isEmpty()) {
      m_pUi->lineEditNotification->setText(sFilepath);
    }
    return true;  // Event was processed
  }
  return QObject::eventFilter(pObj, pEvent);
}
#endif

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#ifdef FRITZ_USE_ONLINE_RESOLVERS
void SettingsDialog::initOnlineResolvers(QDir sharePath) {
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
    if (m_sListEnabledOnlineResolvers.contains(confFile.chopped(5))) {
      itemEnabled->setCheckState(Qt::Checked);
    } else {
      itemEnabled->setCheckState(Qt::Unchecked);
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
#endif

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::initFritzPhonebooks() {
  qDebug() << Q_FUNC_INFO;

  for (const auto &sPhonebook :
       FritzPhonebook::instance()->getPhonebookList()) {
    int row = m_pUi->tableFritzPhonebooks->rowCount();
    m_pUi->tableFritzPhonebooks->insertRow(row);

    QTableWidgetItem *itemEnabled = new QTableWidgetItem();
    itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    if (m_sListEnabledFritzPhoneBooks.contains(sPhonebook)) {
      itemEnabled->setCheckState(Qt::Checked);
    } else {
      itemEnabled->setCheckState(Qt::Unchecked);
    }

    QTableWidgetItem *itemName = new QTableWidgetItem(sPhonebook);
    m_pUi->tableFritzPhonebooks->setItem(row, 0, itemEnabled);
    m_pUi->tableFritzPhonebooks->setItem(row, 1, itemName);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::readSettings() {
  qDebug() << Q_FUNC_INFO;
  Settings settings;

  // General
  m_pUi->cbAutostart->setChecked(settings.getAutostart());
  m_pUi->lineEditCountryCode->setText(settings.getCountryCode());
  m_pUi->spinBoxMaxEntriesCallHistory->setValue(
      settings.getMaxEntriesCallHistory());
  m_pUi->spinBoxMaxDaysOfOldCalls->setValue(settings.getMaxDaysOfOldCalls());
  m_pUi->spinBoxTimeout->setValue(settings.getPopupTimeout());
#ifdef FRITZ_USE_NOTIFICATION_SOUND
  QString sSound(settings.getNotificationSound());
  if (sSound.isEmpty()) {
    m_pUi->cbNotificationSound->setChecked(false);
    m_pUi->lineEditNotification->clear();
    m_pUi->lineEditNotification->setEnabled(false);
  } else {
    m_pUi->cbNotificationSound->setChecked(true);
    m_pUi->lineEditNotification->setText(sSound);
    m_pUi->lineEditNotification->setEnabled(true);
  }
#endif

  // FritzBox
  m_pUi->lineEditHost->setText(settings.getHostName());
  m_pUi->spinBoxCallMonitorPort->setValue(settings.getCallMonitorPort());
  m_pUi->spinBoxTR064Port->setValue(settings.getTR064Port());
  m_pUi->lineEditUserName->setText(settings.getFritzUser());
  m_pUi->lineEditPassword->setText(settings.getFritzPassword());
  m_pUi->spinBoxRetryInterval->setValue(settings.getRetryInterval());
  m_sListEnabledFritzPhoneBooks = settings.getEnabledFritzPhonebooks();
  for (int row = 0; row < m_pUi->tableFritzPhonebooks->rowCount(); ++row) {
    if (m_sListEnabledFritzPhoneBooks.contains(
            m_pUi->tableFritzPhonebooks->item(row, 1)->text())) {
      m_pUi->tableFritzPhonebooks->item(row, 0)->setCheckState(Qt::Checked);
    } else {
      m_pUi->tableFritzPhonebooks->item(row, 0)->setCheckState(Qt::Unchecked);
    }
  }

// NumberResolvers
#ifdef FRITZ_USE_THUNDERBIRD_ADDRESSBOOK
  m_sListModel_TbAddressbooks->setStringList(settings.getTbAddressbooks());
#endif

#ifdef FRITZ_USE_CARDDAV_ADDRESSBOOK
  static const QStyle *style = QApplication::style();
  static const QChar maskChar = static_cast<QChar>(
      style->styleHint(QStyle::SH_LineEdit_PasswordCharacter));
  const QList<QHash<QString, QString>> &carddav =
      settings.getCardDavAddressbooks();
  m_pUi->tableCardDav->setRowCount(0);
  for (int i = 0; i < carddav.size(); ++i) {
    m_pUi->tableCardDav->insertRow(i);
    m_pUi->tableCardDav->setItem(
        i, 0, new QTableWidgetItem(carddav[i].value(QStringLiteral("URL"))));
    m_pUi->tableCardDav->setItem(
        i, 1, new QTableWidgetItem(carddav[i].value(QStringLiteral("User"))));
    QString sPassword = QString(maskChar).repeated(3);
    QTableWidgetItem *pwItem = new QTableWidgetItem(sPassword);
    pwItem->setData(Qt::UserRole, carddav[i].value(QStringLiteral("Password")));
    m_pUi->tableCardDav->setItem(i, 2, pwItem);
  }
#endif

#ifdef FRITZ_USE_ONLINE_RESOLVERS
  m_sListEnabledOnlineResolvers = settings.getEnabledOnlineResolvers();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (m_sListEnabledOnlineResolvers.contains(m_OnlineResolvers.value(
            m_pUi->tableOnlineResolvers->item(row, 1)->text()))) {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Checked);
    } else {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Unchecked);
    }
  }
#endif

  // PhoneNumbers
  uint nMaxOwnNumbers = settings.getMaxOwnNumbers();
  QMap<QString, QString> ownNumbers = settings.getOwnNumbers();

  // Fill table with own numbers
  uint row = 0;
  m_pUi->tableOwnNumbers->clearContents();
  m_pUi->tableOwnNumbers->model()->removeRows(
      0, m_pUi->tableOwnNumbers->rowCount());
  QMapIterator<QString, QString> i(ownNumbers);
  while (i.hasNext()) {
    i.next();
    if (row + 1 > nMaxOwnNumbers) break;

    if (!i.key().isEmpty()) {
      m_pUi->tableOwnNumbers->insertRow(row);
      QTableWidgetItem *itemNum = new QTableWidgetItem(i.key());
      QTableWidgetItem *itemDesc = new QTableWidgetItem(i.value());
      m_pUi->tableOwnNumbers->setItem(row, 0, itemNum);
      m_pUi->tableOwnNumbers->setItem(row, 1, itemDesc);
      row++;
    }
  }

  // Fill table with empty rows
  for (uint j = 0; j < nMaxOwnNumbers - ownNumbers.size(); j++) {
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

void SettingsDialog::accept() {
  qDebug() << Q_FUNC_INFO;
  Settings settings;
  bool bAddressbookChanged = false;
  bool bFritzUserPasswordChanged = false;

  // General
  settings.setCountryCode(m_pUi->lineEditCountryCode->text().trimmed());
  settings.setPopupTimeout(m_pUi->spinBoxTimeout->value());
  settings.setMaxEntriesCallHistory(
      m_pUi->spinBoxMaxEntriesCallHistory->value());
  settings.setMaxDaysOfOldCalls(m_pUi->spinBoxMaxDaysOfOldCalls->value());
  settings.setAutostart(m_pUi->cbAutostart->isChecked());
#ifdef FRITZ_USE_NOTIFICATION_SOUND
  QString sSound(m_pUi->lineEditNotification->text().trimmed());
  if (m_pUi->cbNotificationSound->isChecked() && !sSound.isEmpty()) {
    settings.setNotificationSound(sSound);
  } else {
    settings.setNotificationSound("");
  }
#endif

  // FritzBox
  settings.setHostName(m_pUi->lineEditHost->text().trimmed());
  settings.setCallMonitorPort(m_pUi->spinBoxCallMonitorPort->value());
  settings.setTR064Port(m_pUi->spinBoxTR064Port->value());
  QString sTmpUserPw = m_pUi->lineEditUserName->text().trimmed();
  if (sTmpUserPw != settings.getFritzUser()) {
    settings.setFritzUser(sTmpUserPw);
    bFritzUserPasswordChanged = true;
  }
  sTmpUserPw = m_pUi->lineEditPassword->text().trimmed();
  if (sTmpUserPw != settings.getFritzPassword()) {
    settings.setFritzPassword(sTmpUserPw);
    bFritzUserPasswordChanged = true;
  }
  settings.setRetryInterval(m_pUi->spinBoxRetryInterval->value());
  m_sListEnabledFritzPhoneBooks.clear();
  for (int row = 0; row < m_pUi->tableFritzPhonebooks->rowCount(); ++row) {
    if (Qt::Checked ==
        m_pUi->tableFritzPhonebooks->item(row, 0)->checkState()) {
      m_sListEnabledFritzPhoneBooks
          << m_pUi->tableFritzPhonebooks->item(row, 1)->text();
    }
  }
  if (settings.getEnabledFritzPhonebooks() != m_sListEnabledFritzPhoneBooks) {
    settings.setEnabledFritzPhonebooks(m_sListEnabledFritzPhoneBooks);
    bAddressbookChanged = true;
  }

// NumberResolvers
#ifdef FRITZ_USE_THUNDERBIRD_ADDRESSBOOK
  if (settings.getTbAddressbooks() !=
      m_sListModel_TbAddressbooks->stringList()) {
    settings.setTbAddressbooks(m_sListModel_TbAddressbooks->stringList());
    bAddressbookChanged = true;
  }
#endif

#ifdef FRITZ_USE_CARDDAV_ADDRESSBOOK
  static const QStyle *style = QApplication::style();
  static const QChar maskChar = static_cast<QChar>(
      style->styleHint(QStyle::SH_LineEdit_PasswordCharacter));
  QList<QHash<QString, QString>> carddav;
  for (int r = 0; r < m_pUi->tableCardDav->rowCount(); ++r) {
    QHash<QString, QString> entry;
    entry[QStringLiteral("URL")] = m_pUi->tableCardDav->item(r, 0)
                                       ? m_pUi->tableCardDav->item(r, 0)->text()
                                       : "";
    entry[QStringLiteral("User")] =
        m_pUi->tableCardDav->item(r, 1)
            ? m_pUi->tableCardDav->item(r, 1)->text()
            : "";
    QString sPassword = m_pUi->tableCardDav->item(r, 2)
                            ? m_pUi->tableCardDav->item(r, 2)->text()
                            : "";
    if (sPassword.contains(maskChar)) {
      sPassword =
          m_pUi->tableCardDav->item(r, 2)
              ? m_pUi->tableCardDav->item(r, 2)->data(Qt::UserRole).toString()
              : "";
    }
    entry[QStringLiteral("Password")] = sPassword;
    carddav.append(entry);
  }
  if (carddav != settings.getCardDavAddressbooks()) {
    settings.setCardDavAddressbooks(carddav);
    bAddressbookChanged = true;
  }
#endif

#ifdef FRITZ_USE_ONLINE_RESOLVERS
  m_sListEnabledOnlineResolvers.clear();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (Qt::Checked ==
        m_pUi->tableOnlineResolvers->item(row, 0)->checkState()) {
      m_sListEnabledOnlineResolvers << m_OnlineResolvers.value(
          m_pUi->tableOnlineResolvers->item(row, 1)->text());
    }
  }
  settings.setEnabledOnlineResolvers(m_sListEnabledOnlineResolvers);
#endif

  // PhoneNumbers
  settings.setMaxOwnNumbers(
      settings.getMaxOwnNumbers());  // TODO: Add to dialog
  QMap<QString, QString> ownNumbers;
  for (int row = 0; row < m_pUi->tableOwnNumbers->rowCount(); ++row) {
    QString sTmpNum = m_pUi->tableOwnNumbers->item(row, 0)->text().trimmed();
    if (!sTmpNum.isEmpty()) {
      ownNumbers[sTmpNum] =
          m_pUi->tableOwnNumbers->item(row, 1)->text().trimmed();
    }
  }
  settings.setOwnNumbers(ownNumbers);

  emit changedConnectionSettings(m_pUi->lineEditHost->text().trimmed(),
                                 m_pUi->spinBoxCallMonitorPort->value(),
                                 m_pUi->spinBoxRetryInterval->value());

  if (bAddressbookChanged) {
    emit changedPhonebooks();
  }

  if (bFritzUserPasswordChanged) {
    QMessageBox::information(
        this, QCoreApplication::applicationName(),
        tr("Please restart the application to apply the changes."));
  }

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto SettingsDialog::getThunderbirdProfilePath() -> const QString {
  QString sTbPath =
      QStandardPaths::locate(QStandardPaths::HomeLocation, ".thunderbird",
                             QStandardPaths::LocateDirectory);
#ifndef Q_OS_UNIX
  QDir dRoaming(
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::translateUi() { m_pUi->retranslateUi(this); }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::closeEvent(QCloseEvent *pEvent) {
  QMessageBox::information(this, QStringLiteral(APP_NAME),
                           tr("The program will keep running in the "
                              "system tray. To terminate the program, "
                              "choose <b>Quit</b> in the context menu "
                              "of the system tray entry."));
  hide();
  pEvent->ignore();
}
