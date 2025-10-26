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
#ifndef DEBUG_H
#define DEBUG_H

#include <QDialog>
#include "deviceinterface.h"

namespace Ui {
class Debug;
}

class Debug : public QDialog
{
    Q_OBJECT
    DeviceInterface *devInterface;

public:
    QMap<QString,QIcon> originalIcons;
    explicit Debug(DeviceInterface *deviceInterface, QWidget *parent = nullptr);
    ~Debug();
public slots:
    void error(const QString str);
    void read(const QString str);
    void write(const QString str);
    void warn(const QString str);
private slots:
    void changeMaxLines();
    void pauseClicked();
    void cleanDebugOutput();
    void sendCommand();
    void clearCommandLine();
    void switchAll(int arg1);
private:
    Ui::Debug *ui;
    bool pause = false;
    QSet<QString> txFilter;
    QSet<QString> rxFilter;
    bool isRqNotDisplayed(const QString str);
    bool isRsNotDisplayed(const QString str);
signals:
    void send(const QString str);
    void iconChanged(QPushButton & button);
};

#endif // DEBUG_H
