# Qt 浅色卡片风界面重设计 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 imx6ull Qt 工程的界面从暗色左列表风格重设计为浅色现代卡片风(顶部标签导航 + 白色圆角卡片 + 浅色图标),不修改任何业务逻辑。

**Architecture:** 主框架改为“顶部标题/标签/时间 + QStackedWidget”;7 个页面保持独立类,只改 `setupUi()` 布局与控件 objectName/图标,统一由全局 QSS + 页面局部 QSS 控制外观;图标由 Python/Pillow 脚本生成 PNG 并注册进 `res.qrc`。

**Tech Stack:** Qt 5.15 Widgets/Charts/Multimedia、MSVC 2022(qmake/nmake)、Python 3 + Pillow 10.3.0(图标生成)、QtTest(回归)。

## Global Constraints

- 窗口尺寸保持 1024×600。
- 顶部标签栏使用横向 `QListWidget`,保留 `objectName = "listWidget"` 与 `currentRowChanged → setCurrentIndex` 连接,保证现有测试 API 不变。
- 颜色固定:背景 `#F4F6F8`,卡片 `#FFFFFF`,描边 `#E5E7EB`,主色 `#2F7CF6`,成功 `#22C55E`,警示 `#F59E0B`,错误 `#EF4444`,主文字 `#1F2937`,次文字 `#6B7280`,图标默认 `#374151`。
- 按钮高 ≥ 44px,触控目标 ≥ 44×44;不使用阴影、半透明、渐变等重渲染效果。
- 不修改 `hardware/`、`lib/`、OpenCV 算法、MQTT 协议与设备路径,只改 UI 表现层。
- 媒体页为全浅色,不留深色残留(用户已确认)。
- 图标统一 48×48 透明 PNG、扁平线性;播放/暂停等主控图标在播放态切换为暂停图标。
- 字体使用系统默认;标题 20px,正文 14~16px,大数字 28~36px。
- Windows 构建:VS2022 Developer 环境下 `x86/build.cmd`;测试:`tests/tests.pro` + `nmake` + `QT_QPA_PLATFORM=offscreen`。

## File Structure

| 文件 | 职责 |
|---|---|
| `scripts/generate_icons.py` | 一键生成全部浅色图标 |
| `project/images/icons/*.png` | 图标素材(生成产物,入库) |
| `project/res.qrc` | 注册图标资源 |
| `project/mainwidget.h/.cpp` | 顶部标签栏 + 时间显示框架 |
| `project/style/mainstyle.qss` | 全局浅色主题 |
| `project/style/music_style.qss` | 音频页浅色样式 |
| `project/style/video_style.qss` | 视频页浅色样式 |
| `project/form/banna.cpp` | 轮播图卡片化 |
| `project/form/opencv.cpp` | OpenCV 页卡片化 |
| `project/form/facedetect.cpp` | 摄像头页卡片化 |
| `project/form/imx6ulltest.cpp` | 板级设备页卡片化 + 曲线浅色 |
| `project/form/musicplay.cpp` | 音频页图标与浅色布局 |
| `project/form/videoplayer.cpp` | 视频页图标与浅色布局 |
| `project/form/mqtt_client.cpp` | MQTT 页卡片化 + 连接状态 |
| `tests/tst_mainwidget/tst_mainwidget.cpp` | 现有 3 个用例 + 新增 UI 断言 |

---

### Task 1: 生成浅色图标并注册到资源

**Files:**
- Create: `scripts/generate_icons.py`
- Create: `project/images/icons/*.png`(脚本生成)
- Modify: `project/res.qrc`(新增 `<file>images/icons/...</file>` 条目)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Pillow 10.3.0(`python` 命令可用,已验证)。
- Produces: 下列资源路径,供 Task 4/8/9 使用:
  `:/images/icons/btn_play.png`、`btn_pause.png`、`btn_prev.png`、`btn_next.png`、`btn_volume.png`、`btn_volume_up.png`、`btn_volume_down.png`、`btn_fullscreen.png`、`btn_screen.png`、`btn_favorite.png`、`btn_favorite_on.png`、`btn_list.png`、`btn_menu.png`、`btn_photo.png`、`arrow_left.png`、`arrow_right.png`、`dot_normal.png`、`dot_active.png`、`led_on.png`、`led_off.png`、`beep_on.png`、`beep_off.png`、`btn_connect.png`、`btn_disconnect.png`、`btn_send.png`、`btn_clear.png`、`btn_refresh.png`。

- [ ] **Step 1: 写失败测试(资源存在性)**

在 `tests/tst_mainwidget/tst_mainwidget.cpp` 的 private slots 中追加:

```cpp
void mediaIconsExistInResources();
```

实现:

```cpp
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
```

文件顶部 include 增加 `#include <QPixmap>`。

- [ ] **Step 2: 运行测试确认失败**

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd tests
qmake tests.pro
nmake
set QT_QPA_PLATFORM=offscreen
tst_mainwidget\release\tst_mainwidget.exe -o result.txt
```

Expected: `mediaIconsExistInResources` FAIL(资源不存在)。

- [ ] **Step 3: 创建图标生成脚本**

创建 `scripts/generate_icons.py`,完整内容:

```python
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成浅色卡片风界面图标(Pillow 10.x)。

用法: python scripts/generate_icons.py
输出: project/images/icons/*.png(48x48,透明背景,扁平线性风格)
"""
import os
from PIL import Image, ImageDraw

SIZE = 48
INK = (55, 65, 81, 255)        # #374151
BLUE = (47, 124, 246, 255)     # #2F7CF6
GREEN = (34, 197, 94, 255)     # #22C55E
GRAY = (209, 213, 219, 255)    # #D1D5DB
OUT_DIR = os.path.normpath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "project", "images", "icons"))


def new_img():
    return Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))


def save(img, name):
    path = os.path.join(OUT_DIR, name)
    img.save(path)
    print("生成:", os.path.abspath(path))


def polygon(d, pts, color=INK, width=4):
    d.polygon(pts, fill=color)


def line(d, pts, color=INK, width=4):
    d.line(pts, fill=color, width=width, joint="curve")


def rect(d, box, color=INK, width=4):
    d.rectangle(box, outline=color, width=width)


def fill_rect(d, box, color=INK, width=4):
    d.rectangle(box, fill=color)


def dot(d, center, r, color=INK, width=4):
    x, y = center
    d.ellipse([x - r, y - r, x + r, y + r], fill=color)


def arc(d, box, start, end, color=INK, width=4):
    d.arc(box, start, end, fill=color, width=width)


def speaker(d, x0, y0, x1, y1, color=INK, width=4):
    polygon(d, [(x0, y0 + 6), (x0 + 7, y0 + 6), (x1, y0), (x1, y1),
                (x0 + 7, y1 - 6), (x0, y1 - 6)], color)


def play():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(16, 12), (16, 36), (37, 24)])
    return img


def pause():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [13, 12, 21, 36]); fill_rect(d, [27, 12, 35, 36])
    return img


def prev():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [11, 13, 16, 35]); polygon(d, [(22, 12), (22, 36), (38, 24)])
    return img


def next_():
    img = new_img(); d = ImageDraw.Draw(img)
    fill_rect(d, [32, 13, 37, 35]); polygon(d, [(26, 12), (26, 36), (10, 24)])
    return img


def volume():
    img = new_img(); d = ImageDraw.Draw(img)
    speaker(d, 12, 14, 28, 34)
    arc(d, [22, 10, 38, 26], -60, 60)
    arc(d, [26, 4, 44, 32], -60, 60)
    return img


def volume_up():
    img = volume(); d = ImageDraw.Draw(img)
    line(d, [(40, 20), (40, 30)]); line(d, [(35, 25), (45, 25)])
    return img


def volume_down():
    img = volume(); d = ImageDraw.Draw(img)
    line(d, [(35, 25), (45, 25)])
    return img


def fullscreen():
    img = new_img(); d = ImageDraw.Draw(img)
    for x0, y0 in [(8, 8), (32, 8), (8, 32), (32, 32)]:
        line(d, [(x0, y0), (x0 + 7, y0)])
        line(d, [(x0, y0), (x0, y0 + 7)])
    return img


def screen():
    img = new_img(); d = ImageDraw.Draw(img)
    rect(d, [6, 8, 42, 34])
    line(d, [(20, 36), (28, 36)])
    line(d, [(24, 36), (24, 42)])
    line(d, [(16, 42), (32, 42)])
    return img


def favorite(color=INK):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (17, 19), 7, color)
    dot(d, (31, 19), 7, color)
    polygon(d, [(9, 23), (39, 23), (24, 41)], color)
    return img


def favorite_on():
    return favorite(BLUE)


def list_icon():
    img = new_img(); d = ImageDraw.Draw(img)
    line(d, [(10, 14), (38, 14)]); line(d, [(10, 24), (38, 24)])
    line(d, [(10, 34), (38, 34)])
    return img


def menu():
    img = new_img(); d = ImageDraw.Draw(img)
    for y in (12, 24, 36):
        dot(d, (24, y), 3)
    return img


def photo():
    img = new_img(); d = ImageDraw.Draw(img)
    rect(d, [6, 14, 42, 36])
    polygon(d, [(18, 14), (21, 8), (27, 8), (30, 14)])
    dot(d, (24, 26), 7)
    return img


def arrow_left():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(34, 10), (16, 24), (34, 38)])
    return img


def arrow_right():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(14, 10), (32, 24), (14, 38)])
    return img


def dot_icon(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 6, color)
    return img


def led(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 9, color)
    return img


def beep(color=INK):
    img = new_img(); d = ImageDraw.Draw(img)
    speaker(d, 10, 14, 24, 34)
    arc(d, [20, 10, 36, 26], -60, 60, color)
    arc(d, [24, 4, 42, 32], -60, 60, color)
    return img


def check_icon(color=GREEN):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 11, color)
    line(d, [(17, 25), (22, 30), (32, 18)], color=(255, 255, 255, 255))
    return img


def cross_icon(color=GRAY):
    img = new_img(); d = ImageDraw.Draw(img)
    dot(d, (24, 24), 11, color)
    line(d, [(18, 18), (30, 30)], color=(255, 255, 255, 255))
    line(d, [(30, 18), (18, 30)], color=(255, 255, 255, 255))
    return img


def send():
    img = new_img(); d = ImageDraw.Draw(img)
    polygon(d, [(8, 12), (40, 24), (8, 36), (16, 24)])
    return img


def clear():
    img = new_img(); d = ImageDraw.Draw(img)
    line(d, [(14, 14), (34, 34)]); line(d, [(34, 14), (14, 34)])
    return img


def refresh():
    img = new_img(); d = ImageDraw.Draw(img)
    arc(d, [10, 10, 38, 38], 20, 300)
    polygon(d, [(38, 16), (30, 12), (33, 22)])
    return img


ICONS = {
    "btn_play.png": play,
    "btn_pause.png": pause,
    "btn_prev.png": prev,
    "btn_next.png": next_,
    "btn_volume.png": volume,
    "btn_volume_up.png": volume_up,
    "btn_volume_down.png": volume_down,
    "btn_fullscreen.png": fullscreen,
    "btn_screen.png": screen,
    "btn_favorite.png": favorite,
    "btn_favorite_on.png": favorite_on,
    "btn_list.png": list_icon,
    "btn_menu.png": menu,
    "btn_photo.png": photo,
    "arrow_left.png": arrow_left,
    "arrow_right.png": arrow_right,
    "dot_normal.png": lambda: dot_icon(GRAY),
    "dot_active.png": lambda: dot_icon(BLUE),
    "led_on.png": lambda: led(GREEN),
    "led_off.png": lambda: led(GRAY),
    "beep_on.png": lambda: beep(GREEN),
    "beep_off.png": lambda: beep(GRAY),
    "btn_connect.png": lambda: check_icon(GREEN),
    "btn_disconnect.png": lambda: cross_icon(GRAY),
    "btn_send.png": send,
    "btn_clear.png": clear,
    "btn_refresh.png": refresh,
}


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, fn in ICONS.items():
        save(fn(), name)
    print("共生成 %d 个图标 -> %s" % (len(ICONS), OUT_DIR))


if __name__ == "__main__":
    main()
```

- [ ] **Step 4: 运行脚本生成图标**

```bat
python scripts\generate_icons.py
```

Expected: 27 个 PNG 生成到 `project/images/icons/`,无异常。

- [ ] **Step 5: 注册到 res.qrc**

在 `project/res.qrc` 的 `<qresource prefix="/">` 内追加:

```xml
<file>images/icons/btn_play.png</file>
<file>images/icons/btn_pause.png</file>
<file>images/icons/btn_prev.png</file>
<file>images/icons/btn_next.png</file>
<file>images/icons/btn_volume.png</file>
<file>images/icons/btn_volume_up.png</file>
<file>images/icons/btn_volume_down.png</file>
<file>images/icons/btn_fullscreen.png</file>
<file>images/icons/btn_screen.png</file>
<file>images/icons/btn_favorite.png</file>
<file>images/icons/btn_favorite_on.png</file>
<file>images/icons/btn_list.png</file>
<file>images/icons/btn_menu.png</file>
<file>images/icons/btn_photo.png</file>
<file>images/icons/arrow_left.png</file>
<file>images/icons/arrow_right.png</file>
<file>images/icons/dot_normal.png</file>
<file>images/icons/dot_active.png</file>
<file>images/icons/led_on.png</file>
<file>images/icons/led_off.png</file>
<file>images/icons/beep_on.png</file>
<file>images/icons/beep_off.png</file>
<file>images/icons/btn_connect.png</file>
<file>images/icons/btn_disconnect.png</file>
<file>images/icons/btn_send.png</file>
<file>images/icons/btn_clear.png</file>
<file>images/icons/btn_refresh.png</file>
```

- [ ] **Step 6: 重新构建并运行测试确认通过**

按 Step 2 命令重跑,Expected: `mediaIconsExistInResources` PASS,其余 3 个旧用例仍 PASS。

- [ ] **Step 7: Commit**

```bash
git add scripts/generate_icons.py project/images/icons project/res.qrc tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "feat(icons): 生成浅色 UI 图标并注册到资源"
```

---

### Task 2: 主框架改为顶部标签栏 + 时间显示

**Files:**
- Modify: `project/mainwidget.h`
- Modify: `project/mainwidget.cpp`(`buildUi()`、`registerPages()`)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 1 的资源不影响本任务。
- Produces: `listWidget`(横向)、`timeLabel`、`titleLabel`、`stackedWidget` 四个可被测试/样式引用的对象;`listWidget` 的 `currentRowChanged` 仍驱动堆栈切换。

- [ ] **Step 1: 写失败测试**

在测试类 private slots 追加:

```cpp
void topNavIsHorizontalWithClock();
```

实现:

```cpp
void TestMainWidget::topNavIsHorizontalWithClock()
{
    MainWidget w;
    auto *list = w.findChild<QListWidget *>(QStringLiteral("listWidget"));
    auto *time = w.findChild<QLabel *>(QStringLiteral("timeLabel"));
    QVERIFY(list != nullptr);
    QCOMPARE(list->flow(), QListView::LeftToRight);
    QCOMPARE(list->maximumHeight(), 64);
    QVERIFY(time != nullptr);
    QVERIFY(!time->text().isEmpty());
}
```

include 增加 `#include <QLabel>` 与 `#include <QListView>`。

- [ ] **Step 2: 运行测试确认失败**

命令同 Task 1 Step 2。Expected: `topNavIsHorizontalWithClock` FAIL(flow 不是 LeftToRight 或缺少 timeLabel)。

- [ ] **Step 3: 修改 mainwidget.h**

前置声明增加:

```cpp
class QLabel;
class QTimer;
```

成员变量改为:

```cpp
private:
    void buildUi();
    void registerPages();
    void updateClock();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_timeLabel = nullptr;
    QTimer *m_clockTimer = nullptr;
    QListWidget *m_listWidget = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
```

删除不再使用的 `QHBoxLayout *m_layout`。

- [ ] **Step 4: 重写 mainwidget.cpp 的 buildUi()**

替换 `buildUi()` 与新增 `updateClock()`:

```cpp
void MainWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto *header = new QWidget(this);
    header->setObjectName(QStringLiteral("headerBar"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(16, 0, 16, 0);
    headerLayout->setSpacing(8);

    m_titleLabel = new QLabel(QStringLiteral("imx6ull 工程"), header);
    m_titleLabel->setObjectName(QStringLiteral("titleLabel"));

    m_listWidget = new QListWidget(header);
    m_listWidget->setObjectName(QStringLiteral("listWidget"));
    m_listWidget->setFlow(QListView::LeftToRight);
    m_listWidget->setWrapping(false);
    m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_listWidget->setFixedHeight(64);

    m_timeLabel = new QLabel(QStringLiteral("--:--"), header);
    m_timeLabel->setObjectName(QStringLiteral("timeLabel"));

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addSpacing(16);
    headerLayout->addWidget(m_listWidget, 1);
    headerLayout->addWidget(m_timeLabel);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName(QStringLiteral("stackedWidget"));

    root->addWidget(header);
    root->addWidget(m_stackedWidget, 1);

    connect(m_listWidget, &QListWidget::currentRowChanged,
            m_stackedWidget, &QStackedWidget::setCurrentIndex);

    m_clockTimer = new QTimer(this);
    m_clockTimer->setInterval(1000);
    connect(m_clockTimer, &QTimer::timeout, this, &MainWidget::updateClock);
    m_clockTimer->start();
    updateClock();
}

void MainWidget::updateClock()
{
    m_timeLabel->setText(QTime::currentTime().toString(QStringLiteral("HH:mm")));
}
```

include 增加:

```cpp
#include <QLabel>
#include <QListView>
#include <QTimer>
#include <QTime>
#include <QVBoxLayout>
```

`registerPages()` 中 item 的 sizeHint 改为:

```cpp
item->setSizeHint(QSize(110, 44));
```

- [ ] **Step 5: 运行测试确认通过**

Expected: `topNavIsHorizontalWithClock` PASS,原有 3 个用例仍 PASS。

- [ ] **Step 6: Commit**

```bash
git add project/mainwidget.h project/mainwidget.cpp tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "refactor(ui): 主框架改为顶部标签栏并显示时间"
```

---

### Task 3: 全局浅色主题样式表

**Files:**
- Modify: `project/style/mainstyle.qss`(整体重写)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 2 的 `headerBar`/`titleLabel`/`timeLabel`/`listWidget` 对象名。
- Produces: 全局 QSS 类名/对象名约定:`#card`(白色圆角卡片)、`#imageLabel`、`#videoPanel`、`#btnLeft`、`#btnRight`、`#dot`、`#primaryButton`、`#dangerButton`、`#successButton`、`#btn_start`、`#btn_connect`、`#btn_subscribe`、`#btn_publish`、`#statusLabel`、`#logEdit`、`#chartView`。后续页面任务直接复用。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void globalStyleIsLight();
```

实现:

```cpp
void TestMainWidget::globalStyleIsLight()
{
    QFile f(QStringLiteral(":/style/mainstyle.qss"));
    QVERIFY(f.open(QFile::ReadOnly));
    const QString css = QString::fromUtf8(f.readAll());
    QVERIFY(css.contains(QStringLiteral("#F4F6F8")));
    QVERIFY(css.contains(QStringLiteral("#2F7CF6")));
    QVERIFY(css.contains(QStringLiteral("listWidget")));
    QVERIFY(css.contains(QStringLiteral("#card")));
}
```

include 增加 `#include <QFile>`。

- [ ] **Step 2: 运行测试确认失败**

Expected: `globalStyleIsLight` FAIL(当前 QSS 不含这些内容)。

- [ ] **Step 3: 整体重写 mainstyle.qss**

```css
/* 全局浅色主题:方案 B */
QWidget#project {
    background: #F4F6F8;
    color: #1F2937;
    font-size: 15px;
}

#headerBar {
    background: #FFFFFF;
    border-bottom: 1px solid #E5E7EB;
}

#titleLabel {
    color: #1F2937;
    font-size: 20px;
    font-weight: bold;
}

#timeLabel {
    color: #6B7280;
    font-size: 16px;
}

QListWidget#listWidget {
    background: transparent;
    border: none;
    outline: none;
}
QListWidget#listWidget::item {
    color: #374151;
    padding: 0 10px;
    margin: 6px 4px;
    border-radius: 16px;
    background: transparent;
}
QListWidget#listWidget::item:selected {
    background: #2F7CF6;
    color: #FFFFFF;
}
QListWidget#listWidget::item:hover:!selected {
    background: #EFF6FF;
    color: #2F7CF6;
}

/* 卡片 */
QGroupBox,
QFrame#card,
QWidget#card {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 10px;
}
QFrame#card,
QWidget#card {
    padding: 8px;
}
QGroupBox {
    margin-top: 14px;
    padding: 12px 8px 8px 8px;
}
QGroupBox::title {
    subcontrol-origin: margin;
    left: 12px;
    padding: 0 6px;
    color: #1F2937;
    font-weight: bold;
}

/* 按钮 */
QPushButton {
    min-height: 44px;
    padding: 0 18px;
    border-radius: 8px;
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    color: #374151;
    font-size: 15px;
}
QPushButton:hover {
    border-color: #2F7CF6;
    color: #2F7CF6;
}
QPushButton:pressed {
    background: #EFF6FF;
}
QPushButton:checked {
    background: #2F7CF6;
    color: #FFFFFF;
    border-color: #2F7CF6;
}
QPushButton:disabled {
    color: #9CA3AF;
    background: #F3F4F6;
    border-color: #E5E7EB;
}
QPushButton#primaryButton {
    background: #2F7CF6;
    color: #FFFFFF;
    border: none;
}
QPushButton#dangerButton {
    background: #EF4444;
    color: #FFFFFF;
    border: none;
}
QPushButton#successButton {
    background: #22C55E;
    color: #FFFFFF;
    border: none;
}

/* 输入控件 */
QLineEdit, QComboBox, QSpinBox, QPlainTextEdit {
    background: #FFFFFF;
    border: 1px solid #D1D5DB;
    border-radius: 6px;
    padding: 6px 8px;
    color: #1F2937;
    selection-background-color: #2F7CF6;
}
QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QPlainTextEdit:focus {
    border-color: #2F7CF6;
}
QComboBox::drop-down {
    border: none;
    width: 24px;
}
QComboBox QAbstractItemView {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    selection-background-color: #EFF6FF;
    selection-color: #2F7CF6;
}

/* 列表与日志 */
QListWidget, QPlainTextEdit {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 8px;
}
QListWidget::item {
    padding: 6px;
}
QListWidget::item:selected {
    background: #EFF6FF;
    color: #2F7CF6;
}
QListWidget::item:hover {
    background: #F9FAFB;
}

/* 滑条 */
QSlider::groove:horizontal {
    height: 6px;
    background: #E5E7EB;
    border-radius: 3px;
}
QSlider::handle:horizontal {
    width: 18px;
    height: 18px;
    margin: -6px 0;
    border-radius: 9px;
    background: #2F7CF6;
}
QSlider::sub-page:horizontal {
    background: #2F7CF6;
    border-radius: 3px;
}

/* 滚动条 */
QScrollBar:vertical {
    width: 10px;
    background: transparent;
}
QScrollBar::handle:vertical {
    background: #D1D5DB;
    border-radius: 5px;
    min-height: 30px;
}
QScrollBar::add-line, QScrollBar::sub-line {
    height: 0;
}

QScrollArea {
    background: transparent;
    border: none;
}

QMessageBox {
    background: #FFFFFF;
}

/* 轮播图 */
#imageLabel {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-radius: 10px;
}
#btnLeft, #btnRight {
    min-width: 44px;
    max-width: 48px;
    min-height: 44px;
    max-height: 48px;
    border-radius: 24px;
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    padding: 0;
}
#btnLeft:hover, #btnRight:hover {
    border-color: #2F7CF6;
}
QPushButton#dot {
    min-width: 14px;
    max-width: 14px;
    min-height: 14px;
    max-height: 14px;
    border-radius: 7px;
    padding: 0;
    background: #D1D5DB;
    border: none;
}
QPushButton#dot:checked {
    background: #2F7CF6;
}

/* 摄像头画面 */
#videoPanel {
    background: #E5E7EB;
    color: #6B7280;
    border: 1px solid #D1D5DB;
    border-radius: 10px;
}

/* MQTT 状态 */
#statusLabel {
    font-weight: bold;
    padding: 4px 10px;
    border-radius: 10px;
}
```

- [ ] **Step 4: 运行测试确认通过**

Expected: `globalStyleIsLight` PASS。

- [ ] **Step 5: Commit**

```bash
git add project/style/mainstyle.qss tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(ui): 新增全局浅色主题样式表"
```

---

### Task 4: 轮播图页面浅色卡片化

**Files:**
- Modify: `project/form/banna.cpp`(`setupUi()`)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 1 的 `arrow_left.png`/`arrow_right.png`;Task 3 的 `#card`/`#imageLabel`/`#btnLeft`/`#btnRight`/`#dot` 样式。
- Produces: `imageLabel`、`btnLeft`、`btnRight`、`dot` 对象名,供测试与样式匹配。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void bannaHasLightWidgets();
```

实现:

```cpp
void TestMainWidget::bannaHasLightWidgets()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(0);
    QVERIFY(page->findChild<QLabel *>(QStringLiteral("imageLabel")) != nullptr);
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btnLeft")) != nullptr);
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btnRight")) != nullptr);
    QVERIFY(!page->findChild<QPushButton *>(QStringLiteral("btnLeft"))->icon().isNull());
}
```

- [ ] **Step 2: 运行测试确认失败**

Expected: `bannaHasLightWidgets` FAIL。

- [ ] **Step 3: 修改 banna.cpp 的 setupUi()**

include 增加:

```cpp
#include <QIcon>
```

在 `setupUi()` 中做以下修改:

```cpp
auto *imageArea = new QWidget(this);
imageArea->setObjectName(QStringLiteral("card"));
auto *imageLayout = new QHBoxLayout(imageArea);
imageLayout->setContentsMargins(12, 12, 12, 12);
imageLayout->setSpacing(12);

m_imageLabel = new QLabel(imageArea);
m_imageLabel->setObjectName(QStringLiteral("imageLabel"));
m_imageLabel->setAlignment(Qt::AlignCenter);
m_imageLabel->setScaledContents(true);
m_imageLabel->setMinimumSize(480, 360);

m_leftButton = new QPushButton(imageArea);
m_rightButton = new QPushButton(imageArea);
m_leftButton->setObjectName(QStringLiteral("btnLeft"));
m_rightButton->setObjectName(QStringLiteral("btnRight"));
m_leftButton->setIcon(QIcon(QStringLiteral(":/images/icons/arrow_left.png")));
m_rightButton->setIcon(QIcon(QStringLiteral(":/images/icons/arrow_right.png")));
m_leftButton->setFixedSize(48, 48);
m_rightButton->setFixedSize(48, 48);
```

圆点循环中为每个 dot 增加对象名:

```cpp
dot->setObjectName(QStringLiteral("dot"));
```

底部圆点栏也放进卡片容器:

```cpp
auto *dotBar = new QWidget(this);
dotBar->setObjectName(QStringLiteral("card"));
m_dotLayout = new QHBoxLayout(dotBar);
m_dotLayout->setContentsMargins(12, 12, 12, 12);
```

- [ ] **Step 4: 运行测试确认通过**

Expected: `bannaHasLightWidgets` PASS。

- [ ] **Step 5: 人工视觉检查**

构建运行 `x86\build.cmd` 后启动 `x86\release\project.exe`,切到“轮播图”,确认:白卡片、蓝色圆点、箭头图标清晰、图片不溢出。

- [ ] **Step 6: Commit**

```bash
git add project/form/banna.cpp tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(banna): 轮播图页面浅色卡片化"
```

---

### Task 5: OpenCV 页面浅色卡片化

**Files:**
- Modify: `project/form/opencv.cpp`(`setupUi()`)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 3 的 `#card`、`#imageLabel`、`#primaryButton` 样式。
- Produces: `imageLabel`(4 个预览图共用)、`primaryButton`(选择图像按钮);QGroupBox 标题保持不变。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void opencvHasLightWidgets();
```

实现:

```cpp
void TestMainWidget::opencvHasLightWidgets()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(1);
    QVERIFY(page->findChild<QLabel *>(QStringLiteral("imageLabel")) != nullptr);
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("primaryButton")) != nullptr);
    bool found = false;
    const auto boxes = page->findChildren<QGroupBox *>();
    for (QGroupBox *box : boxes) {
        if (box->title() == QStringLiteral("滤波"))
            found = true;
    }
    QVERIFY(found);
}
```

include 增加 `#include <QGroupBox>`。

- [ ] **Step 2: 运行测试确认失败**

Expected: `opencvHasLightWidgets` FAIL。

- [ ] **Step 3: 修改 opencv.cpp 的 setupUi()**

`imagePanel` 增加卡片对象名:

```cpp
auto *imagePanel = new QWidget(this);
imagePanel->setObjectName(QStringLiteral("card"));
```

删除暗色 `labelStyle`,改为对象名 + 全局样式:

```cpp
for (QLabel *label : {m_srcLabel, m_resultLabel, m_extraLabel1, m_extraLabel2}) {
    label->setObjectName(QStringLiteral("imageLabel"));
    label->setMinimumSize(300, 220);
    label->setAlignment(Qt::AlignCenter);
    label->setScaledContents(true);
}
```

“选择图像”按钮设为主按钮:

```cpp
auto *openButton = new QPushButton(QStringLiteral("选择图像"), imagePanel);
openButton->setObjectName(QStringLiteral("primaryButton"));
```

- [ ] **Step 4: 运行测试确认通过**

Expected: `opencvHasLightWidgets` PASS。

- [ ] **Step 5: 人工视觉检查**

切到“OpenCV测试”,确认预览图白底卡片、按钮网格为白色圆角卡片。

- [ ] **Step 6: Commit**

```bash
git add project/form/opencv.cpp tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(opencv): OpenCV 页面浅色卡片化"
```

---

### Task 6: 摄像头人脸识别页面浅色卡片化

**Files:**
- Modify: `project/form/facedetect.cpp`(`setupUi()`)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 3 的 `#videoPanel`、`#card` 样式。
- Produces: `videoPanel`(画面)、`btn_start`(开始/关闭)、`card`(控制栏)。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void faceDetectHasLightWidgets();
```

实现:

```cpp
void TestMainWidget::faceDetectHasLightWidgets()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(2);
    QVERIFY(page->findChild<QLabel *>(QStringLiteral("videoPanel")) != nullptr);
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btn_start")) != nullptr);
}
```

- [ ] **Step 2: 运行测试确认失败**

Expected: `faceDetectHasLightWidgets` FAIL。

- [ ] **Step 3: 修改 facedetect.cpp 的 setupUi()**

include 增加:

```cpp
#include <QFrame>
```

画面标签改为对象名并去掉内联暗色:

```cpp
m_display = new QLabel(QStringLiteral("摄像头画面"), this);
m_display->setObjectName(QStringLiteral("videoPanel"));
m_display->setAlignment(Qt::AlignCenter);
m_display->setMinimumSize(480, 380);
m_display->setScaledContents(true);
```

控制栏改为卡片容器:

```cpp
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
```

- [ ] **Step 4: 运行测试确认通过**

Expected: `faceDetectHasLightWidgets` PASS。

- [ ] **Step 5: 人工视觉检查**

切到“摄像头人脸识别”,确认画面区浅灰、控制栏白卡片。

- [ ] **Step 6: Commit**

```bash
git add project/form/facedetect.cpp tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(facedetect): 摄像头页面浅色卡片化"
```

---

### Task 7: imx6ull 板级设备页面浅色化与曲线配色

**Files:**
- Modify: `project/form/imx6ulltest.cpp`
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 3 的 `QGroupBox` 卡片样式。
- Produces: `chartView` 对象名;浅色曲线配色(蓝 `#2F7CF6`、绿 `#22C55E`、橙 `#F59E0B`)。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void boardPageHasChart();
```

实现:

```cpp
void TestMainWidget::boardPageHasChart()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(3);
    QVERIFY(page->findChild<QChartView *>(QStringLiteral("chartView")) != nullptr);
}
```

include 增加 `#include <QChartView>`。

- [ ] **Step 2: 运行测试确认失败**

Expected: `boardPageHasChart` FAIL。

- [ ] **Step 3: 修改 imx6ulltest.cpp**

include 增加:

```cpp
#include <QColor>
```

在 `setupUi()` 创建 chart 之后追加浅色配置:

```cpp
chart->setBackgroundBrush(QBrush(QColor(QStringLiteral("#FFFFFF"))));
chart->setPlotAreaBackgroundBrush(QBrush(QColor(QStringLiteral("#F9FAFB"))));
chart->setPlotAreaBackgroundVisible(true);
chart->setTitleBrush(QBrush(QColor(QStringLiteral("#1F2937"))));
chart->legend()->setLabelColor(QColor(QStringLiteral("#6B7280")));

axisX->setLabelsColor(QColor(QStringLiteral("#6B7280")));
axisY->setLabelsColor(QColor(QStringLiteral("#6B7280")));
axisX->setLinePenColor(QColor(QStringLiteral("#E5E7EB")));
axisY->setLinePenColor(QColor(QStringLiteral("#E5E7EB")));
axisX->setGridLineColor(QColor(QStringLiteral("#F3F4F6")));
axisY->setGridLineColor(QColor(QStringLiteral("#F3F4F6")));

m_alsSeries->setColor(QColor(QStringLiteral("#2F7CF6")));
m_irSeries->setColor(QColor(QStringLiteral("#22C55E")));
m_psSeries->setColor(QColor(QStringLiteral("#F59E0B")));
```

`m_chartView` 增加对象名:

```cpp
m_chartView = new QChartView(chart, this);
m_chartView->setObjectName(QStringLiteral("chartView"));
m_chartView->setRenderHint(QPainter::Antialiasing);
```

- [ ] **Step 4: 运行测试确认通过**

Expected: `boardPageHasChart` PASS。

- [ ] **Step 5: 人工视觉检查**

切到“imx6ull板级设备”,确认:分组框白色卡片、曲线浅色背景、图例可读。

- [ ] **Step 6: Commit**

```bash
git add project/form/imx6ulltest.cpp tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(imx6ulltest): 板级设备页浅色化与曲线配色"
```

---

### Task 8: 音频播放器浅色化与新图标

**Files:**
- Modify: `project/form/musicplay.cpp`
- Modify: `project/style/music_style.qss`(整体重写)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 1 的播放/暂停/上一首/下一首/收藏/列表/菜单/音量图标。
- Produces: 按钮图标初始为播放态,`onStateChanged` 中随状态切换播放/暂停图标;左右区域为 `card` 对象名。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void musicPageHasIcons();
```

实现:

```cpp
void TestMainWidget::musicPageHasIcons()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(4);
    auto *play = page->findChild<QPushButton *>(QStringLiteral("btn_play"));
    QVERIFY(play != nullptr);
    QVERIFY(!play->icon().isNull());
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btn_previous")) != nullptr);
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btn_next")) != nullptr);
}
```

- [ ] **Step 2: 运行测试确认失败**

Expected: `musicPageHasIcons` FAIL(当前按钮无图标)。

- [ ] **Step 3: 修改 musicplay.cpp 的 setupUi()**

include 增加:

```cpp
#include <QFrame>
#include <QIcon>
```

将左右两个区域改为卡片:

```cpp
auto *root = new QHBoxLayout(this);
root->setContentsMargins(12, 12, 12, 12);
root->setSpacing(12);

auto *leftCard = new QFrame(this);
leftCard->setObjectName(QStringLiteral("card"));
auto *left = new QVBoxLayout(leftCard);
left->setContentsMargins(12, 12, 12, 12);
```

删除 title 的内联深色样式(保留文字):

```cpp
auto *title = new QLabel(QStringLiteral("Q Music,Enjoy it!"), leftCard);
```

按钮设置图标(替换原来的无图标状态):

```cpp
m_prevButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_prev.png")));
m_playButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_play.png")));
m_nextButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_next.png")));
m_favoriteButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_favorite.png")));
m_modeButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_list.png")));
m_menuButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_menu.png")));
m_volumeButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume.png")));
```

同时把原来的统一 `setFixedSize(64, 64)` 循环改为与 QSS 一致的尺寸(播放键 72,其余 64):

```cpp
m_prevButton->setFixedSize(64, 64);
m_playButton->setFixedSize(72, 72);
m_nextButton->setFixedSize(64, 64);
```

小按钮尺寸由 32 改为 36(与 QSS 一致):

```cpp
for (QPushButton *button : {m_favoriteButton, m_modeButton, m_menuButton, m_volumeButton})
    button->setFixedSize(36, 36);
```

右侧改为:

```cpp
auto *rightCard = new QFrame(this);
rightCard->setObjectName(QStringLiteral("card"));
auto *right = new QVBoxLayout(rightCard);
right->setContentsMargins(12, 12, 12, 12);
```

删除 `m_timeLabel`/`m_durationLabel` 的 `color:white` 内联样式,保留文本与排版。最后:

```cpp
root->addWidget(leftCard, 1);
root->addWidget(rightCard, 1);
```

`onStateChanged` 增加图标切换:

```cpp
void MusicPlay::onStateChanged(QMediaPlayer::State state)
{
    const bool playing = (state == QMediaPlayer::PlayingState);
    m_playButton->setChecked(playing);
    m_playButton->setIcon(QIcon(playing
        ? QStringLiteral(":/images/icons/btn_pause.png")
        : QStringLiteral(":/images/icons/btn_play.png")));
}
```

- [ ] **Step 4: 整体重写 music_style.qss**

```css
/* 音频播放器浅色样式 */
MusicPlay {
    background: #F4F6F8;
}

QLabel {
    color: #1F2937;
}

#listWidget {
    background: transparent;
    border: none;
}
#listWidget::item {
    padding: 8px;
    border-radius: 6px;
}
#listWidget::item:selected {
    background: #EFF6FF;
    color: #2F7CF6;
}

#btn_previous, #btn_next {
    min-width: 64px;
    max-width: 64px;
    min-height: 64px;
    max-height: 64px;
    border-radius: 32px;
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    padding: 0;
}
#btn_play {
    min-width: 72px;
    max-width: 72px;
    min-height: 72px;
    max-height: 72px;
    border-radius: 36px;
    background: #2F7CF6;
    border: none;
    padding: 0;
}
#btn_favorite, #btn_mode, #btn_menu, #btn_volume {
    min-width: 36px;
    max-width: 36px;
    min-height: 36px;
    max-height: 36px;
    border-radius: 18px;
    background: transparent;
    border: none;
    padding: 0;
}
```

- [ ] **Step 5: 运行测试确认通过**

Expected: `musicPageHasIcons` PASS。

- [ ] **Step 6: 人工视觉检查**

切到“音频播放器”,确认:白卡片、蓝色圆形播放键、其余图标深灰清晰。

- [ ] **Step 7: Commit**

```bash
git add project/form/musicplay.cpp project/style/music_style.qss tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(musicplay): 音频播放器浅色化并接入新图标"
```

---

### Task 9: 视频播放器浅色化与新图标

**Files:**
- Modify: `project/form/videoplayer.cpp`
- Modify: `project/style/video_style.qss`(整体重写)
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 1 的播放/暂停/下一集/音量+/音量-/全屏图标。
- Produces: 控制条浅色;播放按钮图标随状态切换;侧栏 `card` 对象名。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void videoPageHasIcons();
```

实现:

```cpp
void TestMainWidget::videoPageHasIcons()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(5);
    auto *play = page->findChild<QPushButton *>(QStringLiteral("btn_play"));
    QVERIFY(play != nullptr);
    QVERIFY(!play->icon().isNull());
    QVERIFY(page->findChild<QPushButton *>(QStringLiteral("btn_screen")) != nullptr);
}
```

- [ ] **Step 2: 运行测试确认失败**

Expected: `videoPageHasIcons` FAIL。

- [ ] **Step 3: 修改 videoplayer.cpp**

include 增加:

```cpp
#include <QIcon>
```

`setupUi()` 中:

```cpp
auto *root = new QHBoxLayout(this);
root->setContentsMargins(12, 12, 12, 12);
root->setSpacing(12);
```

视频面板保留 `vWidget0`,控制条保留 `vWidget1`,侧栏改为卡片:

```cpp
m_sidePanel = new QWidget(this);
m_sidePanel->setObjectName(QStringLiteral("card"));
```

控制按钮设置图标:

```cpp
m_playButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_play.png")));
m_nextButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_next.png")));
m_volumeDownButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume_down.png")));
m_volumeUpButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_volume_up.png")));
m_fullscreenButton->setIcon(QIcon(QStringLiteral(":/images/icons/btn_fullscreen.png")));
```

`onStateChanged` 增加图标切换:

```cpp
void VideoPlayer::onStateChanged(QMediaPlayer::State state)
{
    const bool playing = (state == QMediaPlayer::PlayingState);
    m_playButton->setChecked(playing);
    m_playButton->setIcon(QIcon(playing
        ? QStringLiteral(":/images/icons/btn_pause.png")
        : QStringLiteral(":/images/icons/btn_play.png")));
}
```

- [ ] **Step 4: 整体重写 video_style.qss**

```css
/* 视频播放器浅色样式 */
VideoPlayer {
    background: #F4F6F8;
}

#vWidget0 {
    background: #111827;
    border: 1px solid #E5E7EB;
    border-radius: 10px;
}

#vWidget1 {
    background: #FFFFFF;
    border: 1px solid #E5E7EB;
    border-top: none;
    border-bottom-left-radius: 10px;
    border-bottom-right-radius: 10px;
}

QLabel {
    color: #1F2937;
}

#btn_play {
    min-width: 48px;
    max-width: 48px;
    min-height: 48px;
    max-height: 48px;
    border-radius: 24px;
    background: #2F7CF6;
    border: none;
    padding: 0;
}
#btn_next, #btn_volumedown, #btn_volumeup, #btn_screen {
    min-width: 44px;
    max-width: 44px;
    min-height: 44px;
    max-height: 44px;
    border-radius: 22px;
    background: transparent;
    border: none;
    padding: 0;
}

#listWidget {
    background: transparent;
    border: none;
}
#listWidget::item {
    padding: 8px;
    border-radius: 6px;
}
#listWidget::item:selected {
    background: #EFF6FF;
    color: #2F7CF6;
}
```

- [ ] **Step 5: 运行测试确认通过**

Expected: `videoPageHasIcons` PASS。

- [ ] **Step 6: 人工视觉检查**

切到“视频播放器”,确认:黑色视频区、浅色控制条、右侧白卡片列表。

- [ ] **Step 7: Commit**

```bash
git add project/form/videoplayer.cpp project/style/video_style.qss tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(videoplayer): 视频播放器浅色化并接入新图标"
```

---

### Task 10: MQTT 页面浅色化与连接状态

**Files:**
- Modify: `project/form/mqtt_client.cpp`
- Test: `tests/tst_mainwidget/tst_mainwidget.cpp`

**Interfaces:**
- Consumes: Task 3 的 `QGroupBox` 卡片样式、`#statusLabel` 样式。
- Produces: `statusLabel`、`btn_connect`、`btn_subscribe`、`btn_publish`、`logEdit` 对象名;`onStateChanged` 中状态文字与颜色联动。

- [ ] **Step 1: 写失败测试**

private slots 追加:

```cpp
void mqttPageHasStatus();
```

实现:

```cpp
void TestMainWidget::mqttPageHasStatus()
{
    MainWidget w;
    auto *stack = w.findChild<QStackedWidget *>(QStringLiteral("stackedWidget"));
    QWidget *page = stack->widget(6);
    QVERIFY(page->findChild<QLabel *>(QStringLiteral("statusLabel")) != nullptr);
    QVERIFY(page->findChild<QPlainTextEdit *>(QStringLiteral("logEdit")) != nullptr);
}
```

include 增加 `#include <QPlainTextEdit>`。

- [ ] **Step 2: 运行测试确认失败**

Expected: `mqttPageHasStatus` FAIL。

- [ ] **Step 3: 修改 mqtt_client.cpp**

`setupUi()` 的“连接”卡片内增加状态标签:

```cpp
m_statusLabel = new QLabel(QStringLiteral("未连接"), connBox);
m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
connLayout->addWidget(m_statusLabel);
```

按钮与日志增加对象名:

```cpp
m_connectButton->setObjectName(QStringLiteral("btn_connect"));
m_subscribeButton->setObjectName(QStringLiteral("btn_subscribe"));
m_publishButton->setObjectName(QStringLiteral("btn_publish"));
m_log->setObjectName(QStringLiteral("logEdit"));
```

`onStateChanged` 增加状态文字与颜色:

```cpp
void mqtt_client::onStateChanged()
{
    const SimpleMqttClient::State state = m_client->state();
    const QString text = [state]() {
        switch (state) {
        case SimpleMqttClient::Disconnected: return QStringLiteral("未连接");
        case SimpleMqttClient::Connecting:   return QStringLiteral("连接中");
        case SimpleMqttClient::Connected:    return QStringLiteral("已连接");
        }
        return QStringLiteral("未知");
    }();
    appendLog(QStringLiteral("%1 --- 状态变化: %2")
                  .arg(QDateTime::currentDateTime().toString(Qt::ISODate), text));

    m_connectButton->setText(state == SimpleMqttClient::Connected
                                 ? QStringLiteral("Disconnect")
                                 : QStringLiteral("Connect"));
    m_statusLabel->setText(text);
    if (state == SimpleMqttClient::Connected)
        m_statusLabel->setStyleSheet(QStringLiteral("color:#22C55E;"));
    else if (state == SimpleMqttClient::Connecting)
        m_statusLabel->setStyleSheet(QStringLiteral("color:#F59E0B;"));
    else
        m_statusLabel->setStyleSheet(QStringLiteral("color:#6B7280;"));
}
```

`mqtt_client.h` 增加成员 `QLabel *m_statusLabel = nullptr;`。

- [ ] **Step 4: 运行测试确认通过**

Expected: `mqttPageHasStatus` PASS。

- [ ] **Step 5: 人工视觉检查**

切到“MQTT_CLIENT”,确认:三个白色分组卡片、日志白底、状态标签存在。

- [ ] **Step 6: Commit**

```bash
git add project/form/mqtt_client.cpp project/form/mqtt_client.h tests/tst_mainwidget/tst_mainwidget.cpp
git commit -m "style(mqtt): MQTT 页面浅色化并显示连接状态"
```

---

### Task 11: 全量构建、测试与 UI 回归验证

**Files:**
- 不改业务代码;如发现样式遗漏,按对应 Task 修。

**Interfaces:**
- 无新增接口;验证全部 Task 产物。

- [ ] **Step 1: 重新生成图标并全量构建**

```bat
python scripts\generate_icons.py
cd x86
call build.cmd
```

Expected: 编译无错误,`x86\release\project.exe` 生成。

- [ ] **Step 2: 构建并运行全部测试**

```bat
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cd tests
qmake tests.pro
nmake
set QT_QPA_PLATFORM=offscreen
tst_mainwidget\release\tst_mainwidget.exe -o result.txt
tst_hardware\release\tst_hardware.exe -o result.txt
```

Expected: 两个测试程序全部用例 PASS(原 3 个主框架用例 + 11 个新增 UI 用例 + tst_hardware 用例)。

- [ ] **Step 3: 人工 UI 回归**

启动 `x86\release\project.exe`,依次切换 7 个标签,检查:

- 顶部标签栏:白底、选中蓝色药丸、时间每秒刷新;
- 无深色残留(除视频画面本身);
- 轮播图箭头/圆点、媒体按钮图标清晰;
- OpenCV 预览与按钮组在卡片内;
- 板级设备曲线浅色可读;
- MQTT 三个卡片与状态标签正常;
- 1024×600 下无内容溢出,按钮可点。

- [ ] **Step 4: 提交遗漏修正(如有)**

如有样式遗漏,修正后:

```bash
git add -A
git commit -m "fix(ui): 全量回归修正浅色样式细节"
```

- [ ] **Step 5: 板端验证(用户环境执行)**

在 Ubuntu 执行 `./build_arm.sh` 与 `sudo ./deploy_sd.sh /dev/sdX /dev/sdX2`,板端 eglfs 运行,确认 1024×600 布局与触摸正常。
