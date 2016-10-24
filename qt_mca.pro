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

SOURCES += src/main.cpp\
        src/qcustomplot.cpp \
        src/apMCAE.cpp \
        src/apExceptions.cpp \
        src/SetPreferences.cpp \
        src/MainWindow.cpp

HEADERS  += \
        inc/qcustomplot.h \
        inc/apMCAE.hpp \
        inc/apExceptions.hpp \
        inc/SetPreferences.h \
        inc/MainWindow.h

LIBS += -lboost_system

FORMS += \
        ui/SetPreferences.ui \
        ui/MainWindow.ui

CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}
