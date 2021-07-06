QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Client1
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
    Communication.cpp \
    myWidget.cpp

HEADERS += \
    Communication.h \
    myWidget.h

FORMS += \
    mywidget.ui
QMAKE_CXXFLAGS += -std=gnu++11 \
         -std=c++0x -pthread
LIBS += -pthread
