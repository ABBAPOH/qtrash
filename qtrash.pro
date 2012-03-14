#-------------------------------------------------
#
# Project created by QtCreator 2012-03-02T22:13:40
#
#-------------------------------------------------

QT       += core gui

TARGET = QTrash
TEMPLATE = app


SOURCES += main.cpp\
    qtrash.cpp \
    qtrashfileinfo.cpp \
    qtrashwindow.cpp \
    qtrashmodel.cpp \
    info2.cpp \
    qdriveinfo.cpp

HEADERS  += \
    qtrash.h \
    qtrashfileinfo.h \
    qtrashfileinfo_p.h \
    qtrash_p.h \
    qtrashwindow.h \
    qtrashmodel.h \
    info2_p.h \
    qdriveinfo.h \
    qdriveinfo_p.h

macx-* : {
OBJECTIVE_SOURCES += qtrash_mac.mm
SOURCES += qdriveinfo_mac.cpp
LIBS += -framework CoreServices -framework DiskArbitration -framework IOKit -framework QuickLook -framework AppKit
}

win32 : {
SOURCES += qdriveinfo_win.cpp
SOURCES += qtrash_win.cpp
LIBS += -luserenv -lNetapi32 -lMpr -luser32 -lWinmm
}

linux-* : {
SOURCES += qdriveinfo_linux.cpp
SOURCES += qtrash_linux.cpp
}

FORMS += \
    qtrashwindow.ui

















