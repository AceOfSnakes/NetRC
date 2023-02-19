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

VSCMD_VER = $$(VSCMD_VER)
VSVERSION = $$(VisualStudioVersion)

!isEmpty(VSCMD_VER) {
   message("~~~ VSCMD_VER $$(VSCMD_VER) ~~~")
   DEFINES += __VSCMD_VER=\\\"$$(VSCMD_VER)\\\"
   DEFINES += __VSVERSION=$$(VisualStudioVersion)
}

TARGET = NetRC
TEMPLATE = app
INCLUDEPATH += src/include/
INCLUDEPATH += src/commons/include/
SOURCES += src/main.cpp\
        src/commons/commons.cpp \
        src/commons/filedialogwithhistory.cpp \
        src/commons/settings.cpp \
        src/debug.cpp \
        src/aboutdialog.cpp \
        src/autosearchdialog.cpp \
        src/deviceconnector.cpp \
        src/deviceinterface.cpp \
        src/discoverydevice.cpp \
        src/rcsettings.cpp \
        src/remotecontrol.cpp 

HEADERS  += src/include/remotecontrol.h \
    src/commons/include/commons.h \
    src/commons/include/filedialogwithhistory.h \
    src/commons/include/settings.h \
    src/include/debug.h \
    src/include/aboutdialog.h \
    src/include/autosearchdialog.h \
    src/include/deviceconnector.h \
    src/include/deviceinterface.h \
    src/include/discoverydevice.h \
    src/include/rcsettings.h


FORMS    += src/ui/remotecontrol.ui \
    src/commons/ui/settings.ui \
    src/ui/aboutdialog.ui \
    src/ui/autosearchdialog.ui \
    src/ui/debug.ui \
    src/ui/deviceconnector.ui

RESOURCES += \
    src/resource.qrc

DISTFILES += \
    settings/BDOppo10X.json \
    settings/BDOppo20X.json \
    settings/BDOppo9X.json \
    settings/BDPioneer.json \
    settings/RCVPioneer.json

RC_ICONS = src/NetRC.ico
