#include "ap3216c.h"

#include <QFile>

Ap3216c::Ap3216c(const QString &sysfsDir, QObject *parent)
    : QObject(parent)
    , m_dir(sysfsDir)
{
    m_timer.setInterval(500);
    connect(&m_timer, &QTimer::timeout, this, &Ap3216c::poll);
}

QString Ap3216c::readNode(const QString &name) const
{
    QFile file(m_dir + QLatin1Char('/') + name);
    if (!file.open(QIODevice::ReadOnly))
        return QStringLiteral("设备不存在");
    const QByteArray data = file.readAll().trimmed();
    file.close();
    return QString::fromUtf8(data);
}

QString Ap3216c::readAls() const
{
    return readNode(QStringLiteral("als"));
}

QString Ap3216c::readPs() const
{
    return readNode(QStringLiteral("ps"));
}

QString Ap3216c::readIr() const
{
    return readNode(QStringLiteral("ir"));
}

QString Ap3216c::alsData() const
{
    return m_als;
}

QString Ap3216c::psData() const
{
    return m_ps;
}

QString Ap3216c::irData() const
{
    return m_ir;
}

void Ap3216c::setCapture(bool enabled)
{
    if (enabled)
        m_timer.start();
    else
        m_timer.stop();
}

void Ap3216c::poll()
{
    m_als = readAls();
    m_ps = readPs();
    m_ir = readIr();
    emit dataChanged();
}
