// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRITZCALLINDICATOR_H_
#define FRITZCALLINDICATOR_H_

#include <QSystemTrayIcon>
#include <QTranslator>
#ifdef FRITZ_USE_NOTIFICATION_SOUND
#include <QAudioOutput>
#include <QMediaPlayer>
#endif

#include "settings.h"
#include "settingsdialog.h"

#ifndef QT_NO_SYSTEMTRAYICON
#include <QDir>

#include "callmonitor.h"
#include "numberresolver.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
QT_END_NAMESPACE

class FritzCallIndicator : public QObject {
  Q_OBJECT

 public:
  FritzCallIndicator(const QDir &sharePath);
  ~FritzCallIndicator();

 private slots:
  void showCallHistory();

 private:
  void createActions();
  void createTrayIcon();
  void loadTranslation(const QString &sLang);
  static auto switchTranslator(QTranslator *translator, const QString &sFile,
                               const QString &sPath = QLatin1String(""))
      -> bool;
  void showInfoBox();

  void onErrorOccured(QTcpSocket::SocketError, const QString &sErrorMessage);
  void onStateChanged(QTcpSocket::SocketState state);
  void onIncomingCall(unsigned connectionId, const QString &sCaller,
                      const QString &sCallee);
  void showMessage(
      const QString &sTitle, const QString &sMessage, uint nTimeout = 0,
      const QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);

  QStringList getCallHistory();

  QAction *m_pShowCallHistory;
  QAction *m_pShowSettingsDialog;
  QAction *m_pShowInfoBox;
  QAction *m_pQuit;
  QSystemTrayIcon *m_pTrayIcon;
  QMenu *m_pTrayIconMenu;
  QTranslator m_translator;    // App translations
  QTranslator m_translatorQt;  // Qt translations
  const QString m_sSharePath;
  Settings *m_pSettings;

  SettingsDialog *m_pSettingsDialog = nullptr;
  CallMonitor *m_pCallMonitor = nullptr;
  NumberResolver *m_pNumberResolver = nullptr;

  QStringList m_sListCallHistory;
#ifdef FRITZ_USE_NOTIFICATION_SOUND
  QMediaPlayer *m_pNotificationSound;
  QAudioOutput *m_pNotificationAudioOutput;
#endif
};

#endif  // QT_NO_SYSTEMTRAYICON

#endif  // FRITZCALLINDICATOR_H_
