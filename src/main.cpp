// SPDX-FileCopyrightText: 2024-2025 Thorsten Roth
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON
#include <QDir>
#include <QMessageBox>

#include "fritzcallindicator.h"

int main(int argc, char *argv[]) {
#if defined(Q_OS_WIN) && QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  QApplication::setStyle("Fusion");  // Supports dark scheme on Win 10/11
#endif

  QApplication app(argc, argv);
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMessageBox::critical(
        nullptr, qApp->applicationName(),
        QObject::tr("Could not detect any system tray on this system."));
    return 1;
  }

  app.setApplicationName(QStringLiteral(APP_NAME));
  app.setApplicationVersion(QStringLiteral(APP_VERSION));
  app.setApplicationDisplayName(QStringLiteral(APP_NAME));
#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
  app.setWindowIcon(
      QIcon::fromTheme(QStringLiteral("fritzcallindicator"),
                       QIcon(QStringLiteral(":/fritzcallindicator.png"))));
  app.setDesktopFileName(
      QStringLiteral("com.github.elth0r0.fritzcallindicator"));
#endif
  QApplication::setQuitOnLastWindowClosed(false);

  // Default share data path (Windows and debugging)
  QString sSharePath = app.applicationDirPath() + QStringLiteral("/data");
  // Standard installation path (Linux)
  QDir tmpDir(app.applicationDirPath() + "/../share/" +
              app.applicationName().toLower());
  if (!QDir(sSharePath).exists() && tmpDir.exists()) {
    sSharePath = app.applicationDirPath() + "/../share/" +
                 app.applicationName().toLower();
  }
#if defined(Q_OS_MACOS)
  sSharePath = app.applicationDirPath() + "/../Resources/";
#endif

  FritzCallIndicator w(sSharePath);
  return app.exec();
}
#else
#include <QDebug>
#include <QLabel>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QString sText(
      QStringLiteral("System tray icon is not supported on this platform!"));

  QLabel *errorLabel = new QLabel(sText);
  errorLabel->show();
  qDebug() << sText;

  app.exec();
}
#endif
