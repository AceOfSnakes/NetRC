#include "commons.h"
#include <QOperatingSystemVersion>
#include <QSettings>
#include <QMouseEvent>
#include <QApplication>
#if defined(Q_OS_WIN)
#include <qt_windows.h>
#endif

Commons::Commons() {

}

QString Commons::prettyProductName() {
#if defined(Q_OS_WIN)
    QSettings m("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
    QSettings::Registry64Format);

    QString displayVersion = m.value("DisplayVersion").toString();

    OSVERSIONINFOEX osver;
    //::GetVersionEx(osver);
    osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if(!GetVersionEx((OSVERSIONINFO*)&osver)) {
        return QSysInfo::prettyProductName();
    }
    const bool workstation = osver.wProductType == VER_NT_WORKSTATION;
    QOperatingSystemVersion version = QOperatingSystemVersion::current();

#define Q_WINVER(major, minor) (major << 8 | minor)
    switch (Q_WINVER(version.majorVersion(), version.minorVersion())) {
        case Q_WINVER(10, 0):
            if (!displayVersion.isEmpty()) {
                if(((workstation && version.microVersion() >= 21327) || (!workstation && version.microVersion() >= 17623)))
                    return QString(workstation ? "Windows 11" : "Windows Server 2022").append(" Version ").append(displayVersion);
                else
                    return QString(workstation ? "Windows 10" : "Windows Server 2016").append(" Version ").append(displayVersion);
            }
    }
#undef Q_WINVER

#endif
    return QSysInfo::prettyProductName();
}
void Commons::moveWindow(QObject *obj, QEvent *event, QMainWindow *window) {
    static bool mouseDown = false;
    static int xRealPos = 0;
    static int yRealPos = 0;

    QScreen *screen = QApplication::screens().at(0);
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if (obj->objectName() == window->objectName() && event->type() == QEvent::MouseButtonPress) {
        mouseDown = true;
        xRealPos = mouseEvent->globalPosition().x() - window->x();
        yRealPos = mouseEvent->globalPosition().y() - window->y();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        mouseDown = false;
    } else if (event->type() == QEvent::MouseMove) {
        if (mouseDown) {
            int xPos = window->x() - (window->x() - mouseEvent->globalPosition().x()) - xRealPos;
            int yPos = window->y() - (window->y() - mouseEvent->globalPosition().y()) - yRealPos;
            if(yPos + window->height() > screen->availableSize().height()) yPos = screen->availableSize().height() - window->height();
            if(xPos + window->width() > screen->availableSize().width()) xPos = screen->availableSize().width() - window->width();
            if(xPos< 0) xPos=0;
            if(yPos< 0) yPos=0;
            window->move(xPos, yPos);
        }
    }
}

QString Commons::compilerQString() {
    QString compiler;
#ifdef __clang_version__
    compiler.append(QString().asprintf("Compiler: clang %s",  __clang_version__));
#elif defined __GNUC__ && defined __VERSION__
    compiler.append(QString().asprintf("Compiler: gcc %s", __VERSION__));
#elif defined _MSC_VER
    compiler.append("Compiler: Visual Studio");
#if 0    
    /*
     * https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
     */
#elif defined __VSCMD_VER
    /*
     * Special for Qt
     * add this in .pro file
     *
     * VSCMD_VER = $$(VSCMD_VER)
     * VSVERSION = $$(VisualStudioVersion)
     *
     * !isEmpty(VSCMD_VER) {
     *    message("~~~ VSCMD_VER $$(VSCMD_VER) ~~~")
     *    DEFINES += __VSCMD_VER=\\\"$$(VSCMD_VER)\\\"
     *    DEFINES += __VSVERSION=$$(VisualStudioVersion)
     * }
     */
    compiler.append(QString().asprintf(" %d / MSVC++ %s", 2013 + (((int)__VSVERSION)-13) * 2
                                       + (((int)__VSVERSION) > 16 ? 1 : 0)
                                       , __VSCMD_VER));
#elif _MSC_VER >= 1930
    compiler.append(" 2022 / MSVC++ ").append(QString().asprintf("17.%d",((_MSC_VER % 100) - 30)));
#elif _MSC_VER >= 1929

#if _MSC_FULL_VER >= 192930100
    compiler.append(" 2019 / MSVC++ 16.11");
#else
    compiler.append(" 2019 / MSVC++ 16.10");
#endif
#elif _MSC_VER >= 1928
#if _MSC_FULL_VER >= 192829500
    compiler.append(" 2019 / MSVC++ 16.9");
#else
    compiler.append(" 2019 / MSVC++ 16.8");
#endif
#elif _MSC_VER >= 1920
    compiler.append(" 2019 / MSVC++ 16.").append(QString().asprintf("%d",((_MSC_VER % 100) - 20)));
#elif _MSC_VER > 1911
    compiler.append(" 2017 / MSVC++ 15.").append(QString().asprintf("%d",((_MSC_VER % 100) - 7)));
#elif _MSC_VER == 1911
    compiler.append(" 2017 / MSVC++ 15.3");
#elif _MSC_VER == 1910
    compiler.append(" 2017 / MSVC++ 15.0");
#elif _MSC_VER == 1900
    compiler.append(" 2015 / MSVC++ 14.0");
#else
    compiler.append(", unrecognised version");
#endif
#ifndef QT_NO_DEBUG_OUTPUT
    compiler.append(QString().asprintf("\n_MSC_VER=%d, _MSC_FULL_VER=%d", (int)_MSC_VER,(int)_MSC_FULL_VER));
#endif
#endif
    return compiler;
}
