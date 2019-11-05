TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
QT += network

SOURCES += \
        main.cpp \
        tcpsocket.cpp

HEADERS += \
    tcpsocket.h
