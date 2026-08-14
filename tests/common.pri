QT       += core gui network multimedia multimediawidgets charts testlib

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS

msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += $$PWD/../project
INCLUDEPATH += $$PWD/../project/hardware
INCLUDEPATH += $$PWD/../project/form
INCLUDEPATH += $$PWD/../project/lib
INCLUDEPATH += $$PWD/../project/ota

include(../project/form/form.pri)
include(../project/hardware/hardware.pri)
include(../project/ota/ota.pri)

SOURCES += \
    $$PWD/../project/lib/simplemqtt.cpp \
    $$PWD/../project/mainwidget.cpp

HEADERS += \
    $$PWD/../project/lib/simplemqtt.h \
    $$PWD/../project/mainwidget.h

# OpenCV(与主工程一致)
win32 {
    isEmpty(OPENCV_DIR): OPENCV_DIR = D:/opencv-3.4.16/opencv/build
    INCLUDEPATH += $$OPENCV_DIR/include
    LIBS += -L$$OPENCV_DIR/x64/vc15/lib -lopencv_world3416
}
