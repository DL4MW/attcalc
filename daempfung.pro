#-------------------------------------------------
#
# Project created by QtCreator 2011-03-31T11:56:11
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = coaxatt
TEMPLATE = app

DEFINES-=UNICODE

SOURCES += main.cpp\
        dirmngr.cpp\
        simplex.cpp\
        widget.cpp\
        lmcurve.c\
        lmmin.c

HEADERS  += widget.h

FORMS    += widget.ui \
    cabledialog.ui

TRANSLATIONS = english.ts 

win32::RC_FILE = coaxatt.rc
