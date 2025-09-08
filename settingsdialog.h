/**
 * \file settingsdialog.h
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
 * Class definition of the settings dialog.
 */

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>
#include <QDir>
#include <QHash>
#include <QObject>

#include "fritzphonebook.h"

class QSettings;
class QStringListModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(const QDir sharePath, QObject *pParent = nullptr);
  virtual ~SettingsDialog();

  void translateUi();

  // TODO: Quick and dirty... this function should not be part of SettingsDialog
  auto getFritzPhonebooks() -> const QHash<QString, QString>;

 public slots:
  void accept() override;

 signals:
  void changedConnectionSettings(const QString &sHostName,
                                 const uint nMonitorPort,
                                 const uint nRetryInterval);
  void changedPhonebooks(const QStringList &sListTbAddressbooks,
                         const QHash<QString, QString> &sListFritzPhonebooks);

 protected:
  void showEvent(QShowEvent *pEvent) override;
  void closeEvent(QCloseEvent *event) override;

 private:
  void initOnlineResolvers(QDir sharePath);
  void initFritzPhonebooks();
  QString downloadFritzPhonebook(const QString &sId, const QString &sUrl);
  void readSettings();
  auto getThunderbirdProfilePath() -> const QString;

  Ui::SettingsDialog *m_pUi;
  QStringListModel *m_sListModel_TbAddressbooks;
  FritzPhonebook *m_pFritzPb;
  QHash<QString, QHash<QString, QString>> m_FritzPhoneBooks;
  QStringList m_sListEnabledFritzPhoneBooks;
  QHash<QString, QString> m_OnlineResolvers;
  QStringList m_sListEnabledOnlineResolvers;
};

#endif  // SETTINGSDIALOG_H_
