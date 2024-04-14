#include "settings.h"

#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QStyleHints>
#endif

#include "ui_settings.h"

const QString Settings::DEFAULT_HOST_NAME(QStringLiteral("fritz.box"));
const uint Settings::DEFAULT_CALL_MONITOR_PORT = 1012;
const uint Settings::DEFAULT_RETRY_INTERVAL_SEC = 20;
const uint Settings::DEFAULT_POPUP_TIMEOUT_SEC = 10;

Settings::Settings(QObject *pParent) : m_pUi(new Ui::SettingsDialog()) {
  Q_UNUSED(pParent)
  qDebug() << Q_FUNC_INFO;
  m_pUi->setupUi(this);

#if defined _WIN32
  m_pSettings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#else
  m_pSettings = new QSettings(QSettings::NativeFormat, QSettings::UserScope,
                              qApp->applicationName().toLower(),
                              qApp->applicationName().toLower());
#endif

  connect(m_pUi->buttonBox, &QDialogButtonBox::accepted, this,
          &Settings::accept);
  connect(m_pUi->buttonBox, &QDialogButtonBox::rejected, this,
          &QDialog::reject);

  this->readSettings();
}

Settings::~Settings() {
  delete m_pUi;
  m_pUi = nullptr;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::showEvent(QShowEvent *pEvent) {
  this->readSettings();
  QDialog::showEvent(pEvent);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::readSettings() {
  qDebug() << Q_FUNC_INFO;

  m_nPopupTimeout =
      m_pSettings
          ->value(QStringLiteral("PopupTimeout"), DEFAULT_POPUP_TIMEOUT_SEC)
          .toUInt();
  m_pUi->spinBoxTimeout->setValue(m_nPopupTimeout);
  m_bShowOutgoingCalls =
      m_pSettings->value(QStringLiteral("ShowOutgoingCalls"), true).toBool();
  m_pUi->checkBoxShowOutgoing->setChecked(m_bShowOutgoingCalls);

  m_pSettings->beginGroup(QStringLiteral("Connection"));
  m_sHostName =
      m_pSettings->value(QStringLiteral("HostName"), DEFAULT_HOST_NAME)
          .toString();
  m_pUi->lineEditHost->setText(m_sHostName);
  m_nPortNumber =
      m_pSettings->value(QStringLiteral("Port"), DEFAULT_CALL_MONITOR_PORT)
          .toUInt();
  m_pUi->spinBoxPort->setValue(m_nPortNumber);
  m_nRetryInterval =
      m_pSettings
          ->value(QStringLiteral("RetryInterval"), DEFAULT_RETRY_INTERVAL_SEC)
          .toUInt();
  m_pSettings->endGroup();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::accept() {
  qDebug() << Q_FUNC_INFO;
  m_nPopupTimeout = m_pUi->spinBoxTimeout->value();
  m_pSettings->setValue(QStringLiteral("PopupTimeout"), m_nPopupTimeout);
  m_bShowOutgoingCalls = m_pUi->checkBoxShowOutgoing->isChecked();
  m_pSettings->setValue(QStringLiteral("ShowOutgoingCalls"),
                        m_bShowOutgoingCalls);

  m_pSettings->beginGroup(QStringLiteral("Connection"));
  m_sHostName = m_pUi->lineEditHost->text();
  m_pSettings->setValue(QStringLiteral("HostName"), m_sHostName);
  m_nPortNumber = m_pUi->spinBoxPort->value();
  m_pSettings->setValue(QStringLiteral("Port"), m_nPortNumber);
  m_pSettings->setValue(QStringLiteral("RetryInterval"), m_nRetryInterval);
  m_pSettings->endGroup();

  emit changedSettings(m_sHostName, m_nPortNumber, m_nRetryInterval);

  QDialog::accept();
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getLanguage() -> QString {
#ifdef Q_OS_UNIX
  QByteArray lang = qgetenv("LANG");
  if (!lang.isEmpty()) {
    return QLocale(QString::fromLatin1(lang)).name();
  }
#endif
  return QLocale::system().name();
}

void Settings::translateUi() { m_pUi->retranslateUi(this); }

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

auto Settings::getIconTheme() -> QString {
  QString sIconTheme = QStringLiteral("light");
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  if (Qt::ColorScheme::Dark == QGuiApplication::styleHints()->colorScheme()) {
    sIconTheme = QStringLiteral("dark");
  }
#else
  if (this->window()->palette().window().color().lightnessF() < 0.5) {
    sIconTheme = QStringLiteral("dark");
  }
#endif
  return sIconTheme;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

void Settings::closeEvent(QCloseEvent *pEvent) {
  QMessageBox::information(this, QStringLiteral(APP_NAME),
                           tr("The program will keep running in the "
                              "system tray. To terminate the program, "
                              "choose <b>Quit</b> in the context menu "
                              "of the system tray entry."));
  hide();
  pEvent->ignore();
}
