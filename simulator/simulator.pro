QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TARGET = simulator
TEMPLATE = app


SOURCES += main.cpp\
    mainwindow.cpp \
    ../common/showarea.cpp \
    ../common/wpencoder.cpp \
    ../common/wpdefines.cpp \
    ../common/wpmanager.cpp \
    simdialog.cpp

HEADERS  += mainwindow.h \
    ../common/showarea.h \
    ../common/wpencoder.h \
    ../common/wpdefines.h \
    ../common/wpmanager.h \
    simdialog.h

FORMS    += ui/mainwindow.ui \
    ui/simdialog.ui

INCLUDEPATH += ../common


win32 {
#INCLUDEPATH += C:\package\opencv\build\include
#INCLUDEPATH += C:\package\crypto++\include
#LIBS += -LC:\package\opencv\build\x86\vc10\lib
#LIBS += -LC:\package\crypto++\lib\Release
#LIBS += -lcryptlib_md
}

