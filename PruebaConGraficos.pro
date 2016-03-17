#-------------------------------------------------
#
# Project created by QtCreator 2015-10-14T11:09:17
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt-arpet
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    apMCAE.cpp \
    apExceptions.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    apMCAE.hpp \
    apExceptions.hpp

LIBS += -lboost_system

FORMS    += mainwindow.ui

CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}
