#include <QtTest/QtTest>

#include <QFile>
#include <QTemporaryDir>

#include "../../project/hardware/led.h"
#include "../../project/hardware/ap3216c.h"
#include "../../project/hardware/icm20608.h"

/* 硬件层逻辑测试:sysfs/设备路径可注入,用临时目录模拟 */
class TestHardware : public QObject
{
    Q_OBJECT

private slots:
    void ledWritesAndReadsState();
    void ledMissingFileIsGraceful();
    void ap3216cReadsSysfsNodes();
    void ap3216cMissingDirIsGraceful();
    void icm20608ConvertsRawValues();
};

void TestHardware::ledWritesAndReadsState()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.path() + QStringLiteral("/brightness");

    Led led(path);
    QVERIFY(led.setOn(true));
    QVERIFY(led.isOn());

    QVERIFY(led.setOn(false));
    QVERIFY(!led.isOn());
}

void TestHardware::ledMissingFileIsGraceful()
{
    Led led(QStringLiteral("C:/__no_such_device__/brightness"));
    QVERIFY(!led.isOn());
    QVERIFY(!led.setOn(true));
}

void TestHardware::ap3216cReadsSysfsNodes()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile als(dir.path() + QStringLiteral("/als"));
    QVERIFY(als.open(QIODevice::WriteOnly));
    als.write("1234\n");
    als.close();
    QFile ir(dir.path() + QStringLiteral("/ir"));
    QVERIFY(ir.open(QIODevice::WriteOnly));
    ir.write("55\n");
    ir.close();

    Ap3216c sensor(dir.path());
    QCOMPARE(sensor.readAls(), QStringLiteral("1234"));
    QCOMPARE(sensor.readIr(), QStringLiteral("55"));
    QCOMPARE(sensor.readPs(), QStringLiteral("设备不存在"));
}

void TestHardware::ap3216cMissingDirIsGraceful()
{
    Ap3216c sensor(QStringLiteral("C:/__no_such_device__/ap3216c"));
    QCOMPARE(sensor.readAls(), QStringLiteral("设备不存在"));
}

void TestHardware::icm20608ConvertsRawValues()
{
    QCOMPARE(Icm20608::convertGyro(164), 10.0);
    QCOMPARE(Icm20608::convertAccel(2048), 1.0);
    QCOMPARE(Icm20608::convertTemp(25), 25.0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QFile dev(dir.path() + QStringLiteral("/icm20608"));
    QVERIFY(dev.open(QIODevice::WriteOnly));
    const int raw[7] = {164, 0, 0, 2048, 0, 0, 25};
    dev.write(reinterpret_cast<const char *>(raw), sizeof(raw));
    dev.close();

    Icm20608 sensor(dev.fileName());
    int read[7] = {0, 0, 0, 0, 0, 0, 0};
    QVERIFY(sensor.readRaw(read));
    QCOMPARE(read[0], 164);
    QCOMPARE(read[3], 2048);
    QCOMPARE(read[6], 25);
}

QTEST_MAIN(TestHardware)

#include "tst_hardware.moc"
