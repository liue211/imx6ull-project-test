#include <QtTest/QtTest>

#include <QFile>
#include <QTemporaryDir>

#include "../../project/ota/otaclient.h"

/* OTA 客户端纯函数验收:版本比较 / version.json 解析 / MD5 校验 */
class TestOtaClient : public QObject
{
    Q_OBJECT

private slots:
    void compareVersions_data();
    void compareVersions();
    void parseVersionJson();
    void parseVersionJsonRejectsBadInput();
    void fileMd5MatchesKnownValue();
    void fileMd5FailsOnMissingFile();
};

void TestOtaClient::compareVersions_data()
{
    QTest::addColumn<QString>("a");
    QTest::addColumn<QString>("b");
    QTest::addColumn<int>("expected");

    QTest::newRow("equal") << QStringLiteral("1.0.0") << QStringLiteral("1.0.0") << 0;
    QTest::newRow("patch bump") << QStringLiteral("1.0.1") << QStringLiteral("1.0.0") << 1;
    QTest::newRow("major bump") << QStringLiteral("2.0.0") << QStringLiteral("1.99.99") << 1;
    QTest::newRow("numeric segment compare") << QStringLiteral("1.2.0") << QStringLiteral("1.10.0") << -1;
    QTest::newRow("missing segment is zero") << QStringLiteral("1.1") << QStringLiteral("1.1.0") << 0;
    QTest::newRow("extra segment wins") << QStringLiteral("1.0.0.1") << QStringLiteral("1.0.0") << 1;
    QTest::newRow("downgrade") << QStringLiteral("1.0.0") << QStringLiteral("1.1.0") << -1;
}

void TestOtaClient::compareVersions()
{
    QFETCH(QString, a);
    QFETCH(QString, b);
    QFETCH(int, expected);

    QCOMPARE(OtaClient::compareVersions(a, b), expected);
    QCOMPARE(OtaClient::compareVersions(b, a), -expected);
}

void TestOtaClient::parseVersionJson()
{
    QString version;
    QString url;
    QByteArray md5;
    const QByteArray json =
        "{\"version\":\"1.1.0\",\"url\":\"project_1.1.0.tar.gz\","
        "\"md5\":\"0123456789abcdef0123456789abcdef\"}";

    QVERIFY(OtaClient::parseVersionJson(json, &version, &url, &md5));
    QCOMPARE(version, QStringLiteral("1.1.0"));
    QCOMPARE(url, QStringLiteral("project_1.1.0.tar.gz"));
    QCOMPARE(md5, QByteArray("0123456789abcdef0123456789abcdef"));
}

void TestOtaClient::parseVersionJsonRejectsBadInput()
{
    QString version;
    QString url;
    QByteArray md5;

    QVERIFY(!OtaClient::parseVersionJson("not json", &version, &url, &md5));
    QVERIFY(!OtaClient::parseVersionJson("[]", &version, &url, &md5));
    QVERIFY(!OtaClient::parseVersionJson("{\"url\":\"x\"}", &version, &url, &md5));
    QVERIFY(!OtaClient::parseVersionJson("{\"version\":\"1.0.0\"}", &version, &url, &md5));
}

void TestOtaClient::fileMd5MatchesKnownValue()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString path = dir.filePath(QStringLiteral("hello.txt"));
    QFile f(path);
    QVERIFY(f.open(QFile::WriteOnly));
    f.write("hello");
    f.close();

    /* "hello" 的 MD5 为已知值,验证实现与标准一致 */
    QCOMPARE(OtaClient::fileMd5(path),
             QByteArray("5d41402abc4b2a76b9719d911017c592"));
}

void TestOtaClient::fileMd5FailsOnMissingFile()
{
    QVERIFY(OtaClient::fileMd5(QStringLiteral("C:/__no_such_file__.bin")).isEmpty());
}

QTEST_MAIN(TestOtaClient)

#include "tst_ota.moc"
