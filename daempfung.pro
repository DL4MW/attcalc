#-------------------------------------------------
#
# Project created by QtCreator 2011-03-31T11:56:11
#
#-------------------------------------------------

QT       += core gui widgets network

TARGET = coaxatt
TEMPLATE = app

DEFINES-=UNICODE

SOURCES += main.cpp\
        dirmngr.cpp\
        simplex.cpp\
        widget.cpp\
        cableselect.cpp\
        lmcurve.c\
        lmmin.c

HEADERS  += widget.h \
    cableselect.h

FORMS    += widget.ui \
    cableselect.ui \
    filterdialog.ui

android {
    FORMS += cabledialog_android.ui
} else {
    FORMS += cabledialog.ui
}

TRANSLATIONS = english.ts

RESOURCES += coaxatt.qrc

win32::RC_FILE = coaxatt.rc

android {
    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
}
