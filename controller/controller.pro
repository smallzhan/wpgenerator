#-------------------------------------------------
#
# Project created by QtCreator 2013-09-03T20:42:29
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = controller
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp\
        ../common/wpsocket.cpp \
        ../common/wpdefines.cpp \
        ../common/wpencoder.cpp \
    schemaeditor.cpp

HEADERS  += mainwindow.h \
        ../common/wpsocket.h \
        ../common/wpdefines.h \
        ../common/wpencoder.h \
    schemaeditor.h

FORMS    += ui/mainwindow.ui \
    ui/schemaeditor.ui

INCLUDEPATH += ../common

unix {
INCLUDEPATH += /opt/ros/fuerte/include
LIBS += -L/opt/ros/fuerte/lib
LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc
}

win32 {
INCLUDEPATH += C:\package\opencv\build\include
#INCLUDEPATH += C:\package\crypto++\include
LIBS += -LC:\package\opencv\build\x86\vc10\lib
#LIBS += -LC:\package\crypto++\lib\Release
# LIBS += -lcryptlib_md
LIBS += -lopencv_core246 -lopencv_highgui246 -lopencv_imgproc246 -lws2_32
}
