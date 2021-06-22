#include "commons.h"

Commons::Commons() {

}

QString Commons::compilerQString() {
    QString compiler;
#ifdef __clang_version__
    compiler.append(QString().asprintf("Compiler: clang %s",  __clang_version__));
#elif defined __GNUC__ && defined __VERSION__
    compiler.append(QString().asprintf("Compiler: gcc %s", __VERSION__));
#elif defined _MSC_VER
    compiler.append("Compiler: Visual Studio");
#if _MSC_VER >= 1929
    compiler.append(" 2019 / MSVC++ 16.").append(QString().asprintf("%d",((_MSC_VER % 100) - 19)));
#elif _MSC_VER >= 1928
#if _MSC_FULL_VER >= 192829500
    // once more again M$ version out of control :(
    // https://developercommunity.visualstudio.com/t/the-169-cc-compiler-still-uses-the-same-version-nu/1335194#T-N1337120
    compiler.append(" 2019 / MSVC++ 16.").append(QString().asprintf("%d",((_MSC_VER % 100) - 19)));
#else
    compiler.append(" 2019 / MSVC++ 16.").append(QString().asprintf("%d",((_MSC_VER % 100) - 20)));
#endif
#elif _MSC_VER < 1928
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
#elif _MSC_VER == 1800
    compiler.append(" 2013 / MSVC++ 12.0");
#elif _MSC_VER == 1700
    compiler.append(" 2012 / MSVC++ 11.0");
#elif _MSC_VER == 1600
    compiler.append(" 2010 / MSVC++ 10.0");
#elif  _MSC_VER == 1500
    compiler.append(" 2008 / MSVC++ 9.0");
#elif  _MSC_VER == 1400
    compiler.append(" 2005 / MSVC++ 8.0");
#elif  _MSC_VER == 1310
    compiler.append(" .NET 2003 / MSVC++ 7.1");
#elif  _MSC_VER == 1300
    compiler.append(" .NET 2002 / MSVC++ 7.0");
#elif  _MSC_VER == 1300
    compiler.append(" .NET 2002 / MSVC++ 7.0");
#else
    compiler.append(", unrecognised version");
#endif
    compiler.append(QString().asprintf(" (_MSC_VER=%d, _MSC_FULL_VER=%d)", (int)_MSC_VER,(int)_MSC_FULL_VER));
#endif
    return compiler;
}
