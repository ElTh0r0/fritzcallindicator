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

#include "settings.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(const QDir sharePath, QObject *pParent)
    : m_pUi(new Ui::SettingsDialog()) {
  Q_UNUSED(pParent)
  qDebug() << Q_FUNC_INFO;
  m_pUi->setupUi(this);
  m_pUi->tabWidget->setCurrentIndex(0);

  m_sListModel_TbAddressbooks = new QStringListModel(this);
  m_pUi->listView_TbAddressbooks->setModel(m_sListModel_TbAddressbooks);

  m_pUi->tableOwnNumbers->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &SettingsDialog::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);

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

  this->readSettings();
  this->initOnlineResolvers(sharePath);  // After readSettings!
  m_pFritzPb = new FritzPhonebook(this);
  m_pFritzPb->setHost(Settings().getHostName());
  m_pFritzPb->setPort(Settings().getTR064Port());
  m_pFritzPb->setUsername(Settings().getFritzUser());
  m_pFritzPb->setPassword(Settings().getFritzPassword());
  m_pFritzPb->setSavepath(
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" +
      qApp->applicationName().toLower() +
      QStringLiteral("/fritzbox_phonebooks"));
  this->initFritzPhonebooks();
}

SettingsDialog::~SettingsDialog() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::initFritzPhonebooks() {
  qDebug() << Q_FUNC_INFO;
  m_FritzPhoneBooks.clear();

  m_FritzPhoneBooks = m_pFritzPb->getPhonebookList();

  QHashIterator<QString, QHash<QString, QString>> i(m_FritzPhoneBooks);
  while (i.hasNext()) {
    i.next();

    int row = m_pUi->tableFritzPhonebooks->rowCount();
    m_pUi->tableFritzPhonebooks->insertRow(row);

    QTableWidgetItem *itemEnabled = new QTableWidgetItem();
    itemEnabled->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    if (m_sListEnabledFritzPhoneBooks.contains(
            i.value()[QStringLiteral("Name")])) {
      itemEnabled->setCheckState(Qt::Checked);
    } else {
      itemEnabled->setCheckState(Qt::Unchecked);
    }

    QTableWidgetItem *itemName =
        new QTableWidgetItem(i.value()[QStringLiteral("Name")]);
    m_pUi->tableFritzPhonebooks->setItem(row, 0, itemEnabled);
    m_pUi->tableFritzPhonebooks->setItem(row, 1, itemName);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QString SettingsDialog::downloadFritzPhonebook(const QString &sId,
                                               const QString &sUrl) {
  QString sFilePath;
  if (!Settings().getFritzUser().isEmpty() &&
      !Settings().getFritzPassword().isEmpty()) {
    qDebug() << "Trying to retrieve FritzBox phonebooks";

    if (m_pFritzPb->downloadPhonebook(sId.toInt(), sUrl)) {
      sFilePath = m_pFritzPb->getSavepath().absoluteFilePath("phonebook_" +
                                                             sId + ".xml");
      QFile fPhoneBook(sFilePath);

      if (!fPhoneBook.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open local FritzBox phone book" << sFilePath;
        return QString();
      }

      QDomDocument doc;
      if (!doc.setContent(&fPhoneBook)) {
        qWarning() << "Could not parse XML content from" << sFilePath;
        return QString();
      }

      QDomElement root = doc.documentElement();
      QDomNodeList xmlPhonebook =
          root.elementsByTagName(QStringLiteral("phonebook"));
      if (xmlPhonebook.isEmpty()) return QString();

      QDomElement pb = xmlPhonebook.at(0).toElement();
      QString sPhonebookName = pb.attribute(QStringLiteral("name")).trimmed();

      QRegularExpression re(QStringLiteral("phonebook_(\\d+)\\.xml"));
      QRegularExpressionMatch match = re.match(fPhoneBook.fileName());
      if (!match.hasMatch()) {
        qWarning() << "Filename doesn't match expected format:" << sFilePath;
        return QString();
      }

      bool ok = false;
      int id = match.captured(1).toInt(&ok);
      if (!ok) {
        qWarning() << "Invalid ID extracted from filename:" << sFilePath;
        return QString();
      }
    }
  } else {
    qWarning() << "FritzBox user/password not set!";
    return QString();
  }

  return sFilePath;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void SettingsDialog::readSettings() {
  qDebug() << Q_FUNC_INFO;
  Settings settings;

  // General
  m_pUi->lineEditCountryCode->setText(settings.getCountryCode());
  m_pUi->spinBoxTimeout->setValue(settings.getPopupTimeout());

  // Connection
  m_pUi->lineEditHost->setText(settings.getHostName());
  m_pUi->spinBoxCallMonitorPort->setValue(settings.getCallMonitorPort());
  m_pUi->spinBoxTR064Port->setValue(settings.getTR064Port());
  m_pUi->lineEditUserName->setText(settings.getFritzUser());
  m_pUi->lineEditPassword->setText(settings.getFritzPassword());

  // NumberResolvers
  m_sListModel_TbAddressbooks->setStringList(settings.getTbAddressbooks());

  m_sListEnabledFritzPhoneBooks = settings.getEnabledFritzPhonebooks();
  for (int row = 0; row < m_pUi->tableFritzPhonebooks->rowCount(); ++row) {
    if (m_sListEnabledFritzPhoneBooks.contains(
            m_pUi->tableFritzPhonebooks->item(row, 1)->text())) {
      m_pUi->tableFritzPhonebooks->item(row, 0)->setCheckState(Qt::Checked);
    } else {
      m_pUi->tableFritzPhonebooks->item(row, 0)->setCheckState(Qt::Unchecked);
    }
  }

  m_sListEnabledOnlineResolvers = settings.getEnabledOnlineResolvers();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (m_sListEnabledOnlineResolvers.contains(m_OnlineResolvers.value(
            m_pUi->tableOnlineResolvers->item(row, 1)->text()))) {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Checked);
    } else {
      m_pUi->tableOnlineResolvers->item(row, 0)->setCheckState(Qt::Unchecked);
    }
  }

  // PhoneNumbers
  uint nMaxOwnNumbers = settings.getMaxOwnNumbers();
  QMap<QString, QString> ownNumbers = settings.getOwnNumbers();

  // Fill table with own numbers
  int row = 0;
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

  // General
  settings.setCountryCode(m_pUi->lineEditCountryCode->text().trimmed());
  settings.setPopupTimeout(m_pUi->spinBoxTimeout->value());
  settings.setMaxDaysOfOldCalls(
      settings.getMaxDaysOfOldCalls());  // TODO: Add to dialog
  settings.setMaxEntriesCallHistory(
      settings.getMaxEntriesCallHistory());  // TODO: Add to dialog

  // Connection
  settings.setHostName(m_pUi->lineEditHost->text().trimmed());
  settings.setCallMonitorPort(m_pUi->spinBoxCallMonitorPort->value());
  settings.setTR064Port(m_pUi->spinBoxTR064Port->value());
  settings.setFritzUser(m_pUi->lineEditUserName->text().trimmed());
  settings.setFritzPassword(m_pUi->lineEditPassword->text().trimmed());
  settings.setRetryInterval(
      settings.getRetryInterval());  // TODO: Add to dialog

  // NumberResolvers
  settings.setTbAddressbooks(m_sListModel_TbAddressbooks->stringList());

  m_sListEnabledOnlineResolvers.clear();
  for (int row = 0; row < m_pUi->tableOnlineResolvers->rowCount(); ++row) {
    if (Qt::Checked ==
        m_pUi->tableOnlineResolvers->item(row, 0)->checkState()) {
      m_sListEnabledOnlineResolvers << m_OnlineResolvers.value(
          m_pUi->tableOnlineResolvers->item(row, 1)->text());
    }
  }
  settings.setEnabledOnlineResolvers(m_sListEnabledOnlineResolvers);

  m_sListEnabledFritzPhoneBooks.clear();
  for (int row = 0; row < m_pUi->tableFritzPhonebooks->rowCount(); ++row) {
    if (Qt::Checked ==
        m_pUi->tableFritzPhonebooks->item(row, 0)->checkState()) {
      m_sListEnabledFritzPhoneBooks
          << m_pUi->tableFritzPhonebooks->item(row, 1)->text();
    }
  }
  settings.setEnabledFritzPhonebooks(m_sListEnabledFritzPhoneBooks);

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
                                 settings.getRetryInterval());
  emit changedPhonebooks(settings.getTbAddressbooks(),
                         this->getFritzPhonebooks());

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

auto SettingsDialog::getFritzPhonebooks() -> const QHash<QString, QString> {
  QHash<QString, QString> fritzPhoneBooks;
  QHashIterator<QString, QHash<QString, QString>> i(m_FritzPhoneBooks);
  while (i.hasNext()) {
    i.next();

    if (m_sListEnabledFritzPhoneBooks.contains(
            i.value()[QStringLiteral("Name")])) {
      QString sFilePath = this->downloadFritzPhonebook(
          i.key(), i.value()[QStringLiteral("URL")]);
      if (!sFilePath.isEmpty()) {
        fritzPhoneBooks.insert(i.value()[QStringLiteral("Name")], sFilePath);
      }
    }
  }

  return fritzPhoneBooks;
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
