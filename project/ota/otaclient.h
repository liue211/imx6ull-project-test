#ifndef OTACLIENT_H
#define OTACLIENT_H

#include <QObject>
#include <QByteArray>

class QFile;
class QNetworkAccessManager;
class QNetworkReply;
class QProcess;

#ifndef APP_VERSION
/* 版本号由 project.pro 的 DEFINES 注入,未注入时用默认值(测试工程等场景) */
#define APP_VERSION "1.0.0"
#endif

/* OTA 升级客户端(升级计划·功能组三)
 *
 * 协议(version.json,由 scripts/ota_server 提供):
 *   {"version":"1.1.0","url":"project_1.1.0.tar.gz","md5":"<32位hex>"}
 * 流程: 检查版本 → 下载升级包 → MD5 校验 → 调 apply_update.sh 切换 → 应用重启
 * 目录约定: /home/root/project -> project_vX.Y.Z(symlink),见 docs/ota-说明.md */
class OtaClient : public QObject
{
    Q_OBJECT

public:
    explicit OtaClient(QObject *parent = nullptr);

    /* 版本号按 '.' 分段数值比较: a > b 返回 1, a < b 返回 -1, 相等返回 0
     * (纯函数,供 QtTest 直接测试) */
    static int compareVersions(const QString &a, const QString &b);

    /* 解析 version.json,字段缺失返回 false(纯函数,可测试) */
    static bool parseVersionJson(const QByteArray &json, QString *version,
                                 QString *url, QByteArray *md5);

    /* 计算文件 MD5,失败返回空(纯函数,可测试) */
    static QByteArray fileMd5(const QString &path);

    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }

    /* 板端 apply_update.sh 路径,可注入以便本机模拟验证 */
    void setApplyScriptPath(const QString &path) { m_applyScript = path; }

    QString currentVersion() const { return QStringLiteral(APP_VERSION); }
    bool isBusy() const { return m_busy; }

public slots:
    void checkForUpdate();
    void downloadAndApply();

signals:
    /* 检查完成: hasNewVersion 表示服务器版本高于当前版本 */
    void versionChecked(bool hasNewVersion, const QString &newVersion,
                        const QString &currentVersion);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void logMessage(const QString &text);
    void errorOccurred(const QString &text);
    /* 升级包已应用(apply_update.sh 切换完成),调用方应重启应用 */
    void updateApplied();

private slots:
    void onVersionReplyFinished();
    void onReadyRead();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadReplyFinished();
    void onApplyProcessFinished(int exitCode);

private:
    QString makeUrl(const QString &pathOrUrl) const;
    QString cacheDir() const;
    void emitLog(const QString &text);

    QNetworkAccessManager *m_nam = nullptr;
    QNetworkReply *m_reply = nullptr;
    QProcess *m_applyProcess = nullptr;
    QFile *m_downloadFile = nullptr;

    QString m_baseUrl;
    QString m_applyScript = QStringLiteral("/home/root/apply_update.sh");

    /* checkForUpdate 的结果缓存,供 downloadAndApply 使用 */
    QString m_newVersion;
    QString m_packageUrl;
    QByteArray m_expectedMd5;
    QString m_downloadPath;

    bool m_busy = false;
};

#endif // OTACLIENT_H
