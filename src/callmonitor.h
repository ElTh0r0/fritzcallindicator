// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-FileCopyrightText: 2020 Peter Most
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CALLMONITOR_H_
#define CALLMONITOR_H_

#include <QObject>
#include <QTcpSocket>

class CallMonitor : public QObject {
  Q_OBJECT
 public:
  CallMonitor(QObject *pParent = nullptr) noexcept;

  void connectTo(const QString &sHostName, uint nPortNumber,
                 uint nRetryInterval) noexcept;
  void disconnectFrom() noexcept;

  void parseAndSignal(const QString &sLine);

 signals:
  void stateChanged(QAbstractSocket::SocketState state);
  void errorOccured(QAbstractSocket::SocketError error,
                    const QString &sMessage);

  void incomingCall(unsigned connectionId, const QString &sCaller,
                    const QString &sCallee);

 private slots:
  void onConnected();
  void onError(QAbstractSocket::SocketError socketError);
  void onReadyRead();

 private:
  QString m_sHostName;
  uint m_nPortNumber;
  uint m_nRetryInterval;
  QTcpSocket *m_pSocket;

  void reconnect();
};

#endif  // CALLMONITOR_H_
