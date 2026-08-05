#include "mainwidget.h"

#include <QApplication>
#include <QFile>

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

    MainWidget w;
    w.show();

    return a.exec();
}
