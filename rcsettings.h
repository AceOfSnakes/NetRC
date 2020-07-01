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
    QList<QString> static settingsList();
};

#endif // RCSETTINGS_H
