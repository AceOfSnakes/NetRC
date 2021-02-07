#-------------------------------------------------
#
# Project created by QtCreator 2016-08-05T19:55:58
#
#-------------------------------------------------

QT       += core gui network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

static { # everything below takes effect with CONFIG += static
    CONFIG += static
    DEFINES += STATIC
    message("~~~ static build ~~~") # this is for information, that the static build is done
    message($$[QT_INSTALL_PREFIX])
    win32: TARGET = $$join(TARGET,,,) #this adds an s in the end, so you can seperate static build from non static build
}

TARGET = NetRC
TEMPLATE = app

SOURCES += main.cpp\
        debug.cpp \
        filedialogwithhistory.cpp \
        aboutdialog.cpp \
        autosearchdialog.cpp \
        deviceconnector.cpp \
        deviceinterface.cpp \
        discoverydevice.cpp \
        rcsettings.cpp \
        remotecontrol.cpp \
        settings.cpp

HEADERS  += remotecontrol.h \
    debug.h \
    filedialogwithhistory.h \
    aboutdialog.h \
    autosearchdialog.h \
    deviceconnector.h \
    deviceinterface.h \
    discoverydevice.h \
    global.h \
    rcsettings.h \
    settings.h

FORMS    += remotecontrol.ui \
    aboutdialog.ui \
    autosearchdialog.ui \
    debug.ui \
    deviceconnector.ui \
    settings.ui

RESOURCES += \
    resource.qrc

RC_ICONS = icon_power.ico

DISTFILES += \
    settings/BDOppo10X.json \
    settings/BDOppo20X.json \
    settings/BDOppo9X.json \
    settings/BDPioneer.json \
    settings/RCVPioneer.json

RC_ICONS = icon_power.ico
