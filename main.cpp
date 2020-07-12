#include "remotecontrol.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDateTime>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("NetRC");
    app.setOrganizationName("Ace Of Snakes");

    RemoteControl w;

    QLocale::setDefault(QLocale::English);
    QLocale myLoc;
    QDateTime date = myLoc.toDateTime(QString(__DATE__).replace("  "," ").trimmed(),"MMM d yyyy");
    qDebug()<<date;
    w.setWindowTitle(qApp->applicationName());
    app.setApplicationVersion(date.toString("yy.MM").append("-alpha based on Qt " ).append(QT_VERSION_STR)
                          #ifdef Q_OS_WIN64
                              .append(" x64")
                          #endif
                              );
    w.show();
    return app.exec();
}
