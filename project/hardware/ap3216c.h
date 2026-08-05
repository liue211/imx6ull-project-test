#ifndef AP3216C_H
#define AP3216C_H

#include <QObject>
#include <QString>
#include <QTimer>

/* AP3216C 环境光/红外/接近传感器。
 * 板端由驱动导出 /sys/class/misc/ap3216c/{als,ps,ir},定时轮询。 */
class Ap3216c : public QObject
{
    Q_OBJECT

public:
    explicit Ap3216c(const QString &sysfsDir =
                         QStringLiteral("/sys/class/misc/ap3216c"),
                     QObject *parent = nullptr);

    QString readAls() const;    /* 环境光 */
    QString readPs() const;     /* 接近 */
    QString readIr() const;     /* 红外 */
    QString alsData() const;
    QString psData() const;
    QString irData() const;

    void setCapture(bool enabled);  /* 500ms 轮询 */
    void poll();                    /* 立即读取一次 */

signals:
    void dataChanged();

private:
    QString readNode(const QString &name) const;

    QString m_dir;
    QTimer m_timer;
    QString m_als, m_ps, m_ir;
};

#endif // AP3216C_H
