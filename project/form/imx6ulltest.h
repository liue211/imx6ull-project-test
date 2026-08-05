#ifndef IMX6ULLTEST_H
#define IMX6ULLTEST_H

#include <QWidget>

class Ap3216c;
class Icm20608;
class Led;
class QLabel;
class QPushButton;

namespace QtCharts {
class QChartView;
class QLineSeries;
class QSplineSeries;
}

/* imx6ull 板级设备:LED / 蜂鸣器 / AP3216C 曲线 / ICM20608 六轴 */
class imx6ullTest : public QWidget
{
    Q_OBJECT

public:
    explicit imx6ullTest(QWidget *parent = nullptr);

private slots:
    void toggleLed();
    void toggleBeep();
    void startAp3216c();
    void stopAp3216c();
    void startIcm20608();
    void stopIcm20608();
    void updateAp3216c();
    void updateIcm20608();

private:
    QWidget *buildSensorPanel();
    void updateChart();

    Ap3216c *m_ap3216c = nullptr;
    Icm20608 *m_icm20608 = nullptr;
    Led *m_led = nullptr;
    Led *m_beep = nullptr;

    QPushButton *m_ledButton = nullptr;
    QPushButton *m_beepButton = nullptr;
    QLabel *m_alsLabel = nullptr;
    QLabel *m_psLabel = nullptr;
    QLabel *m_irLabel = nullptr;
    QLabel *m_gxLabel = nullptr;
    QLabel *m_gyLabel = nullptr;
    QLabel *m_gzLabel = nullptr;
    QLabel *m_axLabel = nullptr;
    QLabel *m_ayLabel = nullptr;
    QLabel *m_azLabel = nullptr;
    QLabel *m_tempLabel = nullptr;

    QtCharts::QChartView *m_chartView = nullptr;
    QtCharts::QSplineSeries *m_alsSeries = nullptr;
    QtCharts::QSplineSeries *m_irSeries = nullptr;
    QtCharts::QSplineSeries *m_psSeries = nullptr;
    int m_pointCount = 0;
};

#endif // IMX6ULLTEST_H
