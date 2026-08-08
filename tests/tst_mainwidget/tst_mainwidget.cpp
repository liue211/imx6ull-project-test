#include <QtTest/QtTest>

#include <QListWidget>
#include <QPixmap>
#include <QStackedWidget>

#include "../../project/mainwidget.h"

/* 主框架验收:7 个真实模块页面 + 列表/堆栈联动 */
class TestMainWidget : public QObject
{
    Q_OBJECT

private slots:
    void menuHasExpectedPages();
    void switchingRowChangesPage();
    void pagesAreRealModules();
    void mediaIconsExistInResources();
};

void TestMainWidget::menuHasExpectedPages()
{
    MainWidget w;

    auto *list = w.findChild<QListWidget *>(QStringLiteral("listWidget"));
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));

    QVERIFY(list != nullptr);
    QVERIFY(stack != nullptr);
    QCOMPARE(list->count(), 7);
    QCOMPARE(stack->count(), 7);
    QCOMPARE(list->item(0)->text(), QStringLiteral("轮播图"));
    QCOMPARE(list->item(6)->text(), QStringLiteral("MQTT_CLIENT"));
}

void TestMainWidget::switchingRowChangesPage()
{
    MainWidget w;

    auto *list = w.findChild<QListWidget *>(QStringLiteral("listWidget"));
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QVERIFY(list != nullptr);
    QVERIFY(stack != nullptr);

    QCOMPARE(stack->currentIndex(), 0);
    list->setCurrentRow(3);
    QCOMPARE(stack->currentIndex(), 3);
    list->setCurrentRow(6);
    QCOMPARE(stack->currentIndex(), 6);
}

void TestMainWidget::pagesAreRealModules()
{
    MainWidget w;

    auto *list = w.findChild<QListWidget *>(QStringLiteral("listWidget"));
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QVERIFY(list != nullptr);
    QVERIFY(stack != nullptr);

    /* 每个模块都应是真实页面,而不是占位页 */
    for (int i = 0; i < stack->count(); ++i) {
        QWidget *page = stack->widget(i);
        QVERIFY(page != nullptr);
        QVERIFY(QString::fromLatin1(page->metaObject()->className())
                    != QStringLiteral("PlaceholderPage"));
    }
}

void TestMainWidget::mediaIconsExistInResources()
{
    const QStringList paths = {
        QStringLiteral(":/images/icons/btn_play.png"),
        QStringLiteral(":/images/icons/btn_pause.png"),
        QStringLiteral(":/images/icons/btn_prev.png"),
        QStringLiteral(":/images/icons/btn_next.png"),
        QStringLiteral(":/images/icons/btn_volume.png"),
        QStringLiteral(":/images/icons/btn_volume_up.png"),
        QStringLiteral(":/images/icons/btn_volume_down.png"),
        QStringLiteral(":/images/icons/btn_fullscreen.png"),
        QStringLiteral(":/images/icons/btn_screen.png"),
        QStringLiteral(":/images/icons/arrow_left.png"),
        QStringLiteral(":/images/icons/arrow_right.png"),
        QStringLiteral(":/images/icons/btn_send.png"),
    };
    for (const QString &path : paths)
        QVERIFY2(!QPixmap(path).isNull(), qPrintable(path));
}

QTEST_MAIN(TestMainWidget)

#include "tst_mainwidget.moc"
