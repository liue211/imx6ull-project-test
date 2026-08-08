#include "facedetect.h"

#include "camera.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFile>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include <QDebug>

faceDetect::faceDetect(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    scanDevices();
    loadCascade();
}

faceDetect::~faceDetect()
{
    delete m_cascade;
}

void faceDetect::setupUi()
{
    m_camera = new Camera(this);
    connect(m_camera, &Camera::readyImage, this, &faceDetect::showFrame);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_display = new QLabel(QStringLiteral("摄像头画面"), this);
    m_display->setObjectName(QStringLiteral("videoPanel"));
    m_display->setAlignment(Qt::AlignCenter);
    m_display->setMinimumSize(480, 380);
    m_display->setScaledContents(true);

    auto *controlCard = new QFrame(this);
    controlCard->setObjectName(QStringLiteral("card"));
    auto *control = new QHBoxLayout(controlCard);
    control->setContentsMargins(12, 12, 12, 12);
    m_deviceBox = new QComboBox(controlCard);
    m_startButton = new QPushButton(QStringLiteral("开始"), controlCard);
    m_takePhotoButton = new QPushButton(QStringLiteral("拍照"), controlCard);
    m_detectButton = new QPushButton(QStringLiteral("人脸检测"), controlCard);
    m_startButton->setObjectName(QStringLiteral("btn_start"));

    m_startButton->setCheckable(true);
    m_detectButton->setCheckable(true);
    m_takePhotoButton->setEnabled(false);

    control->addWidget(new QLabel(QStringLiteral("设备:"), controlCard));
    control->addWidget(m_deviceBox, 1);
    control->addSpacing(12);
    control->addWidget(m_startButton);
    control->addWidget(m_takePhotoButton);
    control->addWidget(m_detectButton);

    layout->addWidget(m_display, 1);
    layout->addWidget(controlCard);

    connect(m_startButton, &QPushButton::clicked, this, &faceDetect::onStartClicked);
    connect(m_takePhotoButton, &QPushButton::clicked, this, &faceDetect::onTakePhotoClicked);
    connect(m_detectButton, &QPushButton::clicked, this, &faceDetect::onDetectClicked);
}

void faceDetect::scanDevices()
{
    /* Linux 板端常见 /dev/video0/1/2;Windows 用摄像头 0 */
    bool found = false;
    for (int i = 0; i <= 2; ++i) {
        if (QFile::exists(QStringLiteral("/dev/video%1").arg(i))) {
            m_deviceBox->addItem(QStringLiteral("video%1").arg(i), i);
            found = true;
        }
    }
    if (!found) {
        m_deviceBox->addItem(QStringLiteral("默认摄像头 0"), 0);
#ifdef Q_OS_WIN
        m_deviceBox->setCurrentIndex(0);
#endif
    }
}

void faceDetect::loadCascade()
{
    QStringList candidates;
    candidates << QCoreApplication::applicationDirPath()
                  + QStringLiteral("/opencv_src/face-haar/haarcascade_frontalface_alt2.xml")
               << QCoreApplication::applicationDirPath()
                  + QStringLiteral("/../project/opencv_src/face-haar/haarcascade_frontalface_alt2.xml")
               << QStringLiteral("opencv_src/face-haar/haarcascade_frontalface_alt2.xml");

    m_cascade = new cv::CascadeClassifier();
    for (const QString &path : candidates) {
        if (QFile::exists(path) && m_cascade->load(path.toLocal8Bit().constData()))
            return;
    }
    qWarning() << "Haar cascade not found, face detection disabled";
}

void faceDetect::onStartClicked(bool checked)
{
    if (checked) {
        m_camera->selectDevice(m_deviceBox->currentData().toInt());
        if (m_camera->start()) {
            m_startButton->setText(QStringLiteral("关闭"));
            m_takePhotoButton->setEnabled(true);
        } else {
            m_startButton->setChecked(false);
            QMessageBox::warning(this, QStringLiteral("摄像头"),
                                 QStringLiteral("无法打开摄像头设备。"));
        }
    } else {
        m_camera->stop();
        m_startButton->setText(QStringLiteral("开始"));
        m_takePhotoButton->setEnabled(false);
        m_detectButton->setChecked(false);
        m_detectEnabled = false;
    }
}

void faceDetect::onTakePhotoClicked()
{
    if (m_lastFrame.isNull())
        return;
    const QString file = QCoreApplication::applicationDirPath()
                         + QStringLiteral("/test.png");
    if (m_lastFrame.save(file, "PNG"))
        QMessageBox::information(this, QStringLiteral("拍照"),
                                 QStringLiteral("已保存到 %1").arg(file));
}

void faceDetect::onDetectClicked(bool checked)
{
    m_detectEnabled = checked;
    if (checked && m_cascade->empty())
        QMessageBox::warning(this, QStringLiteral("人脸检测"),
                             QStringLiteral("缺少 Haar 级联模型文件,无法检测。"));
}

QImage faceDetect::detectFaces(const QImage &image)
{
    if (!m_detectEnabled || m_cascade->empty())
        return image;

    cv::Mat rgb(image.height(), image.width(), CV_8UC3,
                const_cast<uchar *>(image.constBits()), image.bytesPerLine());
    cv::Mat bgr;
    cv::cvtColor(rgb, bgr, cv::COLOR_RGB2BGR);
    cv::Mat gray;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(gray, gray);

    std::vector<cv::Rect> faces;
    m_cascade->detectMultiScale(gray, faces, 1.1, 5, 0,
                                cv::Size(40, 40));
    for (const cv::Rect &face : faces) {
        cv::rectangle(bgr, face, cv::Scalar(0, 255, 0), 2);
        cv::putText(bgr, "Face",
                    cv::Point(face.x, face.y - 6),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 1);
    }
    return Camera::matToQImage(bgr);
}

void faceDetect::showFrame(const QImage &image)
{
    m_lastFrame = image;
    const QImage shown = detectFaces(image);
    m_display->setPixmap(QPixmap::fromImage(shown).scaled(
        m_display->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
