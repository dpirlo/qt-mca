#-------------------------------------------------
#
# ARPET Software Management
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
        src/MainWindow.cpp \
        src/SetPMTs.cpp \
        src/apAutoCalib.cpp \
        src/apRecon.cpp \
        src/apThread.cpp \
        src/apAutoCalibThread.cpp

HEADERS  += \
        inc/qcustomplot.h \
        inc/apMCAE.hpp \
        inc/apExceptions.hpp \
        inc/SetPreferences.h \
        inc/MainWindow.h \
        inc/SetPMTs.h \
        inc/apAutoCalib.hpp \
        inc/apRecon.hpp \
        inc/apThread.hpp \
        inc/apAutoCalibThread.hpp

LIBS += -lboost_system

STYLES += qdarkstyle/style.qrc \
        qdarkstyle/style.qss
        qdarkstyle/rc/


FORMS += \
        ui/SetPreferences.ui \
        ui/MainWindow.ui \
        ui/SetPMTs.ui

INCLUDEPATH +=/opt/armadillo/include
LIBS +=-llapack
LIBS +=-lblas

RESOURCES     = qdarkstyle/style.qrc



CONFIG(debug, debug|release) {
    DESTDIR = build/debug
} else {
    DESTDIR = build/release
}

DISTFILES += \
    images/cargando2.gif \
    images/ic_cancel.png \
    images/ic_check_circle.png
