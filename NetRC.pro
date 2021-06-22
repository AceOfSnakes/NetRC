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
INCLUDEPATH += src/include/
SOURCES += src/main.cpp\
        src/commons.cpp \
        src/debug.cpp \
        src/filedialogwithhistory.cpp \
        src/aboutdialog.cpp \
        src/autosearchdialog.cpp \
        src/deviceconnector.cpp \
        src/deviceinterface.cpp \
        src/discoverydevice.cpp \
        src/rcsettings.cpp \
        src/remotecontrol.cpp \
        src/settings.cpp

HEADERS  += src/include/remotecontrol.h \
    src/include/commons.h \
    src/include/debug.h \
    src/include/filedialogwithhistory.h \
    src/include/aboutdialog.h \
    src/include/autosearchdialog.h \
    src/include/deviceconnector.h \
    src/include/deviceinterface.h \
    src/include/discoverydevice.h \
    src/include/global.h \
    src/include/rcsettings.h \
    src/include/settings.h

FORMS    += src/ui/remotecontrol.ui \
    src/ui/aboutdialog.ui \
    src/ui/autosearchdialog.ui \
    src/ui/debug.ui \
    src/ui/deviceconnector.ui \
    src/ui/settings.ui

RESOURCES += \
    src/resource.qrc

RC_ICONS = src/icon_power.ico

DISTFILES += \
    settings/BDOppo10X.json \
    settings/BDOppo20X.json \
    settings/BDOppo9X.json \
    settings/BDPioneer.json \
    settings/RCVPioneer.json

RC_ICONS = src/icon_power.ico
