#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class QLabel;
class QListWidget;
class QStackedWidget;
class QTimer;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

private:
    void buildUi();
    void registerPages();
    void updateClock();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QTimer *m_clockTimer = nullptr;
    QListWidget *m_listWidget = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
};

#endif // MAINWIDGET_H
