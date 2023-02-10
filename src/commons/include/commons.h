#ifndef COMMONS_H
#define COMMONS_H

#include <QString>
#include <QEvent>
#include <QMainWindow>

class Commons
{
public:
    Commons();
    QString static compilerQString();
    QString static prettyProductName();
    void static moveWindow(QObject *obj, QEvent *event, QMainWindow *window);
};

#endif // COMMONS_H
