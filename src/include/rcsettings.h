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
#ifndef RCSETTINGS_H
#define RCSETTINGS_H

#include <QFile>
#include <QSettings>

class RCSettings :public QObject
{
private:
    void static updateSettings(QString & family, QVariant value);
public:
    RCSettings();
    QVariant static load(QFile &file);
    QVariant static load(const QString family);
    QVariant static deviceSettings(QString device);
    QList<QString> static settingsList();
    QList<QString> static devicesList();
    void static removeDevice(QString device);
    void static remove(QString family);
    bool static isMinimizeToTrayEnabled();
    bool static isSaveLatestDeviceEnabled();
    bool static isDevelopmentModeEnabled();
};

#endif // RCSETTINGS_H
