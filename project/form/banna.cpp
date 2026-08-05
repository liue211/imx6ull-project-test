#include "banna.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

banna::banna(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    for (int i = 1; i <= 5; ++i)
        m_imagePaths << QStringLiteral(":/images/banna_pic/banna/%1.jpg").arg(i);

    m_timer = new QTimer(this);
    m_timer->setInterval(3000);
    connect(m_timer, &QTimer::timeout, this, &banna::showNext);
    m_timer->start();

    showImage(0);
}

void banna::setupUi()
{
    /* 主容器:图片 + 左右箭头 */
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 10);

    auto *imageArea = new QWidget(this);
    auto *imageLayout = new QHBoxLayout(imageArea);
    imageLayout->setContentsMargins(0, 0, 0, 0);

    m_imageLabel = new QLabel(imageArea);
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setScaledContents(true);
    m_imageLabel->setMinimumSize(480, 360);

    m_leftButton = new QPushButton(QStringLiteral("◀"), imageArea);
    m_rightButton = new QPushButton(QStringLiteral("▶"), imageArea);
    m_leftButton->setFixedSize(48, 48);
    m_rightButton->setFixedSize(48, 48);

    imageLayout->addWidget(m_leftButton);
    imageLayout->addWidget(m_imageLabel, 1);
    imageLayout->addWidget(m_rightButton);

    /* 底部圆点 */
    auto *dotBar = new QWidget(this);
    m_dotLayout = new QHBoxLayout(dotBar);
    m_dotLayout->setContentsMargins(0, 8, 0, 0);
    m_dotLayout->setAlignment(Qt::AlignCenter);
    m_dotGroup = new QButtonGroup(this);
    m_dotGroup->setExclusive(true);

    for (int i = 0; i < 5; ++i) {
        auto *dot = new QPushButton(dotBar);
        dot->setCheckable(true);
        dot->setFixedSize(14, 14);
        dot->setCursor(Qt::PointingHandCursor);
        m_dotLayout->addWidget(dot);
        m_dotGroup->addButton(dot, i);
    }

    mainLayout->addWidget(imageArea, 1);
    mainLayout->addWidget(dotBar);

    connect(m_leftButton, &QPushButton::clicked, this, &banna::showPrev);
    connect(m_rightButton, &QPushButton::clicked, this, &banna::showNext);
    connect(m_dotGroup, qOverload<int>(&QButtonGroup::idClicked),
            this, &banna::jumpTo);

    /* 滑动动画 */
    m_anim = new QPropertyAnimation(m_imageLabel, "pos", this);
    m_anim->setDuration(250);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void banna::showImage(int index)
{
    m_imageLabel->setPixmap(QPixmap(m_imagePaths.at(index)));
    m_current = index;
    updateDots();
}

void banna::updateDots()
{
    if (m_dotGroup->button(m_current))
        m_dotGroup->button(m_current)->setChecked(true);
}

void banna::showNext()
{
    m_timer->start();   /* 手动切换后重新计时 */
    m_imageLabel->setPixmap(QPixmap(m_imagePaths.at((m_current + 1) % m_imagePaths.size())));
    m_anim->stop();
    m_anim->setStartValue(QPoint(m_imageLabel->width(), 0));
    m_anim->setEndValue(QPoint(0, 0));
    m_current = (m_current + 1) % m_imagePaths.size();
    updateDots();
    m_anim->start();
}

void banna::showPrev()
{
    m_timer->start();
    m_imageLabel->setPixmap(QPixmap(m_imagePaths.at(
        (m_current - 1 + m_imagePaths.size()) % m_imagePaths.size())));
    m_anim->stop();
    m_anim->setStartValue(QPoint(-m_imageLabel->width(), 0));
    m_anim->setEndValue(QPoint(0, 0));
    m_current = (m_current - 1 + m_imagePaths.size()) % m_imagePaths.size();
    updateDots();
    m_anim->start();
}

void banna::jumpTo(int index)
{
    if (index == m_current)
        return;
    m_timer->start();
    showImage(index);
}
