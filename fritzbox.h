/**
 * \file fritzbox.h
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

#ifndef FRITZBOX_H_
#define FRITZBOX_H_

#include <QObject>
#include <QTcpSocket>

class FritzBox : public QObject {
  Q_OBJECT
 public:
  FritzBox(QObject *pParent = nullptr) noexcept;

  void connectTo(const QString &sHostName, uint nPortNumber,
                 uint nRetryInterval) noexcept;
  void disconnectFrom() noexcept;

  void parseAndSignal(const QString &sLine);

 signals:
  void stateChanged(QTcpSocket::SocketState state);
  void errorOccured(QTcpSocket::SocketError error, const QString &sMessage);

  void incomingCall(unsigned connectionId, const QString &sCaller,
                    const QString &sCallee);

 private slots:
  void onConnected();
  void onError(QTcpSocket::SocketError socketError);
  void onReadyRead();

 private:
  QString m_sHostName;
  uint m_nPortNumber;
  uint m_nRetryInterval;
  QTcpSocket *m_pSocket;

  void reconnect();
};

#endif  // FRITZBOX_H_
