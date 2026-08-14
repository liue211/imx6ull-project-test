#include "mainwidget.h"
#include "ota/otaclient.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* 加载全局样式 */
    QFile qss(QStringLiteral(":/style/mainstyle.qss"));
    if (qss.exists()) {
        qss.open(QFile::ReadOnly);
        a.setStyleSheet(QString::fromUtf8(qss.readAll()));
        qss.close();
    }

    /* 启动成功标记:run_project.sh 的升级回滚看门狗据此判断本版本是否可用 */
    QFile bootOk(QDir::tempPath() + QStringLiteral("/.ota_boot_ok"));
    if (bootOk.open(QFile::WriteOnly | QFile::Truncate))
        bootOk.write(QByteArrayLiteral(APP_VERSION));

    /* 后台静默检查版本(不阻塞启动;结果走日志,升级操作在 OTA 页面) */
    QSettings settings(QStringLiteral("ota.ini"), QSettings::IniFormat);
    auto *ota = new OtaClient(&a);
    ota->setBaseUrl(settings.value(
        QStringLiteral("ota/baseUrl"),
        QStringLiteral("http://192.168.137.1:8000")).toString());
    QObject::connect(ota, &OtaClient::versionChecked,
                     [](bool hasNew, const QString &newVersion,
                        const QString &currentVersion) {
        qInfo("OTA: %s", qPrintable(
            hasNew ? QStringLiteral("发现新版本 %1(当前 %2)")
                         .arg(newVersion, currentVersion)
                   : QStringLiteral("已是最新版本 %1").arg(currentVersion)));
    });
    QObject::connect(ota, &OtaClient::errorOccurred, [](const QString &text) {
        qInfo("OTA: %s", qPrintable(text));
    });
    ota->checkForUpdate();

    MainWidget w;
    w.show();

    return a.exec();
}
