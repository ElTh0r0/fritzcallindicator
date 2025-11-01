// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

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

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(
      const QHash<QString, QString> &availableOnlineResolvers,
      QObject *pParent = nullptr);
  virtual ~SettingsDialog();

  void translateUi();

 public slots:
  void accept() override;

 signals:
  void changedConnectionSettings(const QString &sHostName,
                                 const uint nMonitorPort,
                                 const uint nRetryInterval);
  void changedPhonebooks();

 protected:
#ifdef FRITZ_USE_NOTIFICATION_SOUND
  bool eventFilter(QObject *pObj, QEvent *pEvent) override;
#endif
  void showEvent(QShowEvent *pEvent) override;
  void closeEvent(QCloseEvent *event) override;

 private:
#ifdef FRITZ_USE_ONLINE_RESOLVERS
  void initOnlineResolvers(
      const QHash<QString, QString> &availableOnlineResolvers);
#endif
  void initFritzPhonebooks();
  void readSettings();
  auto getThunderbirdProfilePath() -> const QString;

  Ui::SettingsDialog *m_pUi;
  QStringList m_sListEnabledFritzPhoneBooks;
#ifdef FRITZ_USE_THUNDERBIRD_ADDRESSBOOK
  QStringListModel *m_sListModel_TbAddressbooks;
#endif
#ifdef FRITZ_USE_ONLINE_RESOLVERS
  QHash<QString, QString> m_OnlineResolvers;
  QStringList m_sListEnabledOnlineResolvers;
#endif
#ifdef FRITZ_USE_NOTIFICATION_SOUND
  QStringList m_sListSupportedAudioFormats;
#endif
};

#endif  // SETTINGSDIALOG_H_
