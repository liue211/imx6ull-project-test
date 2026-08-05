#include "mainwidget.h"

#include "form/banna.h"
#include "form/facedetect.h"
#include "form/imx6ulltest.h"
#include "form/mqtt_client.h"
#include "form/musicplay.h"
#include "form/opencv.h"
#include "form/videoplayer.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>

namespace {

/* 页面描述符:标题 + 工厂函数指针。
 * 工厂在注册页面时才创建实例,避免启动时实例化全部模块。 */
struct PageDescriptor {
    QString title;
    QWidget *(*createPage)();
};

const PageDescriptor kPages[] = {
    { QStringLiteral("轮播图"),         []() -> QWidget * { return new banna; } },
    { QStringLiteral("OpenCV测试"),     []() -> QWidget * { return new opencv; } },
    { QStringLiteral("摄像头人脸识别"),   []() -> QWidget * { return new faceDetect; } },
    { QStringLiteral("imx6ull板级设备"), []() -> QWidget * { return new imx6ullTest; } },
    { QStringLiteral("音频播放器"),       []() -> QWidget * { return new MusicPlay; } },
    { QStringLiteral("视频播放器"),       []() -> QWidget * { return new VideoPlayer; } },
    { QStringLiteral("MQTT_CLIENT"),    []() -> QWidget * { return new mqtt_client; } },
};

} // namespace

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("imx6ull 复刻工程"));
    setGeometry(0, 0, 1024, 600);

    buildUi();
    registerPages();
}

MainWidget::~MainWidget() = default;

void MainWidget::buildUi()
{
    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName(QStringLiteral("listWidget"));
    m_listWidget->setStyleSheet(
        QStringLiteral("background-color: black; color: white;"));
    m_listWidget->setMaximumWidth(200);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName(QStringLiteral("stackedWidget"));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_listWidget);
    m_layout->addWidget(m_stackedWidget);

    /* 列表行切换 -> 堆栈页面切换 */
    connect(m_listWidget, &QListWidget::currentRowChanged,
            m_stackedWidget, &QStackedWidget::setCurrentIndex);
}

void MainWidget::registerPages()
{
    for (const auto &page : kPages) {
        auto *item = new QListWidgetItem(page.title);
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(100, 60));
        m_listWidget->addItem(item);

        m_stackedWidget->addWidget(page.createPage());
    }

    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}
