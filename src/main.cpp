/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "remotecontrol.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include <QDateTime>
#include <QFontDatabase>
#include <QStyleFactory>
#include <QNetworkProxyFactory>

int main(int argc, char *argv[]) {

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QApplication app(argc, argv);
    app.setApplicationName("NetRC");
    app.setOrganizationName("Ace Of Snakes");
    //QFontDatabase::addApplicationFont(":/fonts/font.ttf");
    RemoteControl w;

    QLocale::setDefault(QLocale::English);
    QLocale myLoc;
    QDateTime date = myLoc.toDateTime(QString(__DATE__).replace("  "," ").trimmed(),"MMM d yyyy");
    w.setWindowTitle(qApp->applicationName());
    QDate  first;
    first.setDate(date.date().year(),date.date().month(),1);
    int week = date.date().weekNumber() -
            (first.weekNumber() > date.date().weekNumber() ? 0 : first.weekNumber());
    app.setApplicationVersion(date.toString("yy.MM")
                              .append(week == 0 ? QString() : QString().asprintf(".%d", week))
                          #ifdef STATIC                                   
                              .append(" (static)" )
                          #endif
                              .append(" based on Qt " ).append(qVersion())
                          #if Q_PROCESSOR_WORDSIZE == 8
                              .append(" x64")
                          #endif
                              );
    w.show();
    int ret = app.exec();
    return ret;
}
