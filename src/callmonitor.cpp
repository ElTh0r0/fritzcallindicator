// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-FileCopyrightText: 2020 Peter Most
// SPDX-License-Identifier: GPL-3.0-or-later

#include "callmonitor.h"

#include <QTimer>

CallMonitor::CallMonitor(QObject *pParent) noexcept : QObject(pParent) {
  qDebug() << Q_FUNC_INFO;
  m_pSocket = new QTcpSocket(this);
  connect(m_pSocket, &QTcpSocket::disconnected, this,
          &CallMonitor::onConnected);
  connect(m_pSocket, &QTcpSocket::errorOccurred, this, &CallMonitor::onError);
  connect(m_pSocket, &QTcpSocket::stateChanged, this,
          &CallMonitor::stateChanged);
  connect(m_pSocket, &QTcpSocket::readyRead, this, &CallMonitor::onReadyRead);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::connectTo(const QString &sHostName, uint nPortNumber,
                            uint nRetryInterval) noexcept {
  qDebug() << Q_FUNC_INFO;
  m_nRetryInterval = nRetryInterval;
  if (sHostName != m_sHostName || nPortNumber != m_nPortNumber) {
    disconnectFrom();

    m_sHostName = sHostName;
    m_nPortNumber = nPortNumber;

    m_pSocket->connectToHost(m_sHostName, m_nPortNumber);
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::disconnectFrom() noexcept {
  qDebug() << Q_FUNC_INFO;
  if (m_pSocket->state() != QTcpSocket::SocketState::ConnectedState)
    m_pSocket->abort();
  m_pSocket->disconnectFromHost();
  m_sHostName.clear();
  m_nPortNumber = 0;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::onConnected() {}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::reconnect() {
  qDebug() << Q_FUNC_INFO;
  m_pSocket->connectToHost(m_sHostName, m_nPortNumber);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

inline bool isRetryableError(QTcpSocket::SocketError socketError) {
  return socketError == QTcpSocket::SocketError::ConnectionRefusedError ||
         socketError == QTcpSocket::SocketError::RemoteHostClosedError ||
         socketError == QTcpSocket::SocketError::HostNotFoundError;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::onError(QAbstractSocket::SocketError socketError) {
  emit errorOccured(socketError,
                    m_pSocket->errorString() +
                        tr(" (Retry in %1 seconds ...)").arg(m_nRetryInterval));

  if (isRetryableError(socketError)) {
    QTimer::singleShot(m_nRetryInterval * 1000, this, [this] {
      if (m_pSocket->state() == QTcpSocket::SocketState::UnconnectedState)
        reconnect();
    });
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::parseAndSignal(const QString &sLine) {
  QStringList parts = sLine.split(';');
  // QString dateTime = parts[0];
  QString command = parts[1];
  unsigned connectionId = parts[2].toUInt();

  if (command == QStringLiteral("RING")) {
    QString caller = parts[3];
    QString callee = parts[4];
    qDebug() << "Received string:" << sLine;
    emit incomingCall(connectionId, caller, callee);
  } else if (command == QStringLiteral("CONNECT") ||
             command == QStringLiteral("DISCONNECT") ||
             command == QStringLiteral("CALL")) {
    // Features not used by FritzCallIndicator
  } else {
    emit errorOccured(QTcpSocket::SocketError::UnknownSocketError,
                      tr("Unknown command '%1'!").arg(sLine));
  }
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void CallMonitor::onReadyRead() {
  qint64 length;
  char buffer[100];

  if ((length = m_pSocket->readLine(buffer, sizeof(buffer))) != -1) {
    QString line =
        QString::fromLatin1(buffer, static_cast<int>(length)).remove('\n');
    parseAndSignal(line);
  }
}
