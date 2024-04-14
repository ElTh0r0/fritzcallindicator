/**
 * \file main.cpp
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
 * Main function, start application.
 */

/** \mainpage
 * \section Introduction
 * FritzCallIndicator is Simple FritzBox! call indicator.<br />
 * GitHub: https://github.com/ElTh0r0/fritzcallindicator
 */

#include <QApplication>

#ifndef QT_NO_SYSTEMTRAYICON
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
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
  app.setWindowIcon(
      QIcon::fromTheme(QStringLiteral("fritzcallindicator"),
                       QIcon(QStringLiteral(":/fritzcallindicator.png"))));
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  app.setDesktopFileName(
      QStringLiteral("com.github.elth0r0.fritzcallindicator"));
#endif
#endif
  QApplication::setQuitOnLastWindowClosed(false);

  FritzCallIndicator w;
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
