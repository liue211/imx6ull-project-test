#ifndef CAMERA_H
#define CAMERA_H

#include <QImage>
#include <QObject>
#include <QTimer>

namespace cv {
class VideoCapture;
class Mat;
}

/* 摄像头封装:OpenCV VideoCapture + 33ms 定时器(≈30fps),
 * 抓到图像后发 readyImage 信号。 */
class Camera : public QObject
{
    Q_OBJECT

public:
    explicit Camera(QObject *parent = nullptr);
    ~Camera() override;

    static QImage matToQImage(const cv::Mat &mat);

    bool isOpened() const;

public slots:
    bool start();               /* 启动采集,返回是否成功打开设备 */
    void stop();                /* 停止采集并释放设备 */
    void selectDevice(int index);

signals:
    void readyImage(const QImage &image);

private slots:
    void grabFrame();

private:
    cv::VideoCapture *m_capture = nullptr;
    QTimer m_timer;
    int m_deviceIndex = 0;
};

#endif // CAMERA_H
