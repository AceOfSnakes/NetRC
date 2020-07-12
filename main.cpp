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
