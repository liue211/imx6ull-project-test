#ifndef ICM20608_H
#define ICM20608_H

#include <QObject>
#include <QString>
#include <QTimer>

/* ICM20608 六轴传感器。板端为字符设备 /dev/icm20608,
 * read 一次得到 7 个有符号 int:gx gy gz ax ay az temp(ADC)。 */
class Icm20608 : public QObject
{
    Q_OBJECT

public:
    explicit Icm20608(const QString &devicePath =
                          QStringLiteral("/dev/icm20608"),
                      QObject *parent = nullptr);

    /* 纯函数:ADC 原始值 -> 实际值(gyro /16.4, accel /2048, 温度公式) */
    static double gyroScale() { return 16.4; }
    static double accelScale() { return 2048.0; }
    static double tempScale() { return 326.8; }
    static double convertGyro(int adc) { return adc / gyroScale(); }
    static double convertAccel(int adc) { return adc / accelScale(); }
    static double convertTemp(int adc) { return (adc - 25) / tempScale() + 25; }

    bool readRaw(int raw[7]) const;   /* 成功返回 true */
    void refresh();                   /* 读取并更新缓存 */

    QString gxData() const;
    QString gyData() const;
    QString gzData() const;
    QString axData() const;
    QString ayData() const;
    QString azData() const;
    QString tempData() const;

    void setCapture(bool enabled);    /* 500ms 轮询 */

signals:
    void dataChanged();

private slots:
    void poll();

private:
    QString m_device;
    QTimer m_timer;
    QString m_gx, m_gy, m_gz, m_ax, m_ay, m_az, m_temp;
};

#endif // ICM20608_H
