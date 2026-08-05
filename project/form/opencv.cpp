#include "opencv.h"

#include "camera.h"

#include <QDir>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTimer>

#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#include <cmath>

opencv::opencv(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    loadSampleImage();
}

opencv::~opencv()
{
    stopCapture();
}

void opencv::setupUi()
{
    auto *root = new QHBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);

    /* 左侧 2x2 图像区 */
    auto *imagePanel = new QWidget(this);
    auto *grid = new QGridLayout(imagePanel);
    grid->setContentsMargins(0, 0, 0, 0);

    const QString labelStyle =
        QStringLiteral("background:#3c3c3c; color:white; border-radius:8px;");

    m_srcLabel = new QLabel(QStringLiteral("src"), imagePanel);
    m_resultLabel = new QLabel(QStringLiteral("结果"), imagePanel);
    m_extraLabel1 = new QLabel(QStringLiteral("通道2"), imagePanel);
    m_extraLabel2 = new QLabel(QStringLiteral("通道3"), imagePanel);
    for (QLabel *label : {m_srcLabel, m_resultLabel, m_extraLabel1, m_extraLabel2}) {
        label->setMinimumSize(300, 220);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(labelStyle);
        label->setScaledContents(true);
    }

    auto *openButton = new QPushButton(QStringLiteral("选择图像"), imagePanel);
    connect(openButton, &QPushButton::clicked, this, &opencv::selectImage);

    grid->addWidget(m_srcLabel, 0, 0);
    grid->addWidget(m_resultLabel, 0, 1);
    grid->addWidget(m_extraLabel1, 1, 0);
    grid->addWidget(m_extraLabel2, 1, 1);
    grid->addWidget(openButton, 2, 0, 1, 2, Qt::AlignHCenter);

    root->addWidget(imagePanel, 1);

    /* 右侧按钮组(可滚动) */
    auto *scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    auto *panel = new QWidget(scroll);
    auto *panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(0, 0, 0, 0);

    using Slot = void (opencv::*)();
    auto group = [this, panelLayout](const QString &title,
                                     const QList<QPair<QString, Slot>> &buttons) {
        auto *box = new QGroupBox(title, panelLayout->parentWidget());
        auto *boxLayout = new QGridLayout(box);
        for (int i = 0; i < buttons.size(); ++i) {
            auto *btn = new QPushButton(buttons.at(i).first, box);
            connect(btn, &QPushButton::clicked, this, buttons.at(i).second);
            boxLayout->addWidget(btn, i / 2, i % 2);
        }
        panelLayout->addWidget(box);
    };

    group(QStringLiteral("图像预处理"), {
        { QStringLiteral("灰度处理"), &opencv::grayProcess },
        { QStringLiteral("灰度直方图"), &opencv::grayHistogram },
        { QStringLiteral("灰度均衡"), &opencv::grayBalance },
        { QStringLiteral("梯度锐化"), &opencv::gradSharpen },
        { QStringLiteral("Laplace锐化"), &opencv::laplaceSharpen },
    });
    group(QStringLiteral("噪声"), {
        { QStringLiteral("椒盐噪声"), &opencv::saltNoise },
        { QStringLiteral("高斯噪声"), &opencv::gaussianNoise },
    });
    group(QStringLiteral("滤波"), {
        { QStringLiteral("均值滤波"), &opencv::averageFilter },
        { QStringLiteral("中值滤波"), &opencv::medianFilter },
        { QStringLiteral("高斯滤波"), &opencv::gaussianFilter },
        { QStringLiteral("形态学滤波"), &opencv::morphologyFilter },
        { QStringLiteral("边窗滤波"), &opencv::edgePreservingFilter },
    });
    group(QStringLiteral("边缘检测"), {
        { QStringLiteral("Roberts"), &opencv::robertsEdge },
        { QStringLiteral("Sobel"), &opencv::sobelEdge },
        { QStringLiteral("Prewitt"), &opencv::prewittEdge },
        { QStringLiteral("Laplace"), &opencv::laplaceEdge },
        { QStringLiteral("Canny"), &opencv::cannyEdge },
        { QStringLiteral("Kirsch"), &opencv::kirschEdge },
    });
    group(QStringLiteral("背景处理"), {
        { QStringLiteral("阈值分割"), &opencv::thresholdSeg },
        { QStringLiteral("OSTU"), &opencv::otsuSeg },
        { QStringLiteral("Kittler"), &opencv::kittlerSeg },
        { QStringLiteral("帧间差分"), &opencv::frameDiff },
        { QStringLiteral("高斯混合背景"), &opencv::mixGaussian },
    });
    group(QStringLiteral("图像变换"), {
        { QStringLiteral("仿射变换"), &opencv::affineTransform },
        { QStringLiteral("透视变换"), &opencv::perspectiveTransform },
    });
    group(QStringLiteral("特征明显"), {
        { QStringLiteral("LBP"), &opencv::lbp },
        { QStringLiteral("模板匹配"), &opencv::templateMatch },
        { QStringLiteral("颜色匹配"), &opencv::colorMatch },
        { QStringLiteral("Gabor滤波"), &opencv::gaborFilter },
    });
    group(QStringLiteral("特征提取"), {
        { QStringLiteral("ORB"), &opencv::orbFeatures },
        { QStringLiteral("Haar-水平"), &opencv::haarHorizontal },
        { QStringLiteral("Haar-竖直"), &opencv::haarVertical },
        { QStringLiteral("坐标点SVM"), &opencv::svmTest },
        { QStringLiteral("字符测试"), &opencv::wordTest },
    });
    group(QStringLiteral("摄像标定"), {
        { QStringLiteral("立体匹配"), &opencv::stereoMatch },
        { QStringLiteral("摄像机标定"), &opencv::cameraCalibration },
    });

    scroll->setWidget(panel);
    root->addWidget(scroll, 0);
}

void opencv::loadSampleImage()
{
    /* 优先加载随工程提供的示例图 */
    QDir sampleDir(QCoreApplication::applicationDirPath() + QStringLiteral("/opencv_src"));
    QStringList candidates;
    candidates << sampleDir.filePath(QStringLiteral("lena.jpg"))
               << sampleDir.filePath(QStringLiteral("1.1.jpg"))
               << QStringLiteral(":/images/banna_pic/banna/1.jpg");
    for (const QString &candidate : candidates) {
        m_src = cv::imread(candidate.toLocal8Bit().constData());
        if (!m_src.empty()) {
            showResult(m_src);
            return;
        }
    }
}

void opencv::selectImage()
{
    const QString file = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择图像"),
        QCoreApplication::applicationDirPath(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp);;All (*)"));
    if (file.isEmpty())
        return;
    m_src = cv::imread(file.toLocal8Bit().constData());
    if (!m_src.empty())
        showResult(m_src);
}

void opencv::showResult(const cv::Mat &mat)
{
    m_resultLabel->setPixmap(
        QPixmap::fromImage(Camera::matToQImage(mat)).scaled(
            m_resultLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void opencv::showExtra(const cv::Mat &mat)
{
    m_extraLabel1->setPixmap(
        QPixmap::fromImage(Camera::matToQImage(mat)).scaled(
            m_extraLabel1->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

static bool requireImage(const cv::Mat &src)
{
    return !src.empty();
}

void opencv::grayProcess()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray;
    cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    showResult(gray);
}

void opencv::grayHistogram()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;

    cv::Mat hist;
    cv::calcHist(std::vector<cv::Mat>{gray}, std::vector<int>{0}, cv::Mat(),
                 hist, std::vector<int>{256}, std::vector<float>{0.0f, 256.0f});
    cv::normalize(hist, hist, 0, 255, cv::NORM_MINMAX);

    cv::Mat canvas(220, 300, CV_8UC3, cv::Scalar(30, 30, 30));
    for (int i = 1; i < 256; ++i) {
        const int x1 = (i - 1) * canvas.cols / 256;
        const int x2 = i * canvas.cols / 256;
        const int y1 = canvas.rows - cvRound(hist.at<float>(i - 1));
        const int y2 = canvas.rows - cvRound(hist.at<float>(i));
        cv::line(canvas, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 1);
    }
    showResult(canvas);
}

void opencv::grayBalance()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, equalized;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::equalizeHist(gray, equalized);
    showResult(equalized);
}

void opencv::gradSharpen()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, grad, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Mat kernel = (cv::Mat_<float>(3, 3) <<
                      0, -1, 0,
                      -1, 4, 0,
                      0, -1, 0);
    cv::filter2D(gray, grad, CV_32F, kernel);
    cv::convertScaleAbs(grad, grad);
    cv::addWeighted(gray, 1, grad, 1, 0, result);
    showResult(result);
}

void opencv::laplaceSharpen()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, lap, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Laplacian(gray, lap, CV_16S, 3);
    cv::convertScaleAbs(lap, lap);
    cv::addWeighted(gray, 1, lap, 1, 0, result);
    showResult(result);
}

void opencv::saltNoise()
{
    if (!requireImage(m_src))
        return;
    cv::Mat noise = m_src.clone();
    const int total = std::max(100, noise.rows * noise.cols / 100);
    cv::RNG rng(12345);
    for (int i = 0; i < total; ++i) {
        const int x = rng.uniform(0, noise.cols);
        const int y = rng.uniform(0, noise.rows);
        noise.at<cv::Vec3b>(y, x) = (i % 2 == 0)
            ? cv::Vec3b(255, 255, 255) : cv::Vec3b(0, 0, 0);
    }
    showResult(noise);
}

void opencv::gaussianNoise()
{
    if (!requireImage(m_src))
        return;
    cv::Mat noise(m_src.size(), CV_32FC3);
    cv::randn(noise, cv::Scalar::all(0), cv::Scalar::all(25));
    cv::Mat result;
    m_src.convertTo(result, CV_32FC3);
    result += noise;
    result.convertTo(result, CV_8UC3);
    showResult(result);
}

void opencv::averageFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat result;
    cv::blur(m_src, result, cv::Size(5, 5));
    showResult(result);
}

void opencv::medianFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat result;
    cv::medianBlur(m_src, result, 5);
    showResult(result);
}

void opencv::gaussianFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat result;
    cv::GaussianBlur(m_src, result, cv::Size(5, 5), 0);
    showResult(result);
}

void opencv::morphologyFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, binary, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::threshold(gray, binary, 127, 255, cv::THRESH_BINARY);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(binary, result, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(result, result, cv::MORPH_CLOSE, kernel);
    showResult(result);
}

void opencv::edgePreservingFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat result;
    /* 边窗滤波(边缘保留)用双边滤波实现 */
    cv::bilateralFilter(m_src, result, 9, 50, 50);
    showResult(result);
}

static cv::Mat edgeByKernel(const cv::Mat &src, const cv::Mat_<float> &kx,
                            const cv::Mat_<float> &ky)
{
    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src;
    cv::Mat gx, gy, mag;
    cv::filter2D(gray, gx, CV_32F, kx);
    cv::filter2D(gray, gy, CV_32F, ky);
    cv::magnitude(gx, gy, mag);
    cv::convertScaleAbs(mag, mag);
    cv::threshold(mag, mag, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    return mag;
}

void opencv::robertsEdge()
{
    if (!requireImage(m_src))
        return;
    showResult(edgeByKernel(m_src,
        (cv::Mat_<float>(2, 2) << 1, 0, 0, -1),
        (cv::Mat_<float>(2, 2) << 0, 1, -1, 0)));
}

void opencv::sobelEdge()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, gx, gy, mag;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Sobel(gray, gx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, gy, CV_32F, 0, 1, 3);
    cv::magnitude(gx, gy, mag);
    cv::convertScaleAbs(mag, mag);
    cv::threshold(mag, mag, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    showResult(mag);
}

void opencv::prewittEdge()
{
    if (!requireImage(m_src))
        return;
    showResult(edgeByKernel(m_src,
        (cv::Mat_<float>(3, 3) << -1, 0, 1, -1, 0, 1, -1, 0, 1),
        (cv::Mat_<float>(3, 3) << -1, -1, -1, 0, 0, 0, 1, 1, 1)));
}

void opencv::laplaceEdge()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, lap, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Laplacian(gray, lap, CV_32F, 3);
    cv::convertScaleAbs(lap, lap);
    cv::threshold(lap, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    showResult(result);
}

void opencv::cannyEdge()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, edges;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Canny(gray, edges, 50, 150);
    showResult(edges);
}

void opencv::kirschEdge()
{
    if (!requireImage(m_src))
        return;
    /* Kirsch 八方向模板取最大值 */
    cv::Mat gray;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    const int masks[8][3][3] = {
        {{5, 5, 5}, {-3, 0, -3}, {-3, -3, -3}},
        {{-3, 5, 5}, {-3, 0, 5}, {-3, -3, -3}},
        {{-3, -3, 5}, {-3, 0, 5}, {-3, -3, 5}},
        {{-3, -3, -3}, {-3, 0, 5}, {-3, 5, 5}},
        {{-3, -3, -3}, {-3, 0, -3}, {5, 5, 5}},
        {{-3, -3, -3}, {5, 0, -3}, {5, 5, -3}},
        {{5, -3, -3}, {5, 0, -3}, {5, -3, -3}},
        {{5, 5, -3}, {5, 0, -3}, {-3, -3, -3}},
    };
    cv::Mat result = cv::Mat::zeros(gray.size(), CV_32F);
    for (const auto &m : masks) {
        cv::Mat_<float> kernel(3, 3);
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c)
                kernel(r, c) = m[r][c];
        cv::Mat response;
        cv::filter2D(gray, response, CV_32F, kernel);
        cv::max(result, response, result);
    }
    cv::convertScaleAbs(result, result);
    cv::threshold(result, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    showResult(result);
}

void opencv::thresholdSeg()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::threshold(gray, result, 127, 255, cv::THRESH_BINARY);
    showResult(result);
}

void opencv::otsuSeg()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray, result;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::threshold(gray, result, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    showResult(result);
}

void opencv::kittlerSeg()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;

    /* Kittler-Illingworth 最小误差阈值法 */
    cv::Mat hist;
    cv::calcHist(std::vector<cv::Mat>{gray}, std::vector<int>{0}, cv::Mat(),
                 hist, std::vector<int>{256}, std::vector<float>{0.0f, 256.0f});
    double bestJ = 1e18;
    int bestT = 127;
    for (int t = 1; t < 255; ++t) {
        double w0 = 0, w1 = 0, m0 = 0, m1 = 0;
        for (int i = 0; i <= t; ++i) { w0 += hist.at<float>(i); m0 += i * hist.at<float>(i); }
        for (int i = t + 1; i < 256; ++i) { w1 += hist.at<float>(i); m1 += i * hist.at<float>(i); }
        if (w0 < 1 || w1 < 1)
            continue;
        m0 /= w0;
        m1 /= w1;
        double s0 = 0, s1 = 0;
        for (int i = 0; i <= t; ++i) s0 += hist.at<float>(i) * (i - m0) * (i - m0);
        for (int i = t + 1; i < 256; ++i) s1 += hist.at<float>(i) * (i - m1) * (i - m1);
        s0 = std::sqrt(s0 / w0 + 1e-6);
        s1 = std::sqrt(s1 / w1 + 1e-6);
        const double j = 1 + 2 * (w0 * std::log(s0) + w1 * std::log(s1))
                           - 2 * (w0 * std::log(w0) + w1 * std::log(w1));
        if (j < bestJ) {
            bestJ = j;
            bestT = t;
        }
    }
    cv::Mat result;
    cv::threshold(gray, result, bestT, 255, cv::THRESH_BINARY);
    showResult(result);
}

void opencv::stopCapture()
{
    if (m_captureTimer) {
        m_captureTimer->stop();
        m_captureTimer->deleteLater();
        m_captureTimer = nullptr;
    }
    if (m_capture) {
        m_capture->release();
        delete m_capture;
        m_capture = nullptr;
    }
    m_subtractor.release();
    m_captureDiffMode = false;
}

void opencv::frameDiff()
{
    if (m_capture) {
        stopCapture();
        return;
    }
    QMessageBox::information(this, QStringLiteral("帧间差分"),
        QStringLiteral("选择一段视频或使用摄像头 0:\n再点一次按钮停止。"));
    m_capture = new cv::VideoCapture();
    const QString video = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择视频"), QCoreApplication::applicationDirPath(),
        QStringLiteral("Videos (*.mp4 *.avi *.mkv);;All (*)"));
    if (!video.isEmpty())
        m_capture->open(video.toLocal8Bit().constData());
    else
        m_capture->open(0);
    if (!m_capture->isOpened()) {
        delete m_capture;
        m_capture = nullptr;
        QMessageBox::warning(this, QStringLiteral("帧间差分"),
                             QStringLiteral("无法打开摄像头/视频。"));
        return;
    }
    m_captureDiffMode = true;
    m_captureTimer = new QTimer(this);
    connect(m_captureTimer, &QTimer::timeout, this, [this]() {
        cv::Mat frame, gray, diff, result;
        *m_capture >> frame;
        if (frame.empty())
            return;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        if (!m_prevGray.empty()) {
            cv::absdiff(gray, m_prevGray, diff);
            cv::threshold(diff, result, 30, 255, cv::THRESH_BINARY);
            showResult(result);
        }
        m_prevGray = gray.clone();
    });
    m_captureTimer->start(66);
}

void opencv::mixGaussian()
{
    if (m_capture) {
        stopCapture();
        return;
    }
    QMessageBox::information(this, QStringLiteral("高斯混合背景"),
        QStringLiteral("基于摄像头/视频的实时前景分割,再点一次按钮停止。"));
    m_capture = new cv::VideoCapture();
    const QString video = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择视频"), QCoreApplication::applicationDirPath(),
        QStringLiteral("Videos (*.mp4 *.avi *.mkv);;All (*)"));
    if (!video.isEmpty())
        m_capture->open(video.toLocal8Bit().constData());
    else
        m_capture->open(0);
    if (!m_capture->isOpened()) {
        delete m_capture;
        m_capture = nullptr;
        QMessageBox::warning(this, QStringLiteral("高斯混合背景"),
                             QStringLiteral("无法打开摄像头/视频。"));
        return;
    }
    m_subtractor = cv::createBackgroundSubtractorMOG2(500, 16, false);
    m_captureTimer = new QTimer(this);
    connect(m_captureTimer, &QTimer::timeout, this, [this]() {
        cv::Mat frame, fg;
        *m_capture >> frame;
        if (frame.empty())
            return;
        m_subtractor->apply(frame, fg);
        cv::medianBlur(fg, fg, 3);
        showResult(fg);
        showExtra(frame);
    });
    m_captureTimer->start(66);
}

void opencv::affineTransform()
{
    if (!requireImage(m_src))
        return;
    cv::Point2f srcTri[3] = {
        cv::Point2f(0, 0), cv::Point2f(m_src.cols - 1, 0),
        cv::Point2f(0, m_src.rows - 1)};
    cv::Point2f dstTri[3] = {
        cv::Point2f(m_src.cols * 0.05f, m_src.rows * 0.15f),
        cv::Point2f(m_src.cols * 0.85f, m_src.rows * 0.10f),
        cv::Point2f(m_src.cols * 0.15f, m_src.rows * 0.85f)};
    cv::Mat warp = cv::getAffineTransform(srcTri, dstTri);
    cv::Mat result;
    cv::warpAffine(m_src, result, warp, m_src.size());
    showResult(result);
}

void opencv::perspectiveTransform()
{
    if (!requireImage(m_src))
        return;
    cv::Point2f srcQuad[4] = {
        cv::Point2f(0, 0), cv::Point2f(m_src.cols - 1, 0),
        cv::Point2f(m_src.cols - 1, m_src.rows - 1), cv::Point2f(0, m_src.rows - 1)};
    cv::Point2f dstQuad[4] = {
        cv::Point2f(m_src.cols * 0.15f, m_src.rows * 0.05f),
        cv::Point2f(m_src.cols * 0.85f, m_src.rows * 0.20f),
        cv::Point2f(m_src.cols * 0.75f, m_src.rows * 0.85f),
        cv::Point2f(m_src.cols * 0.25f, m_src.rows * 0.95f)};
    cv::Mat warp = cv::getPerspectiveTransform(srcQuad, dstQuad);
    cv::Mat result;
    cv::warpPerspective(m_src, result, warp, m_src.size());
    showResult(result);
}

void opencv::lbp()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    cv::Mat lbp = cv::Mat::zeros(gray.size(), CV_8UC1);
    for (int y = 1; y < gray.rows - 1; ++y) {
        for (int x = 1; x < gray.cols - 1; ++x) {
            uchar code = 0;
            const uchar c = gray.at<uchar>(y, x);
            code |= (gray.at<uchar>(y - 1, x - 1) >= c) << 7;
            code |= (gray.at<uchar>(y - 1, x) >= c) << 6;
            code |= (gray.at<uchar>(y - 1, x + 1) >= c) << 5;
            code |= (gray.at<uchar>(y, x + 1) >= c) << 4;
            code |= (gray.at<uchar>(y + 1, x + 1) >= c) << 3;
            code |= (gray.at<uchar>(y + 1, x) >= c) << 2;
            code |= (gray.at<uchar>(y + 1, x - 1) >= c) << 1;
            code |= (gray.at<uchar>(y, x - 1) >= c);
            lbp.at<uchar>(y, x) = code;
        }
    }
    showResult(lbp);
}

void opencv::templateMatch()
{
    if (!requireImage(m_src))
        return;
    const QString tpl = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择模板图像"),
        QCoreApplication::applicationDirPath(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp);;All (*)"));
    if (tpl.isEmpty())
        return;
    cv::Mat templ = cv::imread(tpl.toLocal8Bit().constData());
    if (templ.empty() || templ.cols > m_src.cols || templ.rows > m_src.rows)
        return;
    cv::Mat result;
    cv::matchTemplate(m_src, templ, result, cv::TM_CCOEFF_NORMED);
    double minVal = 0, maxVal = 0;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    cv::Mat display = m_src.clone();
    cv::rectangle(display, maxLoc,
                  cv::Point(maxLoc.x + templ.cols, maxLoc.y + templ.rows),
                  cv::Scalar(0, 255, 0), 2);
    showResult(display);
    showExtra(templ);
}

void opencv::colorMatch()
{
    if (!requireImage(m_src))
        return;
    const QString other = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择待匹配图像"),
        QCoreApplication::applicationDirPath(),
        QStringLiteral("Images (*.png *.jpg *.jpeg *.bmp);;All (*)"));
    if (other.isEmpty())
        return;
    cv::Mat srcHsv, otherHsv;
    cv::cvtColor(m_src, srcHsv, cv::COLOR_BGR2HSV);
    cv::Mat otherImg = cv::imread(other.toLocal8Bit().constData());
    if (otherImg.empty())
        return;
    cv::cvtColor(otherImg, otherHsv, cv::COLOR_BGR2HSV);

    cv::Mat h1, h2;
    cv::calcHist(std::vector<cv::Mat>{srcHsv}, std::vector<int>{0, 1},
                 cv::Mat(), h1, std::vector<int>{30, 32},
                 std::vector<float>{0.0f, 180.0f, 0.0f, 256.0f});
    cv::calcHist(std::vector<cv::Mat>{otherHsv}, std::vector<int>{0, 1},
                 cv::Mat(), h2, std::vector<int>{30, 32},
                 std::vector<float>{0.0f, 180.0f, 0.0f, 256.0f});
    cv::normalize(h1, h1, 1, 0, cv::NORM_L1);
    cv::normalize(h2, h2, 1, 0, cv::NORM_L1);
    const double similarity = cv::compareHist(h1, h2, cv::HISTCMP_CORREL);
    QMessageBox::information(this, QStringLiteral("颜色匹配"),
        QStringLiteral("颜色直方图相似度(CORREL):%1").arg(similarity, 0, 'f', 3));
}

void opencv::gaborFilter()
{
    if (!requireImage(m_src))
        return;
    cv::Mat kernel = cv::getGaborKernel(
        cv::Size(21, 21), 4.0, CV_PI / 4, 10.0, 0.5, 0, CV_32F);
    cv::Mat result;
    cv::filter2D(m_src, result, CV_32F, kernel);
    cv::convertScaleAbs(result, result);
    showResult(result);
}

void opencv::orbFeatures()
{
    if (!requireImage(m_src))
        return;
    cv::Mat gray;
    if (m_src.channels() == 3)
        cv::cvtColor(m_src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = m_src;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
    cv::Ptr<cv::ORB> orb = cv::ORB::create(200);
    orb->detectAndCompute(gray, cv::Mat(), keypoints, descriptors);
    cv::Mat display = m_src.clone();
    cv::drawKeypoints(display, keypoints, display, cv::Scalar(0, 255, 0));
    showResult(display);
}

static int rectSum(const cv::Mat &integral, int x1, int y1, int x2, int y2)
{
    return integral.at<int>(y2, x2) - integral.at<int>(y1, x2)
         - integral.at<int>(y2, x1) + integral.at<int>(y1, x1);
}

static cv::Mat haarFeatureImage(const cv::Mat &src, bool horizontal)
{
    cv::Mat gray;
    if (src.channels() == 3)
        cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    else
        gray = src;
    cv::Mat integral;
    cv::integral(gray, integral, CV_32S);
    cv::Mat response = cv::Mat::zeros(gray.size(), CV_32F);
    const int step = 8;
    for (int y = step; y < gray.rows - step; y += step) {
        for (int x = step; x < gray.cols - step; x += step) {
            const int w = step;
            const int h = step;
            rectSum(integral, x, y, x + w, y + h);
            float value = 0;
            if (horizontal) {
                const int half = w / 2;
                const int left = rectSum(integral, x, y, x + half, y + h);
                const int right = rectSum(integral, x + half, y, x + w, y + h);
                value = static_cast<float>(left - right);
            } else {
                const int half = h / 2;
                const int top = rectSum(integral, x, y, x + w, y + half);
                const int bottom = rectSum(integral, x, y + half, x + w, y + h);
                value = static_cast<float>(top - bottom);
            }
            response.at<float>(y + h / 2, x + w / 2) = value;
        }
    }
    cv::normalize(response, response, 0, 255, cv::NORM_MINMAX);
    cv::Mat result;
    response.convertTo(result, CV_8UC1);
    return result;
}

void opencv::haarHorizontal()
{
    if (!requireImage(m_src))
        return;
    showResult(haarFeatureImage(m_src, true));
}

void opencv::haarVertical()
{
    if (!requireImage(m_src))
        return;
    showResult(haarFeatureImage(m_src, false));
}

void opencv::svmTest()
{
    QMessageBox::information(this, QStringLiteral("坐标点SVM"),
        QStringLiteral("该演示需要训练好的 SVM_HOG.xml 模型与样本,本机未集成,跳过。"));
}

void opencv::wordTest()
{
    QMessageBox::information(this, QStringLiteral("字符测试"),
        QStringLiteral("该演示需要字符样本库与训练流程,本机未集成,跳过。"));
}

void opencv::stereoMatch()
{
    const QString dir = QCoreApplication::applicationDirPath()
                        + QStringLiteral("/opencv_src/camer_cab");
    cv::Mat left = cv::imread((dir + QStringLiteral("/left01.jpg")).toLocal8Bit().constData(),
                              cv::IMREAD_GRAYSCALE);
    cv::Mat right = cv::imread((dir + QStringLiteral("/right01.jpg")).toLocal8Bit().constData(),
                               cv::IMREAD_GRAYSCALE);
    if (left.empty() || right.empty()) {
        QMessageBox::warning(this, QStringLiteral("立体匹配"),
                             QStringLiteral("缺少左右视图素材(left01/right01.jpg)。"));
        return;
    }
    cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(64, 15);
    cv::Mat disparity;
    bm->compute(left, right, disparity);
    cv::normalize(disparity, disparity, 0, 255, cv::NORM_MINMAX);
    cv::Mat result;
    disparity.convertTo(result, CV_8UC1);
    showResult(result);
    showExtra(left);
}

void opencv::cameraCalibration()
{
    QMessageBox::information(this, QStringLiteral("摄像机标定"),
        QStringLiteral("该演示需要棋盘格标定流程与结果文件,本机未集成,跳过。"));
}
