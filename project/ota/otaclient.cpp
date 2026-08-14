#include "otaclient.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QCryptographicHash>

OtaClient::OtaClient(QObject *parent)
    : QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_applyProcess = new QProcess(this);
    connect(m_applyProcess, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, &OtaClient::onApplyProcessFinished);
}

int OtaClient::compareVersions(const QString &a, const QString &b)
{
    const QStringList as = a.split(QLatin1Char('.'));
    const QStringList bs = b.split(QLatin1Char('.'));
    const int n = qMax(as.size(), bs.size());
    for (int i = 0; i < n; ++i) {
        const int av = i < as.size() ? as[i].toInt() : 0;
        const int bv = i < bs.size() ? bs[i].toInt() : 0;
        if (av != bv)
            return av > bv ? 1 : -1;
    }
    return 0;
}

bool OtaClient::parseVersionJson(const QByteArray &json, QString *version,
                                 QString *url, QByteArray *md5)
{
    QJsonParseError err;
    const QJsonDocument doc = QJsonDocument::fromJson(json, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    const QJsonObject obj = doc.object();
    const QString ver = obj.value(QStringLiteral("version")).toString();
    const QString u = obj.value(QStringLiteral("url")).toString();
    if (ver.isEmpty() || u.isEmpty())
        return false;

    if (version)
        *version = ver;
    if (url)
        *url = u;
    if (md5)
        *md5 = obj.value(QStringLiteral("md5")).toString().toLatin1();
    return true;
}

QByteArray OtaClient::fileMd5(const QString &path)
{
    QFile f(path);
    if (!f.open(QFile::ReadOnly))
        return QByteArray();

    QCryptographicHash hash(QCryptographicHash::Md5);
    QByteArray buf;
    buf.resize(64 * 1024);
    qint64 n = 0;
    while ((n = f.read(buf.data(), buf.size())) > 0)
        hash.addData(buf.constData(), static_cast<int>(n));
    return hash.result().toHex();
}

void OtaClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    while (m_baseUrl.endsWith(QLatin1Char('/')))
        m_baseUrl.chop(1);
}

void OtaClient::checkForUpdate()
{
    if (m_busy) {
        emitLog(QStringLiteral("上一操作未完成,请稍候"));
        return;
    }
    if (m_baseUrl.isEmpty()) {
        emit errorOccurred(QStringLiteral("服务器地址为空"));
        return;
    }

    m_busy = true;
    const QUrl url(makeUrl(QStringLiteral("version.json")));
    emitLog(QStringLiteral("检查版本: %1 (当前 %2)").arg(url.toString(),
                                                         currentVersion()));
    m_reply = m_nam->get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::finished, this, &OtaClient::onVersionReplyFinished);
}

void OtaClient::onVersionReplyFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    m_busy = false;
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QStringLiteral("检查版本失败: %1").arg(reply->errorString()));
        return;
    }

    QString ver;
    QString url;
    QByteArray md5;
    if (!parseVersionJson(reply->readAll(), &ver, &url, &md5)) {
        emit errorOccurred(QStringLiteral("version.json 解析失败"));
        return;
    }

    m_newVersion = ver;
    m_packageUrl = url;
    m_expectedMd5 = md5;

    const int cmp = compareVersions(ver, currentVersion());
    const bool hasNew = cmp > 0;
    emitLog(hasNew ? QStringLiteral("发现新版本: %1").arg(ver)
                   : QStringLiteral("已是最新版本(%1)").arg(currentVersion()));
    emit versionChecked(hasNew, ver, currentVersion());
}

void OtaClient::downloadAndApply()
{
    if (m_busy) {
        emitLog(QStringLiteral("上一操作未完成,请稍候"));
        return;
    }
    if (m_newVersion.isEmpty() || m_packageUrl.isEmpty()) {
        emit errorOccurred(QStringLiteral("请先检查版本"));
        return;
    }

    m_busy = true;
    const QString fileName = QFileInfo(m_packageUrl).fileName();
    if (fileName.isEmpty()) {
        m_busy = false;
        emit errorOccurred(QStringLiteral("升级包 URL 无效: %1").arg(m_packageUrl));
        return;
    }

    m_downloadPath = cacheDir() + QLatin1Char('/') + fileName;
    QDir().mkpath(cacheDir());
    m_downloadFile = new QFile(m_downloadPath, this);
    if (!m_downloadFile->open(QFile::WriteOnly | QFile::Truncate)) {
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
        m_busy = false;
        emit errorOccurred(QStringLiteral("无法创建下载文件: %1").arg(m_downloadPath));
        return;
    }

    const QUrl url(makeUrl(m_packageUrl));
    emitLog(QStringLiteral("开始下载: %1").arg(url.toString()));
    m_reply = m_nam->get(QNetworkRequest(url));
    connect(m_reply, &QNetworkReply::readyRead, this, &OtaClient::onReadyRead);
    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &OtaClient::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &OtaClient::onDownloadReplyFinished);
}

void OtaClient::onReadyRead()
{
    if (m_downloadFile)
        m_downloadFile->write(m_reply->readAll());
}

void OtaClient::onDownloadProgress(qint64 received, qint64 total)
{
    emit downloadProgress(received, total);
}

void OtaClient::onDownloadReplyFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();

    if (m_downloadFile) {
        m_downloadFile->close();
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }
    m_busy = false;

    if (reply->error() != QNetworkReply::NoError) {
        QFile::remove(m_downloadPath);
        emit errorOccurred(QStringLiteral("下载失败: %1").arg(reply->errorString()));
        return;
    }

    /* MD5 校验:与服务器 version.json 中的 md5 字段比对 */
    const QByteArray actual = fileMd5(m_downloadPath);
    if (m_expectedMd5.isEmpty()) {
        emitLog(QStringLiteral("警告: version.json 未提供 md5,跳过校验"));
    } else if (actual != m_expectedMd5) {
        QFile::remove(m_downloadPath);
        emit errorOccurred(QStringLiteral("MD5 校验失败(期望 %1,实际 %2),已删除坏包")
                               .arg(QString::fromLatin1(m_expectedMd5),
                                    QString::fromLatin1(actual)));
        return;
    }
    emitLog(QStringLiteral("下载完成,MD5 校验通过: %1").arg(m_downloadPath));

    /* 调用板端脚本:解压到新版本目录 + 切换 symlink(脚本内做回滚点记录) */
    const QString versionDir =
        QStringLiteral("project_v%1").arg(m_newVersion);
    emitLog(QStringLiteral("应用升级: %1 -> %2")
                .arg(m_downloadPath, versionDir));
    m_busy = true;
    m_applyProcess->start(m_applyScript,
                          {versionDir, m_downloadPath, currentVersion()});
}

void OtaClient::onApplyProcessFinished(int exitCode)
{
    m_busy = false;
    if (exitCode == 0) {
        emitLog(QStringLiteral("升级包已应用,应用即将重启"));
        emit updateApplied();
    } else {
        emit errorOccurred(QStringLiteral("升级脚本执行失败(退出码 %1)").arg(exitCode));
    }
}

QString OtaClient::makeUrl(const QString &pathOrUrl) const
{
    if (pathOrUrl.startsWith(QStringLiteral("http://"))
        || pathOrUrl.startsWith(QStringLiteral("https://")))
        return pathOrUrl;
    return m_baseUrl + QLatin1Char('/') + pathOrUrl;
}

QString OtaClient::cacheDir() const
{
    /* 与 myMusic/myVideo 同约定:exe 同目录下的运行时目录 */
    return QCoreApplication::applicationDirPath() + QStringLiteral("/ota_cache");
}

void OtaClient::emitLog(const QString &text)
{
    emit logMessage(text);
}
