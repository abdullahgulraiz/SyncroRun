#-------------------------------------------------
#
# Project created by QtCreator 2017-08-24T03:23:45
#
#-------------------------------------------------

QT       += core gui serialport
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = untitled
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    player.cpp \
    qcustomplot.cpp

HEADERS  += mainwindow.h \
    player.h \
    qcustomplot.h

FORMS    += mainwindow.ui

LIBS += `pkg-config opencv --libs`

RESOURCES += \
    mytextresource.qrc
