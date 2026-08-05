#include "icm20608.h"

#include <QFile>
#include <cstring>

Icm20608::Icm20608(const QString &devicePath, QObject *parent)
    : QObject(parent)
    , m_device(devicePath)
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &Icm20608::poll);
}

bool Icm20608::readRaw(int raw[7]) const
{
    QFile file(m_device);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    const QByteArray data = file.read(7 * sizeof(int));
    file.close();
    if (data.size() < 7 * sizeof(int))
        return false;

    for (int i = 0; i < 7; ++i) {
        qint32 v;
        memcpy(&v, data.constData() + i * sizeof(int), sizeof(v));
        raw[i] = static_cast<int>(v);
    }
    return true;
}

void Icm20608::refresh()
{
    int raw[7];
    if (!readRaw(raw)) {
        const QString err = QStringLiteral("设备不存在");
        m_gx = m_gy = m_gz = m_ax = m_ay = m_az = m_temp = err;
    } else {
        m_gx = QString::number(convertGyro(raw[0]), 'f', 2);
        m_gy = QString::number(convertGyro(raw[1]), 'f', 2);
        m_gz = QString::number(convertGyro(raw[2]), 'f', 2);
        m_ax = QString::number(convertAccel(raw[3]), 'f', 2);
        m_ay = QString::number(convertAccel(raw[4]), 'f', 2);
        m_az = QString::number(convertAccel(raw[5]), 'f', 2);
        m_temp = QString::number(convertTemp(raw[6]), 'f', 2);
    }
    emit dataChanged();
}

QString Icm20608::gxData() const { return m_gx; }
QString Icm20608::gyData() const { return m_gy; }
QString Icm20608::gzData() const { return m_gz; }
QString Icm20608::axData() const { return m_ax; }
QString Icm20608::ayData() const { return m_ay; }
QString Icm20608::azData() const { return m_az; }
QString Icm20608::tempData() const { return m_temp; }

void Icm20608::setCapture(bool enabled)
{
    if (enabled)
        m_timer.start();
    else
        m_timer.stop();
}

void Icm20608::poll()
{
    refresh();
}
