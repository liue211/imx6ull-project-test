TARGET = tst_ota
TEMPLATE = app

QT += core network testlib
CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += $$PWD/../../project

# OTA 客户端纯函数测试:不依赖 UI / OpenCV,直接引用 otaclient
SOURCES += \
    tst_ota.cpp \
    ../../project/ota/otaclient.cpp

HEADERS += \
    ../../project/ota/otaclient.h
