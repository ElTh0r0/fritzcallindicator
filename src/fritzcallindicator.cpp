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
#include <QNetworkReply>
#include <QXmlStreamReader>

#include "fritzsoap.h"

FritzCallIndicator::FritzCallIndicator(const QDir &sharePath)
    : m_sSharePath(sharePath.absolutePath()) {
  qDebug() << Q_FUNC_INFO;
  m_pSettingsDialog = new SettingsDialog(m_sSharePath);
  this->loadTranslation(m_settings.getLanguage());

  this->createActions();
  this->createTrayIcon();
  m_pTrayIcon->show();

  m_pNumberResolver =
      new NumberResolver(m_sSharePath, m_settings.getCountryCode(), this);
  connect(m_pSettingsDialog, &SettingsDialog::changedPhonebooks,
          m_pNumberResolver, &NumberResolver::readPhonebooks);
  m_pNumberResolver->readPhonebooks();

  m_pCallMonitor = new CallMonitor(this);
  connect(m_pCallMonitor, &CallMonitor::errorOccured, this,
          &FritzCallIndicator::onErrorOccured);
  connect(m_pCallMonitor, &CallMonitor::stateChanged, this,
          &FritzCallIndicator::onStateChanged);
  connect(m_pCallMonitor, &CallMonitor::incomingCall, this,
          &FritzCallIndicator::onIncomingCall);
  connect(m_pSettingsDialog, &SettingsDialog::changedConnectionSettings,
          m_pCallMonitor, &CallMonitor::connectTo);

  m_pCallMonitor->connectTo(m_settings.getHostName(),
                            m_settings.getCallMonitorPort(),
                            m_settings.getRetryInterval());

  m_sListCallHistory = this->getCallHistory();

  // 03.11.16 13:17:08;RING;0;03023125222;06990009111;SIP0;
  // m_pCallMonitor->parseAndSignal(
  //    "03.11.16 13:17:08;RING;0;03023125222;06990009111;SIP0;");
  // Number suppressed: 03.11.16 13:17:08;RING;0;;06990009111;SIP0;
  // m_pCallMonitor->parseAndSignal("03.11.16
  // 13:17:08;RING;0;;06990009111;SIP0;");
}

FritzCallIndicator::~FritzCallIndicator() {
  m_pCallMonitor->disconnectFrom();
  delete m_pSettingsDialog;
  m_pSettingsDialog = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::createActions() {
  qDebug() << Q_FUNC_INFO;
  QIcon::setThemeName(m_settings.getIconTheme());
  m_pShowCallHistory =
      new QAction(QIcon::fromTheme(QStringLiteral("view-history")),
                  tr("Call history"), this);
  connect(m_pShowCallHistory, &QAction::triggered, this,
          &FritzCallIndicator::showCallHistory);

  m_pShowSettingsDialog = new QAction(
      QIcon::fromTheme(QStringLiteral("configure")), tr("Settings"), this);
  connect(m_pShowSettingsDialog, &QAction::triggered, m_pSettingsDialog,
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
  m_pTrayIconMenu->addAction(m_pShowSettingsDialog);
  m_pTrayIconMenu->addAction(m_pShowInfoBox);
  m_pTrayIconMenu->addSeparator();
  m_pTrayIconMenu->addAction(m_pQuit);

  m_pTrayIcon = new QSystemTrayIcon(this);
  m_pTrayIcon->setContextMenu(m_pTrayIconMenu);

  QString sTray(QStringLiteral(":/icons/tray/call-start_22_dark.png"));
  if (QStringLiteral("light") == m_settings.getIconTheme()) {
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
    nTimeout = m_settings.getPopupTimeout();
  }
  m_pTrayIcon->showMessage(sTitle, sMessage, icon, nTimeout * 1000);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::onErrorOccured(QTcpSocket::SocketError,
                                        const QString &errorMessage) {
  this->showMessage(QStringLiteral(APP_NAME),
                    tr("Connecting to '%1:%2' failed, because: '%3'")
                        .arg(m_settings.getHostName())
                        .arg(m_settings.getCallMonitorPort())
                        .arg(errorMessage),
                    m_settings.getPopupTimeout(), QSystemTrayIcon::Warning);
  /*
  QMessageBox::critical(nullptr, QString::fromLatin1(APP_NAME),
                        tr("Connecting to '%1:%2' failed, because: '%3'")
                            .arg(m_settings.getHostName())
                            .arg(m_settings.getCallMonitorPort())
                            .arg(errorMessage));
  */
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzCallIndicator::onStateChanged(QTcpSocket::SocketState state) {
  if (state == QTcpSocket::SocketState::ConnectedState) {
    qDebug() << "Connected to" << m_settings.getHostName()
             << "- Port:" << m_settings.getCallMonitorPort();
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
      sCaller.trimmed(), m_settings.getEnabledOnlineResolvers());
  QString sResolvedCallee = m_settings.resolveOwnNumber(sCallee.trimmed());
  QString sTitle(tr("Incoming call"));
  if (!sResolvedCallee.isEmpty()) {
    sTitle = tr("Incoming call to '%1'").arg(sResolvedCallee);
  }

  // TODO: Configurable date format
  m_sListCallHistory.push_front(
      QDateTime::currentDateTime().toString("dd.MM.yy|hh:mm|") +
      sResolvedCaller);
  // Limit the number of recent calls
  if (m_sListCallHistory.count() > m_settings.getMaxEntriesCallHistory()) {
    m_sListCallHistory =
        m_sListCallHistory.mid(0, m_settings.getMaxEntriesCallHistory());
  }

  this->showMessage(sTitle, tr("Caller: '%1'").arg(sResolvedCaller));
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

QStringList FritzCallIndicator::getCallHistory() {
  qDebug() << Q_FUNC_INFO;
  QStringList sListCalls;

  const QString body = QStringLiteral(
      "<u:GetCallList xmlns:u=\"urn:dslforum-org:service:X_AVM-DE_OnTel:1\"/>");

  const QString response = FritzSOAP::instance()->sendRequest(
      QStringLiteral("urn:dslforum-org:service:X_AVM-DE_OnTel:1"),
      QStringLiteral("GetCallList"), body,
      QStringLiteral("/upnp/control/x_contact"));

  QString sCallListUrl;
  QXmlStreamReader xml(response);
  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement() &&
        xml.name() == QStringLiteral("NewCallListURL")) {
      sCallListUrl = xml.readElementText();
    }
  }

  if (xml.hasError() || sCallListUrl.isEmpty()) {
    qWarning() << "XML Parse Error:" << xml.errorString();
    return sListCalls;
  }

  sCallListUrl += "&days=" + QString::number(m_settings.getMaxDaysOfOldCalls());

  qDebug() << "Downloading call list from" << sCallListUrl;

  QNetworkAccessManager nam;
  QNetworkRequest request(sCallListUrl);
  QNetworkReply *reply = nam.get(request);

  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    qWarning() << "Download failed:" << reply->errorString();
    reply->deleteLater();
    return sListCalls;
  }

  QByteArray data = reply->readAll();
  reply->deleteLater();
  // qDebug() << data;
  xml.clear();
  xml.addData(data);

  bool inCall = false;
  bool typeIsOne = false;
  QString sName;
  QString sNumber;
  QString sDate;
  QString sTime;
  while (!xml.atEnd() && !xml.hasError()) {
    xml.readNext();

    if (xml.isStartElement()) {
      if (xml.name() == QStringLiteral("Call")) {
        inCall = true;
        typeIsOne = false;
        sName.clear();
        sNumber.clear();
        sDate.clear();
        sTime.clear();
      } else if (inCall) {
        if (xml.name() == QStringLiteral("Type")) {
          QString typeText = xml.readElementText();
          typeIsOne = (typeText == QStringLiteral("1"));  // 1 = Incoming call
        } else if (xml.name() == QStringLiteral("Caller")) {
          if (typeIsOne)
            sNumber = xml.readElementText();
          else
            xml.skipCurrentElement();
        } else if (xml.name() == QStringLiteral("Name")) {
          if (typeIsOne)
            sName = xml.readElementText();
          else
            xml.skipCurrentElement();
        } else if (xml.name() == QStringLiteral("Date")) {
          if (typeIsOne) {
            sDate = xml.readElementText();
            // TODO: Configurable date format
            // Date format from XML: dd.MM.yy HH:mm
            QStringList sListDateTime = sDate.split(' ');
            if (sListDateTime.size() == 2) {
              sDate = sListDateTime.at(0);
              sTime = sListDateTime.at(1);
            }
          } else {
            xml.skipCurrentElement();
          }
        } else {
          // Skip all other elements
          xml.skipCurrentElement();
        }
      }
    } else if (xml.isEndElement() && xml.name() == QStringLiteral("Call")) {
      if (typeIsOne) {
        if (sName.isEmpty()) {
#ifdef QT_DEBUG
          // Don't use online resolvers during debugging to prevent rate limits
          sName = m_pNumberResolver->resolveNumber(sNumber, QStringList());
#else
          sName = m_pNumberResolver->resolveNumber(
              sNumber, m_settings.getEnabledOnlineResolvers());
#endif
        }
        sListCalls.push_back(sDate + "|" + sTime + "|" + sName);
        if (sListCalls.size() >= m_settings.getMaxEntriesCallHistory()) break;
      }
      inCall = false;
      typeIsOne = false;
    }
  }

  if (xml.hasError()) {
    qWarning() << "XML Parse Error:" << xml.errorString();
    return sListCalls;
  }

  return sListCalls;
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
  m_pSettingsDialog->translateUi();
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
