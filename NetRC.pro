QT       += core gui opengl widgets xml network
APPName = MtkFwTool
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix {
    QMAKE_CC = gcc
    QMAKE_CXX = g++
    QMAKE_LINK = g++
}

static { # everything below takes effect with CONFIG += static
    CONFIG += static
    DEFINES += STATIC
    message("~~~ static build ~~~") # this is for information, that the static build is done
    win32: TARGET = $$join(TARGET,,,) #this adds an s in the end, so you can seperate static build from non static build
    win32: LIBS += -lavcodec -lavdevice -lavfilter -lavformat -lavutil -lswresample -lswscale
}

VSCMD_VER = $$(VSCMD_VER)
VSVERSION = $$(VisualStudioVersion)
FORCEDAPPVERSION = $$(APP_VERSION_VALUE)


VSCMD_VER = $$(VSCMD_VER)
VSVERSION = $$(VisualStudioVersion)
FORCEDAPPVERSION = $$(APP_VERSION_VALUE)

win32: LIBS += -llibcrypto
else: LIBS += -lcrypto

contains(QMAKE_TARGET.arch, x86_64) {
    message("Compiling for a 64-bit system")
    X64 = true
}

isEmpty(X64) {
  INCLUDEPATH += f:/usr/lib32/openssl3/include
  LIBS += -L/usr/local/lib
  LIBS += -Lf:/usr/lib32/openssl3/lib
  LIBS += -Lf:/usr/lib32/openssl3/bin
} else {
  INCLUDEPATH += f:/usr/lib64/openssl3/include
  LIBS += -L/usr/local/lib64
  LIBS += -Lf:/usr/lib64/openssl3/lib
  LIBS += -Lf:/usr/lib64/openssl3/bin
}

#message("~~~ APP_VER $$((APPVERSION)) ~~~")
!isEmpty(FORCEDAPPVERSION) {
    message("~~~ FORCED APP_VER $$(APP_VERSION_VALUE) ~~~")
    DEFINES += __FORCED_APP_VER=\\\"$$(APP_VERSION_VALUE)\\\"
}


!isEmpty(VSCMD_VER) {
   message("~~~ VSCMD_VER $$(VSCMD_VER) ~~~")
   DEFINES += __VSCMD_VER=\\\"$$(VSCMD_VER)\\\"
   DEFINES += __VSVERSION=$$(VisualStudioVersion)
}
contains(QMAKE_TARGET.arch, x86_64) {
    message("Compiling for a 64-bit system")
    X64 = true
}

isEmpty(QMAKE_TARGET.arch) {
    contains(QMAKE_HOST.arch, x86_64) {
        message("64-bit operation system")
        X64 = true
    }
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
        src/crypto.cpp \
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
    src/include/crypto.h \
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