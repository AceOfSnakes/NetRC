#include "rcsettings.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QApplication>

RCSettings::RCSettings() {
}

QVariant RCSettings::load(QString family) {
    QString result;
    QSettings sets(qApp->organizationName(), qApp->applicationName());
    sets.beginGroup("global");
    sets.beginGroup("family");
    QVariant val = sets.value(family);
    result =  val.toMap().value("family").toString();
    sets.endGroup();
    sets.endGroup();
    return val;
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


