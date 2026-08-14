#ifndef OTA_UPGRADE_H
#define OTA_UPGRADE_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class OtaClient;

/* OTA 升级页面:版本检查 / 下载 / 校验 / 应用(升级计划·功能组三) */
class ota_upgrade : public QWidget
{
    Q_OBJECT

public:
    explicit ota_upgrade(QWidget *parent = nullptr);

private slots:
    void onCheckClicked();
    void onUpgradeClicked();
    void onVersionChecked(bool hasNew, const QString &newVersion,
                          const QString &currentVersion);
    void onProgress(qint64 received, qint64 total);
    void onError(const QString &text);
    void onUpdateApplied();

private:
    void setupUi();
    void appendLog(const QString &text);
    void loadConfig();

    OtaClient *m_client = nullptr;
    QLabel *m_curVersionLabel = nullptr;
    QLabel *m_serverVersionLabel = nullptr;
    QLineEdit *m_urlEdit = nullptr;
    QPushButton *m_checkButton = nullptr;
    QPushButton *m_upgradeButton = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QPlainTextEdit *m_log = nullptr;
};

#endif // OTA_UPGRADE_H
