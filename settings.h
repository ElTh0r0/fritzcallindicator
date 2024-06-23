#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QHash>
#include <QObject>

class QSettings;

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class Settings : public QDialog {
  Q_OBJECT

 public:
  explicit Settings(QObject *pParent = nullptr);
  virtual ~Settings();

  QString getHostName() const noexcept { return m_sHostName; }
  uint getPortNumber() const noexcept { return m_nPortNumber; }
  uint getRetryInterval() const noexcept { return m_nRetryInterval; }
  uint getPopupTimeout() const noexcept { return m_nPopupTimeout; }
  QString getCountryCode() const noexcept { return m_sCountryCode; }
  QString resolveOwnNumber(const QString &sNumber) const noexcept {
    return m_OwnNumbers.value(sNumber, "");
  }
  auto getLanguage() -> QString;
  auto getIconTheme() -> QString;
  void translateUi();

 public slots:
  void accept() override;

 signals:
  void changedSettings(const QString &sHostName, const uint nPort,
                       const uint RetryInterval);

 protected:
  void showEvent(QShowEvent *pEvent) override;
  void closeEvent(QCloseEvent *event) override;

 private:
  void readSettings();

  Ui::SettingsDialog *m_pUi;
  QSettings *m_pSettings;
  QString m_sHostName;
  uint m_nPortNumber;
  uint m_nRetryInterval;
  uint m_nPopupTimeout;
  QString m_sCountryCode;
  uint m_nMaxOwnNumbers;
  QHash<QString, QString> m_OwnNumbers;
  static const QString DEFAULT_HOST_NAME;
  static const uint DEFAULT_CALL_MONITOR_PORT;
  static const uint DEFAULT_RETRY_INTERVAL_SEC;
  static const uint DEFAULT_POPUP_TIMEOUT_SEC;
  static const QString DEFAULT_COUNTRY_CODE;
  static const uint DEFAULT_MAX_OWN_NUMBERS;
};

#endif  // SETTINGS_H
