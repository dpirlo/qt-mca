#-------------------------------------------------
#
# Project created by QtCreator 2015-10-14T11:09:17
#
#-------------------------------------------------

QT       += core gui
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-arpet
TEMPLATE = app

SOURCES += main.cpp\
        qcustomplot.cpp \
        apMCAE.cpp \
        apExceptions.cpp \
        SetPreferences.cpp \
        MainWindow.cpp

HEADERS  += \
        qcustomplot.h \
        apMCAE.hpp \
        apExceptions.hpp \
        SetPreferences.h \
        MainWindow.h

LIBS += -lboost_system

FORMS += \
        SetPreferences.ui \
        MainWindow.ui

CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}
