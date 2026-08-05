#ifndef OPENCV_H
#define OPENCV_H

#include <QWidget>

#include <QList>
#include <QPair>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class QLabel;
class QPushButton;

namespace cv {
class VideoCapture;
class BackgroundSubtractor;
}

/* OpenCV 图像处理工具集:4 个图像区 + 9 组功能按钮 */
class opencv : public QWidget
{
    Q_OBJECT

public:
    explicit opencv(QWidget *parent = nullptr);
    ~opencv() override;

private slots:
    void selectImage();

    /* 图像预处理 */
    void grayProcess();
    void grayHistogram();
    void grayBalance();
    void gradSharpen();
    void laplaceSharpen();

    /* 噪声 */
    void saltNoise();
    void gaussianNoise();

    /* 滤波 */
    void averageFilter();
    void medianFilter();
    void gaussianFilter();
    void morphologyFilter();
    void edgePreservingFilter();

    /* 边缘检测 */
    void robertsEdge();
    void sobelEdge();
    void prewittEdge();
    void laplaceEdge();
    void cannyEdge();
    void kirschEdge();

    /* 背景处理 */
    void thresholdSeg();
    void otsuSeg();
    void kittlerSeg();
    void frameDiff();
    void mixGaussian();

    /* 图像变换 */
    void affineTransform();
    void perspectiveTransform();

    /* 特征明显 */
    void lbp();
    void templateMatch();
    void colorMatch();
    void gaborFilter();

    /* 特征提取 */
    void orbFeatures();
    void haarHorizontal();
    void haarVertical();
    void svmTest();
    void wordTest();

    /* 摄像标定 */
    void stereoMatch();
    void cameraCalibration();

private:
    void setupUi();
    QWidget *buildGroup(const QString &title, const QList<QPair<QString, const char *> > &buttons);
    void loadSampleImage();
    void showResult(const cv::Mat &mat);
    void showExtra(const cv::Mat &mat);
    void stopCapture();

    QLabel *m_srcLabel = nullptr;
    QLabel *m_resultLabel = nullptr;
    QLabel *m_extraLabel1 = nullptr;
    QLabel *m_extraLabel2 = nullptr;

    cv::Mat m_src;
    cv::Mat m_prevGray;
    cv::VideoCapture *m_capture = nullptr;
    cv::Ptr<cv::BackgroundSubtractor> m_subtractor;
    QTimer *m_captureTimer = nullptr;
    bool m_captureDiffMode = false;
};

#endif // OPENCV_H
