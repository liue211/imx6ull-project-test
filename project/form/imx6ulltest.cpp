#include "imx6ulltest.h"

#include "ap3216c.h"
#include "icm20608.h"
#include "led.h"

#include <QChart>
#include <QChartView>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineSeries>
#include <QPushButton>
#include <QSplineSeries>
#include <QValueAxis>

using namespace QtCharts;

imx6ullTest::imx6ullTest(QWidget *parent)
    : QWidget(parent)
{
    const QString ledPath =
        QStringLiteral("/sys/devices/platform/leds/leds/sys-led/brightness");
    const QString beepPath =
        QStringLiteral("/sys/devices/platform/leds/leds/beep/brightness");

    m_led = new Led(ledPath, this);
    m_beep = new Led(beepPath, this);
    m_ap3216c = new Ap3216c(QStringLiteral("/sys/class/misc/ap3216c"), this);
    m_icm20608 = new Icm20608(QStringLiteral("/dev/icm20608"), this);

    connect(m_ap3216c, &Ap3216c::dataChanged, this, &imx6ullTest::updateAp3216c);
    connect(m_icm20608, &Icm20608::dataChanged, this, &imx6ullTest::updateIcm20608);

    /* ---------- 布局 ---------- */
    auto *root = new QHBoxLayout(this);

    auto *left = new QVBoxLayout();
    auto *boardBox = new QGroupBox(QStringLiteral("板载设备"), this);
    auto *boardLayout = new QGridLayout(boardBox);
    m_ledButton = new QPushButton(boardBox);
    m_beepButton = new QPushButton(boardBox);
    m_ledButton->setCheckable(true);
    m_beepButton->setCheckable(true);
    boardLayout->addWidget(new QLabel(QStringLiteral("LED:"), boardBox), 0, 0);
    boardLayout->addWidget(m_ledButton, 0, 1);
    boardLayout->addWidget(new QLabel(QStringLiteral("BEEP:"), boardBox), 1, 0);
    boardLayout->addWidget(m_beepButton, 1, 1);
    left->addWidget(boardBox);

    /* AP3216C 数值 */
    auto *apBox = new QGroupBox(QStringLiteral("AP3216C 环境光/接近"), this);
    auto *apLayout = new QGridLayout(apBox);
    m_alsLabel = new QLabel(QStringLiteral("-"), apBox);
    m_psLabel = new QLabel(QStringLiteral("-"), apBox);
    m_irLabel = new QLabel(QStringLiteral("-"), apBox);
    apLayout->addWidget(new QLabel(QStringLiteral("ALS:"), apBox), 0, 0);
    apLayout->addWidget(m_alsLabel, 0, 1);
    apLayout->addWidget(new QLabel(QStringLiteral("PS:"), apBox), 1, 0);
    apLayout->addWidget(m_psLabel, 1, 1);
    apLayout->addWidget(new QLabel(QStringLiteral("IR:"), apBox), 2, 0);
    apLayout->addWidget(m_irLabel, 2, 1);
    auto *apStart = new QPushButton(QStringLiteral("启动采集"), apBox);
    auto *apStop = new QPushButton(QStringLiteral("停止采集"), apBox);
    apLayout->addWidget(apStart, 3, 0);
    apLayout->addWidget(apStop, 3, 1);
    left->addWidget(apBox);

    /* ICM20608 数值 */
    auto *icmBox = new QGroupBox(QStringLiteral("ICM20608 六轴"), this);
    auto *icmLayout = new QGridLayout(icmBox);
    m_gxLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_gyLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_gzLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_axLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_ayLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_azLabel = new QLabel(QStringLiteral("-"), icmBox);
    m_tempLabel = new QLabel(QStringLiteral("-"), icmBox);
    const QList<QPair<QString, QLabel *>> items = {
        { QStringLiteral("陀螺仪X:"), m_gxLabel },
        { QStringLiteral("陀螺仪Y:"), m_gyLabel },
        { QStringLiteral("陀螺仪Z:"), m_gzLabel },
        { QStringLiteral("加速度X:"), m_axLabel },
        { QStringLiteral("加速度Y:"), m_ayLabel },
        { QStringLiteral("加速度Z:"), m_azLabel },
        { QStringLiteral("温度:"), m_tempLabel },
    };
    for (int i = 0; i < items.size(); ++i)
        icmLayout->addWidget(new QLabel(items.at(i).first, icmBox), i, 0),
        icmLayout->addWidget(items.at(i).second, i, 1);
    auto *icmStart = new QPushButton(QStringLiteral("启动采集"), icmBox);
    auto *icmStop = new QPushButton(QStringLiteral("停止采集"), icmBox);
    icmLayout->addWidget(icmStart, 7, 0);
    icmLayout->addWidget(icmStop, 7, 1);
    left->addWidget(icmBox);
    left->addStretch();

    /* 右侧曲线图 */
    m_alsSeries = new QSplineSeries(this);
    m_alsSeries->setName(QStringLiteral("ALS"));
    m_irSeries = new QSplineSeries(this);
    m_irSeries->setName(QStringLiteral("IR"));
    m_psSeries = new QSplineSeries(this);
    m_psSeries->setName(QStringLiteral("PS"));

    auto *chart = new QChart();
    chart->addSeries(m_alsSeries);
    chart->addSeries(m_irSeries);
    chart->addSeries(m_psSeries);
    auto *axisX = new QValueAxis(chart);
    auto *axisY = new QValueAxis(chart);
    axisX->setRange(0, 40);
    axisY->setRange(0, 4000);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    m_alsSeries->attachAxis(axisX);
    m_alsSeries->attachAxis(axisY);
    m_irSeries->attachAxis(axisX);
    m_irSeries->attachAxis(axisY);
    m_psSeries->attachAxis(axisX);
    m_psSeries->attachAxis(axisY);
    chart->legend()->setVisible(true);
    chart->setTitle(QStringLiteral("AP3216C 实时曲线"));

    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    root->addLayout(left, 0);
    root->addWidget(m_chartView, 1);

    /* ---------- 信号槽 ---------- */
    connect(m_ledButton, &QPushButton::clicked, this, &imx6ullTest::toggleLed);
    connect(m_beepButton, &QPushButton::clicked, this, &imx6ullTest::toggleBeep);
    connect(apStart, &QPushButton::clicked, this, &imx6ullTest::startAp3216c);
    connect(apStop, &QPushButton::clicked, this, &imx6ullTest::stopAp3216c);
    connect(icmStart, &QPushButton::clicked, this, &imx6ullTest::startIcm20608);
    connect(icmStop, &QPushButton::clicked, this, &imx6ullTest::stopIcm20608);

    /* 初始状态 */
    m_ledButton->setText(m_led->isOn() ? QStringLiteral("点亮(当前开)")
                                       : QStringLiteral("点亮(当前关)"));
    m_beepButton->setText(m_beep->isOn() ? QStringLiteral("打开(当前开)")
                                         : QStringLiteral("打开(当前关)"));
}

void imx6ullTest::toggleLed()
{
    const bool next = !m_led->isOn();
    if (m_led->setOn(next))
        m_ledButton->setText(next ? QStringLiteral("点亮(当前开)")
                                  : QStringLiteral("点亮(当前关)"));
    else
        m_ledButton->setText(QStringLiteral("未获取到LED设备"));
}

void imx6ullTest::toggleBeep()
{
    const bool next = !m_beep->isOn();
    if (m_beep->setOn(next))
        m_beepButton->setText(next ? QStringLiteral("打开(当前开)")
                                   : QStringLiteral("打开(当前关)"));
    else
        m_beepButton->setText(QStringLiteral("未获取到BEEP设备"));
}

void imx6ullTest::startAp3216c()
{
    m_ap3216c->setCapture(true);
    m_ap3216c->poll();
}

void imx6ullTest::stopAp3216c()
{
    m_ap3216c->setCapture(false);
}

void imx6ullTest::startIcm20608()
{
    m_icm20608->setCapture(true);
    m_icm20608->refresh();
}

void imx6ullTest::stopIcm20608()
{
    m_icm20608->setCapture(false);
}

void imx6ullTest::updateAp3216c()
{
    m_alsLabel->setText(m_ap3216c->alsData());
    m_psLabel->setText(m_ap3216c->psData());
    m_irLabel->setText(m_ap3216c->irData());
    updateChart();
}

void imx6ullTest::updateIcm20608()
{
    m_gxLabel->setText(m_icm20608->gxData());
    m_gyLabel->setText(m_icm20608->gyData());
    m_gzLabel->setText(m_icm20608->gzData());
    m_axLabel->setText(m_icm20608->axData());
    m_ayLabel->setText(m_icm20608->ayData());
    m_azLabel->setText(m_icm20608->azData());
    m_tempLabel->setText(m_icm20608->tempData());
}

void imx6ullTest::updateChart()
{
    bool ok = false;
    const double als = m_ap3216c->alsData().toDouble(&ok);
    const double ir = m_ap3216c->irData().toDouble(&ok);
    const double ps = m_ap3216c->psData().toDouble(&ok);
    if (!ok)
        return;
    const double x = m_pointCount++;
    m_alsSeries->append(x, als);
    m_irSeries->append(x, ir);
    m_psSeries->append(x, ps);
    if (m_pointCount > 40) {
        m_alsSeries->remove(0);
        m_irSeries->remove(0);
        m_psSeries->remove(0);
    }
}
