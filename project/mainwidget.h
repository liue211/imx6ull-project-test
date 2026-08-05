#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class QHBoxLayout;
class QListWidget;
class QStackedWidget;

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget() override;

private:
    void buildUi();
    void registerPages();

    QListWidget *m_listWidget = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    QHBoxLayout *m_layout = nullptr;
};

#endif // MAINWIDGET_H
