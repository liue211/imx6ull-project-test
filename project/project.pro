QT       += core gui network multimedia multimediawidgets charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = project
TEMPLATE = app

CONFIG += c++11

# 编译时提示使用已弃用的 Qt API
DEFINES += QT_DEPRECATED_WARNINGS

# MSVC 下源码为 UTF-8,避免中文字符串按 GBK 误解码
msvc: QMAKE_CXXFLAGS += /utf-8

SOURCES += \
    main.cpp \
    mainwidget.cpp

HEADERS += \
    mainwidget.h

RESOURCES += \
    res.qrc

# ---------- 模块收集(.pri) ----------
include(./form/form.pri)
include(./hardware/hardware.pri)
include(./ota/ota.pri)

INCLUDEPATH += $$PWD/hardware
INCLUDEPATH += $$PWD/form
INCLUDEPATH += $$PWD/lib
INCLUDEPATH += $$PWD/ota

# ---------- 应用版本号(OTA 升级用,打包脚本需保持一致) ----------
DEFINES += APP_VERSION=\\\"1.0.0\\\"

# ---------- MQTT(自研轻量客户端,MQTT 3.1.1 / QoS0) ----------
HEADERS += \
    lib/simplemqtt.h

SOURCES += \
    lib/simplemqtt.cpp

# ---------- OpenCV ----------
# Windows 本机验证(OpenCV 3.4.x):默认 D:/opencv-3.4.16/opencv/build,可用 OPENCV_DIR 覆盖
win32 {
    isEmpty(OPENCV_DIR): OPENCV_DIR = D:/opencv-3.4.16/opencv/build
    INCLUDEPATH += $$OPENCV_DIR/include
    LIBS += -L$$OPENCV_DIR/x64/vc15/lib -lopencv_world3416
}
# Linux x86 / 板端(正点原子 SDK):ARM OpenCV 路径可用 ARM_OPENCV_DIR 覆盖,例如:
#   qmake ARM_OPENCV_DIR=/home/liu/linux/opencv-3.4.1/install ../project/project.pro
unix:!macx {
    isEmpty(ARM_OPENCV_DIR): ARM_OPENCV_DIR = /home/coucou/linux/opencv-3.4.1/install
    !exists($$ARM_OPENCV_DIR/lib/libopencv_core.so): error("OpenCV 未找到: $$ARM_OPENCV_DIR,请用 qmake ARM_OPENCV_DIR=<路径> 指定")
    message("ARM_OPENCV_DIR = $$ARM_OPENCV_DIR")
    INCLUDEPATH += $$ARM_OPENCV_DIR/include
    LIBS += $$ARM_OPENCV_DIR/lib/libopencv_core.so \
            $$ARM_OPENCV_DIR/lib/libopencv_imgproc.so \
            $$ARM_OPENCV_DIR/lib/libopencv_imgcodecs.so \
            $$ARM_OPENCV_DIR/lib/libopencv_highgui.so \
            $$ARM_OPENCV_DIR/lib/libopencv_objdetect.so \
            $$ARM_OPENCV_DIR/lib/libopencv_videoio.so \
            $$ARM_OPENCV_DIR/lib/libopencv_ml.so
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
