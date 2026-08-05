#ifndef FACEDETECT_H
#define FACEDETECT_H

#include <QWidget>

class Camera;
class QComboBox;
class QImage;
class QLabel;
class QPushButton;

namespace cv {
class CascadeClassifier;
}

/* 摄像头人脸识别:采集预览 + Haar 人脸框 + 拍照保存 */
class faceDetect : public QWidget
{
    Q_OBJECT

public:
    explicit faceDetect(QWidget *parent = nullptr);
    ~faceDetect() override;

private slots:
    void onStartClicked(bool checked);
    void onTakePhotoClicked();
    void onDetectClicked(bool checked);

private:
    void setupUi();
    void scanDevices();
    void showFrame(const QImage &image);
    QImage detectFaces(const QImage &image);
    void loadCascade();

    Camera *m_camera = nullptr;
    QLabel *m_display = nullptr;
    QComboBox *m_deviceBox = nullptr;
    QPushButton *m_startButton = nullptr;
    QPushButton *m_takePhotoButton = nullptr;
    QPushButton *m_detectButton = nullptr;

    QImage m_lastFrame;
    bool m_detectEnabled = false;
    cv::CascadeClassifier *m_cascade = nullptr;
};

#endif // FACEDETECT_H
