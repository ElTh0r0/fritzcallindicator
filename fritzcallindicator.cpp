/**
 * \file fritzcallindicator.cpp
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
 * Main application generation (tray icon, object creation etc.).
 */

#include "fritzcallindicator.h"

#include <QAction>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QIcon>
#include <QLabel>
#include <QLibraryInfo>
#include <QMenu>
#include <QMessageBox>

#include "fritzphonebook.h"

FritzCallIndicator::FritzCallIndicator(const QDir &sharePath)
    : m_sSharePath(sharePath.absolutePath()) {
  qDebug() << Q_FUNC_INFO;
  m_pSettings = new Settings(m_sSharePath);
  this->loadTranslation(m_pSettings->getLanguage());

  this->createActions();
  this->createTrayIcon();
  m_pTrayIcon->show();

  m_pNumberResolver =
      new NumberResolver(m_sSharePath, m_pSettings->getCountryCode(), this);
  connect(m_pSettings, &Settings::changedPhonebooks, m_pNumberResolver,
          &NumberResolver::readPhonebooks);
  m_pNumberResolver->readPhonebooks(m_pSettings->getTbAddressbooks(),
                                    m_pSettings->getFritzPhonebooks());

  m_pFritzBox = new FritzBox(this);
  connect(m_pFritzBox, &FritzBox::errorOccured, this,
          &FritzCallIndicator::onErrorOccured);
  connect(m_pFritzBox, &FritzBox::stateChanged, this,
          &FritzCallIndicator::onStateChanged);
  connect(m_pFritzBox, &FritzBox::incomingCall, this,
          &FritzCallIndicator::onIncomingCall);
  connect(m_pSettings, &Settings::changedConnectionSettings, m_pFritzBox,
          &FritzBox::connectTo);

  m_pFritzBox->connectTo(m_pSettings->getHostName(),
                         m_pSettings->getCallMonitorPort(),
                         m_pSettings->getRetryInterval());

  FritzPhonebook fb;
  fb.setHost(m_pSettings->getHostName());
  fb.setUsername(m_pSettings->getFritzUser());
  fb.setPassword(m_pSettings->getFritzPassword());
  fb.setPort(m_pSettings->getTR064Port());
  m_sListCallHistory = fb.getCallHistory(m_pSettings->getMaxDaysOfOldCalls(),
                                         m_pSettings->getMaxCallHistory());

  // 03.11.16 13:17:08;RING;0;03023125222;06990009111;SIP0;
  // m_pFritzBox->parseAndSignal(
  //    "03.11.16 13:17:08;RING;0;03023125222;06990009111;SIP0;");
  // Number suppressed: 03.11.16 13:17:08;RING;0;;06990009111;SIP0;
  // m_pFritzBox->parseAndSignal("03.11.16 13:17:08;RING;0;;06990009111;SIP0;");
}

FritzCallIndicator::~FritzCallIndicator() {
  m_pFritzBox->disconnectFrom();
  delete m_pSettings;
  m_pSettings = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::createActions() {
  qDebug() << Q_FUNC_INFO;
  QIcon::setThemeName(m_pSettings->getIconTheme());
  m_pShowCallHistory = new QAction(tr("Call history"), this);
  connect(m_pShowCallHistory, &QAction::triggered, this,
          &FritzCallIndicator::showCallHistory);

  m_pShowSettings = new QAction(QIcon::fromTheme(QStringLiteral("configure")),
                                tr("Settings"), this);
  connect(m_pShowSettings, &QAction::triggered, m_pSettings,
          &QDialog::showNormal);

  m_pShowInfoBox = new QAction(QIcon::fromTheme(QStringLiteral("help-about")),
                               tr("About"), this);
  connect(m_pShowInfoBox, &QAction::triggered, this,
          &FritzCallIndicator::showInfoBox);

  m_pQuit = new QAction(QIcon::fromTheme(QStringLiteral("application-exit")),
                        tr("Quit"), this);
  connect(m_pQuit, &QAction::triggered, this, &QCoreApplication::quit);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::createTrayIcon() {
  qDebug() << Q_FUNC_INFO;
  m_pTrayIconMenu = new QMenu();
  m_pTrayIconMenu->addAction(m_pShowCallHistory);
  m_pTrayIconMenu->addAction(m_pShowSettings);
  m_pTrayIconMenu->addAction(m_pShowInfoBox);
  m_pTrayIconMenu->addSeparator();
  m_pTrayIconMenu->addAction(m_pQuit);

  m_pTrayIcon = new QSystemTrayIcon(this);
  m_pTrayIcon->setContextMenu(m_pTrayIconMenu);

  QString sTray(QStringLiteral(":/icons/tray/call-start_22_dark.png"));
  if (QStringLiteral("light") == m_pSettings->getIconTheme()) {
    sTray = sTray.replace(QStringLiteral("dark"), QStringLiteral("light"));
  }
#if defined(Q_OS_WIN)
  sTray = sTray.replace(QStringLiteral("22"), QStringLiteral("16"));
#endif
  m_pTrayIcon->setIcon(QIcon(sTray));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::showMessage(const QString &sTitle,
                                     const QString &sMessage, uint nTimeout,
                                     const QSystemTrayIcon::MessageIcon icon) {
  if (0 == nTimeout) {
    nTimeout = m_pSettings->getPopupTimeout();
  }
  m_pTrayIcon->showMessage(sTitle, sMessage, icon, nTimeout * 1000);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::onErrorOccured(QTcpSocket::SocketError,
                                        const QString &errorMessage) {
  this->showMessage(QStringLiteral(APP_NAME),
                    tr("Connecting to '%1:%2' failed, because: '%3'")
                        .arg(m_pSettings->getHostName())
                        .arg(m_pSettings->getCallMonitorPort())
                        .arg(errorMessage),
                    m_pSettings->getRetryInterval() / 2,
                    QSystemTrayIcon::Warning);
  /*
  QMessageBox::critical(nullptr, QString::fromLatin1(APP_NAME),
                        tr("Connecting to '%1:%2' failed, because: '%3'")
                            .arg(m_pSettings->getHostName())
                            .arg(m_pSettings->getCallMonitorPort())
                            .arg(errorMessage));
  */
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::onStateChanged(QTcpSocket::SocketState state) {
  if (state == QTcpSocket::SocketState::ConnectedState) {
    qDebug() << "Connected to" << m_pSettings->getHostName()
             << "- Port:" << m_pSettings->getCallMonitorPort();
    /*
    this->showMessage(QStringLiteral(APP_NAME),
                      tr("Connected to '%1:%2'")
                          .arg(m_pSettings->getHostName())
                          .arg(m_pSettings->getPortNumber()),
                      3);
    */
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::onIncomingCall(unsigned /* connectionId */,
                                        const QString &sCaller,
                                        const QString &sCallee) {
  QString sResolvedCaller = m_pNumberResolver->resolveNumber(
      sCaller.trimmed(), m_pSettings->getEnabledOnlineResolvers());
  QString sResolvedCallee = m_pSettings->resolveOwnNumber(sCallee.trimmed());
  QString sTitle(tr("Incoming call"));
  if (!sResolvedCallee.isEmpty()) {
    sTitle = tr("Incoming call to '%1'").arg(sResolvedCallee);
  }

  // TODO: Configurable date format
  m_sListCallHistory.push_front(
      QDateTime::currentDateTime().toString("dd.MM.yy|hh:mm|") +
      sResolvedCaller);
  // Limit the number of recent calls
  if (m_sListCallHistory.count() > m_pSettings->getMaxCallHistory()) {
    m_sListCallHistory =
        m_sListCallHistory.mid(0, m_pSettings->getMaxCallHistory());
  }

  this->showMessage(sTitle, tr("Caller: '%1'").arg(sResolvedCaller));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::showCallHistory() {
  Qt::AlignmentFlag Align;
  QDialog dialog;
  dialog.setWindowTitle(tr("Call history"));
  dialog.setWindowFlags(dialog.window()->windowFlags() &
                        ~Qt::WindowContextHelpButtonHint);

  auto *layout = new QGridLayout(&dialog);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(10);

  layout->addWidget(new QLabel("<b>" + tr("Date") + "</b>", &dialog), 0, 0,
                    Qt::AlignCenter | Qt::AlignVCenter);
  layout->addWidget(new QLabel("<b>" + tr("Time") + "</b>", &dialog), 0, 1,
                    Qt::AlignCenter | Qt::AlignVCenter);
  layout->addWidget(new QLabel("<b>" + tr("Caller") + "</b>", &dialog), 0, 2,
                    Qt::AlignCenter | Qt::AlignVCenter);

  for (int nRow = 0; nRow < m_sListCallHistory.count(); nRow++) {
    QStringList sCall(m_sListCallHistory.at(nRow).split('|'));
    if (sCall.count() > 2) {
      for (int nCol = 0; nCol < 3; nCol++) {
        if (2 == nCol) {
          Align = Qt::AlignLeft;
        } else {
          Align = Qt::AlignCenter;
        }
        layout->addWidget(new QLabel(sCall.at(nCol), &dialog), nRow + 1, nCol,
                          Align | Qt::AlignVCenter);
      }
    }
  }

  auto *button =
      new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, &dialog);
  connect(button, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  layout->addWidget(button, m_sListCallHistory.count() + 1, 0, 1, 3,
                    Qt::AlignCenter);

  dialog.exec();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::loadTranslation(const QString &sLang) {
  if (!FritzCallIndicator::switchTranslator(
          &m_translatorQt, "qt_" + sLang,
          QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
    FritzCallIndicator::switchTranslator(&m_translatorQt, "qt_" + sLang,
                                         qApp->applicationDirPath() + "/lang");
  }

  FritzCallIndicator::switchTranslator(
      &m_translator,
      ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm");

  if (!FritzCallIndicator::switchTranslator(
          &m_translator,
          ":/" + qApp->applicationName().toLower() + "_" + sLang + ".qm")) {
    FritzCallIndicator::switchTranslator(
        &m_translator, qApp->applicationName().toLower() + "_" + sLang,
        qApp->applicationDirPath() + "/lang");
  }
  m_pSettings->translateUi();
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

auto FritzCallIndicator::switchTranslator(QTranslator *translator,
                                          const QString &sFile,
                                          const QString &sPath) -> bool {
  qApp->removeTranslator(translator);
  if (translator->load(sFile, sPath)) {
    qApp->installTranslator(translator);
  } else {
    qWarning() << "Could not find translation" << sFile << "in" << sPath;
    return false;
  }
  return true;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::showInfoBox() {
  QMessageBox::about(
      nullptr, tr("About"),
      QString::fromLatin1("<center>"
                          "<big><b>%1 %2</b></big><br />"
                          "%3<br />"
                          "<small>%4</small><br /><br />"
                          "%5<br />"
                          "%6<br />"
                          "<small>%7</small>"
                          "</center><br />"
                          "%8")
          .arg(APP_NAME, APP_VERSION, APP_DESC, APP_COPY,
               "URL: <a href=\"https://github.com/ElTh0r0/fritzcallindicator\">"
               "https://github.com/ElTh0r0/fritzcallindicator</a>",
               tr("License") +
                   ": <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">"
                   "GNU General Public License Version 3</a>",
               tr("This application uses "
                  "<a href=\"https://invent.kde.org/frameworks/breeze-icons\">"
                  "Breeze icons from KDE</a>."),
               "<i>" + tr("Translations") +
                   "</i><br />"
                   "&nbsp;&nbsp;- German: ElThoro"));
}
