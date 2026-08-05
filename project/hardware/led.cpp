#include "led.h"

#include <QFile>

Led::Led(const QString &sysfsPath, QObject *parent)
    : QObject(parent)
    , m_path(sysfsPath)
{
}

QString Led::sysfsPath() const
{
    return m_path;
}

bool Led::isOn() const
{
    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    const QByteArray value = file.readAll().trimmed();
    file.close();
    return value == "1";
}

bool Led::setOn(bool on)
{
    QFile file(m_path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    const qint64 written = file.write(on ? "1" : "0");
    file.close();
    return written == 1;
}
