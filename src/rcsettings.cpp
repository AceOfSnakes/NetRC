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
#include "rcsettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QApplication>

RCSettings::RCSettings() {
}

QVariant RCSettings::load(QString family) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("family");
    QVariant val = sets.value(family);
    sets.endGroup();
    sets.endGroup();
    return val;
}

void RCSettings::remove(QString family) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("family");
    sets.remove(family);
    sets.endGroup();
    sets.beginGroup("devices");
    foreach(QString key, sets.childGroups()) {
        sets.beginGroup(key);
        QString fam =sets.value("deviceFamily").toString();
        sets.endGroup();
        if(fam == family) {
            sets.remove(key);
        }
    }
    sets.endGroup();
    sets.endGroup();
}


QVariant RCSettings::load(QFile &file) {
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray array;
        array = file.readAll();
        QJsonDocument document = QJsonDocument::fromJson(array);
        file.close();
        QVariant conf = document.toVariant();
        QString key = conf.toMap().value("family").toString();
        updateSettings(key, conf);
        return conf;
    }
    return QVariant();
}

void RCSettings::updateSettings(QString & family, QVariant value) {
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("family");
    sets.setValue(family,value);
    sets.endGroup();
    sets.endGroup();
}

QList<QString> RCSettings::settingsList() {
    QStringList result;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("family");
    result = sets.allKeys();
    sets.endGroup();
    sets.endGroup();
    return result ;
}

bool RCSettings::isMinimizeToTrayEnabled() {
    bool result = false;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    result = sets.value("minimizeToTray", true).toBool();
    sets.endGroup();
    sets.endGroup();
    return result;
}

bool RCSettings::isDevelopmentModeEnabled() {
    bool result = false;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    result = sets.value("devMode", false).toBool();
    sets.endGroup();
    sets.endGroup();
    return result;
}

bool RCSettings::isSaveLatestDeviceEnabled() {
    bool result = false;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("view");
    result = sets.value("saveLatestDevice", true).toBool();
    sets.endGroup();
    sets.endGroup();
    return result;
}


