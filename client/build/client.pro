TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
QT += network

SOURCES += \
        ../src/main.cpp \
        ../src/tcpclient.cpp

HEADERS += \
    ../src/tcpclient.h
