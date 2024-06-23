/**
 * \file fritzbox.cpp
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
 * FritzBox! connection
 *
 * \section SOURCE
 * This file incorporates work covered by the following copyright:
 * Copyright (c) 2020, Peter Most
 * Released under the GNU GENERAL PUBLIC LICENSE version 3
 * Original code form: https://github.com/petermost/FritzBoxCallMonitor
 */

#include "fritzbox.h"

#include <QTimer>

FritzBox::FritzBox(QObject *pParent) noexcept : QObject(pParent) {
  qDebug() << Q_FUNC_INFO;
  m_pSocket = new QTcpSocket(this);
  connect(m_pSocket, &QTcpSocket::disconnected, this, &FritzBox::onConnected);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  connect(m_pSocket, &QTcpSocket::errorOccurred, this, &FritzBox::onError);
#else
  connect(m_pSocket,
          qOverload<QAbstractSocket::SocketError>(&QTcpSocket::error), this,
          &FritzBox::onError);
#endif
  connect(m_pSocket, &QTcpSocket::stateChanged, this, &FritzBox::stateChanged);
  connect(m_pSocket, &QTcpSocket::readyRead, this, &FritzBox::onReadyRead);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzBox::connectTo(const QString &sHostName, uint nPortNumber,
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

void FritzBox::disconnectFrom() noexcept {
  qDebug() << Q_FUNC_INFO;
  if (m_pSocket->state() != QTcpSocket::SocketState::ConnectedState)
    m_pSocket->abort();
  m_pSocket->disconnectFromHost();
  m_sHostName.clear();
  m_nPortNumber = 0;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzBox::onConnected() {}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void FritzBox::reconnect() {
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

void FritzBox::onError(QTcpSocket::SocketError socketError) {
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

void FritzBox::parseAndSignal(const QString &sLine) {
  QStringList parts = sLine.split(';');
  QString dateTime = parts[0];
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

void FritzBox::onReadyRead() {
  qint64 length;
  char buffer[100];

  if ((length = m_pSocket->readLine(buffer, sizeof(buffer))) != -1) {
    QString line =
        QString::fromLatin1(buffer, static_cast<int>(length)).remove('\n');
    parseAndSignal(line);
  }
}
