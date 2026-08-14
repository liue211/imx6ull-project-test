#include "ota_upgrade.h"

#include "ota/otaclient.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QTimer>
#include <QVBoxLayout>

namespace {

const QString kDefaultBaseUrl = QStringLiteral("http://192.168.137.1:8000");

} // namespace

ota_upgrade::ota_upgrade(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadConfig();

    m_client = new OtaClient(this);
    m_client->setBaseUrl(m_urlEdit->text());

    connect(m_client, &OtaClient::versionChecked,
            this, &ota_upgrade::onVersionChecked);
    connect(m_client, &OtaClient::downloadProgress,
            this, &ota_upgrade::onProgress);
    connect(m_client, &OtaClient::errorOccurred,
            this, &ota_upgrade::onError);
    connect(m_client, &OtaClient::logMessage,
            this, &ota_upgrade::appendLog);
    connect(m_client, &OtaClient::updateApplied,
            this, &ota_upgrade::onUpdateApplied);
    connect(m_checkButton, &QPushButton::clicked,
            this, &ota_upgrade::onCheckClicked);
    connect(m_upgradeButton, &QPushButton::clicked,
            this, &ota_upgrade::onUpgradeClicked);
    connect(m_urlEdit, &QLineEdit::textChanged, m_client,
            &OtaClient::setBaseUrl);

    /* 页面创建时自动检查一次版本 */
    QTimer::singleShot(0, m_client, &OtaClient::checkForUpdate);
}

void ota_upgrade::setupUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);

    auto *infoBox = new QGroupBox(QStringLiteral("版本信息"), this);
    auto *infoLayout = new QGridLayout(infoBox);
    m_curVersionLabel = new QLabel(infoBox);
    m_curVersionLabel->setObjectName(QStringLiteral("curVersionLabel"));
    m_serverVersionLabel = new QLabel(QStringLiteral("未知"), infoBox);
    m_serverVersionLabel->setObjectName(QStringLiteral("serverVersionLabel"));
    infoLayout->addWidget(new QLabel(QStringLiteral("当前版本:"), infoBox), 0, 0);
    infoLayout->addWidget(m_curVersionLabel, 0, 1);
    infoLayout->addWidget(new QLabel(QStringLiteral("服务器版本:"), infoBox), 1, 0);
    infoLayout->addWidget(m_serverVersionLabel, 1, 1);
    root->addWidget(infoBox);

    auto *serverBox = new QGroupBox(QStringLiteral("升级服务器"), this);
    auto *serverLayout = new QHBoxLayout(serverBox);
    m_urlEdit = new QLineEdit(kDefaultBaseUrl, serverBox);
    m_urlEdit->setObjectName(QStringLiteral("urlEdit"));
    m_checkButton = new QPushButton(QStringLiteral("检查更新"), serverBox);
    m_checkButton->setObjectName(QStringLiteral("checkButton"));
    m_upgradeButton = new QPushButton(QStringLiteral("立即升级"), serverBox);
    m_upgradeButton->setObjectName(QStringLiteral("upgradeButton"));
    m_upgradeButton->setEnabled(false);
    serverLayout->addWidget(new QLabel(QStringLiteral("URL:"), serverBox));
    serverLayout->addWidget(m_urlEdit, 1);
    serverLayout->addWidget(m_checkButton);
    serverLayout->addWidget(m_upgradeButton);
    root->addWidget(serverBox);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName(QStringLiteral("progressBar"));
    m_progressBar->setValue(0);
    root->addWidget(m_progressBar);

    m_log = new QPlainTextEdit(this);
    m_log->setObjectName(QStringLiteral("logEdit"));
    m_log->setReadOnly(true);
    m_log->setPlaceholderText(QStringLiteral("检查/下载/校验/升级日志"));
    root->addWidget(m_log, 1);
}

void ota_upgrade::loadConfig()
{
    /* 与 main.cpp 共用 exe 同目录的 ota.ini,可持久化服务器地址 */
    QSettings settings(QStringLiteral("ota.ini"), QSettings::IniFormat);
    const QString url =
        settings.value(QStringLiteral("ota/baseUrl"), kDefaultBaseUrl).toString();
    m_urlEdit->setText(url);
    m_curVersionLabel->setText(m_client ? m_client->currentVersion()
                                        : QStringLiteral(APP_VERSION));
}

void ota_upgrade::appendLog(const QString &text)
{
    m_log->appendPlainText(
        QStringLiteral("%1 --- %2")
            .arg(QDateTime::currentDateTime().toString(Qt::ISODate), text));
}

void ota_upgrade::onCheckClicked()
{
    appendLog(QStringLiteral("手动检查更新"));
    m_client->checkForUpdate();
}

void ota_upgrade::onUpgradeClicked()
{
    appendLog(QStringLiteral("开始升级"));
    m_upgradeButton->setEnabled(false);
    m_client->downloadAndApply();
}

void ota_upgrade::onVersionChecked(bool hasNew, const QString &newVersion,
                                   const QString &currentVersion)
{
    m_curVersionLabel->setText(currentVersion);
    m_serverVersionLabel->setText(hasNew ? newVersion : currentVersion);
    m_upgradeButton->setEnabled(hasNew);
    appendLog(hasNew ? QStringLiteral("发现新版本 %1,可点击立即升级").arg(newVersion)
                     : QStringLiteral("已是最新版本"));
}

void ota_upgrade::onProgress(qint64 received, qint64 total)
{
    m_progressBar->setRange(0, total > 0 ? static_cast<int>(total) : 100);
    m_progressBar->setValue(static_cast<int>(received));
}

void ota_upgrade::onError(const QString &text)
{
    appendLog(QStringLiteral("错误: %1").arg(text));
    m_upgradeButton->setEnabled(true);
}

void ota_upgrade::onUpdateApplied()
{
    appendLog(QStringLiteral("升级完成,应用 1 秒后自动重启"));
    /* 由 run_project.sh 守护循环拉起新版本;手动启动场景需重新运行 */
    QTimer::singleShot(1000, qApp, &QCoreApplication::quit);
}
