#-------------------------------------------------
#
# Project created by QtCreator 2013-06-05T17:33:52
#
#-------------------------------------------------
# QT_CONFIG -= no-pkg-config
# CONFIG += link_pkgconfig
# PKGCONFIG += opencv


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = generator
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    ../common/showarea.cpp \
    ../common/editdialog.cpp \
    ../common/wpencoder.cpp \
    ../common/wpdefines.cpp \
    imagearea.cpp \
    clipdialog.cpp

HEADERS  += mainwindow.h \
    ../common/showarea.h \
    ../common/editdialog.h \
    ../common/wpencoder.h \
    ../common/wpdefines.h \
    imagearea.h \
    clipdialog.h

FORMS    += ui/mainwindow.ui \
    ui/clipdialog.ui \
    ui/editdialog.ui

win32 {
INCLUDEPATH += C:\package\opencv\build\include
INCLUDEPATH += C:\package\crypto++\include
LIBS += -LC:\package\opencv\build\x86\vc10\lib
LIBS += -LC:\package\crypto++\lib\Release
LIBS += -lcryptlib_md
}

INCLUDEPATH += ../common
INCLUDEPATH += /usr/local/Cellar/opencv/2.4.13.2/include/opencv
INCLUDEPATH += /usr/local/Cellar/opencv/2.4.13.2/include

LIBS += -L/usr/local/Cellar/opencv/2.4.13.2/lib -lopencv_core -lopencv_highgui -lopencv_imgproc
#LIBS += -lcryptlib_md




