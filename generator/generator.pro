#-------------------------------------------------
#
# Project created by QtCreator 2013-06-05T17:33:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = generator
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    ../common/showarea.cpp \
    ../common/wpencoder.cpp \
    ../common/wpdefines.cpp \
    imagearea.cpp \
    clipdialog.cpp \
    editdialog.cpp

HEADERS  += mainwindow.h \
    ../common/showarea.h \
    ../common/wpencoder.h \
    ../common/wpdefines.h \
    imagearea.h \
    clipdialog.h \
    editdialog.h

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

LIBS += -lopencv_core245 -lopencv_highgui245 -lopencv_imgproc245
#LIBS += -lcryptlib_md

