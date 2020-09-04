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
#ifndef DEVICECONNECTOR_H
#define DEVICECONNECTOR_H

#include <QDialog>
#include <QVariant>
#include <QRegularExpression>

namespace Ui {
class DeviceConnector;
}

class DeviceConnector : public QDialog
{
    Q_OBJECT
private:
public:
    explicit DeviceConnector(QVariant & sets, QWidget *parent = nullptr);
    ~DeviceConnector();
    QString  device;
    QString  deviceAddress;
    QString  deviceIPAddress;
    int      devicePort;
    QString  deviceFamily;
    QVariant settings;
    void setDevice(QString deviceFamily, QString device, QString address, int port);
private slots:
    void on_pushButtonAuto_clicked();
    void on_closeButton_clicked();

    void on_loadConfig_clicked();

//    void on_deviceProtocol_activated(const QString &arg1);

    void on_deviceProtocol_currentIndexChanged(const QString &arg1);

    void on_deleteFamily_clicked();

    void on_deleteFamily_pressed();

private:
    Ui::DeviceConnector *ui;
    void reloadDevicesFamily();
    QString getIpAddress();
    void select(const QVariant action);
    void setIpAddress(QString ip1, QString ip2, QString ip3, QString ip4, QString port);
};

#endif // DEVICECONNECTOR_H
