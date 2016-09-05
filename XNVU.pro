#-------------------------------------------------
#
# Project created by QtCreator 2016-08-20T16:34:01
#
#-------------------------------------------------
CONFIG += c++11

QT       += core gui
QT       += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = XNVU
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    coremag.cpp \
    NVU.cpp \
    XFMS_DATA.cpp \
    qlistwidgetitemdata.cpp \
    qtablewidgetitemdata.cpp \
    CPoint.cpp \
    LMATH.cpp \
    xnvusettings.cpp \
    dialogsettings.cpp \
    waypoint.cpp \
    dialogwaypointedit.cpp \
    qflightplantable.cpp \
    dialogrsbn.cpp \
    dialogoptions.cpp \
    qframeclickable.cpp \
    dialogwpsedit.cpp \
    qsearchlist.cpp

HEADERS  += mainwindow.h \
    coremag.h \
    NVU.h \
    XFMS_DATA.h \
    qlistwidgetitemdata.h \
    waypoint.h \
    qtablewidgetitemdata.h \
    CPoint.h \
    LMATH.h \
    xnvusettings.h \
    dialogsettings.h \
    dialogwaypointedit.h \
    qflightplantable.h \
    dialogrsbn.h \
    dialogoptions.h \
    qframeclickable.h \
    dialogwpsedit.h \
    qsearchlist.h

FORMS    += mainwindow.ui \
    dialogsettings.ui \
    dialogwaypointedit.ui \
    dialogrsbn.ui \
    dialogoptions.ui \
    dialogwpsedit.ui
