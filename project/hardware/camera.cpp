#include "camera.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#include <QDebug>

Camera::Camera(QObject *parent)
    : QObject(parent)
    , m_capture(new cv::VideoCapture())
{
    m_timer.setInterval(33); /* 1000/33 ≈ 30fps */
    connect(&m_timer, &QTimer::timeout, this, &Camera::grabFrame);
}

Camera::~Camera()
{
    m_timer.stop();
    delete m_capture;
}

bool Camera::isOpened() const
{
    return m_capture->isOpened();
}

void Camera::selectDevice(int index)
{
    m_deviceIndex = index;
    if (m_capture->isOpened())
        m_capture->release();
    m_capture->open(m_deviceIndex);
}

bool Camera::start()
{
    if (!m_capture->isOpened())
        m_capture->open(m_deviceIndex);
    if (m_capture->isOpened()) {
        m_timer.start();
        return true;
    }
    return false;
}

void Camera::stop()
{
    m_timer.stop();
    m_capture->release();
}

void Camera::grabFrame()
{
    if (!m_capture->isOpened()) {
        m_timer.stop();
        return;
    }
    cv::Mat frame;
    *m_capture >> frame;
    if (!frame.empty())
        emit readyImage(matToQImage(frame));
}

QImage Camera::matToQImage(const cv::Mat &mat)
{
    if (mat.type() == CV_8UC3) {
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step,
                      QImage::Format_RGB888).copy();
    }
    if (mat.type() == CV_8UC1) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step,
                      QImage::Format_Grayscale8).copy();
    }
    if (mat.type() == CV_8UC4) {
        return QImage(mat.data, mat.cols, mat.rows, mat.step,
                      QImage::Format_ARGB32).copy();
    }
    return QImage();
}
