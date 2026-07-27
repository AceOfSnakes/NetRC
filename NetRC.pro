QT       += core gui opengl widgets xml network
APPName = MtkFwTool
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

unix {
    # 1. Allow the Debian environment toolchain overrides
    isEmpty(QMAKE_CC): QMAKE_CC = gcc
    isEmpty(QMAKE_CXX): QMAKE_CXX = g++
    isEmpty(QMAKE_LINK): QMAKE_LINK = g++
    TARGET = netrc

    # 2. Get the Multiarch triplet dynamically from the build system
    DEB_HOST_MULTIARCH = $$(DEB_HOST_MULTIARCH)
    isEmpty(DEB_HOST_MULTIARCH) {
        DEB_HOST_MULTIARCH = $$system(dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null)
    }

    # 3. Add the active architecture's system header folder
    !isEmpty(DEB_HOST_MULTIARCH) {
        # This will resolve to /usr/include/x86_64-linux-gnu for amd64 
        # and /usr/include/aarch64-linux-gnu for arm64
        INCLUDEPATH += /usr/include/$$DEB_HOST_MULTIARCH
        DEPENDPATH  += /usr/include/$$DEB_HOST_MULTIARCH
        LIBS        += -L/usr/lib/$$DEB_HOST_MULTIARCH
    }
    
    # 4. Clean way to link OpenSSL dynamically via pkg-config
    CONFIG += link_pkgconfig
    PKGCONFIG += openssl
} else {
    TARGET = NetRC
}

static { # everything below takes effect with CONFIG += static
    CONFIG += static
    DEFINES += STATIC
    message("~~~ static build ~~~") # this is for information, that the static build is done
    win32: TARGET = $$join(TARGET,,,) #this adds an s in the end, so you can seperate static build from non static build
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
#CONFIG += c++17
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

TEMPLATE = app
INCLUDEPATH += src/include/
INCLUDEPATH += src/commons/include/
SOURCES += src/main.cpp\
        src/RemoteButton.cpp \
        src/commons/commons.cpp \
        src/commons/filedialogwithhistory.cpp \
        src/commons/settings.cpp \
        src/debug.cpp \
        src/aboutdialog.cpp \
        src/autosearchdialog.cpp \
        src/deviceconnector.cpp \
        src/deviceinterface.cpp \
        src/discoverydevice.cpp \
        src/appsettings.cpp \
        src/rcsettings.cpp \
        src/crypto.cpp \
        src/remotecontrol.cpp 

HEADERS  += src/include/remotecontrol.h \
    src/commons/include/commons.h \
    src/commons/include/filedialogwithhistory.h \
    src/commons/include/settings.h \
    src/include/RemoteButton.h \
    src/include/appsettings.h \
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

