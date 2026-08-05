#ifndef BANNA_H
#define BANNA_H

#include <QWidget>
#include <QStringList>

class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QTimer;

/* 轮播图:定时自动切换 + 左右按钮 + 底部圆点指示,带滑动动画 */
class banna : public QWidget
{
    Q_OBJECT

public:
    explicit banna(QWidget *parent = nullptr);

private slots:
    void showNext();
    void showPrev();
    void jumpTo(int index);

private:
    void setupUi();
    void showImage(int index);
    void updateDots();

    QLabel *m_imageLabel = nullptr;
    QPushButton *m_leftButton = nullptr;
    QPushButton *m_rightButton = nullptr;
    QHBoxLayout *m_dotLayout = nullptr;
    QButtonGroup *m_dotGroup = nullptr;
    QTimer *m_timer = nullptr;
    QPropertyAnimation *m_anim = nullptr;

    QStringList m_imagePaths;
    int m_current = 0;
};

#endif // BANNA_H
