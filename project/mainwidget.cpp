#include "mainwidget.h"

#include "form/banna.h"
#include "form/facedetect.h"
#include "form/imx6ulltest.h"
#include "form/mqtt_client.h"
#include "form/musicplay.h"
#include "form/opencv.h"
#include "form/videoplayer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QStackedWidget>
#include <QTimer>
#include <QTime>
#include <QVBoxLayout>

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
    setObjectName(QStringLiteral("project"));

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("headerBar"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(8);

    m_titleLabel = new QLabel(QStringLiteral("imx6ull 工程"), header);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));

    m_listWidget = new QListWidget(header);
    m_listWidget->setObjectName(QStringLiteral("listWidget"));
    m_listWidget->setFlow(QListView::LeftToRight);
    m_listWidget->setWrapping(false);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFixedHeight(64);

    m_timeLabel = new QLabel(QStringLiteral("--:--"), header);
    m_timeLabel->setObjectName(QStringLiteral("timeLabel"));

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addSpacing(16);
    headerLayout->addWidget(m_listWidget, 1);
    headerLayout->addWidget(m_timeLabel);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName(QStringLiteral("stackedWidget"));

    root->addWidget(header);
    root->addWidget(m_stackedWidget, 1);

    connect(m_listWidget, &QListWidget::currentRowChanged,
            m_stackedWidget, &QStackedWidget::setCurrentIndex);

    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWidget::updateClock);
    m_clockTimer->start();
    updateClock();
}

void MainWidget::updateClock()
{
    m_timeLabel->setText(QTime::currentTime().toString(QStringLiteral("HH:mm")));
}

void MainWidget::registerPages()
{
    for (const auto &page : kPages) {
        auto *item = new QListWidgetItem(page.title);
        item->setTextAlignment(Qt::AlignCenter);
        item->setSizeHint(QSize(110, 44));
        m_listWidget->addItem(item);

        m_stackedWidget->addWidget(page.createPage());
    }

    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}
