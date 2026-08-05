#ifndef LED_H
#define LED_H

#include <QObject>
#include <QString>

/* 用户态操作 sysfs LED:
 *   写 "1"/"0" 到 brightness 节点即可点亮/熄灭。
 * 路径可注入,便于本机(无设备)测试与板端使用。 */
class Led : public QObject
{
    Q_OBJECT

public:
    explicit Led(const QString &sysfsPath =
                     QStringLiteral("/sys/class/leds/sys-led/brightness"),
                 QObject *parent = nullptr);

    QString sysfsPath() const;
    bool isOn() const;      /* 读取当前状态,文件不存在时返回 false */
    bool setOn(bool on);    /* 写状态,返回是否成功 */

private:
    QString m_path;
};

#endif // LED_H
